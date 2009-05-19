#include "Ethumb.h"
#include "Ethumb_Plugin.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <Eina.h>
#include <Evas.h>
#include <Emotion.h>

static Evas_Object *_emotion = NULL;

static void
_frame_resized_cb(void *data, Evas_Object *o, void *event_info)
{
   Ethumb *e = data;
   Evas_Coord ww, hh;
   Evas_Coord mw, mh;

   emotion_object_size_get(o, &mw, &mh);
   ethumb_calculate_aspect(e, mw, mh, &ww, &hh);
   ethumb_plugin_image_resize(e, ww, hh);

   evas_object_resize(o, ww, hh);
   evas_object_move(o, 0, 0);

   ethumb_image_save(e);

   evas_object_smart_callback_del(o, "frame_resize", _frame_resized_cb);
   emotion_object_play_set(o, 0);
   evas_object_del(o);
   ethumb_finished_callback_call(e, 1);
}

static void
_generate_thumb(Ethumb *e)
{
   Evas_Object *o;
   int r;
   const char *file;
   float time;

   o = emotion_object_add(ethumb_evas_get(e));
   r = emotion_object_init(o, "xine");
   if (!r)
     {
	fprintf(stderr, "ERROR: could not start emotion using gstreamer"
		" plugin.\n");
	evas_object_del(o);
	ethumb_finished_callback_call(e, 0);
	return;
     }

   ethumb_file_get(e, &file, NULL);
   time = ethumb_video_time_get(e);

   emotion_object_file_set(o, file);
   emotion_object_position_set(o, time);
   emotion_object_play_set(o, 1);

   evas_object_smart_callback_add(o, "frame_resize", _frame_resized_cb, e);

   evas_object_show(o);

   _emotion = o;

}

EAPI Ethumb_Plugin *
ethumb_plugin_get(void)
{
   static const char *extensions[] = { "avi", NULL };
   static Ethumb_Plugin plugin =
     {
	extensions,
	_generate_thumb,
     };

   return &plugin;
}

Eina_Bool
_module_init(void)
{
   return EINA_TRUE;
}

void
_module_shutdown()
{
}

EINA_MODULE_INIT(_module_init);
EINA_MODULE_SHUTDOWN(_module_shutdown);
