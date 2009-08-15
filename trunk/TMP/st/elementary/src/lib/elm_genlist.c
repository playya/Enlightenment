#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Genlist Genlist
 *
 * The aim was to have  more expansive list that the simple list in
 * Elementary that could have more flexible items and allow many more entries
 * while still being fast and low on memory usage. At the same time it was
 * also made to be able to do tree structures. But the price to pay is more
 * complexity when it comes to usage. If all you want is a simple list with
 * icons and a single label, use the normal List object.
 *
 * Signals that you can add callbacks for are:
 *
 * clicked - This is called when a user has double-clicked an item. The
 * event_info parameter is the genlist item that as double-clicked.
 *
 * selected - This is called when a user has made an item selected. The
 * event_info parameter is the genlist item that was selected.
 *
 * unselected - This is called when a user has made an item unselected. The
 * event_info parameter is the genlist item that was unselected.
 *
 * expanded -  This is called when elm_genlist_item_expanded_set() is called
 * and the item is now meant to be expanded. The event_info parameter is the
 * genlist item that was indicated to expand. It is the job of this callback
 * to then fill in the child items.
 *
 * contracted - This is called when elm_genlist_item_expanded_set() is called
 * and the item is now meant to be contracted. The event_info parameter is
 * the genlist item that was indicated to contract. It is the job of this
 * callback to then delete the child items
 *
 * expand,request - This is called when a user has indicated they want to
 * expand a tree branch item. The callback should decide if the item can
 * expand (has any children) and then call elm_genlist_item_expanded_set()
 * appropriately to set the state. The event_info parameter is the genlist
 * item that was indicated to expand.
 *
 * contract,request - This is called when a user has indicated they want to
 * contract a tree branch item. The callback should decide if the item can
 * contract (has any children) and then call elm_genlist_item_expanded_set()
 * appropriately to set the state. The event_info parameter is the genlist
 * item that was indicated to contract.
 *
 * Genlist has a fairly large API, mostly because it's relatively complex,
 * trying to be both expansive, powerful and efficient. First we will begin
 * an overview o the theory behind genlist.
 *
 * Evas tracks every object you create. Every time it processes an event
 * (mouse move, down, up etc.) it needs to walk through objects and find out
 * what event that affects. Even worse every time it renders display updates,
 * in order to just calculate what to re-draw, it needs to walk through many
 * many many objects. Thus, the more objects you keep active, the more
 * overhead Evas has in just doing its work. It is advisable to keep your
 8 active objects to the minimum working set you need. Also remember that
 * object creation and deletion carries an overhead, so there is a
 * middle-ground, which is not easily determined. But don't keep massive lists
 * of objects you can't see or use. Genlist does this with list objects. It
 * creates and destroys them dynamically as you scroll around. It groups them
 * into blocks so it can determine the visibility etc. of a whole block at
 * once as opposed to having to walk the whole list. This 2-level list allows
 * for very large numbers of items to be in the list (tests have used up to
 * 2,000,000 items). Also genlist employs a queue for adding items. As items
 * may be different sizes, every item added needs to be calculated as to its
 * size and thus this presents a lot of overhead on populating the list, this
 * genlist employs a queue. Any item added is queued and spooled off over
 * time, actually appearing some time later, so if your list has many members
 * you may find it takes a while for them to all appear, with your process
 * consuming a lot of CPU while it is busy spooling.
 *
 * Genlist also implements a tree structure, but it does so with callbacks to
 * the application, with the application filling in tree structures when
 * requested (allowing for efficient building of a very deep tree that could
 * even be used for file-management). See the above smart signal callbacks for
 * details.
 *
 * An item in the genlist world can have 0 or more text labels (they can be
 * regular text or textblock – that's up to the style to determine), 0 or
 * more icons (which are simply objects swallowed into the genlist item) and
 * 0 or more boolean states that can be used for check, radio or other
 * indicators by the edje theme style. An item may be one of several styles
 * (Elementary provides 2 by default - “default” and “double_label”, but this
 * can be extended by system or application custom themes/overlays/extensions).
 *
 * In order to implement the ability to add and delete items on the fly,
 * Genlist implements a class/callback system where the application provides
 * a structure with information about that type of item (genlist may contain
 * multiple different items with different classes, states and styles).
 * Genlist will call the functions in this struct (methods) when an item is
 * “realized” (that is created dynamically while scrolling). All objects will
 * simply be deleted  when no longer needed with evas_object_del(). The
 * Elm_Genlist_Item_Class structure contains the following members:
 *
 * item_style - This is a constant string and simply defines the name of the
 * item style. It must be specified and the default should be “default”.
 *
 * func.label_get - This function is called when an actual item object is
 * created. The data parameter is the data parameter passed to
 * elm_genlist_item_append() and related item creation functions. The obj
 * parameter is the genlist object and the part parameter is the string name
 * of the text part in the edje design that is listed as one of the possible
 * labels that can be set. This function must return a strudup()'ed string as
 * the caller will free() it when done.
 *
 * func.icon_get - This function is called when an actual item object is
 * created. The data parameter is the data parameter passed to
 * elm_genlist_item_append() and related item creation functions. The obj
 * parameter is the genlist object and the part parameter is the string name
 * of the icon part in the edje design that is listed as one of the possible
 * icons that can be set. This must return NULL for no object or a valid
 * object. The object will be deleted by genlist on shutdown or when the item
 * its unrealized.
 *
 * func.state_get - This function is called when an actual item object is
 * created. The data parameter is the data parameter passed to
 * elm_genlist_item_append() and related item creation functions. The obj
 * parameter is the genlist object and the part parameter is the string name
 * of the state part in the edje design that is listed as one of the possible
 * states that can be set. Return 0 for false or 1 for true. Genlist will
 * emit a signal to the edje object with “elm,state,XXX,active” “elm” when
 * true (the default is false), where XXX is the name of the part.
 *
 * func.del - This is called when elm_genlist_item_del() is called on an
 * item, elm_genlist_clear() is called on the genlist, or
 * elm_genlist_item_subitems_clear() is called to clear sub-items. This is
 * intended for use when actual genlist items are deleted, so any backing
 * data attached to the item (e.g. its data parameter on creation) can be
 * deleted.
 *
 * Items can be added by several calls. All of them return a Elm_Genlist_Item
 * handle that is an internal member inside the genlist. They all take a data
 * parameter that is meant to be used for a handle to the applications
 * internal data (eg the struct with the original item data). The parent
 * parameter is the parent genlist item this belongs to if it is a tree, and
 * NULL if there is no parent. The flags can be a bitmask of
 * ELM_GENLIST_ITEM_NONE and ELM_GENLIST_ITEM_SUBITEMS. If
 * ELM_GENLIST_ITEM_SUBITEMS is set then this item is displayed as a item
 * that is able to expand and have child items. The func parameter is a
 * convenience callback that is called when the item is selected and the data
 * parameter will be the func_data parameter, obj be the genlist object and
 * vent_info will be the genlist item.
 *
 * elm_genlist_item_append() appends an item to the end of the list, or if
 * there is a parent, to the end of all the child items of the parent.
 * elm_genlist_item_prepend() is the same but prepends to the beginning of
 * the list or children list. elm_genlist_item_insert_before() inserts at
 * item before another item and elm_genlist_item_insert_after() inserts after
 * the indicated item.
 *
 * The application can clear the list with elm_genlist_clear() which deletes
 * all the items in the list and elm_genlist_item_del() will delete a specific
 * item. elm_genlist_item_subitems_clear() will clear all items that are
 * children of the indicated parent item.
 *
 * If the application wants multiple items to be able to be selected,
 * elm_genlist_multi_select_set() can enable this. If the list is
 * single-selection only (the default), then elm_genlist_selected_item_get()
 * will return the selected item, if any, or NULL I none is selected. If the
 * list is multi-select then elm_genlist_selected_items_get() will return a
 * list (that is only valid as long as no items are modified (added, deleted,
 * selected or unselected).
 *
 * To help inspect list items you can jump to the item at the top of the list
 * with elm_genlist_first_item_get() which will return the item pointer, and
 * similarly elm_genlist_last_item_get() gets the item at the end of the list.
 * elm_genlist_item_next_get() and elm_genlist_item_prev_get() get the next
 * and previous items respectively relative to the indicated item. Using
 * these calls you can walk the entire item list/tree. Note that as a tree
 * the items are flattened in the list, so elm_genlist_item_parent_get() will
 * let you know which item is the parent (and thus know how to skip them if
 * wanted).
 *
 * There are also convenience functions. elm_genlist_item_genlist_get() will
 * return the genlist object the item belongs to.  elm_genlist_item_show()
 * will make the scroller scroll to show that specific item so its visible.
 * elm_genlist_item_data_get() returns the data pointer set by the item
 * creation functions.
 *
 * If an item changes (state of boolean changes, label or icons change),
 * then use elm_genlist_item_update() to have genlist update the item with
 * the new state. Genlist will re-realize the item thus call the functions
 * in the _Elm_Genlist_Item_Class for that item.
 *
 * To programmatically (un)select an item use elm_genlist_item_selected_set().
 * To get its selected state use elm_genlist_item_selected_get(). Similarly
 * to expand/contract and item and get its expanded state, use
 * elm_genlist_item_expanded_set() and elm_genlist_item_expanded_get(). And
 * again to make an item disabled (unable to be selected and appear
 * differently) use elm_genlist_item_disabled_set() to set this and
 * elm_genlist_item_disabled_get() to get the disabled state.
 *
 * In general to indicate how the genlist should expand items horizontally to
 * fill the list area, use elm_genlist_horizontal_mode_set(). Valid modes are
 * ELM_LIST_LIMIT and ELM_LIST_SCROLL . The default is ELM_LIST_SCROLL. This
 * mode means that if items are too wide to fit, the scroller will scroll
 * horizontally. Otherwise items are expanded to fill the width of the
 * viewport of the scroller. If it is ELM_LIST_LIMIT, Items will be expanded
 * to the viewport width and limited to that size. This can be combined with
 * a different style that uses edjes' ellipsis feature (cutting text off like
 * this: “tex...”).
 *
 * Items will only call their selection func and callback when first becoming
 * selected. Any further clicks will do nothing, unless you enable always
 * select with elm_genlist_always_select_mode_set(). This means even if
 * selected, every click will make the selected callbacks be called.
 * elm_genlist_no_select_mode_set() will turn off the ability to select
 * items entirely and they will neither appear selected nor call selected
 * callback functions.
 *
 * Remember that you can create new styles and add you own theme augmentation
 * per application with elm_theme_extension_add(). If you absolutely must
 * have a specific style that overrides any theme the user or system sets up
 * you can use elm_theme_overlay_add() to add such a file.
 */
typedef struct _Widget_Data Widget_Data;
typedef struct _Item_Block Item_Block;
typedef struct _Pan Pan;

struct _Widget_Data
{
   Evas_Object *obj;
   Evas_Object *scr;
   Evas_Object *pan_smart;
   Eina_Inlist *items;
   Eina_Inlist *blocks;
   Pan *pan;
   Evas_Coord pan_x, pan_y, minw, minh;
   Ecore_Job *calc_job;
   Ecore_Idler *queue_idler;
   Eina_List *queue;
   Eina_List *selected;
   Elm_Genlist_Item *show_item;
   Elm_List_Mode mode;
   Eina_Bool on_hold : 1;
   Eina_Bool multi : 1;
   Eina_Bool min_w : 1;
   Eina_Bool min_h : 1;
   Eina_Bool always_select : 1;
   Eina_Bool no_select : 1;
};

struct _Item_Block
{
   EINA_INLIST;
   int count;
   Widget_Data *wd;
   Eina_List *items;
   Evas_Coord x, y, w, h, minw, minh;
   Eina_Bool realized : 1;
   Eina_Bool changed : 1;
};

struct _Elm_Genlist_Item
{
   EINA_INLIST;
   Widget_Data *wd;
   Item_Block *block;
   Eina_List *items; // FIXME: not done yet
   Evas_Coord x, y, w, h, minw, minh;
   const Elm_Genlist_Item_Class *itc;
   const void *data;
   Elm_Genlist_Item *parent; // not done yet
   Elm_Genlist_Item_Flags flags;
   struct {
      void (*func) (void *data, Evas_Object *obj, void *event_info);
      const void *data;
   } func;

   Evas_Object *base;
   Evas_Object *spacer;
   Eina_List *labels, *icons, *states;
   Eina_List *icon_objs;

   Elm_Genlist_Item *rel;
   int relcount;
   Eina_Bool before : 1;

   Eina_Bool realized : 1;
   Eina_Bool selected : 1;
   Eina_Bool hilighted : 1;
   Eina_Bool expanded : 1; // FIXME: not done yet
   Eina_Bool disabled : 1;
   Eina_Bool mincalcd : 1;
   Eina_Bool queued : 1;
   Eina_Bool showme : 1;
   Eina_Bool delete_me : 1;
};

struct _Pan
{
   Evas_Object_Smart_Clipped_Data __clipped_data;
   Widget_Data *wd;
};

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _show_region_hook(void *data, Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);

static void _item_unrealize(Elm_Genlist_Item *it);
static void _item_block_unrealize(Item_Block *itb);
static void _calc_job(void *data);

static Evas_Smart_Class _pan_sc = {NULL};

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   elm_genlist_clear(obj);
   evas_object_del(wd->pan_smart);
   wd->pan_smart = NULL;
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Item_Block *itb;
   elm_smart_scroller_theme_set(wd->scr, "scroller", "base", elm_widget_style_get(obj));
   edje_object_scale_set(wd->scr, elm_widget_scale_get(obj) * _elm_config->scale);
   EINA_INLIST_FOREACH(wd->blocks, itb)
     {
	if (itb->realized) _item_block_unrealize(itb);

	Eina_List *l;
	Elm_Genlist_Item *it;
	EINA_LIST_FOREACH(itb->items, l, it)
	  {
	     it->mincalcd = EINA_FALSE;
	  }

	itb->changed = EINA_TRUE;
     }
   if (wd->calc_job) ecore_job_del(wd->calc_job);
   wd->calc_job = ecore_job_add(_calc_job, wd);
   _sizing_eval(obj);
}

static void
_show_region_hook(void *data, Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Coord x, y, w, h;
   elm_widget_show_region_get(obj, &x, &y, &w, &h);
   elm_smart_scroller_child_region_show(wd->scr, x, y, w, h);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;

   evas_object_size_hint_min_get(wd->scr, &minw, &minh);
   evas_object_size_hint_max_get(wd->scr, &maxw, &maxh);
   minh = -1;
   if (wd->mode != ELM_LIST_LIMIT) minw = -1;
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static Eina_List *
_stringlist_get(const char *str)
{
   Eina_List *list = NULL;
   const char *s, *b;
   if (!str) return NULL;
   for (b = s = str; 1; s++)
     {
	if ((*s == ' ') || (*s == 0))
	  {
	     char *t = malloc(s - b + 1);
	     if (t)
	       {
		  strncpy(t, b, s - b);
		  t[s - b] = 0;
		  list = eina_list_append(list, eina_stringshare_add(t));
		  free(t);
	       }
	     b = s + 1;
	  }
	if (*s == 0) break;
     }
   return list;
}

static void
_stringlist_free(Eina_List *list)
{
   const char *s;
   EINA_LIST_FREE(list, s)
     eina_stringshare_del(s);
}

static void
_item_hilight(Elm_Genlist_Item *it)
{
   const char *selectraise;
   if (it->wd->no_select) return;
   if (it->delete_me) return;
   if (it->hilighted) return;
   edje_object_signal_emit(it->base, "elm,state,selected", "elm");
   selectraise = edje_object_data_get(it->base, "selectraise");
   if ((selectraise) && (!strcmp(selectraise, "on")))
     evas_object_raise(it->base);
   it->hilighted = EINA_TRUE;
}

static void
_item_select(Elm_Genlist_Item *it)
{
   const char *selectraise;
   if (it->wd->no_select) return;
   if (it->delete_me) return;
   if (it->selected)
     {
	if (it->wd->always_select) goto call;
	return;
     }
   it->selected = EINA_TRUE;
   it->wd->selected = eina_list_append(it->wd->selected, it);
   call:
   if (it->func.func) it->func.func((void *)it->func.data, it->wd->obj, it);
   evas_object_smart_callback_call(it->wd->obj, "selected", it);
}

static void
_item_unselect(Elm_Genlist_Item *it)
{
   const char *stacking, *selectraise;
   if (it->delete_me) return;
   if (!it->hilighted) return;
   edje_object_signal_emit(it->base, "elm,state,unselected", "elm");
   stacking = edje_object_data_get(it->base, "stacking");
   selectraise = edje_object_data_get(it->base, "selectraise");
   if ((selectraise) && (!strcmp(selectraise, "on")))
     {
	if ((stacking) && (!strcmp(stacking, "below")))
	  evas_object_lower(it->base);
     }
   it->hilighted = EINA_FALSE;
   if (it->selected)
     {
	it->selected = EINA_FALSE;
	it->wd->selected = eina_list_remove(it->wd->selected, it);
	evas_object_smart_callback_call(it->wd->obj, "unselected", it);
     }
}

static void
_mouse_move(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
   Elm_Genlist_Item *it = data;
   Evas_Event_Mouse_Move *ev = event_info;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     {
	if (!it->wd->on_hold)
	  {
	     it->wd->on_hold = EINA_TRUE;
	     _item_unselect(it);
	  }
     }
}

static void
_mouse_down(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
   Elm_Genlist_Item *it = data;
   Evas_Event_Mouse_Down *ev = event_info;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) it->wd->on_hold = EINA_TRUE;
   else it->wd->on_hold = EINA_FALSE;
   _item_hilight(it);
   if (ev->flags & EVAS_BUTTON_DOUBLE_CLICK)
     evas_object_smart_callback_call(it->wd->obj, "clicked", it);
}

static void
_mouse_up(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
   Elm_Genlist_Item *it = data;
   Evas_Event_Mouse_Up *ev = event_info;
   Eina_List *l;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) it->wd->on_hold = EINA_TRUE;
   else it->wd->on_hold = EINA_FALSE;
   if (it->wd->on_hold)
     {
	it->wd->on_hold = EINA_FALSE;
	return;
     }
   if (it->disabled) return;
   if (it->wd->multi)
     {
	if (!it->selected)
	  {
	     _item_hilight(it);
	     _item_select(it);
	  }
	else _item_unselect(it);
     }
   else
     {
	if (!it->selected)
	  {
	     Widget_Data *wd = it->wd;
	     while (wd->selected)
	       _item_unselect(wd->selected->data);
	     _item_hilight(it);
	     _item_select(it);
	  }
	else
	  {
	     const Eina_List *l, *l_next;
	     Elm_Genlist_Item *it2;
	     EINA_LIST_FOREACH_SAFE(it->wd->selected, l, l_next, it2)
	       if (it2 != it) _item_unselect(it2);
	     _item_hilight(it);
	     _item_select(it);
	  }
     }
}

static void
_signal_expand_toggle(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Elm_Genlist_Item *it = data;
   if (it->expanded)
     evas_object_smart_callback_call(it->wd->obj, "contract,request", it);
   else
     evas_object_smart_callback_call(it->wd->obj, "expand,request", it);
}

static void
_signal_expand(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Elm_Genlist_Item *it = data;
   if (!it->expanded)
     evas_object_smart_callback_call(it->wd->obj, "expand,request", it);
}

static void
_signal_contract(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Elm_Genlist_Item *it = data;
   if (it->expanded)
     evas_object_smart_callback_call(it->wd->obj, "contract,request", it);
}

static void
_item_realize(Elm_Genlist_Item *it, int in, int calc)
{
   Elm_Genlist_Item *it2;
   const char *stacking;
   const char *treesize;
   char buf[1024];
   int depth, tsize = 20;

   if (it->realized) return;
   if (it->delete_me) return;
   it->base = edje_object_add(evas_object_evas_get(it->wd->obj));
   edje_object_scale_set(it->base, elm_widget_scale_get(it->wd->obj) * _elm_config->scale);
   evas_object_smart_member_add(it->base, it->wd->pan_smart);
   elm_widget_sub_object_add(it->wd->obj, it->base);
   if (it->flags & ELM_GENLIST_ITEM_SUBITEMS)
     {
	if (in & 0x1)
	  snprintf(buf, sizeof(buf), "%s/%s", "tree_odd", it->itc->item_style);
	else
	  snprintf(buf, sizeof(buf), "%s/%s", "tree", it->itc->item_style);
     }
   else
     {
	if (in & 0x1)
	  snprintf(buf, sizeof(buf), "%s/%s", "item_odd", it->itc->item_style);
	else
	  snprintf(buf, sizeof(buf), "%s/%s", "item", it->itc->item_style);
     }
   _elm_theme_set(it->base, "genlist", buf, elm_widget_style_get(it->wd->obj));
   it->spacer = evas_object_rectangle_add(evas_object_evas_get(it->wd->obj));
   evas_object_color_set(it->spacer, 0, 0, 0, 0);
   elm_widget_sub_object_add(it->wd->obj, it->spacer);
   for (it2 = it, depth = 0; it2->parent; it2 = it2->parent) depth += 1;
   treesize = edje_object_data_get(it->base, "treesize");
   if (treesize) tsize = atoi(treesize);
   evas_object_size_hint_min_set(it->spacer, (depth * tsize) * _elm_config->scale, 1);
   edje_object_part_swallow(it->base, "elm.swallow.pad", it->spacer);
   if (!calc)
     {
	edje_object_signal_callback_add(it->base, "elm,action,expand,toggle", "elm", _signal_expand_toggle, it);
	edje_object_signal_callback_add(it->base, "elm,action,expand", "elm", _signal_expand, it);
	edje_object_signal_callback_add(it->base, "elm,action,contract", "elm", _signal_contract, it);
	stacking = edje_object_data_get(it->base, "stacking");
	if (stacking)
	  {
	     if (!strcmp(stacking, "below")) evas_object_lower(it->base);
	     else if (!strcmp(stacking, "above")) evas_object_raise(it->base);
	  }
	evas_object_event_callback_add(it->base, EVAS_CALLBACK_MOUSE_DOWN,
				       _mouse_down, it);
	evas_object_event_callback_add(it->base, EVAS_CALLBACK_MOUSE_UP,
				       _mouse_up, it);
	evas_object_event_callback_add(it->base, EVAS_CALLBACK_MOUSE_MOVE,
				       _mouse_move, it);
	if (it->selected)
	  edje_object_signal_emit(it->base, "elm,state,selected", "elm");
	if (it->disabled)
	  edje_object_signal_emit(it->base, "elm,state,disabled", "elm");
	if (it->expanded)
	  edje_object_signal_emit(it->base, "elm,state,expanded", "elm");
     }

   if (it->itc->func.label_get)
     {
	const Eina_List *l;
	const char *key;

	it->labels = _stringlist_get(edje_object_data_get(it->base, "labels"));
	EINA_LIST_FOREACH(it->labels, l, key)
	  {
	     char *s = it->itc->func.label_get(it->data, it->wd->obj, l->data);
	     if (s)
	       {
		  edje_object_part_text_set(it->base, l->data, s);
		  free(s);
	       }
	  }
     }
   if (it->itc->func.icon_get)
     {
	const Eina_List *l;
	const char *key;

	it->icons = _stringlist_get(edje_object_data_get(it->base, "icons"));
	EINA_LIST_FOREACH(it->icons, l, key)
	  {
	     Evas_Object *ic = it->itc->func.icon_get(it->data, it->wd->obj, l->data);
	     if (ic)
	       {
		  it->icon_objs = eina_list_append(it->icon_objs, ic);
		  edje_object_part_swallow(it->base, key, ic);
		  evas_object_show(ic);
		  elm_widget_sub_object_add(it->wd->obj, ic);
	       }
	  }
     }
   if (it->itc->func.state_get)
     {
	const Eina_List *l;
	const char *key;

	it->states = _stringlist_get(edje_object_data_get(it->base, "states"));
	EINA_LIST_FOREACH(it->states, l, key)
	  {
	     Eina_Bool on = it->itc->func.state_get(it->data, it->wd->obj, l->data);
	     if (on)
	       {
		  snprintf(buf, sizeof(buf), "elm,state,%s,active", key);
		  edje_object_signal_emit(it->base, buf, "elm");
	       }
	  }
     }
   if (!it->mincalcd)
     {
	Evas_Coord mw = -1, mh = -1;
	elm_coords_finger_size_adjust(1, &mw, 1, &mh);
	edje_object_size_min_restricted_calc(it->base, &mw, &mh, mw, mh);
	elm_coords_finger_size_adjust(1, &mw, 1, &mh);
	it->w = it->minw = mw;
	it->h = it->minh = mh;
	it->mincalcd = EINA_TRUE;
     }
   if (!calc) evas_object_show(it->base);
   it->realized = EINA_TRUE;
}

static void
_item_unrealize(Elm_Genlist_Item *it)
{
   Evas_Object *icon;

   if (!it->realized) return;
   evas_object_del(it->base);
   it->base = NULL;
   evas_object_del(it->spacer);
   it->spacer = NULL;
   _stringlist_free(it->labels);
   it->labels = NULL;
   _stringlist_free(it->icons);
   it->icons = NULL;
   _stringlist_free(it->states);

   EINA_LIST_FREE(it->icon_objs, icon) evas_object_del(icon);

   it->states = NULL;
   it->realized = EINA_FALSE;
}

static int
_item_block_recalc(Item_Block *itb, int in)
{
   const Eina_List *l;
   Elm_Genlist_Item *it;
   Evas_Coord minw = 0, minh = 0;
   int showme = 0;
   Evas_Coord y = 0;

   EINA_LIST_FOREACH(itb->items, l, it)
     {
	if (it->delete_me) continue;
	showme |= it->showme;
	if (!itb->realized)
	  {
	     _item_realize(it, in, 1);
	     _item_unrealize(it);
	  }
	else
	  _item_realize(it, in, 0);
	minh += it->minh;
	if (minw < it->minw) minw = it->minw;
	in++;
	it->x = 0;
	it->y = y;
	y += it->minw;
     }
   itb->minw = minw;
   itb->minh = minh;
   itb->changed = EINA_FALSE;
   /* force an evas norender to garbage collect deleted objects */
   evas_norender(evas_object_evas_get(itb->wd->obj));
   return showme;
}

static void
_item_block_realize(Item_Block *itb, int in, int full)
{
   const Eina_List *l;
   Elm_Genlist_Item *it;
   if (itb->realized) return;
   EINA_LIST_FOREACH(itb->items, l, it)
     {
	if (it->delete_me) continue;
	if (full) _item_realize(it, in, 0);
	in++;
     }
   itb->realized = EINA_TRUE;
}

static void
_item_block_unrealize(Item_Block *itb)
{
   const Eina_List *l;
   Elm_Genlist_Item *it;

   if (!itb->realized) return;
   EINA_LIST_FOREACH(itb->items, l, it)
     {
	_item_unrealize(it);
     }
   itb->realized = EINA_FALSE;
}

static void
_item_block_position(Item_Block *itb, int in)
{
   const Eina_List *l;
   Elm_Genlist_Item *it;
   Evas_Coord y = 0, ox, oy, ow, oh;
   int vis;

   evas_object_geometry_get(itb->wd->pan_smart, &ox, &oy, &ow, &oh);
   EINA_LIST_FOREACH(itb->items, l, it)
     {
	if (it->delete_me) continue;
	it->x = 0;
	it->y = y;
	it->w = itb->w;
	vis = (ELM_RECTS_INTERSECT(itb->x + it->x - itb->wd->pan_x,
				   itb->y + it->y - itb->wd->pan_y,
				   it->w, it->h,
				   0, 0, ow, oh));
	if ((itb->realized) && (!it->realized))
	  {
	     if (vis) _item_realize(it, in, 0);
	  }
	if (it->realized)
	  {
	     if (vis)
	       {
		  evas_object_resize(it->base, it->w, it->h);
		  evas_object_move(it->base,
				   ox + itb->x + it->x - itb->wd->pan_x,
				   oy + itb->y + it->y - itb->wd->pan_y);
		  evas_object_show(it->base);
	       }
	     else
	       _item_unrealize(it);
	  }
	y += it->h;
	in++;
     }
}

static void
_calc_job(void *data)
{
   Widget_Data *wd = data;
   Item_Block *itb;
   Evas_Coord minw = -1, minh = 0, x = 0, y = 0, ow, oh;
   Item_Block *chb = NULL;
   int in;
   int minw_change = 0;

   in = 0;
   EINA_INLIST_FOREACH(wd->blocks, itb)
     {
	int showme = 0;
	if (chb)
	  {
	     if (itb->realized) _item_block_unrealize(itb);
	  }
	if (itb->changed)
	  {
	     if (itb->realized) _item_block_unrealize(itb);
	     showme = _item_block_recalc(itb, in);
	     chb = itb;
	  }
	itb->y = y;
	itb->x = 0;
	minh += itb->minh;
	if (minw == -1) minw = itb->minw;
	else if (minw < itb->minw)
	  {
	     minw = itb->minw;
	     minw_change = 1;
	  }
	itb->w = minw;
	itb->h = itb->minh;
	y += itb->h;
	in += itb->count;
	if (showme)
	  {
	     wd->show_item->showme = 0;
	     elm_smart_scroller_child_region_show(wd->scr,
						  wd->show_item->x + wd->show_item->block->x,
						  wd->show_item->y + wd->show_item->block->y,
						  wd->show_item->block->w,
						  wd->show_item->h);
	     wd->show_item = NULL;
	     showme = 0;
	  }
     }
   if (minw_change)
     {
	EINA_INLIST_FOREACH(wd->blocks, itb)
	  {
	     itb->minw = minw;
	     itb->w = itb->minw;
	  }
     }
   if ((chb) && (EINA_INLIST_GET(chb)->next))
     {
	EINA_INLIST_FOREACH(EINA_INLIST_GET(chb)->next, itb)
	  if (itb->realized) _item_block_unrealize(itb);
     }
   evas_object_geometry_get(wd->pan_smart, NULL, NULL, &ow, &oh);
   if (minw < ow) minw = ow;
   if ((minw != wd->minw) || (minh != wd->minh))
     {
	wd->minw = minw;
	wd->minh = minh;
	evas_object_size_hint_min_set(wd->pan_smart, wd->minw, wd->minh);
	evas_object_smart_callback_call(wd->pan_smart, "changed", NULL);
	_sizing_eval(wd->obj);
     }
   wd->calc_job = NULL;
   evas_object_smart_changed(wd->pan_smart);
}

static void
_pan_set(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   Pan *sd = evas_object_smart_data_get(obj);
//   Evas_Coord ow, oh;
//   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
//   ow = sd->wd->minw - ow;
//   if (ow < 0) ow = 0;
//   oh = sd->wd->minh - oh;
//   if (oh < 0) oh = 0;
//   if (x < 0) x = 0;
//   if (y < 0) y = 0;
//   if (x > ow) x = ow;
//   if (y > oh) y = oh;
   if ((x == sd->wd->pan_x) && (y == sd->wd->pan_y)) return;
   sd->wd->pan_x = x;
   sd->wd->pan_y = y;
   evas_object_smart_changed(obj);
}

static void
_pan_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y)
{
   Pan *sd = evas_object_smart_data_get(obj);
   if (x) *x = sd->wd->pan_x;
   if (y) *y = sd->wd->pan_y;
}

static void
_pan_max_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y)
{
   Pan *sd = evas_object_smart_data_get(obj);
   Evas_Coord ow, oh;
   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   ow = sd->wd->minw - ow;
   if (ow < 0) ow = 0;
   oh = sd->wd->minh - oh;
   if (oh < 0) oh = 0;
   if (x) *x = ow;
   if (y) *y = oh;
}

static void
_pan_child_size_get(Evas_Object *obj, Evas_Coord *w, Evas_Coord *h)
{
   Pan *sd = evas_object_smart_data_get(obj);
   if (w) *w = sd->wd->minw;
   if (h) *h = sd->wd->minh;
}

static void
_pan_add(Evas_Object *obj)
{
   Pan *sd;
   Evas_Object_Smart_Clipped_Data *cd;

   _pan_sc.add(obj);
   cd = evas_object_smart_data_get(obj);
   sd = calloc(1, sizeof(Pan));
   if (!sd) return;
   sd->__clipped_data = *cd;
   free(cd);
   evas_object_smart_data_set(obj, sd);
}

static void
_pan_del(Evas_Object *obj)
{
   Pan *sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   _pan_sc.del(obj);
}

static void
_pan_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   Pan *sd = evas_object_smart_data_get(obj);
   Evas_Coord ow, oh;
   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   if ((ow == w) && (oh == h)) return;
   if (sd->wd->calc_job) ecore_job_del(sd->wd->calc_job);
   sd->wd->calc_job = ecore_job_add(_calc_job, sd->wd);
}

static void
_pan_calculate(Evas_Object *obj)
{
   Pan *sd = evas_object_smart_data_get(obj);
   Item_Block *itb;
   Evas_Coord ow, oh;
   int in;

   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   in = 0;
   EINA_INLIST_FOREACH(sd->wd->blocks, itb)
     {
	itb->w = sd->wd->minw;
	if (ELM_RECTS_INTERSECT(itb->x - sd->wd->pan_x,
				itb->y - sd->wd->pan_y,
				itb->w, itb->h,
				0, 0, ow, oh))
	  {
	     if ((!itb->realized) || (itb->changed))
	       {
		  _item_block_realize(itb, in, 0);
	       }
	     _item_block_position(itb,  in);
	  }
	else
	  {
	     if (itb->realized)
	       {
		  _item_block_unrealize(itb);
	       }
	  }
	in += itb->count;
     }
}

/**
 * Add a new Genlist object
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Genlist
 */
EAPI Evas_Object *
elm_genlist_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;
   Evas_Coord minw, minh;
   static Evas_Smart *smart = NULL;

   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   elm_widget_type_set(obj, "genlist");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);

   wd->scr = elm_smart_scroller_add(e);
   elm_widget_resize_object_set(obj, wd->scr);

   elm_smart_scroller_bounce_allow_set(wd->scr, 0, 1);

   wd->obj = obj;
   wd->mode = ELM_LIST_SCROLL;

   if (!smart)
     {
	static Evas_Smart_Class sc;
	evas_object_smart_clipped_smart_set(&_pan_sc);
	sc = _pan_sc;
	sc.name = "elm_genlist_pan";
	sc.version = EVAS_SMART_CLASS_VERSION;
	sc.add = _pan_add;
	sc.del = _pan_del;
	sc.resize = _pan_resize;
	sc.calculate = _pan_calculate;
	smart = evas_smart_class_new(&sc);
     }
   if (smart)
     {
	wd->pan_smart = evas_object_smart_add(e, smart);
	wd->pan = evas_object_smart_data_get(wd->pan_smart);
	wd->pan->wd = wd;
     }

   elm_smart_scroller_extern_pan_set(wd->scr, wd->pan_smart,
				     _pan_set, _pan_get,
				     _pan_max_get, _pan_child_size_get);

   edje_object_size_min_calc(elm_smart_scroller_edje_object_get(wd->scr), &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);

   _sizing_eval(obj);
   return obj;
}

static Elm_Genlist_Item *
_item_new(Widget_Data *wd, const Elm_Genlist_Item_Class *itc,
	  const void *data, Elm_Genlist_Item *parent,
	  Elm_Genlist_Item_Flags flags,
	  void (*func) (void *data, Evas_Object *obj, void *event_info), const void *func_data)
{
   Elm_Genlist_Item *it;

   it = calloc(1, sizeof(Elm_Genlist_Item));
   if (!it) return NULL;
   it->wd = wd;
   it->itc = itc;
   it->data = data;
   it->parent = parent;
   it->flags = flags;
   it->func.func = func;
   it->func.data = func_data;
   return it;
}

static void
_item_block_del(Elm_Genlist_Item *it)
{
   Eina_Inlist *il;
   Item_Block *itb = it->block;

   itb->items = eina_list_remove(itb->items, it);
   itb->count--;
   itb->changed = EINA_TRUE;
   if (it->wd->calc_job) ecore_job_del(it->wd->calc_job);
   it->wd->calc_job = ecore_job_add(_calc_job, it->wd);
   if (itb->count < 1)
     {
	il = EINA_INLIST_GET(itb);
	Item_Block *itbn = (Item_Block *)(il->next);
	if (it->parent)
	  it->parent->items = eina_list_remove(it->parent->items, it);
	else
	  it->wd->blocks = eina_inlist_remove(it->wd->blocks, il);
	free(itb);
	if (itbn) itbn->changed = EINA_TRUE;
     }
   else
     {
	if (itb->count < 16)
	  {
	     il = EINA_INLIST_GET(itb);
	     Item_Block *itbp = (Item_Block *)(il->prev);
	     Item_Block *itbn = (Item_Block *)(il->next);
	     if ((itbp) && ((itbp->count + itb->count) < 48))
	       {
		  Elm_Genlist_Item *it2;
		  EINA_LIST_FREE(itb->items, it2)
		    {
		       it2->block = itbp;
		       itbp->items = eina_list_append(itbp->items, it2);
		       itbp->count++;
		       itbp->changed = EINA_TRUE;
		    }
		  it->wd->blocks = eina_inlist_remove(it->wd->blocks, EINA_INLIST_GET(itb));
		  free(itb);
	       }
	     else if ((itbn) && ((itbn->count + itb->count) < 48))
	       {
		  while (itb->items)
		    {
		       Eina_List *last = eina_list_last(itb->items);
		       Elm_Genlist_Item *it2 = last->data;
		       it2->block = itbn;
		       itb->items = eina_list_remove_list(itb->items, last);
		       itbn->items = eina_list_prepend(itbn->items, it2);
		       itbn->count++;
		       itbn->changed = EINA_TRUE;
		    }
		  it->wd->blocks = eina_inlist_remove(it->wd->blocks, EINA_INLIST_GET(itb));
		  free(itb);
	       }
	  }
     }
}

static void
_item_del(Elm_Genlist_Item *it)
{
   elm_genlist_item_subitems_clear(it);
   if (it->wd->show_item == it) it->wd->show_item = NULL;
   if (it->selected) it->wd->selected = eina_list_remove(it->wd->selected, it);
   if (it->realized) _item_unrealize(it);
   if (it->block) _item_block_del(it);
   if ((!it->delete_me) && (it->itc->func.del)) it->itc->func.del(it->data, it->wd->obj);
   it->delete_me = EINA_TRUE;
   if (it->queued)
     it->wd->queue = eina_list_remove(it->wd->queue, it);
   it->wd->items = eina_inlist_remove(it->wd->items, EINA_INLIST_GET(it));
   if (it->parent)
     it->parent->items = eina_list_remove(it->parent->items, it);
   free(it);
}

static void
_item_block_add(Widget_Data *wd, Elm_Genlist_Item *it)
{
   Item_Block *itb = NULL;

   if (!it->rel)
     {
	newblock:
	if (it->rel)
	  {
	     itb = calloc(1, sizeof(Item_Block));
	     if (!itb) return;
	     itb->wd = wd;
	     if (!it->rel->block)
	       {
		  wd->blocks = eina_inlist_append(wd->blocks, EINA_INLIST_GET(itb));
		  itb->items = eina_list_append(itb->items, it);
	       }
	     else
	       {
		  if (it->before)
		    {
		       wd->blocks = eina_inlist_prepend_relative(wd->blocks, EINA_INLIST_GET(itb), EINA_INLIST_GET(it->rel->block));
		       itb->items = eina_list_prepend_relative(itb->items, it, it->rel);
		    }
		  else
		    {
		       wd->blocks = eina_inlist_append_relative(wd->blocks, EINA_INLIST_GET(itb), EINA_INLIST_GET(it->rel->block));
		       itb->items = eina_list_append_relative(itb->items, it, it->rel);
		    }
	       }
	  }
	else
	  {
	     if (it->before)
	       {
		  if (wd->blocks)
		    {
		       itb = (Item_Block *)(wd->blocks);
		       if (itb->count >= 32)
			 {
			    itb = calloc(1, sizeof(Item_Block));
			    if (!itb) return;
			    itb->wd = wd;
			    wd->blocks = eina_inlist_prepend(wd->blocks, EINA_INLIST_GET(itb));
			 }
		    }
		  else
		    {
		       itb = calloc(1, sizeof(Item_Block));
		       if (!itb) return;
		       itb->wd = wd;
		       wd->blocks = eina_inlist_prepend(wd->blocks, EINA_INLIST_GET(itb));
		    }
		  itb->items = eina_list_prepend(itb->items, it);
	       }
	     else
	       {
		  if (wd->blocks)
		    {
		       itb = (Item_Block *)(wd->blocks->last);
		       if (itb->count >= 32)
			 {
			    itb = calloc(1, sizeof(Item_Block));
			    if (!itb) return;
			    itb->wd = wd;
			    wd->blocks = eina_inlist_append(wd->blocks, EINA_INLIST_GET(itb));
			 }
		    }
		  else
		    {
		       itb = calloc(1, sizeof(Item_Block));
		       if (!itb) return;
		       itb->wd = wd;
		       wd->blocks = eina_inlist_append(wd->blocks, EINA_INLIST_GET(itb));
		    }
		  itb->items = eina_list_append(itb->items, it);
	       }
	  }
     }
   else
     {
	itb = it->rel->block;
	if ((!itb) || (itb->count >= 32))
	  {
	     goto newblock;
	  }
	if (it->before)
	  itb->items = eina_list_prepend_relative(itb->items, it, it->rel);
	else
	  itb->items = eina_list_append_relative(itb->items, it, it->rel);
     }
   itb->count++;
   itb->changed = EINA_TRUE;
   it->block = itb;
   if (itb->wd->calc_job) ecore_job_del(itb->wd->calc_job);
   itb->wd->calc_job = ecore_job_add(_calc_job, itb->wd);
   if (it->rel)
     {
	it->rel->relcount--;
	if ((it->rel->delete_me) && (it->rel->relcount == 0))
	  _item_del(it->rel);
	it->rel = NULL;
     }
}

static int
_item_idler(void *data)
{
   Widget_Data *wd = data;
   int n;

   for (n = 0; (wd->queue) && (n < 8); n++)
     {
	Elm_Genlist_Item *it;

	it = wd->queue->data;
	wd->queue = eina_list_remove_list(wd->queue, wd->queue);
	it->queued = EINA_FALSE;
	_item_block_add(wd, it);
     }
   if (n > 0)
     {
	if (wd->calc_job) ecore_job_del(wd->calc_job);
	wd->calc_job = ecore_job_add(_calc_job, wd);
     }
   if (!wd->queue)
     {
	wd->queue_idler = NULL;
	return 0;
     }
   return 1;
}

static void
_item_queue(Widget_Data *wd, Elm_Genlist_Item *it)
{
   if (it->queued) return;
   if (!wd->queue_idler) wd->queue_idler = ecore_idler_add(_item_idler, wd);
   it->queued = EINA_TRUE;
   wd->queue = eina_list_append(wd->queue, it);
}

/**
 * Append item to the end of the genlist
 *
 * This appends the given item to the end of the list or the end of the
 * children if the parent is given.
 *
 * @param obj The genlist object
 * @param itc The item class for the item
 * @param data The item data
 * @param parent The parent item, or NULL if none
 * @param flags Item flags
 * @param func Convenience function called when item selected
 * @param func_data Data passed to @p func above.
 * @return A handle to the item added or NULL if not possible
 *
 * @ingroup Genlist
 */
EAPI Elm_Genlist_Item *
elm_genlist_item_append(Evas_Object *obj, const Elm_Genlist_Item_Class *itc,
			const void *data, Elm_Genlist_Item *parent,
			Elm_Genlist_Item_Flags flags,
			void (*func) (void *data, Evas_Object *obj, void *event_info), const void *func_data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_Genlist_Item *it = _item_new(wd, itc, data, parent, flags, func, func_data);
   if (!it) return NULL;
   if (!it->parent)
     {
	wd->items = eina_inlist_append(wd->items, EINA_INLIST_GET(it));
	it->rel = NULL;
	it->before = 0;
     }
   else
     {
	Elm_Genlist_Item *it2 = NULL;
	Eina_List *ll = eina_list_last(it->parent->items);
	if (ll) it2 = ll->data;
	it->parent->items = eina_list_append(it->parent->items, it);
	if (!it2) it2 = it->parent;
	wd->items = eina_inlist_append_relative(wd->items, EINA_INLIST_GET(it), EINA_INLIST_GET(it2));
	it->rel = it2;
	it->rel->relcount++;
	it->before = 0;
     }
   _item_queue(wd, it);
   return it;
}

/**
 * Prepend item at start of the genlist
 *
 * This adds an item to the beginning of the list or beginning of the children
 * of the parent if given.
 *
 * @param obj The genlist object
 * @param itc The item class for the item
 * @param data The item data
 * @param parent The parent item, or NULL if none
 * @param flags Item flags
 * @param func Convenience function called when item selected
 * @param func_data Data passed to @p func above.
 * @return A handle to the item added or NULL if not possible
 *
 * @ingroup Genlist
 */
EAPI Elm_Genlist_Item *
elm_genlist_item_prepend(Evas_Object *obj, const Elm_Genlist_Item_Class *itc,
			 const void *data, Elm_Genlist_Item *parent,
			 Elm_Genlist_Item_Flags flags,
			 void (*func) (void *data, Evas_Object *obj, void *event_info), const void *func_data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_Genlist_Item *it = _item_new(wd, itc, data, parent, flags, func, func_data);
   if (!it) return NULL;
   if (!it->parent)
     wd->items = eina_inlist_prepend(wd->items, EINA_INLIST_GET(it));
   else
     {
	printf("FIXME: 12 tree not handled yet\n");
     }
   it->rel = NULL;
   it->before = 1;
   _item_queue(wd, it);
   return it;
}

/**
 * Insert item before another in the genlist
 *
 * This inserts an item before another in the list. It will be in the same tree
 * level as the item it is inseted before.
 *
 * @param obj The genlist object
 * @param itc The item class for the item
 * @param data The item data
 * @param before The item to insert before
 * @param flags Item flags
 * @param func Convenience function called when item selected
 * @param func_data Data passed to @p func above.
 * @return A handle to the item added or NULL if not possible
 *
 * @ingroup Genlist
 */
EAPI Elm_Genlist_Item *
elm_genlist_item_insert_before(Evas_Object *obj, const Elm_Genlist_Item_Class *itc,
			       const void *data, Elm_Genlist_Item *before,
			       Elm_Genlist_Item_Flags flags,
			       void (*func) (void *data, Evas_Object *obj, void *event_info), const void *func_data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_Genlist_Item *it = _item_new(wd, itc, data, NULL, flags, func, func_data);
   if (!it) return NULL;
   if (!it->parent)
     wd->items = eina_inlist_prepend_relative(wd->items, EINA_INLIST_GET(it), EINA_INLIST_GET(before));
   else
     {
	printf("FIXME: 13 tree not handled yet\n");
     }
   it->rel = before;
   it->rel->relcount++;
   it->before = 1;
   _item_queue(wd, it);
   return it;
}

/**
 * Insert and item after another in the genlst
 *
 * This inserts an item after another in the list. It will be in the same tree
 * level as the item it is inseted after.
 *
 * @param obj The genlist object
 * @param itc The item class for the item
 * @param data The item data
 * @param after The item to insert after
 * @param flags Item flags
 * @param func Convenience function called when item selected
 * @param func_data Data passed to @p func above.
 * @return A handle to the item added or NULL if not possible
 *
 * @ingroup Genlist
 */
EAPI Elm_Genlist_Item *
elm_genlist_item_insert_after(Evas_Object *obj, const Elm_Genlist_Item_Class *itc,
			      const void *data, Elm_Genlist_Item *after,
			      Elm_Genlist_Item_Flags flags,
			      void (*func) (void *data, Evas_Object *obj, void *event_info), const void *func_data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_Genlist_Item *it = _item_new(wd, itc, data, NULL, flags, func, func_data);
   if (!it) return NULL;
   if (!it->parent)
     wd->items = eina_inlist_append_relative(wd->items, EINA_INLIST_GET(it), EINA_INLIST_GET(after));
   else
     {
	printf("FIXME: 14 tree not handled yet\n");
     }
   it->rel = after;
   it->rel->relcount++;
   it->before = 0;
   _item_queue(wd, it);
   return it;
}

/**
 * Clear the genlist
 *
 * This clears all items in the list, leaving it empty.
 *
 * @param obj The genlist object
 *
 * @ingroup Genlist
 */
EAPI void
elm_genlist_clear(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   while (wd->items)
     {
	Elm_Genlist_Item *it = (Elm_Genlist_Item *)(wd->items);
	wd->items = eina_inlist_remove(wd->items, wd->items);
	if (it->realized) _item_unrealize(it);
	if (it->itc->func.del) it->itc->func.del(it->data, it->wd->obj);
	free(it);
     }
   while (wd->blocks)
     {
	Item_Block *itb = (Item_Block *)(wd->blocks);
	wd->blocks = eina_inlist_remove(wd->blocks, wd->blocks);
	if (itb->items) eina_list_free(itb->items);
	free(itb);
     }
   if (wd->calc_job)
     {
	ecore_job_del(wd->calc_job);
	wd->calc_job = NULL;
     }
   if (wd->queue_idler)
     {
	ecore_idler_del(wd->queue_idler);
	wd->queue_idler = NULL;
     }
   if (wd->queue)
     {
	eina_list_free(wd->queue);
	wd->queue = NULL;
     }
   if (wd->selected)
     {
	eina_list_free(wd->selected);
	wd->selected = NULL;
     }
   wd->show_item = NULL;
   wd->pan_x = 0;
   wd->pan_y = 0;
   wd->minw = 0;
   wd->minh = 0;
   evas_object_size_hint_min_set(wd->pan_smart, wd->minw, wd->minh);
   evas_object_smart_callback_call(wd->pan_smart, "changed", NULL);
   _sizing_eval(obj);
}

/**
 * Enable or disable multi-select in the genlist
 *
 * This enables (1) or disableds (0) multi-select in the list. This allows
 * more than 1 item to be selected.
 *
 * @param obj The genlist object
 * @param multi Multi-select enable/disable
 *
 * @ingroup Genlist
 */
EAPI void
elm_genlist_multi_select_set(Evas_Object *obj, Eina_Bool multi)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   wd->multi = multi;
}

/**
 * Get the selectd item in the genlist
 *
 * This gets the selected item in the list (if multi-select is enabled only
 * the first item in the list is selected - which is not very useful, so see
 * elm_genlist_selected_items_get()for when multi-select is used).
 *
 * If no item is selected, NULL is returned.
 *
 * @param obj The genlist object
 * @return The selected item, or NULL if none.
 *
 * @ingroup Genlist
 */
EAPI Elm_Genlist_Item *
elm_genlist_selected_item_get(const Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->selected) return wd->selected->data;
   return NULL;
}

/**
 * Get a list of selected items in the genlist
 *
 * This retgurns a list of the selected items. This list pointer is only valid
 * so long as no items are selected or unselected (or unselected implicitly
 * by deletion). The list contains Elm_Genlist_Item pointers.
 *
 * @param obj The genlist object
 * @return The list of selected items, nor NUL if none are selected.
 *
 * @ingroup Genlist
 */
EAPI const Eina_List *
elm_genlist_selected_items_get(const Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   return wd->selected;
}

/**
 * Get the first item in the genlist
 *
 * This returns the first item in the list.
 *
 * @param obj The genlist object
 * @return The first item, or NULL if none
 *
 * @ingroup Genlist
 */
EAPI Elm_Genlist_Item *
elm_genlist_first_item_get(const Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_Genlist_Item *it = (Elm_Genlist_Item *)(wd->items);
   while ((it) && (it->delete_me))
     it = (Elm_Genlist_Item *)(EINA_INLIST_GET(it)->next);
   return it;
}

/**
 * Get the last item in the genlist
 *
 * This returns the last item in the list.
 *
 * @return The last item, or NULL if none
 *
 * @ingroup Genlist
 */
EAPI Elm_Genlist_Item *
elm_genlist_last_item_get(const Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd->items) return NULL;
   Elm_Genlist_Item *it = (Elm_Genlist_Item *)(wd->items->last);
   while ((it) && (it->delete_me))
     it = (Elm_Genlist_Item *)(EINA_INLIST_GET(it)->prev);
   return it;
}

/**
 * Get the next item in the genlist
 *
 * This returns the item after the item @p it.
 *
 * @param it The item
 * @return The item after @p it, or NULL if none
 *
 * @ingroup Genlist
 */
EAPI Elm_Genlist_Item *
elm_genlist_item_next_get(const Elm_Genlist_Item *it)
{
   while (it)
     {
	it = (Elm_Genlist_Item *)(EINA_INLIST_GET(it)->next);
	if ((it) && (!it->delete_me)) break;
     }
   return (Elm_Genlist_Item *)it;
}

/**
 * Get the previous item in the genlist
 *
 * This returns the item before the item @p it.
 *
 * @param it The item
 * @return The item before @p it, or NULL if none
 *
 * @ingroup Genlist
 */
EAPI Elm_Genlist_Item *
elm_genlist_item_prev_get(const Elm_Genlist_Item *it)
{
   while (it)
     {
	it = (Elm_Genlist_Item *)(EINA_INLIST_GET(it)->prev);
	if ((it) && (!it->delete_me)) break;
     }
   return (Elm_Genlist_Item *)it;
}

/**
 * Get the genlist object from an item
 *
 * This returns the genlist object itself that an item belongs to.
 *
 * @param it The item
 * @return The genlist object
 *
 * @ingroup Genlist
 */
EAPI Evas_Object *
elm_genlist_item_genlist_get(const Elm_Genlist_Item *it)
{
   if (!it) return NULL;
   return it->wd->obj;
}

/**
 * Get the parent item of the given item
 *
 * This returns the prent item of the item @p it given.
 *
 * @param it The item
 * @return The parent of the item or NULL if none
 *
 * @ingroup Genlist
 */
EAPI Elm_Genlist_Item *
elm_genlist_item_parent_get(const Elm_Genlist_Item *it)
{
   if (!it) return NULL;
   return it->parent;
}

/**
 * Clear all sub-items (children) of the given item
 *
 * This clears all items that are children (or their descendants) of the
 * given item @p it.
 *
 * @param it The item
 *
 * @ingroup Genlist
 */
EAPI void
elm_genlist_item_subitems_clear(Elm_Genlist_Item *it)
{
   Eina_List *tl = NULL, *l;
   Elm_Genlist_Item *it2;

   if (!it) return;
   EINA_LIST_FOREACH(it->items, l, it2)
     tl = eina_list_append(tl, it2);
   EINA_LIST_FREE(tl, it2)
     elm_genlist_item_del(it2);
}

/**
 * Set the selected state of an item
 *
 * This sets the selected state (1 selected, 0 not selected) of the given
 * item @p it.
 *
 * @param it The item
 * @param selcted The slected state
 *
 * @ingroup Genlist
 */
EAPI void
elm_genlist_item_selected_set(Elm_Genlist_Item *it, Eina_Bool selected)
{
   Widget_Data *wd = elm_widget_data_get(it->wd->obj);

   if (!it) return;
   if (it->delete_me) return;
   selected = !!selected;
   if (it->selected == selected) return;

   if (selected)
     {
	if (!wd->multi)
	  {
	     while (wd->selected)
	       _item_unselect(wd->selected->data);
	  }
	_item_hilight(it);
	_item_select(it);
     }
   else
     _item_unselect(it);
}

/**
 * Get the selected state of an item
 *
 * This gets the selected state of an item (1 selected, 0 not selected).
 *
 * @param it The item
 * @return The selected state
 *
 * @ingroup Genlist
 */
EAPI Eina_Bool
elm_genlist_item_selected_get(const Elm_Genlist_Item *it)
{
   if (!it) return EINA_FALSE;
   return it->selected;
}

/**
 * Sets the expanded state of an item (if it's a parent)
 *
 * This expands or contracts a parent iterm (thus showing or hiding the
 * children).
 *
 * @param it The item
 * @param expanded The expanded state (1 expanded, 0 not expanded).
 *
 * @ingroup Genlist
 */
EAPI void
elm_genlist_item_expanded_set(Elm_Genlist_Item *it, Eina_Bool expanded)
{
   if (!it) return;
   if (it->expanded == expanded) return;
   it->expanded = expanded;
   if (it->expanded)
     {
	if (it->realized)
	  edje_object_signal_emit(it->base, "elm,state,expanded", "elm");
	evas_object_smart_callback_call(it->wd->obj, "expanded", it);
     }
   else
     {
	if (it->realized)
	  edje_object_signal_emit(it->base, "elm,state,contracted", "elm");
	evas_object_smart_callback_call(it->wd->obj, "contracted", it);
     }
}

/**
 * Get the expanded state of an item
 *
 * This gets the expanded state of an item
 *
 * @param it The item
 * @return Thre expanded state
 *
 * @ingroup Genlist
 */
EAPI Eina_Bool
elm_genlist_item_expanded_get(const Elm_Genlist_Item *it)
{
   if (!it) return EINA_FALSE;
   return it->expanded;
}

/**
 * Sets the disabled state of an item.
 *
 * A disabled item cannot be selected or unselected. It will also change
 * appearance to appear disabled. This sets the disabled state (1 disabled, 0
 * not disabled).
 *
 * @param it The item
 * @param disabled The disabled state
 *
 * @ingroup Genlist
 */
EAPI void
elm_genlist_item_disabled_set(Elm_Genlist_Item *it, Eina_Bool disabled)
{
   if (!it) return;
   if (it->disabled == disabled) return;
   if (it->delete_me) return;
   it->disabled = disabled;
   if (it->realized)
     {
	if (it->disabled)
	  edje_object_signal_emit(it->base, "elm,state,disabled", "elm");
	else
	  edje_object_signal_emit(it->base, "elm,state,enabled", "elm");
     }
}

/**
 * Get the disabled state of an item
 *
 * This gets the disabld state of the given item.
 *
 * @param it The item
 * @return The disabled state
 *
 * @ingroup Genlist
 */
EAPI Eina_Bool
elm_genlist_item_disabled_get(const Elm_Genlist_Item *it)
{
   if (!it) return EINA_FALSE;
   if (it->delete_me) return EINA_FALSE;
   return it->disabled;
}

/**
 * Show the given item
 *
 * This causes genlist to jump to the given item @p it and show it (by scrolling),
 * if it is not fully visible.
 *
 * @param it The item
 *
 * @ingroup Genlist
 */
EAPI void
elm_genlist_item_show(Elm_Genlist_Item *it)
{
   if (!it) return;
   if (it->delete_me) return;
   if ((it->queued) || (!it->mincalcd))
     {
	it->wd->show_item = it;
	it->showme = EINA_TRUE;
	return;
     }
   if (it->wd->show_item)
     {
	it->wd->show_item->showme = EINA_FALSE;
	it->wd->show_item = NULL;
     }
   elm_smart_scroller_child_region_show(it->wd->scr,
					it->x + it->block->x,
					it->y + it->block->y,
					it->block->w, it->h);
}

/**
 *  XXX
 *
 * @param xxx XXX
 * @return XXX
 *
 * @ingroup Genlist
 */
EAPI void
elm_genlist_item_del(Elm_Genlist_Item *it)
{
   if (!it) return;
   if (it->relcount > 0)
     {
	elm_genlist_item_subitems_clear(it);
	it->delete_me = EINA_TRUE;
	if (it->wd->show_item == it) it->wd->show_item = NULL;
	if (it->selected) it->wd->selected = eina_list_remove(it->wd->selected, it);
	if (it->block)
	  {
	     if (it->realized) _item_unrealize(it);
	     it->block->changed = EINA_TRUE;
	     if (it->wd->calc_job) ecore_job_del(it->wd->calc_job);
	     it->wd->calc_job = ecore_job_add(_calc_job, it->wd);
	  }
	if (it->itc->func.del) it->itc->func.del(it->data, it->wd->obj);
	return;
     }
   _item_del(it);
}

/**
 * Get the data item from the genlist item
 *
 * This returns the data value passed on the elm_genlist_item_append() and
 * related item addition calls.
 *
 * @param it The item
 * @return The data pointer provided when created
 *
 * @ingroup Genlist
 */
EAPI void *
elm_genlist_item_data_get(const Elm_Genlist_Item *it)
{
   if (!it) return;
   return it->data;
}

EAPI void
elm_genlist_item_data_set(Elm_Genlist_Item *it, void *data)
{
   if (!it) return;
   it->data = data;
   elm_genlist_item_update(it);
}

/**
 * Update the contents of an item
 *
 * This updates an item by calling all the item class functions again to get
 * the icons, labels and states. Use this when he original item data has
 * changed and the changes are desired to be reflected.
 *
 * @param it The item
 *
 * @ingroup Genlist
 */
EAPI void
elm_genlist_item_update(Elm_Genlist_Item *it)
{
   Evas_Coord minw, minh;
   Eina_List *l;
   Elm_Genlist_Item *it2;
   Item_Block *itb;
   int num, numb;
   if (!it->block) return;
   if (it->delete_me) return;
   minw = it->wd->minw;
   minh = it->minh;
   it->mincalcd = EINA_FALSE;
   EINA_INLIST_FOREACH(it->wd->blocks, itb)
     {
	if (itb == it->block) break;
	num += itb->count;
     }
   numb = num;
   EINA_LIST_FOREACH(it->block->items, l, it2)
     {
	if (it2 == it) break;
	num++;
     }
   if (it->realized)
     {
	_item_unrealize(it);
	_item_realize(it, num, 0);
	_item_block_recalc(it->block, numb);
	_item_block_position(it->block, num);
     }
   else
     {
	_item_realize(it, num, 1);
	_item_unrealize(it);
     }
   if ((it->minw > minw) || (it->minh != minh))
     {
	it->block->changed = EINA_TRUE;
	if (it->wd->calc_job) ecore_job_del(it->wd->calc_job);
	it->wd->calc_job = ecore_job_add(_calc_job, it->wd);
     }
}

/**
 * This sets the horizontal stretching mode
 *
 * This sets the mode used for sizing items horizontally. Valid modes are
 * ELM_LIST_LIMIT and ELM_LIST_SCROLL. The default is ELM_LIST_SCROLL. This
 * mode means that if items are too wide to fit, the scroller will scroll
 * horizontally. Otherwise items are expanded to fill the width of the
 * viewport of the scroller. If it is ELM_LIST_LIMIT, Items will be expanded
 * to the viewport width and limited to that size.
 *
 * @param obj The genlist object
 * @param mode The mode to use
 *
 * @ingroup Genlist
 */
EAPI void
elm_genlist_horizontal_mode_set(Evas_Object *obj, Elm_List_Mode mode)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->mode == mode) return;
   wd->mode = mode;
   if (wd->mode == ELM_LIST_LIMIT)
     elm_scroller_content_min_limit(wd->scr, 1, 0);
   else
     elm_scroller_content_min_limit(wd->scr, 0, 0);
}

/**
 * Set the always select mode.
 *
 * Items will only call their selection func and callback when first becoming
 * selected. Any further clicks will do nothing, unless you enable always
 * select with elm_genlist_always_select_mode_set(). This means even if
 * selected, every click will make the selected callbacks be called.
 *
 * @param obj The genlist object
 * @param always_select The always select mode (1 on, 2 off)
 *
 * @ingroup Genlist
 */
EAPI void
elm_genlist_always_select_mode_set(Evas_Object *obj, Eina_Bool always_select)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   wd->always_select = always_select;
}

/**
 * Set no select mode
 *
 * This will turn off the ability to select items entirely and they will
 * neither appear selected nor call selected callback functions.
 *
 * @param obj The genlist object
 * @param no_select The no select mode (1 on, 2 off)
 *
 * @ingroup Genlist
 */
EAPI void
elm_genlist_no_select_mode_set(Evas_Object *obj, Eina_Bool no_select)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   wd->no_select = no_select;
}
