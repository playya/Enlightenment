/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

static Ecore_Event_Handler *_e_screensaver_handler_config_mode = NULL;
static Ecore_Event_Handler *_e_screensaver_handler_screensaver_notify = NULL;
static E_Dialog *_e_screensaver_ask_presentation_dia = NULL;
static int _e_screensaver_ask_presentation_count = 0;

static int
_e_screensaver_handler_config_mode_cb(void *data __UNUSED__, int type __UNUSED__, void *event __UNUSED__)
{
   e_screensaver_init();
   return 1;
}

static void
_e_screensaver_ask_presentation_del(void *data)
{
   if (_e_screensaver_ask_presentation_dia == data)
     _e_screensaver_ask_presentation_dia = NULL;
}

static void
_e_screensaver_ask_presentation_yes(void *data __UNUSED__, E_Dialog *dia)
{
   e_config->mode.presentation = 1;
   e_config_mode_changed();
   e_config_save_queue();
   e_object_del(E_OBJECT(dia));
   _e_screensaver_ask_presentation_count = 0;
}

static void
_e_screensaver_ask_presentation_no(void *data __UNUSED__, E_Dialog *dia)
{
   e_object_del(E_OBJECT(dia));
   _e_screensaver_ask_presentation_count = 0;
}

static void
_e_screensaver_ask_presentation_no_increase(void *data __UNUSED__, E_Dialog *dia)
{
   int timeout, interval, blanking, expose;

   _e_screensaver_ask_presentation_count++;
   timeout = e_config->screensaver_timeout * _e_screensaver_ask_presentation_count;
   interval = e_config->screensaver_interval;
   blanking = e_config->screensaver_blanking;
   expose = e_config->screensaver_expose;

   ecore_x_screensaver_set(timeout, interval, blanking, expose);
   e_object_del(E_OBJECT(dia));
}

static void
_e_screensaver_ask_presentation_no_forever(void *data __UNUSED__, E_Dialog *dia)
{
   e_config->screensaver_ask_presentation = 0;
   e_config_save_queue();
   e_object_del(E_OBJECT(dia));
   _e_screensaver_ask_presentation_count = 0;
}

static void
_e_screensaver_ask_presentation_mode(void)
{
   E_Manager *man;
   E_Container *con;
   E_Dialog *dia;

   if (_e_screensaver_ask_presentation_dia)
     return;

   man = e_manager_current_get();
   if (!man) return;
   con = e_container_current_get(man);
   if (!con) return;
   dia = e_dialog_new(con, "E", "_screensaver_ask_presentation");
   if (!dia) return;

   e_dialog_title_set(dia, _("Activate Presentation Mode?"));
   e_dialog_icon_set(dia, "dialog-ask", 64);
   e_dialog_text_set
     (dia,
      _("You disabled screensaver too fast.<br><br>"
	"Would you like to enabe <b>presentation</b> mode and "
	"temporarily disable screen saver, lock and power saving?"));

   e_object_del_attach_func_set
     (E_OBJECT(dia), _e_screensaver_ask_presentation_del);
   e_dialog_button_add
     (dia, _("Yes"), NULL, _e_screensaver_ask_presentation_yes, NULL);
   e_dialog_button_add
     (dia, _("No"), NULL, _e_screensaver_ask_presentation_no, NULL);
   e_dialog_button_add
     (dia, _("No, but increase timeout"), NULL,
      _e_screensaver_ask_presentation_no_increase, NULL);
   e_dialog_button_add
     (dia, _("No, and stop asking"), NULL,
      _e_screensaver_ask_presentation_no_forever, NULL);

   e_dialog_button_focus_num(dia, 0);
   e_dialog_resizable_set(dia, 1);
   e_widget_list_homogeneous_set(dia->box_object, 0);
   e_util_win_auto_resize_fill(dia->win);
   e_win_centered_set(dia->win, 1);
   e_dialog_show(dia);

   _e_screensaver_ask_presentation_dia = dia;
}

static int
_e_screensaver_handler_screensaver_notify_cb(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_X_Event_Screensaver_Notify *e = event;
   static double last_start = 0.0;

   if (e->on)
     {
	last_start = ecore_loop_time_get();
	_e_screensaver_ask_presentation_count = 0;
     }
   else if ((last_start > 0.0) && (e_config->screensaver_ask_presentation))
     {
	double current = ecore_loop_time_get();
	if (last_start + e_config->screensaver_ask_presentation_timeout >= current)
	  _e_screensaver_ask_presentation_mode();
	last_start = 0.0;
     }
   else if (_e_screensaver_ask_presentation_count)
     _e_screensaver_ask_presentation_count = 0;

   return 1;
}

EAPI int
e_screensaver_init(void)
{
   int timeout=0, interval=0, blanking=0, expose=0;

   if (!_e_screensaver_handler_config_mode)
     _e_screensaver_handler_config_mode = ecore_event_handler_add
       (E_EVENT_CONFIG_MODE_CHANGED, _e_screensaver_handler_config_mode_cb, NULL);

   if (!_e_screensaver_handler_screensaver_notify)
     _e_screensaver_handler_screensaver_notify = ecore_event_handler_add
       (ECORE_X_EVENT_SCREENSAVER_NOTIFY, _e_screensaver_handler_screensaver_notify_cb, NULL);

   if ((e_config->screensaver_enable) && (!e_config->mode.presentation))
     timeout = e_config->screensaver_timeout;
   
   interval = e_config->screensaver_interval;
   blanking = e_config->screensaver_blanking;
   expose = e_config->screensaver_expose;
  
   ecore_x_screensaver_set(timeout, interval, blanking, expose);
   return 1;
}
