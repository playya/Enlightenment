/** @file etk_scrolled_view.c */
#include "etk_scrolled_view.h"
#include <stdlib.h>
#include <string.h>
#include "etk_viewport.h"
#include "etk_event.h"
#include "etk_signal.h"
#include "etk_signal_callback.h"
#include "etk_utils.h"

/**
 * @addtogroup Etk_Scrolled_View
 * @{
 */

enum Etk_Scrolled_View_Property_Id
{
   ETK_SCROLLED_VIEW_HPOLICY_PROPERTY,
   ETK_SCROLLED_VIEW_VPOLICY_PROPERTY
};

static void _etk_scrolled_view_constructor(Etk_Scrolled_View *scrolled_view);
static void _etk_scrolled_view_property_set(Etk_Object *object, int property_id, Etk_Property_Value *value);
static void _etk_scrolled_view_property_get(Etk_Object *object, int property_id, Etk_Property_Value *value);
static void _etk_scrolled_view_size_request(Etk_Widget *widget, Etk_Size *size);
static void _etk_scrolled_view_size_allocate(Etk_Widget *widget, Etk_Geometry geometry);
static void _etk_scrolled_view_key_down_cb(Etk_Object *object, Etk_Event_Key_Down *event, void *data);
static void _etk_scrolled_view_mouse_wheel(Etk_Object *object, Etk_Event_Mouse_Wheel *event, void *data);
static void _etk_scrolled_view_hscrollbar_value_changed_cb(Etk_Object *object, double value, void *data);
static void _etk_scrolled_view_vscrollbar_value_changed_cb(Etk_Object *object, double value, void *data);
static void _etk_scrolled_view_child_added_cb(Etk_Object *object, void *child, void *data);
static void _etk_scrolled_view_child_removed_cb(Etk_Object *object, void *child, void *data);
static void _etk_scrolled_view_child_scroll_size_changed_cb(Etk_Object *object, void *data);

/**************************
 *
 * Implementation
 *
 **************************/

/**
 * @internal
 * @brief Gets the type of an Etk_Scrolled_View
 * @return Returns the type of an Etk_Scrolled_View
 */
Etk_Type *etk_scrolled_view_type_get()
{
   static Etk_Type *scrolled_view_type = NULL;

   if (!scrolled_view_type)
   {
      scrolled_view_type = etk_type_new("Etk_Scrolled_View", ETK_BIN_TYPE, sizeof(Etk_Scrolled_View),
         ETK_CONSTRUCTOR(_etk_scrolled_view_constructor), NULL);

      etk_type_property_add(scrolled_view_type, "hpolicy", ETK_SCROLLED_VIEW_HPOLICY_PROPERTY,
         ETK_PROPERTY_INT, ETK_PROPERTY_READABLE_WRITABLE, etk_property_value_int(ETK_POLICY_AUTO));
      etk_type_property_add(scrolled_view_type, "vpolicy", ETK_SCROLLED_VIEW_VPOLICY_PROPERTY,
         ETK_PROPERTY_INT, ETK_PROPERTY_READABLE_WRITABLE, etk_property_value_int(ETK_POLICY_AUTO));
      
      scrolled_view_type->property_set = _etk_scrolled_view_property_set;
      scrolled_view_type->property_get = _etk_scrolled_view_property_get;
   }

   return scrolled_view_type;
}

/**
 * @brief Creates a new scrolled view
 * @return Returns the new scrolled view widget
 */
Etk_Widget *etk_scrolled_view_new()
{
   return etk_widget_new(ETK_SCROLLED_VIEW_TYPE, "theme-group", "scrolled_view", NULL);
}

/**
 * @brief Gets the hoizontal scrollbar of the scrolled view. You can then change its value, bound values, ...
 * @param scrolled_view a scrolled view
 * @return Returns the hoizontal scrollbar of the scrolled view
 */
Etk_Range *etk_scrolled_view_hscrollbar_get(Etk_Scrolled_View *scrolled_view)
{
   if (!scrolled_view)
      return NULL;
   return ETK_RANGE(scrolled_view->hscrollbar);
}

/**
 * @brief Gets the vertical scrollbar of the scrolled view. You can then change its value, bound values, ...
 * @param scrolled_view a scrolled view
 * @return Returns the vertical scrollbar of the scrolled view
 */
Etk_Range *etk_scrolled_view_vscrollbar_get(Etk_Scrolled_View *scrolled_view)
{
   if (!scrolled_view)
      return NULL;
   return ETK_RANGE(scrolled_view->vscrollbar);
}

/**
 * @brief A convenient function that creates a viewport, attachs the child to it and adds the viewport to the
 * scrolled view. It's useful for widgets that have no scrolling ability
 * @param scrolled_view a scrolled view
 * @param child the child to add to the viewport
 */
void etk_scrolled_view_add_with_viewport(Etk_Scrolled_View *scrolled_view, Etk_Widget *child)
{
   Etk_Widget *viewport;

   if (!scrolled_view || !child)
      return;

   if (ETK_BIN(scrolled_view)->child && ETK_IS_VIEWPORT(ETK_BIN(scrolled_view)->child))
      viewport = ETK_BIN(scrolled_view)->child;
   else
   {
      viewport = etk_viewport_new();
      etk_container_add(ETK_CONTAINER(scrolled_view), viewport);
      etk_widget_internal_set(viewport, ETK_TRUE);
      etk_widget_show(viewport);
   }

   etk_container_add(ETK_CONTAINER(viewport), child);
}

/**
 * @brief Sets the visibility policy of the hscrollbar and the vscrollbar of the scrolled view
 * @param scrolled_view a scrolled view
 * @param hpolicy the visibility policy to use for the hscrollbar
 * @param vpolicy the visibility policy to use for the vscrollbar
 * @see Etk_Scrolled_View_Policy
 */
void etk_scrolled_view_policy_set(Etk_Scrolled_View *scrolled_view, Etk_Scrolled_View_Policy hpolicy, Etk_Scrolled_View_Policy vpolicy)
{
   if (!scrolled_view)
      return;

   if (scrolled_view->hpolicy != hpolicy)
   {
      scrolled_view->hpolicy = hpolicy;
      etk_widget_redraw_queue(ETK_WIDGET(scrolled_view));
      etk_object_notify(ETK_OBJECT(scrolled_view), "hpolicy");
   }
   if (scrolled_view->vpolicy != vpolicy)
   {
      scrolled_view->vpolicy = vpolicy;
      etk_widget_redraw_queue(ETK_WIDGET(scrolled_view));
      etk_object_notify(ETK_OBJECT(scrolled_view), "vpolicy");
   }
}

/**
 * @brief Gets the visibility policy of the hscrollbar and the vscrollbar of the scrolled view
 * @param scrolled_view a scrolled view
 * @param hpolicy the location where to store the visibility policy of the hscrollbar
 * @param vpolicy the location where to store the visibility policy of the vscrollbar
 */
void etk_scrolled_view_policy_get(Etk_Scrolled_View *scrolled_view, Etk_Scrolled_View_Policy *hpolicy, Etk_Scrolled_View_Policy *vpolicy)
{
   if (!scrolled_view)
      return;
   
   if (hpolicy)
      *hpolicy = scrolled_view ? scrolled_view->hpolicy : ETK_POLICY_AUTO;
   if (vpolicy)
      *vpolicy = scrolled_view ? scrolled_view->vpolicy : ETK_POLICY_AUTO;
}


/**************************
 *
 * Etk specific functions
 *
 **************************/

/* Initializes the scrolled view */
static void _etk_scrolled_view_constructor(Etk_Scrolled_View *scrolled_view)
{
   if (!scrolled_view)
      return;

   scrolled_view->hpolicy = ETK_POLICY_AUTO;
   scrolled_view->vpolicy = ETK_POLICY_AUTO;

   scrolled_view->hscrollbar = etk_hscrollbar_new(0.0, 0.0, 0.0, 12.0, 50.0, 0.0);
   etk_widget_theme_parent_set(scrolled_view->hscrollbar, ETK_WIDGET(scrolled_view));
   etk_widget_parent_set(scrolled_view->hscrollbar, ETK_WIDGET(scrolled_view));
   etk_widget_internal_set(scrolled_view->hscrollbar, ETK_TRUE);
   etk_widget_show(scrolled_view->hscrollbar);
   
   scrolled_view->vscrollbar = etk_vscrollbar_new(0.0, 0.0, 0.0, 12.0, 50.0, 0.0);
   etk_widget_theme_parent_set(scrolled_view->vscrollbar, ETK_WIDGET(scrolled_view));
   etk_widget_parent_set(scrolled_view->vscrollbar, ETK_WIDGET(scrolled_view));
   etk_widget_internal_set(scrolled_view->vscrollbar, ETK_TRUE);
   etk_widget_show(scrolled_view->vscrollbar);

   ETK_WIDGET(scrolled_view)->size_request = _etk_scrolled_view_size_request;
   ETK_WIDGET(scrolled_view)->size_allocate = _etk_scrolled_view_size_allocate;

   etk_signal_connect("key-down", ETK_OBJECT(scrolled_view), ETK_CALLBACK(_etk_scrolled_view_key_down_cb), NULL);
   etk_signal_connect("mouse-wheel", ETK_OBJECT(scrolled_view), ETK_CALLBACK(_etk_scrolled_view_mouse_wheel), NULL);
   etk_signal_connect("child-added", ETK_OBJECT(scrolled_view), ETK_CALLBACK(_etk_scrolled_view_child_added_cb), NULL);
   etk_signal_connect("child-removed", ETK_OBJECT(scrolled_view), ETK_CALLBACK(_etk_scrolled_view_child_removed_cb), NULL);
   etk_signal_connect("value-changed", ETK_OBJECT(scrolled_view->hscrollbar), ETK_CALLBACK(_etk_scrolled_view_hscrollbar_value_changed_cb), scrolled_view);
   etk_signal_connect("value-changed", ETK_OBJECT(scrolled_view->vscrollbar), ETK_CALLBACK(_etk_scrolled_view_vscrollbar_value_changed_cb), scrolled_view);
}

/* Sets the property whose id is "property_id" to the value "value" */
static void _etk_scrolled_view_property_set(Etk_Object *object, int property_id, Etk_Property_Value *value)
{
   Etk_Scrolled_View *scrolled_view;

   if (!(scrolled_view = ETK_SCROLLED_VIEW(object)) || !value)
      return;

   switch (property_id)
   {
      case ETK_SCROLLED_VIEW_HPOLICY_PROPERTY:
         etk_scrolled_view_policy_set(scrolled_view, etk_property_value_int_get(value), scrolled_view->vpolicy);
         break;
      case ETK_SCROLLED_VIEW_VPOLICY_PROPERTY:
         etk_scrolled_view_policy_set(scrolled_view, scrolled_view->hpolicy, etk_property_value_int_get(value));
         break;
      default:
         break;
   }
}

/* Gets the value of the property whose id is "property_id" */
static void _etk_scrolled_view_property_get(Etk_Object *object, int property_id, Etk_Property_Value *value)
{
   Etk_Scrolled_View *scrolled_view;

   if (!(scrolled_view = ETK_SCROLLED_VIEW(object)) || !value)
      return;

   switch (property_id)
   {
      case ETK_SCROLLED_VIEW_HPOLICY_PROPERTY:
         etk_property_value_int_set(value, scrolled_view->hpolicy);
         break;
      case ETK_SCROLLED_VIEW_VPOLICY_PROPERTY:
         etk_property_value_int_set(value, scrolled_view->vpolicy);
         break;
      default:
         break;
   }
}

/* Calculates the ideal size for the scrolled view */
static void _etk_scrolled_view_size_request(Etk_Widget *widget, Etk_Size *size)
{
   Etk_Scrolled_View *scrolled_view;
   Etk_Size hscrollbar_size, vscrollbar_size, child_size;

   if (!(scrolled_view = ETK_SCROLLED_VIEW(widget)) || !size)
      return;

   if (ETK_BIN(scrolled_view)->child)
   {
      etk_widget_size_request_full(scrolled_view->hscrollbar, &hscrollbar_size, ETK_FALSE);
      etk_widget_size_request_full(scrolled_view->vscrollbar, &vscrollbar_size, ETK_FALSE);
      etk_widget_size_request(ETK_BIN(scrolled_view)->child, &child_size);

      size->w = ETK_MAX(child_size.w, hscrollbar_size.w + vscrollbar_size.w);
      size->h = ETK_MAX(child_size.h, hscrollbar_size.h + vscrollbar_size.h);
   }
   else
   {
      size->w = 0;
      size->h = 0;
   }
}

/* Resizes the scrolled view to the allocated size */
static void _etk_scrolled_view_size_allocate(Etk_Widget *widget, Etk_Geometry geometry)
{
   Etk_Scrolled_View *scrolled_view;
   Etk_Size hscrollbar_size, vscrollbar_size;
   Etk_Size scrollview_size;
   Etk_Size scrollbar_size;
   Etk_Size scroll_size;
   Etk_Geometry child_geometry;
   Etk_Widget *child;
   Etk_Bool show_vscrollbar = ETK_FALSE, show_hscrollbar = ETK_FALSE;
   
   if (!(scrolled_view = ETK_SCROLLED_VIEW(widget)))
      return;
   
   if (!(child = ETK_BIN(scrolled_view)->child) || !child->scroll_size_get || !child->scroll)
   {
      etk_widget_hide(scrolled_view->hscrollbar);
      etk_widget_hide(scrolled_view->vscrollbar);
      if (child)
         etk_widget_size_allocate(child, geometry);
      return;
   }

   if (scrolled_view->hpolicy == ETK_POLICY_AUTO || scrolled_view->hpolicy == ETK_POLICY_SHOW)
      etk_widget_size_request_full(scrolled_view->hscrollbar, &hscrollbar_size, ETK_FALSE);
   else
   {
      hscrollbar_size.w = 0;
      hscrollbar_size.h = 0;
   }
   if (scrolled_view->vpolicy == ETK_POLICY_AUTO || scrolled_view->vpolicy == ETK_POLICY_SHOW)
      etk_widget_size_request_full(scrolled_view->vscrollbar, &vscrollbar_size, ETK_FALSE);
   else
   {
      vscrollbar_size.w = 0;
      vscrollbar_size.h = 0;
   }
   
   scrollview_size.w = geometry.w - child->inset.left - child->inset.right;
   scrollview_size.h = geometry.h - child->inset.top - child->inset.bottom;
   if (child->scroll_margins_get)
   {
      Etk_Size margins_size;
      
      child->scroll_margins_get(child, &margins_size);
      scrollview_size.w -= margins_size.w;
      scrollview_size.h -= margins_size.h;
   }
   
   scrollbar_size.w = vscrollbar_size.w;
   scrollbar_size.h = hscrollbar_size.h;
   child->scroll_size_get(child, scrollview_size, scrollbar_size, &scroll_size);
   
   if ((scrolled_view->hpolicy == ETK_POLICY_AUTO && scroll_size.w > scrollview_size.w)
      || scrolled_view->hpolicy == ETK_POLICY_SHOW)
   {
      show_hscrollbar = ETK_TRUE;
   }
   if ((scrolled_view->vpolicy == ETK_POLICY_AUTO
         && scroll_size.h > (scrollview_size.h - (show_hscrollbar ? hscrollbar_size.h : 0)))
      || scrolled_view->vpolicy == ETK_POLICY_SHOW)
   {
      show_vscrollbar = ETK_TRUE;
      if (scrolled_view->hpolicy == ETK_POLICY_AUTO && scroll_size.w > (scrollview_size.w - vscrollbar_size.w))
         show_hscrollbar = ETK_TRUE;
   }

   /* Moves and resizes the hscrollbar */
   if (show_hscrollbar)
   {
      scrollview_size.h -= hscrollbar_size.h;
      etk_widget_show(scrolled_view->hscrollbar);
      
      child_geometry.x = geometry.x;
      child_geometry.y = geometry.y + geometry.h - hscrollbar_size.h;
      child_geometry.w = geometry.w - (show_vscrollbar ? vscrollbar_size.w : 0);
      child_geometry.h = hscrollbar_size.h;
      etk_widget_size_allocate(scrolled_view->hscrollbar, child_geometry);
   }
   else
      etk_widget_hide(scrolled_view->hscrollbar);

   /* Moves and resizes the vscrollbar */
   if (show_vscrollbar)
   {
      scrollview_size.w -= vscrollbar_size.w;
      etk_widget_show(scrolled_view->vscrollbar);

      child_geometry.x = geometry.x + geometry.w - vscrollbar_size.w;
      child_geometry.y = geometry.y;
      child_geometry.w = vscrollbar_size.w;
      child_geometry.h = geometry.h - (show_hscrollbar ? hscrollbar_size.h : 0);
      etk_widget_size_allocate(scrolled_view->vscrollbar, child_geometry);
   }
   else
      etk_widget_hide(scrolled_view->vscrollbar);

   etk_range_range_set(ETK_RANGE(scrolled_view->hscrollbar), 0, scroll_size.w);
   etk_range_page_size_set(ETK_RANGE(scrolled_view->hscrollbar), scrollview_size.w);
   etk_range_range_set(ETK_RANGE(scrolled_view->vscrollbar), 0, scroll_size.h);
   etk_range_page_size_set(ETK_RANGE(scrolled_view->vscrollbar), scrollview_size.h);

   /* Moves and resizes the child */
   child_geometry.x = geometry.x;
   child_geometry.y = geometry.y;
   child_geometry.w = geometry.w - (show_vscrollbar ? vscrollbar_size.w : 0);
   child_geometry.h = geometry.h - (show_hscrollbar ? hscrollbar_size.h : 0);
   etk_widget_size_allocate(child, child_geometry);
}

/**************************
 *
 * Callbacks and handlers
 *
 **************************/

/* Called when the user presses a key */
static void _etk_scrolled_view_key_down_cb(Etk_Object *object, Etk_Event_Key_Down *event, void *data)
{
   Etk_Scrolled_View *scrolled_view;
   Etk_Range *hscrollbar_range;
   Etk_Range *vscrollbar_range;
   Etk_Bool propagate = ETK_FALSE;

   if (!(scrolled_view = ETK_SCROLLED_VIEW(object)))
      return;
   hscrollbar_range = ETK_RANGE(scrolled_view->hscrollbar);
   vscrollbar_range = ETK_RANGE(scrolled_view->vscrollbar);

   if (strcmp(event->keyname, "Right") == 0)
      etk_range_value_set(hscrollbar_range, hscrollbar_range->value + hscrollbar_range->step_increment);
   else if (strcmp(event->keyname, "Down") == 0)
      etk_range_value_set(vscrollbar_range, vscrollbar_range->value + vscrollbar_range->step_increment);
   else if (strcmp(event->keyname, "Left") == 0)
      etk_range_value_set(hscrollbar_range, hscrollbar_range->value - hscrollbar_range->step_increment);
   else if (strcmp(event->keyname, "Up") == 0)
      etk_range_value_set(vscrollbar_range, vscrollbar_range->value - vscrollbar_range->step_increment);
   else if (strcmp(event->keyname, "Home") == 0)
      etk_range_value_set(vscrollbar_range, vscrollbar_range->lower);
   else if (strcmp(event->keyname, "End") == 0)
      etk_range_value_set(vscrollbar_range, vscrollbar_range->upper);
   else if (strcmp(event->keyname, "Next") == 0)
      etk_range_value_set(vscrollbar_range, vscrollbar_range->value + vscrollbar_range->page_increment);
   else if (strcmp(event->keyname, "Prior") == 0)
      etk_range_value_set(vscrollbar_range, vscrollbar_range->value - vscrollbar_range->page_increment);
   else
      propagate = ETK_TRUE;
   
   if (!propagate)
      etk_signal_stop();
}

/* Called when the user wants to scroll the scrolled view with the mouse wheel */
static void _etk_scrolled_view_mouse_wheel(Etk_Object *object, Etk_Event_Mouse_Wheel *event, void *data)
{
   Etk_Scrolled_View *scrolled_view;
   Etk_Range *vscrollbar_range;
   
   if (!(scrolled_view = ETK_SCROLLED_VIEW(object)))
      return;
   
   vscrollbar_range = ETK_RANGE(scrolled_view->vscrollbar);
   etk_range_value_set(vscrollbar_range, vscrollbar_range->value + event->z * vscrollbar_range->step_increment);
   etk_signal_stop();
}

/* Called when the value of the hscrollbar has changed */
static void _etk_scrolled_view_hscrollbar_value_changed_cb(Etk_Object *object, double value, void *data)
{
   Etk_Scrolled_View *scrolled_view;
   Etk_Widget *child;

   if (!(scrolled_view = ETK_SCROLLED_VIEW(data)) || !(child = ETK_BIN(scrolled_view)->child) || !child->scroll)
      return;
   child->scroll(child, value, ETK_RANGE(scrolled_view->vscrollbar)->value);
}

/* Called when the value of the vscrollbar has changed */
static void _etk_scrolled_view_vscrollbar_value_changed_cb(Etk_Object *object, double value, void *data)
{
   Etk_Scrolled_View *scrolled_view;
   Etk_Widget *child;

   if (!(scrolled_view = ETK_SCROLLED_VIEW(data)) || !(child = ETK_BIN(scrolled_view)->child) || !child->scroll)
      return;
   child->scroll(child, ETK_RANGE(scrolled_view->hscrollbar)->value, value);
}

/* Called when a new child is added */
static void _etk_scrolled_view_child_added_cb(Etk_Object *object, void *child, void *data)
{
   if (!object || !child)
      return;
   etk_signal_connect("scroll-size-changed", ETK_OBJECT(child),
      ETK_CALLBACK(_etk_scrolled_view_child_scroll_size_changed_cb), object); 
}

/* Called when a child is removed */
static void _etk_scrolled_view_child_removed_cb(Etk_Object *object, void *child, void *data)
{
   if (!object || !child)
      return;
   etk_signal_disconnect("scroll-size-changed", ETK_OBJECT(child), 
      ETK_CALLBACK(_etk_scrolled_view_child_scroll_size_changed_cb));
}

/* Called when the scroll size of the scrolled view's child has changed */
static void _etk_scrolled_view_child_scroll_size_changed_cb(Etk_Object *object, void *data)
{
   Etk_Widget *child;
   Etk_Scrolled_View *scrolled_view;
   Etk_Size hscrollbar_requisition, vscrollbar_requisition;
   Etk_Size scrollview_size;
   Etk_Size scrollbar_size;
   Etk_Size scroll_size;
   
   if (!(child = ETK_WIDGET(object)) || !child->scroll_size_get || !(scrolled_view = ETK_SCROLLED_VIEW(data)))
      return;
   
   if (scrolled_view->hpolicy == ETK_POLICY_AUTO || scrolled_view->hpolicy == ETK_POLICY_SHOW)
      etk_widget_size_request_full(scrolled_view->hscrollbar, &hscrollbar_requisition, ETK_FALSE);
   else
   {
      hscrollbar_requisition.w = 0;
      hscrollbar_requisition.h = 0;
   }
   if (scrolled_view->vpolicy == ETK_POLICY_AUTO || scrolled_view->vpolicy == ETK_POLICY_SHOW)
      etk_widget_size_request_full(scrolled_view->vscrollbar, &vscrollbar_requisition, ETK_FALSE);
   else
   {
      vscrollbar_requisition.w = 0;
      vscrollbar_requisition.h = 0;
   }
   
   etk_widget_inner_geometry_get(ETK_WIDGET(scrolled_view), NULL, NULL, &scrollview_size.w, &scrollview_size.h);
   scrollbar_size.w = vscrollbar_requisition.w;
   scrollbar_size.h = hscrollbar_requisition.h;
   child->scroll_size_get(child, scrollview_size, scrollbar_size, &scroll_size);
   
   etk_range_range_set(ETK_RANGE(scrolled_view->hscrollbar), 0, scroll_size.w);
   etk_range_range_set(ETK_RANGE(scrolled_view->vscrollbar), 0, scroll_size.h);
   etk_widget_redraw_queue(ETK_WIDGET(scrolled_view));
}

/** @} */

/**************************
 *
 * Documentation
 *
 **************************/

/**
 * @addtogroup Etk_Scrolled_View
 *
 * @image html widgets/scrolled_view.png
 * A scrolled view is made up of a hscrollbar which controls the horizontal scrolling of the child, and of a
 * vscrollbar which controls the vertical scrolling of the child. @n
 * These two scrollbars can have different visibility policy:
 * - <b>ETK_POLICY_SHOW</b>: the scrollbar is always shown
 * - <b>ETK_POLICY_HIDE</b>: the scrollbar is always hidden
 * - <b>ETK_POLICY_AUTO</b>: the scrollbar is shown and hidden automatically whether or not the child can fit entirely in the
 * scrolled view
 *
 * The visibility policy can be set with etk_scrolled_view_policy_set(). @n
 *
 * Most of the widgets doesn't have a scrolling ability, which means that you have to create an Etk_Viewport that
 * implements this ability, attach the child to the viewport, and add the viewport to the scrolled view.
 * etk_scrolled_view_add_with_viewport() is a convenient function that does that for you. @n @n
 * 
 * \par Object Hierarchy:
 * - Etk_Object
 *   - Etk_Widget
 *     - Etk_Container
 *       - Etk_Bin
 *         - Etk_Scrolled_View
 *
 * \par Properties:
 * @prop_name "hpolicy": The visibility policy of the horizontal scrollbar
 * @prop_type Integer (Etk_Scrolled_View_Policy)
 * @prop_rw
 * @prop_val ETK_POLICY_AUTO
 * \par
 * @prop_name "vpolicy": The visibility policy of the vertical scrollbar
 * @prop_type Integer (Etk_Scrolled_View_Policy)
 * @prop_rw
 * @prop_val ETK_POLICY_AUTO
 */
