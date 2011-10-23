#include <Elementary.h>
#include <Eio.h>
#include "enna.h"
#include "gettext.h"
#include "udisks.h"

typedef struct _Enna_Shortcut Enna_Shortcut;
typedef struct _Device_Item Device_Item;

struct _Enna_Shortcut
{
   Evas_Object *list;
   Efreet_Desktop *ef;
   char *target;
};

static Eina_List *_device_items = NULL;
static Evas_Object *list;

struct _Device_Item
{
   Evas_Object *list;
   Elm_Genlist_Item *gi;
   Enna_Volume *v;
   const char *udi;
};

static Eina_List *
_enna_shortcut_parse_gtk_bookmarks(void)
{
   char line[PATH_MAX];
   char buf[PATH_MAX];
   Efreet_Uri *uri;
   char *alias;
   FILE *fp;
   Eina_List *list = NULL;

   snprintf(buf, sizeof(buf), "%s/.gtk-bookmarks", getenv("HOME"));
   fp = fopen(buf, "r");
   if (fp)
     {
        while(fgets(line, sizeof(line), fp))
          {
             alias = NULL;
             line[strlen(line) - 1] = '\0';
             alias = strchr(line, ' ');
             if (alias)
               {
                  line[alias - line] = '\0';
                  alias++;
               }
             uri = efreet_uri_decode(line);
             if (uri && uri->path)
               {
                  if (ecore_file_exists(uri->path))
                    {
                       list = eina_list_append(list, eina_stringshare_add(uri->path));
                    }
               }
             if (uri) efreet_uri_free(uri);
          }
        fclose(fp);
     }

   return list;
}



static Device_Item *
_volume_find(const char *udi)
{

   Eina_List *l;
   Device_Item *di;

   if (!udi)  return NULL;

   EINA_LIST_FOREACH(_device_items, l, di)
     {
        if (!di->v || !di->v->udi) continue;
        if (!strcmp(udi, di->udi)) return di;
     }

   return NULL;

}

static char *
_enna_shortcut_favorite_label_get(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   const char *label = data;
   return strdup(label);
}

static Evas_Object *
_enna_shortcut_favorite_icon_get(void *data __UNUSED__, Evas_Object *obj, const char *part)
{
   return NULL;
}

static Elm_Genlist_Item_Class itc_favorite_group = {
  "panel",
  {
    _enna_shortcut_favorite_label_get,
    _enna_shortcut_favorite_icon_get,
    NULL,
    NULL,
    NULL
  },
  NULL
};

static char *
_enna_shortcut_desktop_label_get(void *data, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   Enna_Shortcut *es = data;

   if (es && es->ef)
     return strdup(es->ef->name);
   else if (es)
     return strdup(ecore_file_file_get(es->target));
   else
     return strdup("Test");
}

static Evas_Object *
_enna_shortcut_desktop_icon_get(void *data __UNUSED__, Evas_Object *obj, const char *part __UNUSED__)
{
   Enna_Shortcut *es = data;
   Evas_Object *ic = elm_icon_add(obj);
   elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
   if (es && es->ef)
     elm_icon_standard_set(ic, es->ef->icon);
   else
     elm_icon_standard_set(ic, "emblem-favorite");
   return ic;
}

#if 0
static void
_enna_shortcut_desktop_select(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   Elm_Genlist_Item *item = event_info;
   Enna_Shortcut *es;
   Evas_Object *grid = data;

   es = elm_genlist_item_data_get(item);
   elm_genlist_item_selected_set(item, EINA_FALSE);
   if (!es) return ;
   enna_browse_directory(grid, es->target);
}
#endif

static Elm_Genlist_Item_Class itc_desktop_group = {
  "panel",
  {
    _enna_shortcut_desktop_label_get,
    _enna_shortcut_desktop_icon_get,
    NULL,
    NULL,
    NULL
  },
  NULL
};

static char *
_bookmark_label_get(void *data, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   const char *label = data;

   return strdup(ecore_file_file_get(label));
}

static Evas_Object *
_bookmark_icon_get(void *data, Evas_Object *obj, const char *part)
{
   const char *label = ecore_file_file_get(data);
   Evas_Object *ic;

   if (strcmp(part, "elm.swallow.icon"))
     return NULL;

   ic = elm_icon_add(obj);
   elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
   elm_icon_smooth_set(ic, EINA_FALSE);
   if (!strcmp(label, _("Music")))
     elm_icon_standard_set(ic, "folder-music");
   else if (!strcmp(label, _("Videos")))
     elm_icon_standard_set(ic, "folder-videos");
   else if (!strcmp(label, _("Images")))
     elm_icon_standard_set(ic, "folder_pictures");
   else if (!strcmp(label, _("Downloads")))
     elm_icon_standard_set(ic, "folder-downloads");
   else if (!strcmp(label, _("Documents")))
     elm_icon_standard_set(ic, "folder-documents");
   else
     elm_icon_standard_set(ic, "inode-directory");

   //   evas_object_size_hint_min_set(ic, 16, 16);
   evas_object_show(ic);


   return ic;
}


static char *
_bookmark_volume_label_get(void *data, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   Enna_Volume *volume = data;


   return strdup(ecore_file_file_get(volume->label));
}

static void
_volume_eject_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
   Enna_Volume *v = data;

   if (v->storage && v->storage->requires_eject)
     enna_udisks_volume_eject(v);
   else
     enna_udisks_volume_unmount(v);
}


static Evas_Object *
_bookmark_volume_icon_get(void *data, Evas_Object *obj, const char *part)
{
   Enna_Volume *v = data;

   if (v->mounted && !strcmp(part, "elm.swallow.end"))
     {
        Evas_Object *bt;
        Evas_Object *ic;

        bt = elm_button_add(obj);
        ic = elm_icon_add(bt);
        elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
        elm_icon_standard_set(ic, "media-eject");
        elm_object_style_set(bt, "anchor");
        elm_button_icon_set(bt, ic);
        evas_object_show(ic);
        evas_object_show(bt);
        evas_object_size_hint_min_set(bt, 24, 24);
        evas_object_smart_callback_add(bt, "clicked", _volume_eject_clicked_cb, v);

        return bt;
     }
   else if (!strcmp(part, "elm.swallow.icon"))
     {
        Evas_Object *ic;
        ic = elm_icon_add(obj);
        elm_icon_order_lookup_set(ic, ELM_ICON_LOOKUP_FDO_THEME);
        elm_icon_standard_set(ic, "block-device");
        evas_object_show(ic);
        return ic;
     }
   return NULL;
}


static Elm_Genlist_Item_Class itc_bookmark = {
  "panel",
  {
    _bookmark_label_get,
    _bookmark_icon_get,
    NULL,
    NULL,
    NULL
  },
  NULL
};

static Elm_Genlist_Item_Class itc_bookmark_volume = {
  "panel",
  {
    _bookmark_volume_label_get,
    _bookmark_volume_icon_get,
    NULL,
    NULL,
    NULL
  },
  NULL
};

static void
_enna_shortcut_cleanup(Enna_Shortcut *es)
{
   if (es->ef)
     efreet_desktop_free(es->ef);
   free(es->target);
   free(es);
}

static void
_desktop_stat_ok(void *data, Eio_File *handler __UNUSED__, const struct stat *st)
{
   Enna_Shortcut *es = data;
   Elm_Genlist_Item *egi;

   if (!eio_file_is_dir(st))
     {
        _enna_shortcut_cleanup(es);
        return ;
     }

   egi = evas_object_data_get(es->list, "enna/favorites");

   elm_genlist_item_append(es->list, &itc_desktop_group, es, egi, ELM_GENLIST_ITEM_NONE, NULL, NULL);
}

static void
_desktop_stat_error(void *data, Eio_File *handler __UNUSED__, int error __UNUSED__)
{
   _enna_shortcut_cleanup(data);
}

static Eina_Bool
_desktop_filter_cb(void *data __UNUSED__, Eio_File *handler __UNUSED__, const char *file)
{


   return EINA_TRUE;

   if (!strcmp(file + eina_stringshare_strlen(file) - 8, ".desktop"))
     return EINA_TRUE;
   return EINA_FALSE;
}

static char *
_desktop_flatten_env(const char *file)
{
   Eina_Strbuf *buf;
   int i, length;
   char *steal;

   length = strlen(file);
   buf = eina_strbuf_new();

   for (i = 0; i < length; ++i)
     {
        if (file[i] == '$')
          {
             char *tmp;
             int j = ++i;

             for (; i < length && (isalnum(file[i]) || file[i] == '_'); ++i)
               ;

             if (i > length)
               break ;

             if (i - j <= 0) continue ;

             tmp = alloca(i - j + 1);
             memcpy(tmp, file + j, i - j);
             tmp[i - j] = '\0';

             if (getenv(tmp))
               eina_strbuf_append(buf, getenv(tmp));

             i--;
          }
        else
          {
             eina_strbuf_append_char(buf, file[i]);
          }
     }

   steal = eina_strbuf_string_steal(buf);
   eina_strbuf_free(buf);

   return steal;
}

static void
_desktop_main_cb(void *data, Eio_File *handler __UNUSED__, const char *file)
{
   Enna_Shortcut *es;
   Efreet_Desktop *ef;
   char *replace;

   if (!strcmp("inode/symlink", efreet_mime_type_get(file)))
     {
	ef = NULL;
	replace = strdup(file);
     }
   else
     {

	ef = efreet_desktop_get(file);
	if (!ef) return ;

	if (ef->type != EFREET_DESKTOP_TYPE_LINK)
	  goto end;

	if (!ef->url)
	  goto end;

	if (strncmp(ef->url, "file:", 5))
	  goto end;

	replace = _desktop_flatten_env(ef->url + 5);
     }

   es = malloc(sizeof (Enna_Shortcut));
   if (!es) goto nomem;

   es->ef = ef;
   es->target = replace;
   es->list = data;

   eio_file_direct_stat(replace, _desktop_stat_ok, _desktop_stat_error, es);
   return ;

 nomem:
   free(replace);

 end:
   efreet_desktop_free(ef);
}

static void
_desktop_end_cb(void *data __UNUSED__, Eio_File *handler __UNUSED__)
{
}

static void
_desktop_error_cb(void *data __UNUSED__, Eio_File *handler __UNUSED__, int error __UNUSED__)
{
}

static void
_bookmark_selected_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   char *file = data;
   evas_object_smart_callback_call(obj, "shortcut,selected", file);
}

static void
_volume_selected_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Enna_Volume *v = data;

   if (!v->mounted)
     enna_udisks_volume_mount(v);

   else if (v->mount_point)
     evas_object_smart_callback_call(obj, "shortcut,selected", v->mount_point);
}

static Eina_Bool
_volume_added_cb(void *data, int type, void *event)
{
   Evas_Object *list = data;
   Elm_Genlist_Item *egi, *gi;
   Elm_Genlist_Item *it;
   Enna_Volume *v = event;
   Device_Item *di;

   di = _volume_find(v->udi);
   if (di)
     {
        elm_genlist_item_update(di->gi);
        return ECORE_CALLBACK_DONE;
     }


   egi = evas_object_data_get(list, "devices/item");
   gi = elm_genlist_item_append(list, &itc_bookmark_volume, v, egi, ELM_GENLIST_ITEM_NONE, _volume_selected_cb, v);

   di = calloc(1, sizeof(Device_Item));
   di->gi = gi;
   di->v = v;
   di->list = list;
   di->udi = eina_stringshare_add(v->udi);
   _device_items = eina_list_append(_device_items, di);

   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
_volume_removed_cb(void *data, int type, void *event)
{
   Device_Item *di;
   const char *udi = event;

   di = _volume_find(udi);

   if(!di)
     return ECORE_CALLBACK_DONE;

   elm_genlist_item_del(di->gi);
   _device_items = eina_list_remove(_device_items, di);
   eina_stringshare_del(di->udi);
   free(di);

   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
_storage_added_cb(void *data, int type, void *event)
{
   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
_storage_removed_cb(void *data, int type, void *event)
{
   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
_mount_done_cb(void *data, int type, void *event)
{
   const char *str = event;
   Device_Item *di;

   di = _volume_find(str);
   elm_genlist_item_update(di->gi);
   elm_genlist_item_selected_set(di->gi, EINA_FALSE);

   if (di->v && di->v->mount_point)
     evas_object_smart_callback_call(di->list, "shortcut,selected", di->v->mount_point);

   return ECORE_CALLBACK_DONE;

}

static Eina_Bool
_mount_error_cb(void *data, int type, void *event)
{
   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
_unmount_done_cb(void *data, int type, void *event)
{
   const char *str = event;
   Device_Item *di;

   di = _volume_find(str);
   elm_genlist_item_update(di->gi);
   elm_genlist_item_selected_set(di->gi, EINA_FALSE);
   if (di->v)
     evas_object_smart_callback_call(di->list, "shortcut,selected", getenv("HOME"));

   return ECORE_CALLBACK_DONE;

}

static Eina_Bool
_unmount_error_cb(void *data, int type, void *event)
{
   return ECORE_CALLBACK_DONE;
}


Evas_Object *
enna_shortcut_add(Evas_Object *parent)
{
   Elm_Genlist_Item *egi;
   const char *home = "/root/";
   char buffer[PATH_MAX];

   Eina_List *gtk_bookmarks;
   Eina_List *volumes;
   const char *file;

   list = elm_genlist_add(parent);


   egi = elm_genlist_item_append(list, &itc_favorite_group, "DEVICES", NULL,
                                 ELM_GENLIST_ITEM_SUBITEMS, NULL, NULL);

   evas_object_data_set(list, "devices/item", egi);
   evas_object_data_set(list, "items", egi);

   ecore_event_handler_add(ENNA_EVENT_VOLUMES_ADDED,
                           _volume_added_cb,
                           list);
   ecore_event_handler_add(ENNA_EVENT_VOLUMES_REMOVED,
                           _volume_removed_cb,
                           list);
   ecore_event_handler_add(ENNA_EVENT_STORAGE_ADDED,
                           _storage_added_cb,
                           list);
   ecore_event_handler_add(ENNA_EVENT_STORAGE_REMOVED,
                           _storage_removed_cb,
                           list);

   ecore_event_handler_add(ENNA_EVENT_OP_MOUNT_DONE,
                           _mount_done_cb,
                           list);

   ecore_event_handler_add(ENNA_EVENT_OP_MOUNT_ERROR,
                           _mount_error_cb,
                           list);

   ecore_event_handler_add(ENNA_EVENT_OP_UNMOUNT_DONE,
                           _unmount_done_cb,
                           list);

   ecore_event_handler_add(ENNA_EVENT_OP_UNMOUNT_ERROR,
                           _unmount_error_cb,
                           list);


   egi = elm_genlist_item_append(list, &itc_favorite_group, "NETWORK", NULL,
                                 ELM_GENLIST_ITEM_SUBITEMS, NULL, NULL);

   egi = elm_genlist_item_append(list, &itc_favorite_group, "PLACES", NULL,
                                 ELM_GENLIST_ITEM_SUBITEMS, NULL, NULL);

   gtk_bookmarks = _enna_shortcut_parse_gtk_bookmarks();
   EINA_LIST_FREE(gtk_bookmarks, file)
     {
        elm_genlist_item_append(list, &itc_bookmark, file, egi, ELM_GENLIST_ITEM_NONE, _bookmark_selected_cb, file);
     }

   evas_object_data_set(list, "enna/favorites", egi);

   //evas_object_smart_callback_add(list, "selected", _enna_shortcut_desktop_select, grid);

   if (getenv("HOME"))
     home = getenv("HOME");

   snprintf(buffer, PATH_MAX - 1, "%s/.e/e/fileman/favorites/", home);

   eio_file_ls(buffer,
               _desktop_filter_cb,
               _desktop_main_cb,
               _desktop_end_cb,
               _desktop_error_cb,
               list);

   return list;
}
