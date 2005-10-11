/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

/* local subsystem functions */

static void _e_bindings_mouse_free(E_Binding_Mouse *bind);
static void _e_bindings_key_free(E_Binding_Key *bind);
static void _e_bindings_signal_free(E_Binding_Signal *bind);
static void _e_bindings_wheel_free(E_Binding_Wheel *bind);
static int _e_bindings_context_match(E_Binding_Context bctxt, E_Binding_Context ctxt);

/* local subsystem globals */

static Evas_List *mouse_bindings = NULL;
static Evas_List *key_bindings = NULL;
static Evas_List *signal_bindings = NULL;
static Evas_List *wheel_bindings = NULL;

/* externally accessible functions */

int
e_bindings_init(void)
{
   Evas_List *l;

   for (l = e_config->mouse_bindings; l; l = l->next)
     {
	E_Config_Binding_Mouse *eb;

	eb = l->data;
	e_bindings_mouse_add(eb->context, eb->button, eb->modifiers,
			     eb->any_mod, eb->action, eb->params);
     }

   for (l = e_config->key_bindings; l; l = l->next)
     {
	E_Config_Binding_Key *eb;

	eb = l->data;
	e_bindings_key_add(eb->context, eb->key, eb->modifiers,
			   eb->any_mod, eb->action, eb->params);
     }

   for (l = e_config->signal_bindings; l; l = l->next)
     {
	E_Config_Binding_Signal *eb;

	eb = l->data;
	e_bindings_signal_add(eb->context, eb->signal, eb->source, eb->modifiers,
			      eb->any_mod, eb->action, eb->params);
	/* FIXME: Can this be solved in a generic way? */
	/* FIXME: Only change cursor if action is allowed! */
	if ((!strcmp(eb->action, "window_resize")) &&
	    (!strncmp(eb->signal, "mouse,down,", 11)) &&
	    (!strncmp(eb->source, "resize_", 7)))
	  {
	     char params[32];
	     snprintf(params, sizeof(params), "resize_%s", eb->params);
	     e_bindings_signal_add(eb->context, "mouse,in", eb->source, eb->modifiers,
				   eb->any_mod, "pointer_resize_push", params);
	     e_bindings_signal_add(eb->context, "mouse,out", eb->source, eb->modifiers,
				   eb->any_mod, "pointer_resize_pop", params);
	  }
     }

   for (l = e_config->wheel_bindings; l; l = l->next)
     {
	E_Config_Binding_Wheel *eb;

	eb = l->data;
	e_bindings_wheel_add(eb->context, eb->direction, eb->z, eb->modifiers,
			     eb->any_mod, eb->action, eb->params);
     }

   return 1;
}

int
e_bindings_shutdown(void)
{
   while (mouse_bindings)
     {
	E_Binding_Mouse *bind;
	
	bind = mouse_bindings->data;
	mouse_bindings  = evas_list_remove_list(mouse_bindings, mouse_bindings);
	_e_bindings_mouse_free(bind);
     }
   while (key_bindings)
     {
	E_Binding_Key *bind;
	
	bind = key_bindings->data;
	key_bindings  = evas_list_remove_list(key_bindings, key_bindings);
	_e_bindings_key_free(bind);
     }
   while (signal_bindings)
     {
	E_Binding_Signal *bind;
	
	bind = signal_bindings->data;
	signal_bindings  = evas_list_remove_list(signal_bindings, signal_bindings);
	_e_bindings_signal_free(bind);
     }
   while (wheel_bindings)
     {
	E_Binding_Wheel *bind;
	
	bind = wheel_bindings->data;
	wheel_bindings  = evas_list_remove_list(wheel_bindings, wheel_bindings);
	_e_bindings_wheel_free(bind);
     }
   return 1;
}

void
e_bindings_mouse_add(E_Binding_Context ctxt, int button, E_Binding_Modifier mod, int any_mod, char *action, char *params)
{
   E_Binding_Mouse *bind;
   
   bind = calloc(1, sizeof(E_Binding_Mouse));
   bind->ctxt = ctxt;
   bind->button = button;
   bind->mod = mod;
   bind->any_mod = any_mod;
   bind->action = strdup(action);
   bind->params = strdup(params);
   mouse_bindings = evas_list_append(mouse_bindings, bind);
}

void
e_bindings_mouse_del(E_Binding_Context ctxt, int button, E_Binding_Modifier mod, int any_mod, char *action, char *params)
{
   Evas_List *l;
   
   for (l = mouse_bindings; l; l = l->next)
     {
	E_Binding_Mouse *bind;
	
	bind = l->data;
	if ((bind->ctxt == ctxt) &&
	    (bind->button == button) &&
	    (bind->mod == mod) &&
	    (bind->any_mod == any_mod) &&
	    (!strcmp(bind->action, action)) &&
	    (!strcmp(bind->params, params)))
	  {
	     _e_bindings_mouse_free(bind);
	     mouse_bindings = evas_list_remove_list(mouse_bindings, l);
	     break;
	  }
     }
}

void
e_bindings_mouse_grab(E_Binding_Context ctxt, Ecore_X_Window win)
{
   Evas_List *l;

   for (l = mouse_bindings; l; l = l->next)
     {
	E_Binding_Mouse *bind;
	
	bind = l->data;
	if (_e_bindings_context_match(bind->ctxt, ctxt))
	  {
	     int mod;
	     
	     mod = 0;
	     if (bind->mod & E_BINDING_MODIFIER_SHIFT) mod |= ECORE_X_MODIFIER_SHIFT;
	     if (bind->mod & E_BINDING_MODIFIER_CTRL) mod |= ECORE_X_MODIFIER_CTRL;
	     if (bind->mod & E_BINDING_MODIFIER_ALT) mod |= ECORE_X_MODIFIER_ALT;
	     if (bind->mod & E_BINDING_MODIFIER_WIN) mod |= ECORE_X_MODIFIER_WIN;
	     ecore_x_window_button_grab(win, bind->button, 
					ECORE_X_EVENT_MASK_MOUSE_DOWN | 
					ECORE_X_EVENT_MASK_MOUSE_UP | 
					ECORE_X_EVENT_MASK_MOUSE_MOVE, 
					mod, bind->any_mod);
	  }
     }
}

void
e_bindings_mouse_ungrab(E_Binding_Context ctxt, Ecore_X_Window win)
{
   Evas_List *l;
   
   for (l = mouse_bindings; l; l = l->next)
     {
	E_Binding_Mouse *bind;
	
	bind = l->data;
	if (_e_bindings_context_match(bind->ctxt, ctxt))
	  {
	     int mod;
	     
	     mod = 0;
	     if (bind->mod & E_BINDING_MODIFIER_SHIFT) mod |= ECORE_X_MODIFIER_SHIFT;
	     if (bind->mod & E_BINDING_MODIFIER_CTRL) mod |= ECORE_X_MODIFIER_CTRL;
	     if (bind->mod & E_BINDING_MODIFIER_ALT) mod |= ECORE_X_MODIFIER_ALT;
	     if (bind->mod & E_BINDING_MODIFIER_WIN) mod |= ECORE_X_MODIFIER_WIN;
	     ecore_x_window_button_ungrab(win, bind->button,
					  mod, bind->any_mod);
	  }
     }
}

E_Action *
e_bindings_mouse_down_find(E_Binding_Context ctxt, E_Object *obj, Ecore_X_Event_Mouse_Button_Down *ev, E_Binding_Mouse **bind_ret)
{
   E_Binding_Modifier mod = 0;
   Evas_List *l;
   
   if (ev->modifiers & ECORE_X_MODIFIER_SHIFT) mod |= E_BINDING_MODIFIER_SHIFT;
   if (ev->modifiers & ECORE_X_MODIFIER_CTRL) mod |= E_BINDING_MODIFIER_CTRL;
   if (ev->modifiers & ECORE_X_MODIFIER_ALT) mod |= E_BINDING_MODIFIER_ALT;
   if (ev->modifiers & ECORE_X_MODIFIER_WIN) mod |= E_BINDING_MODIFIER_WIN;
   for (l = mouse_bindings; l; l = l->next)
     {
	E_Binding_Mouse *bind;
	
	bind = l->data;
	if ((bind->button == ev->button) &&
	    ((bind->any_mod) || (bind->mod == mod)))
	  {
	     if (_e_bindings_context_match(bind->ctxt, ctxt))
	       {
		  E_Action *act;
		  
		  act = e_action_find(bind->action);
		  if (bind_ret) *bind_ret = bind;
		  return act;
	       }
	  }
     }
   return NULL;
}

E_Action *
e_bindings_mouse_down_event_handle(E_Binding_Context ctxt, E_Object *obj, Ecore_X_Event_Mouse_Button_Down *ev)
{
   E_Action *act;
   E_Binding_Mouse *bind;
   
   act = e_bindings_mouse_down_find(ctxt, obj, ev, &bind);
   if (act)
     {
	if (act->func.go_mouse)
	  act->func.go_mouse(obj, bind->params, ev);
	else if (act->func.go)
	  act->func.go(obj, bind->params);
	return act;
     }
   return act;
}

E_Action *
e_bindings_mouse_up_find(E_Binding_Context ctxt, E_Object *obj, Ecore_X_Event_Mouse_Button_Up *ev, E_Binding_Mouse **bind_ret)
{
   E_Binding_Modifier mod = 0;
   Evas_List *l;
   
   if (ev->modifiers & ECORE_X_MODIFIER_SHIFT) mod |= E_BINDING_MODIFIER_SHIFT;
   if (ev->modifiers & ECORE_X_MODIFIER_CTRL) mod |= E_BINDING_MODIFIER_CTRL;
   if (ev->modifiers & ECORE_X_MODIFIER_ALT) mod |= E_BINDING_MODIFIER_ALT;
   if (ev->modifiers & ECORE_X_MODIFIER_WIN) mod |= E_BINDING_MODIFIER_WIN;
   for (l = mouse_bindings; l; l = l->next)
     {
	E_Binding_Mouse *bind;
	
	bind = l->data;
	if ((bind->button == ev->button) &&
	    ((bind->any_mod) || (bind->mod == mod)))
	  {
	     if (_e_bindings_context_match(bind->ctxt, ctxt))
	       {
		  E_Action *act;
		  
		  act = e_action_find(bind->action);
		  if (bind_ret) *bind_ret = bind;
		  return act;
	       }
	  }
     }
   return NULL;
}

E_Action *
e_bindings_mouse_up_event_handle(E_Binding_Context ctxt, E_Object *obj, Ecore_X_Event_Mouse_Button_Up *ev)
{
   E_Action *act;
   E_Binding_Mouse *bind;
   
   act = e_bindings_mouse_up_find(ctxt, obj, ev, &bind);
   if (act)
     {
	if (act->func.end_mouse)
	  act->func.end_mouse(obj, bind->params, ev);
	else if (act->func.end)
	  act->func.end(obj, bind->params);
	return act;
     }
   return act;
}

void
e_bindings_key_add(E_Binding_Context ctxt, char *key, E_Binding_Modifier mod, int any_mod, char *action, char *params)
{
   E_Binding_Key *bind;
   
   bind = calloc(1, sizeof(E_Binding_Key));
   bind->ctxt = ctxt;
   bind->key = strdup(key);
   bind->mod = mod;
   bind->any_mod = any_mod;
   bind->action = strdup(action);
   bind->params = strdup(params);
   key_bindings = evas_list_append(key_bindings, bind);
}

void
e_bindings_key_del(E_Binding_Context ctxt, char *key, E_Binding_Modifier mod, int any_mod, char *action, char *params)
{
   Evas_List *l;
   
   for (l = key_bindings; l; l = l->next)
     {
	E_Binding_Key *bind;
	
	bind = l->data;
	if ((bind->ctxt == ctxt) &&
	    (!strcmp(bind->key, key)) &&
	    (bind->mod == mod) &&
	    (bind->any_mod == any_mod) &&
	    (!strcmp(bind->action, action)) &&
	    (!strcmp(bind->params, params)))
	  {
	     _e_bindings_key_free(bind);
	     key_bindings = evas_list_remove_list(key_bindings, l);
	     break;
	  }
     }
}

void
e_bindings_key_grab(E_Binding_Context ctxt, Ecore_X_Window win)
{
   Evas_List *l;

   for (l = key_bindings; l; l = l->next)
     {
	E_Binding_Key *bind;
	
	bind = l->data;
	if (_e_bindings_context_match(bind->ctxt, ctxt))
	  {
	     int mod;
	     
	     mod = 0;
	     if (bind->mod & E_BINDING_MODIFIER_SHIFT) mod |= ECORE_X_MODIFIER_SHIFT;
	     if (bind->mod & E_BINDING_MODIFIER_CTRL) mod |= ECORE_X_MODIFIER_CTRL;
	     if (bind->mod & E_BINDING_MODIFIER_ALT) mod |= ECORE_X_MODIFIER_ALT;
	     if (bind->mod & E_BINDING_MODIFIER_WIN) mod |= ECORE_X_MODIFIER_WIN;
	     ecore_x_window_key_grab(win, bind->key,
				     mod, bind->any_mod);
	  }
     }
}

void
e_bindings_key_ungrab(E_Binding_Context ctxt, Ecore_X_Window win)
{
   Evas_List *l;
   
   for (l = key_bindings; l; l = l->next)
     {
	E_Binding_Key *bind;
	
	bind = l->data;
	if (_e_bindings_context_match(bind->ctxt, ctxt))
	  {
	     int mod;
	     
	     mod = 0;
	     if (bind->mod & E_BINDING_MODIFIER_SHIFT) mod |= ECORE_X_MODIFIER_SHIFT;
	     if (bind->mod & E_BINDING_MODIFIER_CTRL) mod |= ECORE_X_MODIFIER_CTRL;
	     if (bind->mod & E_BINDING_MODIFIER_ALT) mod |= ECORE_X_MODIFIER_ALT;
	     if (bind->mod & E_BINDING_MODIFIER_WIN) mod |= ECORE_X_MODIFIER_WIN;
	     ecore_x_window_key_ungrab(win, bind->key,
				       mod, bind->any_mod);
	  }
     }
}

E_Action *
e_bindings_key_down_event_handle(E_Binding_Context ctxt, E_Object *obj, Ecore_X_Event_Key_Down *ev)
{
   E_Binding_Modifier mod = 0;
   Evas_List *l;
   
   if (ev->modifiers & ECORE_X_MODIFIER_SHIFT) mod |= E_BINDING_MODIFIER_SHIFT;
   if (ev->modifiers & ECORE_X_MODIFIER_CTRL) mod |= E_BINDING_MODIFIER_CTRL;
   if (ev->modifiers & ECORE_X_MODIFIER_ALT) mod |= E_BINDING_MODIFIER_ALT;
   if (ev->modifiers & ECORE_X_MODIFIER_WIN) mod |= E_BINDING_MODIFIER_WIN;
   for (l = key_bindings; l; l = l->next)
     {
	E_Binding_Key *bind;
	
	bind = l->data;
	if ((!strcmp(bind->key, ev->keyname)) &&
	    ((bind->any_mod) || (bind->mod == mod)))
	  {
	     if (_e_bindings_context_match(bind->ctxt, ctxt))
	       {
		  E_Action *act;
	
		  act = e_action_find(bind->action);
		  if (act)
		    {
		       if (act->func.go_key)
			 act->func.go_key(obj, bind->params, ev);
		       else if (act->func.go)
			 act->func.go(obj, bind->params);
		       return act;
		    }
		  return NULL;
	       }
	  }
     }
   return NULL;
}

E_Action *
e_bindings_key_up_event_handle(E_Binding_Context ctxt, E_Object *obj, Ecore_X_Event_Key_Up *ev)
{
   E_Binding_Modifier mod = 0;
   Evas_List *l;
   
   if (ev->modifiers & ECORE_X_MODIFIER_SHIFT) mod |= E_BINDING_MODIFIER_SHIFT;
   if (ev->modifiers & ECORE_X_MODIFIER_CTRL) mod |= E_BINDING_MODIFIER_CTRL;
   if (ev->modifiers & ECORE_X_MODIFIER_ALT) mod |= E_BINDING_MODIFIER_ALT;
   if (ev->modifiers & ECORE_X_MODIFIER_WIN) mod |= E_BINDING_MODIFIER_WIN;
   for (l = key_bindings; l; l = l->next)
     {
	E_Binding_Key *bind;
	
	bind = l->data;
	if ((!strcmp(bind->key, ev->keyname)) &&
	    ((bind->any_mod) || (bind->mod == mod)))
	  {
	     if (_e_bindings_context_match(bind->ctxt, ctxt))
	       {
		  E_Action *act;
		  
		  act = e_action_find(bind->action);
		  if (act)
		    {
		       if (act->func.end_key)
			 act->func.end_key(obj, bind->params, ev);
		       else if (act->func.end)
			 act->func.end(obj, bind->params);
		       return act;
		    }
		  return NULL;
	       }
	  }
     }
   return NULL;
}

/* FIXME: finish off signal bindings */
void
e_bindings_signal_add(E_Binding_Context ctxt, char *sig, char *src, E_Binding_Modifier mod, int any_mod, char *action, char *params)
{
   E_Binding_Signal *bind;
   
   bind = calloc(1, sizeof(E_Binding_Signal));
   bind->ctxt = ctxt;
   bind->sig = strdup(sig);
   bind->src = strdup(src);
   bind->mod = mod;
   bind->any_mod = any_mod;
   bind->action = strdup(action);
   bind->params = strdup(params);
   signal_bindings = evas_list_append(signal_bindings, bind);
}

void
e_bindings_signal_del(E_Binding_Context ctxt, char *sig, char *src, E_Binding_Modifier mod, int any_mod, char *action, char *params)
{
   Evas_List *l;
   
   for (l = signal_bindings; l; l = l->next)
     {
	E_Binding_Signal *bind;
	
	bind = l->data;
	if ((bind->ctxt == ctxt) &&
	    (!strcmp(bind->sig, sig)) &&
	    (!strcmp(bind->src, src)) &&
	    (bind->mod == mod) &&
	    (bind->any_mod == any_mod) &&
	    (!strcmp(bind->action, action)) &&
	    (!strcmp(bind->params, params)))
	  {
	     _e_bindings_signal_free(bind);
	     signal_bindings = evas_list_remove_list(signal_bindings, l);
	     break;
	  }
     }
}

E_Action  *
e_bindings_signal_find(E_Binding_Context ctxt, E_Object *obj, char *sig, char *src, E_Binding_Signal **bind_ret)
{
   E_Binding_Modifier mod = 0;
   Evas_List *l;
   
   if (strstr(sig, "MOD:Shift")) mod |= E_BINDING_MODIFIER_SHIFT;
   if (strstr(sig, "MOD:Control")) mod |= E_BINDING_MODIFIER_CTRL;
   if (strstr(sig, "MOD:Alt")) mod |= E_BINDING_MODIFIER_ALT;
   if (strstr(sig, "MOD:Super")) mod |= E_BINDING_MODIFIER_WIN;
   for (l = signal_bindings; l; l = l->next)
     {
	E_Binding_Signal *bind;
	
	bind = l->data;
	if ((e_util_glob_match(sig, bind->sig)) &&
	    (e_util_glob_match(src, bind->src)) &&
	    ((bind->any_mod) || (bind->mod == mod)))
	  {
	     if (_e_bindings_context_match(bind->ctxt, ctxt))
	       {
		  E_Action *act;
		  
		  act = e_action_find(bind->action);
		  if (bind_ret) *bind_ret = bind;
		  return act;
	       }
	  }
     }
   return NULL;
}

E_Action *
e_bindings_signal_handle(E_Binding_Context ctxt, E_Object *obj, char *sig, char *src)
{
   E_Action *act;
   E_Binding_Signal *bind;
   
   act = e_bindings_signal_find(ctxt, obj, sig, src, &bind);
   if (act)
     {
	if (act->func.go_signal)
	  act->func.go_signal(obj, bind->params, sig, src);
	else if (act->func.go)
	  act->func.go(obj, bind->params);
	return act;
     }
   return act;
}

/* FIXME: finish off wheel bindings */
void
e_bindings_wheel_add(E_Binding_Context ctxt, int direction, int z, E_Binding_Modifier mod, int any_mod, char *action, char *params)
{
   E_Binding_Wheel *bind;
   
   bind = calloc(1, sizeof(E_Binding_Wheel));
   bind->ctxt = ctxt;
   bind->direction = direction;
   bind->z = z;
   bind->mod = mod;
   bind->any_mod = any_mod;
   bind->action = strdup(action);
   bind->params = strdup(params);
   wheel_bindings = evas_list_append(wheel_bindings, bind);
}

void
e_bindings_wheel_del(E_Binding_Context ctxt, int direction, int z, E_Binding_Modifier mod, int any_mod, char *action, char *params)
{
   Evas_List *l;
   
   for (l = wheel_bindings; l; l = l->next)
     {
	E_Binding_Wheel *bind;
	
	bind = l->data;
	if ((bind->ctxt == ctxt) &&
	    (bind->direction == direction) &&
	    (bind->z == z) &&
	    (bind->mod == mod) &&
	    (bind->any_mod == any_mod) &&
	    (!strcmp(bind->action, action)) &&
	    (!strcmp(bind->params, params)))
	  {
	     _e_bindings_wheel_free(bind);
	     wheel_bindings = evas_list_remove_list(wheel_bindings, l);
	     break;
	  }
     }
}

void
e_bindings_wheel_grab(E_Binding_Context ctxt, Ecore_X_Window win)
{
   Evas_List *l;

   for (l = wheel_bindings; l; l = l->next)
     {
	E_Binding_Wheel *bind;
	
	bind = l->data;
	if (_e_bindings_context_match(bind->ctxt, ctxt))
	  {
	     int mod;
	     int button = 0;
	     
	     mod = 0;
	     if (bind->mod & E_BINDING_MODIFIER_SHIFT) mod |= ECORE_X_MODIFIER_SHIFT;
	     if (bind->mod & E_BINDING_MODIFIER_CTRL) mod |= ECORE_X_MODIFIER_CTRL;
	     if (bind->mod & E_BINDING_MODIFIER_ALT) mod |= ECORE_X_MODIFIER_ALT;
	     if (bind->mod & E_BINDING_MODIFIER_WIN) mod |= ECORE_X_MODIFIER_WIN;
	     
	     if (bind->direction == 0)
	       {
		  if (bind->z < 0) button = 4;
		  else if (bind->z > 0) button = 5;
	       }
	     else if (bind->direction == 1)
	       {
		  if (bind->z < 0) button = 6;
		  else if (bind->z > 0) button = 7;
	       }
	     if (button != 0)
	       ecore_x_window_button_grab(win, button,
					  ECORE_X_EVENT_MASK_MOUSE_DOWN,
					  mod, bind->any_mod);
	  }
     }
}

void
e_bindings_wheel_ungrab(E_Binding_Context ctxt, Ecore_X_Window win)
{
   Evas_List *l;
   
   for (l = wheel_bindings; l; l = l->next)
     {
	E_Binding_Wheel *bind;
	
	bind = l->data;
	if (_e_bindings_context_match(bind->ctxt, ctxt))
	  {
	     int mod;
	     int button = 0;
	     
	     mod = 0;
	     if (bind->mod & E_BINDING_MODIFIER_SHIFT) mod |= ECORE_X_MODIFIER_SHIFT;
	     if (bind->mod & E_BINDING_MODIFIER_CTRL) mod |= ECORE_X_MODIFIER_CTRL;
	     if (bind->mod & E_BINDING_MODIFIER_ALT) mod |= ECORE_X_MODIFIER_ALT;
	     if (bind->mod & E_BINDING_MODIFIER_WIN) mod |= ECORE_X_MODIFIER_WIN;
	     if (bind->direction == 0)
	       {
		  if (bind->z < 0) button = 4;
		  else if (bind->z > 0) button = 5;
	       }
	     else if (bind->direction == 1)
	       {
		  if (bind->z < 0) button = 6;
		  else if (bind->z > 0) button = 7;
	       }
	     if (button != 0)
	       ecore_x_window_button_ungrab(win, button,
					    mod, bind->any_mod);
	  }
     }
}

E_Action *
e_bindings_wheel_find(E_Binding_Context ctxt, E_Object *obj, Ecore_X_Event_Mouse_Wheel *ev, E_Binding_Wheel **bind_ret)
{
   E_Binding_Modifier mod = 0;
   Evas_List *l;
   
   if (ev->modifiers & ECORE_X_MODIFIER_SHIFT) mod |= E_BINDING_MODIFIER_SHIFT;
   if (ev->modifiers & ECORE_X_MODIFIER_CTRL) mod |= E_BINDING_MODIFIER_CTRL;
   if (ev->modifiers & ECORE_X_MODIFIER_ALT) mod |= E_BINDING_MODIFIER_ALT;
   if (ev->modifiers & ECORE_X_MODIFIER_WIN) mod |= E_BINDING_MODIFIER_WIN;
   for (l = wheel_bindings; l; l = l->next)
     {
	E_Binding_Wheel *bind;
	
	bind = l->data;
	if ((bind->direction == ev->direction) &&
	    (((bind->z < 0) && (ev->z < 0)) || ((bind->z > 0) && (ev->z > 0))) &&
	    ((bind->any_mod) || (bind->mod == mod)))
	  {
	     if (_e_bindings_context_match(bind->ctxt, ctxt))
	       {
		  E_Action *act;
		  
		  act = e_action_find(bind->action);
		  if (bind_ret) *bind_ret = bind;
		  return act;
	       }
	  }
     }
   return NULL;
}

E_Action *
e_bindings_wheel_event_handle(E_Binding_Context ctxt, E_Object *obj, Ecore_X_Event_Mouse_Wheel *ev)
{
   E_Action *act;
   E_Binding_Wheel *bind;

   act = e_bindings_wheel_find(ctxt, obj, ev, &bind);
   if (act)
     {
	if (act->func.go_wheel)
	  act->func.go_wheel(obj, bind->params, ev);
	else if (act->func.go)
	  act->func.go(obj, bind->params);
	return act;
     }
   return act;
}

/* local subsystem functions */

static void
_e_bindings_mouse_free(E_Binding_Mouse *bind)
{
   E_FREE(bind->action);
   E_FREE(bind->params);
   free(bind);
}

static void
_e_bindings_key_free(E_Binding_Key *bind)
{
   E_FREE(bind->key);
   E_FREE(bind->action);
   E_FREE(bind->params);
   free(bind);
}

static void
_e_bindings_signal_free(E_Binding_Signal *bind)
{
   E_FREE(bind->sig);
   E_FREE(bind->src);
   E_FREE(bind->action);
   E_FREE(bind->params);
   free(bind);
}

static void
_e_bindings_wheel_free(E_Binding_Wheel *bind)
{
   E_FREE(bind->action);
   E_FREE(bind->params);
   free(bind);
}

static int
_e_bindings_context_match(E_Binding_Context bctxt, E_Binding_Context ctxt)
{
   if (bctxt == E_BINDING_CONTEXT_ANY) return 1;
   if (ctxt == E_BINDING_CONTEXT_UNKNOWN) return 0;
   if (bctxt == ctxt) return 1;
   return 0;
}
