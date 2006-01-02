#ifndef _EX_H
#define _EX_H

#include <Evas.h>
#include <Ecore.h>
#include <Ecore_X.h>
#include <Ecore_File.h>
#include <Ecore_X_Cursor.h>
#include <Epsilon.h>
#include <etk/Etk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>

#include "config.h"

#if HAVE_E
#include <E_Lib.h>
#endif

#if HAVE_ENGRAVE
#include <Engrave.h>
#endif

#define ETK_DEFAULT_ICON_SET_FILE ETK_PACKAGE_DATA_DIR "/stock_icons/default.edj"

typedef struct _Exhibit Exhibit;
typedef struct _Ex_Tab Ex_Tab;
typedef struct _Ex_Thumb Ex_Thumb;
typedef enum _Ex_Images
{
   EX_IMAGE_FIT_TO_WINDOW = -1,
   EX_IMAGE_ONE_TO_ONE = -2,
   EX_IMAGE_ZOOM_IN = -3,
   EX_IMAGE_ZOOM_OUT = -4
} Ex_Images;

struct _Ex_Tab
{
   char          *dir;
   char           cur_path[PATH_MAX];
   int            num;

   Etk_Bool       fit_window;   
   
   Evas_List     *images;
   Evas_List     *dirs;
  
   Etk_Widget    *image;   
   Etk_Widget    *dtree;
   Etk_Widget    *itree;   
   Etk_Widget    *scrolled_view;
   Etk_Widget    *alignment;   
   
   Etk_Tree_Col  *dcol;
   Etk_Tree_Col  *icol;
   
   Exhibit       *e;
};


struct _Exhibit
{
   Etk_Widget    *vbox;
   Etk_Widget    *hbox;
   Etk_Widget    *menu_bar;
   Etk_Widget    *statusbar[4];
   Etk_Widget    *notebook;
   Etk_Widget    *table;
   Etk_Widget    *hpaned;
   Etk_Widget    *vpaned;
   Etk_Widget    *entry[2];
   Etk_Widget    *zoom_in[2];
   Etk_Widget    *zoom_out[2];
   Etk_Widget    *fit[2];
   Etk_Widget    *original[2];
   Etk_Widget    *sort;
   Etk_Widget    *sizebar;
   Etk_Widget    *resbar;
   Etk_Widget    *zoombar;
   Etk_Widget    *menu;
   Etk_Widget    *win;

   char          *dir;
   char           cur_path[PATH_MAX];   

   int            zoom;
   int            brightness;
   int            contrast;

   Evas_List     *tabs;
   Ex_Tab        *cur_tab;
   
   struct {
      int down;
      int x;
      int y;
   } mouse;
   
   struct {
      double       interval;
      Ecore_Timer *timer;
      Etk_Bool     active;
   } slideshow;
};

struct _Ex_Thumb
{
   Exhibit  *e;
   char     *name;
   char     *image;
   Etk_Bool  selected;
   Epsilon  *ep;
};

#define WINDOW_TITLE "Exhibit"
#define WINDOW_WIDTH 700
#define WINDOW_HEIGHT 500
#define VIEWABLES 3
#define ZOOM_MAX 16
#define ZOOM_MIN -16

#ifndef DATA64
#define DATA64 unsigned long long
#define DATA32 unsigned int
#define DATA16 unsigned short
#define DATA8  unsigned char
#endif

#include "exhibit_menus.h"
#include "exhibit_file.h"
#include "exhibit.h"
#include "exhibit_image.h"
#include "exhibit_main.h"
#include "exhibit_menus.h"
#include "exhibit_sort.h"
#include "exhibit_thumb.h"
#include "exhibit_tab.h"
#include "exhibit_slideshow.h"

#endif
