#ifndef ENGRAVE_H
#define ENGRAVE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Evas.h>

typedef enum _Engrave_Image_Type Engrave_Image_Type;
typedef enum _Engrave_Part_Type Engrave_Part_Type;
typedef enum _Engrave_Text_Effect Engrave_Text_Effect;
typedef enum _Engrave_Action Engrave_Action;
typedef enum _Engrave_Transition Engrave_Transition;
typedef enum _Engrave_Aspect_Preference Engrave_Aspect_Preference;

typedef struct _Engrave_File Engrave_File;
typedef struct _Engrave_Data Engrave_Data;
typedef struct _Engrave_Image Engrave_Image;
typedef struct _Engrave_Font Engrave_Font;
typedef struct _Engrave_Group Engrave_Group;
typedef struct _Engrave_Part Engrave_Part;
typedef struct _Engrave_Program Engrave_Program;
typedef struct _Engrave_Part_State Engrave_Part_State;

typedef enum _Engrave_Parse_Section Engrave_Parse_Section;


enum _Engrave_Image_Type
{
  ENGRAVE_IMAGE_TYPE_RAW,
  ENGRAVE_IMAGE_TYPE_COMP,
  ENGRAVE_IMAGE_TYPE_LOSSY,
  ENGRAVE_IMAGE_TYPE_EXTERNAL,
  ENGRAVE_IMAGE_TYPE_NUM

};

enum _Engrave_Part_Type
{
  ENGRAVE_PART_TYPE_IMAGE,
  ENGRAVE_PART_TYPE_TEXT,
  ENGRAVE_PART_TYPE_RECT,
  ENGRAVE_PART_TYPE_SWALLOW,
  ENGRAVE_PART_TYPE_NUM
};

enum _Engrave_Text_Effect
{
  ENGRAVE_TEXT_EFFECT_NONE,
  ENGRAVE_TEXT_EFFECT_PLAIN,
  ENGRAVE_TEXT_EFFECT_OUTLINE,
  ENGRAVE_TEXT_EFFECT_SOFT_OUTLINE,
  ENGRAVE_TEXT_EFFECT_SHADOW,
  ENGRAVE_TEXT_EFFECT_OUTLINE_SHADOW,
  ENGRAVE_TEXT_EFFECT_SOFT_SHADOW,
  ENGRAVE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW,
  ENGRAVE_TEXT_EFFECT_NUM
};

enum _Engrave_Action
{
  ENGRAVE_ACTION_STATE_SET,
  ENGRAVE_ACTION_STOP,
  ENGRAVE_ACTION_SIGNAL_EMIT,
  ENGRAVE_ACTION_DRAG_VAL_SET,
  ENGRAVE_ACTION_DRAG_VAL_STEP,
  ENGRAVE_ACTION_DRAG_VAL_PAGE,
  ENGRAVE_ACTION_SCRIPT,
  ENGRAVE_ACTION_NUM
};

enum _Engrave_Transition
{
  ENGRAVE_TRANSITION_LINEAR,
  ENGRAVE_TRANSITION_SINUSOIDAL,
  ENGRAVE_TRANSITION_ACCELERATE,
  ENGRAVE_TRANSITION_DECELERATE,
  ENGRAVE_TRANSITION_NUM
};

enum _Engrave_Aspect_Preference
{
  ENGRAVE_ASPECT_PREFERENCE_NONE,
  ENGRAVE_ASPECT_PREFERENCE_VERTICAL,
  ENGRAVE_ASPECT_PREFERENCE_HORIZONTAL,
  ENGRAVE_ASPECT_PREFERENCE_BOTH,
  ENGRAVE_ASPECT_PREFERENCE_NUM
};

struct _Engrave_File
{
  Evas_List *images;
  Evas_List *fonts;
  Evas_List *data;
  Evas_List *groups;  
};

struct _Engrave_Data
{
  char *key;
  char *value;
  int int_value;
};

struct _Engrave_Image
{
  char *name; /* basename */
  char *path; /* dir path */
  Engrave_Image_Type type;
  double value;
};

struct _Engrave_Font
{
  char *name; /* alias */
  char *file; /* basename */
  char *path; /* dir path */
};

struct _Engrave_Group
{
  char *name;
  struct
  {
    int w, h;
  } min, max;

  Evas_List *parts;
  Evas_List *programs;
  Evas_List *data;

  char *script;
};

struct _Engrave_Part
{
  char *name;
  Engrave_Part_Type type;
  Engrave_Text_Effect effect;
  int mouse_events;
  int repeat_events;
  char *clip_to;

  struct
  {
    signed char x, y; /* can drag in x/y, and which dir to count in */
    struct
    {
      int x, y;
    } step, count; 
    char *confine;

  } dragable;

  Evas_List *states;
};

struct _Engrave_Program 
{
  char *name;
  char *signal;
  char *source;
  Evas_List *targets;
  Evas_List *afters;

  struct {
    double from, range;
  } in;
  
  Engrave_Action action;
  char *state, *state2;
  double value, value2;

  Engrave_Transition transition;
  double duration;
 
  char *script;
};

struct _Engrave_Part_State
{
  char *name;
  double value;

  unsigned char visible;

  struct
  {
    double x, y;
  } align, step;

  struct
  {
    double w, h;
  } min, max;

  struct
  {
    double w, h;
    Engrave_Aspect_Preference prefer;
  } aspect;

  struct
  {
    struct
    {
      double x, y;
    } relative;
    
    struct
    {
      int x, y;
    } offset;
    
    char *to_x;
    char *to_y;
  } rel1, rel2;

  struct
  {
    Engrave_Image *normal;
    Evas_List *tween;
  } image;

  struct
  {
    int l, r, t, b;
  } border;

  char *color_class;

  struct
  {
    int r, g, b, a;
  } color, color2, color3;

  struct
  {
    int           smooth; 

    struct
    {
      double x, y;
    } pos_rel, rel;
    struct
    {
      int x, y;
    } pos_abs, abs;
  } fill;

  struct
  {
    char          *text; 
    char          *text_class; 
    char          *font; 

    int            size; 

    struct {
      int x, y;
    } fit, min;

    struct {
      double      x, y; 
    } align;
  } text;
};

Engrave_File *engrave_load_edc(char *file, char *imdir, char *fontdir);
Engrave_File * engrave_load_eet(char *filename);

int engrave_eet_output(Engrave_File *engrave_file, char *path);
int engrave_file_output(Engrave_File *engrave_file, char *path);

#endif

