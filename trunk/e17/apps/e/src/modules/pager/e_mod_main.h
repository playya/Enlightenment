/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

typedef struct _Config      Config;
typedef struct _Config_Face Config_Face;
typedef struct _Pager       Pager;
typedef struct _Pager_Face  Pager_Face;
typedef struct _Pager_Desk  Pager_Desk;
typedef struct _Pager_Win   Pager_Win;

#define PAGER_RESIZE_NONE 0
#define PAGER_RESIZE_HORZ 1
#define PAGER_RESIZE_VERT 2
#define PAGER_RESIZE_BOTH 3

struct _Config
{
   Evas_List *faces;
};

struct _Config_Face
{
   /* Show face */
   unsigned char enabled;
   /* Keep scale of desktops */
   unsigned char scale;
   /* Resize pager when adding/removing desktops */
   unsigned char resize;
};

struct _Pager
{
   Evas_List   *faces;
   E_Menu      *config_menu;
   Evas_List   *menus;

   Config      *conf;
};

struct _Pager_Face
{
   E_Zone       *zone;
   Evas_List    *desks;
   
   E_Menu       *menu;
   Evas         *evas;
   Evas_Object  *base, *screen;

   Evas_Coord    fx, fy, fw, fh;
   E_Gadman_Client *gmc;

   /* Current nr. of desktops */
   int           xnum, ynum;

   Config_Face  *conf;
   
   Ecore_Event_Handler *ev_handler_border_resize;
   Ecore_Event_Handler *ev_handler_border_move;
   Ecore_Event_Handler *ev_handler_border_add;
   Ecore_Event_Handler *ev_handler_border_remove;
   Ecore_Event_Handler *ev_handler_border_hide;
   Ecore_Event_Handler *ev_handler_border_show;
   Ecore_Event_Handler *ev_handler_border_desk_set;
   Ecore_Event_Handler *ev_handler_zone_desk_count_set;
};

struct _Pager_Desk
{
   E_Desk      *desk;
   Pager_Face  *face;
   Evas_List   *wins;

   Evas_Object *obj;
   int          xpos, ypos;

   int          current : 1;
};

struct _Pager_Win
{
   E_Border    *border;
   Pager_Desk  *desk;

   Evas_Object *obj;
   Evas_Object *icon;
};

EAPI void *init     (E_Module *module);
EAPI int   shutdown (E_Module *module);
EAPI int   save     (E_Module *module);
EAPI int   info     (E_Module *module);
EAPI int   about    (E_Module *module);

#endif
