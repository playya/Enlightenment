#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Hoverlist
 *
 * A hoverlist is a widget that, when show, pops up a list of items.
 * It automatically chooses an area inside its parent object's view
 * (set via elm_hoverlist_add() and elm_hoverlist_hover_parent_set())
 * to optimally fit into it. In the default theme, it will also point
 * an arrow to the cursor position at the time one shows it. Hoverlist
 * items have a label and/or an icon. It is intended for a small
 * number of items (hence the use of list, not genlist).
 *
 * Signals that you can add callbacks for are:
 *
 * dismissed - the hover was dismissed
 */

typedef struct _Widget_Data Widget_Data;

struct _Elm_Hoverlist_Item
{
   Elm_Widget_Item base;

   Elm_List_Item *lptr;

   const char *label;
   Evas_Object *icon;

   Evas_Smart_Cb func;
   const void *data;

   Eina_Bool disabled : 1;
};

struct _Widget_Data
{
   Evas_Object *hover_parent;
   Evas_Object *list;
   Evas_Object *target;
   Evas_Object *hover;
   Evas *evas;

   Eina_List *items;

   Eina_Bool scroller_disabled : 1;
   Eina_Bool horizontal : 1;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _del_pre_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _hover_clicked_cb(void *data, Evas_Object *obj, void *event_info);
static void _parent_resize_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info);
static void _hoverlist_show(void *data, Evas *evas, Evas_Object *obj, void *event_info);
static void _hoverlist_hide(void *data, Evas *evas, Evas_Object *obj, void *event_info);

static const char SIG_DISMISSED[] = "dismissed";
static const Evas_Smart_Cb_Description _signals[] = {
  {SIG_DISMISSED, ""},
  {NULL, NULL}
};

#define ELM_HOVERLIST_ITEM_CHECK_RETURN(it, ...)                        \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, __VA_ARGS__);	\
  ELM_CHECK_WIDTYPE(item->base.widget, widtype) __VA_ARGS__;

static Elm_Hoverlist_Item *
_item_new(Evas_Object *obj, const char *label, Evas_Object *icon, Evas_Smart_Cb func, const void *data)
{
   Elm_Hoverlist_Item *it;

   it = elm_widget_item_new(obj, Elm_Hoverlist_Item);
   if (!it)
     return NULL;

   it->label = eina_stringshare_add(label);
   it->icon = icon;
   it->func = func;
   it->base.data = data;

   return it;
}

static inline void
_item_free(Elm_Hoverlist_Item *it)
{
   eina_stringshare_del(it->label);
   elm_widget_item_del(it);
}

static void
_del_pre_hook(Evas_Object* obj)
{
   Widget_Data * wd = elm_widget_data_get(obj);
   if (!wd)
     return;
   evas_object_event_callback_del_full(wd->hover_parent, EVAS_CALLBACK_RESIZE,
                                       _parent_resize_cb, obj);
}

static void
_del_hook(Evas_Object* obj)
{
   Elm_Hoverlist_Item *it;
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd)
     return;

   EINA_LIST_FREE(wd->items, it)
     _item_free(it);

   free(wd);
}

static Eina_Bool
_event_hook(Evas_Object *obj, Evas_Object *src __UNUSED__, Evas_Callback_Type type, void *event_info)
{
   if (type != EVAS_CALLBACK_KEY_DOWN)
     return EINA_FALSE;

   Evas_Event_Key_Down *ev = event_info;

   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd)
     return EINA_FALSE;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     return EINA_FALSE;

   if (strcmp(ev->keyname, "Escape"))
     return EINA_FALSE;

   evas_object_hide(obj);
   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   return EINA_TRUE;
}

static void
_on_focus_hook(void *data __UNUSED__, Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd)
     return;

   if (elm_widget_focus_get(obj))
     {
	edje_object_signal_emit(wd->list, "elm,action,focus", "elm");
	evas_object_focus_set(wd->list, EINA_TRUE);
     }
   else
     {
	edje_object_signal_emit(wd->list, "elm,action,unfocus", "elm");
	evas_object_focus_set(wd->list, EINA_FALSE);
     }
}

static void
_theme_hook(Evas_Object* obj)
{
   Widget_Data* wd;
   char buf[1024];

   wd = elm_widget_data_get(obj);
   if (!wd)
     return;

   elm_widget_style_set(wd->list, "hoverlist");

   snprintf(buf, sizeof(buf), "hoverlist/%s", elm_widget_style_get(obj));
   elm_object_style_set(wd->hover, buf);
}

static void
_item_func_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   Elm_Hoverlist_Item *it;

   elm_list_item_selected_set(event_info, EINA_FALSE);

   it = data;
   if (it->func)
     it->func((void *)it->base.data, it->base.widget, it);
}

static void
_hover_clicked_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_hide(data);
}

void
_content_placement_changed_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   const char *new_slot;
   Widget_Data *wd;
   char buf[1024];

   wd = elm_widget_data_get(data);
   new_slot = event_info;

   snprintf(buf, sizeof(buf), "elm,action,slot,%s,show", new_slot);
   elm_widget_signal_emit(wd->hover, buf, "elm");
}

static void
_hoverlist_show(void *data __UNUSED__, Evas *evas __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data* wd;
   int px, py;
   int w, h;

   wd = elm_widget_data_get(obj);
   if (eina_list_count(wd->items) < 1)
     return;

   elm_widget_focus_steal(obj);

   evas_pointer_canvas_xy_get(wd->evas, &px, &py);
   evas_object_geometry_get(wd->target, NULL, NULL, &w, &h);
   evas_object_move(wd->target, px - (w / 2), py - (h / 2));

   /* reset list */
   elm_list_item_show(eina_list_data_get(elm_list_items_get(wd->list)));

   evas_object_show(wd->hover);
}

static void
_hoverlist_hide(void *data __UNUSED__, Evas *evas __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd)
     return;

   evas_object_hide(wd->hover);
   evas_object_smart_callback_call(obj, SIG_DISMISSED, NULL);
}

static void
_parent_del(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd)
     return;

   wd->hover_parent = NULL;
}

static void
_parent_resize_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd;
   int w, h;

   wd = elm_widget_data_get(data);
   if (!wd)
     return;

   evas_object_geometry_get(wd->hover_parent, NULL, NULL, &w, &h);
   evas_object_size_hint_max_set(wd->list, w * 0.666667, h/2);
}

/**
 * Add a new Hoverlist object to the parent.
 *
 * @param parent Parent object
 * @return New object or @c NULL, if it cannot be created
 *
 * @ingroup Hoverlist
 */
EAPI Evas_Object*
elm_hoverlist_add(Evas_Object* parent)
{
   Evas_Object* obj;
   Widget_Data* wd;
   char buf[1024];

   wd = ELM_NEW(Widget_Data);
   wd->evas = evas_object_evas_get(parent);
   obj = elm_widget_add(wd->evas);
   ELM_SET_WIDTYPE(widtype, "hoverlist");
   elm_widget_type_set(obj, "hoverlist");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_on_focus_hook_set(obj, _on_focus_hook, NULL);
   elm_widget_can_focus_set(obj, EINA_TRUE);
   elm_widget_del_pre_hook_set(obj, _del_pre_hook);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_event_hook_set(obj, _event_hook);

   wd->list = elm_list_add(obj);
   elm_widget_style_set(wd->list, "hoverlist");

   elm_list_mode_set(wd->list, ELM_LIST_EXPAND);
   evas_object_show(wd->list);

   wd->target = elm_icon_add(obj); /* has to be an elm_widget (dummy,
                                      in this case) because we gotta
                                      make it work with
                                      elm_widget_hover_object_set() */
   evas_object_resize(wd->target, elm_finger_size_get(),
                      elm_finger_size_get());
   evas_object_show(wd->target);
   evas_object_layer_set(wd->target, EVAS_LAYER_MIN);

   wd->hover = elm_hover_add(obj);

   snprintf(buf, sizeof(buf), "hoverlist/%s", elm_widget_style_get(obj));
   elm_object_style_set(wd->hover, buf);

   evas_object_smart_callback_add(wd->hover, "smart,changed",
                                  _content_placement_changed_cb, obj);

   elm_hoverlist_hover_parent_set(obj, parent);
   elm_hover_target_set(wd->hover, wd->target);

   evas_object_smart_callback_add(wd->hover, "clicked", _hover_clicked_cb, obj);
   elm_hover_content_set(wd->hover, "smart", wd->list);

   evas_object_event_callback_add(obj, EVAS_CALLBACK_SHOW, _hoverlist_show,
                                  NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_HIDE, _hoverlist_hide,
                                  NULL);

   elm_widget_sub_object_add(obj, wd->list);
   elm_widget_sub_object_add(obj, wd->hover);
   elm_widget_sub_object_add(obj, wd->target);
   evas_object_smart_callbacks_descriptions_set(obj, _signals);
   return obj;
}

/**
 * This ends the Hoverlist's popup as if the user had clicked
 * outside the hover.
 *
 * @param obj The hoverlist object
 *
 * @ingroup Hoverlist
 */
EAPI void
elm_hoverlist_hover_end(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);

   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd)
     return;

   if (!wd->hover)
     return;

   evas_object_hide(wd->hover);
}

/**
 * Get the icon object for the given hoverlist item.
 *
 * @param item Hoverlist item
 * @return icon object or @c NULL, if the item does not have icon or
 *         an error occured
 *
 * @ingroup Hoverlist
 */
EAPI Evas_Object *
elm_hoverlist_item_icon_get(const Elm_Hoverlist_Item *item)
{
   ELM_HOVERLIST_ITEM_CHECK_RETURN(item, NULL);
   return item->icon;
}

/**
 * Sets the side icon associated with the hoverlist item.
 *
 * Once the icon object is set, a previously set one will be deleted.
 * You probably don't want, then, to have the <b>same</b> icon object
 * set for more than one item of the list (when replacing one of its
 * instances).
 *
 * @param item Hoverlist item
 * @param icon Icon object to be set
 *
 * @ingroup Hoverlist
 */
EAPI void
elm_hoverlist_item_icon_set(Elm_Hoverlist_Item *item, Evas_Object *icon)
{
   ELM_HOVERLIST_ITEM_CHECK_RETURN(item);

   Widget_Data *wd;

   wd = elm_widget_data_get(item->base.widget);
   if (!wd)
     return;

   if (item->icon)
     {
        evas_object_del(item->icon);
        item->icon = NULL;
     }

   item->icon = icon;
   elm_list_item_icon_set(item->lptr, icon);
   elm_list_go(wd->list);
}

/**
 * Get the label object for the given hoverlist item.
 *
 * @param item Hoverlist item
 * @return label object or @c NULL, if the item does not have label or
 *         an error occured
 *
 * @ingroup Hoverlist
 */
EAPI const char *
elm_hoverlist_item_label_get(const Elm_Hoverlist_Item *item)
{
   ELM_HOVERLIST_ITEM_CHECK_RETURN(item, NULL);
   return item->label;
}

/**
 * (Re)set the label on the given hoverlist item.
 *
 * @param obj Hoverlist item
 * @param label String to set as label
 *
 * @ingroup Hoverlist
 */
EAPI void
elm_hoverlist_item_label_set(Elm_Hoverlist_Item *item, const char *label)
{
   ELM_HOVERLIST_ITEM_CHECK_RETURN(item);

   Widget_Data *wd;

   wd = elm_widget_data_get(item->base.widget);
   if (!wd)
     return;

   if (!eina_stringshare_replace(&item->label, label))
     return;

   elm_list_item_label_set(item->lptr, label);
   elm_list_go(wd->list);
}

/**
 * Set the Hoverlist's parent.
 *
 * Sets the hover's parent object (it would much probably be the
 * window that the hoverlist is in). See Hover objects for more
 * information.
 *
 * @param obj The hoverlist object
 * @param parent The parent to use
 *
 * @note elm_hoverlist_add() will automatically call this function
 * with its @c parent argument.
 *
 * @ingroup Hoverlist
 */
EAPI void
elm_hoverlist_hover_parent_set(Evas_Object *obj, Evas_Object *parent)
{
   ELM_CHECK_WIDTYPE(obj, widtype);

   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if ((!wd) || (!parent))
     return;

   elm_hover_parent_set(wd->hover, parent);

   if (wd->hover_parent)
     {
        evas_object_event_callback_del_full(wd->hover_parent, EVAS_CALLBACK_DEL,
                                            _parent_del, obj);
        evas_object_event_callback_del_full(wd->hover_parent,
                                            EVAS_CALLBACK_RESIZE,
                                            _parent_resize_cb, obj);
     }

   wd->hover_parent = parent;
   evas_object_event_callback_add(wd->hover_parent, EVAS_CALLBACK_DEL,
                                  _parent_del, obj);
   evas_object_event_callback_add(wd->hover_parent, EVAS_CALLBACK_RESIZE,
                                  _parent_resize_cb, obj);

   _parent_resize_cb(obj, NULL, NULL, NULL);
}

/**
 * Get the Hoverlist's parent object.
 *
 * @param obj The hoverlist object
 * @param parent The parent to use
 *
 * See elm_hoverlist_hover_parent_set() for more information.
 *
 * @ingroup Hoverlist
 */
EAPI Evas_Object *
elm_hoverlist_hover_parent_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;

   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd)
     return NULL;

   return wd->hover_parent;
}

/**
 * Clear all items in the given hoverlist object.
 *
 * @param obj Hoverlist object
 *
 * @ingroup Hoverlist
 */
EAPI void
elm_hoverlist_clear(Evas_Object* obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);

   Elm_Hoverlist_Item *item;
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if ((!wd) || (!wd->items))
     return;

   EINA_LIST_FREE(wd->items, item)
     {
        elm_list_item_del(item->lptr);
        _item_free(item);
     }

   evas_object_hide(wd->hover);
}

/**
 * Change the hoverlist's orientation to horizontal or vertical.
 *
 * @param obj Hoverlist object
 * @param horizontal @c EINA_TRUE for horizontal mode, @c EINA_FALSE
 *        for vertical
 *
 * @ingroup Hoverlist
 */
EAPI void
elm_hoverlist_horizontal_set(Evas_Object* obj, Eina_Bool horizontal)
{
   ELM_CHECK_WIDTYPE(obj, widtype);

   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd)
     return;

   if (wd->horizontal == horizontal)
     return;

   wd->horizontal = horizontal;
   elm_list_horizontal_set(wd->list, horizontal);
}

/**
 * Get the value of current hoverlist object's orientation.
 *
 * @param obj Hoverlist object
 * @return @c EINA_TRUE for horizontal mode, @c EINA_FALSE for
 *            vertical mode (or errors)
 *
 * @ingroup Hoverlist
 */
EAPI Eina_Bool
elm_hoverlist_horizontal_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;

   Widget_Data *wd;

   wd = elm_widget_data_get( obj );
   if (!wd)
     return EINA_FALSE;

   return wd->horizontal;
}

/**
 * Append a new item to a hoverlist object.
 *
 * @param obj Hoverlist object
 * @param label The label of the new item
 * @param icon Icon to be set on new item
 * @param func Convenience function called when item selected
 * @param data Data passed to @p func above
 * @return A handle to the item added or @c NULL, on errors
 *
 * @ingroup Hoverlist
 */
EAPI Elm_Hoverlist_Item *
elm_hoverlist_item_append(Evas_Object *obj, const char *label, Evas_Object *icon, Evas_Smart_Cb func, const void *data)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;

   Elm_Hoverlist_Item* item;
   Widget_Data* wd;

   wd = elm_widget_data_get(obj);
   if (!wd)
     return NULL;

   item = _item_new(obj, label, icon, func, data);
   if (!item)
     return NULL;

   wd->items = eina_list_append(wd->items, item);
   item->lptr = elm_list_item_append(wd->list, label, icon, NULL,
                                     _item_func_cb, item);
   elm_list_go(wd->list);
   return item;
}

/**
 * Delete the given item in a hoverlist object.
 *
 * @param item Hoverlist item to be deleted
 *
 * @ingroup Hoverlist
 */
EAPI void
elm_hoverlist_item_del(Elm_Hoverlist_Item *item)
{
   ELM_HOVERLIST_ITEM_CHECK_RETURN(item);

   Widget_Data *wd;

   wd = elm_widget_data_get(item->base.widget);
   if (!wd)
     return;

   wd->items = eina_list_remove(wd->items, item);

   elm_list_item_del(item->lptr);
   item->lptr = NULL;

   if (eina_list_count(wd->items) < 1)
     evas_object_hide(wd->hover);

   _item_free(item);
}

/**
 * Set the hoverlist item's state as disabled or enabled
 *
 * @param item Hoverlist item to be enabled/disabled
 * @param disabled @c EINA_TRUE to disable it, @c EINA_FALSE to enable
 *
 * @ingroup Hoverlist
 */
EAPI void
elm_hoverlist_item_disabled_set(Elm_Hoverlist_Item *item, Eina_Bool disabled)
{
   ELM_HOVERLIST_ITEM_CHECK_RETURN(item);

   Widget_Data *wd;

   wd = elm_widget_data_get(item->base.widget);
   if (!wd)
     return;

   if (disabled == item->disabled)
     return;

   item->disabled = disabled;
   elm_list_item_disabled_set(item->lptr, disabled);
}

/**
 * Get the hoverlist item's disabled/enabled state.
 *
 * @param item Hoverlist item to be enabled/disabled
 * @return @c EINA_TRUE, if disabled, @c EINA_FALSE otherwise
 *
 * @ingroup Hoverlist
 */
EAPI Eina_Bool
elm_hoverlist_item_disabled_get(const Elm_Hoverlist_Item *item)
{
   ELM_HOVERLIST_ITEM_CHECK_RETURN(item, EINA_FALSE);

   return item->disabled;
}
