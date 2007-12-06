#include "main.h"

#define TREE_COL_NAME 0
#define TREE_COL_VIS 1
#define TREE_COL_TYPE 2
#define TREE_COL_PARENT 3

#define COL_NAME   etk_tree_nth_col_get(ETK_TREE(UI_PartsTree), TREE_COL_NAME)
#define COL_VIS    etk_tree_nth_col_get(ETK_TREE(UI_PartsTree), TREE_COL_VIS)
#define COL_TYPE   etk_tree_nth_col_get(ETK_TREE(UI_PartsTree), TREE_COL_TYPE)
#define COL_PARENT etk_tree_nth_col_get(ETK_TREE(UI_PartsTree), TREE_COL_PARENT)

Evas *UI_evas;
Ecore_Evas *UI_ecore_MainWin;
//Etk_Widget *UI_MainWin;
Etk_Widget *UI_Toolbar;
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
Etk_Widget *UI_TextEntry;
Etk_Widget *UI_FontComboBox;
Etk_Widget *UI_ImageTweenList;
Etk_Widget *UI_ImageTweenRadio;
Etk_Widget *UI_ImageNormalRadio;
Etk_Widget *UI_ImageComboBox;
Etk_Widget *UI_ImageTweenVBox;
Etk_Widget *UI_DeleteTweenButton;
Etk_Widget *UI_MoveDownTweenButton;
Etk_Widget *UI_MoveUpTweenButton;
Etk_Widget *UI_FontSizeSpinner;
Etk_Widget *UI_EffectComboBox;
Etk_Widget *UI_PartNameEntry;
Etk_Widget *UI_PartTypeComboBox;
Etk_Widget *UI_ImageAlphaSlider;
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
Etk_Widget *UI_OptionsMenu;
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
Etk_Widget *UI_PlayTextView;
Etk_Widget *UI_FilechooserSaveButton;
Etk_Widget *UI_FilechooserLoadButton;
Etk_Widget *UI_tree_vbox;
Etk_Widget *UI_AlertDialog;
Etk_Widget *UI_GroupMinWSpinner;
Etk_Widget *UI_GroupMinHSpinner;
Etk_Widget *UI_GroupMaxWSpinner;
Etk_Widget *UI_GroupMaxHSpinner;
Etk_Widget *UI_ImageAddButton;
Etk_Widget *UI_FontAddButton;
Etk_Widget *UI_FontAlignVSpinner;
Etk_Widget *UI_FontAlignHSpinner;
Etk_Widget *UI_AspectMinSpinner;
Etk_Widget *UI_AspectMaxSpinner;
Etk_Widget *UI_AspectComboBox;
Etk_Widget *UI_StateMinWSpinner;
Etk_Widget *UI_StateMinHSpinner;
Etk_Widget *UI_StateMaxWSpinner;
Etk_Widget *UI_StateMaxHSpinner;
Etk_Widget *UI_StateAlignVSpinner;
Etk_Widget *UI_StateAlignHSpinner;
Etk_Widget *UI_ProgramEntry;
Etk_Widget *UI_SignalEntry;
Etk_Widget *UI_ScriptBox;
Etk_Widget *UI_ScriptSaveButton;
Etk_Widget *UI_ActionComboBox;
Etk_Widget *UI_TransiComboBox;
Etk_Widget *UI_TransiLabel;
Etk_Widget *UI_DurationSpinner;
Etk_Widget *UI_DurationLabel;
Etk_Widget *UI_AfterEntry;
Etk_Widget *UI_SourceEntry;
Etk_Widget *UI_TargetEntry;
Etk_Widget *UI_TargetLabel;
Etk_Widget *UI_Param1Label;
Etk_Widget *UI_Param1Entry;
Etk_Widget *UI_Param1Spinner;
Etk_Widget *UI_Param1Label;
Etk_Widget *UI_Param2Entry;
Etk_Widget *UI_Param2Label;
Etk_Widget *UI_DelayFromSpinner;
Etk_Widget *UI_DelayRangeSpinner;
Etk_Widget *UI_CurrentGroupSizeLabel;
Etk_Widget *UI_PartEventsCheck;

Etk_Widget *UI_PartsTreeEmbed;
Etk_Widget *UI_GroupEmbed;
Etk_Widget *UI_PartEmbed;
Etk_Widget *UI_DescriptionEmbed;
Etk_Widget *UI_RectEmbed;
Etk_Widget *UI_TextEmbed;
Etk_Widget *UI_PositionEmbed;
Etk_Widget *UI_ProgramEmbed;
Etk_Widget *UI_ScriptEmbed;
Etk_Widget *UI_ImageEmbed;
Etk_Widget *UI_ToolbarEmbed;

//Evas_Object *UI_PartEdje;

Evas_Object *edje_ui;

#if TEST_DIRECT_EDJE

Etk_Widget *UI_GroupsComboBox;

void     AddGroupToTree2          (char *name);
void     AddPartToTree2(char *part_name);
void     AddStateToTree2(char *part_name, char *state_name);
#endif

void     create_main_window      (void);
void     ecore_resize_callback   (Ecore_Evas *ecore_evas);
void     _embed_position_set     (void *position_data, int *x, int *y);
void     PopulateTree            (void);
void     AddGroupToTree          (Engrave_Group* group);
void     AddPartToTree           (Engrave_Part* part, int place_after, Engrave_Part* after);
void     AddStateToTree          (Engrave_Part_State* state);
void     AddProgramToTree        (Engrave_Program* prog);
void     PopulateRelComboBoxes	(void);
void     PopulateImagesComboBox	(void);
void     PopulateFontsComboBox	(void);
void     PopulateSourceComboBox	(void);
void     UpdateGroupFrame        (void);
void     UpdatePositionFrame     (void);
void     UpdateComboPositionFrame(void);
void     UpdateImageFrame        (void);
void     UpdateTextFrame         (void);
void     UpdatePartFrame         (void);
void     UpdateRectFrame         (void);
void     UpdateDescriptionFrame  (void);
void     UpdateProgFrame         (void);
void     UpdateScriptFrame       (void);
void     UpdateWindowTitle       (void);
void     ShowFilechooser         (int FileChooserType);
void     ShowAlert               (char* text);
