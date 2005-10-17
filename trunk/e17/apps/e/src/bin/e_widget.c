/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

#define SMART_NAME "e_widget"
#define API_ENTRY E_Smart_Data *sd; sd = evas_object_smart_data_get(obj); if ((!obj) || (!sd) || (evas_object_type_get(obj) && strcmp(evas_object_type_get(obj), SMART_NAME)))
#define INTERNAL_ENTRY E_Smart_Data *sd; sd = evas_object_smart_data_get(obj); if (!sd) return;
typedef struct _E_Smart_Data E_Smart_Data;

struct _E_Smart_Data
{ 
   Evas_Object   *parent_obj;
   Evas_Coord     x, y, w, h;
   Evas_Coord     minw, minh;
   Evas_List     *subobjs;
   Evas_Object   *resize_obj;
   void         (*del_func) (Evas_Object *obj);
   void         (*focus_func) (Evas_Object *obj);
   void         (*activate_func) (Evas_Object *obj);
   void         (*disable_func) (Evas_Object *obj);
   void         (*on_focus_func) (void *data, Evas_Object *obj);
   void          *on_focus_data;
   void         (*on_change_func) (void *data, Evas_Object *obj);
   void          *on_change_data;
   void          *data;
   unsigned char  can_focus : 1;
   unsigned char  child_can_focus : 1;
   unsigned char  focused : 1;
   unsigned char  disabled : 1;
}; 

/* local subsystem functions */
static void _e_smart_reconfigure(E_Smart_Data *sd);
static void _e_smart_add(Evas_Object *obj);
static void _e_smart_del(Evas_Object *obj);
static void _e_smart_layer_set(Evas_Object *obj, int layer);
static void _e_smart_raise(Evas_Object *obj);
static void _e_smart_lower(Evas_Object *obj);
static void _e_smart_stack_above(Evas_Object *obj, Evas_Object *above);
static void _e_smart_stack_below(Evas_Object *obj, Evas_Object *below);
static void _e_smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y);
static void _e_smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h);
static void _e_smart_show(Evas_Object *obj);
static void _e_smart_hide(Evas_Object *obj);
static void _e_smart_color_set(Evas_Object *obj, int r, int g, int b, int a);
static void _e_smart_clip_set(Evas_Object *obj, Evas_Object * clip);
static void _e_smart_clip_unset(Evas_Object *obj);
static void _e_smart_init(void);

/* local subsystem globals */
static Evas_Smart *_e_smart = NULL;

/* externally accessible functions */
Evas_Object *
e_widget_add(Evas *evas)
{
   _e_smart_init();
   return evas_object_smart_add(evas, _e_smart);
}

void
e_widget_del_hook_set(Evas_Object *obj, void (*func) (Evas_Object *obj))
{
   API_ENTRY return;
   sd->del_func = func;
}

void
e_widget_focus_hook_set(Evas_Object *obj, void (*func) (Evas_Object *obj))
{
   API_ENTRY return;
   sd->focus_func = func;
}

void
e_widget_activate_hook_set(Evas_Object *obj, void (*func) (Evas_Object *obj))
{
   API_ENTRY return;
   sd->activate_func = func;
}

void
e_widget_disable_hook_set(Evas_Object *obj, void (*func) (Evas_Object *obj))
{
   API_ENTRY return;
   sd->disable_func = func;
}

void
e_widget_on_focus_hook_set(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj), void *data)
{
   API_ENTRY return;
   sd->on_focus_func = func;
   sd->on_focus_data = data;
}

void
e_widget_on_change_hook_set(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj), void *data)
{
   API_ENTRY return;
   sd->on_change_func = func;
   sd->on_change_data = data;
}

void
e_widget_data_set(Evas_Object *obj, void *data)
{
   API_ENTRY return;
   sd->data = data;
}

void *
e_widget_data_get(Evas_Object *obj)
{
   API_ENTRY return NULL;
   return sd->data;
}

void
e_widget_min_size_set(Evas_Object *obj, Evas_Coord minw, Evas_Coord minh)
{
   API_ENTRY return;
   sd->minw = minw;
   sd->minh = minh;
}

void
e_widget_min_size_get(Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh)
{
   API_ENTRY return;
   if (minw) *minw = sd->minw;
   if (minh) *minh = sd->minh;
}

void
e_widget_sub_object_add(Evas_Object *obj, Evas_Object *sobj)
{
   API_ENTRY return;
   sd->subobjs = evas_list_append(sd->subobjs, sobj);
   if (!sd->child_can_focus)
     {
	if (e_widget_can_focus_get(sobj)) sd->child_can_focus = 1;
     }
   if (!strcmp(evas_object_type_get(sobj), SMART_NAME))
     {
	sd = evas_object_smart_data_get(sobj);
	if (sd) sd->parent_obj = obj;
     }
}

void
e_widget_resize_object_set(Evas_Object *obj, Evas_Object *sobj)
{
   API_ENTRY return;
   if (sd->resize_obj) evas_object_smart_member_del(sd->resize_obj);
   sd->resize_obj = sobj;
   evas_object_smart_member_add(obj, sobj);
   _e_smart_reconfigure(sd);
}

void
e_widget_can_focus_set(Evas_Object *obj, int can_focus)
{
   API_ENTRY return;
   sd->can_focus = can_focus;
}

int
e_widget_can_focus_get(Evas_Object *obj)
{
   API_ENTRY return 0;
   if (sd->can_focus) return 1;
   if (sd->child_can_focus) return 1;
   return 0;
}

int
e_widget_focus_get(Evas_Object *obj)
{
   API_ENTRY return 0;
   return sd->focused;
}

Evas_Object *
e_widget_focused_object_get(Evas_Object *obj)
{
   Evas_List *l;
   API_ENTRY return NULL;
   if (!sd->focused) return NULL;
   for (l = sd->subobjs; l; l = l->next)
     {  
	Evas_Object *fobj;
	
	fobj = e_widget_focused_object_get(l->data);
	if (fobj) return fobj;
     }
   return obj;
}

int
e_widget_focus_jump(Evas_Object *obj, int forward)
{
   API_ENTRY return 0;
   if (!e_widget_can_focus_get(obj)) return 0;
       
   /* if it has a focus func its an end-point widget like a button */
   if (sd->focus_func)
     {
	if (!sd->focused) sd->focused = 1;
	else sd->focused = 0;
	sd->focus_func(obj);
	if (sd->on_focus_func) sd->on_focus_func(sd->on_focus_data, obj);
	return sd->focused;
     }
   /* its some container */
   else
     {
	Evas_List *l;
	int focus_next;
	
	focus_next = 0;
	if (!sd->focused)
	  {
	     e_widget_focus_set(obj, forward);
	     sd->focused = 1;
	     if (sd->on_focus_func) sd->on_focus_func(sd->on_focus_data, obj);
	     return 1;
	  }
	else
	  {
	     if (forward)
	       {
		  for (l = sd->subobjs; l; l = l->next)
		    {
		       if (e_widget_can_focus_get(l->data))
			 {
			    if ((focus_next) &&
				(!e_widget_disabled_get(l->data)))
			      {
				 /* the previous focused item was unfocused - so focus
				  * the next one (that can be focused) */
				 if (e_widget_focus_jump(l->data, forward)) return 1;
				 else break;
			      }
			    else
			      {
				 if (e_widget_focus_get(l->data))
				   {
				      /* jump to the next focused item or focus this item */
				      if (e_widget_focus_jump(l->data, forward)) return 1;
				      /* it returned 0 - it got to the last item and is past it */
				      focus_next = 1;
				   }
			      }
			 }
		    }
	       }
	     else
	       {
		  for (l = evas_list_last(sd->subobjs); l; l = l->prev)
		    {
		       if (e_widget_can_focus_get(l->data))
			 {
			    if ((focus_next) &&
				(!e_widget_disabled_get(l->data)))
			      {
				 /* the previous focused item was unfocused - so focus
				  * the next one (that can be focused) */
				 if (e_widget_focus_jump(l->data, forward)) return 1;
				 else break;
			      }
			    else
			      {
				 if (e_widget_focus_get(l->data))
				   {
				      /* jump to the next focused item or focus this item */
				      if (e_widget_focus_jump(l->data, forward)) return 1;
				      /* it returned 0 - it got to the last item and is past it */
				      focus_next = 1;
				   }
			      }
			 }
		    }
	       }
	  }
     }
   /* no next item can be focused */
   sd->focused = 0;
   return 0;
}

void
e_widget_focus_set(Evas_Object *obj, int first)
{
   API_ENTRY return;
   if (!sd->focused)
     {
	sd->focused = 1;
	if (sd->on_focus_func) sd->on_focus_func(sd->on_focus_data, obj);
     }
   if (sd->focus_func)
     {
	sd->focus_func(obj);
	return;
     }
   else
     {
	Evas_List *l;
	     
	if (first)
	  {
	     for (l = sd->subobjs; l; l = l->next)
	       {
		  if ((e_widget_can_focus_get(l->data)) &&
		      (!e_widget_disabled_get(l->data)))
		    {
		       e_widget_focus_set(l->data, first);
		       break;
		    }
	       }
	  }
	else
	  {
	     for (l = evas_list_last(sd->subobjs); l; l = l->prev)
	       {
		  if ((e_widget_can_focus_get(l->data)) &&
		      (!e_widget_disabled_get(l->data)))
		    {
		       e_widget_focus_set(l->data, first);
		       break;
		    }
	       }
	  }
     }
}

Evas_Object *
e_widget_parent_get(Evas_Object *obj)
{
   API_ENTRY return NULL;
   return sd->parent_obj;
}

void
e_widget_focused_object_clear(Evas_Object *obj)
{
   Evas_List *l;
   API_ENTRY return;
   if (!sd->focused) return;
   sd->focused = 0;
   for (l = sd->subobjs; l; l = l->next)
     {  
	if (e_widget_focus_get(l->data))
	  {
	     e_widget_focused_object_clear(l->data);
	     break;
	  }
     }
   if (sd->focus_func) sd->focus_func(obj);
}

void
e_widget_focus_steal(Evas_Object *obj)
{
   Evas_Object *parent, *o;
   API_ENTRY return;
   if (sd->focused) return;
   if (sd->disabled) return;
   parent = obj;
   for (;;)
     {
	o = e_widget_parent_get(parent);
	if (!o) break;
	parent = o;
     }
   e_widget_focused_object_clear(parent);
   parent = obj;
   for (;;)
     {
	sd = evas_object_smart_data_get(parent);
	sd->focused = 1;
	if (sd->on_focus_func) sd->on_focus_func(sd->on_focus_data, parent);
	o = e_widget_parent_get(parent);
	if (!o) break;
	parent = o;
     }
   sd = evas_object_smart_data_get(obj);
   if (sd->focus_func) sd->focus_func(obj);
   return;
}

void
e_widget_activate(Evas_Object *obj)
{
   API_ENTRY return;
   if (sd->activate_func) sd->activate_func(obj);
   e_widget_change(obj);
}

void
e_widget_change(Evas_Object *obj)
{
   API_ENTRY return;
   if (sd->on_change_func) sd->on_change_func(sd->on_change_data, obj);
   e_widget_change(e_widget_parent_get(obj));
}

void
e_widget_disabled_set(Evas_Object *obj, int disabled)
{
   API_ENTRY return;
   if (((sd->disabled) && (disabled)) ||
       ((!sd->disabled) && (!disabled))) return;
   sd->disabled = disabled;
   if (sd->focused)
     {
	Evas_Object *o, *parent;

	parent = obj;
        for (;;)
          {
	     o = e_widget_parent_get(parent);
	     if (!o) break;
	     parent = o;
	  }
	e_widget_focus_jump(parent, 1);
     }
   if (sd->disable_func) sd->disable_func(obj);
}

int
e_widget_disabled_get(Evas_Object *obj)
{
   API_ENTRY return 0;
   return sd->disabled;
}

/* local subsystem functions */
static void
_e_smart_reconfigure(E_Smart_Data *sd)
{
   if (sd->resize_obj)
     {
	evas_object_move(sd->resize_obj, sd->x, sd->y);
	evas_object_resize(sd->resize_obj, sd->w, sd->h);
     }
}

static void
_e_smart_add(Evas_Object *obj)
{
   E_Smart_Data *sd;
   
   sd = calloc(1, sizeof(E_Smart_Data));
   if (!sd) return;
   sd->x = 0;
   sd->y = 0;
   sd->w = 0;
   sd->h = 0;
   sd->can_focus = 1;
   evas_object_smart_data_set(obj, sd);
}

static void
_e_smart_del(Evas_Object *obj)
{
   INTERNAL_ENTRY;
   if (sd->del_func) sd->del_func(obj);
   while (sd->subobjs)
     {
	evas_object_del(sd->subobjs->data);
	sd->subobjs = evas_list_remove_list(sd->subobjs, sd->subobjs);
     }
   free(sd);
}
   
static void
_e_smart_layer_set(Evas_Object *obj, int layer)
{
   INTERNAL_ENTRY;
   evas_object_layer_set(sd->resize_obj, layer);
}

static void
_e_smart_raise(Evas_Object *obj)
{
   INTERNAL_ENTRY;
   evas_object_raise(sd->resize_obj);
}

static void
_e_smart_lower(Evas_Object *obj)
{
   INTERNAL_ENTRY;
   evas_object_lower(sd->resize_obj);
}
                                                             
static void
_e_smart_stack_above(Evas_Object *obj, Evas_Object *above)
{
   INTERNAL_ENTRY;
   evas_object_stack_above(sd->resize_obj, above);
}
   
static void
_e_smart_stack_below(Evas_Object *obj, Evas_Object *below)
{
   INTERNAL_ENTRY;
   evas_object_stack_below(sd->resize_obj, below);
}

static void
_e_smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   INTERNAL_ENTRY;
   sd->x = x;
   sd->y = y;
   _e_smart_reconfigure(sd);
}

static void
_e_smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   INTERNAL_ENTRY;
   sd->w = w;
   sd->h = h;
   _e_smart_reconfigure(sd);
}

static void
_e_smart_show(Evas_Object *obj)
{
   INTERNAL_ENTRY;
   evas_object_show(sd->resize_obj);
}

static void
_e_smart_hide(Evas_Object *obj)
{
   INTERNAL_ENTRY;
   evas_object_hide(sd->resize_obj);
}

static void
_e_smart_color_set(Evas_Object *obj, int r, int g, int b, int a)
{
   INTERNAL_ENTRY;
   evas_object_color_set(sd->resize_obj, r, g, b, a);
}

static void
_e_smart_clip_set(Evas_Object *obj, Evas_Object *clip)
{
   INTERNAL_ENTRY;
   evas_object_clip_set(sd->resize_obj, clip);
}

static void
_e_smart_clip_unset(Evas_Object *obj)
{
   INTERNAL_ENTRY;
   evas_object_clip_unset(sd->resize_obj);
}  

/* never need to touch this */

static void
_e_smart_init(void)
{
   if (_e_smart) return;
   _e_smart = evas_smart_new
     (SMART_NAME, _e_smart_add, _e_smart_del, _e_smart_layer_set,
      _e_smart_raise, _e_smart_lower, _e_smart_stack_above,
      _e_smart_stack_below, _e_smart_move, _e_smart_resize,
      _e_smart_show, _e_smart_hide, _e_smart_color_set,
      _e_smart_clip_set, _e_smart_clip_unset, NULL);
}
