#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <Evas.h>
#include <Ecore.h>
#include "src/Etox.h"
#include "etox-config.h"

#define MAX_EVAS_COLORS (216)
#define MAX_FONT_CACHE (512 * 1024)
#define MAX_IMAGE_CACHE (1 * (1024 * 1024))
#define RENDER_ENGINE RENDER_METHOD_ALPHA_SOFTWARE

/* the obstacle's coords */
#define OBST_X 100.0
#define OBST_Y 100.0

double obstacle_w = -1.0, obstacle_h = -1.0, obstacle_x, obstacle_y;

static void e_idle(void *data);
static void ecore_window_expose(Ecore_Event *ev);
static void ecore_mouse_down(Ecore_Event *ev);

void setup(void);

Evas evas;
Evas_Render_Method render_method = RENDER_ENGINE;
Etox e;

static void
e_idle(void *data)
{
  evas_render(evas);
}

static void
ecore_window_expose(Ecore_Event *ev)
{
  Ecore_Event_Window_Expose *e;

  e = (Ecore_Event_Window_Expose *)ev->event;
  evas_update_rect(evas, e->x, e->y, e->w, e->h);
}

static void
ecore_mouse_down(Ecore_Event *ev)
{
  Ecore_Event_Mouse_Down *md;
  static Evas_Object obj = NULL;
  double x, y, w, h, et_x = 0.0, et_y = 0.0, pos_x = 0.0, pos_y = 0.0;
  int index = 0;

  md = (Ecore_Event_Mouse_Down *)ev->event;

  printf("MOUSE_DOWN AT x=%d, y=%d\n", md->x, md->y);

  etox_get_geometry(e, &et_x, &et_y, NULL, NULL);

  pos_x = evas_screen_x_to_world(evas, md->x) - et_x;
  pos_y = evas_screen_y_to_world(evas, md->y) - et_y;
  index = etox_get_char_geometry_at_position(e, pos_x, pos_y, &x, &y, &w, &h);

  printf("-> (index = %d) pos_x=%f, pos_y=%f; x=%f, y=%f, w=%f, h=%f\n", 
         index, pos_x, pos_y, x, y, w, h);

  x += et_x;
  y += et_y;

  if (!obj)
    obj = evas_add_rectangle(evas);
  evas_set_color(evas, obj, 0, 255, 255, 50);
  evas_move(evas, obj, x, y);
  evas_resize(evas, obj, w, h);
  evas_show(evas, obj);
}

void setup(void)
{
  Window win, ewin;

  ecore_event_filter_handler_add(ECORE_EVENT_WINDOW_EXPOSE, 
                                 ecore_window_expose);
  ecore_event_filter_handler_add(ECORE_EVENT_MOUSE_DOWN, 
                                 ecore_mouse_down);
  ecore_event_filter_idle_handler_add(e_idle, NULL);
  win = ecore_window_new(0, 0, 0, 400, 400);

  evas = evas_new_all(ecore_display_get(), win, 0, 0, 400, 400, render_method,
		      MAX_EVAS_COLORS, MAX_FONT_CACHE, MAX_IMAGE_CACHE,
		      PACKAGE_DATA_DIR"/fnt");

  ewin = evas_get_window(evas);

  ecore_window_show(ewin);
  ecore_window_set_events(ewin, XEV_EXPOSE | XEV_BUTTON);
  ecore_window_show(win);
}

int
main(int argc, char *argv[])
{
  Etox_Style s;
  Etox_Color c;
  Evas_Object bg, et_bg, obst;

  obstacle_x = OBST_X;
  obstacle_y = OBST_Y;

  {
    int i;

    for (i = 1; i < argc; i++)
      {
	if (!strcmp(argv[i], "soft") ||
	    !strcmp(argv[i], "x11")  ||
            !strcmp(argv[i], "hard") )
          {
	    if (!strcmp(argv[i], "soft"))
	      render_method = RENDER_METHOD_ALPHA_SOFTWARE;
	    if (!strcmp(argv[i], "x11"))
	      render_method = RENDER_METHOD_BASIC_HARDWARE;
	    if (!strcmp(argv[i], "hard"))
	      render_method = RENDER_METHOD_3D_HARDWARE;
	  }
	else
	  {
	    if (obstacle_w < 0.0)
	      obstacle_w = atoi(argv[i]);
	    else 
	      if (obstacle_h < 0.0)
		obstacle_h = atoi(argv[i]);
	  }
      }
  }

  if (obstacle_w < 0.0)
    obstacle_w = 100.0;
  if (obstacle_h < 0.0)
    obstacle_h = 100.0;

  ecore_display_init(NULL);
  ecore_event_signal_init();
  ecore_event_filter_init();
  ecore_event_x_init();

  setup();

  /* add a background */
  bg = evas_add_rectangle(evas);
  evas_resize(evas, bg, 400, 400);
  evas_move(evas, bg, 0, 0);
  evas_set_color(evas, bg, 255, 255, 255, 255);
  evas_show(evas, bg);

  /* add an etox-background */
  et_bg = evas_add_rectangle(evas);
  evas_resize(evas, et_bg, 380, 380);
  evas_move(evas, et_bg, 10, 10);
  evas_set_color(evas, et_bg, 0, 0, 255, 50);
  evas_show(evas, et_bg);

  /* draw obstacle-rect */
  obst = evas_add_rectangle(evas);
  evas_resize(evas, obst, obstacle_w, obstacle_h);
  evas_move(evas, obst, obstacle_x, obstacle_y);
  evas_set_color(evas, obst, 255, 0, 0, 50);
  evas_show(evas, obst);

  /* test the etox stuff.. */
  e = etox_new(evas, "My Etox");
  etox_move(e, 10, 10);
  etox_resize(e, 380, 380);
  etox_set_font(e, "notepad", 10);
  /*  etox_set_padding(e, 10); */

  etox_style_add_path(PACKAGE_DATA_DIR"/style");
  etox_style_add_path("./style");

  s = etox_style_new("plain");

  obst = etox_obstacle_add(e, obstacle_x, obstacle_y,
                              obstacle_w, obstacle_h);

  c = etox_color_new();
  etox_color_set_member(c, "fg", 70, 90, 80, 255);
  etox_color_set_member(c, "sh", 70, 90, 80, 255);
  etox_color_set_member(c, "ol", 70, 90, 80, 255);

  /* Remeber, this is a _test_ program, not an example program.
   * You should keep the amount of ET_TEXT's as low as possible in 
   * a real prog..
   */

  etox_set_text(e, ET_ALIGN(ETOX_ALIGN_TYPE_CENTER, ETOX_ALIGN_TYPE_LEFT),
    ET_TEXT("This is just a test\n\tstring.. some lame "
    "copied\n\tstuff actually.. \n\n"), ET_STYLE(s),
    ET_ALIGN(ETOX_ALIGN_TYPE_CENTER, ETOX_ALIGN_TYPE_RIGHT),
    ET_TEXT("As a result of meeting requests from users, Enlightenment over "),
    ET_TEXT("time has done some nasty hacks,\nbut now for the development of "),
    ET_TEXT("version 0.17.0, we have moved a lot of the design and core code "),
    ET_COLOR(c),
    ET_TEXT("into various subsystems than generalize some back end and let us "),
    ET_COLOR_END,
    ET_TEXT("not only use it in Enlightenment itself, but make this work "),
    ET_TEXT("available to everyone to use as they want. The result is various "),
    ET_ALIGN_END, ET_STYLE_END, ET_FONT("cinema", 8), 
    ET_TEXT("lower level libraries that do lots of useful stuff for you and "  
    "provide a consistant api to them. Also daemons and helper programs "
    "have been written too to make life easier."), ET_END);

  etox_show(e);

  etox_set_alpha(e, 100);

  printf("Text: %s\n", etox_get_text_string(e));
  
  printf ("\n\n");

  printf("Actual Text: %s\n", etox_get_actual_text_string(e));

  {
    double x, y, w, h;
    Evas_Object real_rect;

    etox_get_actual_geometry(e, &x, &y, &w, &h);

    printf("Actual rect: x=%f,y=%f,w=%f,h=%f\n", x, y, w, h);

    real_rect = evas_add_rectangle(evas);
    evas_set_color(evas, real_rect, 0, 255, 0, 50);
    evas_move(evas, real_rect, x, y);
    evas_resize(evas, real_rect, w, h);
    evas_show(evas, real_rect);
  }

  {
    double x, y, w, h;
    Evas_Object char_rect;

    etox_get_char_geometry_at(e, 0, &x, &y, &w, &h);

    printf("Char geometry: x=%f,y=%f,w=%f,h=%f\n", x, y, w, h);

    char_rect = evas_add_rectangle(evas);
    evas_set_color(evas, char_rect, 255, 255, 0, 50);
    evas_move(evas, char_rect, x + 10, y + 10);
    evas_resize(evas, char_rect, w, h);
    evas_show(evas, char_rect);
  }

/*
  {
    Evas_Object clip_rect;

    clip_rect = evas_add_rectangle(evas);
    evas_set_color(evas, clip_rect, 255, 255, 255, 255);
    evas_move(evas, clip_rect, 20, 20);
    evas_resize(evas, clip_rect, 360, 360);
    evas_show(evas, clip_rect);
    etox_set_clip(e, clip_rect);
  }
*/

  ecore_event_loop();

  etox_free(e);
  evas_free(evas);

  return 0;
}
