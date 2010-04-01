#include "e_mod_main.h"

typedef struct _View View;
typedef struct _Smart_Data Smart_Data;
typedef struct _Item Item;

struct _View
{
  Evry_View view;
  Tab_View *tabs;

  const Evry_State *state;
  const Evry_Plugin *plugin;

  Evas *evas;
  Evas_Object *bg, *sframe, *span;
  int          iw, ih;
  int          zoom;
  int         list_mode;
};

/* smart object based on wallpaper module */
struct _Smart_Data
{
  View        *view;
  Eina_List   *items;
  Item        *cur_item;
  Ecore_Idle_Enterer *idle_enter;
  Ecore_Idle_Enterer *thumb_idler;
  Ecore_Idle_Enterer *update_idler;
  Ecore_Animator *animator;
  Evas_Coord   x, y, w, h;
  Evas_Coord   cx, cy, cw, ch;
  Evas_Coord   sx, sy;
  double       selmove;
  Eina_Bool    update : 1;
  Eina_Bool    switch_mode : 1;
  Eina_List *queue;
};

struct _Item
{
  Evry_Item *item;
  Evas_Object *obj;
  Evas_Coord x, y, w, h;
  Evas_Object *frame, *image, *thumb;
  Eina_Bool selected : 1;
  Eina_Bool have_thumb : 1;
  Eina_Bool do_thumb : 1;
  Eina_Bool get_thumb : 1;
  Eina_Bool showing : 1;
  Eina_Bool visible : 1;
  Eina_Bool changed : 1;
  int pos;
};

static View *view = NULL;
static const char *view_types = NULL;

static void
_thumb_gen(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Coord w, h;
   Item *it = data;

   if (!it->frame) return;

   e_icon_size_get(it->thumb, &w, &h);
   edje_extern_object_aspect_set(it->thumb, EDJE_ASPECT_CONTROL_BOTH, w, h);
   edje_object_part_swallow(it->frame, "e.swallow.thumb", it->thumb);
   evas_object_show(it->thumb);
   it->have_thumb = EINA_TRUE;
   it->do_thumb = EINA_FALSE;

   if (it->image) evas_object_del(it->image);
   it->image = NULL;

}

static int
_check_item(const Evry_Item *it)
{
   if (it->plugin->type_out != view_types) return 0;

   ITEM_FILE(file, it);

   if (!file->uri || !file->mime) return 0;

   if (!strncmp(file->mime, "image/", 6))
     return 1;

   return 0;
}

static int
_thumb_idler(void *data)
{
   Smart_Data *sd = data;
   Eina_List *l, *ll;
   Item *it;

   EINA_LIST_FOREACH_SAFE(sd->queue, l, ll, it)
     {
	if (!it->image && !it->have_thumb &&
	    sd->view->state->plugin &&
	    sd->view->state->plugin->icon_get)
	  {
	     it->image = sd->view->state->plugin->icon_get
	       (it->item->plugin, it->item, sd->view->evas);

	     if (it->image)
	       {
		  edje_object_part_swallow(it->frame, "e.swallow.icon", it->image);
		  evas_object_show(it->image);
	       }

	     /* dirbrowse fetches the mimetype for icon_get */
	     if (!it->get_thumb && _check_item(it->item))
	       it->get_thumb = EINA_TRUE;
	  }

	if (it->get_thumb && !it->thumb && !(it->have_thumb || it->do_thumb))
	  {
	     it->thumb = e_thumb_icon_add(sd->view->evas);
	     
	     ITEM_FILE(file, it->item);

	     evas_object_smart_callback_add(it->thumb, "e_thumb_gen", _thumb_gen, it);

	     e_thumb_icon_file_set(it->thumb, file->uri, NULL);
	     e_thumb_icon_size_set(it->thumb, it->w, it->h);
	     e_thumb_icon_begin(it->thumb);
	     it->do_thumb = EINA_TRUE;
	  }

	sd->queue = eina_list_remove_list(sd->queue, l);
	e_util_wakeup();
	return 1;
     }

   sd->thumb_idler = NULL;

   return 0;
}

static int
_e_smart_reconfigure_do(void *data)
{
   Evas_Object *obj = data;
   Smart_Data *sd = evas_object_smart_data_get(obj);
   Eina_List *l;
   Item *it;
   int iw, redo = 0, changed = 0;
   static int recursion = 0;
   Evas_Coord x, y, xx, yy, ww, hh, mw, mh, ox = 0, oy = 0;
   Evas_Coord aspect_w, aspect_h;

   if (!sd) return 0;
   if (sd->cx > (sd->cw - sd->w)) sd->cx = sd->cw - sd->w;
   if (sd->cy > (sd->ch - sd->h)) sd->cy = sd->ch - sd->h;
   if (sd->cx < 0) sd->cx = 0;
   if (sd->cy < 0) sd->cy = 0;

   aspect_w = sd->w;
   aspect_h = sd->h;

   if (sd->view->list_mode)
     {
	iw = sd->w;
     }
   else if (sd->view->zoom == 0)
     {
	int cnt = eina_list_count(sd->items);

	aspect_w *= 3;
	aspect_w /= 4;

	if (cnt < 3)
	  iw = (double)sd->w / 2.5;
	else if (cnt < 7)
	  iw = sd->w / 3;
	else
	  iw = sd->w / 4;
     }
   else if (sd->view->zoom == 1)
     {
	aspect_w *= 2;
	aspect_w /= 3;
	iw = sd->w / 3;
     }
   else /* if (sd->zoom == 2) */
     {
	iw = sd->w;
     }

   if (aspect_w <= 0) aspect_w = 1;
   if (aspect_h <= 0) aspect_h = 1;

   x = 0;
   y = 0;
   ww = iw;
   if (sd->view->list_mode)
     hh = 32;
   else
     hh = (aspect_h * iw) / (aspect_w);

   mw = mh = 0;
   EINA_LIST_FOREACH(sd->items, l, it)
     {
        if (x > (sd->w - ww))
          {
             x = 0;
             y += hh;
          }

        it->x = x;
        it->y = y;
        it->w = ww;
        it->h = hh;

        if ((x + ww) > mw) mw = x + ww;
        if ((y + hh) > mh) mh = y + hh;
        x += ww;
     }

   if ((mw != sd->cw) || (mh != sd->ch))
     {
        sd->cw = mw;
        sd->ch = mh;
        if (sd->cx > (sd->cw - sd->w))
          {
             sd->cx = sd->cw - sd->w;
             redo = 1;
          }
        if (sd->cy > (sd->ch - sd->h))
          {
             sd->cy = sd->ch - sd->h;
             redo = 1;
          }
        if (sd->cx < 0)
          {
             sd->cx = 0;
             redo = 1;
          }
        if (sd->cy < 0)
          {
             sd->cy = 0;
             redo = 1;
          }
        if (redo)
   	  {
   	     recursion = 1;
   	     _e_smart_reconfigure_do(obj);
   	     recursion = 0;
   	  }
        changed = 1;
     }

   if (sd->switch_mode) 
     {
	if (changed)
	  evas_object_smart_callback_call(obj, "changed", NULL);
	
	sd->update = EINA_TRUE;
	sd->switch_mode = EINA_FALSE;

	if (recursion == 0)
	  sd->idle_enter = NULL;
	return 0;
     }
   
   if (!sd->view->list_mode)
     {
	if (sd->w > sd->cw) ox = (sd->w - sd->cw) / 2;
	if (sd->h > sd->ch) oy = (sd->h - sd->ch) / 2;
     }
     
   EINA_LIST_FOREACH(sd->items, l, it)
     {
        xx = sd->x - sd->cx + it->x + ox;
        yy = sd->y - sd->cy + it->y + oy;

        if (E_INTERSECTS(xx, yy, it->w, it->h, 0, sd->y - (it->h*4),
			 sd->x + sd->w, sd->y + sd->h + it->h*8))
          {
	     if (!it->visible)
	       {
		  it->frame = edje_object_add(sd->view->evas);
		  if (sd->view->list_mode)
		    e_theme_edje_object_set(it->frame, "base/theme/widgets",
					    "e/modules/everything/thumbview/item/list");
		  else
		    e_theme_edje_object_set(it->frame, "base/theme/widgets",
					    "e/modules/everything/thumbview/item/thumb");

		  evas_object_smart_member_add(it->frame, obj);
		  evas_object_clip_set(it->frame, evas_object_clip_get(obj));

		  edje_object_part_text_set(it->frame, "e.text.label", it->item->label);
		  evas_object_show(it->frame);

		  if (it->changed)
		    edje_object_signal_emit(it->frame, "e,action,thumb,show_delayed", "e");
		  else
		    edje_object_signal_emit(it->frame, "e,action,thumb,show", "e");
		  
		  if (it->item->browseable)
		    edje_object_signal_emit(it->frame, "e,state,browseable", "e");
		  
		  it->visible = EINA_TRUE;
	       }

	     if (!eina_list_data_find(sd->queue, it))
	       {
		  sd->queue = eina_list_append(sd->queue, it);
	       }
	     
	     evas_object_move(it->frame, xx, yy);
	     evas_object_resize(it->frame, it->w, it->h);

	     /* if (sd->update || it->changed)
	      *   {
	      * 	  if (it->selected && sd->view->zoom < 2)
	      * 	    edje_object_signal_emit(it->frame, "e,state,selected", "e");
	      * 	  else
	      * 	    edje_object_signal_emit(it->frame, "e,state,unselected", "e");
	      *   } */
          }
        else if (it->visible)
	  {
	     sd->queue = eina_list_remove(sd->queue, it); 
	     if (it->do_thumb) e_thumb_icon_end(it->thumb);
	     if (it->thumb) evas_object_del(it->thumb);
	     if (it->image) evas_object_del(it->image);
	     if (it->frame) evas_object_del(it->frame);

	     it->thumb = NULL;
	     it->image = NULL;
	     it->frame = NULL;

	     it->have_thumb = EINA_FALSE;
	     it->do_thumb = EINA_FALSE;
	     it->visible = EINA_FALSE;
	  }
	it->changed = EINA_FALSE;
     }

   if (changed)
     evas_object_smart_callback_call(obj, "changed", NULL);

   if (!sd->thumb_idler)
     sd->thumb_idler = ecore_idle_enterer_before_add(_thumb_idler, sd);
   
   sd->update = EINA_TRUE;

   if (recursion == 0)
     sd->idle_enter = NULL;
   return 0;
}

static void
_e_smart_reconfigure(Evas_Object *obj)
{
   Smart_Data *sd = evas_object_smart_data_get(obj);

   if (sd->idle_enter) return;
   sd->idle_enter = ecore_idle_enterer_before_add(_e_smart_reconfigure_do, obj);
}

static void
_e_smart_add(Evas_Object *obj)
{
   Smart_Data *sd = calloc(1, sizeof(Smart_Data));
   if (!sd) return;
   sd->x = sd->y = sd->w = sd->h = 0;
   sd->sx = sd->sy = -1;
   evas_object_smart_data_set(obj, sd);
}

static void
_e_smart_del(Evas_Object *obj)
{
   Smart_Data *sd = evas_object_smart_data_get(obj);
   Item *it;

   if (sd->idle_enter)
     ecore_idle_enterer_del(sd->idle_enter);
   if (sd->thumb_idler)
     ecore_idle_enterer_del(sd->thumb_idler);

   // sd->view is just referenced
   // sd->child_obj is unused
   EINA_LIST_FREE(sd->items, it)
     {
	if (it->do_thumb) e_thumb_icon_end(it->thumb);
        if (it->thumb) evas_object_del(it->thumb);
        if (it->frame) evas_object_del(it->frame);
        if (it->image) evas_object_del(it->image);
	evry_item_free(it->item);
        free(it);
     }
   free(sd);
   evas_object_smart_data_set(obj, NULL);
}

static void
_e_smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   Smart_Data *sd = evas_object_smart_data_get(obj);
   sd->x = x;
   sd->y = y;
   _e_smart_reconfigure(obj);
}

static void
_e_smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   Smart_Data *sd = evas_object_smart_data_get(obj);
   sd->w = w;
   sd->h = h;
   _e_smart_reconfigure(obj);
   evas_object_smart_callback_call(obj, "changed", NULL);
}

static void
_e_smart_show(Evas_Object *obj){}

static void
_e_smart_hide(Evas_Object *obj){}

static void
_e_smart_color_set(Evas_Object *obj, int r, int g, int b, int a){}

static void
_e_smart_clip_set(Evas_Object *obj, Evas_Object * clip){}

static void
_e_smart_clip_unset(Evas_Object *obj){}

static Evas_Object *
_pan_add(Evas *evas)
{
   static Evas_Smart *smart = NULL;
   static const Evas_Smart_Class sc =
     {
       "wp_pan",
       EVAS_SMART_CLASS_VERSION,
       _e_smart_add,
       _e_smart_del,
       _e_smart_move,
       _e_smart_resize,
       _e_smart_show,
       _e_smart_hide,
       _e_smart_color_set,
       _e_smart_clip_set,
       _e_smart_clip_unset,
       NULL,
       NULL,
       NULL,
       NULL
     };
   smart = evas_smart_class_new(&sc);
   return evas_object_smart_add(evas, smart);
}

static void
_pan_set(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   Smart_Data *sd = evas_object_smart_data_get(obj);
   if (x > (sd->cw - sd->w)) x = sd->cw - sd->w;
   if (y > (sd->ch - sd->h)) y = sd->ch - sd->h;
   if (x < 0) x = 0;
   if (y < 0) y = 0;
   if ((sd->cx == x) && (sd->cy == y)) return;
   sd->cx = x;
   sd->cy = y;
   _e_smart_reconfigure(obj);
}

static void
_pan_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y)
{
   Smart_Data *sd = evas_object_smart_data_get(obj);
   if (x) *x = sd->cx;
   if (y) *y = sd->cy;
}

static void
_pan_max_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y)
{
   Smart_Data *sd = evas_object_smart_data_get(obj);
   if (x)
     {
        if (sd->w < sd->cw) *x = sd->cw - sd->w;
        else *x = 0;
     }
   if (y)
     {
        if (sd->h < sd->ch) *y = sd->ch - sd->h;
        else *y = 0;
     }
}

static void
_pan_child_size_get(Evas_Object *obj, Evas_Coord *w, Evas_Coord *h)
{
   Smart_Data *sd = evas_object_smart_data_get(obj);
   if (w) *w = sd->cw;
   if (h) *h = sd->ch;
}

static void
_pan_view_set(Evas_Object *obj, View *view)
{
   Smart_Data *sd = evas_object_smart_data_get(obj);
   sd->view = view;
}

static Item *
_pan_item_add(Evas_Object *obj, Evry_Item *item)
{
   Smart_Data *sd = evas_object_smart_data_get(obj);
   Item *it;

   it = E_NEW(Item, 1);
   if (!it) return NULL;

   sd->items = eina_list_append(sd->items, it);
   it->obj = obj;
   it->item = item;
   it->changed = EINA_TRUE;

   if (_check_item(item))
     it->get_thumb = EINA_TRUE;

   evry_item_ref(item);

   _e_smart_reconfigure(obj);

   return it;
}

static void
_pan_item_remove(Evas_Object *obj, Item *it)
{
   Smart_Data *sd = evas_object_smart_data_get(obj);

   sd->items = eina_list_remove(sd->items, it);
   if (it->do_thumb) e_thumb_icon_end(it->thumb);
   if (it->thumb) evas_object_del(it->thumb);
   if (it->frame) evas_object_del(it->frame);
   if (it->image) evas_object_del(it->image);

   sd->queue = eina_list_remove(sd->queue, it); 
   
   evry_item_free(it->item);
   E_FREE(it);

   _e_smart_reconfigure(obj);
}

static void
_pan_item_select(Evas_Object *obj, Item *it)
{
   Smart_Data *sd = evas_object_smart_data_get(obj);
   int align = -1;

   if (sd->cur_item)
     {
	sd->cur_item->selected = EINA_FALSE;
	edje_object_signal_emit(sd->cur_item->frame, "e,state,unselected", "e");
     }

   if (it)
     {
	sd->update = EINA_FALSE;

	sd->cur_item = it;
	sd->cur_item->selected = EINA_TRUE;

	if (sd->view->list_mode)
	  align = it->y - (double)it->y / (double)sd->ch * (sd->h - it->h);
	else if ((it->y + it->h) - sd->cy > sd->h)
	  align = it->y - (2 - sd->view->zoom) * it->h;
	else if (it->y < sd->cy)
	  align = it->y;

	if (align >= 0)
	  e_scrollframe_child_pos_set(sd->view->sframe, 0, align);

	if (sd->view->zoom < 2)
	  edje_object_signal_emit(sd->cur_item->frame, "e,state,selected", "e");

	if (sd->idle_enter) ecore_idle_enterer_del(sd->idle_enter);
	sd->idle_enter = ecore_idle_enterer_before_add(_e_smart_reconfigure_do, obj);
     }
}

static void
_clear_items(Evas_Object *obj)
{
   Smart_Data *sd = evas_object_smart_data_get(obj);
   Eina_List *l;
   Item *it;

   EINA_LIST_FOREACH(sd->items, l, it)
     {
	if (it->do_thumb)
	  e_thumb_icon_end(it->thumb);
	if (it->frame) evas_object_del(it->frame);
	if (it->image) evas_object_del(it->image);
	if (it->thumb) evas_object_del(it->thumb);
	it->frame = NULL;
	it->image = NULL;
	it->thumb = NULL;
	it->have_thumb = EINA_FALSE;
	it->do_thumb = EINA_FALSE;
	it->visible = EINA_FALSE;
     }

   if (sd->queue)
     eina_list_free(sd->queue);
   sd->queue = NULL;

   if (sd->thumb_idler)
     ecore_idle_enterer_del(sd->thumb_idler);
   sd->thumb_idler = NULL;
}

static void
_view_clear(Evry_View *view)
{
   View *v = (View*) view;
   Smart_Data *sd = evas_object_smart_data_get(v->span);
   Item *it;

   _clear_items(v->span);
   
   if (sd->idle_enter) ecore_idle_enterer_del(sd->idle_enter);
   sd->idle_enter = ecore_idle_enterer_before_add(_e_smart_reconfigure_do, v->span);
   
   v->tabs->clear(v->tabs);
}

static int
_sort_cb(const void *data1, const void *data2)
{
   const Item *it1 = data1;
   const Item *it2 = data2;

   return it1->pos - it2->pos;
}

static int
_update_frame(Evas_Object *obj)
{
   Smart_Data *sd = evas_object_smart_data_get(obj);
   //sd->switch_mode = EINA_TRUE;
   _e_smart_reconfigure_do(obj);
   //sd->switch_mode = EINA_FALSE;
   _pan_item_select(obj, sd->cur_item); 

   return 0;
}

static int
_view_update(Evry_View *view)
{
   VIEW(v, view);
   Smart_Data *sd = evas_object_smart_data_get(v->span);
   Item *v_it;
   Evry_Item *p_it;
   Eina_List *l, *ll, *p_items, *v_remove = NULL, *v_items = NULL;
   int pos, last_pos, last_vis = 0, first_vis = 0;
   Eina_Bool update = EINA_FALSE;
   
   if (!v->state->plugin)
     {
	_view_clear(view);
	return 1;
     }

   p_items = v->state->plugin->items;

   /* go through current view items */
   EINA_LIST_FOREACH(sd->items, l, v_it)
     {
	last_pos = v_it->pos;
	v_it->pos = 0;
	pos = 1;	
	
	/* go through plugins current items */
	EINA_LIST_FOREACH(p_items, ll, p_it)
	  {
	     if (v_it->item == p_it)
	       {
		  if (pos != last_pos)
		    v_it->changed = EINA_TRUE;
		  
		  v_it->pos = pos;

		  /* set selected state -> TODO remove*/
		  if (p_it == v->state->cur_item)
		    {
		       sd->cur_item = v_it;
		       v_it->selected = EINA_TRUE;
		    }
		  else
		    {
		       v_it->selected = EINA_FALSE;
		       edje_object_signal_emit(v_it->frame, "e,state,unselected", "e");
		    }
		  
		  break;
	       }
	     pos++;
	  }

	if (v_it->visible)
	  {
	     if (!first_vis)
	       first_vis = v_it->pos;
	     last_vis = v_it->pos;
	  }
	
	/* view item is in list of current items */
	if (v_it->pos)
	  {
	     v_items = eina_list_append(v_items, v_it->item);

	     if (_check_item(v_it->item))
	       v_it->get_thumb = EINA_TRUE;
	     
	     if (v_it->visible && v_it->changed)
	       update = EINA_TRUE;
	  }
	else
	  {
	     if (v_it->visible) update = EINA_TRUE;
	     v_remove = eina_list_append(v_remove, v_it);
	  }
     }

   EINA_LIST_FREE(v_remove, v_it)
     _pan_item_remove(v->span, v_it);

   /* go through plugins current items */
   pos = 1;
   EINA_LIST_FOREACH(p_items, l, p_it)
     {
	/* item is not already in view */
	if (!eina_list_data_find_list(v_items, p_it))
	  {
	     v_it = _pan_item_add(v->span, p_it);

	     if (!v_it) continue;

	     v_it->pos = pos;

	     /* TODO no needed */
	     if (p_it == v->state->cur_item)
	       {
		  sd->cur_item = v_it;
		  v_it->selected = EINA_TRUE;
	       }

	     if (pos > first_vis && pos < last_vis)
	       update = EINA_TRUE;
	  }
	pos++;
     }

   sd->items = eina_list_sort(sd->items, eina_list_count(sd->items), _sort_cb);

   if (update || !last_vis || v->plugin != v->state->plugin)
     {
	v->plugin = v->state->plugin;
	
	sd->update = EINA_TRUE;
	_update_frame(v->span);
     }
   
   if (v_items) eina_list_free(v_items);
   
   v->tabs->update(v->tabs);

   sd->update_idler = NULL;
   
   return 0;
}

static int
_cb_key_down(Evry_View *view, const Ecore_Event_Key *ev)
{
   View *v = (View *) view;
   Smart_Data *sd = evas_object_smart_data_get(v->span);
   Eina_List *l = NULL, *ll;
   Item *it = NULL;

   if (!v->state->plugin)
     return 0;

   if ((ev->modifiers & ECORE_EVENT_MODIFIER_CTRL) &&
       (!strcmp(ev->key, "2")))
     {
	v->list_mode = v->list_mode ? EINA_FALSE : EINA_TRUE;
	v->zoom = 0;
	_clear_items(v->span);
	_update_frame(v->span);
	goto end;
     }
   else if ((ev->modifiers & ECORE_EVENT_MODIFIER_CTRL) &&
       ((!strcmp(ev->key, "plus")) ||
	(!strcmp(ev->key, "3"))))
     {
	v->zoom++;
	if (v->zoom > 2) v->zoom = 0;
	if (v->zoom == 2)
	  _clear_items(v->span);

	_update_frame(v->span);
	goto end;
     }

   if (v->tabs->key_down(v->tabs, ev))
     {
	_view_update(view);
	return 1;
     }

   if (sd->items)
     l = eina_list_data_find_list(sd->items, sd->cur_item);

   if (!v->list_mode && !evry_conf->cycle_mode)
     {
	if (!strcmp(ev->key, "Right"))
	  {
	     if (l && l->next)
	       it = l->next->data;

	     if (it)
	       {
		  _pan_item_select(v->span, it);
		  evry_item_select(v->state, it->item);
	       }
	     goto end;
	  }
	else if (!strcmp(ev->key, "Left"))
	  {
	     if (!sd->items) return 1;

	     if (l && l->prev)
	       it = l->prev->data;

	     if (it)
	       {
		  _pan_item_select(v->span, it);
		  evry_item_select(v->state, it->item);
	       }
	     goto end;
	  }
     }
   if (!strcmp(ev->key, "Down"))
     {
	if (!sd->items) return 1;

	if (!evry_conf->cycle_mode)
	  {
	     EINA_LIST_FOREACH(l, ll, it)
	       {
		  if (it->y > sd->cur_item->y &&
		      it->x >= sd->cur_item->x)
		    break;
	       }
	  }

	if (!it && l && l->next)
	  it = l->next->data;

	if (it)
	  {
	     _pan_item_select(v->span, it);
	     evry_item_select(v->state, it->item);
	  }
	goto end;
     }
   else if (!strcmp(ev->key, "Up"))
     {
	if (!sd->items) return 1;

	if (!evry_conf->cycle_mode)
	  {
	     for(ll = l; ll; ll = ll->prev)
	       {
		  it = ll->data;
		  
		  if (it->y < sd->cur_item->y &&
		      it->x <= sd->cur_item->x)
		    break;
	       }
	  }

	if (!it && l && l->prev)
	  it = l->prev->data;

	if (it)
	  {
	     _pan_item_select(v->span, it);
	     evry_item_select(v->state, it->item);
	  }
	goto end;
     }
   else if (!strcmp(ev->key, "Return"))
     {
	if (!v->list_mode)
	  {
	     if (evry_browse_item(NULL))
	       goto end;
	  }
     }
   
   return 0;

 end:
   return 1;
}

static Evry_View *
_view_create(Evry_View *view, const Evry_State *s, const Evas_Object *swallow)
{
   VIEW(parent, view);

   View *v;

   if (!s->plugin)
     return NULL;

   v = E_NEW(View, 1);
   v->view = *view;
   v->state = s;
   v->evas = evas_object_evas_get(swallow);

   if (parent->list_mode < 0)
     v->list_mode = evry_conf->view_mode ? 0 : 1;
   else
     v->list_mode = parent->list_mode;

   v->zoom = parent->zoom;
   
   v->bg = edje_object_add(v->evas);
   e_theme_edje_object_set(v->bg, "base/theme/widgets",
                           "e/modules/everything/thumbview/main/window");
   // scrolled thumbs
   v->span = _pan_add(v->evas);
   _pan_view_set(v->span, v);

   // the scrollframe holding the scrolled thumbs
   v->sframe = e_scrollframe_add(v->evas);
   e_scrollframe_custom_theme_set(v->sframe, "base/theme/widgets",
                                  "e/modules/everything/thumbview/main/scrollframe");
   e_scrollframe_extern_pan_set(v->sframe, v->span,
                                _pan_set, _pan_get, _pan_max_get,
                                _pan_child_size_get);
   edje_object_part_swallow(v->bg, "e.swallow.list", v->sframe);
   evas_object_show(v->sframe);
   evas_object_show(v->span);

   EVRY_VIEW(v)->o_list = v->bg;

   v->tabs = evry_tab_view_new(s, v->evas);
   v->view.o_bar = v->tabs->o_tabs;

   return EVRY_VIEW(v);
}

static void
_view_destroy(Evry_View *view)
{
   VIEW(v, view);

   evas_object_del(v->bg);
   evas_object_del(v->sframe);
   evas_object_del(v->span);

   evry_tab_view_free(v->tabs);

   E_FREE(v);
}

static Eina_Bool
_init(void)
{
   View *v = E_NEW(View, 1);
   
   v->view.id = EVRY_VIEW(v);
   v->view.name = "Icon View";
   v->view.create = &_view_create;
   v->view.destroy = &_view_destroy;
   v->view.update = &_view_update;
   v->view.clear = &_view_clear;
   v->view.cb_key_down = &_cb_key_down;

   v->list_mode = -1;
   
   evry_view_register(EVRY_VIEW(v), 1);

   view = v;

   view_types = eina_stringshare_add("FILE");

   return EINA_TRUE;
}

static void
_shutdown(void)
{
   eina_stringshare_del(view_types);
   evry_view_unregister(EVRY_VIEW(view));
   E_FREE(view);
}


EINA_MODULE_INIT(_init);
EINA_MODULE_SHUTDOWN(_shutdown);

