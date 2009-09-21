#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Smart_Data Smart_Data;
typedef struct _Box_Item   Box_Item;

struct _Smart_Data
{ 
   Evas_Coord       x, y, w, h;
   Evas_Object     *obj;
   Evas_Object     *clip;
   Eina_Bool        changed : 1;
   Eina_Bool        horizontal : 1;
   Eina_Bool        homogenous : 1;
   Eina_Bool        deleting : 1;
   Eina_List       *items;
}; 

/* local subsystem functions */
static void        _smart_adopt(Smart_Data *sd, Evas_Object *obj);
static void        _smart_disown(Evas_Object *obj);
static void        _smart_item_del_hook(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void        _smart_item_changed_size_hints_hook(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void        _smart_reconfigure(Smart_Data *sd);
static void        _smart_extents_calculate(Smart_Data *sd);

static void _smart_init(void);
static void _smart_add(Evas_Object *obj);
static void _smart_del(Evas_Object *obj);
static void _smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y);
static void _smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h);
static void _smart_show(Evas_Object *obj);
static void _smart_hide(Evas_Object *obj);
static void _smart_color_set(Evas_Object *obj, int r, int g, int b, int a);
static void _smart_clip_set(Evas_Object *obj, Evas_Object *clip);
static void _smart_clip_unset(Evas_Object *obj);

/* local subsystem globals */
static Evas_Smart *_e_smart = NULL;

/* externally accessible functions */
Evas_Object *
_els_smart_box_add(Evas *evas)
{
   _smart_init();
   return evas_object_smart_add(evas, _e_smart);
}

void
_els_smart_box_orientation_set(Evas_Object *obj, int horizontal)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (sd->horizontal == horizontal) return;
   sd->horizontal = horizontal;
   _smart_reconfigure(sd);
}

int
_els_smart_box_orientation_get(Evas_Object *obj)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return 0;
   return sd->horizontal;
}

void
_els_smart_box_homogenous_set(Evas_Object *obj, int homogenous)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (sd->homogenous == homogenous) return;
   sd->homogenous = homogenous;
   _smart_reconfigure(sd);
}

int
_els_smart_box_pack_start(Evas_Object *obj, Evas_Object *child)
{
   Smart_Data *sd;
   
   if (!child) return 0;
   sd = evas_object_smart_data_get(obj);
   if (!sd) return 0;
   _smart_adopt(sd, child);
   sd->items = eina_list_prepend(sd->items, child);
   _smart_reconfigure(sd);
   return 0;
}

int
_els_smart_box_pack_end(Evas_Object *obj, Evas_Object *child)
{
   Smart_Data *sd;
   
   if (!child) return 0;
   sd = evas_object_smart_data_get(obj);
   if (!sd) return 0;
   _smart_adopt(sd, child);
   sd->items = eina_list_append(sd->items, child);   
   _smart_reconfigure(sd);
   return eina_list_count(sd->items) - 1;
}

static int
_els_smart_box_find(const Smart_Data *sd, const Evas_Object *child)
{
   int i = 0;
   const Eina_List *l;
   const Evas_Object *oitr;
   EINA_LIST_FOREACH(sd->items, l, oitr)
     {
	if (oitr == child)
	  return i;
	i++;
     }
   return -1;
}

int
_els_smart_box_pack_before(Evas_Object *obj, Evas_Object *child, Evas_Object *before)
{
   Smart_Data *sd;

   if (!child) return 0;
   sd = evas_object_smart_data_get(obj);
   if (!sd) return 0;
   _smart_adopt(sd, child);
   sd->items = eina_list_prepend_relative(sd->items, child, before);
   _smart_reconfigure(sd);

   return _els_smart_box_find(sd, child);
}

int
_els_smart_box_pack_after(Evas_Object *obj, Evas_Object *child, Evas_Object *after)
{
   Smart_Data *sd;

   if (!child) return 0;
   sd = evas_object_smart_data_get(obj);
   if (!sd) return 0;
   _smart_adopt(sd, child);
   sd->items = eina_list_append_relative(sd->items, child, after);
   _smart_reconfigure(sd);
   return _els_smart_box_find(sd, child);
}

void
_els_smart_box_unpack(Evas_Object *obj, Evas_Object *child)
{
   Smart_Data *sd;
   
   if (!obj) return;
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   sd->items = eina_list_remove(sd->items, child);
   elm_widget_sub_object_del(obj, child);
   _smart_disown(child);
   if (!sd->deleting)
     {
        if (!evas_object_clipees_get(sd->clip))
          evas_object_hide(sd->clip);
        _smart_reconfigure(sd);
     }
}

void
_els_smart_box_unpack_all(Evas_Object *obj)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   while (sd->items)
     {
	Evas_Object *child = sd->items->data;
        _els_smart_box_unpack(obj, child);
     }
}

void
_els_smart_box_clear(Evas_Object *obj)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   while (sd->items)
     {
	Evas_Object *child = sd->items->data;
        evas_object_del(child);
     }
}

/* local subsystem functions */
static void
_smart_adopt(Smart_Data *sd, Evas_Object *obj)
{
   evas_object_clip_set(obj, sd->clip);
   evas_object_smart_member_add(obj, sd->obj);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL,
				  _smart_item_del_hook, NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS, 
				  _smart_item_changed_size_hints_hook, NULL);
   if ((!evas_object_visible_get(sd->clip)) &&
       (evas_object_visible_get(sd->obj)))
     evas_object_show(sd->clip);
}

static void
_smart_disown(Evas_Object *obj)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(evas_object_smart_parent_get(obj));
   if (!sd) return;
   if (sd->items)
     {
	if (evas_object_visible_get(sd->clip))
	  evas_object_hide(sd->clip);
     }
   evas_object_event_callback_del(obj, EVAS_CALLBACK_DEL,
				  _smart_item_del_hook);
   evas_object_event_callback_del(obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS, 
				  _smart_item_changed_size_hints_hook);
   evas_object_smart_member_del(obj);
   evas_object_clip_unset(obj);
}

static void
_smart_item_del_hook(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   _els_smart_box_unpack(evas_object_smart_parent_get(obj), obj);
}

static void
_smart_item_changed_size_hints_hook(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(evas_object_smart_parent_get(obj));
   _smart_reconfigure(sd);
}

static void
_smart_reconfigure(Smart_Data *sd)
{
   Evas_Coord x, y, w, h, xx, yy;
   const Eina_List *l;
   Evas_Object *obj;
   Evas_Coord minw, minh, wdif, hdif, mnw, mnh, mxw, mxh;
   int count, expand, fw, fh, xw, xh;
   double ax, ay, wx, wy;

   _smart_extents_calculate(sd);
   
   x = sd->x;
   y = sd->y;
   w = sd->w;
   h = sd->h;

   evas_object_size_hint_min_get(sd->obj, &minw, &minh);
   evas_object_size_hint_align_get(sd->obj, &ax, &ay);
   count = eina_list_count(sd->items);
   expand = 0;
   if (w < minw)
     {
	x = x + ((w - minw) * (1.0 - ax));
	w = minw;
     }
   if (h < minh)
     {
	y = y + ((h - minh) * (1.0 - ay));
	h = minh;
     }
   EINA_LIST_FOREACH(sd->items, l, obj)
     {
	evas_object_size_hint_weight_get(obj, &wx, &wy);
	if (sd->horizontal)
	  {
	     if (wx > 0.0) expand++;
	  }
	else
	  {
	     if (wy > 0.0) expand++;
	  }
     }
   if (expand == 0)
     {
	evas_object_size_hint_align_get(sd->obj, &ax, &ay);
	if (sd->horizontal)
	  {
	     x += (double)(w - minw) * ax;
	     w = minw;
	  }
	else
	  {
	     y += (double)(h - minh) * ay;
	     h = minh;
	  }
     }
   wdif = w - minw;
   hdif = h - minh;
   xx = x;
   yy = y;
   EINA_LIST_FOREACH(sd->items, l, obj)
     {
	evas_object_size_hint_align_get(obj, &ax, &ay);
	evas_object_size_hint_weight_get(obj, &wx, &wy);
	evas_object_size_hint_min_get(obj, &mnw, &mnh);
	evas_object_size_hint_max_get(obj, &mxw, &mxh);
	fw = fh = 0;
	xw = xh = 0;
	if (ax == -1.0) {fw = 1; ax = 0.5;}
	if (ay == -1.0) {fh = 1; ay = 0.5;}
	if (wx > 0.0) xw = 1;
	if (wy > 0.0) xh = 1;
	if (sd->horizontal)
	  {
	     if (sd->homogenous)
	       {
		  Evas_Coord ww, hh, ow, oh;
		  
		  ww = (w / (Evas_Coord)count);
		  hh = h;
		  ow = mnw;
		  if (fw) ow = ww;
		  if ((mxw >= 0) && (mxw < ow)) 
		    ow = mxw;
		  oh = mnh;
		  if (fh) oh = hh;
		  if ((mxh >= 0) && (mxh < oh)) 
		    oh = mxh;
		  evas_object_move(obj, 
				   xx + (Evas_Coord)(((double)(ww - ow)) * ax),
				   yy + (Evas_Coord)(((double)(hh - oh)) * ay));
		  evas_object_resize(obj, ow, oh);
		  xx += ww;
	       }
	     else
	       {
		  Evas_Coord ww, hh, ow, oh;
		  
		  ww = mnw;
		  if ((expand > 0) && (xw))
		    {
		       if (expand == 1) ow = wdif;
		       else ow = (w - minw) / expand;
		       wdif -= ow;
		       ww += ow;
		    }
		  hh = h;
		  ow = mnw;
		  if (fw) ow = ww;
		  if ((mxw >= 0) && (mxw < ow)) ow = mxw;
		  oh = mnh;
		  if (fh) oh = hh;
		  if ((mxh >= 0) && (mxh < oh)) oh = mxh;
		  evas_object_move(obj, 
				   xx + (Evas_Coord)(((double)(ww - ow)) * ax),
				   yy + (Evas_Coord)(((double)(hh - oh)) * ay));
		  evas_object_resize(obj, ow, oh);
		  xx += ww;
	       }
	  }
	else
	  {
	     if (sd->homogenous)
	       {
		  Evas_Coord ww, hh, ow, oh;
		  
		  ww = w;
		  hh = (h / (Evas_Coord)count);
		  ow = mnw;
		  if (fw) ow = ww;
		  if ((mxw >= 0) && (mxw < ow)) ow = mxw;
		  oh = mnh;
		  if (fh) oh = hh;
		  if ((mxh >= 0) && (mxh < oh)) oh = mxh;
		  evas_object_move(obj, 
				   xx + (Evas_Coord)(((double)(ww - ow)) * ax),
				   yy + (Evas_Coord)(((double)(hh - oh)) * ay));
		  evas_object_resize(obj, ow, oh);
		  yy += hh;
	       }
	     else
	       {
		  Evas_Coord ww, hh, ow, oh;
		  
		  ww = w;
		  hh = mnh;
		  if ((expand > 0) && (xh))
		    {
		       if (expand == 1) oh = hdif;
		       else oh = (h - minh) / expand;
		       hdif -= oh;
		       hh += oh;
		    }
		  ow = mnw;
		  if (fw) ow = ww;
		  if ((mxw >= 0) && (mxw < ow)) ow = mxw;
		  oh = mnh;
		  if (fh) oh = hh;
		  if ((mxh >= 0) && (mxh < oh)) oh = mxh;
		  evas_object_move(obj, 
				   xx + (Evas_Coord)(((double)(ww - ow)) * ax),
				   yy + (Evas_Coord)(((double)(hh - oh)) * ay));
		  evas_object_resize(obj, ow, oh);
		  yy += hh;
	       }
	  }
     }
}

static void
_smart_extents_calculate(Smart_Data *sd)
{
   Evas_Coord minw, minh, maxw, maxh, mnw, mnh;

   /* FIXME: need to calc max */
   minw = 0;
   minh = 0;
   maxw = -1;
   maxh = -1;
   if (sd->homogenous)
     {
	const Eina_List *l;
	const Evas_Object *obj;
	EINA_LIST_FOREACH(sd->items, l, obj)
	  {
	     evas_object_size_hint_min_get(obj, &mnw, &mnh);
	     if (minh < mnh) minh = mnh;
	     if (minw < mnw) minw = mnw;
	  }
	if (sd->horizontal)
          minw *= eina_list_count(sd->items);	     
	else
          minh *= eina_list_count(sd->items);	     
     }
   else
     {
	const Eina_List *l;
	const Evas_Object *obj;
	EINA_LIST_FOREACH(sd->items, l, obj)
	  {
	     evas_object_size_hint_min_get(obj, &mnw, &mnh);
	     if (sd->horizontal)
	       {
		  if (minh < mnh) minh = mnh;
		  minw += mnw;
	       }
	     else
	       {
		  if (minw < mnw) minw = mnw;
		  minh += mnh;
	       }
	  }
     }
   evas_object_size_hint_min_set(sd->obj, minw, minh);
}

static void
_smart_init(void)
{
   if (_e_smart) return;
     {
	static const Evas_Smart_Class sc =
	  {
	     "els_box",
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
	_e_smart = evas_smart_class_new(&sc);
     }
}

static void
_smart_add(Evas_Object *obj)
{
   Smart_Data *sd;
   
   sd = calloc(1, sizeof(Smart_Data));
   if (!sd) return;
   sd->obj = obj;
   sd->clip = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_smart_member_add(sd->clip, obj);
   evas_object_move(sd->clip, -100004, -100004);
   evas_object_resize(sd->clip, 200008, 200008);
   evas_object_color_set(sd->clip, 255, 255, 255, 255);
   evas_object_smart_data_set(obj, sd);
}
   
static void
_smart_del(Evas_Object *obj)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   sd->deleting = EINA_TRUE;
   while (sd->items)
     {
	Evas_Object *child;
	
	child = sd->items->data;
	_els_smart_box_unpack(obj, child);
     }
   evas_object_del(sd->clip);
   free(sd);
   evas_object_smart_data_set(obj, NULL);
}

static void
_smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   Smart_Data *sd;
   const Eina_List *l;
   Evas_Object *child;
   Evas_Coord dx, dy;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   dx = x - sd->x;
   dy = y - sd->y;
   sd->x = x;
   sd->y = y;
   EINA_LIST_FOREACH(sd->items, l, child)
     {
	Evas_Coord ox, oy;

	evas_object_geometry_get(child, &ox, &oy, NULL, NULL);
	evas_object_move(child, ox + dx, oy + dy);
     }
}

static void
_smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   sd->w = w;
   sd->h = h;
   _smart_reconfigure(sd);
}

static void
_smart_show(Evas_Object *obj)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (sd->items) evas_object_show(sd->clip);
}

static void
_smart_hide(Evas_Object *obj)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_hide(sd->clip);
}

static void
_smart_color_set(Evas_Object *obj, int r, int g, int b, int a)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;   
   evas_object_color_set(sd->clip, r, g, b, a);
}

static void
_smart_clip_set(Evas_Object *obj, Evas_Object *clip)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_clip_set(sd->clip, clip);
}

static void
_smart_clip_unset(Evas_Object *obj)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_clip_unset(sd->clip);
}  
