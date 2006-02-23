/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

/* mouse use
 * =========
 *
 * 
 *
 *
 * 
 */


/* TODO:
 *
 * - if we resize efm while we're generating files, we crash
 *
 * - convert current layout scheme to canvas layout (AmigaOS style)
 *
 * - begin configuration dialogs
 *
 * - allow per dir bg
 *
 * - allow custom icon images
 *
 * - add ability to have icons on desktop (this works, but we need some fixes)
 *   files should go on ~/.e/e/desktop for example.
 *
 * - when we select multiple items, the right click menu on the icons needs
 *   to display some group related things and its actions need to work
 *   on the group.
 *
 * - is the offset code working properly? i have a feeling we're displayin
 *   more icons that the visible space can take and they are being hidden.
 *
 * + allow for icon movement inside the canvas
 *
 * - double check dir monitoring. note: when we are in a dir that is constantly
 *   changing, we cant keep calling redraw_new as it will kill us.
 *
 */

/* BUGS:
 *
 * - Closing Efm window while its thumbnailing causes a segv
 *
 * - Deleting a dir causes a segv
 *
 * - redo monitor code (incremental changes)
 */

int E_EVENT_FM_RECONFIGURE;
int E_EVENT_FM_DIRECTORY_CHANGE;

#ifdef EFM_DEBUG
# define D(x)  do {printf(__FILE__ ":%d:  ", __LINE__); printf x; fflush(stdout);} while (0)
#else
# define D(x)  ((void) 0)
#endif

#define NEWD(str, typ) \
   eet_data_descriptor_new(str, sizeof(typ), \
			      (void *(*) (void *))evas_list_next, \
			      (void *(*) (void *, void *))evas_list_append, \
			      (void *(*) (void *))evas_list_data, \
			      (void *(*) (void *))evas_list_free, \
			      (void  (*) (void *, int (*) (void *, const char *, void *, void *), void *))evas_hash_foreach, \
			      (void *(*) (void *, const char *, void *))evas_hash_add, \
			      (void  (*) (void *))evas_hash_free)

#define FREED(eed) \
   if (eed) \
       { \
	  eet_data_descriptor_free((eed)); \
	  (eed) = NULL; \
       }
#define INEWI(str, it, type) \
   EET_DATA_DESCRIPTOR_ADD_BASIC(_e_fm_icon_meta_edd, E_Fm_Icon_Metadata, str, it, type)
#define NEWI(str, it, type) \
   EET_DATA_DESCRIPTOR_ADD_BASIC(_e_fm_dir_meta_edd, E_Fm_Dir_Metadata, str, it, type)
#define NEWL(str, it, type) \
   EET_DATA_DESCRIPTOR_ADD_LIST(_e_fm_dir_meta_edd, E_Fm_Dir_Metadata, str, it, type)

static void                _e_fm_smart_add(Evas_Object *object);
static void                _e_fm_smart_del(Evas_Object *object);
static void                _e_fm_smart_move(Evas_Object *object, Evas_Coord x, Evas_Coord y);
static void                _e_fm_smart_resize(Evas_Object *object, Evas_Coord w, Evas_Coord h);
static void                _e_fm_smart_show(Evas_Object *object);
static void                _e_fm_smart_hide(Evas_Object *object);
static void                _e_fm_smart_color_set(Evas_Object *obj, int r, int g, int b, int a);
static void                _e_fm_smart_clip_set(Evas_Object *obj, Evas_Object * clip);
static void                _e_fm_smart_clip_unset(Evas_Object *obj);

static void                _e_fm_redraw(E_Fm_Smart_Data *sd);

static void                _e_fm_file_menu_open(void *data, E_Menu *m, E_Menu_Item *mi);
static void                _e_fm_file_menu_copy(void *data, E_Menu *m, E_Menu_Item *mi);
static void                _e_fm_file_menu_cut(void *data, E_Menu *m, E_Menu_Item *mi);
static void                _e_fm_file_menu_paste(void *data, E_Menu *m, E_Menu_Item *mi);
static void                _e_fm_file_menu_rename(void *data, E_Menu *m, E_Menu_Item *mi);
static void                _e_fm_file_menu_delete(void *data, E_Menu *m, E_Menu_Item *mi);
static void                _e_fm_file_menu_properties(void *data, E_Menu *m, E_Menu_Item *mi);
static void                _e_fm_file_delete_yes_cb(void *data, E_Dialog *dia);

static void                _e_fm_menu_new_dir_cb(void *data, E_Menu *m, E_Menu_Item *mi);
static void                _e_fm_menu_arrange_cb(void *data, E_Menu *m, E_Menu_Item *mi);
static void                _e_fm_menu_refresh_cb(void *data, E_Menu *m, E_Menu_Item *mi);

static void                _e_fm_file_rename_start(E_Fm_Icon *icon);
static void 							 _e_fm_file_rename_abort(E_Fm_Icon *icon);
static void                _e_fm_file_rename         (E_Fm_Icon *icon, const char *name);
static void                _e_fm_file_delete         (E_Fm_Icon *icon);

static void                _e_fm_dir_set                (E_Fm_Smart_Data *sd, const char *dir);
static int                _e_fm_dir_files_get          (void *data);
static char               *_e_fm_dir_pop                (const char *path);
static void                _e_fm_file_free              (E_Fm_Icon *icon);
static void                _e_fm_dir_monitor_cb         (void *data, Ecore_File_Monitor *ecore_file_monitor,  Ecore_File_Event event, const char *path);
static void                _e_fm_selections_clear       (E_Fm_Smart_Data *sd);
static void                _e_fm_selections_add         (E_Fm_Icon *icon, Evas_List *icon_ptr);
static void                _e_fm_selections_current_set (E_Fm_Icon *icon, Evas_List *icon_ptr);
static void                _e_fm_selections_rect_add    (E_Fm_Smart_Data *sd, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h);
static void                _e_fm_selections_del         (E_Fm_Icon *icon);

static void                _e_fm_fake_mouse_up_later     (Evas *evas, int button);
static void                _e_fm_fake_mouse_up_all_later (Evas *evas);
static void                _e_fm_fake_mouse_up_cb        (void *data);

static void                _e_fm_key_down_cb        (void *data, Evas *e, Evas_Object *obj, void *event_info);
static void                _e_fm_mouse_down_cb      (void *data, Evas *e, Evas_Object *obj, void *event_info);
static void                _e_fm_mouse_move_cb      (void *data, Evas *e, Evas_Object *obj, void *event_info);
static void                _e_fm_mouse_up_cb        (void *data, Evas *e, Evas_Object *obj, void *event_info);

static void                _e_fm_icon_mouse_down_cb (void *data, Evas *e, Evas_Object *obj, void *event_info);
static void                _e_fm_icon_mouse_up_cb   (void *data, Evas *e, Evas_Object *obj, void *event_info);
static void                _e_fm_icon_mouse_in_cb   (void *data, Evas *e, Evas_Object *obj, void *event_info);
static void                _e_fm_icon_mouse_out_cb  (void *data, Evas *e, Evas_Object *obj, void *event_info);
static void                _e_fm_icon_mouse_move_cb (void *data, Evas *e, Evas_Object *obj, void *event_info);
static int                _e_fm_icon_autoscroll_cb (void *data);


static int                 _e_fm_drag_mouse_up_cb    (void *data, int type, void *event);
static int                 _e_fm_drag_mouse_move_cb    (void *data, int type, void *event);


static void                _e_fm_string_replace(const char *src, const char *key, const char *replacement, char *result, size_t resultsize);

static void                _e_fm_autocomplete(E_Fm_Smart_Data *sd);
static void                _e_fm_icon_select_glob(E_Fm_Smart_Data *sd, const char *glb);
static void                _e_fm_icon_select_up(E_Fm_Smart_Data *sd);
static void                _e_fm_icon_select_down(E_Fm_Smart_Data *sd);
static void                _e_fm_icon_select_left(E_Fm_Smart_Data *sd);
static void                _e_fm_icon_select_right(E_Fm_Smart_Data *sd);
static void                _e_fm_icon_run(E_Fm_Smart_Data *sd);

static int                 _e_fm_xdnd_enter_cb     (void *data, int type, void *event);
static int                 _e_fm_xdnd_leave_cb     (void *data, int type, void *event);
static int                 _e_fm_xdnd_position_cb  (void *data, int type, void *event);
static int                 _e_fm_xdnd_status_cb    (void *data, int type, void *event);
static int                 _e_fm_xdnd_drop_cb      (void *data, int type, void *event);
static int                 _e_fm_xdnd_selection_cb (void *data, int type, void *event);
static int                 _e_fm_xdnd_finished_cb  (void *data, int type, void *event);

static int                 _e_fm_files_sort_name_cb    (void *d1, void *d2);
static int                 _e_fm_files_sort_modtime_cb (void *d1, void *d2);
static int                 _e_fm_files_sort_layout_name_cb    (void *d1, void *d2);

static void                _e_fm_selector_send_file (E_Fm_Icon *icon);
static void                _e_fm_selector_send_hilite_file (E_Fm_Icon *icon);

static char               *_e_fm_dir_meta_dir_id(char *dir);
static int                 _e_fm_dir_meta_load(E_Fm_Smart_Data *sd);
static int                 _e_fm_dir_meta_generate(E_Fm_Smart_Data *sd);
static void                _e_fm_dir_meta_free(E_Fm_Dir_Metadata *m);
static int                 _e_fm_dir_meta_save(E_Fm_Smart_Data *sd);
static void                _e_fm_dir_meta_fill(E_Fm_Dir_Metadata *m, E_Fm_Smart_Data *sd);

static void                _e_fm_menu_action_display(E_Fm_Smart_Data *sd, Evas_Coord dx, Evas_Coord dy, unsigned int timestamp);

static void                _e_fm_menu_context_display(E_Fm_Smart_Data *sd, Evas_Coord dx, Evas_Coord dy, unsigned int timestamp);
static void                _e_fm_menu_actions(void *data, E_Menu *m, E_Menu_Item *mi);
static void                _e_fm_menu_default(void *data, E_Menu *m, E_Menu_Item *mi);


static int                 _e_fm_init_assoc(E_Fm_Smart_Data *sd);

/* free functions */
/******************/
static void                 _e_fm_operation_menu_free(E_Fm_Smart_Data *sd);
static void                 _e_fm_operation_files_free(E_Fm_Smart_Data *sd);
static void                 _e_fm_operation_free(E_Fm_Smart_Data *sd);

static Ecore_Event_Handler *e_fm_drag_mouse_up_handler = NULL;
static Ecore_Event_Handler *e_fm_drag_mouse_move_handler = NULL;
static double               e_fm_grab_time = 0;
static Evas_Smart          *e_fm_smart = NULL;
static char                *meta_path = NULL;
static Eet_Data_Descriptor *_e_fm_dir_meta_edd = NULL;
static Eet_Data_Descriptor *_e_fm_icon_meta_edd = NULL;

/* externally accessible functions */
EAPI int
e_fm_init(void)
{
   char *homedir;
   char  path[PATH_MAX];

   homedir = e_user_homedir_get();
   if (homedir)
     {
	snprintf(path, sizeof(path), "%s/.e/e/fileman/metadata", homedir);
	if (!ecore_file_exists(path))
	  ecore_file_mkpath(path);
	meta_path = strdup(path);
	free(homedir);
     }
   else return 0;

   e_fm_smart = evas_smart_new("e_fm",
			       _e_fm_smart_add, /* add */
			       _e_fm_smart_del, /* del */
			       NULL, NULL, NULL, NULL, NULL,
			       _e_fm_smart_move, /* move */
			       _e_fm_smart_resize, /* resize */
			       _e_fm_smart_show,/* show */
			       _e_fm_smart_hide,/* hide */
			       _e_fm_smart_color_set, /* color_set */
			       _e_fm_smart_clip_set, /* clip_set */
			       _e_fm_smart_clip_unset, /* clip_unset */
			       NULL); /* data*/

   _e_fm_icon_meta_edd = NEWD("E_Fm_Icon_Metadata", E_Fm_Icon_Metadata);
   INEWI("x", x, EET_T_INT);
   INEWI("y", y, EET_T_INT);
   INEWI("w", w, EET_T_INT);
   INEWI("h", h, EET_T_INT);
   INEWI("nm", name, EET_T_STRING);

   _e_fm_dir_meta_edd = NEWD("E_Fm_Dir_Metadata", E_Fm_Dir_Metadata);
   NEWI("nm", name, EET_T_STRING);
   NEWI("bg", bg, EET_T_STRING);
   NEWI("vw", view, EET_T_INT);
   NEWL("fl", files, _e_fm_icon_meta_edd);

   e_fm_mime_init();
   E_EVENT_FM_RECONFIGURE = ecore_event_type_new();
   E_EVENT_FM_DIRECTORY_CHANGE = ecore_event_type_new();
   return 1;
}

EAPI int
e_fm_shutdown(void)
{
   FREED(_e_fm_dir_meta_edd);
   FREED(_e_fm_icon_meta_edd);
   evas_smart_free(e_fm_smart);
   return 1;
}

EAPI Evas_Object *
e_fm_add(Evas *evas)
{
   return evas_object_smart_add(evas, e_fm_smart);
}

EAPI void
e_fm_dir_set(Evas_Object *object, const char *dir)
{
   E_Fm_Smart_Data *sd;

   sd = evas_object_smart_data_get(object);
   if (!sd) return;

   _e_fm_dir_set(sd, dir);
}

EAPI char *
e_fm_dir_get(Evas_Object *object)
{
   E_Fm_Smart_Data *sd;

   sd = evas_object_smart_data_get(object);
   if (!sd) return NULL;

   return strdup(sd->dir);
}

EAPI void
e_fm_e_win_set(Evas_Object *object, E_Win *win)
{
   E_Fm_Smart_Data *sd;

   sd = evas_object_smart_data_get(object);
   if (!sd) return;

   sd->win = win;
}

EAPI E_Win *
e_fm_e_win_get(Evas_Object *object)
{
   E_Fm_Smart_Data *sd;

   sd = evas_object_smart_data_get(object);
   if (!sd) return NULL;

   return sd->win;
}

EAPI void
e_fm_scroll_set(Evas_Object *object, Evas_Coord x, Evas_Coord y)
{
   E_Fm_Smart_Data *sd;

   sd = evas_object_smart_data_get(object);
   if (!sd) return;

   if (x > (sd->child.w - sd->w)) x = sd->child.w - sd->w;
   if (y > (sd->child.h - sd->h)) y = sd->child.h - sd->h;
   if (x < 0) x = 0;
   if (y < 0) y = 0;
   if ((x == sd->child.x) && (y == sd->child.y)) return;
   sd->child.x = x;
   sd->child.y = y;

   e_icon_canvas_xy_freeze(sd->layout);
   evas_object_move(sd->layout, sd->x - sd->child.x, sd->y - sd->child.y);
   e_icon_canvas_xy_thaw(sd->layout);
   evas_object_smart_callback_call(sd->object, "changed", NULL);
}

EAPI void
e_fm_scroll_max_get(Evas_Object *object, Evas_Coord *x, Evas_Coord *y)
{
   E_Fm_Smart_Data *sd;

   sd = evas_object_smart_data_get(object);
   if (!sd) return;

   if (x)
     {
	if (sd->w < sd->child.w) *x = sd->child.w - sd->w;
	else *x = 0;
     }
   if (y)
     {
	if (sd->h < sd->child.h) *y = sd->child.h - sd->h;
	else *y = 0;
     }
}

EAPI void
e_fm_scroll_get(Evas_Object *object, Evas_Coord *x, Evas_Coord *y)
{
   E_Fm_Smart_Data *sd;

   sd = evas_object_smart_data_get(object);
   if (!sd) return;

   if (x) *x = sd->child.x;
   if (y) *y = sd->child.y;
}

EAPI void
e_fm_geometry_virtual_get(Evas_Object *object, Evas_Coord *w, Evas_Coord *h)
{
   E_Fm_Smart_Data *sd;

   if ((!object) || !(sd = evas_object_smart_data_get(object)))
     return;

   if(w)
     *w = sd->child.w;

   if(h)
     *h = sd->child.h;
}

EAPI void
e_fm_menu_set(Evas_Object *object, E_Menu *menu)
{
   E_Fm_Smart_Data *sd;

   if ((!object) || !(sd = evas_object_smart_data_get(object)))
     return;

   if (menu)
     sd->menu = menu;
}

EAPI E_Menu *
e_fm_menu_get(Evas_Object *object)
{
   E_Fm_Smart_Data *sd;

   if ((!object) || !(sd = evas_object_smart_data_get(object)))
     return NULL;

   return sd->menu;
}

EAPI int
e_fm_freeze(Evas_Object *object)
{
   E_Fm_Smart_Data *sd;

   if ((!object) || !(sd = evas_object_smart_data_get(object)))
     return -1;

   sd->frozen++;
   evas_event_freeze(sd->evas);
   D(("e_fm_freeze: %d\n", sd->frozen));
   return sd->frozen;
}

EAPI int
e_fm_thaw(Evas_Object *object)
{
   E_Fm_Smart_Data *sd;

   if ((!object) || !(sd = evas_object_smart_data_get(object)))
     return -1;

   if (!sd->frozen) return 0;

   sd->frozen--;
   evas_event_thaw(sd->evas);
   D(("e_fm_thaw: %d\n", sd->frozen));
   return sd->frozen;
}

EAPI void
e_fm_selector_enable(Evas_Object *object, void (*func)(Evas_Object *object, char *file, void *data), void (*hilite_func)(Evas_Object *object, char *file, void *data), void *data)
{
   E_Fm_Smart_Data *sd;

   if ((!object) || !(sd = evas_object_smart_data_get(object)))
     return;

   sd->is_selector = 1;
   sd->selector_func = func;
   sd->selector_data = data;
   sd->selector_hilite_func = hilite_func;
}

/* This isnt working yet */
EAPI void
e_fm_background_set(Evas_Object *object, Evas_Object *bg)
{
   E_Fm_Smart_Data *sd;
   Evas_Object *swallow;

   return;

   if ((!object) || !(sd = evas_object_smart_data_get(object)))
     return;

   swallow = NULL;
   swallow = edje_object_part_swallow_get(sd->edje_obj, "background");
   if(swallow)
     edje_object_part_unswallow(sd->edje_obj, swallow);
   edje_object_part_swallow(sd->edje_obj, "background", bg);
}

/* creates the evas_obect of the icon, will be called when the 
 * icon canvas wants to draw again these icon
 */
EAPI Evas_Object *
e_fm_icon_create(void *data)
{
   E_Fm_Icon *icon;

   if(!data) return NULL;
   icon = data;

   icon->icon_obj = e_fm_icon_add(icon->sd->evas);
   e_fm_icon_file_set(icon->icon_obj, icon->file);
   evas_object_event_callback_add(icon->icon_obj, EVAS_CALLBACK_MOUSE_DOWN, _e_fm_icon_mouse_down_cb, icon);
   evas_object_event_callback_add(icon->icon_obj, EVAS_CALLBACK_MOUSE_UP, _e_fm_icon_mouse_up_cb, icon);
   evas_object_event_callback_add(icon->icon_obj, EVAS_CALLBACK_MOUSE_IN, _e_fm_icon_mouse_in_cb, icon);
   evas_object_event_callback_add(icon->icon_obj, EVAS_CALLBACK_MOUSE_OUT, _e_fm_icon_mouse_out_cb, icon);
   evas_object_event_callback_add(icon->icon_obj, EVAS_CALLBACK_MOUSE_MOVE, _e_fm_icon_mouse_move_cb, icon->sd);
   evas_object_show(icon->icon_obj);
	
   if(icon->state.selected)
     e_fm_icon_signal_emit(icon->icon_obj, "clicked", "");
   icon->state.visible = 1;
     
   return icon->icon_obj;
}


/* called when the icon on the icon_canvas is destroyed
 * (disappears from the viewport)
 */
EAPI void
e_fm_icon_destroy(Evas_Object *obj, void *data)
{
   E_Fm_Icon *icon;

   icon = data;
   e_thumb_generate_end(icon->file->path);
   evas_object_del(icon->icon_obj);
   icon->state.visible = 0;
}

/* local subsystem functions */

int
_e_fm_init_assoc(E_Fm_Smart_Data *sd)
{
   E_Fm_Assoc_App *assoc;

   assoc = E_NEW(E_Fm_Assoc_App, 1);
   assoc->mime = (char*)E_NEW(char *, 5);
   snprintf(assoc->mime, 5, "%s", ".jpg");
   assoc->app = (char*)E_NEW(char *, 7);
   snprintf(assoc->app, 5, "gimp-remote");
   sd->conf.main->apps = evas_list_append(sd->conf.main->apps, assoc);

   assoc = E_NEW(E_Fm_Assoc_App, 1);
   assoc->mime = (char*)E_NEW(char *, 5);
   snprintf(assoc->mime, 5, "%s", ".jpg");
   assoc->app = (char*)E_NEW(char *, 7);
   snprintf(assoc->app, 8, "exhibit");
   sd->conf.main->apps = evas_list_append(sd->conf.main->apps, assoc);

   assoc = E_NEW(E_Fm_Assoc_App, 1);
   assoc->mime = (char*)E_NEW(char *, 5);
   snprintf(assoc->mime, 5, "%s", ".png");
   assoc->app = (char*)E_NEW(char *, 7);
   snprintf(assoc->app, 5, "gimp-remote");
   sd->conf.main->apps = evas_list_append(sd->conf.main->apps, assoc);

   assoc = E_NEW(E_Fm_Assoc_App, 1);
   assoc->mime = (char*)E_NEW(char *, 5);
   snprintf(assoc->mime, 5, "%s", ".png");
   assoc->app = (char*)E_NEW(char *, 7);
   snprintf(assoc->app, 8, "exhibit");
   sd->conf.main->apps = evas_list_append(sd->conf.main->apps, assoc);

   assoc = E_NEW(E_Fm_Assoc_App, 1);
   assoc->mime = (char*)E_NEW(char *, 5);
   snprintf(assoc->mime, 5, "%s", ".pdf");
   assoc->app = (char*)E_NEW(char *, 7);
   snprintf(assoc->app, 7, "evince");
   sd->conf.main->apps = evas_list_append(sd->conf.main->apps, assoc);

   assoc = E_NEW(E_Fm_Assoc_App, 1);
   assoc->mime = (char*)E_NEW(char *, 5);
   snprintf(assoc->mime, 5, "%s", ".mp3");
   assoc->app = (char*)E_NEW(char *, 7);
   snprintf(assoc->app, 8, "xmms -Q");
   sd->conf.main->apps = evas_list_append(sd->conf.main->apps, assoc);

   return 1;
}

static void
_e_fm_smart_add(Evas_Object *object)
{
   E_Fm_Smart_Data *sd;

   sd = E_NEW(E_Fm_Smart_Data, 1);
   if (!sd) return;
   sd->object = object;

   sd->meta = NULL;

   sd->icon_info.w = 64;
   sd->icon_info.h = 64;
   sd->icon_info.x_space = 12;
   sd->icon_info.y_space = 10;

   sd->child.x = 0;
   sd->child.y = 0;

   sd->timer_int = 0.001;
   sd->timer = NULL;

   sd->evas = evas_object_evas_get(object);
   sd->frozen = 0;
   sd->is_selector = 0;
   sd->edje_obj = edje_object_add(sd->evas);
   evas_object_repeat_events_set(sd->edje_obj, 1);
   e_theme_edje_object_set(sd->edje_obj,
			   "base/theme/fileman",
			   "fileman/smart");
   evas_object_smart_member_add(sd->edje_obj, object);
   evas_object_show(sd->edje_obj);

   sd->event_obj = evas_object_rectangle_add(sd->evas);
   evas_object_smart_member_add(sd->event_obj, object);
   evas_object_color_set(sd->event_obj, 0, 0, 0, 0);
   evas_object_stack_below(sd->event_obj, sd->edje_obj);
   evas_object_show(sd->event_obj);
   evas_object_event_callback_add(sd->event_obj, EVAS_CALLBACK_MOUSE_DOWN,
				  _e_fm_mouse_down_cb, sd);
   evas_object_event_callback_add(sd->event_obj, EVAS_CALLBACK_MOUSE_UP,
				  _e_fm_mouse_up_cb, sd);
   evas_object_event_callback_add(sd->event_obj, EVAS_CALLBACK_MOUSE_MOVE,
				  _e_fm_mouse_move_cb, sd);

   sd->layout = e_icon_canvas_add(sd->evas);
   e_icon_canvas_viewport_set(sd->layout, sd->edje_obj);
   e_icon_canvas_spacing_set(sd->layout, sd->icon_info.x_space, sd->icon_info.y_space);
   evas_object_show(sd->layout);

   sd->clip_obj = evas_object_rectangle_add(sd->evas);
   evas_object_smart_member_add(sd->clip_obj, object);
   evas_object_show(sd->clip_obj);
   evas_object_move(sd->clip_obj, -100003, -100003);
   evas_object_resize(sd->clip_obj, 200006, 200006);
   evas_object_color_set(sd->clip_obj, 255, 255, 255, 255);

   evas_object_clip_set(sd->edje_obj, sd->clip_obj);

   edje_object_part_swallow(sd->edje_obj, "icons", sd->layout);

   sd->selection.band.obj = edje_object_add(sd->evas);
   evas_object_smart_member_add(sd->selection.band.obj, object);
   e_theme_edje_object_set(sd->selection.band.obj,
			   "base/theme/fileman/rubberband",
			   "fileman/rubberband");

   evas_object_focus_set(sd->object, 1);
   evas_object_event_callback_add(sd->object, EVAS_CALLBACK_KEY_DOWN, _e_fm_key_down_cb, sd);

   sd->event_handlers = NULL;

   
   sd->event_handlers = evas_list_append(sd->event_handlers,
					 ecore_event_handler_add(ECORE_X_EVENT_XDND_ENTER,
								 _e_fm_xdnd_enter_cb,
								 sd));
   sd->event_handlers = evas_list_append(sd->event_handlers,
					 ecore_event_handler_add(ECORE_X_EVENT_XDND_LEAVE,
								 _e_fm_xdnd_leave_cb,
								 sd));
   sd->event_handlers = evas_list_append(sd->event_handlers,
					 ecore_event_handler_add(ECORE_X_EVENT_XDND_STATUS,
								 _e_fm_xdnd_status_cb,
								 sd));
   sd->event_handlers = evas_list_append(sd->event_handlers,
					 ecore_event_handler_add(ECORE_X_EVENT_XDND_POSITION,
								 _e_fm_xdnd_position_cb,
								 sd));
   sd->event_handlers = evas_list_append(sd->event_handlers,
					 ecore_event_handler_add(ECORE_X_EVENT_XDND_DROP,
								 _e_fm_xdnd_drop_cb,
								 sd));
   sd->event_handlers = evas_list_append(sd->event_handlers,
					 ecore_event_handler_add(ECORE_X_EVENT_SELECTION_NOTIFY,
								 _e_fm_xdnd_selection_cb,
								 sd));
   sd->event_handlers = evas_list_append(sd->event_handlers,
					 ecore_event_handler_add(ECORE_X_EVENT_XDND_FINISHED,
								 _e_fm_xdnd_finished_cb,
								 sd));
   sd->monitor = NULL;
   sd->position = 0.0;

   sd->conf.main_edd = E_CONFIG_DD_NEW("E_Fm_Config", E_Fm_Config);

#undef T
#undef DD
#define T E_Fm_Config
#define DD sd->conf.main_edd
   E_CONFIG_VAL(DD, T, width, INT);
   E_CONFIG_VAL(DD, T, height, INT);

   sd->conf.main = e_config_domain_load("efm", sd->conf.main_edd);
   if (!sd->conf.main)
     {
	/* no saved config */
	sd->conf.main = E_NEW(E_Fm_Config, 1);
	sd->conf.main->width = 640;
	sd->conf.main->height = 480;
     }
   _e_fm_init_assoc(sd);
   sd->state =  E_FILEMAN_STATE_IDLE;

   evas_object_smart_data_set(object, sd);
}

static void
_e_fm_smart_del(Evas_Object *object)
{
   E_Fm_Smart_Data *sd;

   if ((!object) || !(sd = evas_object_smart_data_get(object)))
     return;

   e_config_domain_save("efm", sd->conf.main_edd, sd->conf.main);

   if(sd->timer)
     {
	ecore_timer_del(sd->timer);
	sd->timer = NULL;
     }

   if (sd->monitor) ecore_file_monitor_del(sd->monitor);
   sd->monitor = NULL;

   while (sd->event_handlers)
     {
	ecore_event_handler_del(sd->event_handlers->data);
	sd->event_handlers = evas_list_remove_list(sd->event_handlers, sd->event_handlers);
     }

   evas_event_freeze(evas_object_evas_get(object));
   while (sd->icons)
     {
	_e_fm_file_free(sd->icons->data);
	sd->icons = evas_list_remove_list(sd->icons, sd->icons);
     }

   evas_object_del(sd->selection.band.obj);
   evas_object_del(sd->clip_obj);
   evas_object_del(sd->edje_obj);
   evas_object_del(sd->layout);
   if (sd->entry_obj) evas_object_del(sd->entry_obj);
   if (sd->menu) e_object_del(E_OBJECT(sd->menu));

   evas_event_thaw(evas_object_evas_get(object));
   evas_hash_free(sd->mime_menu_hash);

   free(sd->dir);
   free(sd);
}

static void
_e_fm_smart_move(Evas_Object *object, Evas_Coord x, Evas_Coord y)
{
   E_Fm_Smart_Data *sd;

   sd = evas_object_smart_data_get(object);
   if (!sd) return;

   sd->x = x;
   sd->y = y;
   evas_object_move(sd->edje_obj, x, y);
   evas_object_move(sd->clip_obj, x, y);
   evas_object_move(sd->event_obj, x, y);
}

static void
_e_fm_smart_resize(Evas_Object *object, Evas_Coord w, Evas_Coord h)
{
   E_Fm_Smart_Data *sd;
   E_Event_Fm_Reconfigure *ev;

   if ((!object) || !(sd = evas_object_smart_data_get(object)))
     return;

   evas_object_resize(sd->edje_obj, w, h);
   evas_object_resize(sd->clip_obj, w, h);
   evas_object_resize(sd->event_obj, w, h);
   //   e_icon_canvas_width_fix(sd->layout, w);
   //   e_icon_canvas_virtual_size_get(sd->layout, &sd->child.w, &sd->child.h);
   sd->conf.main->width = w;
   sd->conf.main->height = h;

   sd->w = w;
   sd->h = h;

   evas_object_smart_callback_call(sd->object, "changed", NULL);

   if(sd->frozen)
     return;

   ev = E_NEW(E_Event_Fm_Reconfigure, 1);
   if (ev)
     {
	Evas_Coord w, h;

	evas_object_geometry_get(sd->layout, NULL, NULL, &w, &h);
	ev->object = sd->object;
	ev->w = sd->child.w;
	ev->h = sd->child.h;
	//ecore_event_add(E_EVENT_FM_RECONFIGURE, ev, NULL, NULL);
     }
}

static void
_e_fm_smart_show(Evas_Object *object)
{
   E_Fm_Smart_Data *sd;

   if ((!object) || !(sd = evas_object_smart_data_get(object)))
     return;

   evas_object_show(sd->edje_obj);
   evas_object_show(sd->clip_obj);
}

static void
_e_fm_smart_hide(Evas_Object *object)
{
   E_Fm_Smart_Data *sd;

   if ((!object) || !(sd = evas_object_smart_data_get(object)))
     return;

   evas_object_hide(sd->clip_obj);
}

static void
_e_fm_smart_color_set(Evas_Object *object, int r, int g, int b, int a)
{
   E_Fm_Smart_Data *sd;

   if ((!object) || !(sd = evas_object_smart_data_get(object)))
     return;

   evas_object_color_set(sd->clip_obj, r, g, b, a);
}

static void
_e_fm_smart_clip_set(Evas_Object *object, Evas_Object *clip)
{
   E_Fm_Smart_Data *sd;

   if ((!object) || !(sd = evas_object_smart_data_get(object)))
     return;

   evas_object_clip_set(sd->clip_obj, clip);
}

static void
_e_fm_smart_clip_unset(Evas_Object *object)
{
   E_Fm_Smart_Data *sd;

   if ((!object) || !(sd = evas_object_smart_data_get(object)))
     return;

   evas_object_clip_unset(sd->clip_obj);
}

static void
_e_fm_redraw(E_Fm_Smart_Data *sd)
{
   E_Event_Fm_Reconfigure *ev;

   e_icon_canvas_redraw_force(sd->layout);

   if(sd->frozen)
     return;

   ev = E_NEW(E_Event_Fm_Reconfigure, 1);
   if (ev)
     {
	Evas_Coord w, h;
	evas_object_geometry_get(sd->layout, NULL, NULL, &w, &h);

	ev->object = sd->object;
	ev->w = sd->child.w;
	ev->h = sd->child.h;
	//ecore_event_add(E_EVENT_FM_RECONFIGURE, ev, NULL, NULL);
     }
}

static void
_e_fm_file_rename(E_Fm_Icon *icon, const char* name) // could be better called _e_fm_file_rename_apply
{
   e_fm_icon_edit_entry_set(icon->icon_obj, NULL);
   evas_object_focus_set(icon->sd->entry_obj, 0);
   evas_object_del(icon->sd->entry_obj);
   icon->sd->entry_obj = NULL;
   if (!name || !name[0])
     return;

   if (e_fm_file_rename(icon->file, name))
     {
	e_fm_icon_title_set(icon->icon_obj, name);
     }
   evas_object_focus_set(icon->sd->object, 1);
   icon->sd->state = E_FILEMAN_STATE_IDLE;

}

static void
_e_fm_file_rename_abort(E_Fm_Icon *icon)
{
   e_fm_icon_edit_entry_set(icon->icon_obj, NULL);
   evas_object_focus_set(icon->sd->entry_obj, 0);
   evas_object_del(icon->sd->entry_obj);
   icon->sd->entry_obj = NULL;
   e_fm_icon_title_set(icon->icon_obj, icon->file->name);
   evas_object_focus_set(icon->sd->object, 1);
   icon->sd->state = E_FILEMAN_STATE_IDLE;

}

static void
_e_fm_file_delete(E_Fm_Icon *icon)
{
   if (!e_fm_file_delete(icon->file))
     {
	E_Dialog *dia;
	char text[PATH_MAX + 256];

	dia = e_dialog_new(icon->sd->win->container);
	e_dialog_button_add(dia, _("OK"), NULL, NULL, NULL);
	e_dialog_button_focus_num(dia, 1);
	e_dialog_title_set(dia, _("Error"));
	snprintf(text, PATH_MAX + 256, _("Could not delete  <br><b>%s</b>"), icon->file->path);
	e_dialog_text_set(dia, text);
	e_dialog_show(dia);
	return;
     }

   icon->sd->icons = evas_list_remove(icon->sd->icons, icon);
   e_icon_canvas_freeze(icon->sd->layout);
   e_icon_canvas_unpack(icon->icon_obj);
   e_icon_canvas_thaw(icon->sd->layout);
   _e_fm_redraw(icon->sd);
   _e_fm_file_free(icon);
}


/* FIXME delete this */   
static void
_e_fm_file_menu_open(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Fm_Icon *icon;
   E_Fm_Assoc_App *assoc;
   Evas_List *l;
   icon = data;

}

static void
_e_fm_file_menu_copy(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Fm_Icon *icon;

   icon = data;
}

static void
_e_fm_file_menu_cut(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Fm_Icon *icon;

   icon = data;
}

static void
_e_fm_file_menu_paste(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Fm_Icon *icon;

   icon = data;
}

static void
_e_fm_file_menu_rename(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Fm_Icon *icon;

   icon = data;
   _e_fm_file_rename_start(icon);
}

static void
_e_fm_file_rename_start(E_Fm_Icon *icon)
{
   icon->sd->state = E_FILEMAN_STATE_RENAME;
   icon->sd->entry_obj = e_entry_add(icon->sd->evas);
   evas_object_focus_set(icon->sd->entry_obj, 1);
   evas_object_show(icon->sd->entry_obj);
   e_entry_cursor_show(icon->sd->entry_obj);

   e_fm_icon_edit_entry_set(icon->icon_obj, icon->sd->entry_obj);
   e_fm_icon_title_set(icon->icon_obj, "");

   // e_fm_mouse_up_handler = ecore_event_handler_add(ECORE_X_EVENT_MOUSE_BUTTON_UP,
   //					   _e_fm_win_mouse_up_cb, icon);
   //e_grabinput_get(icon->sd->win->evas_win, 1, icon->sd->win->evas_win);
   e_entry_cursor_move_at_start(icon->sd->entry_obj);
   e_entry_text_set(icon->sd->entry_obj, icon->file->name);

   // e_fm_grab_time = ecore_time_get();
}

static void
_e_fm_file_menu_delete(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Fm_Icon *icon;
   E_Dialog *dia;
   char text[PATH_MAX + 256];

   icon = data;

   dia = e_dialog_new(icon->sd->win->container);
   e_dialog_button_add(dia, _("Yes"), NULL, _e_fm_file_delete_yes_cb, icon);
   e_dialog_button_add(dia, _("No"), NULL, NULL, NULL);
   e_dialog_button_focus_num(dia, 1);
   e_dialog_title_set(dia, _("Confirm"));
   snprintf(text, PATH_MAX + 256, _(" Are you sure you want to delete <br><b>%s</b> ?"), icon->file->name);
   e_dialog_text_set(dia, text);
   e_dialog_show(dia);
}

static void
_e_fm_file_delete_yes_cb(void *data, E_Dialog *dia)
{
   E_Fm_Icon *icon;

   icon = data;

   _e_fm_file_delete(icon);
   e_object_del(E_OBJECT(dia));
}

static void
_e_fm_icon_prop_fill_data(E_Fm_Icon_CFData *cfdata)
{
   /*- BASIC -*/
   if((cfdata->icon->file->mode & (S_IWUSR|S_IWGRP|S_IWOTH)))
     cfdata->protect = 0;
   else
     cfdata->protect = 1;

   if((cfdata->icon->file->mode&S_IRGRP) &&
      (cfdata->icon->file->mode&S_IROTH) &&
      !(cfdata->icon->file->mode&S_IWGRP) &&
      !(cfdata->icon->file->mode&S_IWOTH))
     cfdata->readwrite = 0;
   else if((cfdata->icon->file->mode&S_IWGRP) &&
	   (cfdata->icon->file->mode&S_IWOTH))
     cfdata->readwrite = 1;
   else if(!(cfdata->icon->file->mode & (S_IROTH|S_IWOTH|S_IRGRP|S_IWGRP)))
     cfdata->readwrite = 2;
   else
     cfdata->readwrite = 3;

   /*- ADVANCED -*/
   /*- user -*/
   if((cfdata->icon->file->mode & S_IRUSR))
     cfdata->user.r = 1;
   else
     cfdata->user.r = 0;
   if((cfdata->icon->file->mode & S_IWUSR))
     cfdata->user.w = 1;
   else
     cfdata->user.w = 0;
   if((cfdata->icon->file->mode & S_IXUSR))
     cfdata->user.x = 1;
   else
     cfdata->user.x = 0;
   /*- group -*/
   if((cfdata->icon->file->mode & S_IRGRP))
     cfdata->group.r = 1;
   else
     cfdata->group.r = 0;
   if((cfdata->icon->file->mode & S_IWGRP))
     cfdata->group.w = 1;
   else
     cfdata->group.w = 0;
   if((cfdata->icon->file->mode & S_IXGRP))
     cfdata->group.x = 1;
   else
     cfdata->group.x = 0;
   /*- world -*/
   if((cfdata->icon->file->mode & S_IROTH))
     cfdata->world.r = 1;
   else
     cfdata->world.r = 0;
   if((cfdata->icon->file->mode & S_IWOTH))
     cfdata->world.w = 1;
   else
     cfdata->world.w = 0;
   if((cfdata->icon->file->mode & S_IXOTH))
     cfdata->world.x = 1;
   else
     cfdata->world.x = 0;
}

static void *
_e_fm_icon_prop_create_data(E_Config_Dialog *cfd)
{
   E_Fm_Icon_CFData *cfdata;

   cfdata = E_NEW(E_Fm_Icon_CFData, 1);
   if (!cfdata) return NULL;
   cfdata->icon = cfd->data;
   _e_fm_icon_prop_fill_data(cfdata);
   return cfdata;
}

static void
_e_fm_icon_prop_free_data(E_Config_Dialog *cfd, void *data)
{
   free(data);
}

static int
_e_fm_icon_prop_basic_apply_data(E_Config_Dialog *cfd, void *data)
{
   E_Fm_Icon *icon;
   E_Fm_Icon_CFData *cfdata;

   cfdata = data;
   icon = cfdata->icon;

   switch (cfdata->readwrite)
     {
      case 0:
	D(("_e_fm_icon_prop_basic_apply_data: read (%s)\n", icon->file->name));
	icon->file->mode |= (S_IRGRP | S_IROTH);
	icon->file->mode &= (~S_IWGRP & ~S_IWOTH);
	break;
      case 1:
	D(("_e_fm_icon_prop_basic_apply_data: write (%s)\n", icon->file->name));
	icon->file->mode |= (S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	break;
      case 2:
	D(("_e_fm_icon_prop_basic_apply_data: hide (%s)\n", icon->file->name));
	icon->file->mode &= (~S_IRGRP & ~S_IROTH & ~S_IWGRP & ~S_IWOTH);
	break;
     }

   if (cfdata->protect)
     {
	D(("_e_fm_icon_prop_basic_apply_data: protect (%s)\n", icon->file->name));
	icon->file->mode &= (~S_IWUSR & ~S_IWGRP & ~S_IWOTH);
     }
   else
     {
	D(("_e_fm_icon_prop_basic_apply_data: unprotect (%s)\n", icon->file->name));
	icon->file->mode |= S_IWUSR;
     }

   chmod(icon->file->path, icon->file->mode);

   return 1;
}

static int
_e_fm_icon_prop_advanced_apply_data(E_Config_Dialog *cfd, void *data)
{
   E_Fm_Icon *icon;
   E_Fm_Icon_CFData *cfdata;

   cfdata = data;
   icon = cfdata->icon;

   if(cfdata->user.r)
     icon->file->mode |= S_IRUSR;
   else
     icon->file->mode &= ~S_IRUSR;
   if(cfdata->user.w)
     icon->file->mode |= S_IWUSR;
   else
     icon->file->mode &= ~S_IWUSR;
   if(cfdata->user.x)
     icon->file->mode |= S_IXUSR;
   else
     icon->file->mode &= ~S_IXUSR;

   if(cfdata->group.r)
     icon->file->mode |= S_IRGRP;
   else
     icon->file->mode &= ~S_IRGRP;
   if(cfdata->group.w)
     icon->file->mode |= S_IWGRP;
   else
     icon->file->mode &= ~S_IWGRP;
   if(cfdata->group.x)
     icon->file->mode |= S_IXGRP;
   else
     icon->file->mode &= ~S_IXGRP;

   if(cfdata->world.r)
     icon->file->mode |= S_IROTH;
   else
     icon->file->mode &= ~S_IROTH;
   if(cfdata->world.w)
     icon->file->mode |= S_IWOTH;
   else
     icon->file->mode &= ~S_IWOTH;
   if(cfdata->world.x)
     icon->file->mode |= S_IXOTH;
   else
     icon->file->mode &= ~S_IXOTH;

   chmod(icon->file->path, icon->file->mode);

   return 1;
}

static Evas_Object *
_e_fm_icon_prop_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, void *data)
{
   E_Fm_Icon *icon;
   E_Fm_Icon_CFData *cfdata;
   char size[64];
   char text[512];
   Evas_Object *o, *ol;
   E_Radio_Group *rg;
   Evas_Object *img;

   cfdata = data;
   icon = cfdata->icon;

   _e_fm_icon_prop_fill_data(cfdata);

   snprintf(size, 64, "%ld", icon->file->size / 1024);

   ol = e_widget_list_add(evas, 0, 0);

   o = e_widget_frametable_add(evas, _("General"), 0);

   img = e_fm_icon_add(evas);
   e_fm_icon_file_set(img, e_fm_file_new(icon->file->path));
   e_fm_icon_title_set(img, "");
   e_widget_frametable_object_append(o, e_widget_image_add_from_object(evas, img, 48, 48),
				     2, 1, 2, 2,
				     0, 0, 0, 0);

   snprintf(text, 512, _("File:"));
   e_widget_frametable_object_append(o, e_widget_label_add(evas, text),
				     0, 0, 1, 1,
				     1, 1, 1, 1);
   snprintf(text, 512, "%s", icon->file->name);
   e_widget_frametable_object_append(o, e_widget_label_add(evas, text),
				     1, 0, 1, 1,
				     1, 1, 1, 1);

   snprintf(text, 512, _("Size:"));
   e_widget_frametable_object_append(o, e_widget_label_add(evas, text),
				     0, 1, 1, 1,
				     1, 1, 1, 1);
   snprintf(text, 512, "%s Kb", size);
   e_widget_frametable_object_append(o, e_widget_label_add(evas, text),
				     1, 1, 1, 1,
				     1, 1, 1, 1);

   snprintf(text, 512, _("Type:"));
   e_widget_frametable_object_append(o, e_widget_label_add(evas, text),
				     0, 2, 1, 1,
				     1, 1, 1, 1);
   snprintf(text, 512, "%s", icon->file->mime->name);
   e_widget_frametable_object_append(o, e_widget_label_add(evas, text),
				     1, 2, 1, 1,
				     1, 1, 1, 1);

   e_widget_frametable_object_append(o, e_widget_check_add(evas, _("Protect this file"), &(cfdata->protect)),
				     0, 3, 2, 1,
				     1, 1, 1, 1);

   rg = e_widget_radio_group_new(&(cfdata->readwrite));

   e_widget_frametable_object_append(o, e_widget_radio_add(evas, _("Let others see this file"), 0, rg),
				     0, 4, 3, 1,
				     1, 1, 1, 1);

   e_widget_frametable_object_append(o, e_widget_radio_add(evas, _("Let others modify this file"), 1, rg),
				     0, 5, 3, 1,
				     1, 1, 1, 1);

   e_widget_frametable_object_append(o, e_widget_radio_add(evas, _("Dont let others see or modify this file"), 2, rg),
				     0, 6, 3, 1,
				     1, 1, 1, 1);

   e_widget_frametable_object_append(o, e_widget_radio_add(evas, _("Custom settings"), 3, rg),
				     0, 7, 3, 1,
				     1, 1, 1, 1);

   e_widget_list_object_append(ol, o, 1, 1, 0.5);

   return ol;
}

static Evas_Object *
_e_fm_icon_prop_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, void *data)
{
   Evas_Object *o, *ob, *of;
   E_Fm_Icon *icon;
   E_Fm_Icon_CFData *cfdata;
   struct group *grp;
   struct passwd *usr;
   struct tm *t;
   char lastaccess[128], lastmod[128];

   cfdata = data;
   icon = cfdata->icon;

   _e_fm_icon_prop_fill_data(cfdata);

   usr = getpwuid(icon->file->owner);

   grp = getgrgid(icon->file->group);

   t = gmtime(&icon->file->atime);
   strftime(lastaccess, 128, "%a %b %d %T %Y", t);

   t = gmtime(&icon->file->mtime);
   strftime(lastmod, 128, "%a %b %d %T %Y", t);

   o = e_widget_list_add(evas, 0, 0);

   of = e_widget_frametable_add(evas, _("File Info:"), 0);
   ob = e_widget_label_add(evas, _("Owner:"));
   e_widget_frametable_object_append(of, ob, 0, 0, 1, 1, 1, 1, 1, 1);
   ob = e_widget_label_add(evas, strdup(usr->pw_name));
   e_widget_frametable_object_append(of, ob, 1, 0, 1, 1, 1, 1, 1, 1);

   ob = e_widget_label_add(evas, _("Group:"));
   e_widget_frametable_object_append(of, ob, 0, 1, 1, 1, 1, 1, 1, 1);
   ob = e_widget_label_add(evas, strdup(grp->gr_name));
   e_widget_frametable_object_append(of, ob, 1, 1, 1, 1, 1, 1, 1, 1);

   ob = e_widget_label_add(evas, _("Last Access:"));
   e_widget_frametable_object_append(of, ob, 0, 2, 1, 1, 1, 1, 1, 1);
   ob = e_widget_label_add(evas, lastaccess);
   e_widget_frametable_object_append(of, ob, 1, 2, 1, 1, 1, 1, 1, 1);

   ob = e_widget_label_add(evas, _("Last Modified:"));
   e_widget_frametable_object_append(of, ob, 0, 3, 1, 1, 1, 1, 1, 1);
   ob = e_widget_label_add(evas, lastmod);
   e_widget_frametable_object_append(of, ob, 1, 3, 1, 1, 1, 1, 1, 1);

   e_widget_list_object_append(o, of, 1, 1, 0.5);

   of = e_widget_frametable_add(evas, _("Permissions:"), 0);
   ob = e_widget_label_add(evas, _("Me"));
   e_widget_frametable_object_append(of, ob, 0, 0, 3, 1, 1, 1, 1, 1);
   ob = e_widget_check_add(evas, _("r"), &(cfdata->user.r));
   e_widget_frametable_object_append(of, ob, 0, 1, 1, 1, 1, 1, 1, 1);
   ob = e_widget_check_add(evas, _("w"), &(cfdata->user.w));
   e_widget_frametable_object_append(of, ob, 1, 1, 1, 1, 1, 1, 1, 1);
   ob = e_widget_check_add(evas, _("x"), &(cfdata->user.x));
   e_widget_frametable_object_append(of, ob, 2, 1, 1, 1, 1, 1, 1, 1);

   ob = e_widget_label_add(evas, _("My Group"));
   e_widget_frametable_object_append(of, ob, 0, 2, 3, 1, 1, 1, 1, 1);
   ob = e_widget_check_add(evas, _("r"), &(cfdata->group.r));
   e_widget_frametable_object_append(of, ob, 0, 3, 1, 1, 1, 1, 1, 1);
   ob = e_widget_check_add(evas, _("w"), &(cfdata->group.w));
   e_widget_frametable_object_append(of, ob, 1, 3, 1, 1, 1, 1, 1, 1);
   ob = e_widget_check_add(evas, _("x"), &(cfdata->group.x));
   e_widget_frametable_object_append(of, ob, 2, 3, 1, 1, 1, 1, 1, 1);

   ob = e_widget_label_add(evas, _("Everyone"));
   e_widget_frametable_object_append(of, ob, 0, 4, 3, 1, 1, 1, 1, 1);
   ob = e_widget_check_add(evas, _("r"), &(cfdata->world.r));
   e_widget_frametable_object_append(of, ob, 0, 5, 1, 1, 1, 1, 1, 1);
   ob = e_widget_check_add(evas, _("w"), &(cfdata->world.w));
   e_widget_frametable_object_append(of, ob, 1, 5, 1, 1, 1, 1, 1, 1);
   ob = e_widget_check_add(evas, _("x"), &(cfdata->world.x));
   e_widget_frametable_object_append(of, ob, 2, 5, 1, 1, 1, 1, 1, 1);

   e_widget_list_object_append(o, of, 1, 1, 0.5);

   return o;
}

static void
_e_fm_file_menu_properties(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Fm_Icon *icon;
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;

   icon = data;

   v = E_NEW(E_Config_Dialog_View, 1);
   if (v)
      {
         /* methods */
         v->create_cfdata           = _e_fm_icon_prop_create_data;
         v->free_cfdata             = _e_fm_icon_prop_free_data;
         v->basic.apply_cfdata      = _e_fm_icon_prop_basic_apply_data;
         v->basic.create_widgets    = _e_fm_icon_prop_basic_create_widgets;
         v->advanced.apply_cfdata   = _e_fm_icon_prop_advanced_apply_data;
         v->advanced.create_widgets = _e_fm_icon_prop_advanced_create_widgets;
         /* create config diaolg for NULL object/data */
         cfd = e_config_dialog_new(icon->sd->win->container, _("Properties"), NULL, 0, v, icon);
      }
}

static void
_e_fm_menu_new_dir_cb(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Fm_Smart_Data *sd;

   sd = data;
}

static void
_e_fm_menu_arrange_cb(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Fm_Smart_Data *sd;

   sd = data;

   switch (e_menu_item_num_get(mi))
     {
      case E_FILEMAN_CANVAS_ARRANGE_NAME:
	sd->icons = evas_list_sort(sd->icons, evas_list_count(sd->icons), _e_fm_files_sort_name_cb);
	sd->arrange = E_FILEMAN_CANVAS_ARRANGE_NAME;
	_e_fm_redraw(sd);
	break;

      case E_FILEMAN_CANVAS_ARRANGE_MODTIME:
	sd->icons = evas_list_sort(sd->icons, evas_list_count(sd->icons), _e_fm_files_sort_modtime_cb);
	sd->arrange = E_FILEMAN_CANVAS_ARRANGE_MODTIME;
	_e_fm_redraw(sd);
	break;
     }
}

static void
_e_fm_menu_refresh_cb(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Fm_Smart_Data *sd;

   sd = data;
   /* TODO: Update files */
   _e_fm_redraw(sd);
}

static void
_e_fm_dir_set(E_Fm_Smart_Data *sd, const char *dir)
{
   Evas_List             *list;
   Ecore_Sheap            *heap;
   char                   *f;
   int                     type;
   DIR                    *dir_handle;
   struct dirent          *dir_entry;

   if (!dir) return;
   if ((sd->dir) && (!strcmp(sd->dir, dir))) return;

   if (!(dir_handle = opendir(dir))) return;

   /* save the old meta */
   if(sd->meta)
     {
	_e_fm_dir_meta_save(sd);
	_e_fm_dir_meta_free(sd->meta);
	sd->meta = NULL;
     }

   /* Get list of files for directory */
   type = E_FM_FILE_TYPE_NORMAL;
   list = NULL;
   while((dir_entry = readdir(dir_handle)))
     {
	if ((!strcmp(dir_entry->d_name, ".") || (!strcmp (dir_entry->d_name, "..")))) continue;
	if ((dir_entry->d_name[0] == '.') && (!(type & E_FM_FILE_TYPE_HIDDEN))) continue;
	f = strdup(dir_entry->d_name);
	list = evas_list_append(list, f);
     }
   closedir(dir_handle);

   /* Sort file list */
   heap = ecore_sheap_new(ECORE_COMPARE_CB(strcasecmp), evas_list_count(list));
   while (list)
     {
	f = list->data;
	ecore_sheap_insert(heap, f);
	list = evas_list_remove_list(list, list);
     }

   while ((f = ecore_sheap_extract(heap)))
     sd->files_raw = evas_list_append(sd->files_raw, f);

   ecore_sheap_destroy(heap);

   /* Set new directory name */
   if (sd->dir) free (sd->dir);
   sd->dir = strdup(dir);

   /* Load meta data for this directory and the Icons in the directory */
   _e_fm_dir_meta_load(sd);

   if (!sd->meta)
     e_icon_canvas_width_fix(sd->layout, sd->w);

   /* Reset position */
   sd->position = 0.0;

   /* Clear old selection */
   _e_fm_selections_clear(sd);

   /* Remove old files */
   while (sd->icons)
     {
	_e_fm_file_free(sd->icons->data);
	sd->icons = evas_list_remove_list(sd->icons, sd->icons);
     }
   e_icon_canvas_reset(sd->layout);

   /* Get new files */
   if (sd->monitor) ecore_file_monitor_del(sd->monitor);
   sd->monitor = ecore_file_monitor_add(sd->dir, _e_fm_dir_monitor_cb, sd);

   /* Get special prev dir */
   if (strcmp(sd->dir, "/"))
     {
	E_Fm_Icon *icon;
	char       path[PATH_MAX];

	icon = E_NEW(E_Fm_Icon, 1);
	if (icon)
	  {
	     snprintf(path, sizeof(path), "%s/..", sd->dir);
	     icon->file = e_fm_file_new(path);
	     icon->file->mode = 0040000;
	     icon->file->type = E_FM_FILE_TYPE_DIRECTORY;
	     e_fm_mime_set(icon->file);
	     icon->icon_obj = e_fm_icon_add(sd->evas);
	     icon->sd = sd;
	     e_fm_icon_file_set(icon->icon_obj, icon->file);
	     sd->icons = evas_list_prepend(sd->icons, icon);
			    
	     e_icon_canvas_pack(sd->layout, icon->icon_obj, e_fm_icon_create, e_fm_icon_destroy, icon);
	     evas_object_event_callback_add(icon->icon_obj, EVAS_CALLBACK_MOUSE_DOWN, _e_fm_icon_mouse_down_cb, icon);
	     evas_object_event_callback_add(icon->icon_obj, EVAS_CALLBACK_MOUSE_UP, _e_fm_icon_mouse_up_cb, icon);
	     evas_object_event_callback_add(icon->icon_obj, EVAS_CALLBACK_MOUSE_IN, _e_fm_icon_mouse_in_cb, icon);
	     evas_object_event_callback_add(icon->icon_obj, EVAS_CALLBACK_MOUSE_OUT, _e_fm_icon_mouse_out_cb, icon);
	  }
     }

   if(sd->timer)
     ecore_timer_del(sd->timer);
   sd->timer = ecore_timer_add(sd->timer_int, _e_fm_dir_files_get, sd);
}

static int
_e_fm_dir_files_get(void *data)
{
   E_Fm_Smart_Data *sd;
   E_Fm_Icon       *icon;
   char             path[PATH_MAX];
   int              i;

   i = 0;
   sd = data;

   e_icon_canvas_freeze(sd->layout);

   /* add two files per each timer call */
   while (i < 2)
     {
	char *f;

	if(!sd->files_raw)
	  break;
	f = sd->files_raw->data;
	icon = E_NEW(E_Fm_Icon, 1);
	if (!icon) continue;
	snprintf(path, sizeof(path), "%s/%s", sd->dir, f);
	icon->file = e_fm_file_new(path);
	if (!icon->file)
	  {
	     E_FREE(icon);
	  }
	else
	  {
	     icon->icon_obj = e_fm_icon_add(sd->evas);
	     icon->sd = sd;
	     e_fm_icon_file_set(icon->icon_obj, icon->file);
	     sd->icons = evas_list_append(sd->icons, icon);
	     evas_object_event_callback_add(icon->icon_obj, EVAS_CALLBACK_MOUSE_DOWN, _e_fm_icon_mouse_down_cb, icon);
	     evas_object_event_callback_add(icon->icon_obj, EVAS_CALLBACK_MOUSE_UP, _e_fm_icon_mouse_up_cb, icon);
	     evas_object_event_callback_add(icon->icon_obj, EVAS_CALLBACK_MOUSE_IN, _e_fm_icon_mouse_in_cb, icon);
	     evas_object_event_callback_add(icon->icon_obj, EVAS_CALLBACK_MOUSE_OUT, _e_fm_icon_mouse_out_cb, icon);
	     evas_object_event_callback_add(icon->icon_obj, EVAS_CALLBACK_MOUSE_MOVE, _e_fm_icon_mouse_move_cb, sd);
	     evas_object_show(icon->icon_obj);
	     if(sd->meta)
	       {
		  E_Fm_Icon_Metadata *im;
		  if((im = evas_hash_find(sd->meta->files_hash, icon->file->name)) != NULL)
		    {
		       //printf("packing old icon at %d %d\n", im->x, im->y);
		       e_icon_canvas_pack_at_location(sd->layout, icon->icon_obj, e_fm_icon_create, e_fm_icon_destroy, icon, im->x, im->y);
		    }
		  else
		    {
		       E_Fm_Icon_Metadata *im;

		       //printf("packing new icon!\n");
		       im = e_fm_icon_meta_generate(icon->icon_obj);
		       if (im)
			 {
			    sd->meta->files = evas_list_append(sd->meta->files, im);
			    sd->meta->files_hash = evas_hash_add(sd->meta->files_hash, icon->file->name, im);
			 }
		       e_icon_canvas_pack(sd->layout, icon->icon_obj, e_fm_icon_create, e_fm_icon_destroy, icon);
		    }
	       }
	     else
	       {
		  E_Fm_Icon_Metadata *im;

		  //printf("packing new icon!\n");
		  sd->meta = calloc(1, sizeof(E_Fm_Dir_Metadata));
		  sd->meta->files = NULL;
		  im = e_fm_icon_meta_generate(icon->icon_obj);
		  if (im)
		    {
		       sd->meta->files = evas_list_append(sd->meta->files, im);
		       sd->meta->files_hash = evas_hash_add(sd->meta->files_hash, icon->file->name, im);
		    }
		  e_icon_canvas_pack(sd->layout, icon->icon_obj, e_fm_icon_create, e_fm_icon_destroy, icon);
	       }
	  }
	i++;
	sd->files_raw = evas_list_remove_list(sd->files_raw, sd->files_raw);
     }

   e_icon_canvas_thaw(sd->layout);

   e_icon_canvas_virtual_size_get(sd->layout, &sd->child.w, &sd->child.h);
   evas_object_smart_callback_call(sd->object, "changed", NULL);

   if(!sd->files_raw) {
      sd->timer = NULL;
      return 0;
   }
   else
     {
	sd->timer = ecore_timer_add(sd->timer_int, _e_fm_dir_files_get, sd);
	return 0;
     }
}

static char *
_e_fm_dir_pop(const char *path)
{
   char *start, *end, *dir;

   start = strchr(path, '/');
   end = strrchr(path ,'/');

   if (start == end)
     {
	dir = strdup("/");;
     }
   else if ((!start) || (!end))
     {
	dir = strdup("");
     }
   else
     {
	dir = malloc((end - start + 1));
	if (dir)
	  {
	     memcpy(dir, start, end - start);
	     dir[end - start] = 0;
	  }
     }
   return dir;
}

static void
_e_fm_dir_monitor_cb(void *data, Ecore_File_Monitor *ecore_file_monitor,
		       Ecore_File_Event event, const char *path)
{
   E_Fm_Smart_Data *sd;
   char *dir;
   E_Fm_Icon *icon;
   Evas_List *l;

   sd = data;

   switch (event)
     {
      case ECORE_FILE_EVENT_NONE:
	break;
      case ECORE_FILE_EVENT_MODIFIED:
	break;
      case ECORE_FILE_EVENT_DELETED_SELF:
	dir = _e_fm_dir_pop(sd->dir);
	 /* FIXME: we need to fix this, uber hack alert */
	if (sd->win)
	  e_win_title_set(sd->win, dir);
	_e_fm_dir_set(sd, dir);
	free(dir);
	break;

      case ECORE_FILE_EVENT_CREATED_FILE:
      case ECORE_FILE_EVENT_CREATED_DIRECTORY:
	icon = E_NEW(E_Fm_Icon, 1);
	if (!icon) break;
	icon->file = e_fm_file_new(path);
	if (!icon->file)
	  {
	     free(icon);
	     return;
	  }
	icon->icon_obj = e_fm_icon_add(sd->evas);
	icon->sd = sd;
	e_icon_canvas_freeze(sd->layout);
	e_fm_icon_file_set(icon->icon_obj, icon->file);
	evas_object_show(icon->icon_obj);
	e_icon_canvas_pack(sd->layout, icon->icon_obj, e_fm_icon_create, e_fm_icon_destroy, icon);
	evas_object_event_callback_add(icon->icon_obj, EVAS_CALLBACK_MOUSE_DOWN, _e_fm_icon_mouse_down_cb, icon);
	evas_object_event_callback_add(icon->icon_obj, EVAS_CALLBACK_MOUSE_UP, _e_fm_icon_mouse_up_cb, icon);
	evas_object_event_callback_add(icon->icon_obj, EVAS_CALLBACK_MOUSE_IN, _e_fm_icon_mouse_in_cb, icon);
	evas_object_event_callback_add(icon->icon_obj, EVAS_CALLBACK_MOUSE_OUT, _e_fm_icon_mouse_out_cb, icon);
	e_icon_canvas_thaw(sd->layout);
	sd->icons = evas_list_append(sd->icons, icon);
	_e_fm_redraw(sd);
	break;

      case ECORE_FILE_EVENT_DELETED_FILE:
      case ECORE_FILE_EVENT_DELETED_DIRECTORY:
	for (l = sd->icons; l; l = l->next)
	  {
	     icon = l->data;
	     if (!strcmp(icon->file->path, path))
	       {
		  sd->icons = evas_list_remove_list(sd->icons, l);
		  e_icon_canvas_freeze(sd->layout);
		  e_icon_canvas_unpack(icon->icon_obj);
		  e_icon_canvas_thaw(sd->layout);
		  _e_fm_file_free(icon);
		  _e_fm_redraw(sd);
		  break;
	       }
	  }
	break;
     }
}

static void
_e_fm_file_free(E_Fm_Icon *icon)
{
   if(icon->icon_obj)
     {
	e_icon_canvas_unpack(icon->icon_obj);
	evas_object_del(icon->icon_obj);
     }
   if(icon->file)
     e_object_del(E_OBJECT(icon->file));
   /*
    if (file->menu)
    e_object_del(E_OBJECT(file->menu));
    */
   free(icon);
}

static void
_e_fm_selections_clear(E_Fm_Smart_Data *sd)
{
   Evas_List *l;

   D(("_e_fm_selections_clear:\n"));
   for (l = sd->selection.icons; l; l = l->next)
     {
	E_Fm_Icon *icon;

	icon = l->data;
	e_fm_icon_signal_emit(icon->icon_obj, "unclicked", "");
	icon->state.selected = 0;
     }
   sd->selection.icons = evas_list_free(sd->selection.icons);
   sd->selection.band.files = evas_list_free(sd->selection.band.files);
   sd->selection.current.file = NULL;
   sd->selection.current.ptr = NULL;
}

static void
_e_fm_selections_add(E_Fm_Icon *icon, Evas_List *icon_ptr)
{
   icon->sd->selection.current.file = icon;
   icon->sd->selection.current.ptr = icon_ptr;
   if (icon->state.selected) return;
   e_fm_icon_signal_emit(icon->icon_obj, "clicked", "");
   icon->sd->selection.icons = evas_list_append(icon->sd->selection.icons, icon);
   icon->state.selected = 1;
}

static void
_e_fm_selections_del(E_Fm_Icon *icon)
{
   if (!icon->state.selected) return;
   e_fm_icon_signal_emit(icon->icon_obj, "unclicked", "");
   icon->sd->selection.icons = evas_list_remove(icon->sd->selection.icons, icon);
   if (icon->sd->selection.current.file == icon)
     {
	icon->sd->selection.current.file = NULL;
	if (icon->sd->selection.icons)
	  icon->sd->selection.current.file = icon->sd->selection.icons->data;
     }
   icon->state.selected = 0;
}

static void
_e_fm_selections_current_set(E_Fm_Icon *icon, Evas_List *icon_ptr)
{
   icon->sd->selection.current.file = icon;
   icon->sd->selection.current.ptr = icon_ptr;
   if (icon->state.selected) return;
   e_fm_icon_signal_emit(icon->icon_obj, "hilight", "");
}

static void
_e_fm_selections_rect_add(E_Fm_Smart_Data *sd, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   Evas_List *l;

   for (l = sd->icons; l; l = l->next)
     {
	E_Fm_Icon *icon;
	Evas_Coord xx, yy, ww, hh;

	icon = l->data;

	evas_object_geometry_get(icon->icon_obj, &xx, &yy, &ww, &hh);
	if (E_INTERSECTS(x, y, w, h, xx, yy, ww, hh))
	  {
	     if (!evas_list_find(icon->sd->selection.band.files, icon))
	       {
		  if (icon->state.selected)
		    _e_fm_selections_del(icon);
		  else
		    _e_fm_selections_add(icon, l);
		  icon->sd->selection.band.files = evas_list_append(icon->sd->selection.band.files, icon);
	       }
	  }
	else
	  {
	     if (evas_list_find(icon->sd->selection.band.files, icon))
	       {
		  if (icon->state.selected)
		    _e_fm_selections_del(icon);
		  else
		    _e_fm_selections_add(icon, l);
		  icon->sd->selection.band.files = evas_list_remove(icon->sd->selection.band.files, icon);
	       }
	  }
     }
}


/* fake mouse up */
static void
_e_fm_fake_mouse_up_later(Evas *evas, int button)
{
   E_Fm_Fake_Mouse_Up_Info *info;

   info = E_NEW(E_Fm_Fake_Mouse_Up_Info, 1);
   if (!info) return;

   info->canvas = evas;
   info->button = button;
   ecore_job_add(_e_fm_fake_mouse_up_cb, info);
}

static void
_e_fm_fake_mouse_up_all_later(Evas *evas)
{
   _e_fm_fake_mouse_up_later(evas, 1);
   _e_fm_fake_mouse_up_later(evas, 2);
   _e_fm_fake_mouse_up_later(evas, 3);
}

static void
_e_fm_fake_mouse_up_cb(void *data)
{
   E_Fm_Fake_Mouse_Up_Info *info;

   info = data;
   if (!info) return;

   evas_event_feed_mouse_up(info->canvas, info->button, EVAS_BUTTON_NONE, ecore_x_current_time_get(), NULL);
   free(info);
}

/* mouse events on the bg and on the icons */
static void
_e_fm_mouse_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   E_Fm_Smart_Data *sd;
   Evas_Event_Mouse_Down *ev;
   E_Fm_Icon *icon;
   Evas_List *l;
   E_Menu      *mn;
   E_Menu_Item *mi;
   int x, y, w, h;

   ev = event_info;
   sd = data;

   if(sd->state == E_FILEMAN_STATE_RENAME)
     {
	_e_fm_file_rename_abort(sd->selection.current.file);
	return;
     }

   if(!strcmp(edje_object_part_state_get(sd->edje_obj, "typebuffer", NULL), "shown"))
     {
	edje_object_signal_emit(sd->edje_obj, "typebuf_hide", "");
	edje_object_part_text_set(sd->edje_obj, "text", "");
	for (l = sd->icons; l; l = l->next)
	  {
	     icon = l->data;
	     e_fm_icon_signal_emit(icon->icon_obj, "default", "");
	  }
	edje_object_signal_emit(sd->edje_obj, "default", "");
     }

   switch (ev->button)
     {
      case 1:

	if (!evas_key_modifier_is_set(evas_key_modifier_get(sd->evas), "Control"))
	  _e_fm_selections_clear(sd);

	sd->selection.band.enabled = 1;
	evas_object_move(sd->selection.band.obj, ev->canvas.x, ev->canvas.y);
	evas_object_resize(sd->selection.band.obj, 1, 1);
	evas_object_show(sd->selection.band.obj);
	sd->selection.band.x = ev->canvas.x;
	sd->selection.band.y = ev->canvas.y;
	break;

      case 3:
	if (!sd->win) break;

	mn = e_menu_new();
	e_menu_category_set(mn,"fileman/unknown");
	e_menu_category_data_set("fileman/unknown",sd);

	sd->menu = mn;

       /*- Arrange -*/
	mi = e_menu_item_new(mn);
	e_menu_item_label_set(mi, _("Arrange Icons"));
	e_menu_item_icon_edje_set(mi,
				  (char *)e_theme_edje_file_get("base/theme/fileman",
								"fileman/button/arrange"),
				  "fileman/button/arrange");

	mn = e_menu_new();
	e_menu_category_set(mn,"fileman/unknown/arrange");
	e_menu_category_data_set("fileman/unknown/arrange",sd);
	e_menu_item_submenu_set(mi, mn);

	mi = e_menu_item_new(mn);
	e_menu_item_label_set(mi, _("By Name"));
	e_menu_item_radio_set(mi, 1);
	e_menu_item_radio_group_set(mi, 2);
	if (sd->arrange == E_FILEMAN_CANVAS_ARRANGE_NAME) e_menu_item_toggle_set(mi, 1);
	e_menu_item_callback_set(mi, _e_fm_menu_arrange_cb, sd);
	e_menu_item_icon_edje_set(mi,
				  (char *)e_theme_edje_file_get("base/theme/fileman",
								"fileman/button/arrange_name"),
				  "fileman/button/arrange_name");

	mi = e_menu_item_new(mn);
	e_menu_item_label_set(mi, _("By Mod Time"));
	e_menu_item_radio_set(mi, 1);
	e_menu_item_radio_group_set(mi, 2);
	if (sd->arrange == E_FILEMAN_CANVAS_ARRANGE_MODTIME) e_menu_item_toggle_set(mi, 1);
	e_menu_item_callback_set(mi, _e_fm_menu_arrange_cb, sd);
	e_menu_item_icon_edje_set(mi,
				  (char *)e_theme_edje_file_get("base/theme/fileman",
								"fileman/button/arrange_time"),
				  "fileman/button/arrange_time");
       /*- New -*/
	mi = e_menu_item_new(sd->menu);
	e_menu_item_label_set(mi, _("New"));
	e_menu_item_icon_edje_set(mi,
				  (char *)e_theme_edje_file_get("base/theme/fileman",
								"fileman/button/new"),
				  "fileman/button/new");

	mn = e_menu_new();
	e_menu_category_set(mn,"fileman/unknown/new");
	e_menu_category_data_set("fileman/unknown/new",sd);
	e_menu_item_submenu_set(mi, mn);

	mi = e_menu_item_new(mn);
	e_menu_item_label_set(mi, _("Directory"));
	e_menu_item_callback_set(mi, _e_fm_menu_new_dir_cb, sd);
	e_menu_item_icon_edje_set(mi,
				  (char *)e_theme_edje_file_get("base/theme/fileman",
								"fileman/button/new_dir"),
				  "fileman/button/new_dir");
       /*- View -*/
	mi = e_menu_item_new(sd->menu);
	e_menu_item_label_set(mi, _("View"));
	e_menu_item_icon_edje_set(mi,
				  (char *)e_theme_edje_file_get("base/theme/fileman",
								"fileman/button/view"),
				  "fileman/button/view");

	mn = e_menu_new();
	e_menu_category_set(mn,"fileman/unknown/view");
	e_menu_category_data_set("fileman/unknown/view",sd);
	e_menu_item_submenu_set(mi, mn);

	mi = e_menu_item_new(mn);
	e_menu_item_label_set(mi, _("Name Only"));
	e_menu_item_radio_set(mi, 1);
	e_menu_item_radio_group_set(mi, 2);
	e_menu_item_icon_edje_set(mi,
				  (char *)e_theme_edje_file_get("base/theme/fileman",
								"fileman/button/view_name"),
				  "fileman/button/view_name");

	mi = e_menu_item_new(mn);
	e_menu_item_label_set(mi, _("Details"));
	e_menu_item_radio_set(mi, 1);
	e_menu_item_radio_group_set(mi, 2);
	e_menu_item_icon_edje_set(mi,
				  (char *)e_theme_edje_file_get("base/theme/fileman",
								"fileman/button/view_details"),
				  "fileman/button/view_details");

       /*- Refresh -*/
	mi = e_menu_item_new(sd->menu);
	e_menu_item_label_set(mi, _("Refresh"));
	e_menu_item_callback_set(mi, _e_fm_menu_refresh_cb, sd);
	e_menu_item_icon_edje_set(mi,
				  (char *)e_theme_edje_file_get("base/theme/fileman",
								"fileman/button/refresh"),
				  "fileman/button/refresh");
       /*- Properties -*/
	mi = e_menu_item_new(sd->menu);
	e_menu_item_label_set(mi, _("Properties"));
	e_menu_item_icon_edje_set(mi,
				  (char *)e_theme_edje_file_get("base/theme/fileman",
								"fileman/button/properties"),
				  "fileman/button/properties");

	ecore_evas_geometry_get(sd->win->ecore_evas, &x, &y, &w, &h);

	e_menu_activate_mouse(sd->menu, sd->win->border->zone,
			      ev->output.x + x, ev->output.y + y, 1, 1,
			      E_MENU_POP_DIRECTION_DOWN, ev->timestamp);
	_e_fm_fake_mouse_up_all_later(sd->win->evas);
	break;
     }
}

static void
_e_fm_mouse_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   E_Fm_Smart_Data *sd;
   Evas_Event_Mouse_Move *ev;

   ev = event_info;
   sd = data;

   if (!sd->selection.band.obj)
     return;

   if (sd->selection.band.enabled)
     {
	Evas_Coord x, y, w, h;

	evas_object_geometry_get(sd->selection.band.obj, &x, &y, &w, &h);

	if ((ev->cur.canvas.x > sd->selection.band.x) &&
	    (ev->cur.canvas.y < sd->selection.band.y))
	  {
	     /* growing towards top right */
	     evas_object_move(sd->selection.band.obj,
			      sd->selection.band.x,
			      ev->cur.canvas.y);
	     evas_object_resize(sd->selection.band.obj,
				ev->cur.canvas.x - sd->selection.band.x,
				sd->selection.band.y - ev->cur.canvas.y);
	  }
	else if ((ev->cur.canvas.x > sd->selection.band.x) &&
		 (ev->cur.canvas.y > sd->selection.band.y))
	  {
	     /* growing towards bottom right */
	     w = ev->cur.canvas.x - sd->selection.band.x;
	     h = ev->cur.canvas.y - sd->selection.band.y;

	     evas_object_resize(sd->selection.band.obj, w, h);
	  }
	else if ((ev->cur.canvas.x < sd->selection.band.x) &&
		 (ev->cur.canvas.y < sd->selection.band.y))
	  {
	     /* growing towards top left */
	     evas_object_move(sd->selection.band.obj,
			      ev->cur.canvas.x,
			      ev->cur.canvas.y);
	     evas_object_resize(sd->selection.band.obj,
				sd->selection.band.x - ev->cur.canvas.x,
				sd->selection.band.y - ev->cur.canvas.y);
	  }
	else if ((ev->cur.canvas.x < sd->selection.band.x) &&
		 (ev->cur.canvas.y > sd->selection.band.y))
	  {
	     /* growing towards button left */
	     evas_object_move(sd->selection.band.obj,
			      ev->cur.canvas.x,
			      sd->selection.band.y);
	     evas_object_resize(sd->selection.band.obj,
				sd->selection.band.x - ev->cur.canvas.x,
				ev->cur.canvas.y - sd->selection.band.y);
	  }

	evas_object_geometry_get(sd->selection.band.obj, &x, &y, &w, &h);
	_e_fm_selections_rect_add(sd, x, y, w, h);
     }
   
}

static void
_e_fm_mouse_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   E_Fm_Smart_Data *sd;
   Evas_Event_Mouse_Up *ev;

   sd = data;
   ev = event_info;

   if (sd->selection.band.enabled)
     {
	sd->selection.band.enabled = 0;
	evas_object_resize(sd->selection.band.obj, 1, 1);
	evas_object_hide(sd->selection.band.obj);
	sd->selection.band.files = evas_list_free(sd->selection.band.files);
     }
   printf("mouse up on the window\n");
}

static void
_e_fm_icon_mouse_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{

   E_Fm_Icon *icon;
   Evas_Event_Mouse_Down *ev;

   ev = event_info;
   icon = data;

   if(icon->sd->state ==  E_FILEMAN_STATE_RENAME)
     {
	_e_fm_file_rename_abort(icon->sd->selection.current.file);
	return;
     }

   if (ev->button == 1)
     {
	/* double click = default mime action */
	if(ev->flags == EVAS_BUTTON_DOUBLE_CLICK)
	  {
	     icon->sd->operation.files = NULL;
	     _e_fm_selections_clear(icon->sd);
	     _e_fm_selections_add(icon, evas_list_find_list(icon->sd->icons, icon));
	     icon->sd->operation.hover = NULL;
	     icon->sd->operation.files = evas_list_append(icon->sd->operation.files, e_fm_file_new(icon->file->path));
	     icon->sd->operation.mime = icon->file->mime;
	     e_fm_mime_action_default_call(icon->sd);
	  }
	else
	  {
	     if(icon->sd->is_selector && icon->file->type == E_FM_FILE_TYPE_FILE)
	       _e_fm_selector_send_hilite_file(icon);

	     if(icon->sd->win)
	       {
		  int x,y;
		  
		  evas_object_geometry_get(icon->icon_obj, &x, &y, NULL, NULL);
		  
		  icon->sd->drag.status = E_FILEMAN_DRAG_START;
		  icon->sd->drag.button = E_FILEMAN_DRAG_PRIMARY;
		  icon->sd->drag.x = ev->canvas.x;
		  icon->sd->drag.y = ev->canvas.y;
		  icon->sd->drag.icon_obj = icon;
	       }

	     if (!icon->state.selected)
	       {
		  if (!evas_key_modifier_is_set(evas_key_modifier_get(icon->sd->evas), "Control"))
		       _e_fm_selections_clear(icon->sd);
	       }
	  }
     }
   else if (ev->button == 3)
     {
	if(icon->sd->win)
	  {
	     int x,y;
		  
	     evas_object_geometry_get(icon->icon_obj, &x, &y, NULL, NULL);
		  
	     icon->sd->drag.status = E_FILEMAN_DRAG_START;
	     icon->sd->drag.button = E_FILEMAN_DRAG_SECONDARY;
	     icon->sd->drag.x = ev->canvas.x;
	     icon->sd->drag.y = ev->canvas.y;
	     icon->sd->drag.icon_obj = icon;
	  }
	if (!icon->state.selected)
	  {
	     if (!evas_key_modifier_is_set(evas_key_modifier_get(icon->sd->evas), "Control"))
	       _e_fm_selections_clear(icon->sd);
	  }
     }	
}

static void
_e_fm_icon_mouse_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   E_Fm_Icon *icon;
   Evas_Event_Mouse_Up *ev;
   Evas_List *l;

   ev = event_info;
   icon = data;

   if(!strcmp(edje_object_part_state_get(icon->sd->edje_obj, "typebuffer", NULL), "shown"))
     {
	E_Fm_Icon *i;
	edje_object_signal_emit(icon->sd->edje_obj, "typebuf_hide", "");
	edje_object_part_text_set(icon->sd->edje_obj, "text", "");
	for (l = icon->sd->icons; l; l = l->next)
	  {
	     i = l->data;
	     e_fm_icon_signal_emit(i->icon_obj, "default", "");
	  }
	edje_object_signal_emit(icon->sd->edje_obj, "default", "");
     }

   switch(ev->button)
     {
      case 1:
	 /* selection */
	 if(icon->sd->drag.status == E_FILEMAN_DRAG_START)
	   {
	      if (!icon->state.selected)
		{
		   if (evas_key_modifier_is_set(evas_key_modifier_get(icon->sd->evas), "Control"))
		     _e_fm_selections_add(icon, evas_list_find_list(icon->sd->icons, icon));
		   else
		     {
			_e_fm_selections_clear(icon->sd);
			_e_fm_selections_add(icon, evas_list_find_list(icon->sd->icons, icon));
		     }
		}
	      else
		{
		   if (evas_key_modifier_is_set(evas_key_modifier_get(icon->sd->evas), "Control"))
		     _e_fm_selections_del(icon);
		   else
		     {
			_e_fm_selections_clear(icon->sd);
			_e_fm_selections_add(icon, evas_list_find_list(icon->sd->icons, icon));
		     }
		}
	      icon->sd->drag.status = E_FILEMAN_DRAG_NONE;
	   }
	 else if(icon->sd->drag.status == E_FILEMAN_DRAG_DOING)
	   {
	      icon->sd->drag.button = E_FILEMAN_DRAG_PRIMARY;
	   }
	 /* relative action */
	 else if(icon->sd->drag.status == E_FILEMAN_DRAG_DONE)
	   {
	      printf("call the default relative action\n");
	   }
	break;
	
      case 3:
	/* action menu */ 
	if(icon->sd->drag.status == E_FILEMAN_DRAG_START)
	   {
	      if (!icon->state.selected)
		{
		   if (evas_key_modifier_is_set(evas_key_modifier_get(icon->sd->evas), "Control"))
		     _e_fm_selections_add(icon, evas_list_find_list(icon->sd->icons, icon));
		   else
		     {
			_e_fm_selections_clear(icon->sd);
			_e_fm_selections_add(icon, evas_list_find_list(icon->sd->icons, icon));
		     }
		}
	      /* free operation struct before recreating */
	      _e_fm_operation_free(icon->sd);
	      /* set the operation files equal to the selected icons */
	      for(l = icon->sd->selection.icons; l; l = l->next)
		{
		   E_Fm_Icon *ic;
		   ic = (E_Fm_Icon *)l->data;
		   /* duplicate the selected files, to not free the selected ones directly */
		   icon->sd->operation.files = evas_list_append(icon->sd->operation.files, e_fm_file_new(ic->file->path));
		}
	      /* get the overall mime entry for the selected files */
	      icon->sd->operation.hover = NULL; 
	      icon->sd->operation.mime = e_fm_mime_get_from_list(icon->sd->operation.files);
	      icon->sd->drag.status = E_FILEMAN_DRAG_NONE;
	      _e_fm_menu_action_display(icon->sd, ev->output.x, ev->output.y, ev->timestamp);
	   }
	 else if(icon->sd->drag.status == E_FILEMAN_DRAG_DOING)
	   {
	      icon->sd->drag.button = E_FILEMAN_DRAG_SECONDARY;
	   }
	 /* context menu */
	 else if(icon->sd->drag.status == E_FILEMAN_DRAG_DONE)
	  {
	     printf("popping up the context menu\n");
	     _e_fm_menu_context_display(icon->sd, ev->output.x, ev->output.y, ev->timestamp);
	  }
	break;

      default:
	break;
     }
   
}
static void
_e_fm_icon_mouse_in_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   E_Fm_Icon *icon;
   Evas_Event_Mouse_In *ev;

   ev = event_info;
   icon = data;

   e_fm_icon_signal_emit(icon->icon_obj, "mousein", "");
}

static void
_e_fm_icon_mouse_out_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   E_Fm_Icon *icon;
   Evas_Event_Mouse_Out *ev;

   ev = event_info;
   icon = data;

   e_fm_icon_signal_emit(icon->icon_obj, "mouseout", "");
}

static void
_e_fm_icon_mouse_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   E_Fm_Smart_Data *sd;
   E_Fm_Icon *icon;
   Evas_Event_Mouse_Move *ev;

   ev = event_info;
   sd = data;
   icon = NULL;
		  
   //printf("icon mouse move\n");
   if(sd->win)
     icon = sd->drag.icon_obj;

   if (!icon) return;

   if ((sd->drag.status == E_FILEMAN_DRAG_START) && sd->win)
     {
	if ((sd->drag.x == -1) && (sd->drag.y == -1))
	  {
	     sd->drag.x = ev->cur.output.x;
	     sd->drag.y = ev->cur.output.y;
	  }
	else
	  {
	     int dx, dy;
	     
	     dx = sd->drag.x - ev->cur.output.x;
	     dy = sd->drag.y - ev->cur.output.y;

	     if (((dx * dx) + (dy * dy)) > (100))
	       {		  
		  Evas_List *l;
		  Evas_Coord x, y, w, h;
		  int i;
		  int cx, cy;
		  char *data;
		  char *tmp;
		  char **drop_types = NULL;


		  //printf("creating a new fake window\n");
		  _e_fm_selections_add(icon, evas_list_find_list(icon->sd->icons, icon));
		       
		  data = calloc(PATH_MAX, sizeof(char));
		  snprintf(data, PATH_MAX * sizeof(char), "file://");
		  drop_types = calloc(1,sizeof(char*));
		  drop_types[0] = strdup("text/uri-list");

	
		  /* send the dragged signal to all the selected icons */
		  for(l = sd->selection.icons, i = 0; l; l = l->next, i++)
		    {
		       E_Fm_Icon *ic;
		       ic = (E_Fm_Icon*)l->data;
		       if(ic->icon_obj)
			 e_fm_icon_signal_emit(ic->icon_obj, "dragged", "");
		       
		       tmp = calloc(PATH_MAX, sizeof(char));
		       if(!i)
			 strncpy(tmp,ic->file->path,PATH_MAX * sizeof(char));
		       /* the separator on uri-lis is \n, maybe also \r\n? */
		       else
			 snprintf(tmp, PATH_MAX * sizeof(char), "\n%s", ic->file->path);
		       
		       strncat(data, tmp, PATH_MAX * sizeof(char)); 
		       free(tmp);
		    }
		  
		  ecore_evas_geometry_get(sd->win->ecore_evas, &cx, &cy, NULL, NULL);
		  evas_object_geometry_get(icon->icon_obj, &x, &y, &w, &h);
		  
		  
		  if(sd->drag.ecore_evas)
		    ecore_evas_free(sd->drag.ecore_evas);
		       
		  
		  sd->drag.dx = ev->cur.canvas.x - x;
		  sd->drag.dy = ev->cur.canvas.y - y; 
		
		  sd->drag.ecore_evas = ecore_evas_software_x11_new(NULL, 0, cx + x, cy + y, w, h);
		  sd->drag.evas = ecore_evas_get(sd->drag.ecore_evas);
		  sd->drag.win = ecore_evas_software_x11_window_get(sd->drag.ecore_evas);
		  sd->drag.status = E_FILEMAN_DRAG_DOING;
		  		  
		  ecore_evas_shaped_set(sd->drag.ecore_evas, 1);
		  ecore_evas_borderless_set(sd->drag.ecore_evas, 1);
		  ecore_evas_name_class_set(sd->drag.ecore_evas, "E", "_e_drag_window");
		  ecore_evas_title_set(sd->drag.ecore_evas, "E Drag");
		  ecore_evas_ignore_events_set(sd->drag.ecore_evas, 1);
		  ecore_evas_show(sd->drag.ecore_evas);
		  
		  sd->drag.image_object = e_fm_icon_add(sd->drag.evas);
		  e_fm_icon_file_set(sd->drag.image_object, icon->file);
		  e_fm_icon_title_set(sd->drag.image_object, "");
		  if (!sd->drag.image_object)
		    {
		       sd->drag.image_object = evas_object_rectangle_add(sd->drag.evas);
		       evas_object_color_set(sd->drag.image_object, 255, 255, 255, 255);
		    }
                  evas_object_resize(sd->drag.image_object, w, h);
		  evas_object_move(sd->drag.image_object, 0, 0);
		  evas_object_show(sd->drag.image_object);
		  
		  ecore_x_dnd_aware_set(sd->drag.win, 1);
		  ecore_x_dnd_types_set(sd->drag.win, drop_types, 1);
		  ecore_x_dnd_begin(sd->drag.win, data, PATH_MAX * sizeof(char));
		 
		  e_fm_drag_mouse_move_handler = ecore_event_handler_add(ECORE_X_EVENT_MOUSE_MOVE,
                                                                    _e_fm_drag_mouse_move_cb, sd);

                  e_fm_drag_mouse_up_handler = ecore_event_handler_add(ECORE_X_EVENT_MOUSE_BUTTON_UP,
                                                                  _e_fm_drag_mouse_up_cb, sd);
	       
	       }
	  }
     }
}


/* the autoscroll will be disabled, there are several issues with the icon canvas */
static int                
_e_fm_icon_autoscroll_cb (void *data)
{
   E_Fm_Smart_Data *sd;

   sd = data;

#if 0
   if(sd->autoscroll.direction & E_FILEMAN_AUTOSCROLL_UP)
     {
	sd->child.y -= sd->autoscroll.timer_int;
	if(sd->child.y < 0) sd->child.y = 0;

     }
   if(sd->autoscroll.direction & E_FILEMAN_AUTOSCROLL_DOWN)
     {
	sd->child.y += sd->autoscroll.timer_int;
	if(sd->child.y > sd->child.h) sd->child.y = sd->child.h;
     }
   if(sd->autoscroll.direction & E_FILEMAN_AUTOSCROLL_LEFT)
     {
	sd->child.x -= sd->autoscroll.timer_int;
	if(sd->child.x < 0) sd->child.x = 0;

     }
   if(sd->autoscroll.direction & E_FILEMAN_AUTOSCROLL_RIGHT)
     {
	sd->child.x += sd->autoscroll.timer_int;
	if(sd->child.x > sd->child.w) sd->child.x = sd->child.w - 1;
     }
  
   sd->autoscroll.timer_int = exp(sd->autoscroll.timer_int);
   if(sd->autoscroll.timer_int > 50)
     sd->autoscroll.timer_int = 50;

   
   e_icon_canvas_xy_freeze(sd->layout);
   evas_object_move(sd->layout, sd->x - sd->child.x, sd->y - sd->child.y);
   e_icon_canvas_xy_thaw(sd->layout);
   evas_object_smart_callback_call(sd->object, "changed", NULL);*/
#endif   
   return 1;
}

static int                 
_e_fm_drag_mouse_move_cb(void *data, int type, void *event)
{
   E_Fm_Smart_Data *sd;
   Ecore_X_Event_Mouse_Move *ev;
   int cx,cy,cw,ch;
   int x,y;

   sd = data;
   ev = event;

   ecore_evas_geometry_get(sd->win->ecore_evas, &cx, &cy, &cw, &ch);
   x = cx + ev->x - sd->drag.dx;
   y = cy + ev->y - sd->drag.dy;
   ecore_evas_move(sd->drag.ecore_evas, x, y);

   return 1;
}
static int 
_e_fm_drag_mouse_up_cb(void *data, int type, void *event)
{
   E_Fm_Smart_Data *sd;

   sd = data;
	
   ecore_x_dnd_drop();

   ecore_event_handler_del(e_fm_drag_mouse_move_handler);
   e_fm_drag_mouse_move_handler = NULL;
   
   ecore_event_handler_del(e_fm_drag_mouse_up_handler);
   e_fm_drag_mouse_up_handler = NULL;

   ecore_evas_hide(sd->drag.ecore_evas);
   
   return 1;

}

static void
_e_fm_string_replace(const char *src, const char *key, const char *replacement, char *result, size_t resultsize)
{
   size_t resultlen;
   size_t keylen;

   if(resultsize < 0) return;

   /* special case to prevent infinite loop if key==replacement=="" */
   if(strcmp(key, replacement) == 0)
     {
	snprintf(result, resultsize, "%s", src);
	return;
     }

   keylen = strlen(key);

   resultlen = 0;
   while(*src != '\0' && resultlen+1 < resultsize)
     {
	if(strncmp(src, key, keylen) == 0)
	  {
	     snprintf(result+resultlen, resultsize-resultlen, "%s", replacement);
	     resultlen += strlen(result+resultlen);
	     src += keylen;
	  }
	else
	  {
	     result[resultlen++] = *src++;
	  }
     }
   result[resultlen] = '\0';
}

static void
_e_fm_autocomplete(E_Fm_Smart_Data *sd)
{
   /* TODO */
}

static void
_e_fm_icon_select_glob(E_Fm_Smart_Data *sd, const char *glb)
{
   E_Fm_Icon *icon, *anchor;
   Evas_List *l;
   char *glbpath;
   Evas_Coord x, y, w, h;
   E_Event_Fm_Reconfigure *ev;
   int i;
   glob_t globbuf;

   anchor = NULL;
   ev = NULL;
   glbpath = E_NEW(char, strlen(sd->dir) + strlen(glb) + 2);
   snprintf(glbpath, strlen(sd->dir) + strlen(glb) + 2, "%s/%s", sd->dir, glb);

   _e_fm_selections_clear(sd);

   edje_object_signal_emit(sd->edje_obj, "selecting", "");

   if(glob(glbpath, 0, NULL, &globbuf))
     {
	for (l = sd->icons; l; l = l->next)
	  {
	     icon = l->data;
	     e_fm_icon_signal_emit(icon->icon_obj, "disable", "");
	  }
	return;
     }

   for (l = sd->icons; l; l = l->next)
     {
	icon = l->data;
	for (i = 0; i < globbuf.gl_pathc; i++)
	  {
	     const char *file;

	     file = ecore_file_get_file(globbuf.gl_pathv[i]);
	     if (!strcmp(icon->file->name, file))
	       {
		  _e_fm_selections_add(l->data, l);
		  e_fm_icon_signal_emit(icon->icon_obj, "default", "");
		  if (!anchor)
		    {
		       evas_object_geometry_get(icon->icon_obj, &x, &y, &w, &h);

		       if (!E_CONTAINS(sd->x, sd->y, sd->w, sd->h, x, y, w, h))
			 {
			    ev = E_NEW(E_Event_Fm_Reconfigure, 1);
			    if (ev)
			      {
				 anchor = icon;
				 ev->object = sd->object;
				 ev->x = sd->x;
				 ev->y = sd->child.y - (sd->y - (y - sd->icon_info.y_space));
				 ev->w = sd->w;
				 ev->h = sd->h;
			      }
			 }
		    }
	       }
	  }
     }

   if ((anchor) && (ev))
     ecore_event_add(E_EVENT_FM_RECONFIGURE, ev, NULL, NULL);
}

/* do we keep this? might come in handy if we need to jump to a file starting
 * with a certain char
 */
static void
  __e_fm_icon_goto_key(E_Fm_Smart_Data *sd, char *c)
{
   E_Fm_Icon *icon;
   Evas_List *l;

   return;

   if(sd->selection.current.ptr)
     {
	l = sd->selection.current.ptr;
	icon = sd->selection.current.file;
	if(icon->file->name[0] == c[0] && l->next)
	  l = l->next;
	else
	  l = sd->selection.current.ptr;
     }
   else
     l = sd->icons;

   for (; l; l = l->next)
     {
	icon = l->data;
	if (icon->file->name[0] == c[0])
	  {
	     _e_fm_selections_clear(sd);
	     _e_fm_selections_add(l->data, l);
	     goto position;
	  }
     }
   for (l = sd->icons; l != sd->selection.current.ptr; l = l->next)
     {
	icon = l->data;
	if (icon->file->name[0] == c[0])
	  {
	     _e_fm_selections_clear(sd);
	     _e_fm_selections_add(l->data, l);
	     goto position;
	  }
     }

   return;
   position:
     {
	Evas_Coord x, y, w, h;
	icon = l->data;
	evas_object_geometry_get(icon->icon_obj, &x, &y, &w, &h);
	if (!E_CONTAINS(sd->x, sd->y, sd->w, sd->h, x, y, w, h))
	  {
	     E_Event_Fm_Reconfigure *ev;

	     ev = E_NEW(E_Event_Fm_Reconfigure, 1);
	     if (ev)
	       {
		  ev->object = sd->object;
		  ev->x = sd->x;
		  ev->y = sd->child.y - (sd->y - (y - sd->icon_info.y_space));
		  ev->w = sd->w;
		  ev->h = sd->h;
		  ecore_event_add(E_EVENT_FM_RECONFIGURE, ev, NULL, NULL);
	       }
	  }
     }
}

static void
_e_fm_icon_select_up(E_Fm_Smart_Data *sd)
{
   Evas_List *l;
   E_Fm_Icon *icon;

   if(!sd->selection.current.ptr)
     _e_fm_selections_add(sd->icons->data, sd->icons);
   else
     {
	if(sd->selection.current.ptr->prev) /* are we already at the beginning? */
	  {
	     Evas_Coord x, x2, y, w, h;

	     l = sd->selection.current.ptr;
	     icon = l->data;
	     evas_object_geometry_get(icon->icon_obj, &x, NULL, NULL, NULL);
	     x2 = x + 1;
	     l = l->prev;

	     if (!evas_key_modifier_is_set(evas_key_modifier_get(sd->evas), "Control"))
	       {
		  for(; l && (x != x2); l = l->prev)
		    {
		       icon = l->data;
		       evas_object_geometry_get(icon->icon_obj, &x2, NULL, NULL, NULL);

		    }
		  _e_fm_selections_clear(sd);
		  if(!l)
		    {
		       l = sd->icons;  // go to the beginning
		       _e_fm_selections_add(l->data, l);
		    }
		  else
		    _e_fm_selections_add(l->next->data, l->next);
	       }
	     else /* Control modifier set */
	       {
		  int flag = 0;
		  icon = l->data;
		  if ((flag = icon->state.selected))
		    {
		       if (l->next->next)
			 icon = l->next->next->data;
		       else
			 icon = l->next->data;

		       evas_object_geometry_get(icon->icon_obj, &x, NULL, NULL, NULL);
		       _e_fm_selections_del(sd->selection.current.ptr->data);
		       x2 = x + 1;
		    }
		  for (; l && (x != x2); l = l->prev)
		    {
		       icon = l->data;
		       evas_object_geometry_get(icon->icon_obj, &x2, NULL, NULL, NULL);

		       if (icon->state.selected)
			 {
			    _e_fm_selections_del(l->data);
			    _e_fm_selections_current_set(l->data, l);
			 }
		       else
			 _e_fm_selections_add(l->data, l);
		    }
		  if (flag && l)
		    _e_fm_selections_current_set(l->data, l);
	       }
	     evas_object_geometry_get(icon->icon_obj, &x, &y, &w, &h);
	     if(!E_CONTAINS(sd->x, sd->y, sd->w, sd->h, x, y, w, h))
	       {
		  E_Event_Fm_Reconfigure *ev;

		  ev = E_NEW(E_Event_Fm_Reconfigure, 1);
		  if (ev)
		    {
		       ev->object = sd->object;
		       ev->x = sd->x;
		       ev->y = sd->child.y - (sd->y - (y - sd->icon_info.y_space));
		       ev->w = sd->w;
		       ev->h = sd->h;
		       ecore_event_add(E_EVENT_FM_RECONFIGURE, ev, NULL, NULL);
		    }
	       }
	  }
     }
}

static void
_e_fm_icon_select_down(E_Fm_Smart_Data *sd)
{
   Evas_List *l;

   if (!sd->selection.current.ptr)
     _e_fm_selections_add(sd->icons->data, sd->icons);
   else
     {
	if (sd->selection.current.ptr->next) /* are we already at the end? */
	  {

	     E_Fm_Icon *icon;
	     Evas_Coord x, x2, y, w, h;;
	     l = sd->selection.current.ptr;
	     icon = l->data;
	     evas_object_geometry_get(icon->icon_obj, &x, NULL, NULL, NULL);
	     x2 = x + 1;
	     l = l->next;

	     if (!evas_key_modifier_is_set(evas_key_modifier_get(sd->evas), "Control"))
	       {
		  for(; l && (x != x2); l = l->next)
		    {
		       icon = l->data;
		       evas_object_geometry_get(icon->icon_obj, &x2, NULL, NULL, NULL);
		    }
		  _e_fm_selections_clear(sd);
		  if(!l)
		    {
		       l = evas_list_last(sd->icons);
		       _e_fm_selections_add(l->data, l);
		    }
		  else
		    _e_fm_selections_add(l->prev->data, l->prev);
	       }
	     else /* Control modifier set */
	       {
		  int flag = 0;
		  icon = l->data;
		  if((flag = icon->state.selected))
		    {
		       if(l->prev->prev)
			 icon = l->prev->prev->data;
		       else
			 icon = l->prev->data; // This case

		       evas_object_geometry_get(icon->icon_obj, &x, NULL, NULL, NULL);
		       _e_fm_selections_del(sd->selection.current.ptr->data);
		       x2 = x + 1;
		    }
		  for(; l && (x != x2); l = l->next)
		    {
		       icon = l->data;
		       evas_object_geometry_get(icon->icon_obj, &x2, NULL, NULL, NULL);

		       if(icon->state.selected)
			 {
			    _e_fm_selections_del(l->data);
			    _e_fm_selections_current_set(l->data, l);
			 }
		       else
			 _e_fm_selections_add(l->data, l);
		    }
		  if(flag && l)
		    _e_fm_selections_current_set(l->data, l);

	       }
	     evas_object_geometry_get(icon->icon_obj, &x, &y, &w, &h);
	     if(!E_CONTAINS(sd->x, sd->y, sd->w, sd->h, x, y, w, h))
	       {
		  E_Event_Fm_Reconfigure *ev;

		  ev = E_NEW(E_Event_Fm_Reconfigure, 1);
		  if (ev)
		    {
		       ev->object = sd->object;
		       ev->x = sd->x;
		       ev->y = sd->child.y - (sd->y - (y - sd->icon_info.y_space));
		       ev->w = sd->w;
		       ev->h = sd->h;
		       ecore_event_add(E_EVENT_FM_RECONFIGURE, ev, NULL, NULL);
		    }
	       }
	  }
     }
}

static void
_e_fm_icon_select_left(E_Fm_Smart_Data *sd)
{
   if(!sd->selection.current.ptr)
     _e_fm_selections_add(sd->icons->data, sd->icons);
   else
     {
	if(sd->selection.current.ptr->prev)
	  {
	     Evas_List *prev;
	     E_Fm_Icon *icon, *icon_prev;
	     Evas_Coord x, y, w, h;

	     icon = sd->selection.current.ptr->data;
	     prev = sd->selection.current.ptr->prev;
	     icon_prev = prev->data;

	     if (evas_key_modifier_is_set(evas_key_modifier_get(sd->evas), "Control"))
	       {
		  if(icon_prev->state.selected)
		    {
		       if(icon->state.selected)
			 {
			    _e_fm_selections_del(icon);
			    _e_fm_selections_current_set(icon_prev, prev);
			 }
		       else
			 {
			    _e_fm_selections_del(icon_prev);
			    _e_fm_selections_current_set(icon_prev, prev);
			 }
		    }
		  else
		    _e_fm_selections_add(icon_prev, prev);
	       }
	     else
	       {
		  _e_fm_selections_clear(sd);
		  _e_fm_selections_add(icon_prev, prev);
	       }

	     evas_object_geometry_get(icon_prev->icon_obj, &x, &y, &w, &h);
	     if(!E_CONTAINS(sd->x, sd->y, sd->w, sd->h, x, y, w, h))
	       {
		  E_Event_Fm_Reconfigure *ev;
		  ev = E_NEW(E_Event_Fm_Reconfigure, 1);
		  if (ev)
		    {
		       ev->object = sd->object;
		       ev->x = sd->x;
		       ev->y = sd->child.y - (sd->y - (y - sd->icon_info.y_space));
		       ev->w = sd->w;
		       ev->h = sd->h;
		       ecore_event_add(E_EVENT_FM_RECONFIGURE, ev, NULL, NULL);
		    }
	       }
	  }
     }
}

static void
_e_fm_icon_select_right(E_Fm_Smart_Data *sd)
{
   if(!sd->selection.current.ptr)
     _e_fm_selections_add(sd->icons->data, sd->icons);
   else
     {
	if (sd->selection.current.ptr->next)
	  {
	     Evas_List *next;
	     E_Fm_Icon *icon, *icon_next;
	     Evas_Coord x, y, w, h;

	     icon = sd->selection.current.ptr->data;

	     next = sd->selection.current.ptr->next;
	     icon_next = next->data;

	     if (evas_key_modifier_is_set(evas_key_modifier_get(sd->evas), "Control"))
	       {
		  if (icon_next->state.selected)
		    {
		       if(icon->state.selected)
			 {
			    _e_fm_selections_del(icon);
			    _e_fm_selections_current_set(icon_next, next);
			 }
		       else
			 {
			    _e_fm_selections_del(icon_next);
			    _e_fm_selections_current_set(icon_next, next);
			 }
		    }
		  else
		    _e_fm_selections_add(icon_next, next);
	       }
	     else
	       {
		  _e_fm_selections_clear(sd);
		  _e_fm_selections_add(icon_next, next);
	       }

	     evas_object_geometry_get(icon_next->icon_obj, &x, &y, &w, &h);
	     if (!E_CONTAINS(sd->x, sd->y, sd->w, sd->h, x, y, w, h))
	       {
		  E_Event_Fm_Reconfigure *ev;

		  ev = E_NEW(E_Event_Fm_Reconfigure, 1);
		  if (ev)
		    {
		       ev->object = sd->object;
		       ev->x = sd->x;
		       ev->y = sd->child.y + y + h + sd->icon_info.y_space - (sd->y + sd->h);
		       ev->w = sd->w;
		       ev->h = sd->h;
		       ecore_event_add(E_EVENT_FM_RECONFIGURE, ev, NULL, NULL);
		    }
	       }
	  }
     }
}

static void
_e_fm_icon_run(E_Fm_Smart_Data *sd)
{
   E_Fm_Icon *icon;

   if (sd->selection.current.ptr)
     {
	icon = sd->selection.current.file;
	if (icon->file->type == E_FM_FILE_TYPE_DIRECTORY)
	  {
	     char *fullname;

	     if (!strcmp(icon->file->name, ".."))
	       {
		  fullname = _e_fm_dir_pop(icon->sd->dir);
	       }
	     else
	       {
		  fullname = strdup(icon->file->path);
	       }
	     if (fullname)
	       {
		  if (icon->sd->win)
		    e_win_title_set(icon->sd->win, fullname);
		  _e_fm_dir_set(icon->sd, fullname);
		  free(fullname);
	       }
	  }
	else if (icon->file->type == E_FM_FILE_TYPE_FILE)
	  {
	     if (icon->sd->is_selector)
	       {
		  _e_fm_selector_send_file(icon);
		  return;
	       }

	     /*if ((!e_fm_file_assoc_exec(icon->file)) &&
		 (e_fm_file_can_exec(icon->file)))
	       e_fm_file_exec(icon->file);*/
	  }
     }
}

static void
_e_fm_key_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Key_Down *ev;
   E_Fm_Smart_Data *sd;
   Evas_List *l;
   E_Fm_Icon *icon;

   ev = event_info;
   sd = data;
   const char *name;
   if(sd->state == E_FILEMAN_STATE_RENAME)
     {
	if (!strcmp(ev->keyname, "Return"))
	  {
	     if(!sd->selection.current.file) return;///???
	     icon = sd->selection.current.file;
	     name = e_entry_text_get(icon->sd->entry_obj);
	     //	  	name = e_entry_text_get(icon->sd->entry_obj);
	     //      e_fm_icon_edit_entry_set(icon->icon_obj, NULL);
	     //      evas_object_focus_set(icon->sd->entry_obj, 0);
	     //      evas_object_del(icon->sd->entry_obj);
	     //      icon->sd->entry_obj = NULL;
	     _e_fm_file_rename(icon, name);
	  }
	else if (!strcmp(ev->keyname, "Escape"))
	  {
	     if(!sd->selection.current.file) return;///???
	     icon = sd->selection.current.file;
	     _e_fm_file_rename_abort(icon);
	  }
     }
   else
     {
	if(!strcmp(edje_object_part_state_get(sd->edje_obj, "typebuffer", NULL), "shown"))
	  {
	     if (!strcmp(ev->keyname, "Tab"))
	       {
		  _e_fm_autocomplete(sd);
	       }
	     else if (!strcmp(ev->keyname, "BackSpace"))
	       {
		  const char *str;
		  str = NULL;
		  str = edje_object_part_text_get(sd->edje_obj, "text");
		  if (str)
		    {
		       char *buf;
		       int size;
		       size = strlen(str);
		       buf = calloc(size , sizeof(char));
		       snprintf(buf, size, "%s", str);
		       edje_object_part_text_set(sd->edje_obj, "text", buf);
		       _e_fm_icon_select_glob(sd, buf);
		       E_FREE(buf);
		    }
	       }
	     else if (!strcmp(ev->keyname, "Up") ||
		      !strcmp(ev->keyname, "Down") ||
		      !strcmp(ev->keyname, "Escape") ||
		      !strcmp(ev->keyname, "Return"))
	       {

		  edje_object_signal_emit(sd->edje_obj, "typebuf_hide", "");
		  edje_object_part_text_set(sd->edje_obj, "text", "");
		  for (l = sd->icons; l; l = l->next)
		    {
		       icon = l->data;
		       e_fm_icon_signal_emit(icon->icon_obj, "default", "");
		    }
		  edje_object_signal_emit(sd->edje_obj, "default", "");
	       }
	     else if (ev->string)
	       {
		  const char *str;
		  str = NULL;
		  str = edje_object_part_text_get(sd->edje_obj, "text");
		  if (str)
		    {
		       char *buf;
		       int size;
		       size = strlen(str) + strlen(ev->string) + 2;
		       buf = calloc(size, sizeof(char));
		       snprintf(buf, size, "%s%s", str, ev->string);
		       edje_object_part_text_set(sd->edje_obj, "text", buf);
		       _e_fm_icon_select_glob(sd, buf);
		       E_FREE(buf);
		    }
		  else
		    {
		       edje_object_part_text_set(sd->edje_obj, "text", ev->string);
		       _e_fm_icon_select_glob(sd, ev->string);
		    }
	       }
	  }
	else if (!strcmp(ev->keyname, "Up"))
	  {
	     _e_fm_icon_select_up(sd);
	  }
	else if (!strcmp(ev->keyname, "Down"))
	  {
	     _e_fm_icon_select_down(sd);
	  }
	else if (!strcmp(ev->keyname, "Left"))
	  {
	     _e_fm_icon_select_left(sd);
	  }
	else if (!strcmp(ev->keyname, "Right"))
	  {
	     _e_fm_icon_select_right(sd);
	  }
	else if (!strcmp(ev->keyname, "F2"))
	  {
	     icon = sd->selection.current.file;
	     _e_fm_file_rename_start(icon);
	  }
	else if (!strcmp(ev->keyname, "Escape"))
	  {

	  }
	else if (!strcmp(ev->keyname, "Return"))
	  {
	     _e_fm_icon_run(sd);
	  }
	else if (!strcmp(ev->keyname, "BackSpace"))
	  {
	     char *fullname;

	     fullname = _e_fm_dir_pop(sd->dir);
	     if (fullname)
	       {
		  if (sd->win)
		    e_win_title_set(sd->win, fullname);
		  _e_fm_dir_set(sd, fullname);
		  free(fullname);
	       }
	  }
	else if (ev->string)
	  {
	     const char *str;
	     str = edje_object_part_text_get(sd->edje_obj, "text");
	     if(str)
	       {
		  char *buf;
		  int size;
		  size = strlen(str) + strlen(ev->string) + 2;
		  buf = calloc(size, sizeof(char));
		  snprintf(buf, size, "%s%s", str, ev->string);
		  edje_object_part_text_set(sd->edje_obj, "text", buf);
		  _e_fm_icon_select_glob(sd, buf);
		  E_FREE(buf);
	       }
	     else
	       {
		  edje_object_part_text_set(sd->edje_obj, "text", ev->string);
		  _e_fm_icon_select_glob(sd, ev->string);
	       }
	     if(sd->state == E_FILEMAN_STATE_IDLE) edje_object_signal_emit(sd->edje_obj, "typebuf_show", "");
	  }
     }
}

static int
_e_fm_xdnd_enter_cb(void *data, int type, void *event)
{
   Ecore_X_Event_Xdnd_Enter *ev;
   Evas *e;
   Evas_Object *eo;
   Evas_Smart *s;
   E_Fm_Smart_Data *sd;

   ev = event;
   sd = data;

   if (ev->win != sd->win->evas_win) return 1;
   
   return 1;
}

static int
_e_fm_xdnd_leave_cb(void *data, int type, void *event)
{
   Ecore_X_Event_Xdnd_Leave *ev;
   E_Fm_Smart_Data *sd;

   ev = event;
   sd = data;
   
   if (ev->win != sd->win->evas_win) return 1;

#if 0
   /* in we leaved an efm window with autoscroll disable it*/
   if(sd->autoscroll.timer)
     {
	ecore_timer_del(sd->autoscroll.timer);
	sd->autoscroll.timer = NULL;
     }
#endif
   /* if we left any icon hover unhover it */
     {
	Evas_List *l;
	
	for (l = sd->icons; l; l = l->next)
	  {
	     E_Fm_Icon *icon;

	     icon = l->data;
	     if(!icon->icon_obj)
	       continue;
	     if(icon->state.hover)
	       e_fm_icon_signal_emit(icon->icon_obj, "unhover", "");
	  }
     }
   
   return 1;
}

static int
_e_fm_xdnd_status_cb(void *data, int type, void *event)
{
   Ecore_X_Event_Xdnd_Leave *ev;
   E_Fm_Smart_Data *sd;

   ev = event;
   sd = data;
   if (ev->win != sd->win->evas_win) return 1;

   return 1;
}

static int
_e_fm_xdnd_position_cb(void *data, int type, void *event)
{
   Ecore_X_Event_Xdnd_Position *ev;
   E_Fm_Smart_Data *sd;
   Ecore_X_Rectangle rect;
   int x,y;
   int dx,dy,dw,dh;

   ev = event;
   sd = data;
   if (ev->win != sd->win->evas_win) 
     {
	/* outside an efm window */
	return 1;
     }

   ecore_evas_geometry_get(sd->win->ecore_evas, &dx, &dy, &dw, &dh);

   //printf("pointer: %d %d, window: %d %d %d %d\n", ev->position.x, ev->position.y, dx,dy,dw,dh);
   x = ev->position.x - dx;
   y = ev->position.y - dy;
   
#if 0
     {
	/* autoscroll */
	/* look to set up the direction of the autoscroll
	 * add timer callbacks for automatic scroll in case the mouse is over 
	 * that region, the threshold region to scroll is the 20% of the 
	 * window size */
   
	int rw,rh;
		
	rw = dw * 0.2;
	rh = dh * 0.2;
   
	sd->autoscroll.direction = E_FILEMAN_AUTOSCROLL_NONE;
	if( (y < rh) && (y >= 0) )
	  sd->autoscroll.direction |= E_FILEMAN_AUTOSCROLL_UP;

	if( (y > dh - rh) && (y <= dh) )
	  sd->autoscroll.direction |= E_FILEMAN_AUTOSCROLL_DOWN;

	if( (x < rw) && (x >= 0) )
	  sd->autoscroll.direction |= E_FILEMAN_AUTOSCROLL_LEFT;

	if( (x > dw - rw) && (x <= dw) )
	  sd->autoscroll.direction |= E_FILEMAN_AUTOSCROLL_RIGHT;

	if(sd->autoscroll.direction)
	  {
	     /* if the timer callback isnt set, set it */
	     if(!sd->autoscroll.timer)
	       {
		  sd->autoscroll.timer_int = 0.05;
		  sd->autoscroll.timer = ecore_timer_add(sd->autoscroll.timer_int, _e_fm_icon_autoscroll_cb, sd);
	       }
	  }
	else
	  {
	     /* we arent on the region, disable the callback */
	     if(sd->autoscroll.timer)
	       {
		  ecore_timer_del(sd->autoscroll.timer);
		  sd->autoscroll.timer = NULL;
	       }
	  }

     }
#endif
   

   /* hover */
     {
	Evas_List *l;
	/* check the position of every icon and compare the coords
	 * until we find an icon below
	 */
		  
	if(sd->operation.hover)
	  {
	     e_object_del(E_OBJECT(sd->operation.hover));
	     sd->operation.hover = NULL;
	  }
	for (l = sd->icons; l; l = l->next)
	  {
	     E_Fm_Icon *icon;
	     Evas_Coord xx, yy, ww, hh;

	     icon = l->data;
	     if(!icon->icon_obj)
	       continue;
	     evas_object_geometry_get(icon->icon_obj, &xx, &yy, &ww, &hh);
	     if (E_INTERSECTS(x, y, 1, 1, xx, yy, ww, hh))
	       {
		  icon->state.hover = 1;
		  e_fm_icon_signal_emit(icon->icon_obj, "hover", "");
		  if(sd->operation.hover)
		    {
		       e_object_del(E_OBJECT(sd->operation.hover));
		       sd->operation.hover = NULL;
		    }
		  sd->operation.hover = e_fm_file_new(icon->file->path);
		  
	       }
	     else
	       {
		  /* if the icon is outside the pointer and is hilighted
		   * unhilight it */
		  if(icon->state.hover)
		    {
		       e_fm_icon_signal_emit(icon->icon_obj, "unhover", "");
		       icon->state.hover = 0;
		    }
		  if(icon->state.selected)
		    e_fm_icon_signal_emit(icon->icon_obj, "clicked", "");
	       }
	  }
	/* if we arent above any icon, the file is the dir itself */
	if(!sd->operation.hover)
	  sd->operation.hover = e_fm_file_new(sd->dir);
     }
   
   rect.x = 0;
   rect.y = 0;
   rect.width = 0;
   rect.height = 0;

   ecore_x_dnd_send_status(1, 0, rect, ECORE_X_DND_ACTION_PRIVATE);

   return 1;
}

static int
_e_fm_xdnd_drop_cb(void *data, int type, void *event)
{
   Ecore_X_Event_Xdnd_Drop *ev;
   E_Fm_Smart_Data *sd;
   Evas_List *l;
   int dx, dy, dw, dh;
   int x,y;
   static E_Fm_Drag_Button button;

   ev = event;
   sd = data;

   /* we will receive this callback for every efm window, even if the drop isnt done
    * on this window
    */
   if (ev->win != sd->win->evas_win)
     {
	/* this event will be the first we have, the source window. 
	 * store the button that started the drag
	 */
	if(sd->drag.status == E_FILEMAN_DRAG_DOING)
	  {
	     //printf("inside drop saving the correct button %d\n", sd->drag.button);
	     button = sd->drag.button;
	  }
	return 1;
     }

   ecore_evas_geometry_get(sd->win->ecore_evas, &dx, &dy, &dw, &dh);
   if(sd->drag.status == E_FILEMAN_DRAG_DOING)
     {
	/* send the dropped signal to all the selected icons 
	 * move the selected icons relative to the cursor that
	 * that start the drag
	 * */
	
	x = ev->position.x - dx - sd->drag.x; 
	y = ev->position.y - dy - sd->drag.y;
	
	for(l = sd->selection.icons; l; l = l->next)
	  {
	     E_Fm_Icon *ic;
	     ic = (E_Fm_Icon*)l->data;
	     if(ic->icon_obj)
	       {
		  int ix, iy;
		  
		  evas_object_geometry_get(ic->icon_obj, &ix, &iy, NULL, NULL);
		  ix += x; 
		  iy += y; 
		  e_icon_canvas_child_move(ic->icon_obj,ix,iy);
		  e_fm_icon_signal_emit(ic->icon_obj, "dropped", "");
	     
		  if(sd->meta)
		    {
		       Evas_List *l;
		       for(l = sd->meta->files; l; l = l->next)
			 {
			    E_Fm_Icon_Metadata *m;

			    m = l->data;
			    if(!strcmp(m->name, sd->drag.icon_obj->file->name))
			      {
				 /* FIXME here we need the position relative to the canvas
				  * not the window */
				 //m->x = ix;
				 //m->y = iy;
				 break;
			      }
			 }
		    }
	       }
	  }
     }
   /* if we drop over another efm window */
   else
     {
	//printf("outside drop setting the correct button %d\n", button);
	sd->drag.button = button;
     }

   /* if we drop on an auto scrollable area, delete the timer */
   if(sd->autoscroll.timer)
     {
	ecore_timer_del(sd->autoscroll.timer);
	sd->autoscroll.timer = NULL;
     }
   ecore_x_selection_xdnd_request(sd->win->evas_win, "text/uri-list");
   return 1;
}

static int
_e_fm_xdnd_selection_cb(void *data, int type, void *event)
{
   Ecore_X_Event_Selection_Notify *ev;
   E_Fm_Smart_Data *sd;
   Ecore_X_Selection_Data_Files *files;

   ev = event;
   sd = data;
   
   if (ev->win != sd->win->evas_win) return 1;
	
   switch(ev->selection)
     {
      case ECORE_X_SELECTION_XDND:
	 if(!strcmp(ev->target, "text/uri-list"))
	   {
	      int i;
	      int remove = 7;  /* 7 = file:// */
	      files = ev->data;
	      if(files->num_files < 1)
		break;
	      /* free the operation struct before recreating */
	      _e_fm_operation_files_free(sd);
	      i = 0;
	      do
		{
		   E_Fm_File *f;

		   f = e_fm_file_new(files->files[i] + remove); 
		   sd->operation.files = evas_list_append(sd->operation.files,f);
		   i++;
		   remove = 0;
		} while(i < files->num_files);
	      
	      sd->operation.mime = e_fm_mime_get_from_list(sd->operation.files);
	      sd->drag.status = E_FILEMAN_DRAG_DONE;
	      _e_fm_fake_mouse_up_later(sd->win->evas,sd->drag.button);
	   }
	 break;
      default:
	 break;
     }
   ecore_x_dnd_send_finished();
   
   return 1;
}

static int
_e_fm_xdnd_finished_cb(void *data, int type, void *event)
{
   /* FIXME: If someone takes this internal drop, we might want to not free it */

   return 1;
}

/* sort functions */
static int
_e_fm_files_sort_name_cb(void *d1, void *d2)
{
   E_Fm_Icon *e1, *e2;

   e1 = d1;
   e2 = d2;

   return (strcmp(e1->file->name, e2->file->name));
}

static int
_e_fm_files_sort_layout_name_cb(void *d1, void *d2)
{
   Evas_Object *e1, *e2;
   E_Fm_File *f1, *f2;

   e1 = d1;
   e2 = d2;

   f1 = e_fm_icon_file_get(e1);
   f2 = e_fm_icon_file_get(e2);

   return (strcmp(f1->name, f2->name));
}

static int
_e_fm_files_sort_modtime_cb(void *d1, void *d2)
{
   E_Fm_Icon *e1, *e2;

   e1 = d1;
   e2 = d2;

   return (e1->file->mtime > e2->file->mtime);
}

static void
_e_fm_selector_send_file(E_Fm_Icon *icon)
{
   icon->sd->selector_func(icon->sd->object, strdup(icon->file->path), icon->sd->selector_data);
}

static void
_e_fm_selector_send_hilite_file(E_Fm_Icon *icon)
{
   icon->sd->selector_hilite_func(icon->sd->object, strdup(icon->file->path), icon->sd->selector_data);
}

/* Hash function based on directory stat information, Creates unique directory ID. */
static char *
_e_fm_dir_meta_dir_id(char *dir)
{
   char                s[256], *sp;
   const char         *chmap =
     "0123456789abcdef"
     "ghijklmnopqrstuv"
     "wxyz~!@#$%^&*()"
     "[];',.{}<>?-=_+|";
   unsigned int        id[4], i;
   struct stat         st;

   if (stat(dir, &st) < 0)
     return NULL;

   id[0] = st.st_ino;
   id[1] = st.st_dev;
   id[2] = (st.st_size & 0xffffffff);
   id[3] = (st.st_mtime & 0xffffffff);

   sp = s;
   for (i = 0; i < 4; i++)
     {
	unsigned int t, tt;
	int j;

	t = id[i];
	j = 32;
	while (j > 0)
	  {
	     tt = t & ((1 << 6) - 1);
	     *sp = chmap[tt];
	     t >>= 6;
	     j -= 6;
	     sp++;
	  }
     }
   *sp = 0;
   return strdup(s);
}

/* Read metadata from meta data file <meta_path>/<dir_hash>. Set meta data 
 * from file to sd->meta and create sd->meta->files_hash.
 *
 * @sd Smart Data for directory
 * @return 1 if file found and loaded 0 if no data was loaded
 */
static int
_e_fm_dir_meta_load(E_Fm_Smart_Data *sd)
{
   E_Fm_Dir_Metadata *m;
   Eet_File *ef;
   char buf[PATH_MAX];
   char *hash;

   if (!sd->dir) return 0;

   hash = _e_fm_dir_meta_dir_id(sd->dir);
   snprintf(buf, sizeof(buf), "%s/%s", meta_path, hash);
   free(hash);
   ef = eet_open(buf, EET_FILE_MODE_READ);
   if (!ef) return 0;
   m = eet_data_read(ef, _e_fm_dir_meta_edd, "metadata");
   eet_close(ef);

   /* Hash the metadata for quick access */
   if (m)
     {
	Evas_List *l;

	for (l = m->files; l; l = l->next)
	  {
	     E_Fm_Icon_Metadata *im;

	     im = l->data;
	     //printf("Loading meta: %d %d for file %s\n", im->x, im->y, im->name);
	     m->files_hash = evas_hash_add(m->files_hash, im->name, im);
	  }
     }
   sd->meta = m;
   return 1;
}

static int
_e_fm_dir_meta_generate(E_Fm_Smart_Data *sd)
{
   E_Fm_Dir_Metadata *m;
   Evas_List *l;

   if (!sd->dir) return 0;
   m = calloc(1, sizeof(E_Fm_Dir_Metadata));
   m->files = NULL;
   if (!m) return 0;
   _e_fm_dir_meta_fill(m, sd);
   for (l = sd->icons; l; l = l->next)
     {
	E_Fm_Icon_Metadata *im;
	E_Fm_Icon *icon;

	icon = l->data;
	im = e_fm_icon_meta_generate(icon->icon_obj);
	if (im)
	  {
	     m->files = evas_list_append(m->files, im);
	     m->files_hash = evas_hash_add(m->files_hash, icon->file->name, im);
	  }
     }
   sd->meta =  m;
   return 1;
}

static void
_e_fm_dir_meta_free(E_Fm_Dir_Metadata *m)
{
   if (!m) return;
   E_FREE(m->name);
   while (m->files)
     {
	E_Fm_Icon_Metadata *im;

	im = m->files->data;
	m->files = evas_list_remove_list(m->files, m->files);
	e_fm_icon_meta_free(im);
     }
   evas_hash_free(m->files_hash);
   free(m);
}

static int
_e_fm_dir_meta_save(E_Fm_Smart_Data *sd)
{
   Eet_File *ef;
   char     *hash;
   char      buf[4096];
   int       ret;

   if (!sd->meta) return 0;

     {
	Evas_List *l;

	for(l = sd->meta->files; l; l = l->next)
	  {
	     E_Fm_Icon_Metadata *m;

	     m = l->data;
	     //printf("Saving meta: %d %d for file %s\n", m->x, m->y, m->name);
	  }
     }

   hash = _e_fm_dir_meta_dir_id(sd->dir);
   snprintf(buf, sizeof(buf), "%s/%s", meta_path, hash);
   free(hash);
   ef = eet_open(buf, EET_FILE_MODE_WRITE);
   if (!ef) return 0;
   ret = eet_data_write(ef, _e_fm_dir_meta_edd, "metadata", sd->meta, 1);
   eet_close(ef);

   return ret;
}

static void
_e_fm_dir_meta_fill(E_Fm_Dir_Metadata *m, E_Fm_Smart_Data *sd)
{
   m->name = strdup(sd->dir);
   m->bg = NULL;
   m->view = 1;
}

/* displays the action menu for a list of selected files 
 * on the coordinate @dx, @dy relative to the @sd window
 * for the list of @files relative to a directory @dir
 */

static void
_e_fm_menu_action_display(E_Fm_Smart_Data *sd, Evas_Coord dx, Evas_Coord dy, unsigned int timestamp)
{

   E_Menu      *mn;
   E_Menu_Item *mi;
   int x, y, w, h;
   
   Evas_List *l;
   E_Fm_Mime_Action *action;
   E_Fm_Mime_Action *default_action;
   E_Fm_Mime_Entry *mime;
   E_Fm_Menu_Item_Data *mid;
   int multiple = 0; 


   /* if we dont have any operation files, how do we get here ? */
   if(!sd->operation.files)
     return;
   /* to know if the action can be performed on multiple files */
   if(sd->operation.files->next)
     multiple = 1;
   /* to know if the action can be performed on multiple files */
   if(sd->operation.files->next)
     multiple = 1;
 
   mn = e_menu_new();
   e_menu_category_set(mn,"fileman/action");
   e_menu_category_data_set("fileman/action",sd);

   mi = NULL;
   default_action = NULL;

   mime = sd->operation.mime;
   printf("mime for selection %s\n", mime->name);
   /* the default action */
   do
     {
	action = mime->action_default;
	     
	if(!action)
	  {
	     mime = mime->parent;
	     continue;
	  }
	if(action->relative)
	  continue;
	if(!action->multiple && multiple)
	  {
	     mime = mime->parent;
	     continue;
	  }

	mid = E_NEW(E_Fm_Menu_Item_Data,1);
	mid->action = action;
	mid->sd = sd;
	sd->operation.menu_item_data = evas_list_append(sd->operation.menu_item_data, mid);
	mi = e_menu_item_new(mn);
	e_menu_item_label_set(mi, _(action->label));
	e_menu_item_callback_set(mi, _e_fm_menu_actions, mid);
	e_menu_item_icon_edje_set(mi, (char *)e_theme_edje_file_get("base/theme/fileman",
		 "fileman/button/open"),"fileman/button/open");
   
	mi = e_menu_item_new(mn);
	e_menu_item_separator_set(mi, 1);

	default_action = action;
	break;
	
     } while(mime);
   /* the other possible actions */
   mime = sd->operation.mime;
   do
     {
	Evas_List *actions;
	printf("mime %s\n", mime->name);
	actions = mime->actions;
	for(l = actions; l; l = l->next)
	  {

	     action = (E_Fm_Mime_Action*)l->data;
	     if(!action->multiple && multiple)
	       continue;
	     /* if its the same as the default one, skip it */
	     if(action == default_action)
	       continue;
	     if(action->relative)
	       continue;
	     
	
	     mid = E_NEW(E_Fm_Menu_Item_Data,1);
	     mid->action = action;
	     mid->sd = sd;
	     sd->operation.menu_item_data = evas_list_append(sd->operation.menu_item_data, mid);
	     
	     mi = e_menu_item_new(mn);
	     e_menu_item_label_set(mi, _(action->label));
	     e_menu_item_callback_set(mi, _e_fm_menu_actions, mid);
	     e_menu_item_icon_edje_set(mi, (char *)e_theme_edje_file_get("base/theme/fileman",
             "fileman/button/open"),"fileman/button/open");
	       
	  }
	mime = mime->parent;
     } while(mime);
     
   mi = e_menu_item_new(mn);
   e_menu_item_separator_set(mi, 1);
   mi = e_menu_item_new(mn);

   e_menu_item_label_set(mi, _("Copy"));
   //e_menu_item_callback_set(mi, _e_fm_file_menu_copy, icon);
   e_menu_item_icon_edje_set(mi,
	 (char *)e_theme_edje_file_get("base/theme/fileman",
				       "fileman/button/copy"),
				       "fileman/button/copy");
   mi = e_menu_item_new(mn);
   e_menu_item_label_set(mi, _("Cut"));
   //e_menu_item_callback_set(mi, _e_fm_file_menu_cut, icon);
   e_menu_item_icon_edje_set(mi,
	 (char *)e_theme_edje_file_get("base/theme/fileman",
				       "fileman/button/cut"),
				       "fileman/button/cut");
   mi = e_menu_item_new(mn);
   e_menu_item_separator_set(mi, 1);
   
   if(!multiple)
     {
	mi = e_menu_item_new(mn);
	e_menu_item_label_set(mi, _("Rename"));
	//e_menu_item_callback_set(mi, _e_fm_file_menu_rename, icon);
	e_menu_item_icon_edje_set(mi,
	 (char *)e_theme_edje_file_get("base/theme/fileman",
				       "fileman/button/rename"),
				       "fileman/button/rename");
     }

   mi = e_menu_item_new(mn);
   e_menu_item_label_set(mi, _("Delete"));
   //e_menu_item_callback_set(mi, _e_fm_file_menu_delete, icon);
   e_menu_item_icon_edje_set(mi,
	 (char *)e_theme_edje_file_get("base/theme/fileman",
				       "fileman/button/delete"),
				       "fileman/button/delete");


   mi = e_menu_item_new(mn);
   e_menu_item_separator_set(mi, 1);

   mi = e_menu_item_new(mn);
   e_menu_item_label_set(mi, _("Properties"));
   //e_menu_item_callback_set(mi, _e_fm_file_menu_properties, icon);
   e_menu_item_icon_edje_set(mi,
	 (char *)e_theme_edje_file_get("base/theme/fileman",
				       "fileman/button/properties"),
				       "fileman/button/properties");
   sd->operation.menu = mn;
   
   if (!sd->win) return;
   
   ecore_evas_geometry_get(sd->win->ecore_evas, &x, &y, &w, &h);
   e_menu_activate_mouse(mn, sd->win->border->zone,x + dx, y + dy, 1, 1,
	 E_MENU_POP_DIRECTION_DOWN, timestamp);

}

/* displays the context menu on the coordinate
 * @dx, @dy relative to the @sd window
 */
static void
_e_fm_menu_context_display(E_Fm_Smart_Data *sd, Evas_Coord dx, Evas_Coord dy, unsigned int timestamp)
{
   E_Menu      *mn;
   E_Menu_Item *mi;

   int x, y, w, h;
   int  multiple = 0;
   Evas_List *l;
      
   E_Fm_Mime_Entry *mime;
   E_Fm_Mime_Action *action;
   E_Fm_Mime_Action *default_action;
   E_Fm_Menu_Item_Data *mid;
   
   /* if we dont have any operation files, how do we get here ? */
   if(!sd->operation.files)
     return;
   /* to know if the action can be performed on multiple files */
   if(sd->operation.files->next)
     multiple = 1;
    
   mn = e_menu_new();
   e_menu_category_set(mn,"fileman/action");
   e_menu_category_data_set("fileman/action",sd);

   mi = NULL;
   default_action = NULL;
   
   mime = sd->operation.mime;
   printf("mime for selection %s\n", mime->name);
   /* the default relative action */
   do
     {
	action = mime->action_default_relative;
	     
	if(!action)
	  {
	     mime = mime->parent;
	     continue;
	  }
	if(!action->multiple && multiple)
	  {
	     mime = mime->parent;
	     continue;
	  }

	mid = E_NEW(E_Fm_Menu_Item_Data,1);
	mid->action = action;
	mid->sd = sd;
	sd->operation.menu_item_data = evas_list_append(sd->operation.menu_item_data, mid);
	
	mi = e_menu_item_new(mn);
	e_menu_item_label_set(mi, e_fm_mime_translate(sd, action->label));
	e_menu_item_callback_set(mi, _e_fm_menu_actions, mid);
	e_menu_item_icon_edje_set(mi, (char *)e_theme_edje_file_get("base/theme/fileman",
		 "fileman/button/open"),"fileman/button/open");
   
	mi = e_menu_item_new(mn);
	e_menu_item_separator_set(mi, 1);

	default_action = action;
	break;
	
     } while(mime);
   
   /* the other relative actions */
   mime = sd->operation.mime;
   do
     {
	Evas_List *actions;
	
	printf("mime %s\n", mime->name);
	actions = mime->actions;
	for(l = actions; l; l = l->next)
	  {
	     action = (E_Fm_Mime_Action*)l->data;
	     /* if its not relative skip it */
	     if(!action->relative)
	       continue;
	     if(!action->multiple && multiple)
	       continue;
	     /* if its the same as the default one, skip it */
	     if(action == default_action)
	       continue;
	     
	
	     mid = E_NEW(E_Fm_Menu_Item_Data,1);
	     mid->action = action;
	     mid->sd = sd;
	     sd->operation.menu_item_data = evas_list_append(sd->operation.menu_item_data, mid);
	     
	     mi = e_menu_item_new(mn);
	     e_menu_item_label_set(mi, e_fm_mime_translate(sd, action->label));
	     e_menu_item_callback_set(mi, _e_fm_menu_actions, mid);
	     e_menu_item_icon_edje_set(mi, (char *)e_theme_edje_file_get("base/theme/fileman",
             "fileman/button/open"),"fileman/button/open");
	       
	  }
	mime = mime->parent;
     } while(mime);
   
   sd->operation.menu = mn;
  
   if (!sd->win) return;
   
   ecore_evas_geometry_get(sd->win->ecore_evas, &x, &y, &w, &h);
   e_menu_activate_mouse(mn, sd->win->border->zone,x + dx, y + dy, 1, 1,
	 E_MENU_POP_DIRECTION_DOWN, timestamp);
}

/* callback when a mime action is clicked on the actions menu */
static void                
_e_fm_menu_actions(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Fm_Menu_Item_Data *mid;

   mid = data;
   /* execute the action on the files */
   e_fm_mime_action_call(mid->sd,mid->action);
}

static void
_e_fm_operation_from_selection(E_Fm_Smart_Data *sd)
{

}

/* free functions */
/******************/
static void                 
_e_fm_operation_menu_free(E_Fm_Smart_Data *sd)
{
   if(sd->operation.menu)
	e_object_del(E_OBJECT(sd->operation.menu));
   sd->operation.menu = NULL;
   while(sd->operation.menu_item_data)
     {
	E_Fm_Menu_Item_Data *mid;

	mid = (E_Fm_Menu_Item_Data*)sd->operation.menu_item_data->data;
	E_FREE(mid);
	sd->operation.menu_item_data = evas_list_remove_list(sd->operation.menu_item_data, sd->operation.menu_item_data);
     }
}

static void
_e_fm_operation_files_free(E_Fm_Smart_Data *sd)
{
   while (sd->operation.files)
     {
	E_Fm_File *file;
	
	file = sd->operation.files->data;
	sd->operation.files = evas_list_remove_list(
	      sd->operation.files, sd->operation.files);
	/* also delete the file, its a copy of the selected or a new file
	 * in case of a drop
	 */
	e_object_del(E_OBJECT(file));
     }
   /* also delete the menu associated with this operation */
   _e_fm_operation_menu_free(sd);
}

static void
_e_fm_operation_free(E_Fm_Smart_Data *sd)
{
   if(sd->operation.hover)
     {
	e_object_del(E_OBJECT(sd->operation.hover));
	sd->operation.hover = NULL;
     }
   /* delete the files and the menu */
   _e_fm_operation_files_free(sd);
}
