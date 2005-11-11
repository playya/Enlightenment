/** @file etk_toggle_button.c */
#include "etk_toggle_button.h"
#include <stdlib.h>
#include "etk_signal.h"
#include "etk_signal_callback.h"

/**
 * @addtogroup Etk_Toggle_Button
* @{
 */

enum _Etk_Toggle_Button_Signal_Id
{
   ETK_TOGGLE_BUTTON_TOGGLED_SIGNAL,
   ETK_TOGGLE_BUTTON_NUM_SIGNALS
};

enum _Etk_Toggle_Button_Property_Id
{
   ETK_TOGGLE_BUTTON_ACTIVE_PROPERTY
};

static void _etk_toggle_button_constructor(Etk_Toggle_Button *toggle_button);
static void _etk_toggle_button_property_set(Etk_Object *object, int property_id, Etk_Property_Value *value);
static void _etk_toggle_button_property_get(Etk_Object *object, int property_id, Etk_Property_Value *value);
static void _etk_toggle_button_realize_cb(Etk_Object *object, void *data);
static void _etk_toggle_button_clicked_handler(Etk_Button *button);
static void _etk_toggle_button_toggled_handler(Etk_Toggle_Button *button);

static Etk_Signal *_etk_toggle_button_signals[ETK_TOGGLE_BUTTON_NUM_SIGNALS];

/**************************
 *
 * Implementation
 *
 **************************/

/**
 * @brief Gets the type of an Etk_Toggle_Button
 * @return Returns the type on an Etk_Toggle_Button
 */
Etk_Type *etk_toggle_button_type_get()
{
   static Etk_Type *toggle_button_type = NULL;

   if (!toggle_button_type)
   {
      toggle_button_type = etk_type_new("Etk_Toggle_Button", ETK_BUTTON_TYPE, sizeof(Etk_Toggle_Button), ETK_CONSTRUCTOR(_etk_toggle_button_constructor), NULL);
      
      _etk_toggle_button_signals[ETK_TOGGLE_BUTTON_TOGGLED_SIGNAL] = etk_signal_new("toggled", toggle_button_type, ETK_MEMBER_OFFSET(Etk_Toggle_Button, toggled), etk_marshaller_VOID__VOID, NULL, NULL);
   
      etk_type_property_add(toggle_button_type, "active", ETK_TOGGLE_BUTTON_ACTIVE_PROPERTY, ETK_PROPERTY_BOOL, ETK_PROPERTY_READABLE_WRITABLE, etk_property_value_bool(0));
      
      toggle_button_type->property_set = _etk_toggle_button_property_set;
      toggle_button_type->property_get = _etk_toggle_button_property_get;
   }

   return toggle_button_type;
}

/**
 * @brief Creates a new toggle button
 * @return Returns the new toggle button widget
 */
Etk_Widget *etk_toggle_button_new()
{
   return etk_widget_new(ETK_TOGGLE_BUTTON_TYPE, "theme_group", "toggle_button", "focusable", TRUE, NULL);
}

/**
 * @brief Creates a new toggle button with a label
 * @param label the label
 * @return Returns the new toggle button widget
 */
Etk_Widget *etk_toggle_button_new_with_label(const char *label)
{
   return etk_widget_new(ETK_TOGGLE_BUTTON_TYPE, "theme_group", "toggle_button", "label", label, "focusable", TRUE, NULL);
}

/**
 * @brief Emits the "toggled" signal
 * @param toggle_button a toggle button
 */
void etk_toggle_button_toggled(Etk_Toggle_Button *toggle_button)
{
   if (!toggle_button)
      return;
   etk_signal_emit(_etk_toggle_button_signals[ETK_TOGGLE_BUTTON_TOGGLED_SIGNAL], ETK_OBJECT(toggle_button), NULL);
}

/**
 * @brief Sets the status of the toggle button
 * @param toggle_button a toggle button
 * @param active if @a active != 0, the toggle button will be active
 */
void etk_toggle_button_active_set(Etk_Toggle_Button *toggle_button, Etk_Bool active)
{
   if (toggle_button && toggle_button->active != active)
      etk_button_clicked(ETK_BUTTON(toggle_button));
}

/**************************
 *
 * Etk specific functions
 *
 **************************/

/* Initializes the default values of the toggle button */
static void _etk_toggle_button_constructor(Etk_Toggle_Button *toggle_button)
{
   if (!toggle_button)
      return;

   toggle_button->active = 0;
   toggle_button->toggled = _etk_toggle_button_toggled_handler;
   ETK_BUTTON(toggle_button)->clicked = _etk_toggle_button_clicked_handler;

   etk_signal_connect_after("realize", ETK_OBJECT(toggle_button), ETK_CALLBACK(_etk_toggle_button_realize_cb), NULL);
}

/* Sets the property whose id is "property_id" to the value "value" */
static void _etk_toggle_button_property_set(Etk_Object *object, int property_id, Etk_Property_Value *value)
{
   Etk_Toggle_Button *toggle_button;

   if (!(toggle_button = ETK_TOGGLE_BUTTON(object)) || !value)
      return;

   switch (property_id)
   {
      case ETK_TOGGLE_BUTTON_ACTIVE_PROPERTY:
         etk_toggle_button_active_set(toggle_button, etk_property_value_bool_get(value));
         break;
      default:
         break;
   }
}

/* Gets the value of the property whose id is "property_id" */
static void _etk_toggle_button_property_get(Etk_Object *object, int property_id, Etk_Property_Value *value)
{
   Etk_Toggle_Button *toggle_button;

   if (!(toggle_button = ETK_TOGGLE_BUTTON(object)) || !value)
      return;

   switch (property_id)
   {
      case ETK_TOGGLE_BUTTON_ACTIVE_PROPERTY:
         etk_property_value_bool_set(value, toggle_button->active);
         break;
      default:
         break;
   }
}

/**************************
 *
 * Callbacks and handlers
 *
 **************************/

/* Called when the toggle button is realized */
static void _etk_toggle_button_realize_cb(Etk_Object *object, void *data)
{
   Etk_Toggle_Button *toggle_button;

   if (!(toggle_button = ETK_TOGGLE_BUTTON(object)))
      return;

   etk_toggle_button_toggled(toggle_button);
}

/* Called when the toggle button is clicked */
static void _etk_toggle_button_clicked_handler(Etk_Button *button)
{
   Etk_Toggle_Button *toggle_button;

   if (!(toggle_button = ETK_TOGGLE_BUTTON(button)))
      return;

   etk_widget_theme_object_signal_emit(ETK_WIDGET(toggle_button), "clicked");
   toggle_button->active = !toggle_button->active;
   etk_toggle_button_toggled(toggle_button);
   etk_object_notify(ETK_OBJECT(toggle_button), "active");
}

/* Default handler for "toggled" signal */
static void _etk_toggle_button_toggled_handler(Etk_Toggle_Button *button)
{
   Etk_Toggle_Button *toggle_button;

   if (!(toggle_button = ETK_TOGGLE_BUTTON(button)))
      return;
   
   if (toggle_button->active)
      etk_widget_theme_object_signal_emit(ETK_WIDGET(toggle_button), "activated");
   else
      etk_widget_theme_object_signal_emit(ETK_WIDGET(toggle_button), "desactivated");
}

/** @} */
