/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

typedef struct _E_Smart_Data E_Smart_Data;

struct _E_Smart_Data
{ 
   Evas_Coord   x, y, w, h;
   Evas_Object *obj;
   int          size;
   unsigned char fill_inside : 1;
   unsigned char scale_up : 1;
   unsigned char preload : 1;
   unsigned char loading : 1;
}; 

/* local subsystem functions */
static void _e_icon_smart_reconfigure(E_Smart_Data *sd);
static void _e_icon_smart_init(void);
static void _e_icon_smart_add(Evas_Object *obj);
static void _e_icon_smart_del(Evas_Object *obj);
static void _e_icon_smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y);
static void _e_icon_smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h);
static void _e_icon_smart_show(Evas_Object *obj);
static void _e_icon_smart_hide(Evas_Object *obj);
static void _e_icon_smart_color_set(Evas_Object *obj, int r, int g, int b, int a);
static void _e_icon_smart_clip_set(Evas_Object *obj, Evas_Object * clip);
static void _e_icon_smart_clip_unset(Evas_Object *obj);
static void _e_icon_obj_prepare(Evas_Object *obj, E_Smart_Data *sd);
static void _e_icon_preloaded(void *data, Evas *e, Evas_Object *obj, void *event_info);

/* local subsystem globals */
static Evas_Smart *_e_smart = NULL;

/* externally accessible functions */
EAPI Evas_Object *
e_icon_add(Evas *evas)
{
   _e_icon_smart_init();
   return evas_object_smart_add(evas, _e_smart);
}

static void
_e_icon_obj_prepare(Evas_Object *obj, E_Smart_Data *sd)
{
   if (!sd->obj) return;
   
   if (!strcmp(evas_object_type_get(sd->obj), "edje"))
     {
        evas_object_del(sd->obj);
        sd->obj = evas_object_image_add(evas_object_evas_get(obj));
        evas_object_event_callback_add(sd->obj, EVAS_CALLBACK_IMAGE_PRELOADED,
                                       _e_icon_preloaded, obj);
        evas_object_smart_member_add(sd->obj, obj);
     }
}

EAPI void
e_icon_file_set(Evas_Object *obj, const char *file)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   /* smart code here */
   _e_icon_obj_prepare(obj, sd);
   /* FIXME: 64x64 - unhappy about this. use icon size */
   sd->loading = 0;
   if (sd->size != 0)
     evas_object_image_load_size_set(sd->obj, sd->size, sd->size);
   if (sd->preload)
     evas_object_hide(sd->obj);
   evas_object_image_file_set(sd->obj, file, NULL);
   if (sd->preload)
     {
        sd->loading = 1;
        evas_object_image_preload(sd->obj, 0);
     }
   else if (evas_object_visible_get(obj))
     evas_object_show(sd->obj);
   _e_icon_smart_reconfigure(sd);
}

EAPI void
e_icon_file_key_set(Evas_Object *obj, const char *file, const char *key)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   /* smart code here */
   sd->loading = 0;
   _e_icon_obj_prepare(obj, sd);
   if (sd->size != 0)
     evas_object_image_load_size_set(sd->obj, sd->size, sd->size);
   if (sd->preload)
     evas_object_hide(sd->obj);
   evas_object_image_file_set(sd->obj, file, key);
   if (sd->preload)
     {
        sd->loading = 1;
        evas_object_image_preload(sd->obj, 0);
     }
   else if (evas_object_visible_get(obj))
     evas_object_show(sd->obj);
   _e_icon_smart_reconfigure(sd);
}

EAPI void
e_icon_file_edje_set(Evas_Object *obj, const char *file, const char *part)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   /* smart code here */
   if (sd->obj) evas_object_del(sd->obj);
   sd->loading = 0;
   sd->obj = edje_object_add(evas_object_evas_get(obj));
   edje_object_file_set(sd->obj, file, part);
   if (evas_object_visible_get(obj))
     evas_object_show(sd->obj);
   evas_object_smart_member_add(sd->obj, obj);
   _e_icon_smart_reconfigure(sd);
}

EAPI void
e_icon_object_set(Evas_Object *obj, Evas_Object *o)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   /* smart code here */
   if (sd->obj) evas_object_del(sd->obj);
   sd->loading = 0;
   sd->obj = o;
   evas_object_smart_member_add(sd->obj, obj);
   if (evas_object_visible_get(obj))
     evas_object_show(sd->obj);
   _e_icon_smart_reconfigure(sd);   
}

EAPI const char *
e_icon_file_get(Evas_Object *obj)
{
   E_Smart_Data *sd;
   const char *file;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return NULL;
   if (!strcmp(evas_object_type_get(sd->obj), "edje"))
     {
	edje_object_file_get(sd->obj, &file, NULL);
	return file;
     }
   evas_object_image_file_get(sd->obj, &file, NULL);
   return file;
}

EAPI void
e_icon_smooth_scale_set(Evas_Object *obj, int smooth)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (!strcmp(evas_object_type_get(sd->obj), "edje"))
     return;
   evas_object_image_smooth_scale_set(sd->obj, smooth);
}

EAPI int
e_icon_smooth_scale_get(Evas_Object *obj)
{
   E_Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);   
   if (!sd) return 0;
   if (!strcmp(evas_object_type_get(sd->obj), "edje"))
     return 0;   
   return evas_object_image_smooth_scale_get(sd->obj);
}

EAPI void
e_icon_alpha_set(Evas_Object *obj, int alpha)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (!strcmp(evas_object_type_get(sd->obj), "edje"))
     return;   
   evas_object_image_alpha_set(sd->obj, alpha);
}

EAPI int
e_icon_alpha_get(Evas_Object *obj)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return 0;
   if (!strcmp(evas_object_type_get(sd->obj), "edje"))
     return 0;   
   return evas_object_image_alpha_get(sd->obj);
}

EAPI void
e_icon_preload_set(Evas_Object *obj, int preload)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   sd->preload = preload;
}

EAPI int
e_icon_preload_get(Evas_Object *obj)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return 0;
   return sd->preload;
}

EAPI void
e_icon_size_get(Evas_Object *obj, int *w, int *h)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_image_size_get(sd->obj, w, h);
}

EAPI int
e_icon_fill_inside_get(Evas_Object *obj)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (sd->fill_inside) return 1;
   return 0;
}

EAPI void
e_icon_fill_inside_set(Evas_Object *obj, int fill_inside)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (((sd->fill_inside) && (fill_inside)) ||
       ((!sd->fill_inside) && (!fill_inside))) return;
   sd->fill_inside = fill_inside;
   _e_icon_smart_reconfigure(sd);
}

EAPI int
e_icon_scale_up_get(Evas_Object *obj)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (sd->scale_up) return 1;
   return 0;
}

EAPI void
e_icon_scale_up_set(Evas_Object *obj, int scale_up)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (((sd->scale_up) && (scale_up)) ||
       ((!sd->scale_up) && (!scale_up))) return;
   sd->scale_up = scale_up;
   _e_icon_smart_reconfigure(sd);
}

EAPI void
e_icon_data_set(Evas_Object *obj, void *data, int w, int h)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (!strcmp(evas_object_type_get(sd->obj), "edje"))
     return;   
   evas_object_image_size_set(sd->obj, w, h);
   evas_object_image_data_copy_set(sd->obj, data);
}

EAPI void *
e_icon_data_get(Evas_Object *obj, int *w, int *h)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return NULL;
   if (!strcmp(evas_object_type_get(sd->obj), "edje"))
     return NULL;   
   evas_object_image_size_get(sd->obj, w, h);
   return evas_object_image_data_get(sd->obj, 0);
}

EAPI void
e_icon_scale_size_set(Evas_Object *obj, int size)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   sd->size = size;
   if (!strcmp(evas_object_type_get(sd->obj), "edje"))
     return;   
   evas_object_image_load_size_set(sd->obj, sd->size, sd->size);
}

EAPI int
e_icon_scale_size_get(Evas_Object *obj)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return 0;
   return sd->size;
}

/* local subsystem globals */
static void
_e_icon_smart_reconfigure(E_Smart_Data *sd)
{
   int iw, ih;
   Evas_Coord x, y, w, h;
   
   if (!sd->obj) return;
   if (!strcmp(evas_object_type_get(sd->obj), "edje"))
     {
	w = sd->w;
	h = sd->h;
	x = sd->x;
	y = sd->y;
	evas_object_move(sd->obj, x, y);
	evas_object_resize(sd->obj, w, h);
     }
   else
     {
	ih = 0;
	ih = 0;
	evas_object_image_size_get(sd->obj, &iw, &ih);
	if (iw < 1) iw = 1;
	if (ih < 1) ih = 1;
	
	if (sd->fill_inside)
	  {
	     w = sd->w;
	     h = ((double)ih * w) / (double)iw;
	     if (h > sd->h)
	       {
		  h = sd->h;
		  w = ((double)iw * h) / (double)ih;
	       }
	  }
	else
	  {
	     w = sd->w;
	     h = ((double)ih * w) / (double)iw;
	     if (h < sd->h)
	       {
		  h = sd->h;
		  w = ((double)iw * h) / (double)ih;
	       }	
	  }
	if (!sd->scale_up)
	  {
	     if ((w > iw) || (h > ih))
	       {
		  w = iw;
		  h = ih;
	       }
	  }
	x = sd->x + ((sd->w - w) / 2);
	y = sd->y + ((sd->h - h) / 2);
	evas_object_move(sd->obj, x, y);
	evas_object_image_fill_set(sd->obj, 0, 0, w, h);
	evas_object_resize(sd->obj, w, h);
     }
}

static void
_e_icon_smart_init(void)
{
   if (_e_smart) return;
     {
	static const Evas_Smart_Class sc =
	  {
	     "e_icon",
	       EVAS_SMART_CLASS_VERSION,
	       _e_icon_smart_add,
	       _e_icon_smart_del,
	       _e_icon_smart_move,
	       _e_icon_smart_resize,
	       _e_icon_smart_show,
	       _e_icon_smart_hide,
	       _e_icon_smart_color_set,
	       _e_icon_smart_clip_set,
	       _e_icon_smart_clip_unset,
	       NULL,
	       NULL,
	       NULL,
	       NULL
	  };
	_e_smart = evas_smart_class_new(&sc);
     }
}

static void
_e_icon_preloaded(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(data);
   evas_object_smart_callback_call(data, "preloaded", NULL);
   evas_object_show(sd->obj);
   sd->loading = 0;
}

static void
_e_icon_smart_add(Evas_Object *obj)
{
   E_Smart_Data *sd;
   
   sd = calloc(1, sizeof(E_Smart_Data));
   if (!sd) return;
   sd->obj = evas_object_image_add(evas_object_evas_get(obj));
   evas_object_event_callback_add(sd->obj, EVAS_CALLBACK_IMAGE_PRELOADED,
                                  _e_icon_preloaded, obj);
   sd->x = 0;
   sd->y = 0;
   sd->w = 0;
   sd->h = 0;
   sd->fill_inside = 1;
   sd->scale_up = 1;
   sd->size = 64;
   evas_object_smart_member_add(sd->obj, obj);
   evas_object_smart_data_set(obj, sd);
}
   
static void
_e_icon_smart_del(Evas_Object *obj)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_del(sd->obj);
   free(sd);
}

static void
_e_icon_smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if ((sd->x == x) && (sd->y == y)) return;
   sd->x = x;
   sd->y = y;
   _e_icon_smart_reconfigure(sd);
}

static void
_e_icon_smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if ((sd->w == w) && (sd->h == h)) return;
   sd->w = w;
   sd->h = h;
   _e_icon_smart_reconfigure(sd);
}

static void
_e_icon_smart_show(Evas_Object *obj)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (!((sd->preload) && (sd->loading)))
     evas_object_show(sd->obj);
}

static void
_e_icon_smart_hide(Evas_Object *obj)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_hide(sd->obj);
}

static void
_e_icon_smart_color_set(Evas_Object *obj, int r, int g, int b, int a)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_color_set(sd->obj, r, g, b, a);
}

static void
_e_icon_smart_clip_set(Evas_Object *obj, Evas_Object * clip)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_clip_set(sd->obj, clip);
}

static void
_e_icon_smart_clip_unset(Evas_Object *obj)
{
   E_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_clip_unset(sd->obj);
}  
