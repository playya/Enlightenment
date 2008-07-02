#ifndef _EDJE_EDITOR_PROGRAM_H_
#define _EDJE_EDITOR_PROGRAM_H_


/* program frame objects */
Etk_Widget *UI_ProgramEntry;
Etk_Widget *UI_ProgramEntryImage;
Etk_Widget *UI_RunProgButton;
Etk_Widget *UI_SignalEntry;
Etk_Widget *UI_ActionComboBox;
Etk_Widget *UI_TransiComboBox;
Etk_Widget *UI_TransiLabel;
Etk_Widget *UI_DurationSpinner;
Etk_Widget *UI_AfterEntry;
Etk_Widget *UI_SourceEntry;
Etk_Widget *UI_TargetEntry;
Etk_Widget *UI_TargetLabel;
Etk_Widget *UI_Param1Label;
Etk_Widget *UI_Param1Entry;
Etk_Widget *UI_Param1Spinner;
Etk_Widget *UI_Param1Label;
Etk_Widget *UI_Param2Entry;
Etk_Widget *UI_Param2Spinner;
Etk_Widget *UI_Param2Label;
Etk_Widget *UI_DelayFromSpinner;
Etk_Widget *UI_DelayRangeSpinner;


Etk_Widget* program_frame_create(void);
void        program_frame_update(void);
void        program_source_combo_populate(void);
void        program_signal_combo_populate(void);


Etk_Bool _program_ActionComboBox_changed_cb         (Etk_Combobox *combobox, void *data);
Etk_Bool _program_EntryImage_clicked_cb             (Etk_Object *object, void *data);
Etk_Bool _program_Entry_key_down_cb                 (Etk_Object *object, Etk_Event_Key_Down *event, void *data);
Etk_Bool _program_SourceEntry_text_changed_cb       (Etk_Object *object, void *data);
Etk_Bool _program_SourceEntry_item_changed_cb       (Etk_Combobox_Entry *combo, void *data);
Etk_Bool _program_SignalEntry_text_changed_cb       (Etk_Object *object, void *data);
Etk_Bool _program_SignalEntry_item_changed_cb       (Etk_Combobox_Entry *combo, void *data);
Etk_Bool _program_DelaySpinners_value_changed_cb    (Etk_Range *range, double value, void *data);
Etk_Bool _program_TargetEntry_text_changed_cb       (Etk_Object *object, void *data);
Etk_Bool _program_Param1Entry_text_changed_cb       (Etk_Object *object, void *data);
Etk_Bool _program_Param2Entry_text_changed_cb       (Etk_Object *object, void *data);
Etk_Bool _program_Param1Spinner_value_changed_cb    (Etk_Range *range, double value, void *data);
Etk_Bool _program_Param2Spinner_value_changed_cb    (Etk_Range *range, double value, void *data);
Etk_Bool _program_TransitionComboBox_changed_cb     (Etk_Combobox *combobox, void *data);
Etk_Bool _program_DurationSpinner_value_changed_cb  (Etk_Range *range, double value, void *data);
Etk_Bool _program_AfterEntry_text_changed_cb        (Etk_Object *object, void *data);


#endif
