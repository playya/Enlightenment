#include "e.h"
#include "e_mod_main.h"

/* local structures */
typedef struct _Instance Instance;

struct _Instance 
{
   E_Gadcon_Client *gcc;
   Evas_Object *o_btn;
   E_Menu *menu;
   Il_Home_Win *hwin;
};

/* local function prototypes */
static E_Gadcon_Client *_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style);
static void _gc_shutdown(E_Gadcon_Client *gcc);
static void _gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient);
static char *_gc_label(E_Gadcon_Client_Class *cc);
static Evas_Object *_gc_icon(E_Gadcon_Client_Class *cc, Evas *evas);
static const char *_gc_id_new(E_Gadcon_Client_Class *cc);
static void _il_home_btn_cb_click(void *data, void *data2);
static void _il_home_btn_cb_mouse_down(void *data, Evas *evas, Evas_Object *obj, void *event);
static void _il_home_menu_cb_post(void *data, E_Menu *mn);
static void _il_home_win_new(Il_Home_Win *hwin);
static void _il_home_win_cb_free(Il_Home_Win *hwin);
static void _il_home_win_cb_delete(E_Win *win);
static void _il_home_win_cb_resize(E_Win *win);
static void _il_home_pan_set(Evas_Object *obj, Evas_Coord x, Evas_Coord y);
static void _il_home_pan_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y);
static void _il_home_pan_max_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y);
static void _il_home_pan_child_size_get(Evas_Object *obj, Evas_Coord *w, Evas_Coord *h);
static void _il_home_cb_selected(void *data, Evas_Object *obj, void *event);
static void _il_home_desktop_run(Efreet_Desktop *desktop);

/* local variables */
static Eina_List *instances = NULL;
static E_Module *mod;

static const E_Gadcon_Client_Class _gc_class = 
{
   GADCON_CLIENT_CLASS_VERSION, "illume-home", 
     { _gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon, _gc_id_new, NULL, 
          e_gadcon_site_is_not_toolbar
     }, E_GADCON_CLIENT_STYLE_PLAIN
};

/* public functions */
EAPI E_Module_Api e_modapi = { E_MODULE_API_VERSION, "Illume Home" };

EAPI void *
e_modapi_init(E_Module *m) 
{
   mod = m;
   e_gadcon_provider_register(&_gc_class);
   return m;
}

EAPI int 
e_modapi_shutdown(E_Module *m) 
{
   e_gadcon_provider_unregister(&_gc_class);
   mod = NULL;
   return 1;
}

EAPI int 
e_modapi_save(E_Module *m) 
{
   return 1;
}

/* local functions */
static E_Gadcon_Client *
_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style) 
{
   Instance *inst;
   Evas_Object *icon;
   char buff[PATH_MAX];

   snprintf(buff, sizeof(buff), "%s/e-module-illume-home.edj", mod->dir);
   inst = E_NEW(Instance, 1);
   inst->o_btn = e_widget_button_add(gc->evas, NULL, NULL, 
                                     _il_home_btn_cb_click, inst, NULL);
   icon = e_icon_add(evas_object_evas_get(inst->o_btn));
   e_icon_file_edje_set(icon, buff, "btn_icon");
   e_widget_button_icon_set(inst->o_btn, icon);

   inst->gcc = e_gadcon_client_new(gc, name, id, style, inst->o_btn);
   inst->gcc->data = inst;

   evas_object_event_callback_add(inst->o_btn, EVAS_CALLBACK_MOUSE_DOWN, 
                                  _il_home_btn_cb_mouse_down, inst);

   instances = eina_list_append(instances, inst);
   return inst->gcc;
}

static void 
_gc_shutdown(E_Gadcon_Client *gcc) 
{
   Instance *inst;

   if (!(inst = gcc->data)) return;
   instances = eina_list_remove(instances, inst);
   if (inst->menu) 
     {
        e_menu_post_deactivate_callback_set(inst->menu, NULL, NULL);
        e_object_del(E_OBJECT(inst->menu));
        inst->menu = NULL;
     }
   if (inst->o_btn) 
     {
        evas_object_event_callback_del(inst->o_btn, EVAS_CALLBACK_MOUSE_DOWN, 
                                       _il_home_btn_cb_mouse_down);
        evas_object_del(inst->o_btn);
     }
   if (inst->hwin) e_object_del(E_OBJECT(inst->hwin));
   E_FREE(inst);
}

static void 
_gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient) 
{
   e_gadcon_client_aspect_set(gcc, 16, 16);
   e_gadcon_client_min_size_set(gcc, 16, 16);
}

static char *
_gc_label(E_Gadcon_Client_Class *cc) 
{
   return _("Illume-Home");
}

static Evas_Object *
_gc_icon(E_Gadcon_Client_Class *cc, Evas *evas) 
{
   Evas_Object *o;
   char buff[PATH_MAX];

   snprintf(buff, sizeof(buff), "%s/e-module-illume-home.edj", mod->dir);
   o = edje_object_add(evas);
   edje_object_file_set(o, buff, "icon");
   return o;
}

static const char *
_gc_id_new(E_Gadcon_Client_Class *cc) 
{
   char buff[PATH_MAX];

   snprintf(buff, sizeof(buff), "%s.%d", _gc_class.name, 
            eina_list_count(instances));
   return strdup(buff);
}

static void 
_il_home_btn_cb_click(void *data, void *data2) 
{
   Instance *inst;

   if (!(inst = data)) return;
   if (!inst->hwin) 
     {
        inst->hwin = E_OBJECT_ALLOC(Il_Home_Win, IL_HOME_WIN_TYPE, 
                                    _il_home_win_cb_free);
     }
   if (!inst->hwin) return;
   _il_home_win_new(inst->hwin);
}

static void 
_il_home_btn_cb_mouse_down(void *data, Evas *evas, Evas_Object *obj, void *event) 
{
   Instance *inst;
   Evas_Event_Mouse_Down *ev;

   if (!(inst = data)) return;
   ev = event;
   if (ev->button == 1) return;
   if ((ev->button == 3) && (!inst->menu)) 
     {
        E_Menu *mn;
        E_Zone *zone;
        int x = 0, y = 0;

        zone = e_util_zone_current_get(e_manager_current_get());
        mn = e_menu_new();
        inst->menu = mn;
        e_menu_post_deactivate_callback_set(inst->menu, 
                                            _il_home_menu_cb_post, inst);
        e_gadcon_client_util_menu_items_append(inst->gcc, mn, 0);
        e_gadcon_canvas_zone_geometry_get(inst->gcc->gadcon, &x, &y, NULL, NULL);
        e_menu_activate_mouse(mn, zone, x + ev->output.x, y + ev->output.y, 
                              1, 1, E_MENU_POP_DIRECTION_AUTO, ev->timestamp);
     }
}

static void 
_il_home_menu_cb_post(void *data, E_Menu *mn) 
{
   Instance *inst;

   if (!(inst = data)) return;
   if (!inst->menu) return;
   e_object_del(E_OBJECT(inst->menu));
   inst->menu = NULL;
}

static void 
_il_home_win_new(Il_Home_Win *hwin) 
{
   E_Container *con;
   E_Fm2_Config fmc;
   char buff[PATH_MAX];

   if (!hwin) return;
   if (hwin->win) 
     {
        e_win_show(hwin->win);
        return;
     }

   snprintf(buff, sizeof(buff), "%s/e-module-illume-home.edj", mod->dir);
   con = e_container_current_get(e_manager_current_get());

   hwin->win = e_win_new(con);
   if (!hwin->win) return;

   e_win_delete_callback_set(hwin->win, _il_home_win_cb_delete);
   e_win_resize_callback_set(hwin->win, _il_home_win_cb_resize);
   hwin->win->data = hwin;

   hwin->o_bg = edje_object_add(e_win_evas_get(hwin->win));
   if (!e_theme_edje_object_set(hwin->o_bg, "base/theme/modules/illume-home", 
                                "modules/illume-home/window"))
     edje_object_file_set(hwin->o_bg, buff, "modules/illume-home/window");
   evas_object_move(hwin->o_bg, 0, 0);
   evas_object_resize(hwin->o_bg, hwin->win->w, hwin->win->h);
   evas_object_show(hwin->o_bg);

   hwin->o_sf = e_scrollframe_add(e_win_evas_get(hwin->win));
   e_scrollframe_single_dir_set(hwin->o_sf, 1);
   evas_object_move(hwin->o_sf, 0, 0);
   evas_object_resize(hwin->o_sf, hwin->win->w, hwin->win->h);
   evas_object_show(hwin->o_sf);

   e_scrollframe_custom_edje_file_set(hwin->o_sf, buff, 
                                      "modules/illume-home/launcher/scrollview");

   memset(&fmc, 0, sizeof(E_Fm2_Config));
   fmc.view.mode = E_FM2_VIEW_MODE_GRID_ICONS;
   fmc.view.open_dirs_in_place = 1;
   fmc.view.selector = 0;
//   fmc.view.single_click = illume_cfg->launcher.single_click;
//   fmc.view.single_click_delay = illume_cfg->launcher.single_click_delay;
   fmc.view.no_subdir_jump = 1;
   fmc.icon.extension.show = 0;
   fmc.icon.icon.w = 48;
   fmc.icon.icon.h = 48;
   fmc.icon.fixed.w = 48;
   fmc.icon.fixed.h = 48;
//   fmc.icon.icon.w = illume_cfg->launcher.icon_size * e_scale / 2.0;
//   fmc.icon.icon.h = illume_cfg->launcher.icon_size * e_scale / 2.0;
//   fmc.icon.fixed.w = illume_cfg->launcher.icon_size * e_scale / 2.0;
//   fmc.icon.fixed.h = illume_cfg->launcher.icon_size * e_scale / 2.0;
   fmc.list.sort.no_case = 0;
   fmc.list.sort.dirs.first = 1;
   fmc.list.sort.dirs.last = 0;
   fmc.selection.single = 1;
   fmc.selection.windows_modifiers = 0;

   hwin->o_fm = e_fm2_add(e_win_evas_get(hwin->win));
   e_fm2_config_set(hwin->o_fm, &fmc);
   evas_object_show(hwin->o_fm);
   e_fm2_path_set(hwin->o_fm, "/", getenv("HOME"));

   e_scrollframe_extern_pan_set(hwin->o_sf, hwin->o_fm, 
                                _il_home_pan_set, 
                                _il_home_pan_get, 
                                _il_home_pan_max_get, 
                                _il_home_pan_child_size_get);
   evas_object_smart_callback_add(hwin->o_fm, "selected", 
                                  _il_home_cb_selected, NULL);

   e_win_title_set(hwin->win, _("Illume Home"));
   e_win_name_class_set(hwin->win, "Illume-Home", "Home");
   e_win_size_min_set(hwin->win, 24, 24);
   e_win_resize(hwin->win, 200, 200);
   e_win_centered_set(hwin->win, 1);
   e_win_show(hwin->win);
}

static void 
_il_home_win_cb_free(Il_Home_Win *hwin) 
{
   if (hwin->win) e_object_del(E_OBJECT(hwin->win));
   e_object_del(E_OBJECT(hwin));
}

static void 
_il_home_win_cb_delete(E_Win *win) 
{
   Il_Home_Win *hwin;

   hwin = win->data;
   e_object_del(E_OBJECT(hwin));
}

static void 
_il_home_win_cb_resize(E_Win *win) 
{
   Il_Home_Win *hwin;

   hwin = win->data;
   if (hwin->o_bg) 
     {
        if (hwin->win)
          evas_object_resize(hwin->o_bg, hwin->win->w, hwin->win->h);
     }
   if (hwin->o_sf) 
     {
        if (hwin->win)
          evas_object_resize(hwin->o_sf, hwin->win->w, hwin->win->h);
     }
}

static void 
_il_home_pan_set(Evas_Object *obj, Evas_Coord x, Evas_Coord y) 
{
   e_fm2_pan_set(obj, x, y);
}

static void 
_il_home_pan_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y) 
{
   e_fm2_pan_get(obj, x, y);
}

static void 
_il_home_pan_max_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y) 
{
   e_fm2_pan_max_get(obj, x, y);
}

static void 
_il_home_pan_child_size_get(Evas_Object *obj, Evas_Coord *w, Evas_Coord *h) 
{
   e_fm2_pan_child_size_get(obj, w, h);
}

static void 
_il_home_cb_selected(void *data, Evas_Object *obj, void *event) 
{
   Eina_List *selected;
   E_Fm2_Icon_Info *ici;

   selected = e_fm2_selected_list_get(obj);
   if (!selected) return;
   EINA_LIST_FREE(selected, ici) 
     {
        Efreet_Desktop *desktop;

        if (ici) 
          {
             printf("Selected: %s\n", ici->file);
             if (ici->real_link) 
               {
                  desktop = efreet_desktop_get(ici->real_link);
//                  if (desktop) _il_home_desktop_run(desktop);
               }
          }
     }
}
