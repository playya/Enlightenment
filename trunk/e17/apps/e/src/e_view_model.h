#ifndef E_VIEW_MODEL_H
#define E_VIEW_MODEL_H

#include "fs.h"
#include "iconbar.h"
#include <Evas.h>

#ifndef E_VIEW_TYPEDEF
#define E_VIEW_TYPEDEF
typedef struct _E_View    E_View;
#endif

#ifndef E_ICON_TYPEDEF
#define E_ICON_TYPEDEF
typedef struct _E_Icon    E_Icon;
#endif

#ifndef E_VIEW_MODEL_TYPEDEF
#define E_VIEW_MODEL_TYPEDEF
typedef struct _E_View_Model E_View_Model;
#endif

struct _E_View_Model
{
   E_Object               o;
   char                  *dir;
   Evas_List              files;

   Evas_Object            obj_bg;
   
   char                  *bg_file;
 
   E_FS_Restarter        *restarter;
   
   int                    monitor_id;

   Evas_List              views;
   int                    is_desktop;
};

void e_view_model_set_dir(E_View_Model *m, char *dir);
E_View_Model * e_view_model_new();
void e_view_model_init();
E_View_Model * e_view_model_find_by_monitor_id(int id);
void      e_view_model_file_added(int id, char *file);
void      e_view_model_file_deleted(int id, char *file);
void      e_view_model_file_changed(int id, char *file);
void      e_view_model_file_moved(int id, char *file);
#endif
