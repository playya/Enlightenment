/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "ecore_private.h"
#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"
#include "Ecore_X_Atoms.h"

#if 0
static void _ecore_x_event_free_window_prop_name_class_change(void *data, void *ev);
static void _ecore_x_event_free_window_prop_title_change(void *data, void *ev);
static void _ecore_x_event_free_window_prop_visible_title_change(void *data, void *ev);
static void _ecore_x_event_free_window_prop_icon_name_change(void *data, void *ev);
static void _ecore_x_event_free_window_prop_visible_icon_name_change(void *data, void *ev);
static void _ecore_x_event_free_window_prop_client_machine_change(void *data, void *ev);
#endif
static void _ecore_x_event_free_key_down(void *data, void *ev);
static void _ecore_x_event_free_key_up(void *data, void *ev);

void
ecore_x_event_mask_set(Ecore_X_Window w, Ecore_X_Event_Mask mask)
{
   XWindowAttributes attr;
   XSetWindowAttributes s_attr;

   if (!w)
      w = DefaultRootWindow(_ecore_x_disp);

   memset(&attr, 0, sizeof(XWindowAttributes));
   XGetWindowAttributes(_ecore_x_disp, w, &attr);
   s_attr.event_mask = mask | attr.your_event_mask;
   XChangeWindowAttributes(_ecore_x_disp, w, CWEventMask, &s_attr);
}

void
ecore_x_event_mask_unset(Ecore_X_Window w, Ecore_X_Event_Mask mask)
{
   XWindowAttributes attr;
   XSetWindowAttributes s_attr;

   if (!w)
      w = DefaultRootWindow(_ecore_x_disp);

   memset(&attr, 0, sizeof(XWindowAttributes));
   XGetWindowAttributes(_ecore_x_disp, w, &attr);
   s_attr.event_mask = attr.your_event_mask & ~mask;
   XChangeWindowAttributes(_ecore_x_disp, w, CWEventMask, &s_attr);
}

#if 0
static void
_ecore_x_event_free_window_prop_name_class_change(void *data, void *ev)
{
   Ecore_X_Event_Window_Prop_Name_Class_Change *e;
   
   e = ev;
   if (e->name) free(e->name);
   if (e->clas) free(e->clas);
   free(e);
}

static void
_ecore_x_event_free_window_prop_title_change(void *data, void *ev)
{
   Ecore_X_Event_Window_Prop_Title_Change *e;
   
   e = ev;
   if (e->title) free(e->title);
   free(e);
}

static void
_ecore_x_event_free_window_prop_visible_title_change(void *data, void *ev)
{
   Ecore_X_Event_Window_Prop_Visible_Title_Change *e;
   
   e = ev;
   if (e->title) free(e->title);
   free(e);
}

static void
_ecore_x_event_free_window_prop_icon_name_change(void *data, void *ev)
{
   Ecore_X_Event_Window_Prop_Icon_Name_Change *e;
   
   e = ev;
   if (e->name) free(e->name);
   free(e);
}

static void
_ecore_x_event_free_window_prop_visible_icon_name_change(void *data, void *ev)
{
   Ecore_X_Event_Window_Prop_Visible_Icon_Name_Change *e;
   
   e = ev;
   if (e->name) free(e->name);
   free(e);
}

static void
_ecore_x_event_free_window_prop_client_machine_change(void *data, void *ev)
{
   Ecore_X_Event_Window_Prop_Client_Machine_Change *e;
   
   e = ev;
   if (e->name) free(e->name);
   free(e);
}
#endif

static void
_ecore_x_event_free_key_down(void *data __UNUSED__, void *ev)
{
   Ecore_X_Event_Key_Down *e;

   e = ev;
   if (e->keyname) free(e->keyname);
   if (e->keysymbol) free(e->keysymbol);
   if (e->key_compose) free(e->key_compose);
   free(e);
}

static void
_ecore_x_event_free_key_up(void *data __UNUSED__, void *ev)
{
   Ecore_X_Event_Key_Up *e;

   e = ev;
   if (e->keyname) free(e->keyname);
   if (e->keysymbol) free(e->keysymbol);
   if (e->key_compose) free(e->key_compose);
   free(e);
}

static void
_ecore_x_event_free_xdnd_enter(void *data __UNUSED__, void *ev)
{
   Ecore_X_Event_Xdnd_Enter *e;
   int i;

   e = ev;
   for (i = 0; i < e->num_types; i++)
     XFree(e->types[i]);
   free(e->types);
   free(e);
}

static void
_ecore_x_event_free_selection_notify(void *data __UNUSED__, void *ev)
{
   Ecore_X_Event_Selection_Notify *e;
   int i;

   e = ev;
   switch (e->content)
     {
      case ECORE_X_SELECTION_NONE:
	 break;
      case ECORE_X_SELECTION_FILES:
	 for (i = 0; i < e->num_files; i++)
	   free(e->files[i]);
	 free(e->files);
	 break;
      case ECORE_X_SELECTION_TEXT:
	 free(e->text);
	 break;
     }
   free(e->target);
   free(e);
}

void
_ecore_x_event_handle_key_press(XEvent *xevent)
{
   Ecore_X_Event_Key_Down *e;
   char                   *keyname;
   int                     val;
   char                    buf[256];
   KeySym                  sym;
   XComposeStatus          stat;
   
   e = calloc(1, sizeof(Ecore_X_Event_Key_Down));
   if (!e) return;
   keyname = XKeysymToString(XKeycodeToKeysym(xevent->xkey.display, 
					      xevent->xkey.keycode, 0));
   if (!keyname)
     {
	snprintf(buf, sizeof(buf), "Keycode-%i", xevent->xkey.keycode);
	keyname = buf;
     }
   e->keyname = strdup(keyname);
   if (!e->keyname)
     {
	free(e);
	return;
     }
   val = XLookupString((XKeyEvent *)xevent, buf, sizeof(buf), &sym, &stat);
   if (val > 0)
     {
	buf[val] = 0;
	e->key_compose = ecore_txt_convert("LATIN1", "UTF-8", buf);
     }
   else e->key_compose = NULL;
   keyname = XKeysymToString(sym);
   if (keyname) e->keysymbol = strdup(keyname);
   else e->keysymbol = strdup(e->keyname);
   if (!e->keysymbol)
     {
	if (e->keyname) free(e->keyname);
	if (e->key_compose) free(e->key_compose);
	free(e);
	return;
     }
   if (xevent->xkey.subwindow) e->win = xevent->xkey.subwindow;
   else e->win = xevent->xkey.window;
   e->event_win = xevent->xkey.window;
   e->time = xevent->xkey.time;
   e->modifiers = xevent->xkey.state;
   _ecore_x_event_last_time = e->time;
   ecore_event_add(ECORE_X_EVENT_KEY_DOWN, e, _ecore_x_event_free_key_down, NULL);
}

void
_ecore_x_event_handle_key_release(XEvent *xevent)
{
   Ecore_X_Event_Key_Up *e;
   char                   *keyname;
   int                     val;
   char                    buf[256];
   KeySym                  sym;
   XComposeStatus          stat;
   
   e = calloc(1, sizeof(Ecore_X_Event_Key_Up));
   if (!e) return;
   keyname = XKeysymToString(XKeycodeToKeysym(xevent->xkey.display, 
					      xevent->xkey.keycode, 0));
   if (!keyname)
     {
	snprintf(buf, sizeof(buf), "Keycode-%i", xevent->xkey.keycode);
	keyname = buf;
     }
   e->keyname = strdup(keyname);
   if (!e->keyname)
     {
	free(e);
	return;
     }
   val = XLookupString((XKeyEvent *)xevent, buf, sizeof(buf), &sym, &stat);
   if (val > 0)
     {
	buf[val] = 0;
	e->key_compose = ecore_txt_convert("LATIN1", "UTF-8", buf);
     }
   else e->key_compose = NULL;
   keyname = XKeysymToString(sym);
   if (keyname) e->keysymbol = strdup(keyname);
   else e->keysymbol = strdup(e->keyname);
   if (!e->keysymbol)
     {
	if (e->keyname) free(e->keyname);
	if (e->key_compose) free(e->key_compose);
	free(e);
	return;
     }
   if (xevent->xkey.subwindow) e->win = xevent->xkey.subwindow;
   else e->win = xevent->xkey.window;
   e->event_win = xevent->xkey.window;
   e->time = xevent->xkey.time;
   e->modifiers = xevent->xkey.state;
   _ecore_x_event_last_time = e->time;
   ecore_event_add(ECORE_X_EVENT_KEY_UP, e, _ecore_x_event_free_key_up, NULL);
}

void
_ecore_x_event_handle_button_press(XEvent *xevent)
{
   static Window last_win = 0;
   static Window last_last_win = 0;
   static Window last_event_win = 0;
   static Window last_last_event_win = 0;
   static Time last_time = 0;
   static Time last_last_time = 0;
   int did_triple = 0;
   int i;
   
   if ((xevent->xbutton.button > 3) && (xevent->xbutton.button < 6))
     {
	Ecore_X_Event_Mouse_Wheel *e;
	
	e = malloc(sizeof(Ecore_X_Event_Mouse_Wheel));
	
	if (!e)
	  return;
	
	e->modifiers = 0;
	e->direction = 0;
	e->z = 0;
	if      (xevent->xbutton.button == 4) e->z = -1;
	else if (xevent->xbutton.button == 5) e->z = 1;
	e->x = xevent->xbutton.x;
	e->y = xevent->xbutton.y;
	e->root.x = xevent->xbutton.x_root;
         e->root.y = xevent->xbutton.y_root;
	
	if (xevent->xbutton.subwindow)
	  e->win = xevent->xbutton.subwindow;
	else
	  e->win = xevent->xbutton.window;
	
	e->event_win = xevent->xbutton.window;
	e->time = xevent->xbutton.time;
	_ecore_x_event_last_time = e->time;
	_ecore_x_event_last_win = e->win;
	_ecore_x_event_last_root_x = e->root.x;
	_ecore_x_event_last_root_y = e->root.y;
	ecore_event_add(ECORE_X_EVENT_MOUSE_WHEEL, e, NULL, NULL);
	for (i = 0; i < _ecore_window_grabs_num; i++)
	  {
	     if ((_ecore_window_grabs[i] == xevent->xbutton.window) ||
		 (_ecore_window_grabs[i] == xevent->xbutton.subwindow))
	       {
		  int replay = 0;
		  
		  if (_ecore_window_grab_replay_func)
	       replay = _ecore_window_grab_replay_func(_ecore_window_grab_replay_data, 
						       ECORE_X_EVENT_MOUSE_WHEEL,
						       e);
		  if (replay)
		    XAllowEvents(xevent->xbutton.display,
				 ReplayPointer,
				 xevent->xbutton.time);
		  else
		    XAllowEvents(xevent->xbutton.display,
				 AsyncPointer,
				 xevent->xbutton.time);
		  break;
	       }
	  }
     }
   else
     {
	  {
	     Ecore_X_Event_Mouse_Move *e;
	     
	     e = calloc(1, sizeof(Ecore_X_Event_Mouse_Move));
	     if (!e) return;
	     e->modifiers = xevent->xbutton.state;
	     e->x = xevent->xbutton.x;
	     e->y = xevent->xbutton.y;
	     e->root.x = xevent->xbutton.x_root;
	     e->root.y = xevent->xbutton.y_root;
	     if (xevent->xbutton.subwindow) e->win = xevent->xbutton.subwindow;
	     else e->win = xevent->xbutton.window;
	     e->event_win = xevent->xbutton.window;
	     e->time = xevent->xbutton.time;
	     _ecore_x_event_last_time = e->time;
	     _ecore_x_event_last_win = e->win;
	     _ecore_x_event_last_root_x = e->root.x;
	     _ecore_x_event_last_root_y = e->root.y;
	     ecore_event_add(ECORE_X_EVENT_MOUSE_MOVE, e, NULL, NULL);
	  }
	  {
	     Ecore_X_Event_Mouse_Button_Down *e;
	     
	     e = calloc(1, sizeof(Ecore_X_Event_Mouse_Button_Down));
	     if (!e) return;
	     e->button = xevent->xbutton.button;
	     e->modifiers = xevent->xbutton.state;
	     e->x = xevent->xbutton.x;
	     e->y = xevent->xbutton.y;
	     e->root.x = xevent->xbutton.x_root;
	     e->root.y = xevent->xbutton.y_root;
	     if (xevent->xbutton.subwindow) e->win = xevent->xbutton.subwindow;
	     else e->win = xevent->xbutton.window;
	     e->event_win = xevent->xbutton.window;
	     e->time = xevent->xbutton.time;
	     if (((int)(e->time - last_time) <= 
		  (int)(1000 * _ecore_x_double_click_time)) &&
		 (e->win == last_win) &&
		 (e->event_win == last_event_win))
	       e->double_click = 1;
	     if (((int)(e->time - last_last_time) <= 
		  (int)(2 * 1000 * _ecore_x_double_click_time)) &&
		 (e->win == last_win) && (e->win == last_last_win) &&
		 (e->event_win == last_event_win) && (e->event_win == last_last_event_win))
	       {
		  did_triple = 1;
		  e->triple_click = 1;
	       }
	     _ecore_x_event_last_time = e->time;
	     _ecore_x_event_last_win = e->win;
	     _ecore_x_event_last_root_x = e->root.x;
	     _ecore_x_event_last_root_y = e->root.y;
	     ecore_event_add(ECORE_X_EVENT_MOUSE_BUTTON_DOWN, e, NULL, NULL);
	     for (i = 0; i < _ecore_window_grabs_num; i++)
	       {
		  if ((_ecore_window_grabs[i] == xevent->xbutton.window) ||
		      (_ecore_window_grabs[i] == xevent->xbutton.subwindow))
		    {
		       int replay = 0;
		       
		       if (_ecore_window_grab_replay_func)
			 replay = _ecore_window_grab_replay_func(_ecore_window_grab_replay_data, 
								 ECORE_X_EVENT_MOUSE_BUTTON_DOWN,
								 e);
		       if (replay)
			 XAllowEvents(xevent->xbutton.display,
				      ReplayPointer,
				      xevent->xbutton.time);
		       else
			 XAllowEvents(xevent->xbutton.display,
				      AsyncPointer,
				      xevent->xbutton.time);
		       break;
		    }
	       }
	  }
	if (did_triple)
	  {
	     last_win = 0;
	     last_last_win = 0;
	     last_event_win = 0;
	     last_last_event_win = 0;
	     last_time = 0;
	     last_last_time = 0;
	  }
	else
	  {
	     last_last_win = last_win;
	     if (xevent->xbutton.subwindow)
	       last_win = xevent->xbutton.subwindow;
	     else
	       last_win = xevent->xbutton.window;
	     last_last_event_win = last_event_win;
	     last_event_win = xevent->xbutton.window;
	     last_last_time = last_time;
	     last_time = xevent->xbutton.time;
	  }
     }
}

void
_ecore_x_event_handle_button_release(XEvent *xevent)
{
   /* filter out wheel buttons */
   if (xevent->xbutton.button <= 3)
     {
	  {
	     Ecore_X_Event_Mouse_Move *e;
	     
	     e = calloc(1, sizeof(Ecore_X_Event_Mouse_Move));
	     if (!e) return;
	     e->modifiers = xevent->xbutton.state;
	     e->x = xevent->xbutton.x;
	     e->y = xevent->xbutton.y;
	     e->root.x = xevent->xbutton.x_root;
	     e->root.y = xevent->xbutton.y_root;
	     if (xevent->xbutton.subwindow) e->win = xevent->xbutton.subwindow;
	     else e->win = xevent->xbutton.window;
	     e->event_win = xevent->xbutton.window;
	     e->time = xevent->xbutton.time;
	     _ecore_x_event_last_time = e->time;
	     _ecore_x_event_last_win = e->win;
	     _ecore_x_event_last_root_x = e->root.x;
	     _ecore_x_event_last_root_y = e->root.y;
	     ecore_event_add(ECORE_X_EVENT_MOUSE_MOVE, e, NULL, NULL);
	  }
	  {
	     Ecore_X_Event_Mouse_Button_Up *e;
	     
	     e = calloc(1, sizeof(Ecore_X_Event_Mouse_Button_Up));
	     if (!e) return;
	     e->button = xevent->xbutton.button;
	     e->modifiers = xevent->xbutton.state;
	     e->x = xevent->xbutton.x;
	     e->y = xevent->xbutton.y;
	     e->root.x = xevent->xbutton.x_root;
	     e->root.y = xevent->xbutton.y_root;
	     if (xevent->xbutton.subwindow) e->win = xevent->xbutton.subwindow;
	     else e->win = xevent->xbutton.window;
	     e->event_win = xevent->xbutton.window;
	     e->time = xevent->xbutton.time;
	     _ecore_x_event_last_time = e->time;
	     _ecore_x_event_last_win = e->win;
	     _ecore_x_event_last_root_x = e->root.x;
	     _ecore_x_event_last_root_y = e->root.y;
	     ecore_event_add(ECORE_X_EVENT_MOUSE_BUTTON_UP, e, NULL, NULL);
	  }
     }
}

void
_ecore_x_event_handle_motion_notify(XEvent *xevent)
{
   Ecore_X_Event_Mouse_Move     *e;

   e = calloc(1, sizeof(Ecore_X_Event_Mouse_Move));
   if (!e) return;
   e->modifiers = xevent->xmotion.state;
   e->x = xevent->xmotion.x;
   e->y = xevent->xmotion.y;
   e->root.x = xevent->xmotion.x_root;
   e->root.y = xevent->xmotion.y_root;
   if (xevent->xmotion.subwindow) e->win = xevent->xmotion.subwindow;
   else e->win = xevent->xmotion.window;
   e->event_win = xevent->xmotion.window;
   e->time = xevent->xmotion.time;
   _ecore_x_event_last_time = e->time;
   _ecore_x_event_last_win = e->win;
   _ecore_x_event_last_root_x = e->root.x;
   _ecore_x_event_last_root_y = e->root.y;

   /* Xdnd handling */
   _ecore_x_dnd_drag(e->root.x, e->root.y);

   ecore_event_add(ECORE_X_EVENT_MOUSE_MOVE, e, NULL, NULL);
}

void
_ecore_x_event_handle_enter_notify(XEvent *xevent)
{
     {
	Ecore_X_Event_Mouse_Move *e;
	
	e = calloc(1, sizeof(Ecore_X_Event_Mouse_Move));
	if (!e) return;
	e->modifiers = xevent->xcrossing.state;
	e->x = xevent->xcrossing.x;
	e->y = xevent->xcrossing.y;
	e->root.x = xevent->xcrossing.x_root;
	e->root.y = xevent->xcrossing.y_root;
	if (xevent->xcrossing.subwindow) e->win = xevent->xcrossing.subwindow;
	else e->win = xevent->xcrossing.window;
	e->event_win = xevent->xcrossing.window;
	e->time = xevent->xcrossing.time;
	_ecore_x_event_last_time = e->time;
	_ecore_x_event_last_win = e->win;
	_ecore_x_event_last_root_x = e->root.x;
	_ecore_x_event_last_root_y = e->root.y;
	ecore_event_add(ECORE_X_EVENT_MOUSE_MOVE, e, NULL, NULL);
     }
     {
	Ecore_X_Event_Mouse_In *e;
	
	e = calloc(1, sizeof(Ecore_X_Event_Mouse_In));
	if (!e) return;
	e->modifiers = xevent->xcrossing.state;
	e->x = xevent->xcrossing.x;
	e->y = xevent->xcrossing.y;
	e->root.x = xevent->xcrossing.x_root;
	e->root.y = xevent->xcrossing.y_root;
	if (xevent->xcrossing.subwindow) e->win = xevent->xcrossing.subwindow;
	else e->win = xevent->xcrossing.window;
	e->event_win = xevent->xcrossing.window;
	if      (xevent->xcrossing.mode == NotifyNormal) e->mode = ECORE_X_EVENT_MODE_NORMAL;
	else if (xevent->xcrossing.mode == NotifyGrab)   e->mode = ECORE_X_EVENT_MODE_GRAB;
	else if (xevent->xcrossing.mode == NotifyUngrab) e->mode = ECORE_X_EVENT_MODE_UNGRAB;
	if      (xevent->xcrossing.detail == NotifyAncestor)         e->detail = ECORE_X_EVENT_DETAIL_ANCESTOR;
	else if (xevent->xcrossing.detail == NotifyVirtual)          e->detail = ECORE_X_EVENT_DETAIL_VIRTUAL;
	else if (xevent->xcrossing.detail == NotifyInferior)         e->detail = ECORE_X_EVENT_DETAIL_INFERIOR;
	else if (xevent->xcrossing.detail == NotifyNonlinear)        e->detail = ECORE_X_EVENT_DETAIL_NON_LINEAR;
	else if (xevent->xcrossing.detail == NotifyNonlinearVirtual) e->detail = ECORE_X_EVENT_DETAIL_NON_LINEAR_VIRTUAL;
	e->time = xevent->xcrossing.time;
	_ecore_x_event_last_time = e->time;
	ecore_event_add(ECORE_X_EVENT_MOUSE_IN, e, NULL, NULL);
     }
}

void
_ecore_x_event_handle_leave_notify(XEvent *xevent)
{
     {
	Ecore_X_Event_Mouse_Move *e;
	
	e = calloc(1, sizeof(Ecore_X_Event_Mouse_Move));
	if (!e) return;
	e->modifiers = xevent->xcrossing.state;
	e->x = xevent->xcrossing.x;
	e->y = xevent->xcrossing.y;
	e->root.x = xevent->xcrossing.x_root;
	e->root.y = xevent->xcrossing.y_root;
	if (xevent->xcrossing.subwindow) e->win = xevent->xcrossing.subwindow;
	else e->win = xevent->xcrossing.window;
	e->event_win = xevent->xcrossing.window;
	e->time = xevent->xcrossing.time;
	_ecore_x_event_last_time = e->time;
	_ecore_x_event_last_win = e->win;
	_ecore_x_event_last_root_x = e->root.x;
	_ecore_x_event_last_root_y = e->root.y;
	ecore_event_add(ECORE_X_EVENT_MOUSE_MOVE, e, NULL, NULL);
     }
     {
	Ecore_X_Event_Mouse_Out *e;
	
	e = calloc(1, sizeof(Ecore_X_Event_Mouse_Out));
	if (!e) return;
	e->modifiers = xevent->xcrossing.state;
	e->x = xevent->xcrossing.x;
	e->y = xevent->xcrossing.y;
	e->root.x = xevent->xcrossing.x_root;
	e->root.y = xevent->xcrossing.y_root;
	if (xevent->xcrossing.subwindow) e->win = xevent->xcrossing.subwindow;
	else e->win = xevent->xcrossing.window;
	e->event_win = xevent->xcrossing.window;
	if      (xevent->xcrossing.mode == NotifyNormal) e->mode = ECORE_X_EVENT_MODE_NORMAL;
	else if (xevent->xcrossing.mode == NotifyGrab)   e->mode = ECORE_X_EVENT_MODE_GRAB;
	else if (xevent->xcrossing.mode == NotifyUngrab) e->mode = ECORE_X_EVENT_MODE_UNGRAB;
	if      (xevent->xcrossing.detail == NotifyAncestor)         e->detail = ECORE_X_EVENT_DETAIL_ANCESTOR;
	else if (xevent->xcrossing.detail == NotifyVirtual)          e->detail = ECORE_X_EVENT_DETAIL_VIRTUAL;
	else if (xevent->xcrossing.detail == NotifyInferior)         e->detail = ECORE_X_EVENT_DETAIL_INFERIOR;
	else if (xevent->xcrossing.detail == NotifyNonlinear)        e->detail = ECORE_X_EVENT_DETAIL_NON_LINEAR;
	else if (xevent->xcrossing.detail == NotifyNonlinearVirtual) e->detail = ECORE_X_EVENT_DETAIL_NON_LINEAR_VIRTUAL;
	e->time = xevent->xcrossing.time;
	_ecore_x_event_last_time = e->time;
	_ecore_x_event_last_win = e->win;
	_ecore_x_event_last_root_x = e->root.x;
	_ecore_x_event_last_root_y = e->root.y;
	ecore_event_add(ECORE_X_EVENT_MOUSE_OUT, e, NULL, NULL);
     }
}

void
_ecore_x_event_handle_focus_in(XEvent *xevent)
{
   Ecore_X_Event_Window_Focus_In *e;
	
   e = calloc(1, sizeof(Ecore_X_Event_Window_Focus_In));
   if (!e) return;
   e->win = xevent->xfocus.window;
   if      (xevent->xfocus.mode == NotifyNormal)       e->mode = ECORE_X_EVENT_MODE_NORMAL;
   else if (xevent->xfocus.mode == NotifyWhileGrabbed) e->mode = ECORE_X_EVENT_MODE_WHILE_GRABBED;
   else if (xevent->xfocus.mode == NotifyGrab)         e->mode = ECORE_X_EVENT_MODE_GRAB;
   else if (xevent->xfocus.mode == NotifyUngrab)       e->mode = ECORE_X_EVENT_MODE_UNGRAB;
   if      (xevent->xfocus.detail == NotifyAncestor)         e->detail = ECORE_X_EVENT_DETAIL_ANCESTOR;
   else if (xevent->xfocus.detail == NotifyVirtual)          e->detail = ECORE_X_EVENT_DETAIL_VIRTUAL;
   else if (xevent->xfocus.detail == NotifyInferior)         e->detail = ECORE_X_EVENT_DETAIL_INFERIOR;
   else if (xevent->xfocus.detail == NotifyNonlinear)        e->detail = ECORE_X_EVENT_DETAIL_NON_LINEAR;
   else if (xevent->xfocus.detail == NotifyNonlinearVirtual) e->detail = ECORE_X_EVENT_DETAIL_NON_LINEAR_VIRTUAL;
   else if (xevent->xfocus.detail == NotifyPointer)          e->detail = ECORE_X_EVENT_DETAIL_POINTER;
   else if (xevent->xfocus.detail == NotifyPointerRoot)      e->detail = ECORE_X_EVENT_DETAIL_POINTER_ROOT;
   else if (xevent->xfocus.detail == NotifyDetailNone)       e->detail = ECORE_X_EVENT_DETAIL_DETAIL_NONE;
   e->time = _ecore_x_event_last_time;
   _ecore_x_event_last_time = e->time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_FOCUS_IN, e, NULL, NULL);
}

void
_ecore_x_event_handle_focus_out(XEvent *xevent)
{
   Ecore_X_Event_Window_Focus_Out *e;
	
   e = calloc(1, sizeof(Ecore_X_Event_Window_Focus_Out));
   if (!e) return;
   e->win = xevent->xfocus.window;
   if      (xevent->xfocus.mode == NotifyNormal)       e->mode = ECORE_X_EVENT_MODE_NORMAL;
   else if (xevent->xfocus.mode == NotifyWhileGrabbed) e->mode = ECORE_X_EVENT_MODE_WHILE_GRABBED;
   else if (xevent->xfocus.mode == NotifyGrab)         e->mode = ECORE_X_EVENT_MODE_GRAB;
   else if (xevent->xfocus.mode == NotifyUngrab)       e->mode = ECORE_X_EVENT_MODE_UNGRAB;
   if      (xevent->xfocus.detail == NotifyAncestor)         e->detail = ECORE_X_EVENT_DETAIL_ANCESTOR;
   else if (xevent->xfocus.detail == NotifyVirtual)          e->detail = ECORE_X_EVENT_DETAIL_VIRTUAL;
   else if (xevent->xfocus.detail == NotifyInferior)         e->detail = ECORE_X_EVENT_DETAIL_INFERIOR;
   else if (xevent->xfocus.detail == NotifyNonlinear)        e->detail = ECORE_X_EVENT_DETAIL_NON_LINEAR;
   else if (xevent->xfocus.detail == NotifyNonlinearVirtual) e->detail = ECORE_X_EVENT_DETAIL_NON_LINEAR_VIRTUAL;
   else if (xevent->xfocus.detail == NotifyPointer)          e->detail = ECORE_X_EVENT_DETAIL_POINTER;
   else if (xevent->xfocus.detail == NotifyPointerRoot)      e->detail = ECORE_X_EVENT_DETAIL_POINTER_ROOT;
   else if (xevent->xfocus.detail == NotifyDetailNone)       e->detail = ECORE_X_EVENT_DETAIL_DETAIL_NONE;
   e->time = _ecore_x_event_last_time;
   _ecore_x_event_last_time = e->time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_FOCUS_OUT, e, NULL, NULL);
}

void
_ecore_x_event_handle_keymap_notify(XEvent *xevent __UNUSED__)
{
   /* FIXME: handle this event type */   
}

void
_ecore_x_event_handle_expose(XEvent *xevent)
{
   Ecore_X_Event_Window_Damage *e;
   
   e = calloc(1, sizeof(Ecore_X_Event_Window_Damage));
   if (!e) return;
   e->win = xevent->xexpose.window;
   e->time = _ecore_x_event_last_time;
   e->x = xevent->xexpose.x;
   e->y = xevent->xexpose.y;
   e->w = xevent->xexpose.width;
   e->h = xevent->xexpose.height;
   ecore_event_add(ECORE_X_EVENT_WINDOW_DAMAGE, e, NULL, NULL);   
}

void
_ecore_x_event_handle_graphics_expose(XEvent *xevent)
{
   Ecore_X_Event_Window_Damage *e;
   
   e = calloc(1, sizeof(Ecore_X_Event_Window_Damage));
   if (!e) return;
   e->win = xevent->xgraphicsexpose.drawable;
   e->time = _ecore_x_event_last_time;
   e->x = xevent->xgraphicsexpose.x;
   e->y = xevent->xgraphicsexpose.y;
   e->w = xevent->xgraphicsexpose.width;
   e->h = xevent->xgraphicsexpose.height;
   ecore_event_add(ECORE_X_EVENT_WINDOW_DAMAGE, e, NULL, NULL);   
}

void
_ecore_x_event_handle_visibility_notify(XEvent *xevent)
{
   if (xevent->xvisibility.state != VisibilityPartiallyObscured)
   {
      Ecore_X_Event_Window_Visibility_Change *e;
      
      e = calloc(1, sizeof(Ecore_X_Event_Window_Visibility_Change));
      if (!e) return;
      e->win = xevent->xvisibility.window;
      e->time = _ecore_x_event_last_time;
      if (xevent->xvisibility.state == VisibilityFullyObscured)
	 e->fully_obscured = 1;
      else
	 e->fully_obscured = 0;	    
      ecore_event_add(ECORE_X_EVENT_WINDOW_VISIBILITY_CHANGE, e, NULL, NULL);
   }
}

void
_ecore_x_event_handle_create_notify(XEvent *xevent)
{
   Ecore_X_Event_Window_Create *e;

   e = calloc(1, sizeof(Ecore_X_Event_Window_Create));
   e->win = xevent->xcreatewindow.window;
   if (xevent->xcreatewindow.override_redirect)
      e->override = 1;
   else
      e->override = 0;
   e->time = _ecore_x_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_CREATE, e, NULL, NULL);
}

void
_ecore_x_event_handle_destroy_notify(XEvent *xevent)
{
   Ecore_X_Event_Window_Destroy *e;
   
   e = calloc(1, sizeof(Ecore_X_Event_Window_Destroy));
   if (!e) return;
   e->win =  xevent->xdestroywindow.window;
   e->time = _ecore_x_event_last_time;
   if (e->win == _ecore_x_event_last_win) _ecore_x_event_last_win = 0;
   ecore_event_add(ECORE_X_EVENT_WINDOW_DESTROY, e, NULL, NULL);   
}

void
_ecore_x_event_handle_unmap_notify(XEvent *xevent)
{
   Ecore_X_Event_Window_Hide *e;
   
   e = calloc(1, sizeof(Ecore_X_Event_Window_Hide));
   if (!e) return;
   e->win = xevent->xunmap.window;
   e->time = _ecore_x_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_HIDE, e, NULL, NULL);
}

void
_ecore_x_event_handle_map_notify(XEvent *xevent)
{
   Ecore_X_Event_Window_Show *e;
   
   e = calloc(1, sizeof(Ecore_X_Event_Window_Show));
   if (!e) return;
   e->win = xevent->xmap.window;
   e->time = _ecore_x_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_SHOW, e, NULL, NULL);
}

void
_ecore_x_event_handle_map_request(XEvent *xevent)
{
   Ecore_X_Event_Window_Show_Request *e;
   
   e = calloc(1, sizeof(Ecore_X_Event_Window_Show_Request));
   if (!e) return;
   e->win = xevent->xmaprequest.window;
   e->time = _ecore_x_event_last_time;
   e->parent = xevent->xmaprequest.parent;
   ecore_event_add(ECORE_X_EVENT_WINDOW_SHOW_REQUEST, e, NULL, NULL);
}

void
_ecore_x_event_handle_reparent_notify(XEvent *xevent)
{
   Ecore_X_Event_Window_Reparent *e;
   
   e = calloc(1, sizeof(Ecore_X_Event_Window_Reparent));
   if (!e) return;
   e->win = xevent->xreparent.window;
   e->parent = xevent->xreparent.parent;
   e->time = _ecore_x_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_REPARENT, e, NULL, NULL);
}

void
_ecore_x_event_handle_configure_notify(XEvent *xevent)
{
   Ecore_X_Event_Window_Configure *e;
   
   e = calloc(1, sizeof(Ecore_X_Event_Window_Configure));
   if (!e) return;
   e->win = xevent->xconfigure.window;
   e->abovewin = xevent->xconfigure.above;
   e->x = xevent->xconfigure.x;
   e->y = xevent->xconfigure.y;
   e->w = xevent->xconfigure.width;
   e->h = xevent->xconfigure.height;
   e->border = xevent->xconfigure.border_width;
   e->override = xevent->xconfigure.override_redirect;
   e->from_wm = xevent->xconfigure.send_event;
   e->time = _ecore_x_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_CONFIGURE, e, NULL, NULL);      
}

void
_ecore_x_event_handle_configure_request(XEvent *xevent)
{
   Ecore_X_Event_Window_Configure_Request *e;

   e = calloc(1, sizeof(Ecore_X_Event_Window_Configure_Request));
   if (!e) return;
   e->win = xevent->xconfigurerequest.window;
   e->abovewin = xevent->xconfigurerequest.above;
   e->x = xevent->xconfigurerequest.x;
   e->y = xevent->xconfigurerequest.y;
   e->w = xevent->xconfigurerequest.width;
   e->h = xevent->xconfigurerequest.height;
   e->border = xevent->xconfigurerequest.border_width;
   e->value_mask = xevent->xconfigurerequest.value_mask;
   e->time = _ecore_x_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_CONFIGURE_REQUEST, e, NULL, NULL);
}

void
_ecore_x_event_handle_gravity_notify(XEvent *xevent __UNUSED__)
{
   /* FIXME: handle this event type */
}

void
_ecore_x_event_handle_resize_request(XEvent *xevent)
{
   Ecore_X_Event_Window_Resize_Request *e;

   e = calloc(1, sizeof(Ecore_X_Event_Window_Resize_Request));
   if (!e) return;
   e->win = xevent->xresizerequest.window;
   e->w = xevent->xresizerequest.width;
   e->h = xevent->xresizerequest.height;
   e->time = _ecore_x_event_last_time;
   ecore_event_add(ECORE_X_EVENT_WINDOW_RESIZE_REQUEST, e, NULL, NULL);
}

void
_ecore_x_event_handle_circulate_notify(XEvent *xevent __UNUSED__)
{
   /* FIXME: handle this event type */
}

void
_ecore_x_event_handle_circulate_request(XEvent *xevent __UNUSED__)
{
   /* FIXME: handle this event type */
}

void
_ecore_x_event_handle_property_notify(XEvent *xevent)
{
#if 0 /* for now i disabled this. nice idea though this is - it leaves a lot
       * to be desired for efficiency that is better left to the app layer
       */
   if (xevent->xproperty.atom == ECORE_X_ATOM_WM_CLASS)
     {
	Ecore_X_Event_Window_Prop_Name_Class_Change *e;
	
	e = calloc(1, sizeof(Ecore_X_Event_Window_Prop_Name_Class_Change));
	if (!e) return;
	ecore_x_window_prop_name_class_get(xevent->xproperty.window, 
					   &(e->name), &(e->clas));
   e->time = xevent->xproperty.time;
   _ecore_x_event_last_time = e->time;
	ecore_event_add(ECORE_X_EVENT_WINDOW_PROP_NAME_CLASS_CHANGE, e, _ecore_x_event_free_window_prop_name_class_change, NULL);
     }
   else if ((xevent->xproperty.atom == ECORE_X_ATOM_WM_NAME) || (xevent->xproperty.atom == ECORE_X_ATOM_NET_WM_NAME))
     {
	Ecore_X_Event_Window_Prop_Title_Change *e;
	
	e = calloc(1, sizeof(Ecore_X_Event_Window_Prop_Title_Change));
	if (!e) return;
	e->title = ecore_x_window_prop_title_get(xevent->xproperty.window);
   e->time = xevent->xproperty.time;
   _ecore_x_event_last_time = e->time;
	ecore_event_add(ECORE_X_EVENT_WINDOW_PROP_TITLE_CHANGE, e, _ecore_x_event_free_window_prop_title_change, NULL);
     }
   else if (xevent->xproperty.atom == ECORE_X_ATOM_NET_WM_VISIBLE_NAME)
     {
	Ecore_X_Event_Window_Prop_Visible_Title_Change *e;
	
	e = calloc(1, sizeof(Ecore_X_Event_Window_Prop_Visible_Title_Change));
	if (!e) return;
	e->title = ecore_x_window_prop_visible_title_get(xevent->xproperty.window);
   e->time = xevent->xproperty.time;
   _ecore_x_event_last_time = e->time;
	ecore_event_add(ECORE_X_EVENT_WINDOW_PROP_VISIBLE_TITLE_CHANGE, e, _ecore_x_event_free_window_prop_visible_title_change, NULL);
     }
   else if ((xevent->xproperty.atom == ECORE_X_ATOM_WM_ICON_NAME) || (xevent->xproperty.atom == ECORE_X_ATOM_NET_WM_ICON_NAME))
     {
	Ecore_X_Event_Window_Prop_Icon_Name_Change *e;
	
	e = calloc(1, sizeof(Ecore_X_Event_Window_Prop_Icon_Name_Change));
	if (!e) return;
	e->name = ecore_x_window_prop_icon_name_get(xevent->xproperty.window);
   e->time = xevent->xproperty.time;
   _ecore_x_event_last_time = e->time;
	ecore_event_add(ECORE_X_EVENT_WINDOW_PROP_ICON_NAME_CHANGE, e, _ecore_x_event_free_window_prop_icon_name_change, NULL);
     }
   else if (xevent->xproperty.atom == ECORE_X_ATOM_NET_WM_VISIBLE_ICON_NAME)
     {
	Ecore_X_Event_Window_Prop_Visible_Icon_Name_Change *e;
	
	e = calloc(1, sizeof(Ecore_X_Event_Window_Prop_Visible_Icon_Name_Change));
	if (!e) return;
	e->name = ecore_x_window_prop_visible_icon_name_get(xevent->xproperty.window);
   e->time = xevent->xproperty.time;
   _ecore_x_event_last_time = e->time;
	ecore_event_add(ECORE_X_EVENT_WINDOW_PROP_VISIBLE_ICON_NAME_CHANGE, e, _ecore_x_event_free_window_prop_visible_icon_name_change, NULL);
     }
   else if (xevent->xproperty.atom == ECORE_X_ATOM_WM_CLIENT_MACHINE)
     {
	Ecore_X_Event_Window_Prop_Client_Machine_Change *e;
	
	e = calloc(1, sizeof(Ecore_X_Event_Window_Prop_Client_Machine_Change));
	if (!e) return;
	e->name = ecore_x_window_prop_client_machine_get(xevent->xproperty.window);
   e->time = xevent->xproperty.time;
   _ecore_x_event_last_time = e->time;
	ecore_event_add(ECORE_X_EVENT_WINDOW_PROP_CLIENT_MACHINE_CHANGE, e, _ecore_x_event_free_window_prop_client_machine_change, NULL);
     }
   else if (xevent->xproperty.atom == ECORE_X_ATOM_NET_WM_PID)
     {
	Ecore_X_Event_Window_Prop_Pid_Change *e;
	
	e = calloc(1, sizeof(Ecore_X_Event_Window_Prop_Pid_Change));
	if (!e) return;
	e->pid = ecore_x_window_prop_pid_get(xevent->xproperty.window);
   e->time = xevent->xproperty.time;
   _ecore_x_event_last_time = e->time;
	ecore_event_add(ECORE_X_EVENT_WINDOW_PROP_PID_CHANGE, e, NULL, NULL);
     }
   else if (xevent->xproperty.atom == ECORE_X_ATOM_NET_WM_DESKTOP)
     {
	Ecore_X_Event_Window_Prop_Desktop_Change *e;
	
	e = calloc(1, sizeof(Ecore_X_Event_Window_Prop_Desktop_Change));
	if (!e) return;
	e->desktop = ecore_x_window_prop_desktop_get(xevent->xproperty.window);
	ecore_event_add(ECORE_X_EVENT_WINDOW_PROP_PID_CHANGE, e, NULL, NULL);
     }
   else 
#endif     
   {
      Ecore_X_Event_Window_Property *e;

      e = calloc(1,sizeof(Ecore_X_Event_Window_Property));
      if (!e) return;
      e->win = xevent->xproperty.window;
      e->atom = xevent->xproperty.atom;
      e->time = xevent->xproperty.time;
      _ecore_x_event_last_time = e->time;
      ecore_event_add(ECORE_X_EVENT_WINDOW_PROPERTY, e, NULL, NULL);
   }
}

void
_ecore_x_event_handle_selection_clear(XEvent *xevent)
{
   Ecore_X_Selection_Data *d;
   Ecore_X_Event_Selection_Clear *e;
   Atom sel;

   if (!(d = _ecore_x_selection_get(xevent->xselectionclear.selection)))
     return;
   if (xevent->xselectionclear.time > d->time)
     {
	_ecore_x_selection_set(None, NULL, 0, 
			       xevent->xselectionclear.selection);
     }

   /* Generate event for app cleanup */
   e = malloc(sizeof(Ecore_X_Event_Selection_Clear));
   e->win = xevent->xselectionclear.window;
   e->time = xevent->xselectionclear.time;
   sel = xevent->xselectionclear.selection;
   if (sel == ECORE_X_ATOM_SELECTION_PRIMARY)
     e->selection = ECORE_X_SELECTION_PRIMARY;
   else if (sel == ECORE_X_ATOM_SELECTION_SECONDARY)
     e->selection = ECORE_X_SELECTION_SECONDARY;
   else
     e->selection = ECORE_X_SELECTION_CLIPBOARD;
   ecore_event_add(ECORE_X_EVENT_SELECTION_CLEAR, e, NULL, NULL);

}

void
_ecore_x_event_handle_selection_request(XEvent *xevent)
{
   Ecore_X_Selection_Data           *sd;
   XSelectionEvent                  xnotify;
   XEvent                           xev;
   void                             *data;

   xnotify.type = SelectionNotify;
   xnotify.display = xevent->xselectionrequest.display;
   xnotify.requestor = xevent->xselectionrequest.requestor;
   xnotify.selection = xevent->xselectionrequest.selection;
   xnotify.target = xevent->xselectionrequest.target;
   xnotify.time = CurrentTime;

   if ((sd = _ecore_x_selection_get(xnotify.selection)) 
       && (sd->win == xevent->xselectionrequest.owner))
     {
	if (!_ecore_x_selection_convert(xnotify.selection, xnotify.target,
					&data) == -1)
	  {
	     /* Refuse selection, conversion to requested target failed */
	     xnotify.property = None;
	  }
	else
	  {
	     /* FIXME: This does not properly handle large data transfers */
	     ecore_x_window_prop_property_set(xevent->xselectionrequest.requestor,
					      xevent->xselectionrequest.property,
					      xevent->xselectionrequest.target,
					      8, data, sd->length);
	     xnotify.property = xevent->xselectionrequest.property;
	     free(data);
	  }
     }
   else
     {
	xnotify.property = None;
	return;
     }

   xev.xselection = xnotify;
   XSendEvent(xevent->xselectionrequest.display, 
	      xevent->xselectionrequest.requestor, False, 0, &xev);
}

void
_ecore_x_event_handle_selection_notify(XEvent *xevent)
{
   Ecore_X_Event_Selection_Notify   *e;
   unsigned char                    *data = NULL;
   Atom                             selection;
   int                              num_ret;
   Ecore_X_Selection_Data           sel_data;

   e = calloc(1, sizeof(Ecore_X_Event_Selection_Notify));
   e->win = xevent->xselection.requestor;
   e->time = xevent->xselection.time;
   e->target = _ecore_x_selection_target_get(xevent->xselection.target);
   selection = xevent->xselection.selection;

   if (!ecore_x_window_prop_property_get(e->win, xevent->xselection.property,
					 AnyPropertyType, 8, &data, &num_ret))
     {
	free(e);
	return;
     }

   sel_data.win = e->win;
   sel_data.selection = selection;
   sel_data.data = data;
   sel_data.length = num_ret;
   _ecore_x_selection_request_data_set(sel_data);

   if (selection == ECORE_X_ATOM_SELECTION_PRIMARY)
     e->selection = ECORE_X_SELECTION_PRIMARY;
   else if (selection == ECORE_X_ATOM_SELECTION_SECONDARY)
     e->selection = ECORE_X_SELECTION_SECONDARY;
   else if (selection == ECORE_X_ATOM_SELECTION_XDND)
     {
	e->selection = ECORE_X_SELECTION_XDND;
	if (!strcmp(e->target, "text/uri-list"))
	  {
	     int i, is;
	     char *tmp;

	     e->content = ECORE_X_SELECTION_FILES;

	     tmp = malloc(num_ret * sizeof(char));
	     i = 0;
	     is = 0;
	     e->files = NULL;
	     while ((is < num_ret) && (data[is]))
	       {
		  if ((i == 0) && (data[is] == '#'))
		    {
		       for (; ((data[is]) && (data[is] != '\n')); is++);
		    }
		  else
		    {
		       if ((data[is] != '\r')
			   && (data[is] != '\n'))
			 {
			    tmp[i++] = data[is++];
			 }
		       else
			 {
			    while ((data[is] == '\r')
				   || (data[is] == '\n'))
			      is++;
			    tmp[i] = 0;
			    e->num_files++;
			    e->files = realloc(e->files, e->num_files * sizeof(char *));
			    e->files[e->num_files - 1] = strdup(tmp);
			    tmp[0] = 0;
			    i = 0;
			 }
		    }
	       }
	     if (i > 0)
	       {
		  tmp[i] = 0;
		  e->num_files++;
		  e->files = realloc(e->files, e->num_files * sizeof(char *));
		  e->files[e->num_files - 1] = strdup(tmp);
	       }
	     free(tmp);
	  }
	else if (!strcmp(e->target, "_NETSCAPE_URL"))
	  {
	     e->content = ECORE_X_SELECTION_FILES;
	     e->num_files = 1;
	     e->files = malloc(sizeof(char *));
	     e->files[0] = data;
	  }
	else if (!strcmp(e->target, "text/plain"))
	  {
	     e->content = ECORE_X_SELECTION_TEXT;
	     e->text = data;
	  }
     }
   else if (selection == ECORE_X_ATOM_SELECTION_CLIPBOARD)
     e->selection = ECORE_X_SELECTION_CLIPBOARD;
   else
     {
	free(e);
	return;
     }

   ecore_event_add(ECORE_X_EVENT_SELECTION_NOTIFY, e, _ecore_x_event_free_selection_notify, NULL);
}

void
_ecore_x_event_handle_colormap_notify(XEvent *xevent)
{
   Ecore_X_Event_Window_Colormap *e;

   e = calloc(1,sizeof(Ecore_X_Event_Window_Colormap));
   e->win = xevent->xcolormap.window;
   e->cmap = xevent->xcolormap.colormap;
   e->time = _ecore_x_event_last_time;
   if (xevent->xcolormap.state == ColormapInstalled)
      e->installed = 1;
   else
      e->installed = 0;
   ecore_event_add(ECORE_X_EVENT_WINDOW_COLORMAP, e, NULL, NULL);
}

void
_ecore_x_event_handle_client_message(XEvent *xevent)
{
   /* Special client message event handling here. need to put LOTS of if */
   /* checks here and generate synthetic events per special message known */
   /* otherwise generate generic client message event. this would handle*/
   /* netwm, ICCCM, gnomewm, old kde and mwm hint client message protocols */
   if ((xevent->xclient.message_type == ECORE_X_ATOM_WM_PROTOCOLS) &&
	 (xevent->xclient.format == 32) &&
	 (xevent->xclient.data.l[0] == (long)ECORE_X_ATOM_WM_DELETE_WINDOW))
     {
	Ecore_X_Event_Window_Delete_Request *e;

	e = calloc(1, sizeof(Ecore_X_Event_Window_Delete_Request));
	if (!e) return;
	e->win = xevent->xclient.window;
	e->time = _ecore_x_event_last_time;
	ecore_event_add(ECORE_X_EVENT_WINDOW_DELETE_REQUEST, e, NULL, NULL);
     }

   /* Xdnd Client Message Handling Begin */
   /* Message Type: XdndEnter target */
   else if (xevent->xclient.message_type == ECORE_X_ATOM_XDND_ENTER)
     {
	Ecore_X_Event_Xdnd_Enter *e;
	Ecore_X_DND_Target *target;
	unsigned long three;

	e = calloc(1, sizeof(Ecore_X_Event_Xdnd_Enter));
	if (!e) return;

	target = _ecore_x_dnd_target_get();
	target->state = ECORE_X_DND_TARGET_ENTERED;

	target = _ecore_x_dnd_target_get();
	target->source = xevent->xclient.data.l[0];
	target->win = xevent->xclient.window;
	target->version = (int) (xevent->xclient.data.l[1] >> 24);
	if (target->version > ECORE_X_DND_VERSION)
	  {
	     printf("DND: Requested version %d, we only support up to %d\n", target->version,
									     ECORE_X_DND_VERSION);
	     return;
	  }

	if ((three = xevent->xclient.data.l[1] & 0x1UL))
	  {
	     /* source supports more than 3 types, fetch property */
	     unsigned char *data;
	     Ecore_X_Atom *types;
	     int i, num_ret;
	     if (!(ecore_x_window_prop_property_get(target->source, 
						    ECORE_X_ATOM_XDND_TYPE_LIST,
						    XA_ATOM,
						    32,
						    &data,
						    &num_ret)))
	       {
		  printf("DND: Could not fetch data type list from source window, aborting.\n");
		  return;
	       }
	     types = (Ecore_X_Atom *)data;
	     e->types = calloc(num_ret, sizeof(char *));
	     for (i = 0; i < num_ret; i++)
	       e->types[i] = XGetAtomName(_ecore_x_disp, types[i]);
	     e->num_types = num_ret;
	  }
	else
	  {
	     int i = 0;

	     e->types = calloc(3, sizeof(char *));
	     while ((i < 3) && (xevent->xclient.data.l[i + 2]))
	       {
		  e->types[i] = XGetAtomName(_ecore_x_disp, xevent->xclient.data.l[i + 2]);
		  i++;
	       }
	     e->num_types = i;
	  }

	e->win = target->win;
	e->source = target->source;
	ecore_event_add(ECORE_X_EVENT_XDND_ENTER, e, _ecore_x_event_free_xdnd_enter, NULL);
     }

   /* Message Type: XdndPosition target */
   else if (xevent->xclient.message_type == ECORE_X_ATOM_XDND_POSITION)
     {
	Ecore_X_Event_Xdnd_Position *e;
	Ecore_X_DND_Target *target;

	target = _ecore_x_dnd_target_get();
	if ((target->source != xevent->xclient.data.l[0])
	    || (target->win != xevent->xclient.window))
	  return;

	target->pos.x = xevent->xclient.data.l[2] >> 16;
	target->pos.y = xevent->xclient.data.l[2] & 0xFFFFUL;
	target->action = xevent->xclient.data.l[4]; /* Version 2 */

	target->time = (target->version >= 1) ? 
	   (Time)xevent->xclient.data.l[3] : CurrentTime;

	e = calloc(1, sizeof(Ecore_X_Event_Xdnd_Position));
	if (!e) return;
	e->win = target->win;
	e->source = target->source;
	e->position.x = target->pos.x;
	e->position.y = target->pos.y;
	e->action = target->action;
	ecore_event_add(ECORE_X_EVENT_XDND_POSITION, e, NULL, NULL);
     }

   /* Message Type: XdndStatus source */
   else if (xevent->xclient.message_type == ECORE_X_ATOM_XDND_STATUS)
     {
	Ecore_X_Event_Xdnd_Status *e;
	Ecore_X_DND_Source *source;

	source = _ecore_x_dnd_source_get();
	/* Make sure source/target match */
	if ((source->win != xevent->xclient.window )
	    || (source->dest != (Window)xevent->xclient.data.l[0]))
	  return;

	source->await_status = 0;

	source->will_accept = xevent->xclient.data.l[1] & 0x1UL;
	source->suppress = (xevent->xclient.data.l[1] & 0x2UL) ? 0 : 1;

	source->rectangle.x = xevent->xclient.data.l[2] >> 16;
	source->rectangle.y = xevent->xclient.data.l[2] & 0xFFFFUL;
	source->rectangle.width = xevent->xclient.data.l[3] >> 16;
	source->rectangle.height = xevent->xclient.data.l[3] & 0xFFFFUL;

	source->accepted_action = xevent->xclient.data.l[4];

	e = calloc(1, sizeof(Ecore_X_Event_Xdnd_Status));
	if (!e) return;
	e->win = source->win;
	e->target = source->dest;
	e->will_accept = source->will_accept;
	e->rectangle.x = source->rectangle.x;
	e->rectangle.y = source->rectangle.y;
	e->rectangle.width = source->rectangle.width;
	e->rectangle.height = source->rectangle.height;
	e->action = source->accepted_action;

	ecore_event_add(ECORE_X_EVENT_XDND_STATUS, e, NULL, NULL);
     }

   /* Message Type: XdndLeave target */
   /* Pretend the whole thing never happened, sort of */
   else if (xevent->xclient.message_type == ECORE_X_ATOM_XDND_LEAVE)
     {
	Ecore_X_Event_Xdnd_Leave *e;
	Ecore_X_DND_Target *target;

	target = _ecore_x_dnd_target_get();
	if ((target->source != xevent->xclient.data.l[0])
	    || (target->win != xevent->xclient.window))
	  return;

	target->state = ECORE_X_DND_TARGET_IDLE;

	e = calloc(1, sizeof(Ecore_X_Event_Xdnd_Leave));
	if (!e) return;
	e->win = xevent->xclient.window;
	e->source = (Window)xevent->xclient.data.l[0];
	ecore_event_add(ECORE_X_EVENT_XDND_LEAVE, e, NULL, NULL);
     }

   /* Message Type: XdndDrop target */
   else if (xevent->xclient.message_type == ECORE_X_ATOM_XDND_DROP)
     {
	Ecore_X_Event_Xdnd_Drop *e;
	Ecore_X_DND_Target *target;

	target = _ecore_x_dnd_target_get();
	/* Match source/target */
	if ((target->source != (Window)xevent->xclient.data.l[0])
	    || (target->win != xevent->xclient.window))
	  return;

	target->time = (target->version >= 1) ? 
	   (Time)xevent->xclient.data.l[2] : _ecore_x_event_last_time;

	e = calloc(1, sizeof(Ecore_X_Event_Xdnd_Drop));
	if (!e) return;
	e->win = target->win;
	e->source = target->source;
	e->action = target->action;
	e->position.x = target->pos.x;
	e->position.y = target->pos.y;
	ecore_event_add(ECORE_X_EVENT_XDND_DROP, e, NULL, NULL);
     }

   /* Message Type: XdndFinished source */
   else if (xevent->xclient.message_type == ECORE_X_ATOM_XDND_FINISHED)
     {
	Ecore_X_Event_Xdnd_Finished *e;
	Ecore_X_DND_Source *source;
	int completed = 1;

	source = _ecore_x_dnd_source_get();
	/* Match source/target */
	if ((source->win != xevent->xclient.window)
	    || (source->dest != (Window)xevent->xclient.data.l[0]))
	  return;

	if ((source->version >= 5) && (xevent->xclient.data.l[1] & 0x1UL))
	  {
	     /* Target successfully performed drop action */
	     ecore_x_selection_xdnd_clear();
	     source->state = ECORE_X_DND_SOURCE_IDLE;
	  } else {
	       completed = 0;
	       source->state = ECORE_X_DND_SOURCE_CONVERTING;

	       /* FIXME: Probably need to add a timer to switch back to idle 
		* and discard the selection data */
	  } 

	e = calloc(1, sizeof(Ecore_X_Event_Xdnd_Finished));
	if (!e) return;
	e->win = source->win;
	e->target = source->dest;
	e->completed = completed;
	if (source->version >= 5)
	  {
	     source->accepted_action = xevent->xclient.data.l[2];
	     e->action = source->accepted_action;
	  }
	else
	  {
	     source->accepted_action = 0;
	     e->action = source->action;
	  }

	ecore_event_add(ECORE_X_EVENT_XDND_FINISHED, e, NULL, NULL);
     }
   else
     {
	Ecore_X_Event_Client_Message *e;
	int i;

	e = (Ecore_X_Event_Client_Message *) calloc(1, sizeof(Ecore_X_Event_Client_Message));
	e->win = xevent->xclient.window;
	e->message_type = xevent->xclient.message_type;
	e->format = xevent->xclient.format;
	for (i = 0; i < 5; i++) 
	  e->data.l[i] = xevent->xclient.data.l[i];

	ecore_event_add(ECORE_X_EVENT_CLIENT_MESSAGE, e, NULL, NULL);
     }
}

void
_ecore_x_event_handle_mapping_notify(XEvent *xevent __UNUSED__)
{
   /* FIXME: handle this event type */
}

void
_ecore_x_event_handle_shape_change(XEvent *xevent __UNUSED__)
{
   /* FIXME: handle this event type */
}
