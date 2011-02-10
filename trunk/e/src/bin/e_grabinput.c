#include "e.h"

/* local subsystem functions */
static void _e_grabinput_focus_job(void *data);
static void _e_grabinput_focus(Ecore_X_Window win, E_Focus_Method method);

/* local subsystem globals */
static Ecore_X_Window grab_mouse_win = 0;
static Ecore_X_Window grab_key_win = 0;
static Ecore_X_Window focus_win = 0;
static E_Focus_Method focus_method = E_FOCUS_METHOD_NO_INPUT;
static Ecore_X_Window do_focus_win = 0;
static E_Focus_Method do_focus_method = E_FOCUS_METHOD_NO_INPUT;
static Ecore_Job *focus_job = NULL;
static double last_focus_time = 0.0;

/* externally accessible functions */
EINTERN int
e_grabinput_init(void)
{
   return 1;
}

EINTERN int
e_grabinput_shutdown(void)
{
   return 1;
}

EAPI int
e_grabinput_get(Ecore_X_Window mouse_win, int confine_mouse, Ecore_X_Window key_win)
{
   if (grab_mouse_win)
     {
	ecore_x_pointer_ungrab();
	grab_mouse_win = 0;
     }
   if (grab_key_win)
     {
	ecore_x_keyboard_ungrab();
	grab_key_win = 0;
	focus_win = 0;
     }
   if (mouse_win)
     {
	int ret = 0;
	
	if (confine_mouse)
	  ret = ecore_x_pointer_confine_grab(mouse_win);
	else
	  ret = ecore_x_pointer_grab(mouse_win);
	if (!ret) return 0;
	grab_mouse_win = mouse_win;
     }
   if (key_win)
     {
	int ret = 0;
	
	ret = ecore_x_keyboard_grab(key_win);
	if (!ret)
	  {
	     if (grab_mouse_win)
	       {
		  ecore_x_pointer_ungrab();
		  grab_mouse_win = 0;
	       }
	     return 0;
	  }
	grab_key_win = key_win;
     }
   return 1;
}

EAPI void
e_grabinput_release(Ecore_X_Window mouse_win, Ecore_X_Window key_win)
{
   if (mouse_win == grab_mouse_win)
     {
	ecore_x_pointer_ungrab();
	grab_mouse_win = 0;
     }
   if (key_win == grab_key_win)
     {
	ecore_x_keyboard_ungrab();
	grab_key_win = 0;
	if (focus_win != 0)
	  {
	     _e_grabinput_focus(focus_win, focus_method);
	     focus_win = 0;
	     focus_method = E_FOCUS_METHOD_NO_INPUT;
	  }
     }
}

EAPI void
e_grabinput_focus(Ecore_X_Window win, E_Focus_Method method)
{
   if (grab_key_win != 0)
     {
	focus_win = win;
	focus_method = method;
     }
   else
     {
        do_focus_win = win;
        do_focus_method = method;
        if (!focus_job)
          focus_job = ecore_job_add(_e_grabinput_focus_job, NULL);
	//_e_grabinput_focus(win, method);
     }
}

EAPI double
e_grabinput_last_focus_time_get(void)
{
   return last_focus_time;
}

/* local subsystem functions */
static void
_e_grabinput_focus_job(void *data __UNUSED__)
{
   focus_job = NULL;
   _e_grabinput_focus(do_focus_win, do_focus_method);
   do_focus_win = 0;
}

static void
_e_grabinput_focus(Ecore_X_Window win, E_Focus_Method method)
{
   switch (method)
     {
      case E_FOCUS_METHOD_NO_INPUT:
	break;
      case E_FOCUS_METHOD_LOCALLY_ACTIVE:
	ecore_x_window_focus(win);
        ecore_x_sync(); // let x actually get the x focus request
	ecore_x_icccm_take_focus_send(win, ecore_x_current_time_get());
        ecore_x_sync(); // let x actually get the x focus request
	break;
      case E_FOCUS_METHOD_GLOBALLY_ACTIVE:
	ecore_x_icccm_take_focus_send(win, ecore_x_current_time_get());
        ecore_x_sync(); // let x actually get the x focus request
	break;
      case E_FOCUS_METHOD_PASSIVE:
	ecore_x_window_focus(win);
        ecore_x_sync(); // let x actually get the x focus request
	break;
      default:
	break;
     }
   last_focus_time = ecore_loop_time_get();
}
