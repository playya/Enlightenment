/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

typedef struct _E_Smart_Data E_Smart_Data;
typedef struct _E_Table_Item E_Table_Item;

struct _E_Smart_Data
{ 
   Evas_Coord       x, y, w, h;
   Evas_Object     *obj;
   Evas_Object     *clip;
   int              frozen;
   unsigned char    changed : 1;
   unsigned char    homogenous : 1;
   Evas_List       *items;
   struct {
      Evas_Coord    w, h;
   } min, max;
   struct {
      double        x, y;
   } align;
   struct {
      int           cols, rows;
   } size;
}; 

struct _E_Table_Item
{
   E_Smart_Data    *sd;
   int              col, row, colspan, rowspan;
   unsigned char    fill_w : 1;
   unsigned char    fill_h : 1;
   unsigned char    expand_w : 1;
   unsigned char    expand_h : 1;
   struct {
      Evas_Coord    w, h;
   } min, max;
   struct {
      double        x, y;
   } align;
   Evas_Object     *obj;
};

/* local subsystem functions */
static E_Table_Item *_e_table_smart_adopt(E_Smart_Data *sd, Evas_Object *obj);
static void        _e_table_smart_disown(Evas_Object *obj);
static void        _e_table_smart_item_del_hook(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void        _e_table_smart_reconfigure(E_Smart_Data *sd);
static void        _e_table_smart_extents_calcuate(E_Smart_Data *sd);

static void _e_table_smart_init(void);
static void _e_table_smart_add(Evas_Object *obj);
static void _e_table_smart_del(Evas_Object *obj);
static void _e_table_smart_layer_set(Evas_Object *obj, int layer);
static void _e_table_smart_raise(Evas_Object *obj);
static void _e_table_smart_lower(Evas_Object *obj);
static void _e_table_smart_stack_above(Evas_Object *obj, Evas_Object * above);
static void _e_table_smart_stack_below(Evas_Object *obj, Evas_Object * below);
static void _e_table_smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y);
static void _e_table_smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h);
static void _e_table_smart_show(Evas_Object *obj);
static void _e_table_smart_hide(Evas_Object *obj);
static void _e_table_smart_color_set(Evas_Object *obj, int r, int g, int b, int a);
static void _e_table_smart_clip_set(Evas_Object *obj, Evas_Object *clip);
static void _e_table_smart_clip_unset(Evas_Object *obj);

/* local subsystem globals */
static Evas_Smart *_e_smart = NULL;

/* externally accessible functions */
Evas_Object *
e_table_add(Evas *evas)
{
   _e_table_smart_init();
   return evas_object_smart_add(evas, _e_smart);
}

int
e_table_freeze(Evas_Object *obj)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   sd->frozen++;
   return sd->frozen;
}

int
e_table_thaw(Evas_Object *obj)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   sd->frozen--;
   if (sd->frozen <= 0) _e_table_smart_reconfigure(sd);
   return sd->frozen;
}

void
e_table_homogenous_set(Evas_Object *obj, int homogenous)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (sd->homogenous == homogenous) return;
   sd->homogenous = homogenous;
   sd->changed = 1;
   if (sd->frozen <= 0) _e_table_smart_reconfigure(sd);
}

void
e_table_pack(Evas_Object *obj, Evas_Object *child, int col, int row, int colspan, int rowspan)
{
   E_Smart_Data *sd;
   E_Table_Item *ti;
   
   sd = evas_object_smart_data_get(obj);
   _e_table_smart_adopt(sd, child);
   sd->items = evas_list_append(sd->items, child);
   ti = evas_object_data_get(obj, "e_table_data");
   if (ti)
     {
	ti->col = col;
	ti->row = row;
	ti->colspan = colspan;
	ti->rowspan = rowspan;
	if (sd->size.cols < (col + colspan)) sd->size.cols = col + colspan;
	if (sd->size.rows < (row + rowspan)) sd->size.rows = row + rowspan;
     }
   sd->changed = 1;
   if (sd->frozen <= 0) _e_table_smart_reconfigure(sd);
}

void
e_table_pack_options_set(Evas_Object *obj, int fill_w, int fill_h, int expand_w, int expand_h, double align_x, double align_y, Evas_Coord min_w, Evas_Coord min_h, Evas_Coord max_w, Evas_Coord max_h)
{
   E_Table_Item *ti;
   
   ti = evas_object_data_get(obj, "e_table_data");
   if (!ti) return;
   ti->fill_w = fill_w;
   ti->fill_h = fill_h;
   ti->expand_w = expand_w;
   ti->expand_h = expand_h;
   ti->align.x = align_x;
   ti->align.y = align_y;
   ti->min.w = min_w;
   ti->min.h = min_h;
   ti->max.w = max_w;
   ti->max.h = max_h;
   ti->sd->changed = 1;
   if (ti->sd->frozen <= 0) _e_table_smart_reconfigure(ti->sd);
}

void
e_table_unpack(Evas_Object *obj)
{
   E_Table_Item *ti;
   E_Smart_Data *sd;
   
   ti = evas_object_data_get(obj, "e_table_data");
   if (!ti) return;
   sd = ti->sd;
   sd->items = evas_list_remove(sd->items, obj);
   _e_table_smart_disown(obj);
   sd->changed = 1;
   if (sd->frozen <= 0) _e_table_smart_reconfigure(sd);
}

void
e_table_col_row_size_get(Evas_Object *obj, int *cols, int *rows)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (sd->changed) _e_table_smart_extents_calcuate(sd);
   if (cols) *cols = sd->size.cols;
   if (rows) *rows = sd->size.rows;
}

void
e_table_min_size_get(Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (sd->changed) _e_table_smart_extents_calcuate(sd);
   if (minw) *minw = sd->min.w;
   if (minh) *minh = sd->min.h;
}

void
e_table_max_size_get(Evas_Object *obj, Evas_Coord *maxw, Evas_Coord *maxh)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (sd->changed) _e_table_smart_extents_calcuate(sd);
   if (maxw) *maxw = sd->max.w;
   if (maxh) *maxh = sd->max.h;
}

void
e_table_align_get(Evas_Object *obj, double *ax, double *ay)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (ax) *ax = sd->align.x;
   if (ay) *ay = sd->align.y;
}

void
e_table_align_set(Evas_Object *obj, double ax, double ay)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if ((sd->align.x == ax) && (sd->align.y == ay)) return;
   sd->align.x = ax;
   sd->align.y = ay;
   sd->changed = 1;
   if (sd->frozen <= 0) _e_table_smart_reconfigure(sd);
}

/* local subsystem functions */
static E_Table_Item *
_e_table_smart_adopt(E_Smart_Data *sd, Evas_Object *obj)
{
   E_Table_Item *ti;
   
   ti = calloc(1, sizeof(E_Table_Item));
   if (!ti) return NULL;
   ti->sd = sd;
   ti->obj = obj;
   /* defaults */
   ti->col = 0;
   ti->row = 0;
   ti->colspan = 1;
   ti->rowspan = 1;
   ti->fill_w = 0;
   ti->fill_h = 0;
   ti->expand_w = 0;
   ti->expand_h = 0;
   ti->align.x = 0.5;
   ti->align.y = 0.5;
   ti->min.w = 0;
   ti->min.h = 0;
   ti->max.w = 0;
   ti->max.h = 0;
   evas_object_clip_set(obj, sd->clip);
   evas_object_stack_above(obj, sd->obj);
   evas_object_smart_member_add(ti->sd->obj, obj);
   evas_object_data_set(obj, "e_table_data", ti);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_FREE,
				  _e_table_smart_item_del_hook, NULL);
   evas_object_stack_below(obj, sd->obj);
   if (!evas_object_visible_get(sd->clip))
     evas_object_show(sd->clip);
   return ti;
}

static void
_e_table_smart_disown(Evas_Object *obj)
{
   E_Table_Item *ti;
   
   ti = evas_object_data_get(obj, "e_table_data");
   if (!ti) return;
   if (!ti->sd->items)
     {
	if (evas_object_visible_get(ti->sd->clip))
	  evas_object_hide(ti->sd->clip);
     }
   evas_object_event_callback_del(obj,
				  EVAS_CALLBACK_FREE,
				  _e_table_smart_item_del_hook);
   evas_object_smart_member_del(ti->sd->obj);
   evas_object_data_del(obj, "e_table_data");
   free(ti);
}

static void
_e_table_smart_item_del_hook(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   e_table_unpack(obj);
}

static void
_e_table_smart_reconfigure(E_Smart_Data *sd)
{
   Evas_Coord x, y, w, h, xx, yy;
   Evas_List *l;
   int minw, minh, expandw, expandh;

   if (!sd->changed) return;
   
   x = sd->x;
   y = sd->y;
   w = sd->w;
   h = sd->h;

   _e_table_smart_extents_calcuate(sd);
   
   minw = sd->min.w;
   minh = sd->min.h;
   expandw = 0;
   expandh = 0;
   if (w < minw)
     {
	x = x + ((w - minw) * (1.0 - sd->align.x));
	w = minw;
     }
   if (h < minh)
     {
	y = y + ((h - minh) * (1.0 - sd->align.y));
	h = minh;
     }
   for (l = sd->items; l; l = l->next)
     {
	E_Table_Item *ti;
	Evas_Object *obj;
	
	obj = l->data;
	ti = evas_object_data_get(obj, "e_table_data");
	if (ti->expand_w) expandw++;
	if (ti->expand_w) expandh++;
     }
   if (expandw == 0)
     {
	x += (w - minw) / 2;
	w = minw;
     }
   if (expandh == 0)
     {
	y += (h - minh) / 2;
	h = minh;
     }
   xx = x;
   yy = y;
   for (l = sd->items; l; l = l->next)
     {
	E_Table_Item *ti;
	Evas_Object *obj;
	
	obj = l->data;
	ti = evas_object_data_get(obj, "e_table_data");
	if (sd->homogenous)
	  {
	     Evas_Coord ww, hh, ow, oh;
	     
	     xx = x + ((ti->col) * (w / (Evas_Coord)sd->size.cols));
	     yy = y + ((ti->row) * (h / (Evas_Coord)sd->size.rows));
	     ww = ((w / (Evas_Coord)sd->size.cols) * (ti->colspan));
	     hh = ((h / (Evas_Coord)sd->size.rows) * (ti->rowspan));
	     ow = ti->min.w;
	     if (ti->expand_w) ow = ww;
	     if ((ti->max.w >= 0) && (ti->max.w < ow)) ow = ti->max.w;
	     oh = ti->min.h;
	     if (ti->expand_h) oh = hh;
	     if ((ti->max.h >= 0) && (ti->max.h < oh)) oh = ti->max.h;
	     evas_object_move(obj, 
			      xx + (Evas_Coord)(((double)(ww - ow)) * ti->align.x),
			      yy + (Evas_Coord)(((double)(hh - oh)) * ti->align.y));
	     evas_object_resize(obj, ow, oh);
	  }
	else
	  {
	     /* FIXME: not done - this is fucked atm */
	  }
     }
   sd->changed = 0;
}

static void
_e_table_smart_extents_calcuate(E_Smart_Data *sd)
{
   Evas_List *l;
   int minw, minh;

   /* FIXME: need to calc max */
   sd->max.w = -1; /* max < 0 == unlimited */
   sd->max.h = -1;
   sd->size.cols = 0;
   sd->size.rows = 0;
   
   minw = 0;
   minh = 0;
   if (sd->homogenous)
     {
	for (l = sd->items; l; l = l->next)
	  {
	     E_Table_Item *ti;
	     Evas_Object *obj;
	     
	     obj = l->data;
	     ti = evas_object_data_get(obj, "e_table_data");	
	     if (sd->size.cols < (ti->col + ti->colspan))
	       sd->size.cols = ti->col + ti->colspan;
	     if (sd->size.rows < (ti->row + ti->rowspan))
	       sd->size.rows = ti->row + ti->rowspan;
	     /* FIXME: does not handle colspan or rowspan > 1 */
	     if (minw < ti->min.w) minw = ti->min.w;
	     if (minh < ti->min.h) minh = ti->min.h;
	  }
	minw *= sd->size.cols;
	minh *= sd->size.rows;
     }
   else
     {
	/* FIXME: non homogenous does not work */
     }
   sd->min.w = minw;
   sd->min.h = minh;
}

static void
_e_table_smart_init(void)
{
   if (_e_smart) return;
   _e_smart = evas_smart_new("e_table",
			     _e_table_smart_add,
			     _e_table_smart_del,
			     _e_table_smart_layer_set,
			     _e_table_smart_raise,
			     _e_table_smart_lower,
			     _e_table_smart_stack_above,
			     _e_table_smart_stack_below,
			     _e_table_smart_move,
			     _e_table_smart_resize,
			     _e_table_smart_show,
			     _e_table_smart_hide,
			     _e_table_smart_color_set,
			     _e_table_smart_clip_set,
			     _e_table_smart_clip_unset,
			     NULL);
}

static void
_e_table_smart_add(Evas_Object *obj)
{
   E_Smart_Data *sd;
   
   sd = calloc(1, sizeof(E_Smart_Data));
   if (!sd) return;
   sd->obj = obj;
   sd->x = 0;
   sd->y = 0;
   sd->w = 0;
   sd->h = 0;
   sd->clip = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_smart_member_add(obj, sd->clip);
   evas_object_move(sd->clip, -100000, -100000);
   evas_object_resize(sd->clip, 200000, 200000);
   evas_object_color_set(sd->clip, 255, 255, 255, 255);
   evas_object_smart_data_set(obj, sd);
}
   
static void
_e_table_smart_del(Evas_Object *obj)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   while (sd->items)
     {
	Evas_Object *child;
	
	child = sd->items->data;
	e_table_unpack(child);
     }
   evas_object_del(sd->clip);
   free(sd);
}
   
static void
_e_table_smart_layer_set(Evas_Object *obj, int layer)
{
   E_Smart_Data *sd;
      
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   
     {
	Evas_List *l;
	
	for (l = sd->items; l; l = l->next)
	  {
	     evas_object_layer_set(l->data, layer);
	  }
     }
}

static void
_e_table_smart_raise(Evas_Object *obj)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;

     {
	Evas_List *l;
	
	for (l = evas_list_last(sd->items); l; l = l->prev)
	  {
	     evas_object_raise(l->data);
	  }
     }
}

static void
_e_table_smart_lower(Evas_Object *obj)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return; 
   
     {
	Evas_List *l;
	
	for (l = sd->items; l; l = l->next)
	  {
	     evas_object_lower(l->data);
	  }
     }
}
                                                             
static void
_e_table_smart_stack_above(Evas_Object *obj, Evas_Object *above)
{
   E_Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;

     {
	Evas_List *l;
	
	for (l = sd->items; l; l = l->next)
	  {
	     evas_object_stack_above(l->data, above);
	  }
     }
}
   
static void
_e_table_smart_stack_below(Evas_Object *obj, Evas_Object *below)
{
   E_Smart_Data *sd;
      
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;

     {
	Evas_List *l;
	
	for (l = evas_list_last(sd->items); l; l = l->prev)
	  {
	     evas_object_stack_below(l->data, below);
	  }
     }
}

static void
_e_table_smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if ((x == sd->x) && (y == sd->y)) return;
   if ((x == sd->x) && (y == sd->y)) return;
     {
	Evas_List *l;
	Evas_Coord dx, dy;
	
	dx = x - sd->x;
	dy = y - sd->y;
	for (l = sd->items; l; l = l->next)
	  {
	     Evas_Coord ox, oy;
	     
	     evas_object_geometry_get(l->data, &ox, &oy, NULL, NULL);
	     evas_object_move(l->data, ox + dx, oy + dy);
	  }
     }
   sd->x = x;
   sd->y = y;
}

static void
_e_table_smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if ((w == sd->w) && (h == sd->h)) return;
   sd->w = w;
   sd->h = h;
   sd->changed = 1;
   _e_table_smart_reconfigure(sd);
}

static void
_e_table_smart_show(Evas_Object *obj)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (sd->items) evas_object_show(sd->clip);
}

static void
_e_table_smart_hide(Evas_Object *obj)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_hide(sd->clip);
}

static void
_e_table_smart_color_set(Evas_Object *obj, int r, int g, int b, int a)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;   
   evas_object_color_set(sd->clip, r, g, b, a);
}

static void
_e_table_smart_clip_set(Evas_Object *obj, Evas_Object *clip)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_clip_set(sd->clip, clip);
}

static void
_e_table_smart_clip_unset(Evas_Object *obj)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_clip_unset(sd->clip);
}  
