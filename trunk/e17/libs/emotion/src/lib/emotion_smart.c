#include "Emotion.h"
#include "emotion_private.h"
#include <dlfcn.h>

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

#define E_OBJ_NAME "emotion_object"

typedef struct _Smart_Data Smart_Data;

struct _Smart_Data
{
   Emotion_Video_Module  *module;
   void                  *video;
   
   char          *file;
   Evas_Object   *obj;
   double         ratio;
   double         pos;
   double         seek_pos;
   double         len;
   
   Ecore_Job     *job;
   
   unsigned char  play : 1;
   unsigned char  seek : 1;
};

static void _mouse_move(void *data, Evas *ev, Evas_Object *obj, void *event_info);
static void _mouse_down(void *data, Evas *ev, Evas_Object *obj, void *event_info);
static void _pos_set_job(void *data);
static void _pixels_get(void *data, Evas_Object *obj);

static void _smart_init(void);
static void _smart_add(Evas_Object * obj);
static void _smart_del(Evas_Object * obj);
static void _smart_layer_set(Evas_Object * obj, int layer);
static void _smart_raise(Evas_Object * obj);
static void _smart_lower(Evas_Object * obj);
static void _smart_stack_above(Evas_Object * obj, Evas_Object * above);
static void _smart_stack_below(Evas_Object * obj, Evas_Object * below);
static void _smart_move(Evas_Object * obj, Evas_Coord x, Evas_Coord y);
static void _smart_resize(Evas_Object * obj, Evas_Coord w, Evas_Coord h);
static void _smart_show(Evas_Object * obj);
static void _smart_hide(Evas_Object * obj);
static void _smart_color_set(Evas_Object * obj, int r, int g, int b, int a);
static void _smart_clip_set(Evas_Object * obj, Evas_Object * clip);
static void _smart_clip_unset(Evas_Object * obj);

/**********************************/
/* Globals for the E Video Object */
/**********************************/
static Evas_Smart  *smart = NULL;

static Emotion_Video_Module *
_emotion_module_open(const char *name)
{
   void *handle;
   char buf[4096];
   
   snprintf(buf, sizeof(buf), "%s/%s", PACKAGE_LIB_DIR"/emotion/", name);
   handle = dlopen(buf, RTLD_NOW | RTLD_GLOBAL);
   if (handle)
     {
	Emotion_Video_Module *(*func_module_open) (void);
	
	func_module_open = dlsym(handle, "module_open");
	if (func_module_open)
	  {
	     Emotion_Video_Module *mod;
	     
	     mod = func_module_open();
	     if (mod)
	       {
		  mod->handle = handle;
		  return mod;
	       }
	  }
	dlclose(handle);
     }
   return NULL;
}

static void
_emotion_module_close(Emotion_Video_Module *mod)
{
   void *handle;
   void (*module_close) (Emotion_Video_Module *module);
   
   handle = mod->handle;
   module_close = dlsym(handle, "module_close");
   if (module_close) module_close(mod);
   dlclose(handle);
}

/*******************************/
/* Externally accessible calls */
/*******************************/
Evas_Object *
emotion_object_add(Evas *evas)
{
   _smart_init();
   return evas_object_smart_add(evas, smart);   
}

void
emotion_object_file_set(Evas_Object *obj, const char *file)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   
   if ((file) && (sd->file) && (!strcmp(file, sd->file))) return;
   if (sd->file) free(sd->file);
   sd->file = NULL;
   
   if (file)
     {
	int w, h;
	
	sd->file = strdup(file);
	if (sd->video) sd->module->file_close(sd->video);
	if (sd->module) _emotion_module_close(sd->module);
	sd->module = _emotion_module_open("emotion_decoder_xine.so");
	if (!sd->module) return;
	sd->video = sd->module->file_open(sd->file, obj);
	if (sd->video)
	  {
	     sd->module->size_get(sd->video, &w, &h);
	     evas_object_image_size_set(sd->obj, w, h);
	     sd->ratio = sd->module->ratio_get(sd->video);
	     sd->pos = 0.0;
	     if (sd->play) sd->module->play(sd->video, 0.0);
	  }
     }
   else
     {
	if (sd->video)
	  {
	     sd->module->file_close(sd->video);
	     evas_object_image_size_set(sd->obj, 0, 0);
	  }
	if (sd->module) _emotion_module_close(sd->module);
	sd->module = NULL;
     }
}

const char *
emotion_object_file_get(Evas_Object *obj)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, NULL);
   return sd->file;
}

void
emotion_object_play_set(Evas_Object *obj, Evas_Bool play)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (((play) && (sd->play)) || ((!play) && (!sd->play))) return;
   if (!sd->module) return;
   if (!sd->video) return;
   sd->play = play;
   if (sd->play) sd->module->play(sd->video, sd->pos);
   else sd->module->stop(sd->video);
}

Evas_Bool
emotion_object_play_get(Evas_Object *obj)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->video) return 0;
   return sd->play;
}

void
emotion_object_position_set(Evas_Object *obj, double sec)
{
   Smart_Data *sd;
   int frame;
   
   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (!sd->module) return;
   if (!sd->video) return;
   sd->seek_pos = sec;
   sd->seek = 1;
   sd->pos = sd->seek_pos;
   if (sd->job) ecore_job_del(sd->job);
   sd->job = ecore_job_add(_pos_set_job, obj);
}

double
emotion_object_position_get(Evas_Object *obj)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0.0);
   if (!sd->module) return 0.0;
   if (!sd->video) return 0.0;
   return sd->pos;
}

Evas_Bool
emotion_object_seekable_get(Evas_Object *obj)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->module) return 0;
   if (!sd->video) return 0;
   return sd->module->seekable(sd->video);
}

double
emotion_object_play_length_get(Evas_Object *obj)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0.0);
   if (!sd->module) return 0.0;
   if (!sd->video) return 0.0;
   sd->len = sd->module->len_get(sd->video);
   return sd->len;
}

void
emotion_object_size_get(Evas_Object *obj, int *iw, int *ih)
{
   Smart_Data *sd;
   
   if (iw) *iw = 0;
   if (ih) *ih = 0;
   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   evas_object_image_size_get(sd->obj, iw, ih);
}

void
emotion_object_smooth_scale_set(Evas_Object *obj, Evas_Bool smooth)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   evas_object_image_smooth_scale_set(sd->obj, smooth);
}

Evas_Bool
emotion_object_smooth_scale_get(Evas_Object *obj)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   return evas_object_image_smooth_scale_get(sd->obj);
}

double
emotion_object_ratio_get(Evas_Object *obj)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 1.0);
   if (!sd->module) return 0.0;
   if (!sd->video) return 0.0;
   return sd->ratio;
}

void
emotion_object_event_simple_send(Evas_Object *obj, Emotion_Event ev)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (!sd->module) return;
   if (!sd->video) return;
   sd->module->event_feed(sd->video, ev);
}

void
emotion_object_audio_volume_set(Evas_Object *obj, double vol)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (!sd->module) return;
   if (!sd->video) return;
   sd->module->audio_channel_volume_set(sd->video, vol);
}

double
emotion_object_audio_volume_get(Evas_Object *obj)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 1.0);
   if (!sd->module) return 0.0;
   if (!sd->video) return 0.0;
   return sd->module->audio_channel_volume_get(sd->video);
}

void
emotion_object_audio_mute_set(Evas_Object *obj, Evas_Bool mute)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (!sd->module) return;
   if (!sd->video) return;
   sd->module->audio_channel_mute_set(sd->video, mute);
}

Evas_Bool
emotion_object_audio_mute_get(Evas_Object *obj)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->module) return 0;
   if (!sd->video) return 0;
   return sd->module->audio_channel_mute_get(sd->video);
}

int
emotion_object_audio_channel_count(Evas_Object *obj)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->module) return 0;
   if (!sd->video) return 0;
   return sd->module->audio_channel_count(sd->video);
}

const char *
emotion_object_audio_channel_name_get(Evas_Object *obj, int channel)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, NULL);
   if (!sd->module) return NULL;
   if (!sd->video) return NULL;
   return sd->module->audio_channel_name_get(sd->video, channel);
}

void
emotion_object_audio_channel_set(Evas_Object *obj, int channel)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (!sd->module) return;
   if (!sd->video) return;
   sd->module->audio_channel_set(sd->video, channel);
}

int
emotion_object_audio_channel_get(Evas_Object *obj)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->module) return 0;
   if (!sd->video) return 0;
   return sd->module->audio_channel_get(sd->video);
}

void
emotion_object_video_mute_set(Evas_Object *obj, Evas_Bool mute)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (!sd->module) return;
   if (!sd->video) return;
   sd->module->video_channel_mute_set(sd->video, mute);
}

Evas_Bool
emotion_object_video_mute_get(Evas_Object *obj)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->module) return 0;
   if (!sd->video) return 0;
   return sd->module->video_channel_mute_get(sd->video);
}

int
emotion_object_video_channel_count(Evas_Object *obj)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->module) return 0;
   if (!sd->video) return 0;
   return sd->module->video_channel_count(sd->video);
}

const char *
emotion_object_video_channel_name_get(Evas_Object *obj, int channel)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, NULL);
   if (!sd->module) return NULL;
   if (!sd->video) return NULL;
   return sd->module->video_channel_name_get(sd->video, channel);
}

void
emotion_object_video_channel_set(Evas_Object *obj, int channel)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (!sd->module) return;
   if (!sd->video) return;
   sd->module->video_channel_set(sd->video, channel);
}

int
emotion_object_video_channel_get(Evas_Object *obj)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->module) return 0;
   if (!sd->video) return 0;
   return sd->module->video_channel_get(sd->video);
}

void
emotion_object_spu_mute_set(Evas_Object *obj, Evas_Bool mute)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (!sd->module) return;
   if (!sd->video) return;
   sd->module->spu_channel_mute_set(sd->video, mute);
}

Evas_Bool
emotion_object_spu_mute_get(Evas_Object *obj)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->module) return 0;
   if (!sd->video) return 0;
   return sd->module->spu_channel_mute_get(sd->video);
}

int
emotion_object_spu_channel_count(Evas_Object *obj)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->module) return 0;
   if (!sd->video) return 0;
   return sd->module->spu_channel_count(sd->video);
}

const char *
emotion_object_spu_channel_name_get(Evas_Object *obj, int channel)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, NULL);
   if (!sd->module) return NULL;
   if (!sd->video) return NULL;
   return sd->module->spu_channel_name_get(sd->video, channel);
}

void
emotion_object_spu_channel_set(Evas_Object *obj, int channel)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (!sd->module) return;
   if (!sd->video) return;
   sd->module->spu_channel_set(sd->video, channel);
}

int
emotion_object_spu_channel_get(Evas_Object *obj)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->module) return 0;
   if (!sd->video) return 0;
   return sd->module->spu_channel_get(sd->video);
}

int
emotion_object_chapter_count(Evas_Object *obj)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->module) return 0;
   if (!sd->video) return 0;
   return sd->module->chapter_count(sd->video);
}

void
emotion_object_chapter_set(Evas_Object *obj, int chapter)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (!sd->module) return;
   if (!sd->video) return;
   sd->module->chapter_set(sd->video, chapter);
}

int
emotion_object_chapter_get(Evas_Object *obj)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0);
   if (!sd->module) return 0;
   if (!sd->video) return 0;
   return sd->module->chapter_get(sd->video);
}

const char *
emotion_object_chapter_name_get(Evas_Object *obj, int chapter)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, NULL);
   if (!sd->module) return NULL;
   if (!sd->video) return NULL;
   return sd->module->chapter_name_get(sd->video, chapter);
}

void
emotion_object_play_speed_set(Evas_Object *obj, double speed)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (!sd->module) return;
   if (!sd->video) return;
   sd->module->speed_set(sd->video, speed);
}

double
emotion_object_play_speed_get(Evas_Object *obj)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, 0.0);
   if (!sd->module) return 0.0;
   if (!sd->video) return 0.0;
   return sd->module->speed_get(sd->video);
}

void
emotion_object_eject(Evas_Object *obj)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (!sd->module) return;
   if (!sd->video) return;
   sd->module->eject(sd->video);
}





/*****************************/
/* Utility calls for modules */
/*****************************/

void *
_emotion_video_get(Evas_Object *obj)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET_RETURN(sd, obj, E_OBJ_NAME, NULL);
   return sd->video;
}

void
_emotion_frame_new(Evas_Object *obj)
{
   Smart_Data *sd;
   
   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   evas_object_image_pixels_dirty_set(sd->obj, 1);
}

void
_emotion_video_pos_update(Evas_Object *obj, double pos, double len)
{
   Smart_Data *sd;
   int npos = 0, nlen = 0;
   
   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   if (pos != sd->pos) npos = 1;
   if (len != sd->len) nlen = 1;
   sd->pos = pos;
   sd->len = len;
   if (npos) evas_object_smart_callback_call(obj, "frame_decode", NULL);
   if (nlen) evas_object_smart_callback_call(obj, "length_change", NULL);
}

void
_emotion_frame_resize(Evas_Object *obj, int w, int h, double ratio)
{
   Smart_Data *sd;
   int iw, ih;
   int changed = 0;
   
   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   evas_object_image_size_get(sd->obj, &iw, &ih);
   if ((w != iw) || (h != ih))
     {
	evas_object_image_size_set(sd->obj, w, h);
	changed = 1;
     }
   if (ratio != sd->ratio)
     {
	sd->ratio = ratio;
	changed = 1;
     }
   if (changed) evas_object_smart_callback_call(obj, "frame_resize", NULL);
}


/****************************/
/* Internal object routines */
/****************************/

static void         
_mouse_move(void *data, Evas *ev, Evas_Object *obj, void *event_info)
{                      
   Evas_Event_Mouse_Move *e;
   Smart_Data *sd;
   int x, y, iw, ih;
   Evas_Coord ox, oy, ow, oh;
   
   e = event_info;
   sd = data;
   if (!sd->module) return;
   if (!sd->video) return;
   evas_object_geometry_get(obj, &ox, &oy, &ow, &oh);
   evas_object_image_size_get(obj, &iw, &ih);
   if ((iw < 1) || (ih < 1)) return;
   x = (((int)e->cur.canvas.x - ox) * iw) / ow;
   y = (((int)e->cur.canvas.y - oy) * ih) / oh;
   sd->module->event_mouse_move_feed(sd->video, x, y);
}

static void         
_mouse_down(void *data, Evas *ev, Evas_Object *obj, void *event_info)
{                      
   Evas_Event_Mouse_Down *e;
   Smart_Data *sd;
   int x, y, iw, ih;
   Evas_Coord ox, oy, ow, oh;
   
   e = event_info;
   sd = data;
   if (!sd->module) return;
   if (!sd->video) return;
   evas_object_geometry_get(obj, &ox, &oy, &ow, &oh);
   evas_object_image_size_get(obj, &iw, &ih);
   if ((iw < 1) || (ih < 1)) return;
   x = (((int)e->canvas.x - ox) * iw) / ow;
   y = (((int)e->canvas.y - oy) * ih) / oh;
   sd->module->event_mouse_button_feed(sd->video, 1, x, y);
}

static void
_pos_set_job(void *data)
{
   Evas_Object *obj;
   Smart_Data *sd;
   
   obj = data;
   E_SMART_OBJ_GET(sd, obj, E_OBJ_NAME);
   sd->job = NULL;
   if (sd->seek)
     {
	sd->module->pos_set(sd->video, sd->seek_pos);
	sd->seek = 0;
     }
}

/* called by evas when it needs pixels for the image object */
static void
_pixels_get(void *data, Evas_Object *obj)
{
   Evas_Pixel_Import_Source ps;
   int iw, ih, w, h;
   int i;
   unsigned char **rows;
   Smart_Data *sd;

   sd = data;
   evas_object_image_size_get(obj, &iw, &ih);
   sd->module->yuv_size_get(sd->video, &w, &h);
   if ((w != iw) || (h != ih))
     {
	evas_object_image_size_set(obj, w, h);
	iw = w;
	ih = h;
     }
   ps.format = EVAS_PIXEL_FORMAT_YUV420P_601;
   ps.w = iw;
   ps.h = ih;
   
   ps.rows = malloc(ps.h * 2 * sizeof(void *));
   if (!ps.rows)
     {
	sd->module->frame_done(sd->video);
	return;
     }
   
   
   rows = (unsigned char **)ps.rows;
   
   if (sd->module->yuv_rows_get(sd->video, iw, ih,
				rows, 
				&rows[ps.h], 
				&rows[ps.h + (ps.h / 2)]))
     evas_object_image_pixels_import(obj, &ps);
   evas_object_image_pixels_dirty_set(obj, 0);
   free(ps.rows);
   sd->module->frame_done(sd->video);
}

/*******************************************/
/* Internal smart object required routines */
/*******************************************/
static void
_smart_init(void)
{
   if (smart) return;
   smart = evas_smart_new(E_OBJ_NAME,
			  _smart_add,
			  _smart_del,
			  _smart_layer_set,
			  _smart_raise,
			  _smart_lower,
			  _smart_stack_above,
			  _smart_stack_below,
			  _smart_move,
			  _smart_resize,
			  _smart_show,
			  _smart_hide,
			  _smart_color_set,
			  _smart_clip_set,
			  _smart_clip_unset,
			  NULL);
}

static void
_smart_add(Evas_Object * obj)
{
   Smart_Data *sd;
   
   sd = calloc(1, sizeof(Smart_Data));
   if (!sd) return;
   sd->obj = evas_object_image_add(evas_object_evas_get(obj));
   evas_object_event_callback_add(sd->obj, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move, sd);
   evas_object_event_callback_add(sd->obj, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down, sd);
   evas_object_image_pixels_get_callback_set(sd->obj, _pixels_get, sd);
   evas_object_smart_member_add(sd->obj, obj);
   sd->ratio = 1.0;
   evas_object_image_alpha_set(sd->obj, 0);
   evas_object_smart_data_set(obj, sd);
}
   
static void
_smart_del(Evas_Object * obj)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (sd->video) sd->module->file_close(sd->video);
   if (sd->module) _emotion_module_close(sd->module);
   evas_object_del(sd->obj);
   if (sd->file) free(sd->file);
   if (sd->job) ecore_job_del(sd->job);
   free(sd);
}
   
static void
_smart_layer_set(Evas_Object * obj, int layer)
{
   Smart_Data *sd;
      
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_layer_set(sd->obj, layer);
}

static void
_smart_raise(Evas_Object * obj)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_raise(sd->obj);
}

static void
_smart_lower(Evas_Object * obj)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   evas_object_lower(sd->obj);
}
                                                             
static void
_smart_stack_above(Evas_Object * obj, Evas_Object * above)
{
   Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_stack_above(sd->obj, above);
}
   
static void
_smart_stack_below(Evas_Object * obj, Evas_Object * below)
{
   Smart_Data *sd;
      
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_stack_below(sd->obj, below);
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
   evas_object_image_fill_set(sd->obj, 0, 0, w, h);
   evas_object_resize(sd->obj, w, h);
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
_smart_color_set(Evas_Object * obj, int r, int g, int b, int a)
{
   Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_color_set(sd->obj, r, g, b, a);
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
