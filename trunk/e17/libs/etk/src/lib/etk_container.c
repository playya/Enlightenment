/** @file etk_container.c */
#include "etk_container.h"
#include <stdlib.h>
#include "etk_signal.h"
#include "etk_signal_callback.h"

/**
 * @addtogroup Etk_Container
* @{
 */

enum _Etk_Container_Signal_Id
{
   ETK_CONTAINER_CHILD_ADDED_SIGNAL,
   ETK_CONTAINER_CHILD_REMOVED_SIGNAL,
   ETK_CONTAINER_NUM_SIGNALS
};

enum _Etk_Container_Property_Id
{
   ETK_CONTAINER_BORDER_WIDTH_PROPERTY
};

static void _etk_container_constructor(Etk_Container *container);
static void _etk_container_destructor(Etk_Container *container);
static void _etk_container_property_set(Etk_Object *object, int property_id, Etk_Property_Value *value);
static void _etk_container_property_get(Etk_Object *object, int property_id, Etk_Property_Value *value);

static Etk_Signal *_etk_container_signals[ETK_CONTAINER_NUM_SIGNALS];

/**************************
 *
 * Implementation
 *
 **************************/

/**
 * @brief Gets the type of an Etk_Container
 * @return Returns the type on an Etk_Container
 */
Etk_Type *etk_container_type_get()
{
   static Etk_Type *container_type = NULL;

   if (!container_type)
   {
      container_type = etk_type_new("Etk_Container", ETK_WIDGET_TYPE, sizeof(Etk_Container), ETK_CONSTRUCTOR(_etk_container_constructor), ETK_DESTRUCTOR(_etk_container_destructor));
   
      _etk_container_signals[ETK_CONTAINER_CHILD_ADDED_SIGNAL] = etk_signal_new("child_added", container_type, -1, etk_marshaller_VOID__POINTER, NULL, NULL);
      _etk_container_signals[ETK_CONTAINER_CHILD_REMOVED_SIGNAL] = etk_signal_new("child_removed", container_type, -1, etk_marshaller_VOID__POINTER, NULL, NULL);

      etk_type_property_add(container_type, "border_width", ETK_CONTAINER_BORDER_WIDTH_PROPERTY, ETK_PROPERTY_INT, ETK_PROPERTY_READABLE_WRITABLE,  etk_property_value_int(0));
   
      container_type->property_set = _etk_container_property_set;
      container_type->property_get = _etk_container_property_get;
   }

   return container_type;
}

/**
 * @brief Adds a child to the container
 * @param container a container
 * @param widget the widget to add
 */
void etk_container_add(Etk_Container *container, Etk_Widget *widget)
{
   if (!container || !widget || !container->child_add)
      return;
   container->child_add(container, widget);
}

/**
 * @brief Removes a child from the container
 * @param container a container
 * @param widget the widget to remove
 */
void etk_container_remove(Etk_Container *container, Etk_Widget *widget)
{
   if (!container || !widget || !container->child_remove)
      return;
   container->child_remove(container, widget);
}

/**
 * @brief Sets the border width of the container
 * @param container a container
 * @param border_width the border width to set
 */
void etk_container_border_width_set(Etk_Container *container, int border_width)
{
   if (!container)
      return;

   container->border_width = border_width;
   etk_object_notify(ETK_OBJECT(container), "border_width");
   etk_widget_size_recalc_queue(ETK_WIDGET(container));
}

/**
 * @brief Gets the border width of the container
 * @param container a container
 * @return Returns the border width of the container
 */
int etk_container_border_width_get(Etk_Container *container)
{
   if (!container)
      return 0;
   return container->border_width;
}

/**
 * @brief Gets the list of the children of the container
 * @param container a container
 * @return Returns the list of the children of @a container
 */
Evas_List *etk_container_children_get(Etk_Container *container)
{
   if (!container)
      return NULL;
   return container->children;
}

/**
 * @brief Gets whether the widget is a child of the container
 * @param container a container
 * @param widget the widget you want to check if it is a child of the container
 */
Etk_Bool etk_container_is_child(Etk_Container *container, Etk_Widget *widget)
{
   if (!container || !widget)
      return FALSE;
   return (evas_list_find(container->children, widget) != NULL);
}

/**
 * @brief Calls @a for_each_cb(child, data) for each child of the container
 * @param container the container
 * @param for_each_cb the function to call
 */
void etk_container_for_each(Etk_Container *container, void (*for_each_cb)(Etk_Widget *child))
{
   Evas_List *l;

   if (!container || !container->children || !for_each_cb)
      return;

   for (l = container->children; l; l = l->next)
      for_each_cb(ETK_WIDGET(l->data));
}

/**
 * @brief Calls @a for_each_cb(child) for each child of the container
 * @param container the container
 * @param for_each_cb the function to call
 * @param data the data to pass as the second argument of @a for_each_cb()
 */
void etk_container_for_each_data(Etk_Container *container, void (*for_each_cb)(Etk_Widget *child, void *data), void *data)
{
   Evas_List *l;

   if (!container || !container->children || !for_each_cb)
      return;

   for (l = container->children; l; l = l->next)
      for_each_cb(ETK_WIDGET(l->data), data);
}

/**
 * @brief Resizes the allocated child space acoording to the fill policy (mainly for container implementations)
 * @param child a child
 * @param child_space the allocated space for the child
 * @param hfill if hfill == TRUE, the child should fill the space horizontally
 * @param vfill if vfill == TRUE, the child should fill the space vertically
 */
void etk_container_child_space_fill(Etk_Widget *child, Etk_Geometry *child_space, Etk_Bool hfill, Etk_Bool vfill)
{
   Etk_Size min_size;

   if (!child || !child_space)
      return;

   etk_widget_size_request(child, &min_size);
   if (!hfill && child_space->w > min_size.w)
   {
      child_space->x += (child_space->w - min_size.w) / 2;
      child_space->w = min_size.w;
   }
   if (!vfill && child_space->h > min_size.h)
   {
      child_space->y += (child_space->h - min_size.h) / 2;
      child_space->h = min_size.h;
   }
}

/**************************
 *
 * Etk specific functions
 *
 **************************/

/* Initializes the members of the container */
static void _etk_container_constructor(Etk_Container *container)
{
   if (!container)
      return;

   container->children = NULL;
   container->child_add = NULL;
   container->child_remove = NULL;
   container->border_width = 0;
}

/* Destroys the container */
static void _etk_container_destructor(Etk_Container *container)
{
   Etk_Widget *child;

   if (!container)
      return;

   while (container->children)
   {
      child = ETK_WIDGET(container->children->data);
      etk_object_destroy(ETK_OBJECT(child));
   }
}


/* Sets the property whose id is "property_id" to the value "value" */
static void _etk_container_property_set(Etk_Object *object, int property_id, Etk_Property_Value *value)
{
   Etk_Container *container;

   if (!(container = ETK_CONTAINER(object)) || !value)
      return;

   switch (property_id)
   {
      case ETK_CONTAINER_BORDER_WIDTH_PROPERTY:
         etk_container_border_width_set(container, etk_property_value_int_get(value));
         break;
      default:
         break;
   }
}

/* Gets the value of the property whose id is "property_id" */
static void _etk_container_property_get(Etk_Object *object, int property_id, Etk_Property_Value *value)
{
   Etk_Container *container;

   if (!(container = ETK_CONTAINER(object)) || !value)
      return;

   switch (property_id)
   {
      case ETK_CONTAINER_BORDER_WIDTH_PROPERTY:
         etk_property_value_int_set(value, etk_container_border_width_get(container));
         break;
      default:
         break;
   }
}

/** @} */
