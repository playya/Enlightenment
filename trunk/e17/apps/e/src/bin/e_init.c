#include "e.h"

/* local subsystem globals */
static Ecore_X_Window  _e_init_win = 0;
static Ecore_Evas     *_e_init_ecore_evas = NULL;
static Evas           *_e_init_evas = NULL;
static Evas_Object    *_e_init_object = NULL;

/* externally accessible functions */
int
e_init_init(void)
{
   int x, y, w, h;
   Ecore_X_Window root;
   Ecore_X_Window *roots;
   int num, i;
   Evas_Object *o;
   int n;
   
   num = 0;
   roots = ecore_x_window_root_list(&num);
   if ((!roots) || (num <= 0))
     {
        e_error_message_show("X reports there are no root windows and %i screens!\n",
			     num);
	return 0;
     }
   root = roots[0];
   ecore_x_window_size_get(root, &w, &h);
   _e_init_ecore_evas = ecore_evas_software_x11_new(NULL, root, 0, 0, w, h);
   e_canvas_add(_e_init_ecore_evas);
   _e_init_evas = ecore_evas_get(_e_init_ecore_evas);
   _e_init_win = ecore_evas_software_x11_window_get(_e_init_ecore_evas);
   ecore_evas_override_set(_e_init_ecore_evas, 1);
   ecore_evas_name_class_set(_e_init_ecore_evas, "E", "Init_Window");
   ecore_evas_title_set(_e_init_ecore_evas, "Enlightenment Init");
   e_path_evas_append(path_fonts, _e_init_ecore_evas);
   e_pointer_ecore_evas_set(_e_init_ecore_evas);
   ecore_evas_raise(_e_init_ecore_evas);
   ecore_evas_show(_e_init_ecore_evas);
   
   n = ecore_x_xinerama_screen_count_get();
   if (n == 0)
     {
	o = edje_object_add(_e_init_evas);
	edje_object_file_set(o,
			     /* FIXME: "init.eet" needs to come from config */
			     e_path_find(path_init, "init.eet"),
			     "init/splash");
	evas_object_move(o, x, y);
	evas_object_resize(o, w, h);
	evas_object_show(o);
	_e_init_object = o;
     }
   else
     {
	int i;
	
	for (i = 0; i < n; i++)
	  {
	     ecore_x_xinerama_screen_geometry_get(i, &x, &y, &w, &h);
	     printf("$$$ INIT scr %i, %i %i, %ix%i\n", i, x, y, w, h);
	     if (i == 0)
	       {
		  o = edje_object_add(_e_init_evas);
		  edje_object_file_set(o,
				       /* FIXME: "init.eet" needs to come from config */
				       e_path_find(path_init, "init.eet"),
				       "init/splash");
		  evas_object_move(o, x, y);
		  evas_object_resize(o, w, h);
		  evas_object_show(o);
		  _e_init_object = o;
	       }
	     else
	       {
		  o = edje_object_add(_e_init_evas);
		  edje_object_file_set(o,
				       /* FIXME: "init.eet" needs to come from config */
				       e_path_find(path_init, "init.eet"),
				       "init/splash");
		  evas_object_move(o, x, y);
		  evas_object_resize(o, w, h);
		  evas_object_show(o);
	       }
	  }
     }
   
   free(roots);
   return 1;
}

int
e_init_shutdown(void)
{
   e_init_hide();
   e_canvas_cache_flush();
   return 1;
}

void
e_init_show(void)
{
   if (!_e_init_ecore_evas) return;
   ecore_evas_raise(_e_init_ecore_evas);
   ecore_evas_show(_e_init_ecore_evas);
}

void
e_init_hide(void)
{
   /* FIXME: emit signal to edje and wait for it to respond or until a */
   /* in case the edje was badly created and never responds */
   if (!_e_init_ecore_evas) return;
   ecore_evas_hide(_e_init_ecore_evas);
   evas_object_del(_e_init_object);
   e_canvas_del(_e_init_ecore_evas);
   ecore_evas_free(_e_init_ecore_evas);
   _e_init_ecore_evas = NULL;
   _e_init_evas = NULL;
   _e_init_win = 0;
   _e_init_object = NULL;
}

void
e_init_title_set(const char *str)
{
   if (!_e_init_object) return;
   edje_object_part_text_set(_e_init_object, "title", str);
}

void
e_init_version_set(const char *str)
{
   if (!_e_init_object) return;
   edje_object_part_text_set(_e_init_object, "version", str);
}

void
e_init_status_set(const char *str)
{
   if (!_e_init_object) return;
   edje_object_part_text_set(_e_init_object, "status", str);
}

Ecore_X_Window
e_init_window_get(void)
{
   return _e_init_win;
}
