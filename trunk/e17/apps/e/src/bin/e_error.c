/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

/* TODO List:
 * 
 * * edjify error dialogs if edje data can be found for them
 * * current gui dialg needs to resize to fit contents if they are bigger
 */

/* local subsystem functions */
static void _e_error_message_show_x(char *txt);

static void _e_error_cb_ok_down(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _e_error_cb_ok_up(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _e_error_edje_cb_ok_up(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _e_error_cb_job_ecore_evas_free(void *data);
    
/* local subsystem globals */
static int error_gui = 0;

/* externally accessible functions */
void
e_error_message_show_internal(char *txt)
{
   printf("_______                     _______\n"
	  "|:::::| Enlightenment Error |:::::|\n"
	  "~~~~~~~                     ~~~~~~~\n"
	  "%s\n", 
	  txt);
   if (error_gui) _e_error_message_show_x(txt);
}

void
e_error_dialog_show_internal(char *title, char *txt)
{
   Evas_List *l;
   E_Manager *man;

   l = e_manager_list();
   if (!l) return;
   man = l->data;
   e_error_message_manager_show(man, title, txt);
}

void
e_error_gui_set(int on)
{
   error_gui = on;
}

void
e_error_message_manager_show(E_Manager *man, char *title, char *txt)
{
   Ecore_Evas  *ee;
   Evas        *e;
   Evas_Object *o;
   int          error_w, error_h;
   Evas_List   *l, *shapelist = NULL;
   Evas_Coord   maxw, maxh;
   
   error_w = 400;
   error_h = 200;
   ee = ecore_evas_software_x11_new(NULL, man->win, 
				    (man->w - error_w) / 2, (man->h - error_h) / 2,
				    error_w, error_h);
   ecore_evas_software_x11_direct_resize_set(ee, 1);
   e_canvas_add(ee);
   
   ecore_evas_name_class_set(ee, "E", "Low_Level_Dialog");
   ecore_evas_title_set(ee, "Enlightenment: Low Level Dialog");
   e = ecore_evas_get(ee);
   e_pointer_ecore_evas_set(ee);
   
   o = edje_object_add(e);
   if (!edje_object_file_set(o,
			     /* FIXME: "default.eet" needs to come from conf */
			     e_path_find(path_themes, "default.eet"),
			     "error/main"))

     {
	Evas_Coord tw, th;
	char *newstr;
	
	if (o)
	  evas_object_del(o);

	maxw = 0;
	maxh = 0;
	
	o = evas_object_image_add(e);
	evas_object_image_file_set(o, e_path_find(path_images, "e.png"), NULL);
	evas_object_move(o, 16, 16);
	evas_object_resize(o, 64, 64);
	evas_object_image_fill_set(o, 0, 0, 64, 64);
	evas_object_pass_events_set(o, 1);
	evas_object_show(o);
	
	o = evas_object_text_add(e);
	evas_object_color_set(o, 255, 255, 255, 128);
	evas_object_text_font_set(o, "Vera-Bold", 12);
	evas_object_text_text_set(o, title);
	evas_object_geometry_get(o, NULL, NULL, &tw, &th);
	evas_object_move(o, 
			 (16 + 64 + 16) + 1,
			 (16 + ((64 - th) / 2)) + 1);
	evas_object_pass_events_set(o, 1);
	evas_object_show(o);
	
	maxw = 16 + 64 + 16 + tw + 16;
	maxh = 16 + 64;

	o = evas_object_text_add(e);
	evas_object_color_set(o, 0, 0, 0, 255);
	evas_object_text_font_set(o, "Vera-Bold", 12);
	evas_object_text_text_set(o, title);
	evas_object_geometry_get(o, NULL, NULL, &tw, &th);
	evas_object_move(o, 
			 16 + 64 + 16,
			 16 + ((64 - th) / 2));
	evas_object_pass_events_set(o, 1);
	evas_object_show(o);

	newstr = strdup(txt);
	if (newstr)
	  {
	     char *p;
	     Evas_Coord y;

	     y = 16 + 64 + 16;
	     for (p = newstr; p;)
	       {
		  char *pp;

		  pp = strchr(p, '\n');
		  if (pp) *pp = 0;
		  o = evas_object_text_add(e);
		  evas_object_color_set(o, 255, 255, 255, 128);
		  evas_object_text_font_set(o, "Vera", 10);
		  evas_object_text_text_set(o, p);
		  evas_object_geometry_get(o, NULL, NULL, &tw, &th);
		  evas_object_move(o, 16 + 1, y + 1);
		  evas_object_pass_events_set(o, 1);
		  evas_object_show(o);

		  o = evas_object_text_add(e);
		  evas_object_color_set(o, 0, 0, 0, 255);
		  evas_object_text_font_set(o, "Vera", 10);
		  evas_object_text_text_set(o, p);
		  evas_object_geometry_get(o, NULL, NULL, &tw, &th);
		  evas_object_move(o, 16, y);
		  evas_object_pass_events_set(o, 1);
		  evas_object_show(o);

		  if ((16 + tw + 16) > maxw) maxw = 16 + tw + 16;
		  y += th;
		  if (pp) p = pp + 1;
		  else p = NULL;
	       }
	     free(newstr);
	     maxh = y;
	  }

	maxh += 16 + 32 + 16;
	error_w = maxw;
	error_h = maxh;

	if (error_w > man->w) error_w = man->w;
	if (error_h > man->h) error_h = man->h;
	
	o = evas_object_image_add(e);
	evas_object_image_file_set(o, e_path_find(path_images, "button_out.png"), NULL);
	evas_object_move(o, (error_w - 64) / 2, error_h - 16 - 32);
	evas_object_resize(o, 64, 32);
	evas_object_image_fill_set(o, 0, 0, 64, 32);
	evas_object_image_border_set(o, 8, 8, 8, 8);
	evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, _e_error_cb_ok_down, ee);
	evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_UP, _e_error_cb_ok_up, ee);
	evas_object_show(o);
	
	o = evas_object_text_add(e);
	evas_object_color_set(o, 255, 255, 255, 128);
	evas_object_text_font_set(o, "Vera-Bold", 12);
	evas_object_text_text_set(o, "OK");
	evas_object_geometry_get(o, NULL, NULL, &tw, &th);
	evas_object_move(o, ((error_w - tw) / 2) + 1, (error_h - 16 - 32 + ((32 - th) / 2)) + 1);
	evas_object_pass_events_set(o, 1);
	evas_object_show(o);
	
	o = evas_object_text_add(e);
	evas_object_color_set(o, 0, 0, 0, 255);
	evas_object_text_font_set(o, "Vera-Bold", 12);
	evas_object_text_text_set(o, "OK");
	evas_object_geometry_get(o, NULL, NULL, &tw, &th);
	evas_object_move(o, (error_w - tw) / 2, error_h - 16 - 32 + ((32 - th) / 2));
	evas_object_pass_events_set(o, 1);
	evas_object_show(o);

	o = evas_object_image_add(e);
	evas_object_image_file_set(o, e_path_find(path_images, "error_bg.png"), NULL);
	evas_object_move(o, 0, 0);
	evas_object_image_fill_set(o, 0, 0, error_w, error_h);
	evas_object_resize(o, error_w, error_h);
	evas_object_image_border_set(o, 3, 3, 3, 3);
	evas_object_pass_events_set(o, 1);
	evas_object_layer_set(o, -10);
	evas_object_show(o);

	ecore_evas_move(ee, (man->w - error_w) / 2, (man->h - error_h) / 2);
	ecore_evas_resize(ee, error_w, error_h);
	
	for (l = man->containers; l; l = l->next)
	  {
	     E_Container *con;
	     E_Container_Shape *es;
	     
	     con = l->data;
	     es = e_container_shape_add(con);
	     e_container_shape_move(es, (man->w - error_w) / 2, (man->h - error_h) / 2);
	     e_container_shape_resize(es, error_w, error_h);
	     e_container_shape_show(es);
	     shapelist = evas_list_append(shapelist, es);
	  }
	ecore_evas_data_set(ee, "shapes", shapelist);
	
	o = evas_object_rectangle_add(e);
	evas_object_name_set(o, "allocated");
     }
   else
     {
	char format[1024];
	Evas_Object *text;
	int x, y, w, h, nw, nh;

	evas_object_move(o, 0, 0);
	evas_object_resize(o, error_w, error_h);
	edje_object_signal_callback_add(o, "close", "",
					_e_error_edje_cb_ok_up, ee);
	evas_object_show(o);

	edje_object_part_text_set(o, "title", title);

	snprintf(format, sizeof(format), 
		 "source='%s' font='%s' size=%d wrap=word",
		 e_path_find(path_themes, "default.eet"),
		 "fonts/Edje Vera", 10);
	text = evas_object_textblock_add(e);
	evas_object_color_set(text, 0, 0, 0, 255);
	evas_object_textblock_format_insert(text, format);
	  {
	     char *pp, *newstr, *p;
	     newstr = strdup(txt);
	     p = newstr;
	     while (p)
	       {
		  pp = strchr(p, '\n');
		  if (pp) *pp = 0;
		  evas_object_textblock_text_insert(text, p);
		  if (pp)
		    {
		       p = pp + 1;
		       evas_object_textblock_format_insert(text, "\n");
		    }
		  else
		    p = NULL;
	       }
	     free(newstr);
	  }
	edje_object_part_swallow(o, "text", text);
	evas_object_show(text);

	edje_object_part_geometry_get(o, "text", &x, &y, &w, &h);
	evas_object_textblock_format_size_get(text, &nw, &nh);
	/* FIXME: How to handle the width of the text? */
	error_h += (nh - h);

	evas_object_move(o, 0, 0);
	evas_object_resize(o, error_w, error_h);
	evas_object_show(o);

	ecore_evas_move(ee, (man->w - error_w) / 2, (man->h - error_h) / 2);
	ecore_evas_resize(ee, error_w, error_h);
	
	for (l = man->containers; l; l = l->next)
	  {
	     E_Container *con;
	     E_Container_Shape *es;
	     
	     con = l->data;
	     es = e_container_shape_add(con);
	     e_container_shape_move(es, (man->w - error_w) / 2, (man->h - error_h) / 2);
	     e_container_shape_resize(es, error_w, error_h);
	     e_container_shape_show(es);
	     shapelist = evas_list_append(shapelist, es);
	  }
	ecore_evas_data_set(ee, "shapes", shapelist);
	
	o = evas_object_rectangle_add(e);
	evas_object_name_set(o, "allocated");
     }
     {
	Ecore_X_Window mwin, win;
	
	win = ecore_evas_software_x11_window_get(ee);
	mwin = e_menu_grab_window_get();
	if (!mwin) mwin = e_init_window_get();
	if (!mwin)
	  ecore_x_window_raise(win);
	else
	  ecore_x_window_configure(win,
				   ECORE_X_WINDOW_CONFIGURE_MASK_SIBLING |
				   ECORE_X_WINDOW_CONFIGURE_MASK_STACK_MODE,
				   0, 0, 0, 0, 0,
				   mwin, ECORE_X_WINDOW_STACK_BELOW);
	ecore_evas_show(ee);
     }
}

/* local subsystem functions */
static void
_e_error_message_show_x(char *txt)
{
   e_error_dialog_show_internal("Enlightenment: Error!", txt);
}

static void
_e_error_cb_ok_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Down *ev;
   Ecore_Evas *ee;
   
   ev = event_info;
   if (ev->button != 1) return;
   ee = data;
   evas_object_image_file_set(obj, e_path_find(path_images, "button_in.png"), NULL);
}

static void
_e_error_cb_ok_up(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Up *ev;
   Ecore_Evas *ee;
   Evas_Object *o;
   
   ev = event_info;
   if (ev->button != 1) return;
   ee = data;
   evas_object_image_file_set(obj, e_path_find(path_images, "button_out.png"), NULL);
   o = evas_object_name_find(ecore_evas_get(ee), "allocated");
   if (o)
     {
	evas_object_del(o);
	ecore_job_add(_e_error_cb_job_ecore_evas_free, ee);
     }
}

static void
_e_error_edje_cb_ok_up(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Ecore_Evas *ee;
   Evas_Object *o;
   
   ee = data;
   o = evas_object_name_find(ecore_evas_get(ee), "allocated");
   if (o)
     {
	evas_object_del(o);
	ecore_job_add(_e_error_cb_job_ecore_evas_free, ee);
     }
}

static void
_e_error_cb_job_ecore_evas_free(void *data)
{
   Ecore_Evas *ee;
   Evas_List *shapelist, *l;
   
   ee = data;
   shapelist = ecore_evas_data_get(ee, "shapes");
   for (l = shapelist; l; l = l->next)
     e_object_del(E_OBJECT(l->data));
   evas_list_free(shapelist);

   e_canvas_del(ee);
   ecore_evas_free(ee);
}
