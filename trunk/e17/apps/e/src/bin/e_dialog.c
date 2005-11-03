/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

/* local subsystem functions */
static void _e_dialog_free(E_Dialog *dia);
static void _e_dialog_del_func_cb(void *data, E_Dialog *dia);
static void _e_dialog_cb_delete(E_Win *win);
static void _e_dialog_cb_resize(E_Win *win);
static void _e_dialog_cb_key_down(void *data, Evas *e, Evas_Object *obj, void *event);
static void _e_dialog_cb_wid_on_focus(void *data, Evas_Object *obj);

/* local subsystem globals */

/* externally accessible functions */

E_Dialog *
e_dialog_new(E_Container *con)
{
   E_Dialog *dia;
   E_Manager *man;
   Evas_Object *o;
   Evas_Modifier_Mask mask;
   
   if (!con)
     {
	man = e_manager_current_get();
	if (!man) return NULL;
	con = e_container_current_get(man);
	if (!con) con = e_container_number_get(man, 0);
	if (!con) return NULL;
     }
   dia = E_OBJECT_ALLOC(E_Dialog, E_DIALOG_TYPE, _e_dialog_free);
   if (!dia) return NULL;
   dia->win = e_win_new(con);
   if (!dia->win)
     {
	free(dia);
	return NULL;
     }
   ecore_x_netwm_window_type_set(dia->win->evas_win, ECORE_X_WINDOW_TYPE_DIALOG);
   e_win_delete_callback_set(dia->win, _e_dialog_cb_delete);
   e_win_resize_callback_set(dia->win, _e_dialog_cb_resize);
   dia->win->data = dia;
   e_win_name_class_set(dia->win, "E", "_dialog");
   o = edje_object_add(e_win_evas_get(dia->win));
   dia->bg_object = o;
   e_theme_edje_object_set(o, "base/theme/dialog",
			   "widgets/dialog/main");
   evas_object_move(o, 0, 0);
   evas_object_show(o);

   o = e_widget_list_add(e_win_evas_get(dia->win), 1, 1);
   e_widget_on_focus_hook_set(o, _e_dialog_cb_wid_on_focus, dia);
   dia->box_object = o;
   edje_object_part_swallow(dia->bg_object, "buttons_swallow", o);

   o = evas_object_rectangle_add(e_win_evas_get(dia->win));
   dia->event_object = o;
   mask = 0;
   evas_object_key_grab(o, "Tab", mask, ~mask, 0);
   mask = evas_key_modifier_mask_get(e_win_evas_get(dia->win), "Shift");
   evas_object_key_grab(o, "Tab", mask, ~mask, 0);
   mask = 0;
   evas_object_key_grab(o, "Return", mask, ~mask, 0);
   mask = 0;
   evas_object_key_grab(o, "KP_Enter", mask, ~mask, 0);
   mask = 0;
   evas_object_key_grab(o, "space", mask, ~mask, 0);
   
   evas_object_event_callback_add(o, EVAS_CALLBACK_KEY_DOWN, _e_dialog_cb_key_down, dia);

   return dia;
}

void
e_dialog_button_add(E_Dialog *dia, char *label, char *icon, void (*func) (void *data, E_Dialog *dia), void *data)
{
   Evas_Object *o;

   if (!func) func = _e_dialog_del_func_cb;
   o = e_widget_button_add(e_win_evas_get(dia->win), label, icon, func, data, dia);
   e_widget_list_object_append(dia->box_object, o, 1, 0, 0.5);
   dia->buttons = evas_list_append(dia->buttons, o);
}

int
e_dialog_button_focus_num(E_Dialog *dia, int button)
{
   Evas_Object *o;
   
   o = evas_list_nth(dia->buttons, button);
   if (o) e_widget_focus_steal(o);
   return 1;
}

int
e_dialog_button_disable_num_set(E_Dialog *dia, int button, int disabled)
{
   Evas_Object *o;
   
   o = evas_list_nth(dia->buttons, button);
   if (o) e_widget_disabled_set(o, disabled);
   return 1;
}

void
e_dialog_title_set(E_Dialog *dia, char *title)
{
   e_win_title_set(dia->win, title);
}

void
e_dialog_text_set(E_Dialog *dia, char *text)
{
   if (!dia->text_object)
     {
	Evas_Object *o;
	
	o = edje_object_add(e_win_evas_get(dia->win));
	dia->text_object = o;
	e_theme_edje_object_set(o, "base/theme/dialog",
				"widgets/dialog/text");
	edje_object_part_swallow(dia->bg_object, "content_swallow", o);
	evas_object_show(o);
     }
   edje_object_part_text_set(dia->text_object, "text", text);
}

void
e_dialog_icon_set(E_Dialog *dia, char *icon, Evas_Coord size)
{
   if (!icon) return;
   
   dia->icon_object = edje_object_add(e_win_evas_get(dia->win));
   e_util_edje_icon_set(dia->icon_object, icon);
   edje_extern_object_min_size_set(dia->icon_object, size, size);
   edje_object_part_swallow(dia->bg_object, "icon_swallow", dia->icon_object);
   evas_object_show(dia->icon_object);
}

void
e_dialog_content_set(E_Dialog *dia, Evas_Object *obj, Evas_Coord minw, Evas_Coord minh)
{
   dia->content_object = obj;
   e_widget_on_focus_hook_set(obj, _e_dialog_cb_wid_on_focus, dia);
   edje_extern_object_min_size_set(obj, minw, minh);
   edje_object_part_swallow(dia->bg_object, "content_swallow", obj);
   evas_object_show(obj);
}

void
e_dialog_show(E_Dialog *dia)
{
   Evas_Coord mw, mh;
   Evas_Object *o;
   
   o = dia->text_object;
   if (o)
     {
	edje_object_size_min_calc(o, &mw, &mh);
	edje_extern_object_min_size_set(o, mw, mh);
	edje_object_part_swallow(dia->bg_object, "content_swallow", o);
     }

   o = dia->box_object;
   e_widget_min_size_get(o, &mw, &mh);
   edje_extern_object_min_size_set(o, mw, mh);
   edje_object_part_swallow(dia->bg_object, "buttons_swallow", o);
   
   edje_object_size_min_calc(dia->bg_object, &mw, &mh);
   evas_object_resize(dia->bg_object, mw, mh);
   e_win_resize(dia->win, mw, mh);
   e_win_size_min_set(dia->win, mw, mh);
   e_win_size_max_set(dia->win, mw, mh);
   e_win_show(dia->win);
   
   if (!e_widget_focus_get(dia->box_object))
     e_widget_focus_set(dia->box_object, 1);
}

/* local subsystem functions */
static void
_e_dialog_free(E_Dialog *dia)
{
   if (dia->buttons) evas_list_free(dia->buttons);
   if (dia->text_object) evas_object_del(dia->text_object);
   if (dia->icon_object) evas_object_del(dia->icon_object);
   if (dia->box_object) evas_object_del(dia->box_object);
   if (dia->bg_object) evas_object_del(dia->bg_object);
   if (dia->content_object) evas_object_del(dia->content_object);
   if (dia->event_object) evas_object_del(dia->event_object);
   e_object_del(E_OBJECT(dia->win));
   free(dia);
}

static void
_e_dialog_del_func_cb(void *data, E_Dialog *dia)
{
   e_object_del(E_OBJECT(dia));
}

static void
_e_dialog_cb_key_down(void *data, Evas *e, Evas_Object *obj, void *event)
{
   Evas_Event_Key_Down *ev;
   E_Dialog *dia;

   ev = event;
   dia = data;
   if (!strcmp(ev->keyname, "Tab"))
     {
	if (evas_key_modifier_is_set(evas_key_modifier_get(e_win_evas_get(dia->win)), "Shift"))
	  {
	     if (e_widget_focus_get(dia->box_object))
	       {
		  if (!e_widget_focus_jump(dia->box_object, 0))
		    {
		       if (dia->text_object)
			 e_widget_focus_set(dia->box_object, 0);
		       else
			 {
			    e_widget_focus_set(dia->content_object, 0);
			    if (!e_widget_focus_get(dia->content_object))
			      e_widget_focus_set(dia->box_object, 0);
			 }
		    }
	       }
	     else
	       {
		  if (!e_widget_focus_jump(dia->content_object, 0))
		    e_widget_focus_set(dia->box_object, 0);
	       }
	  }
	else
	  {
	     if (e_widget_focus_get(dia->box_object))
	       {
		  if (!e_widget_focus_jump(dia->box_object, 1))
		    {
		       if (dia->text_object)
			 e_widget_focus_set(dia->box_object, 1);
		       else
			 {
			    e_widget_focus_set(dia->content_object, 1);
			    if (!e_widget_focus_get(dia->content_object))
			      e_widget_focus_set(dia->box_object, 1);
			 }
		    }
	       }
	     else
	       {
		  if (!e_widget_focus_jump(dia->content_object, 1))
		    e_widget_focus_set(dia->box_object, 1);
	       }
	  }
     }
   else if (((!strcmp(ev->keyname, "Return")) || 
	     (!strcmp(ev->keyname, "KP_Enter")) || 
	     (!strcmp(ev->keyname, "space"))))
     {
	Evas_Object *o = NULL;
	
	if ((dia->content_object) && (e_widget_focus_get(dia->content_object)))
	  o = e_widget_focused_object_get(dia->content_object);
	else
	  o = e_widget_focused_object_get(dia->box_object);
	if (o) e_widget_activate(o);
     }
}

static void
_e_dialog_cb_delete(E_Win *win)
{
   E_Dialog *dia;
   
   dia = win->data;
   e_object_del(E_OBJECT(dia));
}

static void
_e_dialog_cb_resize(E_Win *win)
{
   E_Dialog *dia;
   
   dia = win->data;
   evas_object_resize(dia->bg_object, dia->win->w, dia->win->h);
}

static void
_e_dialog_cb_wid_on_focus(void *data, Evas_Object *obj)
{
   E_Dialog *dia;
   
   dia = data;
   if (obj == dia->content_object)
     e_widget_focused_object_clear(dia->box_object);
   else if (dia->content_object)
     e_widget_focused_object_clear(dia->content_object);
}
	  
