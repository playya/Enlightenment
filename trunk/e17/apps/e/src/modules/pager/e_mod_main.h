#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

typedef struct _Config     Config;
typedef struct _Pager      Pager;
typedef struct _Pager_Desk Pager_Desk;
typedef struct _Pager_Win  Pager_Win;

struct _Config
{
   int    width, height;
   double x, y;
};

struct _Pager
{
   E_Menu       *config_menu;
   Evas         *evas;
   Evas_Object  *base, *screen;
   Evas_List    *desks;
   
   E_Container  *con;
   E_Config_DD  *conf_edd;
   Config       *conf;
   unsigned char move : 1;
   unsigned char resize : 1;
   Ecore_Event_Handler *ev_handler_container_resize;
   
   Ecore_Event_Handler *ev_handler_border_resize;
   Ecore_Event_Handler *ev_handler_border_move;
   Ecore_Event_Handler *ev_handler_border_add;
   Ecore_Event_Handler *ev_handler_border_remove;
   Ecore_Event_Handler *ev_handler_border_hide;
   Ecore_Event_Handler *ev_handler_border_show;
   Ecore_Event_Handler *ev_handler_border_desk_set;
   Ecore_Event_Handler *ev_handler_zone_desk_count_set;

   Evas_Coord    fx, fy, fw, fh, tw, th;
   Evas_Coord    xx, yy;

   /* FIXME: want to fix click detection once leftdrag is not used */
   Evas_Coord    clickhackx, clickhacky;
};

struct _Pager_Desk
{
   Evas_List   *wins;
   Evas_Object *obj;
   int          xpos, ypos;

   E_Desk      *desk;
   int          current:1;
};

struct _Pager_Win
{
   Evas_Object *obj;
   Evas_Object *icon;
   
   Pager_Desk  *owner;
   E_Border    *border;
};

EAPI void *init     (E_Module *m);
EAPI int   shutdown (E_Module *m);
EAPI int   save     (E_Module *m);
EAPI int   info     (E_Module *m);
EAPI int   about    (E_Module *m);

#endif
