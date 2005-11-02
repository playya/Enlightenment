/** @file etk_button.h */
#ifndef _ETK_BUTTON_H_
#define _ETK_BUTTON_H_

#include "etk_bin.h"
#include "etk_types.h"
#include "etk_stock.h"

/**
 * @defgroup Etk_Button Etk_Button
 * @{
 */

/** @brief Gets the type of a button */
#define ETK_BUTTON_TYPE       (etk_button_type_get())
/** @brief Casts the object to an Etk_Button */
#define ETK_BUTTON(obj)       (ETK_OBJECT_CAST((obj), ETK_BUTTON_TYPE, Etk_Button))
/** @brief Checks if the object is an Etk_Button */
#define ETK_IS_BUTTON(obj)    (ETK_OBJECT_CHECK_TYPE((obj), ETK_BUTTON_TYPE))

struct _Etk_Button
{
   /* private: */
   /* Inherit from Etk_Bin */
   Etk_Bin bin;

   Etk_Widget *alignment;
   Etk_Widget *hbox;
   Etk_Image *image;
   Etk_Widget *label;
   
   void (*pressed)(Etk_Button *button);
   void (*released)(Etk_Button *button);
   void (*clicked)(Etk_Button *button);

   float xalign;
   float yalign;
};

Etk_Type *etk_button_type_get();
Etk_Widget *etk_button_new();
Etk_Widget *etk_button_new_with_label(const char *label);
Etk_Widget *etk_button_new_from_stock(Etk_Stock_Id stock_id);

void etk_button_pressed(Etk_Button *button);
void etk_button_released(Etk_Button *button);
void etk_button_clicked(Etk_Button *button);

void etk_button_label_set(Etk_Button *button, const char *label);
const char *etk_button_label_get(Etk_Button *button);

void etk_button_image_set(Etk_Button *button, Etk_Image *image);
Etk_Image *etk_button_image_get(Etk_Button *button);

void etk_button_alignment_set(Etk_Button *button, float xalign, float yalign);
void etk_button_alignment_get(Etk_Button *button, float *xalign, float *yalign);

/** @} */

#endif
