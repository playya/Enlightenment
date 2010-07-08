#include "e.h"
#include "e_mod_main.h"
#include "itask_item.h"
#include "itask_items.h"


static Eina_Bool  _itask_items_cb_event_border_add(void *data, int type, void *event);
static Eina_Bool  _itask_items_cb_event_border_remove(void *data, int type, void *event);
static Eina_Bool  _itask_items_cb_event_border_iconify(void *data, int type, void *event);
static Eina_Bool  _itask_items_cb_event_border_uniconify(void *data, int type, void *event);
static Eina_Bool  _itask_items_cb_event_border_icon_change(void *data, int type, void *event);
static Eina_Bool  _itask_items_cb_event_border_property(void *data, int type, void *event);
static Eina_Bool  _itask_items_cb_event_border_zone_set(void *data, int type, void *event);
static Eina_Bool  _itask_items_cb_event_border_focus_in(void *data, int type, void *event);
static Eina_Bool  _itask_items_cb_event_border_focus_out(void *data, int type, void *event);
static Eina_Bool  _itask_items_cb_event_border_stack(void *data, int type, void *event);
static Itask_Item *_itask_items_find_item(Itask *it, E_Border *bd);
static Itask_Item *_itask_items_find_item_in_menu(Itask *it, E_Border *bd);
static Itask_Item *_itask_items_find_item_in_bar(Itask *it, E_Border *bd);

EAPI void
itask_items_init(Config *itask_config)
{

   itask_config->handlers = eina_list_append
                            (itask_config->handlers, ecore_event_handler_add
                             (E_EVENT_BORDER_ADD, _itask_items_cb_event_border_add, NULL));
   itask_config->handlers = eina_list_append
                            (itask_config->handlers, ecore_event_handler_add
                             (E_EVENT_BORDER_REMOVE, _itask_items_cb_event_border_remove, NULL));
   itask_config->handlers = eina_list_append
                            (itask_config->handlers, ecore_event_handler_add
                             (E_EVENT_BORDER_ICONIFY, _itask_items_cb_event_border_iconify, NULL));
   itask_config->handlers = eina_list_append
                            (itask_config->handlers, ecore_event_handler_add
                             (E_EVENT_BORDER_UNICONIFY, _itask_items_cb_event_border_uniconify, NULL));
   itask_config->handlers = eina_list_append
                            (itask_config->handlers, ecore_event_handler_add
                             (E_EVENT_BORDER_ICON_CHANGE, _itask_items_cb_event_border_icon_change, NULL));
   itask_config->handlers = eina_list_append
                            (itask_config->handlers, ecore_event_handler_add
                             (E_EVENT_BORDER_PROPERTY, _itask_items_cb_event_border_property, NULL));
   itask_config->handlers = eina_list_append
                            (itask_config->handlers, ecore_event_handler_add
			    (E_EVENT_BORDER_ZONE_SET, _itask_items_cb_event_border_zone_set, NULL));
   itask_config->handlers = eina_list_append
                            (itask_config->handlers, ecore_event_handler_add
                             (E_EVENT_BORDER_FOCUS_IN, _itask_items_cb_event_border_focus_in, NULL));
   itask_config->handlers = eina_list_append
                            (itask_config->handlers, ecore_event_handler_add
                             (E_EVENT_BORDER_FOCUS_OUT, _itask_items_cb_event_border_focus_out, NULL));
   itask_config->handlers = eina_list_append
                            (itask_config->handlers, ecore_event_handler_add
                             (E_EVENT_BORDER_STACK, _itask_items_cb_event_border_stack, NULL));
}



static Itask_Item *
_itask_items_find_item(Itask *it, E_Border *bd)
{
   Eina_List *l;
   Itask_Item *ic;

   for (l = it->items; l; l = l->next)
   {
      ic = l->data;
      if (ic->border == bd)
         return ic;
   }
   return NULL;
}



static Itask_Item *
_itask_items_find_item_in_menu(Itask *it, E_Border *bd)
{
   Eina_List *l;
   Itask_Item *ic;

   for (l = it->items_menu; l; l = l->next)
   {
      ic = l->data;
      if (ic->border == bd)
         return ic;
   }
   return NULL;
}



static Itask_Item *
_itask_items_find_item_in_bar(Itask *it, E_Border *bd)
{
   Eina_List *l;
   Itask_Item *ic;

   for (l = it->items_bar; l; l = l->next)
   {
      ic = l->data;
      if (ic->border == bd)
         return ic;
   }
   return NULL;
}



static Eina_Bool
_itask_items_cb_event_border_add(void *data, int type, void *event)
{
   E_Event_Border_Add *ev;
   E_Border *bd;
   Itask *it;
   Eina_List *l, *itask;
   Itask_Item *ic;
   
   ev = event;
   bd = ev->border;
   itask = itask_zone_find(bd->zone);
   for (l = itask; l; l = l->next)
   {
      it = l->data;
      ic = itask_item_new(it, bd);
      if(ic) itask_item_realize(ic);
   }
   while (itask)
      itask = eina_list_remove_list(itask, itask);

   return EINA_TRUE;
}



static Eina_Bool
_itask_items_cb_event_border_remove(void *data, int type, void *event)
{
   //printf("_itask_items_cb_event_border_remove\n");
   E_Event_Border_Remove *ev;
   Itask *it;
   Itask_Item *ic;
   Eina_List *l, *itask;

   ev = event;

   itask = itask_zone_find(ev->border->zone);
   for (l = itask; l; l = l->next)
   {
      it = l->data;
      ic = _itask_items_find_item(it, ev->border);
      if (!ic)
         return;

      itask_item_remove(ic);
      itask_item_swap_youngest_from_menu(it);

      itask_update_gc_orient(it);
   }
   while (itask)
      itask = eina_list_remove_list(itask, itask);

   return EINA_TRUE;
}



static Eina_Bool
_itask_items_cb_event_border_uniconify(void *data, int type, void *event)
{
   E_Event_Border_Iconify *ev;
   Itask *it;
   Itask_Item *ic;
   Eina_List *l, *itask;

   ev = event;

   itask = itask_zone_find(ev->border->zone);
   for (l = itask; l; l = l->next)
   {
      it = l->data;
      ic = _itask_items_find_item_in_bar(it, ev->border);
      if (!ic)
         continue;
      itask_icon_signal_emit(ic, "uniconify", "");
      //ic->last_time = ecore_time_get();
   }
   while (itask)
      itask = eina_list_remove_list(itask, itask);

   return EINA_TRUE;
}



static Eina_Bool
_itask_items_cb_event_border_iconify(void *data, int type, void *event)
{
   E_Event_Border_Uniconify *ev;
   Itask *it;
   Itask_Item *ic;
   Eina_List *l, *itask;

   ev = event;

   itask = itask_zone_find(ev->border->zone);
   for (l = itask; l; l = l->next)
   {
      it = l->data;
      ic = _itask_items_find_item_in_bar(it, ev->border);
      if (!ic)
         continue;
      itask_icon_signal_emit(ic, "iconify", "");
   }
   while (itask)
      itask = eina_list_remove_list(itask, itask);

   return EINA_TRUE;
}



static Eina_Bool
_itask_items_cb_event_border_icon_change(void *data, int type, void *event)
{
   E_Event_Border_Icon_Change *ev;
   Itask *it;
   Itask_Item *ic;
   Eina_List *l, *itask;

   ev = event;

   itask = itask_zone_find(ev->border->zone);
   for (l = itask; l; l = l->next)
   {
      it = l->data;
      ic = _itask_items_find_item_in_bar(it, ev->border);
      if (!ic)
         continue;
      itask_item_del_icon(ic);
      itask_item_set_icon(ic);
   }
   while (itask)
      itask = eina_list_remove_list(itask, itask);

   return EINA_TRUE;
}


static Eina_Bool
_itask_items_cb_event_border_property(void *data, int type, void *event)
{
   E_Event_Border_Property *ev;
   E_Border *bd;
   Itask *it;
   Itask_Item *ic;
   Eina_List *l, *itask;
   char *label;

   ev = event;
   bd = ev->border;

   itask = itask_zone_find(bd->zone);
   for (l = itask; l; l = l->next)
   {
      it = l->data;
      ic = _itask_items_find_item_in_bar(it, bd);
      if(ic)
      {
         int remove= 0;
         if(!itask_item_add_check(it, bd))
            remove = 1;

         //ic = _itask_items_find_item(it, bd);
         if (!ic && !remove)
         {
            itask_item_new(it, bd);
            continue;
         }
         else if (!ic && remove)
         {
               continue;
         }
         else if (ic && remove)
         {
               it->items = eina_list_remove(it->items, ic);
               itask_item_remove(ic);
              // itask_resize_handle(it);
               itask_update_gc_orient(it); 
               continue;
         }
         else /* if(ic && !remove)*/
         {
            // some other stuff to update ?
            itask_item_set_label(ic);
         }
      }
   }
   while (itask)
      itask = eina_list_remove_list(itask, itask);

   return EINA_TRUE;
}



static Eina_Bool
_itask_items_cb_event_border_focus_in(void *data, int type, void *event)
{
//	printf("_itask_items_cb_event_border_focus\n");
   E_Event_Border_Focus_In *ev;
   Itask *it;
   Itask_Item *ic;
   E_Border* bd;
   Eina_List *l, *ll, *itask;

   ev = event;
   bd = ev->border;
   itask = itask_zone_find(bd->zone);

   for (l = itask; l; l = l->next)
   {
      it = l->data;
      ic = _itask_items_find_item_in_bar(it, ev->border);
      if (ic)
      {  
      	 if(it->swap_on_focus) // this should probably happen always
      	   ic->last_time = ecore_time_get();
         itask_icon_signal_emit(ic, "focused", "");
      }
      if(it->swap_on_focus)
      {
      ic = _itask_items_find_item_in_menu(it, ev->border);
      if (ic)
      {
         
         itask_item_swap_to_bar(ic);
         itask_item_swap_oldest_from_bar(it);
         
         itask_resize_handle(it);
         itask_update_gc_orient(it);
         
         itask_icon_signal_emit(ic, "focused", "");
      }
      }
   }
   while (itask)
      itask = eina_list_remove_list(itask, itask);

   return EINA_TRUE;
}



static Eina_Bool
_itask_items_cb_event_border_focus_out(void *data, int type, void *event)
{
   E_Event_Border_Focus_Out *ev;
   Itask *it;
   Itask_Item *ic;
   Eina_List *l, *itask;

   ev = event;
   itask = itask_zone_find(ev->border->zone);

   for (l = itask; l; l = l->next)
   {
      it = l->data;
      ic = _itask_items_find_item_in_bar(it, ev->border);
      if (ic)
         itask_icon_signal_emit(ic, "unfocused", "");
   }
   while (itask)
      itask = eina_list_remove_list(itask, itask);

   return EINA_TRUE;
}



static Eina_Bool
_itask_items_cb_event_border_zone_set(void *data, int type, void *event)
{
   E_Event_Border_Zone_Set *ev;
   Itask *it;
   Itask_Item *ic;
   Eina_List *l, *itask;
   E_Border *bd;

   ev = event;
   bd = ev->border;
 
   /* delete from current zone itask, add to new one */
   for (l = itask_config->instances; l; l = l->next)
   {
      if(!l->data){
         continue;
      }         

      it = ((Instance *) l->data)->itask;
      if(!it){
         continue;
      }         
      if (it->show_zone != 0)
      {
          if(it->zone == ev->zone && (ic = itask_item_new(it, bd)))
          { 
             if(ic) itask_item_realize(ic);
          }
          if(it->zone != ev->zone && (ic = _itask_items_find_item(it, ev->border)))
          {
            itask_item_remove(ic);
            itask_item_swap_youngest_from_menu(it);
          }   
      }
   }

   return EINA_TRUE;
}


static Eina_Bool 
_itask_items_cb_event_border_stack(void *data, int type, void *event)
{
   E_Event_Border_Stack *ev;
   Itask *it;
   Itask_Item *ic;
   Eina_List *l, *itask;
 // printf("_itask_items_cb_event_border_stack\n");
   ev = event;
   itask = itask_zone_find(ev->border->zone);
   
   // TODO do this only on stack above event
   for (l = itask; l; l = l->next)
   {
      it = l->data;
      ic = _itask_items_find_item_in_menu(it, ev->border);
      if (ic)
      {
         
         itask_item_swap_to_bar(ic);
         
         itask_item_swap_oldest_from_bar(it);
         
         itask_resize_handle(it);
         
         itask_icon_signal_emit(ic, "focused", "");       
      }
   }
   while (itask)
      itask = eina_list_remove_list(itask, itask);

   return EINA_TRUE;
}

