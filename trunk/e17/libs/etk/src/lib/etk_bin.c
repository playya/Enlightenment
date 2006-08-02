/** @file etk_bin.c */
#include "etk_bin.h"
#include <stdlib.h>
#include <Evas.h>
#include "etk_utils.h"
#include "etk_signal.h"
#include "etk_signal_callback.h"

/**
 * @addtogroup Etk_Bin
 * @{
 */

enum Etk_Bin_Property_Id
{
   ETK_BIN_CHILD_PROPERTY
};

static void _etk_bin_constructor(Etk_Bin *bin);
static void _etk_bin_property_set(Etk_Object *object, int property_id, Etk_Property_Value *value);
static void _etk_bin_property_get(Etk_Object *object, int property_id, Etk_Property_Value *value);
static void _etk_bin_child_add(Etk_Container *container, Etk_Widget *widget);
static void _etk_bin_child_remove(Etk_Container *container, Etk_Widget *widget);
static Evas_List *_etk_bin_children_get(Etk_Container *container);
static void _etk_bin_size_request(Etk_Widget *widget, Etk_Size *size);
static void _etk_bin_size_allocate(Etk_Widget *widget, Etk_Geometry geometry);
static void _etk_bin_realize_cb(Etk_Object *object, void *data);

/**************************
 *
 * Implementation
 *
 **************************/

/**
 * @brief Gets the type of an Etk_Bin
 * @return Returns the type of an Etk_Bin
 */
Etk_Type *etk_bin_type_get()
{
   static Etk_Type *bin_type = NULL;

   if (!bin_type)
   {
      bin_type = etk_type_new("Etk_Bin", ETK_CONTAINER_TYPE, sizeof(Etk_Bin),
         ETK_CONSTRUCTOR(_etk_bin_constructor), NULL);
      
      etk_type_property_add(bin_type, "child", ETK_BIN_CHILD_PROPERTY,
         ETK_PROPERTY_POINTER, ETK_PROPERTY_READABLE_WRITABLE,  etk_property_value_pointer(NULL));
      
      bin_type->property_set = _etk_bin_property_set;
      bin_type->property_get = _etk_bin_property_get;
   }

   return bin_type;
}

/**
 * @brief Gets the child of the bin
 * @param bin a bin
 * @return Returns the child of the bin or NULL if it doesn't have a child
 */
Etk_Widget *etk_bin_child_get(Etk_Bin *bin)
{
   if (!bin)
      return NULL;
   return bin->child;
}

/**
 * @brief Sets the child of the bin
 * @param bin a bin
 * @param child the child to set
 */
void etk_bin_child_set(Etk_Bin *bin, Etk_Widget *child)
{
   if (!bin || bin->child == child)
      return;

   _etk_bin_child_remove(ETK_CONTAINER(bin), bin->child);

   if (child)
   {
      etk_widget_parent_set(child, ETK_WIDGET(bin));
      bin->child = child;
      
      /* TODO; warnings? */
      etk_widget_swallow_widget(ETK_WIDGET(bin), "swallow_area", bin->child);
      
      etk_object_notify(ETK_OBJECT(bin), "child");
      etk_signal_emit_by_name("child_added", ETK_OBJECT(bin), NULL, child);
   }
}

/**************************
 *
 * Etk specific functions
 *
 **************************/

/* Initializes the bin */
static void _etk_bin_constructor(Etk_Bin *bin)
{
   if (!bin)
      return;

   bin->child = NULL;
   ETK_CONTAINER(bin)->child_add = _etk_bin_child_add;
   ETK_CONTAINER(bin)->child_remove = _etk_bin_child_remove;
   ETK_CONTAINER(bin)->children_get = _etk_bin_children_get;
   ETK_WIDGET(bin)->size_request = _etk_bin_size_request;
   ETK_WIDGET(bin)->size_allocate = _etk_bin_size_allocate;
   
   etk_signal_connect("realize", ETK_OBJECT(bin), ETK_CALLBACK(_etk_bin_realize_cb), NULL);
}

/* Sets the property whose id is "property_id" to the value "value" */
static void _etk_bin_property_set(Etk_Object *object, int property_id, Etk_Property_Value *value)
{
   Etk_Bin *bin;

   if (!(bin = ETK_BIN(object)) || !value)
      return;

   switch (property_id)
   {
      case ETK_BIN_CHILD_PROPERTY:
         etk_bin_child_set(bin, ETK_WIDGET(etk_property_value_pointer_get(value)));
         break;
      default:
         break;
   }
}

/* Gets the value of the property whose id is "property_id" */
static void _etk_bin_property_get(Etk_Object *object, int property_id, Etk_Property_Value *value)
{
   Etk_Bin *bin;

   if (!(bin = ETK_BIN(object)) || !value)
      return;

   switch (property_id)
   {
      case ETK_BIN_CHILD_PROPERTY:
         etk_property_value_pointer_set(value, bin->child);
         break;
      default:
         break;
   }
}

/* Calculates the ideal size of the bin */
static void _etk_bin_size_request(Etk_Widget *widget, Etk_Size *size)
{
   Etk_Bin *bin;
   Etk_Container *container;

   if (!(bin = ETK_BIN(widget)) || !size)
      return;
   container = ETK_CONTAINER(bin);

   if (!bin->child || etk_widget_is_swallowed(bin->child))
   {
      size->w = 0;
      size->h = 0;
   }
   else
      etk_widget_size_request(bin->child, size);

   size->w += 2 * etk_container_border_width_get(container);
   size->h += 2 * etk_container_border_width_get(container);
}

/* Resizes the bin to the allocated size */
static void _etk_bin_size_allocate(Etk_Widget *widget, Etk_Geometry geometry)
{
   Etk_Bin *bin;
   Etk_Container *container;
   int border;

   if (!(bin = ETK_BIN(widget)))
      return;
   container = ETK_CONTAINER(widget);
   
   if (bin->child && !etk_widget_is_swallowed(bin->child))
   {
      border = etk_container_border_width_get(container);
      geometry.x += border;
      geometry.y += border;
      geometry.w -= 2 * border;
      geometry.h -= 2 * border;
      etk_widget_size_allocate(bin->child, geometry);
   }
}

/* Adds a child to the bin */
static void _etk_bin_child_add(Etk_Container *container, Etk_Widget *widget)
{
   if (!container || !widget)
      return;
   etk_bin_child_set(ETK_BIN(container), widget);
}

/* Removes the child from the bin */
static void _etk_bin_child_remove(Etk_Container *container, Etk_Widget *widget)
{
   Etk_Bin *bin;

   if (!(bin = ETK_BIN(container)) || !widget || bin->child != widget)
      return;

   etk_widget_parent_set_full(widget, NULL, ETK_FALSE);
   bin->child = NULL;
   
   etk_object_notify(ETK_OBJECT(bin), "child");
   etk_signal_emit_by_name("child_removed", ETK_OBJECT(bin), NULL, widget);
}

/* Gets the children (the child actually) of the bin */
static Evas_List *_etk_bin_children_get(Etk_Container *container)
{
   Etk_Bin *bin;
   Evas_List *children;
   
   if (!(bin = ETK_BIN(container)))
      return NULL;
   
   children = NULL;
   if (bin->child)
      children = evas_list_append(children, bin->child);
   
   return children;
}

/**************************
 *
 * Callbacks and handlers
 *
 **************************/

/* Called when the bin is realized */
static void _etk_bin_realize_cb(Etk_Object *object, void *data)
{
   Etk_Bin *bin;

   if (!(bin = ETK_BIN(object)) || !bin->child)
      return;
   /* TODO; warnings? */
   etk_widget_swallow_widget(ETK_WIDGET(bin), "swallow_area", bin->child);
}

/** @} */

/**************************
 *
 * Documentation
 *
 **************************/

/**
 * @addtogroup Etk_Bin
 *
 * The Etk_Bin widget is mainly used as a base class for containers that only have one children
 * (such as Etk_Alignment, Etk_Window, Etk_Frame...).
 *
 * \par Object Hierarchy:
 * - Etk_Object
 *   - Etk_Widget
 *     - Etk_Container
 *       - Etk_Bin
 *
 * \par Properties:
 * @prop_name "child": The child of the bin.
 * @prop_type Pointer (Etk_Widget *)
 * @prop_rw
 * @prop_val NULL
 */
