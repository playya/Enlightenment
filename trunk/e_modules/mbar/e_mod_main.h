/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

typedef struct _Config      Config;
typedef struct _Config_Bar  Config_Bar;
typedef struct _MBar        MBar;
typedef struct _MBar_Bar    MBar_Bar;
typedef struct _MBar_Icon   MBar_Icon;

#define MBAR_WIDTH_AUTO -1
#define MBAR_WIDTH_FIXED -2

#define FSTAB "/etc/fstab"
#define PROCMOUNTS "/proc/mounts"
#define MTAB "/etc/mtab"

struct _Config
{
   const char   *appdir;
   int           follower;
   double        follow_speed;
   double        autoscroll_speed;
   int           iconsize;
   int           width;
   int		 allow_overlap;
   Evas_List    *bars;
};

struct _Config_Bar
{
   unsigned char enabled;
};

struct _MBar
{
   E_App       *apps;
   Evas_List   *bars;
   E_Menu      *config_menu;

   Config      *conf;
   E_Config_Dialog *config_dialog;   
};

struct _MBar_Bar
{
   MBar        *mbar;
   E_Container *con;
   Evas        *evas;
   E_Menu      *menu;

   Evas_Object *bar_object;
   Evas_Object *overlay_object;
   Evas_Object *box_object;
   Evas_Object *event_object;
   Evas_Object *drag_object;
   Evas_Object *drag_object_overlay;

   Evas_List   *icons;

   double          align, align_req;
   double          follow, follow_req;
   Ecore_Timer    *timer;
   Ecore_Animator *animator;

   Evas_Coord      x, y, w, h;
   struct {
	Evas_Coord l, r, t, b;
   } bar_inset;
   struct {
	Evas_Coord l, r, t, b;
   } icon_inset;

   E_Gadman_Client *gmc;

   Config_Bar     *conf;

   E_Drop_Handler *drop_handler;
};

struct _MBar_Icon
{
   MBar_Bar      *mbb;
   E_App         *app;
   Evas_Object   *bg_object;
   Evas_Object   *overlay_object;
   Evas_Object   *icon_object;
   Evas_Object   *event_object;
   Evas_List     *extra_icons;
   
   unsigned char  raise_on_hilight : 1;
};

EAPI extern E_Module_Api e_modapi;

EAPI void *e_modapi_init     (E_Module *m);
EAPI int   e_modapi_shutdown (E_Module *m);
EAPI int   e_modapi_save     (E_Module *m);
EAPI int   e_modapi_info     (E_Module *m);
EAPI int   e_modapi_about    (E_Module *m);
EAPI int   e_modapi_config   (E_Module *m);

void _mbar_bar_cb_config_updated(void *data);

#endif
