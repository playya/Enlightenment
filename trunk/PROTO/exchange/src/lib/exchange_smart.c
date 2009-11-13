/* EXCHANGE - a library to interact with exchange.enlightenment.org
 * Copyright (C) 2008 Massimiliano Calamelli
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>
#include <Ecore_File.h>
#include "Exchange.h"
#include "exchange_private.h"

/* TODO
 *
 *    SMART OBJECT
 * Kill download in progress on exit
 *
 *    EXCHANGE
 * Add to Theme_Data the field filename
 * Screenshots and description for local themes ?? must be placed inside themes !!
 * Fix all the remaining free functions
 * fix all the typedef names with exchange_*
 */

typedef struct _Exchange_Smart_Data Exchange_Smart_Data;

//Evas Smart Object protos
static void _exchange_smart_add(Evas_Object *obj);
static void _exchange_smart_del(Evas_Object *obj);
static void _exchange_smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y);
static void _exchange_smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h);
static void _exchange_smart_show(Evas_Object *obj);
static void _exchange_smart_hide(Evas_Object *obj);
static void _exchange_smart_color_set(Evas_Object *obj, int r, int g, int b, int a);
static void _exchange_smart_clip_set(Evas_Object *obj, Evas_Object *clip);
static void _exchange_smart_clip_unset(Evas_Object *obj);

//Internals protos
static void _exchange_smart_size_hint_changed_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _exchange_smart_child_delete_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _exchange_smart_separator_append(Exchange_Smart_Data *sd, const char *text);
static void _exchange_smart_element_append(Exchange_Smart_Data *sd, Exchange_Object *td);
static void _exchange_smart_element_update(Evas_Object *elem, Exchange_Object *td, Exchange_Smart_Data *sd);
static const char *_exchange_user_homedir_get(void);

//Internal Callbacks protos
static const char*_exchange_smart_thumb_get(Evas_Object *elem, int id, const char *url);
static void _exchange_smart_thumb_swallow(Evas_Object *elem, const char *thumb);
static void _download_thumb_complete_cb(void *data, const char *file, int status);
static int _download_thumb_progress_cb(void *data, const char *file, long int dltotal, long int dlnow, long int ultotal, long int ulnow);
void _download_theme_complete_cb(void *data, const char *file, int status);
int _download_theme_progress_cb(void *data, const char *file, long int dltotal, long int dlnow, long int ultotal, long int ulnow);
int _exchange_smart_themes_sort_cb(const void *d1, const void *d2);
static void _exchange_smart_button_click_cb(void *data, Evas_Object *o, const char *emission, const char *source);
static void _exchange_smart_thumb_swallow(Evas_Object *elem, const char *thumb);


struct _Exchange_Smart_Data
{
   Evas_Coord       x, y, w, h;  //Coords of the whole smart object
   struct {
      Evas_Coord    x, y;        //Offset to use
   } offset;

   Evas_Object     *obj_box;     //The Evas Smart Box
   Evas_Object     *obj_lbl;     //The big label
   const char      *group;       //Exchange theme group
   const char      *local_dir;   //Local user themes directory

   struct {
      void (*func)(const char *path, void *data);
      void *data;
   } apply;                      //Callback for the use button press
};

//Globals
static Evas_Smart *_smart = NULL;
static const char *_smart_theme = NULL;
static const char *_smart_cache = NULL;

/**
 * @addtogroup Exchange_Smart_Group Exchange Smart Object Functions
 * @{
 */

/**
 * @param evas The evas canvas
 * @return A new exchange smart object
 * @brief Create a new exchange smart object
 */
EAPI Evas_Object*
exchange_smart_object_add(Evas *evas)
{
   if (!evas) return NULL;
   if (!exchange_smart_init()) return NULL;
   return evas_object_smart_add(evas, _smart);
}

/**
 * @param obj The exchange smart object
 * @param group The name of the group to search for (could be "Borders", "Wallpapers", etc)
 * @return 1 on success, 0 on errors
 * @brief Set the group to serach for
 * The group is the title of one of the group that you can find on exchange.enlightement.org.
 * If group is NULL then all the themes from exchange is shown.
 */
EAPI unsigned char
exchange_smart_object_remote_group_set(Evas_Object *obj, const char *group)
{
   Exchange_Smart_Data *sd;
   //EINA_LOG_DBG("group:%s\n", group);

   sd = evas_object_smart_data_get(obj);
   if (!sd) return 0;

   if (sd->group) eina_stringshare_del(sd->group);
   sd->group = eina_stringshare_add(group);
   return 1;
}

/**
 * @param obj The exchange smart object
 * @param system The directory where the system themes are stored.
 * @param user The directory where the user themes are stored.
 * @return 1 on success, 0 on errors
 * @brief Set the local directorys for themes
 * You need to set at last the user directory if you want to use
 * EXCHANGE_SMART_SHOW_LOCAL or EXCHANGE_SMART_SHOW_BOTH.
 */
EAPI unsigned char
exchange_smart_object_local_path_set(Evas_Object *obj, const char *user_dir)
{
   Exchange_Smart_Data *sd;
   //EINA_LOG_DBG("user:'%s' system: '%s'\n", user, system);

   if (!obj) return 0;
   sd = evas_object_smart_data_get(obj);
   if (!sd) return 0;

   if (sd->local_dir) eina_stringshare_del(sd->local_dir);
   sd->local_dir = ecore_file_is_dir(user_dir) ? eina_stringshare_add(user_dir) : NULL;
   return 1;
}

/**
 * @param obj The exchange smart object
 * @param x The horizontal offset in pixel
 * @param y The vertical offset in pixel
 * @return 1 on success, 0 on errors
 * @brief Set the offset of the smart object.
 * Usefull for use the object inside a scroller container
 */
EAPI unsigned char
exchange_smart_object_offset_set(Evas_Object *obj, int x, int y)
{
   Exchange_Smart_Data *sd;
   //EINA_LOG_DBG("x:%d y:%d\n", x, y);

   sd = evas_object_smart_data_get(obj);
   if (!sd) return 0;

   if (x == sd->offset.x && y == sd->offset.y)
      return 0;

   sd->offset.x = x;
   sd->offset.y = y;

   evas_object_move(sd->obj_box, sd->x - x, sd->y - y);
   return 1;
}

/**
 * @param obj The exchange smart object
 * @param x The address where to store the horizontal offset
 * @param y The address where to store the vertical offset
 * @return 1 on success, 0 on errors
 * @brief Get the offset of the smart object.
 * Usefull for use the object inside a scroller container
 */
EAPI unsigned char
exchange_smart_object_offset_get(Evas_Object *obj, int *x, int *y)
{
   Exchange_Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return 0;

   if (x) *x = sd->offset.x;
   if (y) *y = sd->offset.y;
   return 1;
}

/**
 * @param obj The exchange smart object
 * @param func The function to call when the button is pressed
 * @param data Data to attach to the callback function
 * @return 1 on success, 0 on errors
 * @brief Set the function to call when the user press 'Select'
 * If you don't set any callback then no 'Select' button is presented
 */
EAPI unsigned char
exchange_smart_object_apply_cb_set(Evas_Object *obj, void (*func)(const char *path, void *data), void *data)
{
   Exchange_Smart_Data *sd;

   if (!obj || !func) return 0;
   sd = evas_object_smart_data_get(obj);
   if (!sd) return 0;

   sd->apply.func = func;
   sd->apply.data = data ? data : NULL;
   return 1;
}



unsigned char
_list_complete_cb(Eina_List *results, void *data)
{
   Eina_List *l;
   Exchange_Object *td;
   Exchange_Smart_Data *sd = data;
   char buf[255];

   EINA_LOG_DBG("POPULATING... %d\n", eina_list_count(results));

   /* Populate the List */
   evas_object_text_text_set(sd->obj_lbl, "");

   snprintf(buf, sizeof(buf),"%s (%d)", "Online", eina_list_count(results));
   _exchange_smart_separator_append(sd, buf);

   results = eina_list_sort(results, 0, _exchange_smart_themes_sort_cb);

   EINA_LIST_FOREACH(results, l, td)
      _exchange_smart_element_append(sd, td);

   return 0;
}

/**
 * @param obj The exchange smart object
 * @return 1 on success, 0 on errors
 * @brief Run the given smart object
 * Start a new query using the setted parameters
 */
EAPI unsigned char
exchange_smart_object_run(Evas_Object *obj)
{
   Exchange_Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return 0;

   evas_object_box_remove_all(sd->obj_box, 1);
   exchange_smart_object_offset_set(obj, 0, 0);

   if (exchange_remote_list(sd->group, NULL, 0, 0, 0, NULL, 0, 0,
                            _list_complete_cb, sd))
      evas_object_text_text_set(sd->obj_lbl, "Getting data...");
   else
      evas_object_text_text_set(sd->obj_lbl, "Error fetching data");

   return 1;
}

/**
 * @}
 */
unsigned char
exchange_smart_init(void)
{
   char buf[4096]; //TODO FIXME MAX_PATH

   eina_error_log_level_set(EINA_LOG_LEVEL_WARN);
   EINA_LOG_DBG("\n");

   /* check theme file */
   if (!_smart_theme)
   {
      snprintf(buf, sizeof(buf), "%s/exchange_smart.edj", PACKAGE_DATA_DIR);
      if (!ecore_file_exists(buf))
      {
         EINA_ERROR_PERR("Can't find smart theme file '%s'\n", buf);
         return 0;
      }
      _smart_theme = eina_stringshare_add(buf);
   }

   /* check cache dir */
   if (!_smart_cache)
   {
      snprintf(buf, sizeof(buf), "%s/.exchange", _exchange_user_homedir_get());
      if (!ecore_file_exists(buf) &&
          !ecore_file_mkpath(buf))
      {
         EINA_ERROR_PERR("Can't create cache dir '%s'\n", buf);
         return 0;
      }
      _smart_cache = eina_stringshare_add(buf);
   }

   /* create the smart class */
   if (!_smart)
   {
      static const Evas_Smart_Class sc =
      {
         "Exchange_Smart_Class",
         EVAS_SMART_CLASS_VERSION,
         _exchange_smart_add,
         _exchange_smart_del,
         _exchange_smart_move,
         _exchange_smart_resize,
         _exchange_smart_show,
         _exchange_smart_hide,
         _exchange_smart_color_set,
         _exchange_smart_clip_set,
         _exchange_smart_clip_unset,
         NULL,
         NULL,
         NULL,
         NULL
      };
      _smart = evas_smart_class_new(&sc);
   }

   return 1;
}

void
exchange_smart_shutdown(void)
{
   if (_smart_theme) eina_stringshare_del(_smart_theme);
   if (_smart_cache) eina_stringshare_del(_smart_cache);
   //if (_smart) TODO How to del the smart class?
   eina_shutdown();
}

/*** The Evas Smart Object ***/
static void
_exchange_smart_add(Evas_Object *obj)
{
   Exchange_Smart_Data *sd;

   if (!obj) return;

   /* Create the Smart_Data */
   sd = calloc(1, sizeof(Exchange_Smart_Data));
   if (!sd) return;

   sd->x = 0;
   sd->y = 0;
   sd->w = 0;
   sd->h = 0;
   sd->offset.x = 0;
   sd->offset.y = 0;
   sd->group = NULL;
   sd->local_dir = NULL;
   sd->apply.func = NULL;
   sd->apply.data = NULL;

   /* Create the evas_box */
   sd->obj_box = evas_object_box_add(evas_object_evas_get(obj));
   evas_object_box_layout_set(sd->obj_box, evas_object_box_layout_vertical,
                              NULL, NULL);
   evas_object_box_align_set(sd->obj_box, 0.0, 0.0);
   evas_object_box_padding_set(sd->obj_box, 0, 5);
   evas_object_smart_member_add(sd->obj_box, obj); //???

   evas_object_event_callback_add(sd->obj_box, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                  _exchange_smart_size_hint_changed_cb, obj); //TODO FREE??

   /* Create the label */
   sd->obj_lbl = evas_object_text_add(evas_object_evas_get(obj));
   evas_object_text_text_set(sd->obj_lbl, "Not yet started!");
   evas_object_text_font_set(sd->obj_lbl, "Sans", 26);
   evas_object_size_hint_align_set(sd->obj_lbl, 0.5, 0.5);
   evas_object_text_style_set(sd->obj_lbl, EVAS_TEXT_STYLE_SOFT_SHADOW);

   evas_object_color_set(sd->obj_lbl, 255, 255, 255, 255);
   evas_object_text_shadow_color_set(sd->obj_lbl, 150, 150, 150, 255);
   evas_object_show(sd->obj_lbl);
   evas_object_smart_member_add(sd->obj_lbl, obj);

   evas_object_smart_data_set(obj, sd);
}

static void
_exchange_smart_del(Evas_Object *obj)
{
   EINA_LOG_DBG("%p\n", obj);
   Exchange_Smart_Data *sd;

   if (!obj) return;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;

   //TODO Kill all download in progress
   if (sd->group) eina_stringshare_del(sd->group);
   if (sd->local_dir) eina_stringshare_del(sd->local_dir);
   evas_object_del(sd->obj_box);//TODO is this free ok?? or we need to del all the elements??
   free(sd);
}

static void
_exchange_smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   //EINA_LOG_DBG("\n");
   Exchange_Smart_Data *sd;

   if (!obj) return;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if ((x == sd->x) && (y == sd->y)) return;

   evas_object_move(sd->obj_box, x - sd->offset.x, y - sd->offset.y);
   evas_object_move(sd->obj_lbl, x + 20, y + 80);//TODO want  align 0.5 0.3 !!

   sd->x = x;
   sd->y = y;
}

static void
_exchange_smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   //EINA_LOG_DBG("\n");
   Exchange_Smart_Data *sd;

   if (!obj) return;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if ((w == sd->w) && (h == sd->h)) return;
   sd->w = w;
   sd->h = h;

   evas_object_resize(sd->obj_box, w, h);
   evas_object_resize(sd->obj_lbl, w, h);
}

static void
_exchange_smart_show(Evas_Object *obj)
{
   //EINA_LOG_DBG("\n");
   Exchange_Smart_Data *sd;

   if (!obj) return;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (sd->obj_box) evas_object_show(sd->obj_box);
}

static void
_exchange_smart_hide(Evas_Object *obj)
{
   //EINA_LOG_DBG("\n");
   Exchange_Smart_Data *sd;

   if (!obj) return;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (sd->obj_box) evas_object_hide(sd->obj_box);

}

static void
_exchange_smart_color_set(Evas_Object *obj, int r, int g, int b, int a)
{
   //EINA_LOG_DBG("\n");
   Exchange_Smart_Data *sd;

   if (!obj) return;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;

   evas_object_color_set(sd->obj_box, r, g, b, a);
}

static void
_exchange_smart_clip_set(Evas_Object *obj, Evas_Object *clip)
{
   //EINA_LOG_DBG("\n");
   Exchange_Smart_Data *sd;

   if (!obj) return;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;

   evas_object_clip_set(sd->obj_box, clip);
}

static void
_exchange_smart_clip_unset(Evas_Object *obj)
{
   //EINA_LOG_DBG("\n");
   Exchange_Smart_Data *sd;

   if (!obj) return;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;

   evas_object_clip_unset(sd->obj_box);
}

/*** Private functions ***/
static const char *
_exchange_user_homedir_get(void)
{ // TODO This function should be moved into to ecore
   char *homedir;
   int len;

   homedir = getenv("HOME");
   if (!homedir) return "/tmp";
   len = strlen(homedir);
   while ((len > 1) && (homedir[len - 1] == '/'))
     {
        homedir[len - 1] = 0;
        len--;
     }
   return homedir;
}

static void
_exchange_smart_size_hint_changed_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Object *smart = data;
   int w, h;

   evas_object_size_hint_min_get(obj, &w, &h);
   evas_object_size_hint_min_set(smart, w, h);
}

static void
_exchange_smart_child_delete_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Exchange_Object *td = data;
   Evas_Object *img;

   if (!td) return;
   exchange_object_free(td);

   img = edje_object_part_swallow_get(obj, "thumb.swallow");
   if (img) evas_object_del(img);
}

static const char*
_exchange_smart_thumb_get(Evas_Object *elem, int id, const char *url)
{
   char dst[4096];

   if (!url) return NULL;

   if (strstr(url, "/files/module/"))
      snprintf(dst, sizeof(dst), "%s/module.%d.thumb.png", _smart_cache, id);
   else if (strstr(url, "/files/application/"))
      snprintf(dst, sizeof(dst), "%s/application.%d.thumb.png", _smart_cache, id);
   else
      snprintf(dst, sizeof(dst), "%s/%d.thumb.png", _smart_cache, id);

   //check if we have a copy in cache...
   if (ecore_file_exists(dst)) //TODO check if the thumb is updated
      return eina_stringshare_add(dst);

   //...else start downloading
   ecore_file_download(url, dst, _download_thumb_complete_cb, _download_thumb_progress_cb, elem, NULL);
   edje_object_signal_emit(elem, "set,busy", "exchange");

   return NULL;
}

void
_download_thumb_complete_cb(void *data, const char *file, int status)
{
   Evas_Object *elem = data;

   EINA_LOG_DBG("THUMB COMPLETE %d %s\n", status, file);
   //TODO check if download finish well
   edje_object_signal_emit(elem, "set,idle", "exchange");
   _exchange_smart_thumb_swallow(elem, file);
}

int
_download_thumb_progress_cb(void *data, const char *file, long int dltotal, long int dlnow, long int ultotal, long int ulnow)
{
   //Evas_Object *elem = data;
   //EINA_LOG_DBG("DOWNLOAD THUMB PROGRESS '%s' dltotal: %ld dlnow: %ld\n", file, dltotal, dlnow);
   return 0;
}

static void
_exchange_smart_thumb_swallow(Evas_Object *elem, const char *thumb)
{
   Evas_Object *img;
   Evas_Coord w, h;

   //EINA_LOG_DBG("SWALLOW %s\n", thumb);
   img = evas_object_image_add(evas_object_evas_get(elem));
   evas_object_image_file_set(img, thumb, NULL);
   evas_object_image_size_get(img, &w, &h);
   evas_object_move(img, 0, 0);
   evas_object_resize(img, w, h);
   evas_object_image_fill_set(img, 0, 0, w, h);
   //evas_object_clip_set(img, box);
   //evas_object_smart_member_add(sd->obj_bg, obj); //???
   edje_object_part_swallow(elem, "thumb.swallow", img);
   evas_object_show(img);
}

int
_exchange_smart_themes_sort_cb(const void *d1, const void *d2)
{
   Exchange_Object *t1 = (Exchange_Object *)d1;
   Exchange_Object *t2 = (Exchange_Object *)d2;

   if(!t1 || !t1->name) return 1;
   if(!t2 || !t2->name) return -1;

   return strcmp(t1->name, t2->name);
}

static void
_exchange_smart_separator_append(Exchange_Smart_Data *sd, const char *text)
{
   Evas_Object *sep;
   Evas_Coord w, h;
   Evas_Object_Box_Option *opt;

   if (!sd || !_smart_theme) return;

   sep = edje_object_add(evas_object_evas_get(sd->obj_box));
   edje_object_file_set(sep, _smart_theme, "exchange/smart/separator");
   edje_object_size_min_get(sep, &w, &h);
   evas_object_resize(sep, 480, h);  //TODO FIXME 500
   //evas_object_size_hint_min_set(elem, w, h);
   //evas_object_size_hint_request_set(elem, w, h);
   evas_object_show(sep); //TODO WE NEED THIS???
   evas_object_smart_member_add(sep, sd->obj_box); //???
   //evas_object_data_set(elem, "EXCHANGE_SMART_DATA", sd);

   opt = evas_object_box_append(sd->obj_box, sep);
   evas_object_size_hint_align_set(opt->obj, 0.0, 0.0);
   evas_object_size_hint_padding_set(opt->obj, 0, 0, 0, 0);
   evas_object_size_hint_weight_set(opt->obj, 0.0, 0.0);

   if (text) edje_object_part_text_set(sep, "text", text);
}

static void
_exchange_smart_element_update(Evas_Object *elem, Exchange_Object *td, Exchange_Smart_Data *sd)
{
   char buf[4096];
   const char *thumb;
   char *local_ver = NULL;

   snprintf(buf, sizeof(buf), "<title>%s </title> <version>%s</version>",
            td->name, td->version ? td->version : "");
   edje_object_part_text_set(elem, "textblock", buf);
   if (strlen(td->description))
      edje_object_part_text_set(elem, "textblock2", td->description);
   else
      edje_object_part_text_set(elem, "textblock2", "No description available");

   if (td->rating < 0.0)
      edje_object_signal_emit(elem, "set,star,hide,all", "exchange");
   else if (td->rating < 1.0)
      edje_object_signal_emit(elem, "set,star,0", "exchange");
   else if (td->rating < 2.0)
      edje_object_signal_emit(elem, "set,star,1", "exchange");
   else if (td->rating < 3.0)
      edje_object_signal_emit(elem, "set,star,2", "exchange");
   else if (td->rating < 4.0)
      edje_object_signal_emit(elem, "set,star,3", "exchange");
   else if (td->rating < 5.0)
      edje_object_signal_emit(elem, "set,star,4", "exchange");
   else
      edje_object_signal_emit(elem, "set,star,5", "exchange");

   //This are all the signals that the element accept
   //edje_object_signal_emit(elem, "set,updatable", "exchange");
   //edje_object_signal_emit(elem, "set,updated", "exchange");
   //edje_object_signal_emit(elem, "set,busy", "exchange");
   //edje_object_signal_emit(elem, "set,idle", "exchange");
   //edje_object_signal_emit(elem, "gauge,show","exchange");
   //edje_object_signal_emit(obj, "gauge,hide","exchange");
   //edje_object_part_drag_size_set(elem, "gauge.bar", 0.3, 0.0);
   //edje_object_signal_emit(elem, "download,disable","exchange");
   //edje_object_signal_emit(elem, "download,enable","exchange");
   //edje_object_signal_emit(elem, "use,disable","exchange");
   //edje_object_signal_emit(elem, "use,enable","exchange");

   snprintf(buf, sizeof(buf), "%s/%s.edj", sd->local_dir, td->name);
   if (ecore_file_exists(buf))
   {
      local_ver = exchange_local_theme_version_get(buf);
      if (td->version && local_ver)
      {
         if (strcmp(td->version, local_ver))
         {
            //need update
            edje_object_signal_emit(elem, "set,updatable", "exchange");
            edje_object_signal_emit(elem, "download,enable","exchange");
            edje_object_signal_emit(elem, "use,enable","exchange");
            edje_object_part_text_set(elem, "btn_download.text", "Update");
         }
         else
         {
            //updated
            edje_object_signal_emit(elem, "set,updated", "exchange");
            edje_object_signal_emit(elem, "download,disable","exchange");
            edje_object_signal_emit(elem, "use,enable","exchange");
         }
      }
      EINA_LOG_DBG("CHECKING: %s %s(%s %s)\n", td->name, td->version, buf, local_ver);
      if (local_ver) free(local_ver);
   }
   else
   {
      //new theme
      edje_object_signal_emit(elem, "set,new", "exchange");
      edje_object_signal_emit(elem, "download,enable","exchange");
      edje_object_signal_emit(elem, "use,disable","exchange");
   }

   thumb = _exchange_smart_thumb_get(elem, td->id, td->thumbnail);
   if (thumb)
   {
      _exchange_smart_thumb_swallow(elem, thumb);
      eina_stringshare_del(thumb);
   }
}

static void
_exchange_smart_element_append(Exchange_Smart_Data *sd, Exchange_Object *td)
{
   Evas_Object *elem;
   Evas_Object_Box_Option *opt;
   Evas_Coord w, h;

   /* Create the element edje object */
   elem = edje_object_add(evas_object_evas_get(sd->obj_box));
   edje_object_file_set(elem, _smart_theme, "exchange/smart/element");
   edje_object_size_min_get(elem, &w, &h);
   evas_object_resize(elem, 480, h);  //TODO FIXME 500
   //evas_object_size_hint_min_set(elem, w, h);
   //evas_object_size_hint_request_set(elem, w, h);
   evas_object_show(elem);
   evas_object_smart_member_add(elem, sd->obj_box);
   evas_object_data_set(elem, "EXCHANGE_SMART_DATA", sd); //TODO FREE ??
   edje_object_signal_callback_add(elem, "clicked", "btn_*",
                                   _exchange_smart_button_click_cb,  td); //TODO FREEME ??
   evas_object_event_callback_add(elem, EVAS_CALLBACK_DEL,
                                  _exchange_smart_child_delete_cb, td); //TODO FREE??

   /* Append the element to the box */
   opt = evas_object_box_append(sd->obj_box, elem);
   evas_object_size_hint_align_set(opt->obj, 0.0, 0.0);
   evas_object_size_hint_padding_set(opt->obj, 0, 0, 0, 0);
   evas_object_size_hint_weight_set(opt->obj, 0.0, 0.0);

   _exchange_smart_element_update(elem, td, sd);
}

void
_download_theme_complete_cb(void *data, const char *file, int status)
{
   Evas_Object *obj = data;
   Exchange_Smart_Data *sd = evas_object_data_get(obj, "EXCHANGE_SMART_DATA");

   if (!obj || !sd) return;

   EINA_LOG_DBG("[status %d] %s\n", status, file);
   edje_object_signal_emit(obj, "set,idle","exchange");
   edje_object_signal_emit(obj, "gauge,hide","exchange");
   edje_object_part_text_set(obj, "btn_download.text", "Completed");
   edje_object_signal_emit(obj, "use,enable","exchange");
}

int
_download_theme_progress_cb(void *data, const char *file, long int dltotal, long int dlnow, long int ultotal, long int ulnow)
{
   Evas_Object *obj = data;
   Exchange_Smart_Data *sd = evas_object_data_get(obj, "EXCHANGE_SMART_DATA");
   static int count = 0;

   if (!obj || !sd) return 0;

   /* NOTE Ecore call a lot of progress callback.
    * Try to skip some...need test on different connections */
   if (count++ > 100) count = 0;
   else return 0;

   //EINA_LOG_DBG("'%s' dltotal: %ld dlnow: %ld ultotal: %ld ulnow: %ld\n", file, dltotal, dlnow, ultotal, ulnow);

   if (dltotal && dlnow)
      edje_object_part_drag_size_set(obj, "gauge.bar", (float)dlnow / (float)dltotal, 0.0);
   else
      edje_object_part_drag_size_set(obj, "gauge.bar", 0.0, 0.0);
   return 0;
}

static void
_exchange_smart_button_click_cb(void *data, Evas_Object *obj, const char *em, const char *src)
{
   Exchange_Smart_Data *sd;
   Exchange_Object *td = data;
   char dst[4096];

   sd = evas_object_data_get(obj, "EXCHANGE_SMART_DATA");
   if (!sd) return;

   if (!strcmp(src, "btn_download") && td->url)
   {
      snprintf(dst, sizeof(dst),"%s/%s.edj", sd->local_dir, td->name);
      EINA_LOG_DBG("DOWNLOAD URL %s\n", td->url);
      EINA_LOG_DBG("DOWNLOAD DST %s\n", dst);

      edje_object_signal_emit(obj, "set,updated","exchange");
      edje_object_signal_emit(obj, "set,busy","exchange");
      edje_object_signal_emit(obj, "set,downloading","exchange");

      edje_object_signal_emit(obj, "gauge,show","exchange");
      edje_object_part_drag_size_set(obj, "gauge.bar", 0.0, 0.0);
      edje_object_part_text_set(obj, "btn_download.text", "Downloading...");

      if (ecore_file_exists(dst)) ecore_file_unlink(dst);
      ecore_file_download(td->url, dst, _download_theme_complete_cb,
                          _download_theme_progress_cb, obj, NULL);
   }
   else if (!strcmp(src, "btn_use") && sd->apply.func)
   {
      snprintf(dst, sizeof(dst),"%s/%s.edj", sd->local_dir, td->name);
      if (ecore_file_exists(dst))
         sd->apply.func(dst, sd->apply.data);
   }
}

