/**
 * Filename: main.c
 * Corey Donohoe <atmos@atmos.org>
 * October 10, 2003
 * Description: main.c for entice, an edje based image viewer
 */
#include <stdio.h>
#include <stdlib.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Ecore_Ipc.h>
#include <Ecore_Job.h>
#include <limits.h>
#include "entice.h"
#include "ipc.h"
#include "prefs.h"
#include "../config.h"

/** 
 * win_mouse_out_cb - for later
 */
static void
win_mouse_out_cb(Ecore_Evas * ee)
{
   return;
   ee = NULL;
}

/** 
 * win_mouse_in_cb - for later
 */
static void
win_mouse_in_cb(Ecore_Evas * ee)
{
   return;
   ee = NULL;

}

/**
 * win_resize_cb - when our ecore evas gets resized
 * @ee - pointer to our ecore evas
 */
static void
win_resize_cb(Ecore_Evas * ee)
{
   int w, h;
   Evas *e = NULL;

   if ((e = ecore_evas_get(ee)))
   {
      ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
      entice_resize(w, h);
   }

}

/**
 * win_move_cb - when our ecore evas gets moved
 * @ee - pointer to our ecore evas
 */
static void
win_move_cb(Ecore_Evas * ee)
{
   return;
   ee = NULL;
}

/**
 * win_move_cb - when our ecore evas gets a delete request
 * @ee - pointer to our ecore evas
 */
static void
win_del_cb(Ecore_Evas * ee)
{
   ecore_main_loop_quit();
   return;
   ee = NULL;
}

/**
 * win_post_render_cb - freeze after we render
 */
static void
win_post_render_cb(Ecore_Evas * ee)
{
   edje_freeze();
}

/**
 * win_pre_render_cb - thaw before we render
 */
static void
win_pre_render_cb(Ecore_Evas * ee)
{
   edje_thaw();
}

/**
 * exit_cb - when our ecore event loop gets killed, i.e ctrl-c
 * data - should be NULL
 * type - exit type
 * event - event data
 */
static int
exit_cb(void *data, int type, void *event)
{
   ecore_main_loop_quit();
   return (0);
   data = NULL;
   type = 0;
   event = NULL;
}

/**
 * main - does a few things
 * 1. startup ecore, ecore_evas, ecore_ipc, and edje
 * 2. parse our config file
 * 3. Create either a software or gl x11 windows based on the config 
 * 4. Append our ecore_evas callbacks to the ecore_evas
 * 5. Initialize entice with our ecore_evas
 * 6. Fork off thumbnail generation, and show the window
 * 7. for(;;)
 */
int
main(int argc, char *argv[])
{
   int i = 0;

   ecore_init();
   ecore_app_args_set(argc, (const char **) argv);
   ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, exit_cb, NULL);

   if (!entice_ipc_init(argc, (const char **) argv))
   {
      if (ecore_evas_init())
      {
         Evas_Object *o = NULL;
         Ecore_Evas *ee = NULL;

         edje_init();
         edje_frametime_set(1.0 / 60.0);

         entice_config_init();
         if (entice_config_engine_get() == GL_X11)
            ee = ecore_evas_gl_x11_new(NULL, 0, 0, 0, 320, 240);
         else
            ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 320, 240);

         if (ee)
         {
            ecore_evas_callback_mouse_out_set(ee, win_mouse_out_cb);
            ecore_evas_callback_mouse_in_set(ee, win_mouse_in_cb);
            ecore_evas_callback_resize_set(ee, win_resize_cb);
            ecore_evas_callback_move_set(ee, win_move_cb);
            ecore_evas_callback_delete_request_set(ee, win_del_cb);
            ecore_evas_callback_post_render_set(ee, win_post_render_cb);
            ecore_evas_callback_pre_render_set(ee, win_pre_render_cb);

            ecore_evas_name_class_set(ee, "Entice", "Main");
            ecore_evas_title_set(ee, "Entice !!!!");
            ecore_evas_borderless_set(ee, 0);
            ecore_evas_shaped_set(ee, 0);

            evas_font_cache_set(ecore_evas_get(ee), 1 * 1024 * 1024);
            evas_image_cache_set(ecore_evas_get(ee), 8 * 1024 * 1024);

            evas_font_path_append(ecore_evas_get(ee), 
				 PACKAGE_DATA_DIR"/fonts");
            o = evas_object_rectangle_add(ecore_evas_get(ee));
            evas_object_color_set(o, 255, 255, 255, 255);
            evas_object_resize(o, 5555, 5555);
            evas_object_move(o, 0, 0);
            evas_object_layer_set(o, 0);
            evas_object_show(o);

            entice_init(ee);
            switch (fork())
            {
              case 0:
                 for (i = 1; i < argc; i++)
                    entice_file_add_job_cb((void *) argv[i]);
                 exit(0);
                 break;
              default:
                 break;
            }
            ecore_evas_resize(ee, 640, 480);
            ecore_evas_show(ee);
         }
         ecore_main_loop_begin();
         entice_free();
         ecore_evas_shutdown();
      }
      entice_ipc_shutdown();
   }
   ecore_shutdown();

   return (0);
}
