// vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2

#include "photo_object.h"
#include <locale.h>

typedef struct _Smart_Data Smart_Data;

struct _Smart_Data
{
   const char *theme_file;
   const char *theme_group;
   const char *photo_file;
   const char *photo_group;

   Evas_Object *obj, *image;
   Evas_Coord iw, ih;
   Evas_Coord w, h;

   int zoom;
   Eina_Bool camera;

   Eina_Bool progressbar;
   Evas_Object *o_progressbar;

   Evas_Object *flickr;

   Eina_Bool done : 1;
   Eina_Bool selected: 1;
};

#define E_SMART_OBJ_GET_RETURN(smart, o, type, ret) \
{ \
   char *_e_smart_str; \
   \
   if (!o) return ret; \
   smart = evas_object_smart_data_get(o); \
   if (!smart) return ret; \
   _e_smart_str = (char *)evas_object_type_get(o); \
   if (!_e_smart_str) return ret; \
   if (strcmp(_e_smart_str, type)) return ret; \
}

#define E_SMART_OBJ_GET(smart, o, type) \
{ \
   char *_e_smart_str; \
   \
   if (!o) return; \
   smart = evas_object_smart_data_get(o); \
   if (!smart) return; \
   _e_smart_str = (char *)evas_object_type_get(o); \
   if (!_e_smart_str) return; \
   if (strcmp(_e_smart_str, type)) return; \
}
// vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2

#define E_OBJ_NAME "photo_object"
static Evas_Smart  *smart = NULL;

static void _smart_init(void);
static void _smart_add(Evas_Object * obj);
static void _smart_del(Evas_Object * obj);
static void _smart_move(Evas_Object * obj, Evas_Coord x, Evas_Coord y);
static void _smart_resize(Evas_Object * obj, Evas_Coord w, Evas_Coord h);
static void _smart_show(Evas_Object * obj);
static void _smart_hide(Evas_Object * obj);
static void _smart_clip_set(Evas_Object * obj, Evas_Object * clip);
static void _smart_clip_unset(Evas_Object * obj);

static void _update(Evas_Object *obj);

static void _clicked_right(void *data, Evas_Object *obj, const char *signal, const char *source)
{
   Smart_Data *sd;
   Evas_Object *o = data;
   sd = evas_object_smart_data_get(o);
   if (!sd) return;


   if(!sd->selected)
     {
	sd->selected = EINA_TRUE;
	edje_object_signal_emit(sd->obj, "select,extern", "photo");
	evas_object_smart_callback_call(o, "select", NULL);
     }
   evas_object_smart_callback_call(o, "clicked,right", NULL);
}

static void _clicked_menu(void *data, Evas_Object *obj, const char *signal, const char *source)
{
   Evas_Object *o = data;
   evas_object_smart_callback_call(o, "clicked,menu", NULL);
}

void _unselect_cb(void *data, Evas_Object *obj, void *event_info)
{
   Smart_Data *sd;
   sd = evas_object_smart_data_get(data);
   if (!sd) return;

   if(sd->obj)
     {
	sd->selected = EINA_FALSE;
	edje_object_signal_emit(sd->obj, "unselect", "photo");
     }
}

void _select_cb(void *data, Evas_Object *obj, const char *signal, const char *source)
{
   Smart_Data *sd;
   Evas_Object *o = data;
   sd = evas_object_smart_data_get(o);
   if (!sd) return;

   sd->selected = EINA_TRUE;
   evas_object_smart_callback_call(o, "select", NULL);
}

void _open_cb(void *data, Evas_Object *obj, const char *signal, const char *source)
{
   Evas_Object *o = data;
   evas_object_smart_callback_call(o, "open", NULL);
}

void _select_extern_cb(void *data, Evas_Object *obj, void *event_info)
{
   Smart_Data *sd;
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;

   if(sd->obj)
     {
	sd->selected = EINA_TRUE;
	edje_object_signal_emit(sd->obj, "select,extern", "photo");
     }
}

Evas_Object *photo_object_add(Evas_Object *obj)
{
   _smart_init();
   return evas_object_smart_add(evas_object_evas_get(obj), smart);
}

void photo_object_theme_file_set(Evas_Object *obj, const char *theme, const char* theme_group)
{
   Smart_Data *sd;
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;

   sd->theme_file= eina_stringshare_add(theme);
   sd->theme_group = eina_stringshare_add(theme_group);

   sd->obj = NULL;

   _update(obj);
}

void photo_object_file_set(Evas_Object *obj, const char *image, const char *photo_group)
{
   Smart_Data *sd;
   sd = evas_object_smart_data_get(obj);
   Evas_Coord iw, ih;
   if (!sd) return;

   eina_stringshare_del(sd->photo_file);
   eina_stringshare_del(sd->photo_group);

   sd->photo_file = eina_stringshare_add(image);
   if(photo_group)
     sd->photo_group = eina_stringshare_add(photo_group);
   else
     sd->photo_group = NULL;

   if(sd->image)
     evas_object_del(sd->image);

   sd->image = evas_object_image_add(evas_object_evas_get(obj));
   evas_object_image_filled_set(sd->image, 1);
   evas_object_smart_member_add(obj, sd->image);
   evas_object_clip_set(sd->image, obj);
   evas_object_image_file_set(sd->image, NULL, NULL);
   evas_object_image_load_scale_down_set(sd->image, 0);
   evas_object_image_file_set(sd->image, image, photo_group);
   evas_object_image_size_get(sd->image, &iw, &ih);
   evas_object_show(sd->image);
   sd->iw = iw;
   sd->ih = ih;

   if(sd->obj)
     edje_object_part_swallow(sd->obj, "object.photo.swallow", sd->image);

   _update(obj);
}

void photo_object_progressbar_set(Evas_Object *obj, Eina_Bool b)
{
   Smart_Data *sd;
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;

   sd->progressbar = b;

   if(!b && sd->o_progressbar)
     evas_object_del(sd->o_progressbar);

   _update(obj);
}

void photo_object_done_set(Evas_Object *obj, Eina_Bool b)
{
   Smart_Data *sd;
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;

   sd->done = b;

   _update(obj);
}

void photo_object_radio_set(Evas_Object *obj, Eina_Bool b)
{
   Smart_Data *sd;
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;

   if(b)
     edje_object_signal_emit(sd->obj, "radio,on", "photo");
   else
     edje_object_signal_emit(sd->obj, "radio,off", "photo");
}

void photo_object_camera_set(Evas_Object *obj, Eina_Bool b)
{
   Smart_Data *sd;
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;

   sd->camera = b;
   if(b)
     edje_object_signal_emit(sd->obj, "show,camera", "photo");
   else
     edje_object_signal_emit(sd->obj, "hide,camera", "photo");
}

void photo_object_size_set(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   Smart_Data *sd;
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;

   sd->w = w;
   sd->h = h;
}

void photo_object_text_set(Evas_Object *obj, const char *s)
{
   Smart_Data *sd;
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;

   edje_object_part_text_set(sd->obj, "object.text", s);
}

Evas_Object *photo_object_flickr_state_set(Evas_Object *obj, const char* state)
{
   Smart_Data *sd;
   sd = evas_object_smart_data_get(obj);
   if (!sd) return NULL;

   if(!sd->flickr)
     {
	sd->flickr = edje_object_add(evas_object_evas_get(obj));
	evas_object_show(sd->flickr);
	edje_object_file_set(sd->flickr, PACKAGE_DATA_DIR"/theme.edj", "flickr/sync");
	evas_object_size_hint_weight_set(sd->flickr, 1.0, 1.0);
	evas_object_size_hint_align_set(sd->flickr, 1.0, 0.0);

	edje_object_part_swallow(sd->obj, "object.swallow.sync", sd->flickr);
     }

   edje_object_signal_emit(sd->flickr, state, "");
   return sd->flickr;
}

static void _update(Evas_Object *obj)
{
   int zoomw = 1, zoomh = 1;
   Smart_Data *sd;
   int x,y,w,h, w_img = 0, h_img = 0, w_img2, h_img2;
   int left_marge = 0, right_marge = 0, top_marge =  0, bottom_marge = 0;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;

   if(!sd->obj)
     {
	 if(!sd->theme_file || !sd->theme_group)
	   return ;

	 sd->obj= edje_object_add(evas_object_evas_get(obj));
	 edje_object_file_set(sd->obj, sd->theme_file, sd->theme_group);
	 evas_object_smart_member_add(sd->obj, obj);
	 evas_object_clip_set(sd->obj, obj);
	 evas_object_show(sd->obj);

	 edje_object_signal_callback_add(sd->obj, "clicked,right", "photo",
	       _clicked_right, obj);
	 edje_object_signal_callback_add(sd->obj, "clicked,menu", "photo",
	       _clicked_menu, obj);
	 edje_object_signal_callback_add(sd->obj, "select", "photo",
	       _select_cb, obj);
	 edje_object_signal_callback_add(sd->obj, "open", "photo",
	       _open_cb, obj);

	 if(sd->image)
	      edje_object_part_swallow(sd->obj, "object.photo.swallow", sd->image);
     }

   if(sd->done)
     edje_object_signal_emit(sd->obj, "done", "photo");
   else
     edje_object_signal_emit(sd->obj, "undone", "photo");

   if(sd->progressbar)
     {
	if(!sd->o_progressbar)
	  {
	     Evas_Object *loading = elm_progressbar_add(obj);
	     sd->o_progressbar = loading;
	     elm_object_style_set(loading, "wheel");
	     elm_progressbar_pulse(loading, EINA_TRUE);
	     evas_object_size_hint_weight_set(loading, 1.0, 0.0);
	     evas_object_size_hint_align_set(loading, -1.0, 0.5);
	     evas_object_show(loading);

	     edje_object_part_swallow(sd->obj, "object.loading.swallow", loading);
	     edje_object_signal_emit(sd->obj, "loading", "photo");
	  }
	return ;
     }

   if(sd->camera)
     edje_object_signal_emit(sd->obj, "show,camera", "photo");

   if(sd->w == -1)
      evas_object_geometry_get(sd->obj, &x, &y, &w, &h);
   else
   {
      w = sd->w;
      h = sd->h;
   }


   w_img = sd->iw;
   h_img = sd->ih;

   while(w_img/2>w)
     {
	 w_img = w_img/2;
	 zoomw++;
     }
   while(h_img/2>h)
     {
	 h_img = h_img/2;
	 zoomh++;
     }
   sd->zoom = (zoomw<zoomh?zoomw:zoomh);

   if(sd->w > -1)
     evas_object_image_load_size_set(sd->image, sd->w, sd->h);
   evas_object_image_file_set(sd->image, NULL, NULL);
   evas_object_image_load_scale_down_set(sd->image, sd->zoom);
   evas_object_image_file_set(sd->image, sd->photo_file, sd->photo_group);

   w_img = sd->iw;
   h_img = sd->ih;

   setlocale(LC_NUMERIC,"C");
   const char *s_right_marge = edje_object_data_get(sd->obj, "right_marge");
   if(s_right_marge)
     right_marge = atoi(s_right_marge);
   const char *s_left_marge = edje_object_data_get(sd->obj, "left_marge");
   if(s_left_marge)
     left_marge = atoi(s_left_marge);

   const char *s_top_marge = edje_object_data_get(sd->obj, "top_marge");
   if(s_top_marge)
     top_marge = atoi(s_top_marge);
   const char *s_bottom_marge = edje_object_data_get(sd->obj, "bottom_marge");
   if(s_bottom_marge)
     bottom_marge = atoi(s_bottom_marge);

   w = w - right_marge - left_marge;
   h = h - top_marge - bottom_marge;

   w_img2 = w - w_img;
   h_img2 = h - h_img;

   if(w_img2 >= 0 && w_img2 < h_img2)
     {
	 h_img2 = h_img * (w/(double)w_img);
	 w_img2 = w_img * (w/(double)w_img);
     }
   else if(h_img2 >= 0 && h_img2 < w_img2)
     {
	 w_img2 = w_img * (h/(double)h_img);
	 h_img2 = h_img * (h/(double)h_img);
     }
   else if(w_img2 < 0 && w_img2 < h_img2)
     {
	 h_img2 = h_img * (w/(double)w_img);
	 w_img2 = w_img * (w/(double)w_img);
     }
   else
     {
	w_img2 = w_img * (h/(double)h_img);
	h_img2 = h_img * (h/(double)h_img);
     }

   Edje_Message_Int_Set *msg = alloca(sizeof(Edje_Message_Int_Set) + (3 * sizeof(int)));
   msg->count=4;
   msg->val[0] = (int)(w - w_img2) / 2;
   msg->val[1] = (int)(h - h_img2) / 2;
   msg->val[2] = (int)- (w - msg->val[0] - w_img2);
   msg->val[3] = (int)- (h - msg->val[1] - h_img2);

   edje_object_message_send(sd->obj,EDJE_MESSAGE_INT_SET , 0, msg);
}


/*******************************************/
/* Internal smart object required routines */
/*******************************************/

   static void
_smart_init(void)
{
   if (smart) return;
     {
	static const Evas_Smart_Class sc =
	  {
	     E_OBJ_NAME,
	     EVAS_SMART_CLASS_VERSION,
	     _smart_add,
	     _smart_del,
	     _smart_move,
	     _smart_resize,
	     _smart_show,
	     _smart_hide,
	     NULL,
	     _smart_clip_set,
	     _smart_clip_unset,
	     NULL,
	     NULL,
	     NULL,
	     NULL
	  };
	smart = evas_smart_class_new(&sc);
     }
}

   static void
_smart_add(Evas_Object * obj)
{
   Smart_Data *sd;

   sd = calloc(1, sizeof(Smart_Data));
   if (!sd) return;
   evas_object_smart_data_set(obj, sd);

   sd->w = -1;
   sd->h = -1;

   evas_object_smart_callback_add(obj, "unselect", _unselect_cb, obj);
   evas_object_smart_callback_add(obj, "select,extern", _select_extern_cb, obj);
}

   static void
_smart_del(Evas_Object * obj)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;


   if(sd->theme_file)
     eina_stringshare_del(sd->theme_file);
   if(sd->theme_group)
     eina_stringshare_del(sd->theme_group);

   if(sd->photo_file)
     eina_stringshare_del(sd->photo_file);
   if(sd->photo_group)
     eina_stringshare_del(sd->photo_group);

   if(sd->o_progressbar)
     evas_object_del(sd->o_progressbar);

   if(sd->obj)
     evas_object_del(sd->obj);
   if(sd->image)
     {
	evas_object_del(sd->image);
     }
   if(sd->flickr)
     evas_object_del(sd->flickr);


   free(sd);
}

   static void
_smart_move(Evas_Object * obj, Evas_Coord x, Evas_Coord y)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;

   evas_object_move(sd->obj, x, y);
}

   static void
_smart_resize(Evas_Object * obj, Evas_Coord w, Evas_Coord h)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_resize(sd->obj, w, h);
   _update(obj);
}

   static void
_smart_show(Evas_Object * obj)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_show(sd->obj);
}

   static void
_smart_hide(Evas_Object * obj)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_hide(sd->obj);
}

   static void
_smart_clip_set(Evas_Object * obj, Evas_Object * clip)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_clip_set(sd->obj, clip);
}

   static void
_smart_clip_unset(Evas_Object * obj)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_clip_unset(sd->obj);
}

