#ifndef _EX_H
#define _EX_H

#include <Evas.h>
#include <Ecore.h>
#include <Ecore_X.h>
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

typedef enum _Ex_Images
{
   EX_IMAGE_FIT_TO_WINDOW = -1,
   EX_IMAGE_ONE_TO_ONE = -2,
   EX_IMAGE_ZOOM_IN = -3,
   EX_IMAGE_ZOOM_OUT = -4
} Ex_Images;

typedef struct _Exhibit Exhibit;
struct _Exhibit
{
   Etk_Widget    *vbox;
   Etk_Widget    *hbox;
   Etk_Widget    *menu_bar;
   Etk_Widget    *statusbar[4];
   Etk_Widget    *scrolled_view;
   Etk_Widget    *alignment;
   Etk_Widget    *table;
   Etk_Widget    *hpaned;
   Etk_Widget    *vpaned;
   Etk_Widget    *dtree;
   Etk_Widget    *itree;
   Etk_Widget    *entry[2];
   Etk_Widget    *zoom_in[2];
   Etk_Widget    *zoom_out[2];
   Etk_Widget    *fit[2];
   Etk_Widget    *original[2];
   Etk_Widget    *image;
   Etk_Widget    *sort;
   Etk_Widget    *sizebar;
   Etk_Widget    *resbar;
   Etk_Widget    *zoombar;
   Etk_Tree_Col  *dcol;
   Etk_Tree_Col  *icol;
   Etk_Widget    *menu;
   Etk_Widget    *win;

   Evas_List     *images;
   Evas_List     *dirs;

   char          *dir;
   char           cur_path[PATH_MAX];   

   int            zoom;
   int            brightness;
   int            contrast;
   Etk_Bool       fit_window;
   
   struct {
      int down;
      int x;
      int y;
   } mouse;
};

typedef struct _Ex_Thumb Ex_Thumb;
struct _Ex_Thumb
{
   Exhibit  *e;
   char    *name;
   char    *image;
   Epsilon *ep;
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
  
#endif
