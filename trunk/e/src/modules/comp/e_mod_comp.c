#include "e.h"
#include "e_mod_main.h"
#include "e_mod_comp.h"
#include "e_mod_comp_update.h"
#include "config.h"

//////////////////////////////////////////////////////////////////////////
//
// TODO (no specific order):
//   1. abstract evas object and compwin so we can duplicate the object N times
//      in N canvases - for winlist, everything, pager etc. too
//   2. implement "unmapped composite cache" -> N pixels worth of unmapped
//      windows to be fully composited. only the most active/recent.
//   3. for unmapped windows - when window goes out of unmapped comp cache
//      make a miniature copy (1/4 width+height?) and set property on window
//      with pixmap id
//   6. other engine fast-paths (gl specifically)!
//   8. transparenty property
//   9. shortcut lots of stuff to draw inside the compositor - shelf,
//      wallpaper, efm - hell even menus and anything else in e (this is what
//      e18 was mostly about)
//  10. fullscreen windows need to be able to bypass compositing
//  11. handle root window resize!
//  
//////////////////////////////////////////////////////////////////////////

struct _E_Comp
{
   Ecore_X_Window  win;
   Ecore_Evas     *ee;
   Ecore_X_Window  ee_win;
   Evas           *evas;
   E_Manager      *man;
   Eina_Inlist    *wins;
   Eina_List      *updates;
   Ecore_Animator *render_animator;
   Ecore_Job      *update_job;
   int             render_overflow;

   Eina_Bool       gl : 1;
   Eina_Bool       grabbed : 1;
   Eina_Bool       nocomp : 1;
};

struct _E_Comp_Win
{
   EINA_INLIST;
   
   E_Comp               *c; // parent compositor
   Ecore_X_Window        win; // raw window - for menus etc.
   E_Border             *bd; // if its a border - later
   E_Popup              *pop; // if its a popup - later
   E_Menu               *menu; // if it is a menu - later
   int                   x, y, w, h; // geometry
   int                   pw, ph; // pixmap w/h
   int                   border; // border width
   Ecore_X_Pixmap        pixmap; // the compositing pixmap
   Ecore_X_Damage        damage; // damage region
   Ecore_X_Visual        vis; // window visual
   int                   depth; // window depth
   Evas_Object          *obj; // composite object
   Evas_Object          *shobj; // shadow object
   Ecore_X_Image        *xim; // x image - software fallback
   E_Update             *up; // update handler
   E_Object_Delfn       *dfn; // delete function handle for objects being tracked
   Ecore_X_Sync_Counter  counter; // sync counter for syncronised drawing
   Ecore_Timer          *update_timeout; // max time between damage and "done" event
   int                   dmg_updates; // num of damage event updates since a redirect
   Ecore_X_Rectangle    *rects; // shape rects... if shaped :(
   int                   rects_num; // num rects above
   
   Eina_List            *effects; // list of effects attached to this window currently

   Ecore_X_Pixmap        cache_pixmap; // the cached pixmap (1/nth the dimensions)
   int                   cache_w, cache_h; // cached pixmap size
   int                   update_count; // how many updates have happend to this win
   double                last_visible_time; // last time window was visible
   double                last_draw_time; // last time window was damaged
   
   Eina_Bool             visible : 1; // is visible
   Eina_Bool             input_only : 1; // is input_only
   Eina_Bool             override : 1; // is override-redirect
   Eina_Bool             argb : 1; // is argb
   Eina_Bool             shaped : 1; // is shaped
   Eina_Bool             update : 1; // has updates to fetch
   Eina_Bool             redirected : 1; // has updates to fetch
   Eina_Bool             shape_changed : 1; // shape changed
   Eina_Bool             native : 1; // native
   Eina_Bool             drawme : 1; // drawme flag fo syncing rendering
   Eina_Bool             invalid : 1; // invalid depth used - just use as marker
   Eina_Bool             defer_hide : 1; // flag to get hide to work on deferred hide
   Eina_Bool             delete_me : 1; // delete me!
   Eina_Bool             nocomp : 1; // nocomp applied
};

static Eina_List *handlers = NULL;
static Eina_List *compositors = NULL;
static Eina_Hash *windows = NULL;
static Eina_Hash *borders = NULL;
static Eina_Hash *damages = NULL;

//////////////////////////////////////////////////////////////////////////
// not working yet.
typedef struct _Effect Effect;

struct _Effect
{
   E_Comp_Win *cw;
   Ecore_Animator *animator;
   double start, len;
   double src, dst;
   int type;
};

#define EFFECT_SHOW 1
#define EFFECT_HIDE 2
#define EFFECT_MOVE 3

static Effect *
_e_mod_comp_effect_add(E_Comp_Win *cw, int (*func) (void *data), double len, int type)
{
   Effect *ef;
   
   ef = calloc(1, sizeof(Effect));
   ef->animator = ecore_animator_add(func, ef);
   ef->start = ecore_loop_time_get();
   ef->len = len;
   ef->cw = cw;
   ef->type = type;
   cw->effects = eina_list_append(cw->effects, ef);
   return ef;
}

static void
_e_mod_comp_effect_del(Effect *ef)
{
   ef->cw->effects = eina_list_remove(ef->cw->effects, ef);
   ecore_animator_del(ef->animator);
   free(ef);
}

static Effect *
_e_mod_comp_win_effect_find(E_Comp_Win *cw, int type)
{
   Eina_List *l;
   Effect *ef;
   
   // fixme: use hash if compositors list > 4
   EINA_LIST_FOREACH(cw->effects, l, ef)
     {
        if (ef->type == type) return ef;
     }
   return NULL;
}

static void _e_mod_comp_win_hide(E_Comp_Win *cw); // quick and dirty
static void _e_mod_comp_win_del(E_Comp_Win *cw); // quick and dirty
static void _e_mod_comp_win_render_queue(E_Comp_Win *cw); // quick and dirty

static int
_e_mod_comp_ef_cb_fade_to(void *data)
{
   Effect *ef = data;
   double t = (ecore_loop_time_get() - ef->start) / ef->len;
   E_Comp_Win *cw = ef->cw;
   double v;
   int col;
   if (t > 1.0) t = 1.0;
   
   v = ef->src + ((ef->dst - ef->src) * t);
   col = 255.0 * v;
   evas_object_color_set(cw->obj, col, col, col, col);
   if (ef->cw->shobj)
     evas_object_color_set(cw->shobj, col, col, col, col);
   if (t >= 1.0)
     {
        evas_object_color_set(cw->obj, 255, 255, 255, 255);
        _e_mod_comp_effect_del(ef);
        _e_mod_comp_win_render_queue(cw);
        if (!cw->effects)
          {
             if (cw->defer_hide)
               {
                  cw->defer_hide = 0;
                  _e_mod_comp_win_hide(cw);
               }
             if (cw->delete_me)
               _e_mod_comp_win_del(cw);
          }
        return 0;
     }
   return 1;
}

static void
_e_mod_comp_win_show_effects_add(E_Comp_Win *cw)
{
   Effect *ef;
   
   do
     {
        ef = _e_mod_comp_win_effect_find(cw, EFFECT_SHOW);
        if (ef) _e_mod_comp_effect_del(ef);
     }
   while (ef);
   
   do
     {
        ef = _e_mod_comp_win_effect_find(cw, EFFECT_HIDE);
        if (ef) _e_mod_comp_effect_del(ef);
     }
   while (ef);
   
   if (_comp_mod->conf->effect_fade)
     {
        ef = _e_mod_comp_effect_add(cw, _e_mod_comp_ef_cb_fade_to, 0.2, EFFECT_SHOW);
        ef->src = 0.0;
        ef->dst = 1.0;
        cw->defer_hide = 0;
        _e_mod_comp_ef_cb_fade_to(ef);
     }
}

static void
_e_mod_comp_win_hide_effects_add(E_Comp_Win *cw)
{
   Effect *ef;
   
   do
     {
        ef = _e_mod_comp_win_effect_find(cw, EFFECT_SHOW);
        if (ef) _e_mod_comp_effect_del(ef);
     }
   while (ef);
   
   do
     {
        ef = _e_mod_comp_win_effect_find(cw, EFFECT_HIDE);
        if (ef) _e_mod_comp_effect_del(ef);
     }
   while (ef);
   
   if (_comp_mod->conf->effect_fade)
     {
        int a;
        
        ef = _e_mod_comp_effect_add(cw, _e_mod_comp_ef_cb_fade_to, 0.2, EFFECT_HIDE);
        evas_object_color_get(cw->obj, NULL, NULL, NULL, &a);
        ef->src = (double)a / 255.0;
        ef->dst = 0.0;
        cw->defer_hide = 1;
        _e_mod_comp_ef_cb_fade_to(ef);
     }
}

static void
_e_mod_comp_win_move_effects_add(E_Comp_Win *cw)
{
}

//////////////////////////////////////////////////////////////////////////

#if 0
#define DBG(f, x...) printf(f, ##x)
#else
#define DBG(f, x...)
#endif

static void _e_mod_comp_render_queue(E_Comp *c);
static void _e_mod_comp_win_damage(E_Comp_Win *cw, int x, int y, int w, int h, Eina_Bool dmg);

static E_Comp_Win *
_e_mod_comp_fullscreen_check(E_Comp *c)
{
   E_Comp_Win *cw;
   
   if (!c->wins) return NULL;
   EINA_INLIST_REVERSE_FOREACH(c->wins, cw)
     {
        if ((!cw->visible) || (cw->input_only) || (cw->invalid))
          continue;
        if ((cw->x == 0) && (cw->y == 0) &&
            ((cw->x + cw->w) >= c->man->w) &&
            ((cw->y + cw->h) >= c->man->h) &&
            (!cw->argb) && (!cw->shaped)
            )
          {
             return cw;
          }
        return NULL;
     }
   return NULL;
}

static inline Eina_Bool
_e_mod_comp_shaped_check(int w, int h, const Ecore_X_Rectangle *rects, int num)
{
   if ((!rects) || (num < 1)) return EINA_FALSE;
   if (num > 1) return EINA_TRUE;
   if ((rects[0].x == 0) && (rects[0].y == 0) &&
       (rects[0].width == w) && (rects[0].height == h))
     return EINA_FALSE;
   return EINA_TRUE;
}

static inline Eina_Bool
_e_mod_comp_win_shaped_check(const E_Comp_Win *cw, const Ecore_X_Rectangle *rects, int num)
{
   return _e_mod_comp_shaped_check(cw->w, cw->h, rects, num);
}

static void
_e_mod_comp_win_shape_rectangles_apply(E_Comp_Win *cw, const Ecore_X_Rectangle *rects, int num)
{
   int i;

   DBG("SHAPE [0x%x] change, rects=%p (%d)\n", cw->win, rects, num);
   if (!_e_mod_comp_win_shaped_check(cw, rects, num))
     {
        rects = NULL;
     }
   if (rects)
     {
        unsigned int *pix, *p;
        unsigned char *spix, *sp;
        int w, h, px, py;
        
        evas_object_image_size_get(cw->obj, &w, &h);
        if ((w > 0) && (h > 0))
          {
	     if (cw->native)
	       {
		  printf("BUGGER: shape with native surface? cw=%p\n", cw);
		  return;
	       }

             evas_object_image_native_surface_set(cw->obj, NULL);
             evas_object_image_alpha_set(cw->obj, 1);
             pix = evas_object_image_data_get(cw->obj, 1);
             if (pix)
               {
                  spix = calloc(w * h, sizeof(unsigned char));
                  if (spix)
                    {
                       DBG("SHAPE [0x%x] rects %i\n", cw->win, num);
                       for (i = 0; i < num; i++)
                         {
                            int rx, ry, rw, rh;
                            
                            rx = rects[i].x; ry = rects[i].y;
                            rw = rects[i].width; rh = rects[i].height;
                            E_RECTS_CLIP_TO_RECT(rx, ry, rw, rh, 0, 0, w, h);
                            sp = spix + (w * ry) + rx;
                            for (py = 0; py < rh; py++)
                              {
                                 for (px = 0; px < rw; px++)
                                   {
                                      *sp = 0xff; sp++;
                                   }
                                 sp += w - rw;
                              }
                         }
                       sp = spix;
                       p = pix;
                       for (py = 0; py < h; py++)
                         {
                            for (px = 0; px < w; px++)
                              {
                                 unsigned int mask, imask;
                                 
                                 mask = ((unsigned int)(*sp)) << 24;
                                 imask = mask >> 8;
                                 imask |= imask >> 8;
                                 imask |= imask >> 8;
                                 *p = mask | (*p & imask);
//                                 if (*sp) *p = 0xff000000 | *p;
//                                 else *p = 0x00000000;
                                 sp++;
                                 p++;
                              }
                         }
                       free(spix);
                    }
                  evas_object_image_data_set(cw->obj, pix);
                  evas_object_image_data_update_add(cw->obj, 0, 0, w, h);
               }
          }
     }
   else
     {
        if (cw->shaped)
          {
             unsigned int *pix, *p;
             int w, h, px, py;
        
             evas_object_image_size_get(cw->obj, &w, &h);
             if ((w > 0) && (h > 0))
               {
                  if (cw->native)
                    {
                       fprintf(stderr, "BUGGER: shape with native surface? cw=%p\n", cw);
                       return;
                    }

                  evas_object_image_alpha_set(cw->obj, 0);
                  pix = evas_object_image_data_get(cw->obj, 1);
                  if (pix)
                    {
                       p = pix;
                       for (py = 0; py < h; py++)
                         {
                            for (px = 0; px < w; px++)
                              *p |= 0xff000000;
                         }
                    }
                  evas_object_image_data_set(cw->obj, pix);
                  evas_object_image_data_update_add(cw->obj, 0, 0, w, h);
               }
          }
        // dont need to fix alpha chanel as blending 
        // should be totally off here regardless of
        // alpha channe; content
     }
}

static void
_e_mod_comp_win_update(E_Comp_Win *cw)
{
   E_Update_Rect *r;
   int i;

   if (_comp_mod->conf->grab) ecore_x_grab();
   cw->update = 0;
   
   if (cw->argb)
     {
        if (cw->rects)
          {
             free(cw->rects);
             cw->rects = NULL;
             cw->rects_num = 0;
          }
     }
   else
     {
        if (cw->shape_changed)
          {
             if (cw->rects)
               {
                  free(cw->rects);
                  cw->rects = NULL;
                  cw->rects_num = 0;
               }
             ecore_x_pixmap_geometry_get(cw->win, NULL, NULL, &(cw->w), &(cw->h));
             cw->rects = ecore_x_window_shape_rectangles_get(cw->win, &(cw->rects_num));
             if (cw->rects)
               {
                  for (i = 0; i < cw->rects_num; i++)
                    {
                       E_RECTS_CLIP_TO_RECT(cw->rects[i].x,
                                            cw->rects[i].y, 
                                            cw->rects[i].width,
                                            cw->rects[i].height, 
                                            0, 0, cw->w, cw->h);
                    }
               }
             if (!_e_mod_comp_win_shaped_check(cw, cw->rects, cw->rects_num))
               {
                  free(cw->rects);
                  cw->rects = NULL;
                  cw->rects_num = 0;
               }
             if ((cw->rects) && (!cw->shaped))
               {
                  cw->shaped = 1;
               }
             else if ((!cw->rects) && (cw->shaped))
               {
                  cw->shaped = 0;
               }
             
          }
     }

   if (!cw->pixmap)
     {
        cw->pixmap = ecore_x_composite_name_window_pixmap_get(cw->win);
        if (cw->pixmap)
          ecore_x_pixmap_geometry_get(cw->pixmap, NULL, NULL, &(cw->pw), &(cw->ph));
        else
          {
             cw->pw = 0;
             cw->ph = 0;
          }
        DBG("REND [0x%x] pixmap = [0x%x], %ix%i\n", cw->win, cw->pixmap, cw->pw, cw->ph);
        if ((cw->pw <= 0) || (cw->ph <= 0))
          {
             if (cw->native)
               {
                  DBG("  [0x%x] free native\n", cw->win);
                  evas_object_image_native_surface_set(cw->obj, NULL);
                  cw->native = 0;
               }
             if (cw->pixmap)
               {
                  DBG("  [0x%x] free pixmap\n", cw->win);
                  ecore_x_pixmap_free(cw->pixmap);
                  cw->pixmap = 0;
               }
             cw->pw = 0;
             cw->ph = 0;
          }
        cw->native = 0;
        e_mod_comp_update_resize(cw->up, cw->pw, cw->ph);
        e_mod_comp_update_add(cw->up, 0, 0, cw->pw, cw->ph);
     }
   if (!((cw->pw > 0) && (cw->ph > 0)))
     {
        if (_comp_mod->conf->grab) ecore_x_ungrab();
        return;
     }

   evas_object_move(cw->obj, cw->x, cw->y);
   if (cw->shobj)
     {
        evas_object_move(cw->shobj, cw->x, cw->y);
     }
   evas_object_resize(cw->obj, 
                      cw->pw + (cw->border * 2), 
                      cw->ph + (cw->border * 2));
   if (cw->shobj)
     {
        evas_object_resize(cw->shobj, 
                           cw->pw + (cw->border * 2),
                           cw->ph + (cw->border * 2));
     }

   if ((cw->c->gl) && (_comp_mod->conf->texture_from_pixmap) &&
       (!cw->shaped) && (!cw->rects)/* && (!cw->shape_changed)*/)
     {
//        DBG("DEBUG - pm now %x\n", ecore_x_composite_name_window_pixmap_get(cw->win));
        evas_object_image_size_set(cw->obj, cw->pw, cw->ph);
        if (!cw->native)
          {
             Evas_Native_Surface ns;
             
             ns.data.x11.visual = cw->vis;
             ns.data.x11.pixmap = cw->pixmap;
             evas_object_image_native_surface_set(cw->obj, &ns);
             DBG("NATIVE [0x%x] %x %ix%i\n", cw->win, cw->pixmap, cw->pw, cw->ph);
             cw->native = 1;
          }
        r = e_mod_comp_update_rects_get(cw->up);
        if (r) 
          {
             e_mod_comp_update_clear(cw->up);
             for (i = 0; r[i].w > 0; i++)
               {
                  int x, y, w, h;
                       
                  x = r[i].x; y = r[i].y;
                  w = r[i].w; h = r[i].h;
                  DBG("UPDATE [0x%x] pm [0x%x] %i %i %ix%i\n", cw->win, cw->pixmap, x, y, w, h);
                  evas_object_image_data_update_add(cw->obj, x, y, w, h);
               }
             free(r);
          }
        else
          cw->update = 1;
     }
   else
     {
        evas_object_image_native_surface_set(cw->obj, NULL);
        cw->native = 0;
        if (!cw->xim)
          {
             if (cw->xim = ecore_x_image_new(cw->pw, cw->ph, cw->vis, cw->depth))
               e_mod_comp_update_add(cw->up, 0, 0, cw->pw, cw->ph);
          }
        r = e_mod_comp_update_rects_get(cw->up);
        if (r) 
          {
             if (cw->xim)
               {
                  unsigned int *pix;
                  
                  pix = ecore_x_image_data_get(cw->xim, NULL, NULL, NULL);
                  evas_object_image_data_set(cw->obj, pix);
                  evas_object_image_size_set(cw->obj, cw->pw, cw->ph);
                  
                  e_mod_comp_update_clear(cw->up);
                  for (i = 0; r[i].w > 0; i++)
                    {
                       int x, y, w, h;
                       
                       x = r[i].x; y = r[i].y;
                       w = r[i].w; h = r[i].h;
                       if (!ecore_x_image_get(cw->xim, cw->pixmap, x, y, x, y, w, h))
                         {
                            e_mod_comp_update_add(cw->up, x, y, w, h);
                            cw->update = 1;
                         }
                       else
                         {
// why do we neeed these 2? this smells wrong
                            pix = ecore_x_image_data_get(cw->xim, NULL, NULL, NULL);
                            DBG("UPDATE [0x%x] %i %i %ix%i -- pix = %p\n", cw->win, x, y, w, h, pix);
                            evas_object_image_data_set(cw->obj, pix);
                       
                            evas_object_image_data_update_add(cw->obj, x, y, w, h);
                         }
                    }
               }
             free(r);
             if (cw->shaped)
               {
                  _e_mod_comp_win_shape_rectangles_apply(cw, cw->rects, cw->rects_num);
               }
             else
               {
                  if (cw->shape_changed)
                    _e_mod_comp_win_shape_rectangles_apply(cw, cw->rects, cw->rects_num);
               }
             cw->shape_changed = 0;
          }
        else
          cw->update = 1;
     }
   if ((!cw->update) && (cw->visible) && (cw->dmg_updates > 0))
     {
        if (!evas_object_visible_get(cw->obj))
          {
             evas_object_show(cw->obj);
             if (cw->shobj) evas_object_show(cw->shobj);
          }
     }
   if (cw->shobj)
     {
        if (cw->shaped) evas_object_hide(cw->shobj);
     }

   if (_comp_mod->conf->grab) ecore_x_ungrab();
}

static void
_e_mod_comp_pre_swap(void *data, Evas *e)
{
   E_Comp *c = data;
   
   if (_comp_mod->conf->grab)
     {
        if (c->grabbed)
          {
             ecore_x_ungrab();
             c->grabbed = 0;
          }
     }
}

static int
_e_mod_comp_cb_update(E_Comp *c)
{
   E_Comp_Win *cw;
   Eina_List *new_updates = NULL; // for failed pixmap fetches - get them next frame
   Eina_List *update_done = NULL;
   static int doframeinfo = -1;

   c->update_job = NULL;
   if (c->nocomp) goto nocomp;
   if (_comp_mod->conf->grab)
     {
        ecore_x_grab();
        ecore_x_sync();
        c->grabbed = 1;
     }
   EINA_LIST_FREE(c->updates, cw)
     {
        if (_comp_mod->conf->efl_sync)
          {
             if (((cw->counter) && (cw->drawme)) || (!cw->counter))
               {
                  _e_mod_comp_win_update(cw);
                  if (cw->drawme)
                    update_done = eina_list_append(update_done, cw);
                  cw->drawme = 0;
               }
             else
               cw->update = 0;
          }
        else
          _e_mod_comp_win_update(cw);
        if (cw->update)
          new_updates = eina_list_append(new_updates, cw);
     }
   if (_comp_mod->conf->lock_fps)
     ecore_evas_manual_render(c->ee);
   if (_comp_mod->conf->efl_sync)
     {  
        EINA_LIST_FREE(update_done, cw)
          {
             ecore_x_sync_counter_inc(cw->counter, 1);
          }
     }
   if (_comp_mod->conf->grab)
     {
        if (c->grabbed)
          {
             c->grabbed = 0;
             ecore_x_ungrab();
          }
     }
   if (new_updates) _e_mod_comp_render_queue(c);
   c->updates = new_updates;
   c->render_overflow--;
   
   if (doframeinfo == -1)
     {
        doframeinfo = 0;
        if (getenv("DFI")) doframeinfo = 1;
     }
   if (doframeinfo)
     {
        static double t0 = 0.0;
        double td, t;
        
        t = ecore_time_get();
        td = t - t0;
        if (td > 0.0)
          {
             int fps, i;
             
             fps = 1.0 / td;
             for (i = 0; i < fps; i+= 2) putchar('=');
             printf(" : %3.3f\n", 1.0 / td);
          }
        t0 = t;
     }

   nocomp:
   cw = _e_mod_comp_fullscreen_check(c);
   if (cw)
     {
        if (_comp_mod->conf->nocomp_fs)
          {
             if (!c->nocomp)
               {
                  printf("NOCOMP!\n");
                  printf("kill comp %x\n", cw->win);
                  c->nocomp = 1;
                  c->render_overflow = 1;
//                  ecore_x_composite_render_window_disable(c->win);
//                  ecore_x_window_shape_rectangle_set(c->win, -2, -2, 1, 1);
                  ecore_x_window_hide(c->win);
                  cw->nocomp = 1;
                  if (cw->redirected)
                    {
                       printf("undirect\n");
                       ecore_x_composite_unredirect_window(cw->win, ECORE_X_COMPOSITE_UPDATE_MANUAL);
                       cw->redirected = 0;
                       cw->pw = 0;
                       cw->ph = 0;
                    }
                  if (cw->native)
                    {
                       evas_object_image_native_surface_set(cw->obj, NULL);
                       cw->native = 0;
                    }
                  if (cw->pixmap)
                    {
                       ecore_x_pixmap_free(cw->pixmap);
                       cw->pixmap = 0;
                       cw->pw = 0;
                       cw->ph = 0;
                    }
                  if (cw->xim)
                    {
                       evas_object_image_size_set(cw->obj, 1, 1);
                       evas_object_image_data_set(cw->obj, NULL);
                       ecore_x_image_free(cw->xim);
                       cw->xim = NULL;
                    }
                  if (cw->damage)
                    {
                       Ecore_X_Region parts;
                       
                       eina_hash_del(damages, e_util_winid_str_get(cw->damage), cw);
                       parts = ecore_x_region_new(NULL, 0);
                       ecore_x_damage_subtract(cw->damage, 0, parts);
                       ecore_x_region_free(parts);
                       ecore_x_damage_free(cw->damage);
                       cw->damage = 0;
                    }
                  if (cw->update_timeout)
                    {
                       ecore_timer_del(cw->update_timeout);
                       cw->update_timeout = NULL;
                    }
                  if (cw->update)
                    {
                       cw->update = 0;
                       cw->c->updates = eina_list_remove(cw->c->updates, cw);
                    }
                  if (cw->counter)
                    {
                       printf("nosync\n");
                       ecore_x_e_comp_sync_cancel_send(cw->bd->client.win);
                    }
//                  ecore_x_window_hide(cw->win);
//                  ecore_x_window_show(cw->win);
                  _e_mod_comp_render_queue(c);
               }
          }
     }
   else
     {
        if (c->nocomp)
          {
             printf("COMP!\n");
             c->nocomp = 0;
             c->render_overflow = 1;
//             ecore_x_window_shape_rectangle_set(c->win, 0, 0, c->man->w, c->man->h);
             ecore_x_window_show(c->win);
//             c->win = ecore_x_composite_render_window_enable(c->man->root);
             EINA_INLIST_FOREACH(c->wins, cw)
               {
                  if (!cw->nocomp) continue;
                  cw->nocomp = 0;
                  printf("restore comp %x --- %x\n", cw->win, cw->pixmap);
                  if (cw->pixmap) ecore_x_pixmap_free(cw->pixmap);
                  cw->pixmap = 0;
                  cw->pw = 0;
                  cw->ph = 0;
                  cw->native = 0;
                  if (!cw->damage)
                    {
                       cw->damage = ecore_x_damage_new
                         (cw->win, ECORE_X_DAMAGE_REPORT_DELTA_RECTANGLES);
                       eina_hash_add(damages, e_util_winid_str_get(cw->damage), cw);
                    }
                  if (!cw->redirected)
                    {
                       printf("  redr\n");
                       ecore_x_composite_redirect_window(cw->win, ECORE_X_COMPOSITE_UPDATE_MANUAL);
                       cw->pixmap = ecore_x_composite_name_window_pixmap_get(cw->win);
                       if (cw->pixmap)
                         ecore_x_pixmap_geometry_get(cw->pixmap, NULL, NULL, &(cw->pw), &(cw->ph));
                       else
                         {
                            cw->pw = 0;
                            cw->ph = 0;
                         }
                       printf("  %x %ix%i\n", cw->pixmap, cw->pw, cw->ph);
                       if ((cw->pw <= 0) || (cw->ph <= 0))
                         {
                            ecore_x_pixmap_free(cw->pixmap);
                            cw->pixmap = 0;
                         }
                       cw->redirected = 1;
                       cw->dmg_updates = 0;
                    }
//                  _e_mod_comp_win_damage(cw, 0, 0, cw->w, cw->h, 0);
                  if (cw->visible)
                    {
                       evas_object_show(cw->obj);
                       if (cw->shobj) evas_object_show(cw->shobj);
                    }
                  _e_mod_comp_win_render_queue(cw);
                  if (cw->counter)
                    ecore_x_e_comp_sync_begin_send(cw->bd->client.win);
               }
          }
     }
   
   if (c->render_overflow == 0)
     {
        if (c->render_animator) c->render_animator = NULL;
        return 0;
     }
   return 1;
}

static void
_e_mod_comp_cb_job(void *data)
{
   _e_mod_comp_cb_update(data);
}

static int
_e_mod_comp_cb_animator(void *data)
{
   return _e_mod_comp_cb_update(data);
}

static void
_e_mod_comp_render_queue(E_Comp *c)
{
   if (_comp_mod->conf->lock_fps)
     {
        if (c->render_animator)
          {
             c->render_overflow = 10;
             return;
          }
        c->render_animator = ecore_animator_add(_e_mod_comp_cb_animator, c);
     }
   else
     {
        if (c->update_job)
          {
             ecore_job_del(c->update_job);
             c->update_job = NULL;
             c->render_overflow = 0;
          }
        c->update_job = ecore_job_add(_e_mod_comp_cb_job, c);
     }
}

static void
_e_mod_comp_win_render_queue(E_Comp_Win *cw)
{
   _e_mod_comp_render_queue(cw->c);
}

static E_Comp *
_e_mod_comp_find(Ecore_X_Window root)
{
   Eina_List *l;
   E_Comp *c;
   
   // fixme: use hash if compositors list > 4
   EINA_LIST_FOREACH(compositors, l, c)
     {
        if (c->man->root == root) return c;
     }
   return NULL;
}

static E_Comp_Win *
_e_mod_comp_win_find(Ecore_X_Window win)
{
   return eina_hash_find(windows, e_util_winid_str_get(win));
}

static E_Comp_Win *
_e_mod_comp_border_client_find(Ecore_X_Window win)
{
   return eina_hash_find(borders, e_util_winid_str_get(win));
}

static E_Comp_Win *
_e_mod_comp_win_damage_find(Ecore_X_Damage damage)
{
   return eina_hash_find(damages, e_util_winid_str_get(damage));
}

static Eina_Bool
_e_mod_comp_win_is_borderless(E_Comp_Win *cw)
{
   if (!cw->bd) return 1;
   if ((cw->bd->client.border.name) &&
       (!strcmp(cw->bd->client.border.name, "borderless")))
     return 1;
   return 0;
}

static Eina_Bool
_e_mod_comp_win_do_shadow(E_Comp_Win *cw)
{
   if (cw->shaped) return 0;
   if (cw->argb)
     {
        if (_e_mod_comp_win_is_borderless(cw)) return 0;
     }
   return 1;
}

static int
_e_mod_comp_win_damage_timeout(void *data)
{
   E_Comp_Win *cw = data;
   
   if (!cw->update)
     {
        if (cw->update_timeout)
          {
             ecore_timer_del(cw->update_timeout);
             cw->update_timeout = NULL;
          }
        cw->update = 1;
        cw->c->updates = eina_list_append(cw->c->updates, cw);
     }
   cw->drawme = 1;
   _e_mod_comp_win_render_queue(cw);
   cw->update_timeout = NULL;
   return 0;
}

static void
_e_mod_comp_object_del(void *data, void *obj)
{
   E_Comp_Win *cw = data;

   _e_mod_comp_win_render_queue(cw);
   if (obj == cw->bd)
     {
        eina_hash_del(borders, e_util_winid_str_get(cw->bd->client.win), cw);
        cw->bd = NULL;
        cw->counter = 0;
     }
   else if (obj == cw->pop) cw->pop = NULL;
   else if (obj == cw->menu) cw->menu = NULL;
   if (cw->dfn)
     {
        e_object_delfn_del(obj, cw->dfn);
        cw->dfn = NULL;
     }
}

static E_Comp_Win *
_e_mod_comp_win_add(E_Comp *c, Ecore_X_Window win)
{
   Ecore_X_Window_Attributes att;
   E_Comp_Win *cw;
   
   cw = calloc(1, sizeof(E_Comp_Win));
   if (!cw) return NULL;
   cw->win = win;
   cw->c = c;
   cw->bd = e_border_find_by_window(cw->win);
   if (_comp_mod->conf->grab) ecore_x_grab();
   if (cw->bd)
     {
        eina_hash_add(borders, e_util_winid_str_get(cw->bd->client.win), cw);
        cw->dfn = e_object_delfn_add(E_OBJECT(cw->bd), 
                                     _e_mod_comp_object_del, cw);
        if (_comp_mod->conf->efl_sync)
          {
             if (_e_mod_comp_win_is_borderless(cw) ||
                 (_comp_mod->conf->loose_sync))
               {
                  cw->counter = 
                    ecore_x_e_comp_sync_counter_get(cw->bd->client.win);
                  if (cw->counter)
                    {
                       ecore_x_sync_counter_inc(cw->counter, 1);
                       ecore_x_e_comp_sync_begin_send(cw->bd->client.win);
                    }
               }
             else
               ecore_x_e_comp_sync_cancel_send(cw->bd->client.win);
          }
     }
   else
     {
        cw->pop = e_popup_find_by_window(cw->win);
        if (cw->pop)
          cw->dfn = e_object_delfn_add(E_OBJECT(cw->pop), 
                                       _e_mod_comp_object_del, cw);
        else
          {
             cw->menu = e_menu_find_by_window(cw->win);
             if (cw->menu)
               cw->dfn = e_object_delfn_add(E_OBJECT(cw->menu), 
                                            _e_mod_comp_object_del, cw);
          }
     }
   // fixme: could use bd/pop/menu for this too
   memset((&att), 0, sizeof(Ecore_X_Window_Attributes));
   if (!ecore_x_window_attributes_get(cw->win, &att))
     {
        free(cw);
        if (_comp_mod->conf->grab) ecore_x_ungrab();
        return NULL;
     }
   if ((!att.input_only) && 
       ((att.depth != 24) && (att.depth != 32)))
     {
        printf("WARNING: window 0x%x not 24/32bpp -> %ibpp\n", cw->win, att.depth);
        cw->invalid = 1;
     }
   cw->input_only = att.input_only;
   cw->override = att.override;
   cw->vis = att.visual;
   cw->depth = att.depth;
   cw->argb = ecore_x_window_argb_get(cw->win);
   eina_hash_add(windows, e_util_winid_str_get(cw->win), cw);
   if ((!cw->input_only) && (!cw->invalid))
     {
        Ecore_X_Rectangle *rects;
        int num;
        
        cw->damage = ecore_x_damage_new
          (cw->win, ECORE_X_DAMAGE_REPORT_DELTA_RECTANGLES);
        eina_hash_add(damages, e_util_winid_str_get(cw->damage), cw);
        cw->shobj = edje_object_add(c->evas);
        cw->obj = evas_object_image_filled_add(c->evas);
        evas_object_image_colorspace_set(cw->obj, EVAS_COLORSPACE_ARGB8888);
        if (cw->argb) evas_object_image_alpha_set(cw->obj, 1);
        else evas_object_image_alpha_set(cw->obj, 0);
        ecore_x_window_shape_events_select(cw->win, 1);
        rects = ecore_x_window_shape_rectangles_get(cw->win, &num);
        if (rects)
          {
             int i;
             
             if (rects)
               {
                  for (i = 0; i < num; i++)
                    {
                       E_RECTS_CLIP_TO_RECT(rects[i].x, rects[i].y, 
                                            rects[i].width, rects[i].height, 
                                            0, 0, att.w, att.h);
                    }
               }
	     if (!_e_mod_comp_shaped_check(att.w, att.h, rects, num))
	       {
                  free(rects);
                  rects = NULL;
               }
             if (rects)
               {
                  cw->shape_changed = 1;
                  free(rects);
               }
          }
     }
   else
     {
        cw->obj = evas_object_rectangle_add(c->evas);
        evas_object_color_set(cw->obj, 0, 0, 0, 64);
     }
   c->wins = eina_inlist_append(c->wins, EINA_INLIST_GET(cw));
   cw->up = e_mod_comp_update_new();
   e_mod_comp_update_tile_size_set(cw->up, 32, 32);
   // for software:
   e_mod_comp_update_policy_set
     (cw->up, E_UPDATE_POLICY_HALF_WIDTH_OR_MORE_ROUND_UP_TO_FULL_WIDTH);
   if (((!cw->input_only) && (!cw->invalid)) && (cw->override))
     {
        cw->redirected = 1;
        ecore_x_composite_redirect_window(cw->win, ECORE_X_COMPOSITE_UPDATE_MANUAL);
        cw->dmg_updates = 0;
     }
   DBG("  [0x%x] add\n", cw->win);
   if (_comp_mod->conf->grab) ecore_x_ungrab();
   return cw;
}

static void
_e_mod_comp_win_del(E_Comp_Win *cw)
{
   e_mod_comp_update_free(cw->up);
   DBG("  [0x%x] del\n", cw->win);
   while (cw->effects) _e_mod_comp_effect_del(cw->effects->data);
   if (cw->rects)
     {
        free(cw->rects);
        cw->rects = NULL;
     }
   if (cw->update_timeout)
     {
        ecore_timer_del(cw->update_timeout);
        cw->update_timeout = NULL;
     }
   if (cw->dfn)
     {
        if (cw->bd)
          {
             eina_hash_del(borders, e_util_winid_str_get(cw->bd->client.win), cw);
             e_object_delfn_del(E_OBJECT(cw->bd), cw->dfn);
          }
        else if (cw->pop) e_object_delfn_del(E_OBJECT(cw->pop), cw->dfn);
        else if (cw->menu) e_object_delfn_del(E_OBJECT(cw->menu), cw->dfn);
        cw->dfn = NULL;
     }
   if (cw->pixmap)
     {
        ecore_x_pixmap_free(cw->pixmap);
        cw->pixmap = 0;
        cw->pw = 0;
        cw->ph = 0;
     }
   if (cw->redirected)
     {
        ecore_x_composite_unredirect_window(cw->win, ECORE_X_COMPOSITE_UPDATE_MANUAL);
        cw->redirected = 0;
        cw->pw = 0;
        cw->ph = 0;
     }
// segv?   
//   evas_object_image_native_surface_set(cw->obj, NULL);
//   cw->native = 0;
   if (cw->update)
     {
        cw->update = 0;
        cw->c->updates = eina_list_remove(cw->c->updates, cw);
     }
   if (cw->xim)
     {
        evas_object_image_data_set(cw->obj, NULL);
        ecore_x_image_free(cw->xim);
        cw->xim = NULL;
     }
   if (cw->obj)
     {
        evas_object_del(cw->obj);
        cw->obj = NULL;
     }
   if (cw->shobj)
     {
        evas_object_del(cw->shobj);
        cw->shobj = NULL;
     }
   eina_hash_del(windows, e_util_winid_str_get(cw->win), cw);
   if (cw->damage)
     {
        Ecore_X_Region parts;
   
        eina_hash_del(damages, e_util_winid_str_get(cw->damage), cw);
        parts = ecore_x_region_new(NULL, 0);
        ecore_x_damage_subtract(cw->damage, 0, parts);
        ecore_x_region_free(parts);
        ecore_x_damage_free(cw->damage);
        cw->damage = 0;
     }
   cw->c->wins = eina_inlist_remove(cw->c->wins, EINA_INLIST_GET(cw));
   free(cw);
}

static void
_e_mod_comp_win_show(E_Comp_Win *cw)
{
   if (cw->visible) return;
   cw->visible = 1;
   DBG("  [0x%x] sho ++++++++++\n", cw->win);
   if ((cw->input_only) || (cw->invalid)) return;
   if (cw->pixmap) ecore_x_pixmap_free(cw->pixmap);
   evas_object_image_size_set(cw->obj, cw->pw, cw->ph);
/*   
   cw->pixmap = ecore_x_composite_name_window_pixmap_get(cw->win);
   if (cw->pixmap)
     ecore_x_pixmap_geometry_get(cw->pixmap, NULL, NULL, &(cw->pw), &(cw->ph));
   else
     {
        cw->pw = 0;
        cw->ph = 0;
     }
   if ((cw->pw <= 0) || (cw->ph <= 0))
     {
        ecore_x_pixmap_free(cw->pixmap);
        cw->pixmap = 0;
     }
   if ((cw->c->gl) && (_comp_mod->conf->texture_from_pixmap) &&
       (!cw->shaped) && (!cw->rects)
 // && (!cw->shape_changed)
       )
//   if ((cw->c->gl) && (_comp_mod->conf->texture_from_pixmap) &&
//       (!cw->shaped) && (!cw->shape_changed) && (cw->pixmap))
     {
        Evas_Native_Surface ns;
        
        ns.data.x11.visual = cw->vis;
        ns.data.x11.pixmap = cw->pixmap;
        evas_object_image_native_surface_set(cw->obj, &ns);
        DBG("NATIVE SHOW1 [0x%x] %x %ix%i\n", cw->win, cw->pixmap, cw->pw, cw->ph);
     }
*/
   if (cw->pixmap) ecore_x_pixmap_free(cw->pixmap);
   cw->pixmap = 0; 
   cw->pw = 0;
   cw->ph = 0;
   cw->native = 0;
   if (!cw->redirected)
     {
        ecore_x_composite_redirect_window(cw->win, ECORE_X_COMPOSITE_UPDATE_MANUAL);
        cw->pixmap = ecore_x_composite_name_window_pixmap_get(cw->win);
        if (cw->pixmap)
          ecore_x_pixmap_geometry_get(cw->pixmap, NULL, NULL, &(cw->pw), &(cw->ph));
        else
          {
             cw->pw = 0;
             cw->ph = 0;
          }
        if ((cw->pw <= 0) || (cw->ph <= 0))
          {
             ecore_x_pixmap_free(cw->pixmap);
             cw->pixmap = 0;
          }
        cw->redirected = 1;
        cw->dmg_updates = 0;
     }
   if (cw->shobj)
     {
        if (_comp_mod->conf->use_shadow)
          {
             if (_e_mod_comp_win_do_shadow(cw))
               {
                  int ok = 0;
                  char buf[PATH_MAX];
                  
                  // fimxe: make shadow object configurable - use theme first
                  if (_comp_mod->conf->shadow_file)
                    {
                       ok = 1;
                       if (!edje_object_file_set(cw->shobj, 
                                                 _comp_mod->conf->shadow_file,
                                                 "shadow"))
                         ok = 0;
                    }
                  if (!ok)
                    {
                       ok = 1;
                       if (!e_theme_edje_object_set(cw->shobj,
                                                    "base/theme/borders",
                                                    "e/shadow/box"))
                         ok = 0;
                   }
                  if (!ok)
                    {
                       snprintf(buf, sizeof(buf), "%s/shadow.edj", 
                                e_module_dir_get(_comp_mod->module)
                                );
                       edje_object_file_set(cw->shobj, buf, "shadow");
                    }
               }
          }
     }
   if (cw->dmg_updates > 0)
     {
        evas_object_show(cw->obj);
        if (cw->shobj) evas_object_show(cw->shobj);
     }
   _e_mod_comp_win_render_queue(cw);
   _e_mod_comp_win_show_effects_add(cw);
}

static void
_e_mod_comp_win_hide(E_Comp_Win *cw)
{
   if (!cw->defer_hide)
     {
        if (!cw->visible) return;
     }
   cw->defer_hide = 0;
   cw->visible = 0;
   if ((cw->input_only) || (cw->invalid)) return;
   DBG("  [0x%x] hid --\n", cw->win);
   _e_mod_comp_win_hide_effects_add(cw);
   if (cw->effects)
     {
        cw->defer_hide = 1;
        return; // defer rest of hide
     }
   if (cw->update_timeout)
     {
        ecore_timer_del(cw->update_timeout);
        cw->update_timeout = NULL;
     }
   evas_object_hide(cw->obj);
   if (cw->shobj) evas_object_hide(cw->shobj);

   if (_comp_mod->conf->keep_unmapped)
     {
        // fixme: ask the x homies. why does this need to be done?
        // why free pixmap (+texture bound), undriect, then redirect again
        // getting pixmap againand well - getting texture too again. why?
        if (cw->redirected)
          {
             if (cw->pixmap) ecore_x_pixmap_free(cw->pixmap);
             evas_object_image_size_set(cw->obj, 1, 1);
             cw->pixmap = 0;
             cw->pw = 0;
             cw->ph = 0;
             cw->native = 0;
             cw->pixmap = ecore_x_composite_name_window_pixmap_get(cw->win);
             if (cw->pixmap)
               ecore_x_pixmap_geometry_get(cw->pixmap, NULL, NULL, &(cw->pw), &(cw->ph));
             else
               {
                  cw->pw = 0;
                  cw->ph = 0;
               }
             if ((cw->pw <= 0) || (cw->ph <= 0))
               {
                  ecore_x_pixmap_free(cw->pixmap);
                  cw->pixmap = 0;
               }
             if ((cw->c->gl) && (_comp_mod->conf->texture_from_pixmap) &&
                 (!cw->shaped) && (!cw->rects)/* && (!cw->shape_changed)*/)
//             if ((cw->c->gl) && (_comp_mod->conf->texture_from_pixmap) &&
//                 (!cw->shaped) && (!cw->shape_changed) && (cw->pixmap))
               {
                  Evas_Native_Surface ns;
                  
                  ns.data.x11.visual = cw->vis;
                  ns.data.x11.pixmap = cw->pixmap;
                  evas_object_image_native_surface_set(cw->obj, &ns);
                  DBG("NATIVE SHOW1 [0x%x] %x %ix%i\n", cw->win, cw->pixmap, cw->pw, cw->ph);
                  cw->native = 1;
               }
             cw->dmg_updates = 0;
          }
        return;
     }
   
   if (cw->native)
     {
        evas_object_image_native_surface_set(cw->obj, NULL);
        cw->native = 0;
     }
   if (cw->pixmap)
     {
        ecore_x_pixmap_free(cw->pixmap);
        cw->pixmap = 0;
        cw->pw = 0;
        cw->ph = 0;
     }
   if (cw->xim)
     {
        evas_object_image_size_set(cw->obj, 1, 1);
        evas_object_image_data_set(cw->obj, NULL);
        ecore_x_image_free(cw->xim);
        cw->xim = NULL;
     }
   if (cw->redirected)
     {
        ecore_x_composite_unredirect_window(cw->win, ECORE_X_COMPOSITE_UPDATE_MANUAL);
        cw->redirected = 0;
        cw->pw = 0;
        cw->ph = 0;
     }
   _e_mod_comp_win_render_queue(cw);
}

static void
_e_mod_comp_win_raise_above(E_Comp_Win *cw, E_Comp_Win *cw2)
{
   DBG("  [0x%x] abv [0x%x]\n", cw->win, cw2->win);
   cw->c->wins = eina_inlist_remove(cw->c->wins, EINA_INLIST_GET(cw));
   cw->c->wins = eina_inlist_append_relative(cw->c->wins, 
                                             EINA_INLIST_GET(cw), 
                                             EINA_INLIST_GET(cw2));
   evas_object_stack_above(cw->obj, cw2->obj);
   if (cw->shobj)
     {
        evas_object_stack_below(cw->shobj, cw->obj);
     }
   _e_mod_comp_win_render_queue(cw);
}

static void
_e_mod_comp_win_raise(E_Comp_Win *cw)
{
   DBG("  [0x%x] rai\n", cw->win);
   cw->c->wins = eina_inlist_remove(cw->c->wins, EINA_INLIST_GET(cw));
   cw->c->wins = eina_inlist_append(cw->c->wins, EINA_INLIST_GET(cw));
   evas_object_raise(cw->obj);
   if (cw->shobj)
     {
        evas_object_stack_below(cw->shobj, cw->obj);
     }
   _e_mod_comp_win_render_queue(cw);
}

static void
_e_mod_comp_win_lower(E_Comp_Win *cw)
{
   DBG("  [0x%x] low\n", cw->win);
   cw->c->wins = eina_inlist_remove(cw->c->wins, EINA_INLIST_GET(cw));
   cw->c->wins = eina_inlist_prepend(cw->c->wins, EINA_INLIST_GET(cw));
   evas_object_lower(cw->obj);
   if (cw->shobj)
     {
        evas_object_stack_below(cw->shobj, cw->obj);
     }
   _e_mod_comp_win_render_queue(cw);
}

static void
_e_mod_comp_win_configure(E_Comp_Win *cw, int x, int y, int w, int h, int border)
{
   Eina_Bool moved = 0;
   
   if (!((x == cw->x) && (y == cw->y)))
     {
        DBG("  [0x%x] mov %4i %4i\n", cw->win, x, y);
        cw->x = x;
        cw->y = y;
        evas_object_move(cw->obj, cw->x, cw->y);
        if (cw->shobj)
          {
             evas_object_move(cw->shobj, cw->x, cw->y);
          }
        moved = 1;
     }
   if (!((w == cw->w) && (h == cw->h)))
     {
        DBG("  [0x%x] rsz %4ix%4i\n", cw->win, w, h);
        if (cw->pixmap)
          {
             DBG("  [0x%x] free pm %x\n", cw->win, cw->pixmap);
             ecore_x_pixmap_free(cw->pixmap);
             cw->pixmap = 0;
             cw->native = 0;
             cw->pw = 0;
             cw->ph = 0;
          }
        cw->w = w;
        cw->h = h;
        evas_object_resize(cw->obj, 
                           cw->w + (cw->border * 2), 
                           cw->h + (cw->border * 2));
        if (cw->shobj)
          {
             evas_object_resize(cw->shobj, 
                                cw->w + (cw->border * 2),
                                cw->h + (cw->border * 2));
          }
        if (cw->xim)
          {
             evas_object_image_data_set(cw->obj, NULL);
             ecore_x_image_free(cw->xim);
             cw->xim = NULL;
          }
        _e_mod_comp_win_damage(cw, 0, 0, cw->w, cw->h, 0);
     }
   if (cw->border != border)
     {
        cw->border = border;
        evas_object_resize(cw->obj, 
                           cw->w + (cw->border * 2), 
                           cw->h + (cw->border * 2));
        if (cw->shobj)
          {
             evas_object_resize(cw->shobj, 
                                cw->w + (cw->border * 2),
                                cw->h + (cw->border * 2));
          }
     }
   if ((cw->input_only) || (cw->invalid)) return;
   _e_mod_comp_win_render_queue(cw);
   if (moved) _e_mod_comp_win_move_effects_add(cw);
}

static void
_e_mod_comp_win_damage(E_Comp_Win *cw, int x, int y, int w, int h, Eina_Bool dmg)
{
   if ((cw->input_only) || (cw->invalid)) return;
   DBG("  [0x%x] dmg [%x] %4i %4i %4ix%4i\n", cw->win, cw->damage, x, y, w, h);
   if ((dmg) && (cw->damage))
     {
        Ecore_X_Region parts;
   
        parts = ecore_x_region_new(NULL, 0);
        ecore_x_damage_subtract(cw->damage, 0, parts);
        ecore_x_region_free(parts);
        cw->dmg_updates++;
     }
   e_mod_comp_update_add(cw->up, x, y, w, h);
   if (dmg)
     {
        if (cw->counter)
          {
             if (!cw->update_timeout)
               cw->update_timeout = ecore_timer_add
               (ecore_animator_frametime_get() * 2, 
                _e_mod_comp_win_damage_timeout, cw);
             return;
          }
     }
   if (!cw->update)
     {
        cw->update = 1;
        cw->c->updates = eina_list_append(cw->c->updates, cw);
     }
   _e_mod_comp_win_render_queue(cw);
}

static void
_e_mod_comp_win_reshape(E_Comp_Win *cw)
{
   if (cw->shape_changed) return;
   cw->shape_changed = 1;
   if (!cw->update)
     {
        cw->update = 1;
        cw->c->updates = eina_list_append(cw->c->updates, cw);
     }
   e_mod_comp_update_add(cw->up, 0, 0, cw->w, cw->h);
   _e_mod_comp_win_render_queue(cw);
}

//////////////////////////////////////////////////////////////////////////

static int
_e_mod_comp_create(void *data, int type, void *event)
{
   Ecore_X_Event_Window_Create *ev = event;
   E_Comp_Win *cw;
   E_Comp *c = _e_mod_comp_find(ev->parent);
   if (!c) return 1;
   if (_e_mod_comp_win_find(ev->win)) return 1;
   if (c->win == ev->win) return 1;
   if (c->ee_win == ev->win) return 1;
   cw = _e_mod_comp_win_add(c, ev->win);
   if (cw) _e_mod_comp_win_configure(cw, ev->x, ev->y, ev->w, ev->h, ev->border);
   return 1;
}

static int
_e_mod_comp_destroy(void *data, int type, void *event)
{
   Ecore_X_Event_Window_Destroy *ev = event;
   E_Comp_Win *cw = _e_mod_comp_win_find(ev->win);
   if (!cw) return 1;
   if (cw->effects) cw->delete_me = 1;
   else _e_mod_comp_win_del(cw);
   return 1;
}

static int
_e_mod_comp_show(void *data, int type, void *event)
{
   Ecore_X_Event_Window_Show *ev = event;
   E_Comp_Win *cw = _e_mod_comp_win_find(ev->win);
   if (!cw) return 1;
   if (cw->visible) return 1;
   _e_mod_comp_win_show(cw);
   return 1;
}

static int
_e_mod_comp_hide(void *data, int type, void *event)
{
   Ecore_X_Event_Window_Hide *ev = event;
   E_Comp_Win *cw = _e_mod_comp_win_find(ev->win);
   if (!cw) return 1;
   if (!cw->visible) return 1;
   _e_mod_comp_win_hide(cw);
   return 1;
}

static int
_e_mod_comp_reparent(void *data, int type, void *event)
{
   Ecore_X_Event_Window_Reparent *ev = event;
   E_Comp_Win *cw = _e_mod_comp_win_find(ev->win);
   if (!cw) return 1;
   DBG("== repar [0x%x] to [0x%x]\n", ev->win, ev->parent);
   if (ev->parent != cw->c->man->root)
     _e_mod_comp_win_del(cw);
   return 1;
}

static int
_e_mod_comp_configure(void *data, int type, void *event)
{
   Ecore_X_Event_Window_Configure *ev = event;
   E_Comp_Win *cw = _e_mod_comp_win_find(ev->win);
   if (!cw) return 1;

   if (ev->abovewin == 0)
     {
        if (EINA_INLIST_GET(cw)->prev) _e_mod_comp_win_lower(cw);
     }
   else
     {
        E_Comp_Win *cw2 = _e_mod_comp_win_find(ev->abovewin);

        if (cw2)
          {
             E_Comp_Win *cw3 = (E_Comp_Win *)(EINA_INLIST_GET(cw)->prev);

             if (cw3 != cw2)
               _e_mod_comp_win_raise_above(cw, cw2);
          }
     }
   
   if (!((cw->x == ev->x) && (cw->y == ev->y) &&
         (cw->w == ev->h) && (cw->h == ev->h) &&
         (cw->border == ev->border)))
     {
        _e_mod_comp_win_configure(cw, ev->x, ev->y, ev->w, ev->h, ev->border);
     }
   return 1;
}

static int
_e_mod_comp_stack(void *data, int type, void *event)
{
   Ecore_X_Event_Window_Stack *ev = event;
   E_Comp_Win *cw = _e_mod_comp_win_find(ev->win);
   if (!cw) return 1;
   if (ev->detail == ECORE_X_WINDOW_STACK_ABOVE) _e_mod_comp_win_raise(cw);
   else _e_mod_comp_win_lower(cw);
   return 1;
}

static int
_e_mod_comp_property(void *data, int type, void *event)
{
   Ecore_X_Event_Window_Property *ev = event;
   return 1;
}

static int
_e_mod_comp_message(void *data, int type, void *event)
{
   Ecore_X_Event_Client_Message *ev = event;
   E_Comp_Win *cw = NULL;
   if ((ev->message_type != ECORE_X_ATOM_E_COMP_SYNC_DRAW_DONE) ||
       (ev->format != 32)) return 1;
   cw = _e_mod_comp_border_client_find(ev->data.l[0]);
   if (!cw) return 1;
   if (!cw->bd) return 1;
   if (ev->data.l[0] != cw->bd->client.win) return 1;
   if (cw->bd)
     {
        if (cw->counter)
          {
             if (!cw->update)
               {
                  if (cw->update_timeout)
                    {
                       ecore_timer_del(cw->update_timeout);
                       cw->update_timeout = NULL;
                    }
                  cw->update = 1;
                  cw->c->updates = eina_list_append(cw->c->updates, cw);
               }
             cw->drawme = 1;
             _e_mod_comp_win_render_queue(cw);
          }
     }
   return 1;
}

static int
_e_mod_comp_shape(void *data, int type, void *event)
{
   Ecore_X_Event_Window_Shape *ev = event;
   E_Comp_Win *cw = _e_mod_comp_win_find(ev->win);
   if (!cw) return 1;
   _e_mod_comp_win_reshape(cw);
   return 1;
}

static int
_e_mod_comp_damage(void *data, int type, void *event)
{
   Ecore_X_Event_Damage *ev = event;
   E_Comp_Win *cw = _e_mod_comp_win_damage_find(ev->damage);
   if (!cw) return 1;
   _e_mod_comp_win_damage(cw, 
                          ev->area.x, ev->area.y, 
                          ev->area.width, ev->area.height, 1); 
   return 1;
}

static int
_e_mod_comp_randr(void *data, int type, void *event)
{
   Eina_List *l;
   E_Comp *c;

   EINA_LIST_FOREACH(compositors, l, c)
     {
        ecore_evas_resize(c->ee, c->man->w, c->man->h);
     }
   return 1;
}

static int
_e_mod_comp_bd_add(void *data, int type, void *event)
{
   E_Event_Border_Add *ev = event;
   E_Comp_Win *cw = _e_mod_comp_win_find(ev->border->win);
   if (!cw) return 1;
   // fimxe: add/enable compositing here not in show event for borders
   return 1;
}

static int
_e_mod_comp_bd_del(void *data, int type, void *event)
{
   E_Event_Border_Remove *ev = event;
   E_Comp_Win *cw = _e_mod_comp_win_find(ev->border->win);
   if (!cw) return 1;
   if (cw->bd == ev->border) _e_mod_comp_object_del(cw, ev->border);
   return 1;
}

static int
_e_mod_comp_bd_show(void *data, int type, void *event)
{
   E_Event_Border_Show *ev = event;
   E_Comp_Win *cw = _e_mod_comp_win_find(ev->border->win);
   if (!cw) return 1;
   // fimxe: show compwin here
   return 1;
}

static int
_e_mod_comp_bd_hide(void *data, int type, void *event)
{
   E_Event_Border_Hide *ev = event;
   E_Comp_Win *cw = _e_mod_comp_win_find(ev->border->win);
   if (!cw) return 1;
   // fimxe: hide compwin here
   return 1;
}

static int
_e_mod_comp_bd_move(void *data, int type, void *event)
{
   E_Event_Border_Move *ev = event;
   E_Comp_Win *cw = _e_mod_comp_win_find(ev->border->win);
   if (!cw) return 1;
   // fimxe: do move here for composited bd
   return 1;
}

static int
_e_mod_comp_bd_resize(void *data, int type, void *event)
{
   E_Event_Border_Resize *ev = event;
   E_Comp_Win *cw = _e_mod_comp_win_find(ev->border->win);
   if (!cw) return 1;
   // fimxe: do resize here instead of conf notify
   return 1;
}

static int
_e_mod_comp_bd_iconify(void *data, int type, void *event)
{
   E_Event_Border_Iconify *ev = event;
   E_Comp_Win *cw = _e_mod_comp_win_find(ev->border->win);
   if (!cw) return 1;
   // fimxe: special iconfiy anim
   return 1;
}

static int
_e_mod_comp_bd_uniconify(void *data, int type, void *event)
{
   E_Event_Border_Uniconify *ev = event;
   E_Comp_Win *cw = _e_mod_comp_win_find(ev->border->win);
   if (!cw) return 1;
   // fimxe: special uniconfiy anim
   return 1;
}

static int
_e_mod_comp_bd_urgent_change(void *data, int type, void *event)
{
   E_Event_Border_Urgent_Change *ev = event;
   E_Comp_Win *cw = _e_mod_comp_win_find(ev->border->win);
   if (!cw) return 1;
   // fimxe: do special anim to show urgency
   return 1;
}

static int
_e_mod_comp_bd_focus_in(void *data, int type, void *event)
{
   E_Event_Border_Focus_In *ev = event;
   E_Comp_Win *cw = _e_mod_comp_win_find(ev->border->win);
   if (!cw) return 1;
   // fimxe: can do anim on focus in
   return 1;
}

static int
_e_mod_comp_bd_focus_out(void *data, int type, void *event)
{
   E_Event_Border_Focus_Out *ev = event;
   E_Comp_Win *cw = _e_mod_comp_win_find(ev->border->win);
   if (!cw) return 1;
   // fimxe: can do anim on focus out
   return 1;
}

static int
_e_mod_comp_bd_property(void *data, int type, void *event)
{
   E_Event_Border_Property *ev = event;
   E_Comp_Win *cw = _e_mod_comp_win_find(ev->border->win);
   if (!cw) return 1;
   // fimxe: check properties and adjust things if needed...
   return 1;
}

//////////////////////////////////////////////////////////////////////////

static E_Comp *
_e_mod_comp_add(E_Manager *man)
{
   E_Comp *c;
   Ecore_X_Window *wins;
   Ecore_X_Window_Attributes att;
   int i, num;
   
   c = calloc(1, sizeof(E_Comp));
   if (!c) return NULL;

   if (_comp_mod->conf->vsync)
     {
        e_util_env_set("__GL_SYNC_TO_VBLANK", "1");
     }
   else
     {
        e_util_env_set("__GL_SYNC_TO_VBLANK", NULL);
     }
   
   ecore_x_e_comp_sync_supported_set(man->root, _comp_mod->conf->efl_sync);
   
   c->man = man;
   c->win = ecore_x_composite_render_window_enable(man->root);
   if (!c->win)
     {
        e_util_dialog_internal
          (_("Compositor Error"),
           _("Your screen does not support the compositor<br>"
             "overlay window. This is needed for it to<br>"
             "function."));
        free(c);
        return NULL;
     }
   
   memset((&att), 0, sizeof(Ecore_X_Window_Attributes));
   ecore_x_window_attributes_get(c->win, &att);   
   
   if ((att.depth != 24) && (att.depth != 32))
     {
        e_util_dialog_internal
          (_("Compositor Error"),
           _("Your screen is not in 24/32bit display mode.<br>"
             "This is required to be your default depth<br>"
             "setting for the compositor to work properly."));
        ecore_x_composite_render_window_disable(c->win);
        free(c);
        return NULL;
     }
   
   if (c->man->num == 0) e_alert_composite_win = c->win;
   
   if (_comp_mod->conf->engine == E_EVAS_ENGINE_GL_X11)
     {
        if (_comp_mod->conf->indirect)
          {
             const int opt[] = 
               { ECORE_EVAS_GL_X11_OPT_INDIRECT, 1,
                    ECORE_EVAS_GL_X11_OPT_NONE 
               };
             c->ee = ecore_evas_gl_x11_options_new
               (NULL, c->win, 0, 0, man->w, man->h, opt);
          }
        if (!c->ee)
          c->ee = ecore_evas_gl_x11_new(NULL, c->win, 0, 0, man->w, man->h);
        if (c->ee)
          {
             c->gl = 1;
             ecore_evas_gl_x11_pre_post_swap_callback_set
               (c->ee, c, _e_mod_comp_pre_swap, NULL);
          }
     }
   if (!c->ee)
     c->ee = ecore_evas_software_x11_new(NULL, c->win, 0, 0, man->w, man->h);
   
   ecore_evas_comp_sync_set(c->ee, 0);
   ecore_evas_manual_render_set(c->ee, _comp_mod->conf->lock_fps);
   c->evas = ecore_evas_get(c->ee);
   ecore_evas_show(c->ee);
   
   c->ee_win = ecore_evas_software_x11_window_get(c->ee);
   ecore_x_screen_is_composited_set(c->man->num, c->ee_win);
   ecore_x_composite_unredirect_subwindows
     (c->man->root, ECORE_X_COMPOSITE_UPDATE_MANUAL);

   wins = ecore_x_window_children_get(c->man->root, &num);
   if (wins)
     {
        for (i = 0; i < num; i++)
          {
             E_Comp_Win *cw;
             int x, y, w, h, border;
             
             cw = _e_mod_comp_win_add(c, wins[i]);
             if (!cw) continue;
             ecore_x_window_geometry_get(cw->win, &x, &y, &w, &h);
             border = ecore_x_window_border_width_get(cw->win);
             if (wins[i] == c->win) continue;
             _e_mod_comp_win_configure(cw, x, y, w, h, border);
             if (ecore_x_window_visible_get(wins[i]))
               {
                  _e_mod_comp_win_show(cw);
               }
          }
        free(wins);
     }
   
   return c;
}

static void
_e_mod_comp_del(E_Comp *c)
{
   E_Comp_Win *cw;
   
   if (c->grabbed)
     {
        c->grabbed = 0;
        ecore_x_ungrab();
     }
   ecore_x_screen_is_composited_set(c->man->num, 0);
   while (c->wins)
     {
        cw = (E_Comp_Win *)(c->wins);
        if (cw->counter)
          {
             ecore_x_sync_counter_free(cw->counter);
             cw->counter = 0;
          }
        _e_mod_comp_win_hide(cw);
        _e_mod_comp_win_del(cw);
     }
   ecore_evas_free(c->ee);
   ecore_x_composite_unredirect_subwindows
     (c->man->root, ECORE_X_COMPOSITE_UPDATE_MANUAL);
   ecore_x_composite_render_window_disable(c->win);
   if (c->man->num == 0) e_alert_composite_win = 0;
   if (c->render_animator) ecore_animator_del(c->render_animator);
   if (c->update_job) ecore_job_del(c->update_job);
   
   ecore_x_e_comp_sync_supported_set(c->man->root, 0);
   
   free(c);
}

//////////////////////////////////////////////////////////////////////////

Eina_Bool
e_mod_comp_init(void)
{
   Eina_List *l;
   E_Manager *man;

   windows = eina_hash_string_superfast_new(NULL);
   borders = eina_hash_string_superfast_new(NULL);
   damages = eina_hash_string_superfast_new(NULL);
   
   handlers = eina_list_append(handlers, ecore_event_handler_add(ECORE_X_EVENT_WINDOW_CREATE,    _e_mod_comp_create,           NULL));
   handlers = eina_list_append(handlers, ecore_event_handler_add(ECORE_X_EVENT_WINDOW_DESTROY,   _e_mod_comp_destroy,          NULL));
   handlers = eina_list_append(handlers, ecore_event_handler_add(ECORE_X_EVENT_WINDOW_SHOW,      _e_mod_comp_show,             NULL));
   handlers = eina_list_append(handlers, ecore_event_handler_add(ECORE_X_EVENT_WINDOW_HIDE,      _e_mod_comp_hide,             NULL));
   handlers = eina_list_append(handlers, ecore_event_handler_add(ECORE_X_EVENT_WINDOW_REPARENT,  _e_mod_comp_reparent,         NULL));
   handlers = eina_list_append(handlers, ecore_event_handler_add(ECORE_X_EVENT_WINDOW_CONFIGURE, _e_mod_comp_configure,        NULL));
   handlers = eina_list_append(handlers, ecore_event_handler_add(ECORE_X_EVENT_WINDOW_STACK,     _e_mod_comp_stack,            NULL));
   handlers = eina_list_append(handlers, ecore_event_handler_add(ECORE_X_EVENT_WINDOW_PROPERTY,  _e_mod_comp_property,         NULL));
   handlers = eina_list_append(handlers, ecore_event_handler_add(ECORE_X_EVENT_CLIENT_MESSAGE,   _e_mod_comp_message,          NULL));
   handlers = eina_list_append(handlers, ecore_event_handler_add(ECORE_X_EVENT_WINDOW_SHAPE,     _e_mod_comp_shape,            NULL));
   handlers = eina_list_append(handlers, ecore_event_handler_add(ECORE_X_EVENT_DAMAGE_NOTIFY,    _e_mod_comp_damage,           NULL));

   handlers = eina_list_append(handlers, ecore_event_handler_add(E_EVENT_CONTAINER_RESIZE,       _e_mod_comp_randr,            NULL));
   
   handlers = eina_list_append(handlers, ecore_event_handler_add(E_EVENT_BORDER_ADD,             _e_mod_comp_bd_add,           NULL));
   handlers = eina_list_append(handlers, ecore_event_handler_add(E_EVENT_BORDER_REMOVE,          _e_mod_comp_bd_del,           NULL));
   handlers = eina_list_append(handlers, ecore_event_handler_add(E_EVENT_BORDER_SHOW,            _e_mod_comp_bd_show,          NULL));
   handlers = eina_list_append(handlers, ecore_event_handler_add(E_EVENT_BORDER_HIDE,            _e_mod_comp_bd_hide,          NULL));
   handlers = eina_list_append(handlers, ecore_event_handler_add(E_EVENT_BORDER_MOVE,            _e_mod_comp_bd_move,          NULL));
   handlers = eina_list_append(handlers, ecore_event_handler_add(E_EVENT_BORDER_RESIZE,          _e_mod_comp_bd_resize,        NULL));
   handlers = eina_list_append(handlers, ecore_event_handler_add(E_EVENT_BORDER_ICONIFY,         _e_mod_comp_bd_iconify,       NULL));
   handlers = eina_list_append(handlers, ecore_event_handler_add(E_EVENT_BORDER_UNICONIFY,       _e_mod_comp_bd_uniconify,     NULL));
   handlers = eina_list_append(handlers, ecore_event_handler_add(E_EVENT_BORDER_URGENT_CHANGE,   _e_mod_comp_bd_urgent_change, NULL));
   handlers = eina_list_append(handlers, ecore_event_handler_add(E_EVENT_BORDER_FOCUS_IN,        _e_mod_comp_bd_focus_in,      NULL));
   handlers = eina_list_append(handlers, ecore_event_handler_add(E_EVENT_BORDER_FOCUS_OUT,       _e_mod_comp_bd_focus_out,     NULL));
   handlers = eina_list_append(handlers, ecore_event_handler_add(E_EVENT_BORDER_PROPERTY,        _e_mod_comp_bd_property,      NULL));

   EINA_LIST_FOREACH(e_manager_list(), l, man)
     {
        E_Comp *c;
        
        c = _e_mod_comp_add(man);
        if (c) compositors = eina_list_append(compositors, c);
     }
   
   return 1;
}

void
e_mod_comp_shutdown(void)
{
   E_Comp *c;
   
   EINA_LIST_FREE(compositors, c) _e_mod_comp_del(c);
   
   E_FREE_LIST(handlers, ecore_event_handler_del);
   
   eina_hash_free(damages);
   eina_hash_free(windows);
   eina_hash_free(borders);
}

void
e_mod_comp_shadow_set(void)
{
   Eina_List *l;
   E_Comp *c;

   EINA_LIST_FOREACH(compositors, l, c)
     {
        E_Comp_Win *cw;
        
        ecore_evas_manual_render_set(c->ee, _comp_mod->conf->lock_fps);
        EINA_INLIST_FOREACH(c->wins, cw)
          {
             if (evas_object_visible_get(cw->obj) && (cw->shobj))
               {
                  if (_comp_mod->conf->use_shadow)
                    {
                       if (_e_mod_comp_win_do_shadow(cw))
                         {
                            int ok = 0;
                            char buf[PATH_MAX];
                            
                            if (_comp_mod->conf->shadow_file)
                              {
                                 ok = 1;
                                 if (!edje_object_file_set(cw->shobj, 
                                                           _comp_mod->conf->shadow_file,
                                                           "shadow"))
                                   ok = 0;
                              }
                            if (!ok)
                              {
                                 ok = 1;
                                 if (!e_theme_edje_object_set(cw->shobj,
                                                              "base/theme/borders",
                                                              "e/shadow/box"))
                                   ok = 0;
                              }
                            if (!ok)
                              {
                                 snprintf(buf, sizeof(buf), "%s/shadow.edj", 
                                          e_module_dir_get(_comp_mod->module)
                                          );
                                 edje_object_file_set(cw->shobj, buf, "shadow");
                              }
                            evas_object_show(cw->shobj);
                         }
                    }
                  else
                    evas_object_hide(cw->shobj);
               }
          }
     }
}

E_Comp *
e_mod_comp_manager_get(E_Manager *man)
{
   return _e_mod_comp_find(man->root);
}

E_Comp_Win *
e_mod_comp_win_find_by_window(E_Comp *c, Ecore_X_Window win)
{
   E_Comp_Win *cw;
   
   EINA_INLIST_FOREACH(c->wins, cw)
     {
        if (cw->win == win) return cw;
     }
   return NULL;
}

E_Comp_Win *
e_mod_comp_win_find_by_border(E_Comp *c, E_Border *bd)
{
   E_Comp_Win *cw;
   
   EINA_INLIST_FOREACH(c->wins, cw)
     {
        if (cw->bd == bd) return cw;
     }
   return NULL;
}

E_Comp_Win *
e_mod_comp_win_find_by_popup(E_Comp *c, E_Popup *pop)
{
   E_Comp_Win *cw;
   
   EINA_INLIST_FOREACH(c->wins, cw)
     {
        if (cw->pop == pop) return cw;
     }
   return NULL;
}

E_Comp_Win *
e_mod_comp_win_find_by_menu(E_Comp *c, E_Menu *menu)
{
   E_Comp_Win *cw;
   
   EINA_INLIST_FOREACH(c->wins, cw)
     {
        if (cw->menu == menu) return cw;
     }
   return NULL;
}

Evas_Object *
e_mod_comp_win_evas_object_get(E_Comp_Win *cw)
{
   return cw->obj;
}

Evas_Object *
e_mod_comp_win_mirror_object_add(Evas *e, E_Comp_Win *cw)
{
}
