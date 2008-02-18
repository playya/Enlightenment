#ifndef _EDJE_EDITOR_MAIN_H_
#define _EDJE_EDITOR_MAIN_H_

#include <Edje.h>
#include <Ecore_Data.h>
#include <Ecore_File.h>
#include <Ecore_Evas.h>

/* DEFINES */
#define FAKEWIN_BORDER_TOP    16
#define FAKEWIN_BORDER_LEFT   7
#define FAKEWIN_BORDER_RIGHT  4
#define FAKEWIN_BORDER_BOTTOM 4

#define USE_GL_ENGINE 0
#define DEBUG_MODE 0

#if DEBUG_MODE
   #define TREE_WIDTH 365
#else
   #define TREE_WIDTH 265
#endif

#undef FREE
#define FREE(val) \
{ \
  free(val); val = NULL; \
}

#undef IF_FREE
#define IF_FREE(val) \
{ \
  if (val) FREE(val) \
  val = NULL; \
}

//All the enum used are declared here
enum various
{
   FILECHOOSER_OPEN,
   FILECHOOSER_IMAGE,
   FILECHOOSER_FONT,
   FILECHOOSER_SAVE_EDC,
   FILECHOOSER_SAVE_EDJ,
   TOOLBAR_NEW,
   TOOLBAR_OPEN,
   TOOLBAR_ADD,
   TOOLBAR_REMOVE,
   TOOLBAR_OPTIONS,
   TOOLBAR_DEBUG,
   TOOLBAR_SAVE,
   TOOLBAR_SAVE_EDC,
   TOOLBAR_SAVE_EDJ,
   TOOLBAR_PLAY,
   TOOLBAR_MOVE_UP,
   TOOLBAR_MOVE_DOWN,
   TOOLBAR_OPTION_BG1,
   TOOLBAR_OPTION_BG2,
   TOOLBAR_OPTION_BG3,
   TOOLBAR_OPTION_BG4,
   TOOLBAR_IMAGE_FILE_ADD,
   TOOLBAR_FONT_FILE_ADD,
   COLOR_OBJECT_RECT,
   COLOR_OBJECT_TEXT,
   COLOR_OBJECT_SHADOW,
   COLOR_OBJECT_OUTLINE,
   NEW_IMAGE,
   NEW_RECT,
   NEW_TEXT,
   NEW_PROG,
   NEW_DESC,
   NEW_GROUP,
   REMOVE_DESCRIPTION,
   REMOVE_PART,
   REMOVE_GROUP,
   REMOVE_PROG,
   REL1X_SPINNER,
   REL1Y_SPINNER,
   REL2X_SPINNER,
   REL2Y_SPINNER,
   MINW_SPINNER,
   MAXW_SPINNER,
   MINH_SPINNER,
   MAXH_SPINNER,
   STATE_ALIGNV_SPINNER,
   STATE_ALIGNH_SPINNER,
   TEXT_ALIGNV_SPINNER,
   TEXT_ALIGNH_SPINNER,
   BORDER_TOP,
   BORDER_LEFT,
   BORDER_RIGHT,
   BORDER_BOTTOM,
   ROW_GROUP,
   ROW_PART,
   ROW_DESC,
   ROW_PROG,
   DRAG_MINIARROW,
   DRAG_REL1,
   DRAG_REL2,
   DRAG_MOVEBOX,
   REL_COMBO_INTERFACE,
   IMAGE_TWEEN_UP,
   IMAGE_TWEEN_DOWN,
   IMAGE_TWEEN_ADD,
   IMAGE_TWEEN_DELETE,
   SAVE_SCRIPT,
   LOAD_WIN,
   SAVE_WIN
};

struct Current_State
{
   char *open_file_name;      //Full path to the open edje file
   char *source_dir;          //Full path to sources
   char *main_source_file;    //Full path to the main edc file

   Etk_String *group;         //The current selected group name
   Etk_String *part;          //The current selected part name
   Etk_String *state;         //The current selected state name
   Etk_String *prog;          //The current selected prog name
   Etk_String *tween;         //The current selected tween name
   
   Etk_String *edj_file_name;
   Etk_String *edj_temp_name;
    
}Cur;

/* GLOBALS */
int            FileChooserOperation;   //The current file chooser operation (FILECHOOSER_OPEN,FILECHOOSER_NEW etc)
char           *EdjeFile;              //The filename of the edje_editor.edj file (witch contain all the graphics used by the program)
Evas_Object    *edje_o;                //The edje object we are editing
Evas_Hash      *Parts_Hash;            //Associate part names with Etk_Tree_Row*

Evas_Object    *EV_fakewin;            //The simple window implementation
Evas_Object    *EV_movebox;            //  FIXME
Evas_Object    *focus_handler;         //The yellow box around the selected part
Evas_Object    *rel1_handler;          //The red point
Evas_Object    *rel2_handler;          //The blue point
Evas_Object    *rel1X_parent_handler;  //The 4 line that show the container for each side of the part
Evas_Object    *rel1Y_parent_handler;  //
Evas_Object    *rel2X_parent_handler;  //
Evas_Object    *rel2Y_parent_handler;  //
Evas_Object    *Consolle;              //The lower consolle panel
Evas_List      *stack;                 //Stack for the consolle
int            consolle_count;        //Counter for the consolle

/* FUNCTION PROTOTYPES*/
void DebugInfo(int full);
int LoadEDJ(char *file);
void ChangeGroup(char *group);


//This define is copied from edje_private.h (find a way to export it)
#define EDJE_PART_TYPE_NONE      0
#define EDJE_PART_TYPE_RECTANGLE 1
#define EDJE_PART_TYPE_TEXT      2
#define EDJE_PART_TYPE_IMAGE     3
#define EDJE_PART_TYPE_SWALLOW   4
#define EDJE_PART_TYPE_TEXTBLOCK 5
#define EDJE_PART_TYPE_GRADIENT  6
#define EDJE_PART_TYPE_GROUP     7
#define EDJE_PART_TYPE_LAST      8

#define EDJE_TEXT_EFFECT_NONE                0
#define EDJE_TEXT_EFFECT_PLAIN               1
#define EDJE_TEXT_EFFECT_OUTLINE             2
#define EDJE_TEXT_EFFECT_SOFT_OUTLINE        3
#define EDJE_TEXT_EFFECT_SHADOW              4
#define EDJE_TEXT_EFFECT_SOFT_SHADOW         5
#define EDJE_TEXT_EFFECT_OUTLINE_SHADOW      6
#define EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW 7
#define EDJE_TEXT_EFFECT_FAR_SHADOW          8
#define EDJE_TEXT_EFFECT_FAR_SOFT_SHADOW     9
#define EDJE_TEXT_EFFECT_GLOW               10
#define EDJE_TEXT_EFFECT_LAST               11

#define EDJE_ACTION_TYPE_NONE          0
#define EDJE_ACTION_TYPE_STATE_SET     1
#define EDJE_ACTION_TYPE_ACTION_STOP   2
#define EDJE_ACTION_TYPE_SIGNAL_EMIT   3
#define EDJE_ACTION_TYPE_DRAG_VAL_SET  4
#define EDJE_ACTION_TYPE_DRAG_VAL_STEP 5
#define EDJE_ACTION_TYPE_DRAG_VAL_PAGE 6
#define EDJE_ACTION_TYPE_SCRIPT        7
#define EDJE_ACTION_TYPE_LAST          8

#define EDJE_TWEEN_MODE_NONE       0
#define EDJE_TWEEN_MODE_LINEAR     1
#define EDJE_TWEEN_MODE_SINUSOIDAL 2
#define EDJE_TWEEN_MODE_ACCELERATE 3
#define EDJE_TWEEN_MODE_DECELERATE 4
#define EDJE_TWEEN_MODE_LAST       5

#define EDJE_ASPECT_PREFER_NONE       0
#define EDJE_ASPECT_PREFER_VERTICAL   1
#define EDJE_ASPECT_PREFER_HORIZONTAL 2
#define EDJE_ASPECT_PREFER_BOTH       3

#endif // INCLUSION_GUARD
