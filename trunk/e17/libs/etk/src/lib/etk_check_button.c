/** @file etk_check_button.c */
#include "etk_check_button.h"
#include <stdlib.h>

/**
 * @addtogroup Etk_Check_Button
 * @{
 */

/**************************
 *
 * Implementation
 *
 **************************/

/**
 * @internal
 * @brief Gets the type of an Etk_Check_Button
 * @return Returns the type of an Etk_Check_Button
 */
Etk_Type *etk_check_button_type_get()
{
   static Etk_Type *check_button_type = NULL;

   if (!check_button_type)
   {
      check_button_type = etk_type_new("Etk_Check_Button", ETK_TOGGLE_BUTTON_TYPE,
         sizeof(Etk_Check_Button), NULL, NULL);
   }

   return check_button_type;
}

/**
 * @brief Creates a new check button
 * @return Returns the new check button widget
 */
Etk_Widget *etk_check_button_new()
{
   return etk_widget_new(ETK_CHECK_BUTTON_TYPE, "theme_group", "check_button",
      "focusable", ETK_TRUE, "xalign", 0.0, NULL);
}

/**
 * @brief Creates a new check button with a label
 * @param label the label
 * @return Returns the new check button widget
 */
Etk_Widget *etk_check_button_new_with_label(const char *label)
{
   return etk_widget_new(ETK_CHECK_BUTTON_TYPE, "theme_group", "check_button",
      "label", label, "focusable", ETK_TRUE, "xalign", 0.0, NULL);
}

/** @} */

/**************************
 *
 * Documentation
 *
 **************************/

/**
 * @addtogroup Etk_Check_Button
 *
 * @image html widgets/check_button.png
 * \par Object Hierarchy:
 * - Etk_Object
 *   - Etk_Widget
 *     - Etk_Container
 *       - Etk_Bin
 *         - Etk_Button
 *           - Etk_Toggle_Button
 *             - Etk_Check_Button
 */
