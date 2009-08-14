#include <Elementary.h>
#include "elm_priv.h"

#define SMART_NAME "elm_scroller"
#define API_ENTRY Smart_Data *sd; sd = evas_object_smart_data_get(obj); if ((!obj) || (!sd) || (evas_object_type_get(obj) && strcmp(evas_object_type_get(obj), SMART_NAME)))
#define INTERNAL_ENTRY Smart_Data *sd; sd = evas_object_smart_data_get(obj); if (!sd) return;
typedef struct _Smart_Data Smart_Data;

struct _Smart_Data
{ 
   Evas_Coord   x, y, w, h;
   
   Evas_Object *smart_obj;
   Evas_Object *child_obj;
   Evas_Object *pan_obj;
   Evas_Object *edje_obj;
   Evas_Object *event_obj;

   Elm_Smart_Scroller_Policy hbar_flags, vbar_flags;

   struct {
      Evas_Coord x, y;
      Evas_Coord sx, sy;
      Evas_Coord dx, dy;
      Evas_Coord bx, by;
      Evas_Coord ax, ay;
      Evas_Coord bx0, by0;
      Evas_Coord b0x, b0y;
      Evas_Coord b2x, b2y;
      struct {
	 Evas_Coord    x, y;
	 double        timestamp;
      } history[20];
      double anim_start;
      double anim_start2;
      double anim_start3;
      double onhold_vx, onhold_vy, onhold_tlast, onhold_vxe, onhold_vye;
      Evas_Coord hold_x, hold_y;
      Ecore_Animator *hold_animator;
      Ecore_Animator *onhold_animator;
      Ecore_Animator *momentum_animator;
      Ecore_Animator *bounce_x_animator;
      Ecore_Animator *bounce_y_animator;
      Evas_Coord locked_x, locked_y;
      unsigned char now : 1;
      unsigned char dragged : 1;
      unsigned char dir_x : 1;
      unsigned char dir_y : 1;
      unsigned char dir_none : 1;
      unsigned char locked : 1;
      unsigned char bounce_x_hold : 1;
      unsigned char bounce_y_hold : 1;
   } down;
   
   struct {
      Evas_Coord w, h;
   } child;
   struct {
      Evas_Coord x, y;
   } step, page;

   struct {
      void (*set) (Evas_Object *obj, Evas_Coord x, Evas_Coord y);
      void (*get) (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y);
      void (*max_get) (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y);
      void (*child_size_get) (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y);
   } pan_func;
   
   
   unsigned char hbar_visible : 1;
   unsigned char vbar_visible : 1;
   unsigned char extern_pan : 1;
   unsigned char one_dir_at_a_time : 1;
   unsigned char hold : 1;
   unsigned char freeze : 1;
   unsigned char bouncemex : 1;
   unsigned char bouncemey : 1;
   unsigned char bounce_horiz : 1;
   unsigned char bounce_vert : 1;
}; 

/* local subsystem functions */
static void _smart_child_del_hook(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _smart_pan_changed_hook(void *data, Evas_Object *obj, void *event_info);
static void _smart_pan_pan_changed_hook(void *data, Evas_Object *obj, void *event_info);
static void _smart_event_wheel(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _smart_event_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info);
static int  _smart_hold_animator(void *data);
static int  _smart_momentum_animator(void *data);
static void _smart_event_mouse_up(void *data, Evas *e, Evas_Object *obj, void *event_info);
static int  _smart_onhold_animator(void *data);
static void _smart_event_mouse_move(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _smart_event_key_down(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _smart_edje_drag_v(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _smart_edje_drag_h(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _smart_scrollbar_read(Smart_Data *sd);
static void _smart_scrollbar_reset(Smart_Data *sd);
static int  _smart_scrollbar_bar_h_visibility_adjust(Smart_Data *sd);
static int  _smart_scrollbar_bar_v_visibility_adjust(Smart_Data *sd);
static void _smart_scrollbar_bar_visibility_adjust(Smart_Data *sd);
static void _smart_scrollbar_size_adjust(Smart_Data *sd);
static void _smart_reconfigure(Smart_Data *sd);
static void _smart_add(Evas_Object *obj);
static void _smart_del(Evas_Object *obj);
static void _smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y);
static void _smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h);
static void _smart_show(Evas_Object *obj);
static void _smart_hide(Evas_Object *obj);
static void _smart_color_set(Evas_Object *obj, int r, int g, int b, int a);
static void _smart_clip_set(Evas_Object *obj, Evas_Object * clip);
static void _smart_clip_unset(Evas_Object *obj);
static void _smart_init(void);

/* local subsystem globals */
static Evas_Smart *_smart = NULL;

/* externally accessible functions */
Evas_Object *
elm_smart_scroller_add(Evas *evas)
{
   _smart_init();
   return evas_object_smart_add(evas, _smart);
}

void
elm_smart_scroller_child_set(Evas_Object *obj, Evas_Object *child)
{
   Evas_Coord w, h;
   Evas_Object *o;
   
   API_ENTRY return;
   if (sd->child_obj)
     {
	_elm_smart_pan_child_set(sd->pan_obj, NULL);
	evas_object_event_callback_del(sd->child_obj, EVAS_CALLBACK_DEL, _smart_child_del_hook);
     }
   
   sd->child_obj = child;
   if (!child) return;

   if (!sd->pan_obj)
     {
	o = _elm_smart_pan_add(evas_object_evas_get(obj));
	sd->pan_obj = o;
	evas_object_smart_callback_add(o, "changed", _smart_pan_changed_hook, sd);
	evas_object_smart_callback_add(o, "pan_changed", _smart_pan_pan_changed_hook, sd);
	evas_object_show(o);
	edje_object_part_swallow(sd->edje_obj, "elm.swallow.content", o);
     }
   
   sd->pan_func.set = _elm_smart_pan_set;
   sd->pan_func.get = _elm_smart_pan_get;
   sd->pan_func.max_get = _elm_smart_pan_max_get;
   sd->pan_func.child_size_get = _elm_smart_pan_child_size_get;
   
   evas_object_event_callback_add(child, EVAS_CALLBACK_DEL, _smart_child_del_hook, sd);
   _elm_smart_pan_child_set(sd->pan_obj, sd->child_obj);
   sd->pan_func.child_size_get(sd->pan_obj, &w, &h);
   sd->child.w = w;
   sd->child.h = h;
   _smart_scrollbar_size_adjust(sd);
   _smart_scrollbar_reset(sd);
}

void
elm_smart_scroller_extern_pan_set(Evas_Object *obj, Evas_Object *pan,
			     void (*pan_set) (Evas_Object *obj, Evas_Coord x, Evas_Coord y),
			     void (*pan_get) (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y),
			     void (*pan_max_get) (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y),
			     void (*pan_child_size_get) (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y))
{
   API_ENTRY return;
   
   elm_smart_scroller_child_set(obj, NULL);
   if (sd->extern_pan)
     {
	if (sd->pan_obj)
	  {
	     edje_object_part_unswallow(sd->edje_obj, sd->pan_obj);
	     sd->pan_obj = NULL;
	  }
     }
   else
     {
	if (sd->pan_obj)
	  {
	     evas_object_del(sd->pan_obj);
	     sd->pan_obj = NULL;
	  }
     }
   if (!pan)
     {
	sd->extern_pan = 0;
	return;
     }

   sd->pan_obj = pan;
   sd->pan_func.set = pan_set;
   sd->pan_func.get = pan_get;
   sd->pan_func.max_get = pan_max_get;
   sd->pan_func.child_size_get = pan_child_size_get;
   sd->extern_pan = 1;
   evas_object_smart_callback_add(sd->pan_obj, "changed", _smart_pan_changed_hook, sd);
   evas_object_smart_callback_add(sd->pan_obj, "pan_changed", _smart_pan_pan_changed_hook, sd);
   edje_object_part_swallow(sd->edje_obj, "elm.swallow.content", sd->pan_obj);
   evas_object_show(sd->pan_obj);
}

void
elm_smart_scroller_custom_edje_file_set(Evas_Object *obj, char *file, char *group)
{
   API_ENTRY return;

   edje_object_file_set(sd->edje_obj, file, group);
   if (sd->pan_obj)
     edje_object_part_swallow(sd->edje_obj, "elm.swallow.content", sd->pan_obj);
   sd->vbar_visible = !sd->vbar_visible;
   sd->hbar_visible = !sd->hbar_visible;
   _smart_scrollbar_bar_visibility_adjust(sd);
   if (sd->hbar_flags == ELM_SMART_SCROLLER_POLICY_ON)
     edje_object_signal_emit(sd->edje_obj, "elm,action,show_always,hbar", "elm");
   else if (sd->hbar_flags == ELM_SMART_SCROLLER_POLICY_OFF)
     edje_object_signal_emit(sd->edje_obj, "elm,action,hide,hbar", "elm");
   else
     edje_object_signal_emit(sd->edje_obj, "elm,action,show_notalways,hbar", "elm");
   if (sd->vbar_flags == ELM_SMART_SCROLLER_POLICY_ON)
     edje_object_signal_emit(sd->edje_obj, "elm,action,show_always,vbar", "elm");
   else if (sd->vbar_flags == ELM_SMART_SCROLLER_POLICY_OFF)
     edje_object_signal_emit(sd->edje_obj, "elm,action,hide,vbar", "elm");
   else
     edje_object_signal_emit(sd->edje_obj, "elm,action,show_notalways,vbar", "elm");
}

static int
_smart_bounce_x_animator(void *data)
{
   Smart_Data *sd;
   Evas_Coord x, y, dx, dy/*, ox, oy*/;
   double t, p, dt;
   
   sd = data;
   t = ecore_loop_time_get();
   dt = t - sd->down.anim_start2;
   if (dt >= 0.0)
     {
	dt = dt / _elm_config->thumbscroll_bounce_friction;
	if (dt > 1.0) dt = 1.0;
	p = 1.0 - ((1.0 - dt) * (1.0 - dt) * (1.0 - dt));
        elm_smart_scroller_child_pos_get(sd->smart_obj, &x, &y);
        dx = sd->down.b2x - sd->down.bx;
	dx = (dx * p);
	x = sd->down.bx + dx;
        elm_smart_scroller_child_pos_set(sd->smart_obj, x, y);
        if (dt >= 1.0)
          {
             if (sd->down.momentum_animator)
               sd->down.bounce_x_hold = 1;
             sd->down.bounce_x_animator = NULL;
             sd->bouncemex = 0;
             return 0;
          }
     }
   return 1;
}

static int
_smart_bounce_y_animator(void *data)
{
   Smart_Data *sd;
   Evas_Coord x, y, dx, dy;
   double t, p, dt;
   
   sd = data;
   t = ecore_loop_time_get();
   dt = t - sd->down.anim_start3;
   if (dt >= 0.0)
     {
	dt = dt / _elm_config->thumbscroll_bounce_friction;
	if (dt > 1.0) dt = 1.0;
	p = 1.0 - ((1.0 - dt) * (1.0 - dt) * (1.0 - dt));
        elm_smart_scroller_child_pos_get(sd->smart_obj, &x, &y);
        dy = sd->down.b2y - sd->down.by;
	dy = (dy * p);
	y = sd->down.by + dy;
        elm_smart_scroller_child_pos_set(sd->smart_obj, x, y);
        if (dt >= 1.0)
          {
             if (sd->down.momentum_animator)
               sd->down.bounce_y_hold = 1;
             sd->down.bounce_y_animator = NULL;
             sd->bouncemey = 0;
             return 0;
          }
     }
   return 1;
}

static int
_smart_momentum_animator(void *data)
{
   Smart_Data *sd;
   double t, dt, p;
   Evas_Coord x, y, dx, dy, px, py;
   
   sd = data;
   t = ecore_loop_time_get();
   dt = t - sd->down.anim_start;
   if (dt >= 0.0)
     {
	dt = dt / _elm_config->thumbscroll_friction;
	if (dt > 1.0) dt = 1.0;
	p = 1.0 - ((1.0 - dt) * (1.0 - dt) * (1.0 - dt));
	dx = (sd->down.dx * _elm_config->thumbscroll_friction * p);
	dy = (sd->down.dy * _elm_config->thumbscroll_friction * p);
        sd->down.ax = dx;
        sd->down.ay = dy;
	x = sd->down.sx - dx;
	y = sd->down.sy - dy;
        elm_smart_scroller_child_pos_get(sd->smart_obj, &px, &py);
        if ((sd->down.bounce_x_animator) ||
            (sd->down.bounce_x_hold))
          {
             sd->down.bx = sd->down.bx0 - dx + sd->down.b0x;
             x = px;
          }
        if ((sd->down.bounce_y_animator) ||
            (sd->down.bounce_y_hold))
          {
             sd->down.by = sd->down.by0 - dy + sd->down.b0y;
             y = py;
          }
	elm_smart_scroller_child_pos_set(sd->smart_obj, x, y);
	if (dt >= 1.0)
	  {
	     sd->down.momentum_animator = NULL;
             sd->down.bounce_x_hold = 0;
             sd->down.bounce_y_hold = 0;
             sd->down.ax = 0;
             sd->down.ay = 0;
	     return 0;
	  }
     }
   return 1;
}

static void
bounce_eval(Smart_Data *sd)
{
   Evas_Coord mx, my, px, py, bx, by, b2x, b2y;
   
   if ((!sd->bouncemex) && (!sd->bouncemey)) return;
   if (sd->down.now) return; // down bounce while still held down
   if (sd->down.onhold_animator)
     {
        ecore_animator_del(sd->down.onhold_animator);
        sd->down.onhold_animator = NULL;
     }
   if (sd->down.hold_animator)
     {
        ecore_animator_del(sd->down.hold_animator);
        sd->down.hold_animator = NULL;
     }
   sd->pan_func.max_get(sd->pan_obj, &mx, &my);
   sd->pan_func.get(sd->pan_obj, &px, &py);
   bx = px;
   by = py;
   if (px < 0) px = 0;
   if (px > mx) px = mx;
   if (py < 0) py = 0;
   if (py > my) py = my;
   b2x = px;
   b2y = py;
   if (!sd->down.bounce_x_animator)
     {
        if (sd->bouncemex)
          {
             sd->down.bounce_x_animator = ecore_animator_add(_smart_bounce_x_animator, sd);
             sd->down.anim_start2 = ecore_loop_time_get();
             sd->down.bx = bx;
             sd->down.bx0 = bx;
             sd->down.b2x = b2x;
             if (sd->down.momentum_animator) sd->down.b0x = sd->down.ax;
             else sd->down.b0x = 0;
          }
     }
   if (!sd->down.bounce_y_animator)
     {
        if (sd->bouncemey)
          {
             sd->down.bounce_y_animator = ecore_animator_add(_smart_bounce_y_animator, sd);
             sd->down.anim_start3 = ecore_loop_time_get();
             sd->down.by = by;
             sd->down.by0 = by;
             sd->down.b2y = b2y;
             if (sd->down.momentum_animator) sd->down.b0y = sd->down.ay;
             else sd->down.b0y = 0;
          }
     }
}

void
elm_smart_scroller_child_pos_set(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   Evas_Coord mx = 0, my = 0, px, py;
   double vx, vy;
   
   API_ENTRY return;
   // FIXME: allow for bounce outside of range
   sd->pan_func.max_get(sd->pan_obj, &mx, &my);
   if (mx > 0) vx = (double)x / (double)mx;
   else vx = 0.0;
   if (vx < 0.0) vx = 0.0;
   else if (vx > 1.0) vx = 1.0;
   if (my > 0) vy = (double)y / (double)my;
   else vy = 0.0;
   if (vy < 0.0) vy = 0.0;
   else if (vy > 1.0) vy = 1.0;
   edje_object_part_drag_value_set(sd->edje_obj, "elm.dragable.vbar", 0.0, vy);
   edje_object_part_drag_value_set(sd->edje_obj, "elm.dragable.hbar", vx, 0.0);
   sd->pan_func.get(sd->pan_obj, &px, &py);
   if (!_elm_config->thumbscroll_bounce_enable)
     {
        if (x < 0) x = 0;
        if (x > mx) x = mx;
        if (y < 0) y = 0;
        if (y > my) y = my;
     }

   if (!sd->bounce_horiz)
     {
        if (x < 0) x = 0;
        if (x > mx) x = mx;
     }
   if (!sd->bounce_vert)
     {
        if (y < 0) y = 0;
        if (y > my) y = my;
     }
   
   sd->pan_func.set(sd->pan_obj, x, y);
   if ((px != x) || (py != y))
     edje_object_signal_emit(sd->edje_obj, "elm,action,scroll", "elm");
   if (!sd->down.bounce_x_animator)
     {
        if ((x < 0) || (x > mx))
          {
             sd->bouncemex = 1;
             bounce_eval(sd);
          }
     }
   if (!sd->down.bounce_y_animator)
     {
        if ((y < 0) || (y > my))
          {
             sd->bouncemey = 1;
             bounce_eval(sd);
          }
     }
   if ((x != px) || (y != py))
     {
        evas_object_smart_callback_call(obj, "scroll", NULL);
     }
   if ((x != px)/* && (!sd->bouncemex)*/)
     {
        if (x == 0)
          evas_object_smart_callback_call(obj, "edge,left", NULL);
        if (x == mx)
          evas_object_smart_callback_call(obj, "edge,right", NULL);
     }
   if ((y != py)/* && (!sd->bouncemey)*/)
     {
        if (y == 0)
          evas_object_smart_callback_call(obj, "edge,top", NULL);
        if (y == my)
          evas_object_smart_callback_call(obj, "edge,bottom", NULL);
     }
}

void
elm_smart_scroller_child_pos_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y)
{
   API_ENTRY return;
   sd->pan_func.get(sd->pan_obj, x, y);
}

void
elm_smart_scroller_child_region_show(Evas_Object *obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   Evas_Coord mx = 0, my = 0, cw = 0, ch = 0, px = 0, py = 0, nx, ny;

   API_ENTRY return;
   sd->pan_func.max_get(sd->pan_obj, &mx, &my);
   sd->pan_func.child_size_get(sd->pan_obj, &cw, &ch);
   sd->pan_func.get(sd->pan_obj, &px, &py);
   
   nx = px;
   if (x < px) nx = x;
   else if ((x + w) > (px + (cw - mx)))
     {
	nx = x + w - (cw - mx);
	if (nx > x) nx = x;
     }
   ny = py;
   if (y < py) ny = y;
   else if ((y + h) > (py + (ch - my))) 
     {
	ny = y + h - (ch - my);
	if (ny > y) ny = y;
     }
   if ((nx == px) && (ny == py)) return;
   if (sd->down.bounce_x_animator)
     {
        ecore_animator_del(sd->down.bounce_x_animator);
        sd->down.bounce_x_animator = NULL;
        sd->bouncemex = 0;
     }
   if (sd->down.bounce_y_animator)
     {
        ecore_animator_del(sd->down.bounce_y_animator);
        sd->down.bounce_y_animator = NULL;
        sd->bouncemey = 0;
     }
   elm_smart_scroller_child_pos_set(obj, nx, ny);
}

void
elm_smart_scroller_child_viewport_size_get(Evas_Object *obj, Evas_Coord *w, Evas_Coord *h)
{
   API_ENTRY return;
   edje_object_calc_force(sd->edje_obj);
   evas_object_geometry_get(sd->pan_obj, NULL, NULL, w, h);
}

void
elm_smart_scroller_step_size_set(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   API_ENTRY return;
   if (x < 1) x = 1;
   if (y < 1) y = 1;
   sd->step.x = x;
   sd->step.y = y;
   _smart_scrollbar_size_adjust(sd);
}

void
elm_smart_scroller_step_size_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y)
{
   API_ENTRY return;
   if (x) *x = sd->step.x;
   if (y) *y = sd->step.y;
}

void
elm_smart_scroller_page_size_set(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   API_ENTRY return;
   sd->page.x = x;
   sd->page.y = y;
   _smart_scrollbar_size_adjust(sd);
}

void
elm_smart_scroller_page_size_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y)
{
   API_ENTRY return;
   if (x) *x = sd->page.x;
   if (y) *y = sd->page.y;
}

void
elm_smart_scroller_policy_set(Evas_Object *obj, Elm_Smart_Scroller_Policy hbar, Elm_Smart_Scroller_Policy vbar)
{
   API_ENTRY return;
   if ((sd->hbar_flags == hbar) && (sd->vbar_flags == vbar)) return;
   sd->hbar_flags = hbar;
   sd->vbar_flags = vbar;
   if (sd->hbar_flags == ELM_SMART_SCROLLER_POLICY_ON)
     edje_object_signal_emit(sd->edje_obj, "elm,action,show_always,hbar", "elm");
   else if (sd->hbar_flags == ELM_SMART_SCROLLER_POLICY_OFF)
     edje_object_signal_emit(sd->edje_obj, "elm,action,hide,hbar", "elm");
   else
     edje_object_signal_emit(sd->edje_obj, "elm,action,show_notalways,hbar", "elm");
   if (sd->vbar_flags == ELM_SMART_SCROLLER_POLICY_ON)
     edje_object_signal_emit(sd->edje_obj, "elm,action,show_always,vbar", "elm");
   else if (sd->vbar_flags == ELM_SMART_SCROLLER_POLICY_OFF)
     edje_object_signal_emit(sd->edje_obj, "elm,action,hide,vbar", "elm");
   else
     edje_object_signal_emit(sd->edje_obj, "elm,action,show_notalways,vbar", "elm");
   _smart_scrollbar_size_adjust(sd);
}

void
elm_smart_scroller_policy_get(Evas_Object *obj, Elm_Smart_Scroller_Policy *hbar, Elm_Smart_Scroller_Policy *vbar)
{
   API_ENTRY return;
   if (hbar) *hbar = sd->hbar_flags;
   if (vbar) *vbar = sd->vbar_flags;
}

Evas_Object *
elm_smart_scroller_edje_object_get(Evas_Object *obj)
{
   API_ENTRY return NULL;
   return sd->edje_obj;
}

void
elm_smart_scroller_single_dir_set(Evas_Object *obj, Eina_Bool single_dir)
{
   API_ENTRY return;
   sd->one_dir_at_a_time = single_dir;
}

Eina_Bool
elm_smart_scroller_single_dir_get(Evas_Object *obj)
{
   API_ENTRY return EINA_FALSE;
   return sd->one_dir_at_a_time;
}

void
elm_smart_scroller_theme_set(Evas_Object *obj, const char *clas, const char *group, const char *style)
{
   API_ENTRY return;
   _elm_theme_set(sd->edje_obj, clas, group, style);
   if (sd->pan_obj)
     edje_object_part_swallow(sd->edje_obj, "elm.swallow.content", sd->pan_obj);
   sd->vbar_visible = !sd->vbar_visible;
   sd->hbar_visible = !sd->hbar_visible;
   _smart_scrollbar_bar_visibility_adjust(sd);
}

void
elm_smart_scroller_hold_set(Evas_Object *obj, Eina_Bool hold)
{
   API_ENTRY return;
   sd->hold = hold;
}

void
elm_smart_scroller_freeze_set(Evas_Object *obj, Eina_Bool freeze)
{
   API_ENTRY return;
   sd->freeze = freeze;
   if (sd->freeze)
     {
        if (sd->down.onhold_animator)
          {
             ecore_animator_del(sd->down.onhold_animator);
             sd->down.onhold_animator = NULL;
          }
     }
}

void
elm_smart_scroller_bounce_allow_set(Evas_Object *obj, Eina_Bool horiz, Eina_Bool vert)
{
   API_ENTRY return;
   sd->bounce_horiz = horiz;
   sd->bounce_vert = vert;
}

/* local subsystem functions */
static void
_smart_edje_drag_v(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Smart_Data *sd;
   
   sd = data;
   _smart_scrollbar_read(sd);
}

static void
_smart_edje_drag_h(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Smart_Data *sd;
   
   sd = data;
   _smart_scrollbar_read(sd);
}

static void
_smart_child_del_hook(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Smart_Data *sd;
   
   sd = data;
   sd->child_obj = NULL;
   _smart_scrollbar_size_adjust(sd);
   _smart_scrollbar_reset(sd);
}

static void
_smart_pan_changed_hook(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Coord x, y;
   Evas_Coord w, h;
   Smart_Data *sd;
   
   sd = data;
   if (sd->down.bounce_x_animator)
     {
        ecore_animator_del(sd->down.bounce_x_animator);
        sd->down.bounce_x_animator = NULL;
        sd->bouncemex = 0;
     }
   if (sd->down.bounce_y_animator)
     {
        ecore_animator_del(sd->down.bounce_y_animator);
        sd->down.bounce_y_animator = NULL;
        sd->bouncemey = 0;
     }
   sd->pan_func.get(sd->pan_obj, &x, &y);
   sd->pan_func.child_size_get(sd->pan_obj, &w, &h);
   if ((w != sd->child.w) || (h != sd->child.h))
     {
	sd->child.w = w;
	sd->child.h = h;
	_smart_scrollbar_size_adjust(sd);
        evas_object_size_hint_min_set(sd->smart_obj, sd->child.w, sd->child.h);
        elm_smart_scroller_child_pos_set(sd->smart_obj, x, y);
     }
}

static void
_smart_pan_pan_changed_hook(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Coord x, y;
   Smart_Data *sd;
   
   sd = data;
   sd->pan_func.get(sd->pan_obj, &x, &y);
   if (sd->down.bounce_x_animator)
     {
        ecore_animator_del(sd->down.bounce_x_animator);
        sd->down.bounce_x_animator = NULL;
        sd->bouncemex = 0;
     }
   if (sd->down.bounce_y_animator)
     {
        ecore_animator_del(sd->down.bounce_y_animator);
        sd->down.bounce_y_animator = NULL;
        sd->bouncemey = 0;
     }
   elm_smart_scroller_child_pos_set(sd->smart_obj, x, y);
}

static void
_smart_event_wheel(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Wheel *ev;
   Smart_Data *sd;
   Evas_Coord x = 0, y = 0;

   sd = data;
   ev = event_info;
   if (evas_key_modifier_is_set(ev->modifiers, "Control") ||
       evas_key_modifier_is_set(ev->modifiers, "Alt") ||
       evas_key_modifier_is_set(ev->modifiers, "Shift") ||
       evas_key_modifier_is_set(ev->modifiers, "Meta") ||
       evas_key_modifier_is_set(ev->modifiers, "Hyper") ||
       evas_key_modifier_is_set(ev->modifiers, "Super"))
     return;
   elm_smart_scroller_child_pos_get(sd->smart_obj, &x, &y);
   if (sd->down.bounce_y_animator)
     {
        ecore_animator_del(sd->down.bounce_y_animator);
        sd->down.bounce_y_animator = NULL;
        sd->bouncemey = 0;
     }
   if (ev->direction == 0)
     y += ev->z * sd->step.y;
   else if (ev->direction == 1)
     x += ev->z * sd->step.x;
   elm_smart_scroller_child_pos_set(sd->smart_obj, x, y);
}

static void
_smart_event_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Down *ev;
   Smart_Data *sd;
   Evas_Coord x = 0, y = 0;

   sd = data;
   ev = event_info;
   if (_elm_config->thumbscroll_enable)
     {
        if (sd->down.bounce_x_animator)
          {
             ecore_animator_del(sd->down.bounce_x_animator);
             sd->down.bounce_x_animator = NULL;
             sd->bouncemex = 0;
          }
        if (sd->down.bounce_y_animator)
          {
             ecore_animator_del(sd->down.bounce_y_animator);
             sd->down.bounce_y_animator = NULL;
             sd->bouncemey = 0;
          }
	if (sd->down.hold_animator)
	  {
	     ecore_animator_del(sd->down.hold_animator);
	     sd->down.hold_animator = NULL;
	  }
	if (sd->down.momentum_animator)
	  {
	     ecore_animator_del(sd->down.momentum_animator);
	     sd->down.momentum_animator = NULL;
             sd->down.bounce_x_hold = 0;
             sd->down.bounce_y_hold = 0;
             sd->down.ax = 0;
             sd->down.ay = 0;
	  }
	if (ev->button == 1)
	  {
	     sd->down.now = 1;
	     sd->down.dragged = 0;
	     sd->down.dir_x = 0;
	     sd->down.dir_y = 0;
	     sd->down.dir_none = 0;
	     sd->down.x = ev->canvas.x;
	     sd->down.y = ev->canvas.y;
	     elm_smart_scroller_child_pos_get(sd->smart_obj, &x, &y);
	     sd->down.sx = x;
	     sd->down.sy = y;
	     sd->down.locked = 0;
	     memset(&(sd->down.history[0]), 0, sizeof(sd->down.history[0]) * 20);
	     sd->down.history[0].timestamp = ecore_loop_time_get();
	     sd->down.history[0].x = ev->canvas.x;
	     sd->down.history[0].y = ev->canvas.y;
	  }
     }
}

static int
_smart_hold_animator(void *data)
{
   Smart_Data *sd;
   
   sd = data;
   sd->down.hold_animator = NULL;
   elm_smart_scroller_child_pos_set(sd->smart_obj, sd->down.hold_x, sd->down.hold_y);
   return 0;
}

static void
_smart_event_mouse_up(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Down *ev;
   Smart_Data *sd;
   Evas_Coord x = 0, y = 0;

   sd = data;
   ev = event_info;
   // FIXME: respect elm_widget_scroll_hold_get of parent container
   if (_elm_config->thumbscroll_enable)
     {
	if (ev->button == 1)
	  {
             if (sd->down.onhold_animator)
               {
                  ecore_animator_del(sd->down.onhold_animator);
                  sd->down.onhold_animator = NULL;
               }
	     x = ev->canvas.x - sd->down.x;
	     y = ev->canvas.y - sd->down.y;
             if (sd->down.dragged)
               {
                  if ((!sd->hold) && (!sd->freeze))
                    {
                       double t, at, dt;
                       int i;
                       Evas_Coord ax, ay, dx, dy, vel;
                       
                       t = ecore_loop_time_get();
                       ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
                       ax = ev->canvas.x;
                       ay = ev->canvas.y;
                       at = 0.0;
                       for (i = 0; i < 20; i++)
                         {
                            dt = t - sd->down.history[i].timestamp;
                            if (dt > 0.2) break;
                            at += dt;
                            ax += sd->down.history[i].x;
                            ay += sd->down.history[i].y;
                         }
                       ax /= (i + 1);
                       ay /= (i + 1);
                       at /= (i + 1);
                       at *= 4.0;
                       dx = ev->canvas.x - ax;
                       dy = ev->canvas.y - ay;
                       if (at > 0)
                         {
                            vel = sqrt((dx * dx) + (dy * dy)) / at;
                            if ((_elm_config->thumbscroll_friction > 0.0) &&
                                (vel > _elm_config->thumbscroll_momentum_threshhold))
                              {
                                 if (!sd->down.momentum_animator)
                                   sd->down.momentum_animator = ecore_animator_add(_smart_momentum_animator, sd);
                                 if (sd->down.hold_animator)
                                   {
                                      ecore_animator_del(sd->down.hold_animator);
                                      sd->down.hold_animator = NULL;
                                   }
                                 sd->down.dx = ((double)dx / at);
                                 sd->down.dy = ((double)dy / at);
                                 sd->down.anim_start = t;
                                 elm_smart_scroller_child_pos_get(sd->smart_obj, &x, &y);
                                 sd->down.sx = x;
                                 sd->down.sy = y;
                                 sd->down.b0x = 0;
                                 sd->down.b0y = 0;
                              }
                         }
                    }
                  evas_event_feed_hold(e, 0, ev->timestamp, ev->data);
	       }
	     sd->down.dragged = 0;
	     sd->down.now = 0;
             elm_smart_scroller_child_pos_get(sd->smart_obj, &x, &y);
             elm_smart_scroller_child_pos_set(sd->smart_obj, x, y);
             bounce_eval(sd);
	  }
     }
}

static int
_smart_onhold_animator(void *data)
{
   Smart_Data *sd;
   double t, td;
   double vx, vy;
   Evas_Coord x, y, ox, oy, dx, dy;
   
   sd = data;
   t = ecore_loop_time_get();
   if (sd->down.onhold_tlast > 0.0)
     {
        td = t - sd->down.onhold_tlast;
        vx = sd->down.onhold_vx * td * (double)_elm_config->thumbscroll_threshhold * 2.0;
        vy = sd->down.onhold_vy * td * (double)_elm_config->thumbscroll_threshhold * 2.0;
        elm_smart_scroller_child_pos_get(sd->smart_obj, &ox, &oy);
        sd->down.onhold_vxe += vx;
        sd->down.onhold_vye += vy;
        x = ox + (int)sd->down.onhold_vxe;
        y = oy + (int)sd->down.onhold_vye;
        sd->down.onhold_vxe -= (int)sd->down.onhold_vxe;
        sd->down.onhold_vye -= (int)sd->down.onhold_vye;
        elm_smart_scroller_child_pos_set(sd->smart_obj, x, y);
     }
   sd->down.onhold_tlast = t;
   return 1;
}

static void
_smart_event_mouse_move(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Move *ev;
   Smart_Data *sd;
   Evas_Coord x = 0, y = 0;

   sd = data;
   ev = event_info;
   // FIXME: respect elm_widget_scroll_hold_get of parent container
   if (_elm_config->thumbscroll_enable)
     {
	if (sd->down.now)
	  {
             int faildir = 0;
             
	     memmove(&(sd->down.history[1]), &(sd->down.history[0]),
		     sizeof(sd->down.history[0]) * 19);
	     sd->down.history[0].timestamp = ecore_loop_time_get();
	     sd->down.history[0].x = ev->cur.canvas.x;
	     sd->down.history[0].y = ev->cur.canvas.y;
	     
	     x = ev->cur.canvas.x - sd->down.x;
	     if (x < 0) x = -x;
	     y = ev->cur.canvas.y - sd->down.y;
	     if (y < 0) y = -y;
	     if ((sd->one_dir_at_a_time) && 
		 (!sd->down.dir_x) && (!sd->down.dir_y) && (!sd->down.dir_none))
	       {
                  if (x > _elm_config->thumbscroll_threshhold)
                    {
                       if (x > (y * 2))
                         {
			    sd->down.dir_x = 1;
			    sd->down.dir_y = 0;
			 }
                       else faildir++;
		    }
                  if (y > _elm_config->thumbscroll_threshhold)
		    {
                       if (y > (x * 2))
			 {
			    sd->down.dir_x = 0;
			    sd->down.dir_y = 1;
                         }
                       else faildir++;
		    }
                  if (faildir) sd->down.dir_none = 1;
	       }
             if (!sd->hold)
               {
                  if ((sd->down.dragged) ||
                      (((x * x) + (y * y)) > 
                       (_elm_config->thumbscroll_threshhold *
                        _elm_config->thumbscroll_threshhold)))
                    {
                       if (!sd->down.dragged)
                         evas_event_feed_hold(e, 1, ev->timestamp, ev->data);
                       ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
                       sd->down.dragged = 1;
                       x = sd->down.sx - (ev->cur.canvas.x - sd->down.x);
                       y = sd->down.sy - (ev->cur.canvas.y - sd->down.y);
                       if ((sd->down.dir_x) || (sd->down.dir_y))
                         {
                            if (!sd->down.locked)
                              {
                                 sd->down.locked_x = x;
                                 sd->down.locked_y = y;
                                 sd->down.locked = 1;
                              }
                            if (sd->down.dir_x) y = sd->down.locked_y;
                            else x = sd->down.locked_x;
                         }
                       sd->down.hold_x = x;
                       sd->down.hold_y = y;
                       if (!sd->down.hold_animator)
                         sd->down.hold_animator = ecore_animator_add(_smart_hold_animator, sd);
//                       elm_smart_scroller_child_pos_set(sd->smart_obj, x, y);
                    }
	       }
             else if (!sd->freeze)
               {
                  Evas_Coord ex, ey, ew, eh;
                  double vx = 0.0, vy = 0.0;
                  
                  evas_object_geometry_get(sd->event_obj, &ex, &ey, &ew, &eh); 
                  x = ev->cur.canvas.x - ex;
                  y = ev->cur.canvas.y - ey;
                  if (x < _elm_config->thumbscroll_threshhold)
                    {
                       if (_elm_config->thumbscroll_threshhold > 0.0)
                         vx = -(double)(_elm_config->thumbscroll_threshhold - x) /
                         _elm_config->thumbscroll_threshhold;
                       else
                         vx = -1.0;
                    }
                  else if (x > (ew - _elm_config->thumbscroll_threshhold))
                    {
                       if (_elm_config->thumbscroll_threshhold > 0.0)
                         vx = (double)(_elm_config->thumbscroll_threshhold - (ew - x)) /
                         _elm_config->thumbscroll_threshhold;
                       else
                         vx = 1.0;
                    }
                  if (y < _elm_config->thumbscroll_threshhold)
                    {
                       if (_elm_config->thumbscroll_threshhold > 0.0)
                         vy = -(double)(_elm_config->thumbscroll_threshhold - y) /
                         _elm_config->thumbscroll_threshhold;
                       else
                         vy = -1.0;
                    }
                  else if (y > (eh - _elm_config->thumbscroll_threshhold))
                    {
                       if (_elm_config->thumbscroll_threshhold > 0.0)
                         vy = (double)(_elm_config->thumbscroll_threshhold - (eh - y)) /
                         _elm_config->thumbscroll_threshhold;
                       else
                         vy = 1.0;
                    }
                  if ((vx != 0.0) || (vy != 0.0))
                    {
                       sd->down.onhold_vx = vx;
                       sd->down.onhold_vy = vy;
                       if (!sd->down.onhold_animator)
                         {
                            sd->down.onhold_vxe = 0.0;
                            sd->down.onhold_vye = 0.0;
                            sd->down.onhold_tlast = 0.0;
                            sd->down.onhold_animator = ecore_animator_add(_smart_onhold_animator, sd);
                         }
                    }
                  else
                    {
                       if (sd->down.onhold_animator)
                         {
                            ecore_animator_del(sd->down.onhold_animator);
                            sd->down.onhold_animator = NULL;
                         }
                    }
               }
	  }
     }
}

static void
_smart_event_key_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Key_Down *ev;
   Smart_Data *sd;
   Evas_Coord x = 0, y = 0, vw = 0, vh = 0, mx = 0, my = 0;
   int xch = 0, ych = 0;
   
   sd = data;
   ev = event_info;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   elm_smart_scroller_child_pos_get(sd->smart_obj, &x, &y);
   sd->pan_func.max_get(sd->pan_obj, &mx, &my);
   evas_object_geometry_get(sd->pan_obj, NULL, NULL, &vw, &vh);
   if (!strcmp(ev->keyname, "Left"))
     {
        x -= sd->step.x;
        xch = 1;
     }
   else if (!strcmp(ev->keyname, "Right"))
     {
        x += sd->step.x;
        xch = 1;
     }
   else if (!strcmp(ev->keyname, "Up"))
     {
        y -= sd->step.y;
        ych = 1;
     }
   else if (!strcmp(ev->keyname, "Home"))
     {
        y = 0;
        ych = 1;
     }
   else if (!strcmp(ev->keyname, "End"))
     {
        y = my;
        ych = 1;
     }
   else if (!strcmp(ev->keyname, "Down"))
     {
        y += sd->step.y;
        ych = 1;
     }
   else if (!strcmp(ev->keyname, "Prior"))
     {
	if (sd->page.y < 0)
	  y -= -(sd->page.y * vh) / 100;
	else
	  y -= sd->page.y;
        ych = 1;
     }
   else if (!strcmp(ev->keyname, "Next"))
     {
	if (sd->page.y < 0)
	  y += -(sd->page.y * vh) / 100;
	else
	  y += sd->page.y;
        ych = 1;
     }
   if (xch)
     {
        if (sd->down.bounce_x_animator)
          {
             ecore_animator_del(sd->down.bounce_x_animator);
             sd->down.bounce_x_animator = NULL;
             sd->bouncemex = 0;
          }
     }
   if (ych)
     {
        if (sd->down.bounce_y_animator)
          {
             ecore_animator_del(sd->down.bounce_y_animator);
             sd->down.bounce_y_animator = NULL;
             sd->bouncemey = 0;
          }
     }

   elm_smart_scroller_child_pos_set(sd->smart_obj, x, y);
}

static void
_smart_scrollbar_read(Smart_Data *sd)
{
   Evas_Coord x, y, mx = 0, my = 0, px, py;
   double vx, vy;
   
   edje_object_part_drag_value_get(sd->edje_obj, "elm.dragable.vbar", NULL, &vy);
   edje_object_part_drag_value_get(sd->edje_obj, "elm.dragable.hbar", &vx, NULL);
   sd->pan_func.max_get(sd->pan_obj, &mx, &my);
   x = vx * (double)mx;
   y = vy * (double)my;
   sd->pan_func.get(sd->pan_obj, &px, &py);
   sd->pan_func.set(sd->pan_obj, x, y);
   if ((px != x) || (py != y))
     edje_object_signal_emit(sd->edje_obj, "elm,action,scroll", "elm");
}

static void
_smart_scrollbar_reset(Smart_Data *sd)
{
   Evas_Coord px = 0, py = 0;
   
   edje_object_part_drag_value_set(sd->edje_obj, "elm.dragable.vbar", 0.0, 0.0);
   edje_object_part_drag_value_set(sd->edje_obj, "elm.dragable.hbar", 0.0, 0.0);
   if ((!sd->child_obj) && (!sd->extern_pan))
     {
	edje_object_part_drag_size_set(sd->edje_obj, "elm.dragable.vbar", 1.0, 1.0);
	edje_object_part_drag_size_set(sd->edje_obj, "elm.dragable.hbar", 1.0, 1.0);
     }
   sd->pan_func.get(sd->pan_obj, &px, &py);
   sd->pan_func.set(sd->pan_obj, 0, 0);
   if ((px != 0) || (py != 0))
     edje_object_signal_emit(sd->edje_obj, "elm,action,scroll", "elm");
}

static int
_smart_scrollbar_bar_v_visibility_adjust(Smart_Data *sd)
{
   int scroll_v_vis_change = 0;
   Evas_Coord w, h, vw, vh;
   
   w = sd->child.w;
   h = sd->child.h;
   evas_object_geometry_get(sd->pan_obj, NULL, NULL, &vw, &vh);
   if (sd->vbar_visible)
     {
	if (sd->vbar_flags == ELM_SMART_SCROLLER_POLICY_AUTO)
	  {
	     if ((sd->child_obj) || (sd->extern_pan))
	       {
		  if (h <= vh)
		    {
		       scroll_v_vis_change = 1;
		       sd->vbar_visible = 0;
		    }
	       }
	     else
	       {
		  scroll_v_vis_change = 1;
		  sd->vbar_visible = 0;
	       }
	  }
	else if (sd->vbar_flags == ELM_SMART_SCROLLER_POLICY_OFF)
	  {
	     scroll_v_vis_change = 1;
	     sd->vbar_visible = 0;
	  }
     }
   else
     {
	if (sd->vbar_flags == ELM_SMART_SCROLLER_POLICY_AUTO)
	  {
	     if ((sd->child_obj) || (sd->extern_pan))
	       {
		  if (h > vh)
		    {
		       scroll_v_vis_change = 1;
		       sd->vbar_visible = 1;
		    }
	       }
	  }
	else if (sd->vbar_flags == ELM_SMART_SCROLLER_POLICY_ON)
	  {
	     scroll_v_vis_change = 1;
	     sd->vbar_visible = 1;
	  }
     }
   if (scroll_v_vis_change)
     {
        if (sd->vbar_flags != ELM_SMART_SCROLLER_POLICY_OFF)
          {
             if (sd->vbar_visible)
               edje_object_signal_emit(sd->edje_obj, "elm,action,show,vbar", "elm");
             else
               edje_object_signal_emit(sd->edje_obj, "elm,action,hide,vbar", "elm");
             edje_object_message_signal_process(sd->edje_obj);
             _smart_scrollbar_size_adjust(sd);
          }
        else
          edje_object_signal_emit(sd->edje_obj, "elm,action,hide,vbar", "elm");
     }
   return scroll_v_vis_change;
}

static int
_smart_scrollbar_bar_h_visibility_adjust(Smart_Data *sd)
{
   int scroll_h_vis_change = 0;
   Evas_Coord w, h, vw, vh;
   
   w = sd->child.w;
   h = sd->child.h;
   evas_object_geometry_get(sd->pan_obj, NULL, NULL, &vw, &vh);
   if (sd->hbar_visible)
     {
	if (sd->hbar_flags == ELM_SMART_SCROLLER_POLICY_AUTO)
	  {
	     if ((sd->child_obj) || (sd->extern_pan))
	       {
		  if (w <= vw)
		    {
		       scroll_h_vis_change = 1;
		       sd->hbar_visible = 0;
		    }
	       }
	     else
	       {
		  scroll_h_vis_change = 1;
		  sd->hbar_visible = 0;
	       }
	  }
	else if (sd->hbar_flags == ELM_SMART_SCROLLER_POLICY_OFF)
	  {
	     scroll_h_vis_change = 1;
	     sd->hbar_visible = 0;
	  }
     }
   else
     {
	if (sd->hbar_flags == ELM_SMART_SCROLLER_POLICY_AUTO)
	  {
	     if ((sd->child_obj) || (sd->extern_pan))
	       {
		  if (w > vw)
		    {
		       scroll_h_vis_change = 1;
		       sd->hbar_visible = 1;
		    }
	       }
	  }
	else if (sd->hbar_flags == ELM_SMART_SCROLLER_POLICY_ON)
	  {
	     scroll_h_vis_change = 1;
	     sd->hbar_visible = 1;
	  }
     }
   if (scroll_h_vis_change)
     {
        if (sd->hbar_flags != ELM_SMART_SCROLLER_POLICY_OFF)
          {
             if (sd->hbar_visible)
               edje_object_signal_emit(sd->edje_obj, "elm,action,show,hbar", "elm");
             else
               edje_object_signal_emit(sd->edje_obj, "elm,action,hide,hbar", "elm");
             edje_object_message_signal_process(sd->edje_obj);
             _smart_scrollbar_size_adjust(sd);
          }
        else
          edje_object_signal_emit(sd->edje_obj, "elm,action,hide,hbar", "elm");
	_smart_scrollbar_size_adjust(sd);
     }
   return scroll_h_vis_change;
}

static void
_smart_scrollbar_bar_visibility_adjust(Smart_Data *sd)
{
   int changed = 0;
   
   changed |= _smart_scrollbar_bar_h_visibility_adjust(sd);
   changed |= _smart_scrollbar_bar_v_visibility_adjust(sd);
   if (changed)
     {
	_smart_scrollbar_bar_h_visibility_adjust(sd);
	_smart_scrollbar_bar_v_visibility_adjust(sd);
     }
}

static void
_smart_scrollbar_size_adjust(Smart_Data *sd)
{
   if ((sd->child_obj) || (sd->extern_pan))
     {
	Evas_Coord x, y, w, h, mx = 0, my = 0, vw = 0, vh = 0, px, py;
	double vx, vy, size;

	edje_object_part_geometry_get(sd->edje_obj, "elm.swallow.content", 
				      NULL, NULL, &vw, &vh);
	w = sd->child.w;
	if (w < 1) w = 1;
	size = (double)vw / (double)w;
	if (size > 1.0)
	  {
	     size = 1.0;
	     edje_object_part_drag_value_set(sd->edje_obj, "elm.dragable.hbar", 0.0, 0.0);
	  }
	edje_object_part_drag_size_set(sd->edje_obj, "elm.dragable.hbar", size, 1.0);
	
	h = sd->child.h;
	if (h < 1) h = 1;
	size = (double)vh / (double)h;
	if (size > 1.0)
	  {
	     size = 1.0;
	     edje_object_part_drag_value_set(sd->edje_obj, "elm.dragable.vbar", 0.0, 0.0);
	  }
	edje_object_part_drag_size_set(sd->edje_obj, "elm.dragable.vbar", 1.0, size);

	edje_object_part_drag_value_get(sd->edje_obj, "elm.dragable.hbar", &vx, NULL);
	edje_object_part_drag_value_get(sd->edje_obj, "elm.dragable.vbar", NULL, &vy);
	sd->pan_func.max_get(sd->pan_obj, &mx, &my);
	x = vx * mx;
	y = vy * my;
	
	edje_object_part_drag_step_set(sd->edje_obj, "elm.dragable.hbar", (double)sd->step.x / (double)w, 0.0);
	edje_object_part_drag_step_set(sd->edje_obj, "elm.dragable.vbar", 0.0, (double)sd->step.y / (double)h);
	if (sd->page.x > 0)
	  edje_object_part_drag_page_set(sd->edje_obj, "elm.dragable.hbar", (double)sd->page.x / (double)w, 0.0);
	else
	  edje_object_part_drag_page_set(sd->edje_obj, "elm.dragable.hbar", -((double)sd->page.x * ((double)vw / (double)w)) / 100.0, 0.0);
	if (sd->page.y > 0)
	  edje_object_part_drag_page_set(sd->edje_obj, "elm.dragable.vbar", 0.0, (double)sd->page.y / (double)h);
	else
	  edje_object_part_drag_page_set(sd->edje_obj, "elm.dragable.vbar", 0.0, -((double)sd->page.y * ((double)vh / (double)h)) / 100.0);
	
	sd->pan_func.get(sd->pan_obj, &px, &py);
        if (vx != mx) x = px;
        if (vy != my) y = py;
	sd->pan_func.set(sd->pan_obj, x, y);
//	if ((px != 0) || (py != 0))
//	  edje_object_signal_emit(sd->edje_obj, "elm,action,scroll", "elm");
     }
   else
     {
	Evas_Coord px = 0, py = 0;
	
	edje_object_part_drag_size_set(sd->edje_obj, "elm.dragable.vbar", 1.0, 1.0);
	edje_object_part_drag_size_set(sd->edje_obj, "elm.dragable.hbar", 1.0, 1.0);
	sd->pan_func.get(sd->pan_obj, &px, &py);
	sd->pan_func.set(sd->pan_obj, 0, 0);
	if ((px != 0) || (py != 0))
	  edje_object_signal_emit(sd->edje_obj, "elm,action,scroll", "elm");
     }
   _smart_scrollbar_bar_visibility_adjust(sd);
}
                    
static void
_smart_reconfigure(Smart_Data *sd)
{
   evas_object_move(sd->edje_obj, sd->x, sd->y);
   evas_object_resize(sd->edje_obj, sd->w, sd->h);
   evas_object_move(sd->event_obj, sd->x, sd->y);
   evas_object_resize(sd->event_obj, sd->w, sd->h);
   _smart_scrollbar_size_adjust(sd);
}

static void
_smart_add(Evas_Object *obj)
{
   Smart_Data *sd;
   Evas_Object *o;
   
   sd = calloc(1, sizeof(Smart_Data));
   if (!sd) return;
   evas_object_smart_data_set(obj, sd);
   
   sd->smart_obj = obj;
   sd->x = 0;
   sd->y = 0;
   sd->w = 0;
   sd->h = 0;
   sd->step.x = 32;
   sd->step.y = 32;
   sd->page.x = -50;
   sd->page.y = -50;
   sd->hbar_flags = ELM_SMART_SCROLLER_POLICY_AUTO;
   sd->vbar_flags = ELM_SMART_SCROLLER_POLICY_AUTO;
   sd->hbar_visible = 1;
   sd->vbar_visible = 1;
   
   sd->bounce_horiz = 1;
   sd->bounce_vert = 1;
   
   sd->one_dir_at_a_time = 1;
   
   evas_object_event_callback_add(obj, EVAS_CALLBACK_KEY_DOWN, _smart_event_key_down, sd);
   evas_object_propagate_events_set(obj, 0);
   
   o = edje_object_add(evas_object_evas_get(obj));
   sd->edje_obj = o;
   _elm_theme_set(o, "scroller", "base", "default");
   edje_object_signal_callback_add(o, "drag", "elm.dragable.vbar", _smart_edje_drag_v, sd);
   edje_object_signal_callback_add(o, "drag,start", "elm.dragable.vbar", _smart_edje_drag_v, sd);
   edje_object_signal_callback_add(o, "drag,stop", "elm.dragable.vbar", _smart_edje_drag_v, sd);
   edje_object_signal_callback_add(o, "drag,step", "elm.dragable.vbar", _smart_edje_drag_v, sd);
   edje_object_signal_callback_add(o, "drag,page", "elm.dragable.vbar", _smart_edje_drag_v, sd);
   edje_object_signal_callback_add(o, "drag", "elm.dragable.hbar", _smart_edje_drag_h, sd);
   edje_object_signal_callback_add(o, "drag,start", "elm.dragable.hbar", _smart_edje_drag_h, sd);
   edje_object_signal_callback_add(o, "drag,stop", "elm.dragable.hbar", _smart_edje_drag_h, sd);
   edje_object_signal_callback_add(o, "drag,step", "elm.dragable.hbar", _smart_edje_drag_h, sd);
   edje_object_signal_callback_add(o, "drag,page", "elm.dragable.hbar", _smart_edje_drag_h, sd);
   evas_object_smart_member_add(o, obj);
   
   o = evas_object_rectangle_add(evas_object_evas_get(obj));
   sd->event_obj = o;
   evas_object_color_set(o, 0, 0, 0, 0);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_WHEEL, _smart_event_wheel, sd);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, _smart_event_mouse_down, sd);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_UP, _smart_event_mouse_up, sd);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_MOVE, _smart_event_mouse_move, sd);
   evas_object_smart_member_add(o, obj);
   evas_object_repeat_events_set(o, 1);

   sd->pan_func.set = _elm_smart_pan_set;
   sd->pan_func.get = _elm_smart_pan_get;
   sd->pan_func.max_get = _elm_smart_pan_max_get;
   sd->pan_func.child_size_get = _elm_smart_pan_child_size_get;
   
   _smart_scrollbar_reset(sd);
}

static void
_smart_del(Evas_Object *obj)
{
   INTERNAL_ENTRY;
   elm_smart_scroller_child_set(obj, NULL);
   if (!sd->extern_pan) evas_object_del(sd->pan_obj);
   evas_object_del(sd->edje_obj);
   evas_object_del(sd->event_obj);
   if (sd->down.hold_animator) ecore_animator_del(sd->down.hold_animator);
   if (sd->down.onhold_animator) ecore_animator_del(sd->down.onhold_animator);
   if (sd->down.momentum_animator) ecore_animator_del(sd->down.momentum_animator);
   if (sd->down.bounce_x_animator) ecore_animator_del(sd->down.bounce_x_animator);
   if (sd->down.bounce_y_animator) ecore_animator_del(sd->down.bounce_y_animator);
   free(sd);
   evas_object_smart_data_set(obj, NULL);
}

static void
_smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   INTERNAL_ENTRY;
   sd->x = x;
   sd->y = y;
   _smart_reconfigure(sd);
}

static void
_smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   INTERNAL_ENTRY;
   sd->w = w;
   sd->h = h;
   _smart_reconfigure(sd);
}

static void
_smart_show(Evas_Object *obj)
{
   INTERNAL_ENTRY;
   evas_object_show(sd->edje_obj);
   evas_object_show(sd->event_obj);
}

static void
_smart_hide(Evas_Object *obj)
{
   INTERNAL_ENTRY;
   evas_object_hide(sd->edje_obj);
   evas_object_hide(sd->event_obj);
}

static void
_smart_color_set(Evas_Object *obj, int r, int g, int b, int a)
{
   INTERNAL_ENTRY;
   evas_object_color_set(sd->edje_obj, r, g, b, a);
}

static void
_smart_clip_set(Evas_Object *obj, Evas_Object * clip)
{
   INTERNAL_ENTRY;
   evas_object_clip_set(sd->edje_obj, clip);
   evas_object_clip_set(sd->event_obj, clip);
}

static void
_smart_clip_unset(Evas_Object *obj)
{
   INTERNAL_ENTRY;
   evas_object_clip_unset(sd->edje_obj);
   evas_object_clip_unset(sd->event_obj);
}  

/* never need to touch this */

static void
_smart_init(void)
{
   if (_smart) return;
     {
	static const Evas_Smart_Class sc =
	  {
	     SMART_NAME,
	       EVAS_SMART_CLASS_VERSION,
	       _smart_add,
	       _smart_del, 
	       _smart_move,
	       _smart_resize,
	       _smart_show,
	       _smart_hide,
	       _smart_color_set,
	       _smart_clip_set,
	       _smart_clip_unset,
	       NULL,
	       NULL,
	       NULL,
	       NULL
	  };
	_smart = evas_smart_class_new(&sc);
     }
}

