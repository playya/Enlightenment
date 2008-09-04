#include <Elementary.h>
#include "elm_priv.h"

static void _elm_win_name_set(Elm_Win *win, const char *name);
static void _elm_win_title_set(Elm_Win *win, const char *title);
static void _elm_win_show(Elm_Win *win);
static void _elm_win_hide(Elm_Win *win);
static void _elm_win_del(Elm_Obj *obj);

Elm_Win_Class _elm_win_class =
{
   &_elm_obj_class, /* parent */
   _elm_win_name_set,
     _elm_win_title_set,
     _elm_win_show,
     _elm_win_hide
};

static void
_elm_win_name_set(Elm_Win *win, const char *name)
{
   if (win->name) evas_stringshare_del(win->name);
   win->name = evas_stringshare_add(name);
   if (win->ee) ecore_evas_name_class_set(win->ee, win->name, _elm_appname);
}

static void
_elm_win_title_set(Elm_Win *win, const char *title)
{
   if (win->title) evas_stringshare_del(win->title);
   win->title = evas_stringshare_add(title);
   if (win->ee) ecore_evas_title_set(win->ee, win->title);
}

static void
_elm_win_show(Elm_Win *win)
{
   ecore_evas_show(win->ee);
}

static void
_elm_win_hide(Elm_Win *win)
{
   ecore_evas_hide(win->ee);
}

static void
_elm_win_type_set(Elm_Win *win, Elm_Win_Type type)
{
   if (win->win_type == type) return;
   win->win_type = type;
   switch (win->win_type)
     {
      case ELM_WIN_BASIC:
	if (win->xwin) ecore_x_netwm_window_type_set(win->xwin, ECORE_X_WINDOW_TYPE_NORMAL);
	// FIXME: if child object is a scroll region, then put its child back
	break;
      case ELM_WIN_DIALOG_BASIC:
	if (win->xwin) ecore_x_netwm_window_type_set(win->xwin, ECORE_X_WINDOW_TYPE_DIALOG);
	// FIXME: if child object is a scroll region, then put its child back
	break;
      case ELM_WIN_SCROLLABLE:
	if (win->xwin) ecore_x_netwm_window_type_set(win->xwin, ECORE_X_WINDOW_TYPE_NORMAL);
	// FIXME: take child object and put into scroll region
	break;
      default:
	break;
     }
}

static void
_elm_win_del(Elm_Obj *obj)
{
   if (_elm_obj_del_defer(obj)) return;
   if (((Elm_Win *)obj)->ee)
     {
	ecore_evas_free(((Elm_Win *)obj)->ee);
	evas_stringshare_del(((Elm_Win *)obj)->title);
	evas_stringshare_del(((Elm_Win *)obj)->name);
     }
   if (((Elm_Win *)obj)->deferred_resize_job)
     ecore_job_del(((Elm_Win *)obj)->deferred_resize_job);
   ((Elm_Obj_Class *)(((Elm_Win_Class *)(obj->clas))->parent))->del(obj);
}

static void
_elm_win_delete_request(Ecore_Evas *ee)
{
   Elm_Win *win = ecore_evas_data_get(ee, "__Elm");
   if (!win) return;
   _elm_obj_nest_push();
   _elm_cb_call(ELM_OBJ(win), ELM_CB_DEL_REQ, NULL);
   if (win->autodel) win->del(ELM_OBJ(win));
   _elm_obj_nest_pop();
}

static void
_elm_win_resize_job(Elm_Win *win)
{
   win->deferred_resize_job = NULL;
   ecore_evas_geometry_get(win->ee, NULL, NULL, &(win->w), &(win->h));
   _elm_cb_call(ELM_OBJ(win), ELM_CB_RESIZE, NULL);
}

static void
_elm_win_resize(Ecore_Evas *ee)
{
   Elm_Win *win = ecore_evas_data_get(ee, "__Elm");
   if (!win) return;
   if (win->deferred_resize_job) ecore_job_del(win->deferred_resize_job);
   win->deferred_resize_job = ecore_job_add(_elm_win_resize_job, win);
}

EAPI Elm_Win *
elm_win_new(void)
{
   Elm_Win *win;
   
   win = ELM_NEW(Elm_Win);
   
   _elm_obj_init(ELM_OBJ(win));
   win->clas = &_elm_win_class;
   win->type = ELM_OBJ_WIN;
   
   win->del = _elm_win_del;
   
   win->name_set = _elm_win_name_set;
   win->title_set = _elm_win_title_set;
   win->show = _elm_win_show;
   win->hide = _elm_win_hide;
   
   switch (_elm_engine)
     {
      case ELM_SOFTWARE_X11:
	win->ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 100, 100);
	if (win->ee) win->xwin = ecore_evas_software_x11_window_get(win->ee);
	break;
      case ELM_SOFTWARE_FB:
	win->ee = ecore_evas_fb_new(NULL, 0, 100, 100);
        ecore_evas_fullscreen_set(win->ee, 1);
	break;
      case ELM_SOFTWARE_16_X11:
	win->ee = ecore_evas_software_x11_16_new(NULL, 0, 0, 0, 100, 100);
	if (win->ee) win->xwin = ecore_evas_software_x11_16_window_get(win->ee);
	break;
      case ELM_XRENDER_X11:
	win->ee = ecore_evas_xrender_x11_new(NULL, 0, 0, 0, 100, 100);
	if (win->ee) win->xwin = ecore_evas_xrender_x11_window_get(win->ee);
	break;
      case ELM_OPENGL_X11:
	win->ee = ecore_evas_gl_x11_new(NULL, 0, 0, 0, 100, 100);
	if (win->ee) win->xwin = ecore_evas_gl_x11_window_get(win->ee);
	break;
      default:
	break;
     }
   if (!win->ee)
     {
	printf("ELEMENTARY: Error. Cannot create window.\n");
	win->del(ELM_OBJ(win));
	return NULL;
     }
   win->type = ELM_WIN_BASIC;
   win->name = evas_stringshare_add("default"); 
   win->title = evas_stringshare_add("Elementary Window");

   win->evas = ecore_evas_get(win->ee);
   ecore_evas_title_set(win->ee, win->title);
   ecore_evas_name_class_set(win->ee, win->name, _elm_appname);
   ecore_evas_data_set(win->ee, "__Elm", win);
   ecore_evas_callback_delete_request_set(win->ee, _elm_win_delete_request);
   ecore_evas_callback_resize_set(win->ee, _elm_win_resize);
   // FIXME: use elm config for this
   evas_image_cache_set(win->evas, 4096 * 1024);
   evas_font_cache_set(win->evas, 512 * 1024);
   evas_font_path_append(win->evas, "fonts");
   edje_frametime_set(1.0 / 30.0);
   
   return win;
}
