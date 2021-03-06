#include <Evoak.h>
#include <Ecore.h>
#include <math.h>

#include "config.h"

Evoak_Object *oo_title, *oo_close, *oo_resize;
Evoak_Coord   oo_x, oo_y, oo_w, oo_h;
char          oo_do_move = 0, oo_do_resize = 0;
static void (*oo_resize_func) (Evoak_Coord x, Evoak_Coord y, Evoak_Coord w, Evoak_Coord h) = NULL;
static void (*oo_raise_func) (void);


void oo_configure(void);
void oo_raise(void);

static void
oo_title_down(void *data, Evoak *e, Evoak_Object *obj, void *event_info)
{
   Evoak_Event_Mouse_Down *ev;
   
   ev = event_info;
   evoak_freeze(e);
   evoak_object_image_file_set(oo_title, PACKAGE_DATA_DIR"/data/bar2.png", NULL);
   if (ev->button == 1) oo_do_move = 1;
   oo_raise();
   evoak_thaw(e);
}

static void
oo_title_up(void *data, Evoak *e, Evoak_Object *obj, void *event_info)
{
   Evoak_Event_Mouse_Up *ev;
   
   ev = event_info;
   evoak_freeze(e);
   evoak_object_image_file_set(oo_title, PACKAGE_DATA_DIR"/data/bar1.png", NULL);
   if (ev->button == 1) oo_do_move = 0;
   evoak_thaw(e);
}

static void
oo_title_move(void *data, Evoak *e, Evoak_Object *obj, void *event_info)
{
   Evoak_Event_Mouse_Move *ev;
   
   ev = event_info;
   evoak_freeze(e);
   if (oo_do_move)
     {
	oo_x += ev->cur.x - ev->prev.x;
	oo_y += ev->cur.y - ev->prev.y;
	oo_configure();
     }
   evoak_thaw(e);
}

static void
oo_title_in(void *data, Evoak *e, Evoak_Object *obj, void *event_info)
{
   Evoak_Event_Mouse_In *ev;
   
   ev = event_info;
   evoak_freeze(e);
   evoak_object_image_file_set(oo_title, PACKAGE_DATA_DIR"/data/bar1.png", NULL);
   evoak_object_image_file_set(oo_close, PACKAGE_DATA_DIR"/data/close1.png", NULL);
   evoak_thaw(e);
}

static void
oo_title_out(void *data, Evoak *e, Evoak_Object *obj, void *event_info)
{
   Evoak_Event_Mouse_Out *ev;
   
   ev = event_info;
   evoak_freeze(e);
   evoak_object_image_file_set(oo_title, PACKAGE_DATA_DIR"/data/bar0.png", NULL);
   evoak_object_image_file_set(oo_close, PACKAGE_DATA_DIR"/data/close0.png", NULL);
   evoak_thaw(e);
}

static void
oo_resize_down(void *data, Evoak *e, Evoak_Object *obj, void *event_info)
{
   Evoak_Event_Mouse_Down *ev;
   
   ev = event_info;
   evoak_freeze(e);
   evoak_object_image_file_set(oo_resize, PACKAGE_DATA_DIR"/data/res2.png", NULL);
   if (ev->button == 1) oo_do_resize = 1;
   evoak_thaw(e);
}

static void
oo_resize_up(void *data, Evoak *e, Evoak_Object *obj, void *event_info)
{
   Evoak_Event_Mouse_Up *ev;
   
   ev = event_info;
   evoak_freeze(e);
   evoak_object_image_file_set(oo_resize, PACKAGE_DATA_DIR"/data/res1.png", NULL);
   if (ev->button == 1) oo_do_resize = 0;
   evoak_thaw(e);
}

static void
oo_resize_move(void *data, Evoak *e, Evoak_Object *obj, void *event_info)
{
   Evoak_Event_Mouse_Move *ev;
   
   ev = event_info;
   evoak_freeze(e);
   if (oo_do_resize)
     {
	oo_w += ev->cur.x - ev->prev.x;
	oo_h += ev->cur.y - ev->prev.y;
	if (oo_w < 0) oo_w = 0;
	if (oo_h < 0) oo_h = 0;
	oo_configure();
     }
   evoak_thaw(e);
}

static void
oo_close_down(void *data, Evoak *e, Evoak_Object *obj, void *event_info)
{
   Evoak_Event_Mouse_Down *ev;
   
   ev = event_info;
   evoak_freeze(e);
   evoak_object_image_file_set(oo_close, PACKAGE_DATA_DIR"/data/close2.png", NULL);
   evoak_thaw(e);
}

static void
oo_close_up(void *data, Evoak *e, Evoak_Object *obj, void *event_info)
{
   Evoak_Event_Mouse_Up *ev;
   
   ev = event_info;
   evoak_freeze(e);
   evoak_object_image_file_set(oo_close, PACKAGE_DATA_DIR"/data/close1.png", NULL);
   evoak_thaw(e);
   ecore_main_loop_quit();
}

static void
oo_close_in(void *data, Evoak *e, Evoak_Object *obj, void *event_info)
{
   Evoak_Event_Mouse_In *ev;
   
   ev = event_info;
   evoak_freeze(e);
   evoak_object_image_file_set(oo_close, PACKAGE_DATA_DIR"/data/close1.png", NULL);
   evoak_object_image_file_set(oo_title, PACKAGE_DATA_DIR"/data/bar1.png", NULL);
   evoak_thaw(e);
}

static void
oo_close_out(void *data, Evoak *e, Evoak_Object *obj, void *event_info)
{
   Evoak_Event_Mouse_Out *ev;
   
   ev = event_info;
   evoak_freeze(e);
   evoak_object_image_file_set(oo_close, PACKAGE_DATA_DIR"/data/close0.png", NULL);
   evoak_object_image_file_set(oo_title, PACKAGE_DATA_DIR"/data/bar0.png", NULL);
   evoak_thaw(e);
}

void
oo_configure(void)
{
   Evoak_Object *o;
   
   evoak_freeze(evoak_object_evoak_get(oo_title));
   o = oo_title;
   evoak_object_move(o, oo_x - 8, oo_y - 16);
   evoak_object_resize(o, oo_w + 16, 20);
   evoak_object_image_fill_set(o, 0, 0, oo_w + 16, 20);
   o = oo_close;
   evoak_object_move(o, oo_x + 8 + oo_w - 18, oo_y - 8 - 16);
   evoak_object_resize(o, 28, 26);
   evoak_object_image_fill_set(o, 0, 0, 28, 26);
   o = oo_resize;
   evoak_object_move(o, oo_x + oo_w - 6, oo_y + oo_h - 6);
   evoak_object_resize(o, 24, 24);
   evoak_object_image_fill_set(o, 0, 0, 24, 24);
   
   if (oo_resize_func) oo_resize_func(oo_x, oo_y, oo_w, oo_h);

   evoak_thaw(evoak_object_evoak_get(oo_title));   
}

void
oo_raise(void)
{
   evoak_freeze(evoak_object_evoak_get(oo_title));
   
   if (oo_raise_func) oo_raise_func();
   
   evoak_object_raise(oo_title);
   evoak_object_raise(oo_resize);
   evoak_object_raise(oo_close);
   evoak_thaw(evoak_object_evoak_get(oo_title));
}

void
oo_setup(Evoak *e)
{
   Evoak_Object *o;
   
   oo_x = rand() % 50;
   oo_y = rand() % 50;
   oo_w = 80;
   oo_h = 60;
   
   evoak_freeze(e);
   o = evoak_object_image_add(e);
   evoak_object_event_callback_add(o, EVOAK_CALLBACK_MOUSE_DOWN, oo_title_down, NULL);
   evoak_object_event_callback_add(o, EVOAK_CALLBACK_MOUSE_UP,   oo_title_up,   NULL);
   evoak_object_event_callback_add(o, EVOAK_CALLBACK_MOUSE_MOVE, oo_title_move, NULL);
   evoak_object_event_callback_add(o, EVOAK_CALLBACK_MOUSE_IN,   oo_title_in,   NULL);
   evoak_object_event_callback_add(o, EVOAK_CALLBACK_MOUSE_OUT,  oo_title_out,  NULL);
   evoak_object_image_border_set(o, 10, 10, 0, 0);
   evoak_object_image_file_set(o, PACKAGE_DATA_DIR"/data/bar0.png", NULL);
   evoak_object_show(o);
   oo_title = o;

   o = evoak_object_image_add(e);
   evoak_object_event_callback_add(o, EVOAK_CALLBACK_MOUSE_DOWN, oo_resize_down, NULL);
   evoak_object_event_callback_add(o, EVOAK_CALLBACK_MOUSE_UP,   oo_resize_up,   NULL);
   evoak_object_event_callback_add(o, EVOAK_CALLBACK_MOUSE_MOVE, oo_resize_move, NULL);
   evoak_object_image_file_set(o, PACKAGE_DATA_DIR"/data/res1.png", NULL);
   evoak_object_show(o);
   oo_resize = o;
   
   o = evoak_object_image_add(e);
   evoak_object_event_callback_add(o, EVOAK_CALLBACK_MOUSE_DOWN, oo_close_down, NULL);
   evoak_object_event_callback_add(o, EVOAK_CALLBACK_MOUSE_UP,   oo_close_up,   NULL);
   evoak_object_event_callback_add(o, EVOAK_CALLBACK_MOUSE_IN,   oo_close_in,   NULL);
   evoak_object_event_callback_add(o, EVOAK_CALLBACK_MOUSE_OUT,  oo_close_out,  NULL);
   evoak_object_image_file_set(o, PACKAGE_DATA_DIR"/data/close0.png", NULL);
   evoak_object_show(o);
   oo_close = o;
   
   oo_configure();
   
   evoak_thaw(e);
}




Evoak_Object *o_grad;

int set = 0;

static void
resize(Evoak_Coord x, Evoak_Coord y, Evoak_Coord w, Evoak_Coord h)
{
   evoak_object_move(o_grad, x, y);
   evoak_object_resize(o_grad, w, h);
}

static void
rais(void)
{
   evoak_object_raise(o_grad);
}

static void
mouse_move(void *data, Evoak *e, Evoak_Object *obj, void *event_info)
{
   Evoak_Event_Mouse_Move *ev;
   Evoak_Coord x, y, cx, cy, cw, ch;
   int pos;
   
   ev = event_info;
   evoak_object_gradient_angle_set(o_grad, ev->cur.x * 2);
}

static void
mouse_down(void *data, Evoak *e, Evoak_Object *obj, void *event_info)
{
   Evoak_Event_Mouse_Down *ev;
   Evoak_Coord x, y, cx, cy, cw, ch;
   int pos;
   
   ev = event_info;
   if (ev->button == 1)
     {
	int i, n;
	
	evoak_object_gradient_colors_clear(o_grad);
	n = 3 + (rand() % 10);
	for (i = 0; i < n; i++)
	  {
	     int r, g, b, a, d;
	     
	     r = rand() & 0xff;
	     g = rand() & 0xff;
	     b = rand() & 0xff;
	     a = rand() & 0xff;
	     d = 1 + (rand() % 20);
	     if (i == n - 1) d = 0;
	     evoak_object_gradient_color_add(o_grad, r, g, b, a, d);
	  }
     }
   if (ev->button == 2)
     {
	evoak_object_gradient_colors_clear(o_grad);
     }
   if (ev->button == 3)
     {
	evoak_object_gradient_colors_clear(o_grad);
	evoak_object_gradient_color_add(o_grad, 0, 0, 0, 100, 10);
	evoak_object_gradient_color_add(o_grad, 0, 0, 255, 150, 10);
     }
}

void
setup(Evoak *e)
{
   Evoak_Object *o;
   Evoak_Coord w, h;

   evoak_freeze(e);
      
   evoak_font_path_append(e, PACKAGE_DATA_DIR"/data/fonts");

   o = evoak_object_gradient_add(e);
   evoak_object_event_callback_add(o, EVOAK_CALLBACK_MOUSE_MOVE, mouse_move, NULL);
   evoak_object_event_callback_add(o, EVOAK_CALLBACK_MOUSE_DOWN, mouse_down, NULL);
   evoak_object_gradient_color_add(o, 0, 0, 0, 100, 10);
   evoak_object_gradient_color_add(o, 0, 0, 255, 150, 10);
   evoak_object_gradient_color_add(o, 0, 255, 255, 200, 10);
   evoak_object_gradient_color_add(o, 255, 128, 0, 250, 10);
   evoak_object_gradient_color_add(o, 255, 255, 0, 255, 10);
   evoak_object_gradient_color_add(o, 255, 255, 255, 255, 0);
   evoak_object_show(o);
   o_grad = o;
   
   oo_resize_func = resize;
   oo_raise_func = rais;
   oo_setup(e);
   
   evoak_thaw(e);
}

int
cb_canvas_info(void *data, int type, void *event)
{
   Evoak_Event_Canvas_Info *ev;
   
   ev = event;
   if (!set)
     {
	setup(ev->evoak);
	set = 1;
     }
   return 1;
}

int
cb_disconnect(void *data, int type, void *event)
{
   printf("disconnected!\n");
   ecore_main_loop_quit();
   return 1;
}

int
main(int argc, char **argv)
{
   if (evoak_init())
     {
	Evoak *ev;
	
	ecore_event_handler_add(EVOAK_EVENT_CANVAS_INFO, cb_canvas_info, NULL);
	ecore_event_handler_add(EVOAK_EVENT_DISCONNECT, cb_disconnect, NULL);
	
	ev = evoak_connect(NULL, "evoak_test_grad", "custom");
	if (ev)
	  {
	     ecore_main_loop_begin();
	     evoak_disconnect(ev);
	  }
	evoak_shutdown();
     }
}
