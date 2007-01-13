#include "main.h"


Etk_Widget *UI_MainWin;
Etk_Widget *UI_PartsTree;
Etk_Widget *UI_GroupComboBox;
Etk_Widget *UI_ColorPickerWin;
Etk_Widget *UI_StateEntry;
Etk_Widget *UI_GroupNameEntry;
Etk_Widget *UI_StateIndexSpinner;
Evas_Object *RectColorObject;
Evas_Object *TextColorObject;
Evas_Object *ShadowColorObject;
Evas_Object *OutlineColorObject;
Etk_Widget *UI_ColorWin;
Etk_Widget *UI_ColorPicker;
Etk_Widget *UI_DescriptionFrame;
Etk_Widget *UI_PositionFrame;
Etk_Widget *UI_RectFrame;
Etk_Widget *UI_ImageFrame;
Etk_Widget *UI_TextFrame;
Etk_Widget *UI_TextEntry;
Etk_Widget *UI_FontComboBox;
Etk_Widget *UI_ImageComboBox;
Etk_Widget *UI_FontSizeSpinner;
Etk_Widget *UI_EffectComboBox;
Etk_Widget *UI_PartFrame;
Etk_Widget *UI_GroupFrame;
Etk_Widget *UI_PartNameEntry;
Etk_Widget *UI_PartTypeComboBox;
Etk_Widget *UI_ColorAlphaSlider;
Etk_Widget *UI_AlphaLabel;
Etk_Widget *UI_ImageAlphaSlider;
Etk_Widget *UI_TextAlphaSlider;
Etk_Widget *UI_Rel1XSpinner;
Etk_Widget *UI_Rel1XOffsetSpinner;
Etk_Widget *UI_Rel1YSpinner;
Etk_Widget *UI_Rel1YOffsetSpinner;
Etk_Widget *UI_Rel2XSpinner;
Etk_Widget *UI_Rel2XOffsetSpinner;
Etk_Widget *UI_Rel2YSpinner;
Etk_Widget *UI_Rel2YOffsetSpinner;
Etk_Widget *UI_AddMenu;
Etk_Widget *UI_RemoveMenu;
Etk_Widget *UI_Rel1ToXComboBox;
Etk_Widget *UI_Rel1ToYComboBox;
Etk_Widget *UI_Rel2ToXComboBox;
Etk_Widget *UI_Rel2ToYComboBox;
Etk_Widget *UI_BorderTopSpinner;
Etk_Widget *UI_BorderLeftSpinner;
Etk_Widget *UI_BorderBottomSpinner;
Etk_Widget *UI_BorderRightSpinner;
Etk_Widget *UI_FileChooser;
Etk_Widget *UI_FileChooserDialog;
Etk_Widget *UI_PlayDialog;
Etk_Widget *UI_PlayTextView;
Etk_Widget *UI_FilechooserFileNameEntry;
Etk_Widget *UI_tree_vbox;
Etk_Widget *UI_AlertDialog;
Etk_Widget *UI_GroupMinWSpinner;
Etk_Widget *UI_GroupMinHSpinner;
Etk_Widget *UI_GroupMaxWSpinner;
Etk_Widget *UI_GroupMaxHSpinner;
Etk_Widget *UI_ImageAddButton;
Etk_Widget *UI_FontAddButton;

void     create_main_window      (void);
void     UpdatePartFrame         (void);
void     UpdateRectFrame         (void);
void     UpdateDescriptionFrame  (void);
void     UpdatePositionFrame     (void);
void     UpdateComboPositionFrame(void);
void     UpdateImageFrame        (void);
void     UpdateTextFrame         (void);
void     UpdateWindowTitle       (void);




void     ShowFilechooser         (int FileChooserType);
void     ShowAlert               (char* text);


void     PopulateTree            (void);
void     AddGroupToTree          (Engrave_Group* group);
void     AddPartToTree           (Engrave_Part* part);
void     AddStateToTree          (Engrave_Part_State* state);
void     PopulateRelComboBoxes	(void);
void     PopulateImagesComboBox	(void);
void     PopulateFontsComboBox	(void);
void     UpdateGroupFrame        (void);
