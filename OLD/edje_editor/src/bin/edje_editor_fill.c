/*  Copyright (C) 2006-2008 Davide Andreoli (see AUTHORS)
 *
 *  This file is part of Edje_editor.
 *  Edje_editor is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Edje_editor is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Edje_editor.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <Etk.h>
#include <Edje.h>
#include <Edje_Edit.h>
#include "main.h"


Etk_Widget*
fill_frame_create(void)
{
   Etk_Widget *label;
   Etk_Widget *table;

   table = etk_table_new(8, 2, ETK_TABLE_NOT_HOMOGENEOUS);

   label = etk_label_new("<b>Origin X </b>");
   etk_table_attach_default(ETK_TABLE(table), label, 0, 0, 0, 0);

   //UI_FillRelXSpinner
   UI_FillRelXSpinner = etk_spinner_new(-100.0, 100.0, 0.0, 0.01, 0.1);
   etk_spinner_digits_set(ETK_SPINNER(UI_FillRelXSpinner), 2);
   etk_widget_size_request_set(UI_FillRelXSpinner, 45, 20);
   etk_table_attach_default(ETK_TABLE(table), UI_FillRelXSpinner, 1, 1, 0, 0);

   label = etk_label_new("+");
   etk_table_attach_default(ETK_TABLE(table), label, 2, 2, 0, 0);

   //UI_FillOffsetXSpinner
   UI_FillOffsetXSpinner = etk_spinner_new(-2000, 2000, 0, 1, 10);
   etk_widget_size_request_set(UI_FillOffsetXSpinner, 45, 20);
   etk_table_attach_default(ETK_TABLE(table), UI_FillOffsetXSpinner, 3, 3, 0, 0);

   label = etk_label_new("<b> Y </b>");
   etk_table_attach_default(ETK_TABLE(table), label, 4, 4, 0, 0);

   //UI_FillRelYSpinner
   UI_FillRelYSpinner = etk_spinner_new(-100.0, 100.0, 0.0, 0.01, 0.1);
   etk_spinner_digits_set(ETK_SPINNER(UI_FillRelYSpinner), 2);
   etk_widget_size_request_set(UI_FillRelYSpinner, 45, 20);
   etk_table_attach_default(ETK_TABLE(table), UI_FillRelYSpinner, 5, 5, 0, 0);

   label = etk_label_new("+");
   etk_table_attach_default(ETK_TABLE(table), label, 6, 6, 0, 0);

   //UI_FillOffsetYSpinner
   UI_FillOffsetYSpinner = etk_spinner_new(-2000, 2000, 0, 1, 10);
   etk_widget_size_request_set(UI_FillOffsetYSpinner, 45, 20);
   etk_table_attach_default(ETK_TABLE(table), UI_FillOffsetYSpinner, 7, 7, 0, 0);


   label = etk_label_new("<b>Size     X </b>");
   etk_table_attach_default(ETK_TABLE(table), label, 0, 0, 1, 1);

   //UI_FillSizeRelXSpinner
   UI_FillSizeRelXSpinner = etk_spinner_new(-100.0, 100.0, 0.0, 0.01, 0.1);
   etk_spinner_digits_set(ETK_SPINNER(UI_FillSizeRelXSpinner), 2);
   etk_widget_size_request_set(UI_FillSizeRelXSpinner, 45, 20);
   etk_table_attach_default(ETK_TABLE(table), UI_FillSizeRelXSpinner, 1, 1, 1, 1);

   label = etk_label_new("+");
  etk_table_attach_default(ETK_TABLE(table), label, 2, 2, 1, 1);

   //UI_FillSizeOffsetXSpinner
   UI_FillSizeOffsetXSpinner = etk_spinner_new(-2000, 2000, 0, 1, 10);
   etk_widget_size_request_set(UI_FillSizeOffsetXSpinner, 45, 20);
   etk_table_attach_default(ETK_TABLE(table), UI_FillSizeOffsetXSpinner, 3, 3, 1, 1);

   label = etk_label_new("<b> Y </b>");
   etk_table_attach_default(ETK_TABLE(table), label, 4, 4, 1, 1);

   //UI_FillSizeRelYSpinner
   UI_FillSizeRelYSpinner = etk_spinner_new(-100.0, 100.0, 0.0, 0.01, 0.1);
   etk_spinner_digits_set(ETK_SPINNER(UI_FillSizeRelYSpinner), 2);
   etk_widget_size_request_set(UI_FillSizeRelYSpinner, 45, 20);
   etk_table_attach_default(ETK_TABLE(table), UI_FillSizeRelYSpinner, 5, 5, 1, 1);

   label = etk_label_new("+");
   etk_table_attach_default(ETK_TABLE(table), label, 6, 6, 1, 1);

   //UI_FillSizeOffsetYSpinner
   UI_FillSizeOffsetYSpinner = etk_spinner_new(-2000, 2000, 0, 1, 10);
   etk_widget_size_request_set(UI_FillSizeOffsetYSpinner, 45, 20);
   etk_table_attach_default(ETK_TABLE(table), UI_FillSizeOffsetYSpinner, 7, 7, 1, 1);
   
   etk_signal_connect("value-changed", ETK_OBJECT(UI_FillRelXSpinner),
                      ETK_CALLBACK(_fill_spinners_value_changed_cb),
                      (void *)REL1X_SPINNER);
   etk_signal_connect("value-changed", ETK_OBJECT(UI_FillRelYSpinner),
                      ETK_CALLBACK(_fill_spinners_value_changed_cb),
                      (void *)REL1Y_SPINNER);
   etk_signal_connect("value-changed", ETK_OBJECT(UI_FillOffsetXSpinner),
                      ETK_CALLBACK(_fill_spinners_value_changed_cb),
                      (void *)REL1XO_SPINNER);
   etk_signal_connect("value-changed", ETK_OBJECT(UI_FillOffsetYSpinner),
                      ETK_CALLBACK(_fill_spinners_value_changed_cb),
                      (void *)REL1YO_SPINNER);

   etk_signal_connect("value-changed", ETK_OBJECT(UI_FillSizeRelXSpinner),
                      ETK_CALLBACK(_fill_size_spinners_value_changed_cb),
                      (void *)REL1X_SPINNER);
   etk_signal_connect("value-changed", ETK_OBJECT(UI_FillSizeRelYSpinner),
                      ETK_CALLBACK(_fill_size_spinners_value_changed_cb),
                      (void *)REL1Y_SPINNER);
   etk_signal_connect("value-changed", ETK_OBJECT(UI_FillSizeOffsetXSpinner),
                      ETK_CALLBACK(_fill_size_spinners_value_changed_cb),
                      (void *)REL1XO_SPINNER);
   etk_signal_connect("value-changed", ETK_OBJECT(UI_FillSizeOffsetYSpinner),
                      ETK_CALLBACK(_fill_size_spinners_value_changed_cb),
                      (void *)REL1YO_SPINNER);

   return table;
}
void
fill_frame_update(void)
{
   if (!etk_string_length_get(Cur.part)) return;
   if (!etk_string_length_get(Cur.state)) return;

   //Block Signal
   etk_signal_block("value-changed", ETK_OBJECT(UI_FillRelXSpinner),
                    ETK_CALLBACK(_fill_spinners_value_changed_cb),
                    (void *)REL1X_SPINNER);
   etk_signal_block("value-changed", ETK_OBJECT(UI_FillRelYSpinner),
                    ETK_CALLBACK(_fill_spinners_value_changed_cb),
                    (void *)REL1Y_SPINNER);
   etk_signal_block("value-changed", ETK_OBJECT(UI_FillOffsetXSpinner),
                    ETK_CALLBACK(_fill_spinners_value_changed_cb),
                    (void *)REL1XO_SPINNER);
   etk_signal_block("value-changed", ETK_OBJECT(UI_FillOffsetYSpinner),
                    ETK_CALLBACK(_fill_spinners_value_changed_cb),
                    (void *)REL1YO_SPINNER);
   etk_signal_block("value-changed", ETK_OBJECT(UI_FillSizeRelXSpinner),
                    ETK_CALLBACK(_fill_size_spinners_value_changed_cb),
                    (void *)REL1X_SPINNER);
   etk_signal_block("value-changed", ETK_OBJECT(UI_FillSizeRelYSpinner),
                    ETK_CALLBACK(_fill_size_spinners_value_changed_cb),
                    (void *)REL1Y_SPINNER);
   etk_signal_block("value-changed", ETK_OBJECT(UI_FillSizeOffsetXSpinner),
                    ETK_CALLBACK(_fill_size_spinners_value_changed_cb),
                    (void *)REL1XO_SPINNER);
   etk_signal_block("value-changed", ETK_OBJECT(UI_FillSizeOffsetYSpinner),
                    ETK_CALLBACK(_fill_size_spinners_value_changed_cb),
                    (void *)REL1YO_SPINNER);

   //Update origin spinners
   etk_range_value_set(ETK_RANGE(UI_FillRelXSpinner),
      edje_edit_state_fill_origin_relative_x_get(edje_o, Cur.part->string,
                                                 Cur.state->string));
   etk_range_value_set(ETK_RANGE(UI_FillRelYSpinner),
      edje_edit_state_fill_origin_relative_y_get(edje_o, Cur.part->string,
                                                 Cur.state->string));
   etk_range_value_set(ETK_RANGE(UI_FillOffsetXSpinner),
      edje_edit_state_fill_origin_offset_x_get(edje_o, Cur.part->string,
                                               Cur.state->string));
   etk_range_value_set(ETK_RANGE(UI_FillOffsetYSpinner),
      edje_edit_state_fill_origin_offset_y_get(edje_o, Cur.part->string,
                                               Cur.state->string));

   //Update size spinners
   etk_range_value_set(ETK_RANGE(UI_FillSizeRelXSpinner),
      edje_edit_state_fill_size_relative_x_get(edje_o, Cur.part->string,
                                                 Cur.state->string));
   etk_range_value_set(ETK_RANGE(UI_FillSizeRelYSpinner),
      edje_edit_state_fill_size_relative_y_get(edje_o, Cur.part->string,
                                                 Cur.state->string));
   etk_range_value_set(ETK_RANGE(UI_FillSizeOffsetXSpinner),
      edje_edit_state_fill_size_offset_x_get(edje_o, Cur.part->string,
                                               Cur.state->string));
   etk_range_value_set(ETK_RANGE(UI_FillSizeOffsetYSpinner),
      edje_edit_state_fill_size_offset_y_get(edje_o, Cur.part->string,
                                               Cur.state->string));

   //UnBlock Signals
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_FillRelXSpinner),
                      ETK_CALLBACK(_fill_spinners_value_changed_cb),
                      (void *)REL1X_SPINNER);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_FillRelYSpinner),
                      ETK_CALLBACK(_fill_spinners_value_changed_cb),
                      (void *)REL1Y_SPINNER);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_FillOffsetXSpinner),
                      ETK_CALLBACK(_fill_spinners_value_changed_cb),
                      (void *)REL1XO_SPINNER);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_FillOffsetYSpinner),
                      ETK_CALLBACK(_fill_spinners_value_changed_cb),
                      (void *)REL1YO_SPINNER);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_FillSizeRelXSpinner),
                      ETK_CALLBACK(_fill_size_spinners_value_changed_cb),
                      (void *)REL1X_SPINNER);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_FillSizeRelYSpinner),
                      ETK_CALLBACK(_fill_size_spinners_value_changed_cb),
                      (void *)REL1Y_SPINNER);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_FillSizeOffsetXSpinner),
                      ETK_CALLBACK(_fill_size_spinners_value_changed_cb),
                      (void *)REL1XO_SPINNER);
   etk_signal_unblock("value-changed", ETK_OBJECT(UI_FillSizeOffsetYSpinner),
                      ETK_CALLBACK(_fill_size_spinners_value_changed_cb),
                      (void *)REL1YO_SPINNER);
}

Etk_Bool
_fill_spinners_value_changed_cb(Etk_Range *range, double value, void *data)
{
   printf("Value Changed Signal on Fill Origin Spinners EMITTED (value: %f)\n",etk_range_value_get(range));

   if (!etk_string_length_get(Cur.state) || !etk_string_length_get(Cur.part))
      return ETK_TRUE;

   switch ((int)(long)data)
   {
      case REL1X_SPINNER:
         edje_edit_state_fill_origin_relative_x_set(edje_o, 
                                 Cur.part->string, Cur.state->string,
                                 etk_range_value_get(range));
         break;
      case REL1Y_SPINNER:
         edje_edit_state_fill_origin_relative_y_set(edje_o, 
                                 Cur.part->string, Cur.state->string,
                                 etk_range_value_get(range));
         break;
      case REL1XO_SPINNER:
         edje_edit_state_fill_origin_offset_x_set(edje_o, 
                                 Cur.part->string, Cur.state->string,
                                 etk_range_value_get(range));
         break;
      case REL1YO_SPINNER:
         edje_edit_state_fill_origin_offset_y_set(edje_o, 
                                 Cur.part->string, Cur.state->string,
                                 etk_range_value_get(range));
         break;
      default:
         break;
   }

   return ETK_TRUE;
}
Etk_Bool
_fill_size_spinners_value_changed_cb(Etk_Range *range, double value, void *data)
{
   printf("Value Changed Signal on Fill Size Spinners EMITTED (value: %f)\n",etk_range_value_get(range));

   if (!etk_string_length_get(Cur.state) || !etk_string_length_get(Cur.part))
      return ETK_TRUE;

   switch ((int)(long)data)
   {
      case REL1X_SPINNER:
         edje_edit_state_fill_size_relative_x_set(edje_o, 
                                 Cur.part->string, Cur.state->string,
                                 etk_range_value_get(range));
         break;
      case REL1Y_SPINNER:
         edje_edit_state_fill_size_relative_y_set(edje_o, 
                                 Cur.part->string, Cur.state->string,
                                 etk_range_value_get(range));
         break;
      case REL1XO_SPINNER:
         edje_edit_state_fill_size_offset_x_set(edje_o, 
                                 Cur.part->string, Cur.state->string,
                                 etk_range_value_get(range));
         break;
      case REL1YO_SPINNER:
         edje_edit_state_fill_size_offset_y_set(edje_o, 
                                 Cur.part->string, Cur.state->string,
                                 etk_range_value_get(range));
         break;
      default:
         break;
   }
   return ETK_TRUE;
}
