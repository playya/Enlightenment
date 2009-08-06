/*  Copyright (C) 2006-2009 Davide Andreoli (see AUTHORS)
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

#ifndef _EDJE_EDITOR_MAIN_H_
#define _EDJE_EDITOR_MAIN_H_

#include "config.h"
#include <Elementary.h>
#include <Ecore_Str.h>
#include <Edje_Edit.h>
//~ #include <Edje.h>
//~ #include <Ecore_Data.h>
//~ #include <Ecore_File.h>
//~ #include <Ecore_Evas.h>
//~ #include "edje_editor_images.h"
#include "edje_editor_tree.h"
#include "edje_editor_group.h"
#include "edje_editor_part.h"
#include "edje_editor_state.h"
#include "edje_editor_position.h"
#include "edje_editor_text.h"
//~ #include "edje_editor_rect.h"
//~ #include "edje_editor_program.h"
//~ #include "edje_editor_script.h"
#include "edje_editor_toolbar.h"
#include "edje_editor_canvas.h"
#include "edje_editor_consolle.h"
#include "edje_editor_dialogs.h"
#include "edje_editor_window.h"
#include "edje_editor_fonts.h"
//~ #include "edje_editor_gradient.h"
//~ #include "edje_editor_fill.h"
//~ #include "edje_editor_spectra.h"
//~ #include "edje_editor_spectra_widget.h"
//~ #include "edje_editor_tree_model_spectra.h"
//~ #include "edje_editor_data.h"
//~ #include "edje_editor_colors.h"

/* DEFINES */
#define USE_GL_ENGINE 0
#define DEBUG_MODE 0

//~ #if DEBUG_MODE
   //~ #define TREE_WIDTH 365
//~ #else
   //~ #define TREE_WIDTH 265
//~ #endif

#define IFREE(ptr) { free(ptr); ptr = NULL; }

#define MSG_FLOAT "<b>Can't understand size.</b><br>The number need to be a float:<br> (for ex.) '0.35'"
#define MSG_INT "<b>Can't understand size.</b><br>The number need to be an integer :<br> (for ex.) '12'"
#define MSG_SIZE "<b>Can't understand sizes.</b><br>The format need to be:<br> (for ex.) '100x120'"
#define MSG_COLOR "<b>Can't understand color.</b><br>The format need to be \"r g b a\":<br> (for ex.) '255 128 0 200'"

//All the enum used are declared here
enum various
{
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
   TOOLBAR_PAUSE,
   TOOLBAR_MOVE_UP,
   TOOLBAR_MOVE_DOWN,
   TOOLBAR_IMAGE_BROWSER,
   TOOLBAR_SPECTRUM,
   TOOLBAR_DATA,
   TOOLBAR_COLORS,
   TOOLBAR_EXIT,
   FILECHOOSER_OPEN,
   //~ FILECHOOSER_IMAGE,
   FILECHOOSER_FONT,
   //~ FILECHOOSER_SAVE_EDC,
   //~ FILECHOOSER_SAVE_EDJ,
   //~ IMAGE_BROWSER_SHOW,
   //~ IMAGE_BROWSER_CLOSE,
   //~ TOOLBAR_FONT_BROWSER,
   //~ TOOLBAR_OPTION_BG1,
   //~ TOOLBAR_OPTION_BG2,
   //~ TOOLBAR_OPTION_BG3,
   //~ TOOLBAR_OPTION_BGC,
   //~ TOOLBAR_OPTION_FOPA100,
   //~ TOOLBAR_OPTION_FOPA50,
   //~ TOOLBAR_OPTION_FOPA25,
   //~ TOOLBAR_OPTION_FULLSCREEN,
   //~ TOOLBAR_IMAGE_FILE_ADD,
   //~ TOOLBAR_FONT_FILE_ADD,
   //~ TOOLBAR_QUIT,
   //~ COLOR_OBJECT_RECT,
   //~ COLOR_OBJECT_TEXT,
   //~ COLOR_OBJECT_SHADOW,
   //~ COLOR_OBJECT_OUTLINE,
   //~ COLOR_OBJECT_BG,
   //~ COLOR_OBJECT_CC1,
   //~ COLOR_OBJECT_CC2,
   //~ COLOR_OBJECT_CC3,
   //~ NEW_IMAGE,
   //~ NEW_GRADIENT,
   //~ NEW_RECT,
   //~ NEW_TEXT,
   //~ NEW_SWAL,
   //~ NEW_GROUPSWAL,
   //~ NEW_PROG,
   //~ NEW_DESC,
   //~ NEW_GROUP,
   //~ REMOVE_DESCRIPTION,
   //~ REMOVE_PART,
   //~ REMOVE_GROUP,
   //~ REMOVE_PROG,
   //~ REL1X_SPINNER,
   //~ REL1Y_SPINNER,
   //~ REL2X_SPINNER,
   //~ REL2Y_SPINNER,
   //~ REL1XO_SPINNER,
   //~ REL1YO_SPINNER,
   //~ REL2XO_SPINNER,
   //~ REL2YO_SPINNER,
   //~ MINW_SPINNER,
   //~ MAXW_SPINNER,
   //~ MINH_SPINNER,
   //~ MAXH_SPINNER,
   //~ STEPX_SPINNER,
   //~ STEPY_SPINNER,
   //~ COUNTX_SPINNER,
   //~ COUNTY_SPINNER,
   //~ STATE_ALIGNV_SPINNER,
   //~ STATE_ALIGNH_SPINNER,
   //~ TEXT_ALIGNV_SPINNER,
   //~ TEXT_ALIGNH_SPINNER,
   //~ BORDER_TOP,
   //~ BORDER_LEFT,
   //~ BORDER_RIGHT,
   //~ BORDER_BOTTOM,
   //~ ROW_GROUP,
   //~ ROW_PART,
   //~ ROW_DESC,
   //~ ROW_PROG,
   DRAG_MINIARROW,
   DRAG_REL1,
   DRAG_REL2,
   DRAG_MOVE,
   DRAG_MOVEBOX,
   DRAG_GRAD_1,
   DRAG_GRAD_2,
   EVENTS_NO,
   EVENTS_YES,
   EVENTS_YES_REPEAT,
   FIT_NONE,
   FIT_X,
   FIT_Y,
   FIT_BOTH
   //~ REL_COMBO_INTERFACE,
   //~ IMAGE_TWEEN_UP,
   //~ IMAGE_TWEEN_DOWN,
   //~ IMAGE_TWEEN_ADD,
   //~ IMAGE_TWEEN_DELETE,
   //~ SPECTRA_ADD,
   //~ SPECTRA_DELETE,
   //~ SAVE_SCRIPT,
   //~ RUN_PROG
};



struct CurrentState
{
   const char *group;           //The current selected group name (stringshared)
   const char *part;            //The current selected part name (stringshared)
   const char *state;           //The current selected state name (stringshared)
   const char *prog;            //The current selected prog name (stringshared)
   const char *tween;           //The current selected tween name in the image frame (stringshared)
   //~ Etk_String *spectra;     //The current selected spectra in the spectra editor

   const char *open_file_name;  // The original name of the opened file (stringshared)
   const char *open_temp_name;  // The name of the temp file currently open (stringshared)

   int fullscreen;              // The current main window state
}cur;


struct UserInterfaceObjects
{
   Evas_Object *win;         // The elementary window
   Evas_Object *tree_pager;  // The left pager (for the tree)
   Evas_Object *parts_tree;  // The elm genlist that contain all the parts
   Evas_Object *edje_ui;     // The main edje interface that contain the layout

   Evas_Object *edje_o;      // The edje object we are editing
}ui;

/* GLOBALS */
//~ int            ImageBroserUpdate;      //When set to true the image browser will update the current state with the selected image
char           *EdjeFile;              //The filename of the edje_editor.edj file (witch contain all the graphics used by the program)

//~ Eina_Hash      *Parts_Hash;            //Associate part names with Etk_Tree_Row*


/* FUNCTION PROTOTYPES*/
//~ void print_debug_info (int full);
void change_to_group(const char *group);

void set_current_group(const char *group);
void set_current_part(const char *part);
void set_current_state(const char *state);
void set_current_prog(const char *prog);
void set_current_tween(const char *tween);


int  load_edje        (const char *file);
//~ void reload_edje      (void);


void elm_entry_printf(Evas_Object *obj, const char *format, ...);
Eina_Bool ecore_str_equal(const char *s1, const char *s2);


//This define is copied from edje_private.h (find a way to export it)
#define EDJE_PART_TYPE_NONE      0
#define EDJE_PART_TYPE_RECTANGLE 1
#define EDJE_PART_TYPE_TEXT      2
#define EDJE_PART_TYPE_IMAGE     3
#define EDJE_PART_TYPE_SWALLOW   4
#define EDJE_PART_TYPE_TEXTBLOCK 5
#define EDJE_PART_TYPE_GRADIENT  6
#define EDJE_PART_TYPE_GROUP     7
#define EDJE_PART_TYPE_BOX       8
#define EDJE_PART_TYPE_TABLE     9
#define EDJE_PART_TYPE_LAST     10

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


#endif
