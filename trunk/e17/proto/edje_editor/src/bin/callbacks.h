
/* Called when the window is destroyed */
void etk_main_quit_cb(void *data);

//void on_GroupComboBox_changed        (Etk_Combobox *combobox, void *data);
void on_GroupSpinner_value_changed     (Etk_Range *range, double value,
                                       void *data);
void on_canvas_geometry_changed        (Etk_Object *canvas, 
                                       const char *property_name, void *data);
void on_PartsTree_row_selected         (Etk_Object *object, Etk_Tree_Row *row,
                                       void *data);
void on_ColorCanvas_click              (void *data, Evas *e, Evas_Object *obj,
                                       void *event_info);
void on_ColorDialog_change             (Etk_Object *object, void *data);
void on_PartNameEntry_text_changed     (Etk_Object *object, void *data);
void on_PartEventsCheck_toggled        (Etk_Object *object, void *data);
void on_StateEntry_text_changed        (Etk_Object *object, void *data);
void on_AllButton_click                (Etk_Button *button, void *data);
void on_AddMenu_item_activated         (Etk_Object *object, void *data);
void on_RemoveMenu_item_activated      (Etk_Object *object, void *data);
void on_RelToComboBox_changed          (Etk_Combobox *combobox, void *data);
void on_TextEntry_text_changed         (Etk_Object *object, void *data);
void on_FileChooser_response           (Etk_Dialog *dialog, int response_id,
                                       void *data);
void on_BorderSpinner_value_changed    (Etk_Range *range, double value, void *data);
void on_RelSpinner_value_changed       (Etk_Range *range, double value, void *data);
void on_RelOffsetSpinner_value_changed	(Etk_Range *range, double value, void *data);
void on_FontComboBox_changed           (Etk_Combobox *combobox, void *data);
void on_FontSizeSpinner_value_changed  (Etk_Range *range, double value, void *data);
void on_FileChooser_row_selected       (Etk_Object *object, Etk_Tree_Row *row,
                                       void *data);
void on_ColorCanvas_realize            (Etk_Widget *canvas, void *data);
void on_ImageAlphaSlider_value_changed (Etk_Object *object, double value, void *data);
void on_ImageComboBox_changed          (Etk_Combobox *combobox, void *data);
void on_ImageTweenList_row_selected    (Etk_Object *object, Etk_Tree_Row *row, void *data);
void on_EffectComboBox_changed         (Etk_Combobox *combobox, void *data);
void on_AlertDialog_response           (Etk_Dialog *dialog, int response_id, void *data);
void on_StateIndexSpinner_value_changed(Etk_Range *range, double value, void *data);
void on_GroupNameEntry_text_changed    (Etk_Object *object, void *data);
void on_AspectSpinner_value_changed    (Etk_Range *range, double value, void *data);
void on_AspectComboBox_changed         (Etk_Combobox *combobox, void *data);
void on_StateMinMaxSpinner_value_changed(Etk_Range *range, double value, void *data);
void on_ActionComboBox_changed         (Etk_Combobox *combobox, void *data);
void on_ProgramEntry_text_changed      (Etk_Object *object, void *data);
void on_SourceEntry_text_changed       (Etk_Object *object, void *data);
void on_SourceEntry_item_changed       (Etk_Combobox_Entry *combo, void *data);
void on_SignalEntry_text_changed       (Etk_Object *object, void *data);
void on_DelaySpinners_value_changed    (Etk_Range *range, double value, void *data);
void on_TargetEntry_text_changed       (Etk_Object *object, void *data);
void on_Param1Entry_text_changed       (Etk_Object *object, void *data);
void on_Param2Entry_text_changed       (Etk_Object *object, void *data);
void on_Param1Spinner_value_changed    (Etk_Range *range, double value, void *data);
void on_TransitionComboBox_changed     (Etk_Combobox *combobox, void *data);
void on_DurationSpinner_value_changed  (Etk_Range *range, double value, void *data);
void on_AfterEntry_text_changed        (Etk_Object *object, void *data);
