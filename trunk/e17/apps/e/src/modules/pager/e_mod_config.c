#include "e.h"
#include "e_mod_main.h"

typedef enum _Pager_Grab_Button Pager_Grab_Button;
enum _Pager_Grab_Button
{
   GRAB_BUTTON_DRAG,
   GRAB_BUTTON_NOPLACE,
   GRAB_BUTTON_DESK
};

struct _E_Config_Dialog_Data
{
   int show_popup;
   double popup_speed;
   int show_popup_urgent;
   int popup_urgent_stick;
   double popup_urgent_speed;
   int popup_pager_height;
   int drag_resist;
   unsigned int btn_drag;
   unsigned int btn_noplace;
   unsigned int btn_desk;
   int flip_desk;

   struct {
      Ecore_X_Window bind_win;
      E_Dialog *dia;
      Evas_List *handlers;
      int btn;
   } grab;
   
   struct {
      Evas_Object *o_urgent_stick;
      Evas_Object *o_urgent_speed;
      Evas_Object *o_btn1;
      Evas_Object *o_btn2;
      Evas_Object *o_btn3;
   } gui;
};

/* Protos */
static void *_create_data(E_Config_Dialog *cfd);
static void _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int _basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object *_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int _advanced_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static void _advanced_update_button_label(E_Config_Dialog_Data *cfdata);
static void _grab_wnd_show(void *data1, void *data2);
static void _grab_wnd_hide(E_Config_Dialog_Data *cfdata);
static int _grab_mouse_down_cb(void *data, int type, void *event);
static int _grab_key_down_cb(void *data, int type, void *event);
static void _check_urgent_stick_cb_change(void *data, Evas_Object *obj);

void 
_config_pager_module(Config_Item *ci)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;
   char buf[4096];
   
   v = E_NEW(E_Config_Dialog_View, 1);
   
   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _basic_apply_data;
   v->basic.create_widgets = _basic_create_widgets;
   v->advanced.apply_cfdata = _advanced_apply_data;
   v->advanced.create_widgets = _advanced_create_widgets;

   snprintf(buf, sizeof(buf), "%s/e-module-pager.edj", e_module_dir_get(pager_config->module));
   cfd = e_config_dialog_new(e_container_current_get(e_manager_current_get()),
			     _("Pager Configuration"),
			     "E", "_e_mod_pager_config_dialog",
			     buf, 0, v, ci);
   pager_config->config_dialog = cfd;
}

static void 
_fill_data(Config_Item *ci, E_Config_Dialog_Data *cfdata) 
{
   /* FIXME: configure zone config item */
   cfdata->show_popup = pager_config->popup;
   cfdata->popup_speed = pager_config->popup_speed;
   cfdata->show_popup_urgent = pager_config->popup_urgent;
   cfdata->popup_urgent_stick = pager_config->popup_urgent_stick;
   cfdata->popup_urgent_speed = pager_config->popup_urgent_speed;
   cfdata->popup_pager_height = pager_config->popup_pager_height;
   cfdata->drag_resist = pager_config->drag_resist;
   cfdata->btn_drag = pager_config->btn_drag;
   cfdata->btn_noplace = pager_config->btn_noplace;
   cfdata->btn_desk = pager_config->btn_desk;
   cfdata->flip_desk = pager_config->flip_desk;
}

static void *
_create_data(E_Config_Dialog *cfd) 
{
   E_Config_Dialog_Data *cfdata;
   Config_Item *ci;
   
   ci = cfd->data;
   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   _fill_data(ci, cfdata);
   return cfdata;
}

static void 
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   pager_config->config_dialog = NULL;
   E_FREE(cfdata);
}

static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *o, *of, *ob;

   o = e_widget_list_add(evas, 0, 0);
   of = e_widget_framelist_add(evas, _("General Settings"), 0);
   ob = e_widget_check_add(evas, _("Show Popup on desktop change"), &(cfdata->show_popup));
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(o, of, 1, 1, 0.5);

   return o;
}

static int 
_basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   pager_config->popup = cfdata->show_popup;
   _pager_cb_config_updated();
   e_config_save_queue();
   return 1;
}

static Evas_Object *
_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata)
{
   Evas_Object *o, *of, *of2, *ob;

   o = e_widget_list_add(evas, 0, 0);
   
   of = e_widget_frametable_add(evas, _("Pager Settings"), 0);
   ob = e_widget_check_add(evas, _("Flip desktop on mouse wheel"), &(cfdata->flip_desk));
   e_widget_frametable_object_append(of, ob, 1, 1, 1, 1, 1, 1, 0, 0);
   ob = e_widget_label_add(evas, _("Select and Slide button"));
   e_widget_frametable_object_append(of, ob, 1, 2, 1, 1, 1, 1, 1, 1);
   ob = e_widget_label_add(evas, _("Drag and Drop button (Keeps rel. loc.)"));
   e_widget_frametable_object_append(of, ob, 1, 3, 1, 1, 1, 1, 1, 1);
   ob = e_widget_label_add(evas, _("Drag whole desktop (Move all windows of a desktop)"));
   e_widget_frametable_object_append(of, ob, 1, 4, 1, 1, 1, 1, 1, 1);
   ob = e_widget_button_add(evas, _("Click to set"), NULL, _grab_wnd_show, (void *)GRAB_BUTTON_DRAG, cfdata);
   e_widget_frametable_object_append(of, ob, 2, 2, 1, 1, 1, 1, 0, 0);
   cfdata->gui.o_btn1 = ob;
   ob = e_widget_button_add(evas, _("Click to set"), NULL, _grab_wnd_show, (void *)GRAB_BUTTON_NOPLACE, cfdata);
   e_widget_frametable_object_append(of, ob, 2, 3, 1, 1, 1, 1, 0, 0);
   cfdata->gui.o_btn2 = ob;
   ob = e_widget_button_add(evas, _("Click to set"), NULL, _grab_wnd_show, (void *)GRAB_BUTTON_DESK, cfdata);
   e_widget_frametable_object_append(of, ob, 2, 4, 1, 1, 1, 1, 0, 0);
   cfdata->gui.o_btn3 = ob;
   _advanced_update_button_label(cfdata);
   ob = e_widget_label_add(evas, _("Resistance to dragging"));
   e_widget_frametable_object_append(of, ob, 1, 5, 1, 1, 1, 1, 0, 0);
   ob = e_widget_slider_add(evas, 1, 0, _("%.0f px"), 0.0, 10.0, 1.0, 0, NULL, &(cfdata->drag_resist), 200);
   e_widget_frametable_object_append(of, ob, 1, 6, 2, 1, 1, 1, 0, 0);
   e_widget_list_object_append(o, of, 1, 1, 0.5);

   of = e_widget_framelist_add(evas, _("Pager Popup Settings"), 0);   
   ob = e_widget_check_add(evas, _("Show Popup on desktop change"), &(cfdata->show_popup));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_label_add(evas, _("Popup Pager Height"));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_slider_add(evas, 1, 0, _("%.0f px"), 20.0, 200.0, 0.1, 0, NULL, &(cfdata->popup_pager_height), 200);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_label_add(evas, _("Popup Speed"));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_slider_add(evas, 1, 0, _("%1.1f seconds"), 0.1, 10.0, 0.1, 0, &(cfdata->popup_speed), NULL, 200);
   e_widget_framelist_object_append(of, ob);

   of2 = e_widget_framelist_add(evas, _("Urgent window"), 0);
   ob = e_widget_check_add(evas, _("Show Popup on urgent window"), &(cfdata->show_popup_urgent));
   e_widget_framelist_object_append(of2, ob);
   ob = e_widget_check_add(evas, _("Popup on urgent window sticks on the screen"), &(cfdata->popup_urgent_stick));
   cfdata->gui.o_urgent_stick = ob;
   e_widget_on_change_hook_set(ob, _check_urgent_stick_cb_change, cfdata);
   e_widget_framelist_object_append(of2, ob);
   ob = e_widget_label_add(evas, _("Popup Speed"));
   e_widget_framelist_object_append(of2, ob);
   ob = e_widget_slider_add(evas, 1, 0, _("%1.1f seconds"), 0.1, 10.0, 0.1, 0, &(cfdata->popup_urgent_speed), NULL, 200);
   cfdata->gui.o_urgent_speed = ob;
   _check_urgent_stick_cb_change(cfdata, cfdata->gui.o_urgent_stick);
   e_widget_framelist_object_append(of2, ob);

   e_widget_framelist_object_append(of, of2);
   e_widget_list_object_append(o, of, 1, 1, 0.5);

   return o;
}

static int 
_advanced_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata)
{
   pager_config->popup = cfdata->show_popup;
   pager_config->popup_speed = cfdata->popup_speed;
   pager_config->popup_urgent = cfdata->show_popup_urgent;
   pager_config->popup_urgent_stick = cfdata->popup_urgent_stick;
   pager_config->popup_urgent_speed = cfdata->popup_urgent_speed;
   pager_config->popup_pager_height = cfdata->popup_pager_height;
   pager_config->drag_resist = cfdata->drag_resist;
   pager_config->btn_drag = cfdata->btn_drag;
   pager_config->btn_noplace = cfdata->btn_noplace;
   pager_config->btn_desk = cfdata->btn_desk;
   pager_config->flip_desk = cfdata->flip_desk;
   _pager_cb_config_updated();
   e_config_save_queue();
   return 1;
}

static void
_advanced_update_button_label(E_Config_Dialog_Data *cfdata)
{
   char label[256] = "";
   
   if (cfdata->btn_drag)
     snprintf(label, sizeof(label), _("Button %i"), cfdata->btn_drag);
   else
     snprintf(label, sizeof(label), _("Click to set"));
   e_widget_button_label_set(cfdata->gui.o_btn1, label);
   
   if (cfdata->btn_noplace)
     snprintf(label, sizeof(label), _("Button %i"), cfdata->btn_noplace);
   else
     snprintf(label, sizeof(label), _("Click to set"));
   e_widget_button_label_set(cfdata->gui.o_btn2, label);
   
   if (cfdata->btn_desk)
     snprintf(label, sizeof(label), _("Button %i"), cfdata->btn_desk);
   else
     snprintf(label, sizeof(label), _("Click to set"));
   e_widget_button_label_set(cfdata->gui.o_btn3, label);
}

static void
_grab_wnd_show(void *data1, void *data2)
{
   E_Manager *man;
   E_Config_Dialog_Data *cfdata;

   man = e_manager_current_get();
   cfdata = data2;

   if ((Pager_Grab_Button)data1 == GRAB_BUTTON_DRAG)
     cfdata->grab.btn = 1;
   else if ((Pager_Grab_Button)data1 == GRAB_BUTTON_NOPLACE)
     cfdata->grab.btn = 2;
   else
     cfdata->grab.btn = 0;

   cfdata->grab.dia = e_dialog_new(e_container_current_get(man), "Pager", "_pager_button_grab_dialog");
   if (!cfdata->grab.dia) return;
   e_dialog_title_set(cfdata->grab.dia, _("Pager Button Grab"));
   e_dialog_icon_set(cfdata->grab.dia, "enlightenment/mouse_clean", 48);
   e_dialog_text_set(cfdata->grab.dia, _("Please press a mouse button<br>"
					 "Press <hilight>Escape</hilight> to abort.<br>"
					 "Or <hilight>Del</hilight> to reset the button."));
   e_win_centered_set(cfdata->grab.dia->win, 1);
   e_win_borderless_set(cfdata->grab.dia->win, 1);

   cfdata->grab.bind_win = ecore_x_window_input_new(man->root, 0, 0, man->w, man->h);
   ecore_x_window_show(cfdata->grab.bind_win);
   e_grabinput_get(cfdata->grab.bind_win, 0, cfdata->grab.bind_win);

   cfdata->grab.handlers = evas_list_append(cfdata->grab.handlers,
			    ecore_event_handler_add(ECORE_X_EVENT_KEY_DOWN,
				 _grab_key_down_cb, cfdata));
   cfdata->grab.handlers = evas_list_append(cfdata->grab.handlers,
			      ecore_event_handler_add(ECORE_X_EVENT_MOUSE_BUTTON_DOWN,
				 _grab_mouse_down_cb, cfdata));

   e_dialog_show(cfdata->grab.dia);
   ecore_x_icccm_transient_for_set(cfdata->grab.dia->win->evas_win, pager_config->config_dialog->dia->win->evas_win);
}

static void
_grab_wnd_hide(E_Config_Dialog_Data *cfdata)
{
   while (cfdata->grab.handlers)
     {
	ecore_event_handler_del(cfdata->grab.handlers->data);
	cfdata->grab.handlers = evas_list_remove_list(cfdata->grab.handlers, cfdata->grab.handlers);
     }
   cfdata->grab.handlers = NULL;
   e_grabinput_release(cfdata->grab.bind_win, cfdata->grab.bind_win);
   ecore_x_window_del(cfdata->grab.bind_win);
   cfdata->grab.bind_win = 0;

   e_object_del(E_OBJECT(cfdata->grab.dia));
   cfdata->grab.dia = NULL;
   _advanced_update_button_label(cfdata);
}

static int
_grab_mouse_down_cb(void *data, int type, void *event)
{
   E_Config_Dialog_Data *cfdata;
   Ecore_X_Event_Mouse_Button_Down *ev;
   
   ev = event;
   cfdata = data;

   if (ev->button != 3)
     {
	if (cfdata->grab.btn == 1)
	  cfdata->btn_drag = ev->button;
	else if (cfdata->grab.btn == 2)
	  cfdata->btn_noplace = ev->button;
	else
	  cfdata->btn_desk = ev->button;
     }
   else
     {
	e_util_dialog_show(_("Error - Invalid Button"),
			   _("You cannot use the right mouse button<br>"
			     "for this as it is already taken by internal<br>"
			     "code for context menus."));
     }

   _grab_wnd_hide(cfdata);
   return 1;
}

static int
_grab_key_down_cb(void *data, int type, void *event)
{
   E_Config_Dialog_Data *cfdata;
   Ecore_X_Event_Key_Down *ev = event;

   cfdata = data;

   if (ev->win != cfdata->grab.bind_win) return 1;

   if (!strcmp(ev->keyname, "Escape")) _grab_wnd_hide(cfdata);
   if (!strcmp(ev->keyname, "Delete"))
     {
	if (cfdata->grab.btn == 1)
	  cfdata->btn_drag = 0;
	else if (cfdata->grab.btn == 2)
	  cfdata->btn_noplace = 0;
	else
	  cfdata->btn_desk = 0;
	_grab_wnd_hide(cfdata);
     }
   return 1;
}

static void
_check_urgent_stick_cb_change(void *data, Evas_Object *obj)
{
   E_Config_Dialog_Data *cfdata;

   cfdata = data;

   if (e_widget_check_checked_get(cfdata->gui.o_urgent_stick))
     e_widget_disabled_set(cfdata->gui.o_urgent_speed, 1);
   else
     e_widget_disabled_set(cfdata->gui.o_urgent_speed, 0);
}
