
#include "main.h"

static Evas_Object *inwin = NULL;

static Evas_Object *libraries_list;
static Evas_Object *bt_new_library;
static Evas_Object *bt_import;
static Evas_Object *bt_slideshow;
static Evas_Object *bt_del_bg;
static Evas_Object *bt_album_new;

static void _library_select(void *data, Evas_Object *obj, void *event_info);
static void _new_library_cb(void *data, Evas_Object *obj, void *event_info);
static void _new_library_done_cb(void *data, Evas_Object *obj, void *event_info);

static void _slideshow_cb(void *data, Evas_Object *obj, void *event_info);
static void _album_new_cb(void *data, Evas_Object *obj, void *event_info);
static void _import_cb(void *data, Evas_Object *obj, void *event_info);
static void _del_bg_cb(void *data, Evas_Object *obj, void *event_info);
static void _geocaching_import_cb(void *data, Evas_Object *obj, void *event_info);
static void _geocaching_import_done_cb(void *data, Evas_Object *obj, void *event_info);
static void _preferences_cb(void *data, Evas_Object *obj, void *event_info);
static void _quit_cb(void *data, Evas_Object *obj, void *event_info);


Evas_Object *main_menu_new(Evas_Object *parent)
{
    Evas_Object *ly, *bt, *edje, *ebx, *list, *ic;

    ly = elm_layout_add(parent);
    elm_layout_file_set(ly, PACKAGE_DATA_DIR"/theme.edj", "main_menu");
    evas_object_size_hint_weight_set(ly, -1.0, -1.0);
    evas_object_size_hint_align_set(ly, -1.0, -1.0);
    evas_object_show(ly);

    edje = elm_layout_edje_get(ly);

    //Libraries
    ebx = (Evas_Object*)edje_object_part_object_get(edje, "library");

    list = elm_list_add(parent);
    libraries_list = list;
    evas_object_size_hint_weight_set(list, -1.0, 1.0);
    evas_object_size_hint_align_set(list, -1.0, -1.0);
    evas_object_show(list);
    evas_object_box_append(ebx, list);

    bt = elm_button_add(parent);
    bt_new_library = bt;
    evas_object_size_hint_weight_set(bt, -1.0, 0.0);
    evas_object_size_hint_align_set(bt, -1.0, 0.0);
    evas_object_show(bt);
    elm_button_label_set(bt, D_("New Library"));
    evas_object_box_append(ebx, bt);
    evas_object_smart_callback_add(bt, "clicked", _new_library_cb, NULL);
    //

    //tools
    ebx = (Evas_Object*)edje_object_part_object_get(edje, "tools");

    bt = elm_button_add(parent);
    bt_slideshow = bt;
    evas_object_size_hint_weight_set(bt, -1.0, 0.0);
    evas_object_size_hint_align_set(bt, -1.0, 0.0);
    evas_object_show(bt);
    elm_button_label_set(bt, D_("Slideshow"));
    evas_object_box_append(ebx, bt);
    evas_object_smart_callback_add(bt, "clicked", _slideshow_cb, NULL);

    ic = elm_icon_add(parent);
    elm_icon_standard_set(ic, "edit");
    elm_icon_fill_outside_set(ic, EINA_TRUE);
    evas_object_show(ic);
    elm_button_icon_set(bt, ic);


    bt = elm_separator_add(parent);
    evas_object_size_hint_weight_set(bt, -1.0, 0.0);
    evas_object_size_hint_align_set(bt, -1.0, 0.0);
    evas_object_show(bt);
    evas_object_box_append(ebx, bt);

    bt = elm_button_add(parent);
    bt_album_new = bt;
    evas_object_size_hint_weight_set(bt, -1.0, 0.0);
    evas_object_size_hint_align_set(bt, -1.0, 0.0);
    evas_object_show(bt);
    elm_button_label_set(bt, D_("New Album"));
    evas_object_box_append(ebx, bt);
    evas_object_smart_callback_add(bt, "clicked", _album_new_cb, NULL);

    ic = elm_icon_add(parent);
    elm_icon_standard_set(ic, "edit");
    elm_icon_fill_outside_set(ic, EINA_TRUE);
    evas_object_show(ic);
    elm_button_icon_set(bt, ic);


    bt = elm_button_add(parent);
    bt_import = bt;
    evas_object_size_hint_weight_set(bt, -1.0, 0.0);
    evas_object_size_hint_align_set(bt, -1.0, 0.0);
    evas_object_show(bt);
    elm_button_label_set(bt, D_("Import Photos"));
    evas_object_box_append(ebx, bt);
    evas_object_smart_callback_add(bt, "clicked", _import_cb, NULL);

    ic = elm_icon_add(parent);
    elm_icon_standard_set(ic, "edit");
    elm_icon_fill_outside_set(ic, EINA_TRUE);
    evas_object_show(ic);
    elm_button_icon_set(bt, ic);


    bt = elm_button_add(parent);
    bt_del_bg = bt;
    evas_object_size_hint_weight_set(bt, -1.0, 0.0);
    evas_object_size_hint_align_set(bt, -1.0, 0.0);
    evas_object_show(bt);
    elm_button_label_set(bt, D_("Remove The Wallpaper"));
    evas_object_box_append(ebx, bt);
    evas_object_smart_callback_add(bt, "clicked", _del_bg_cb, NULL);

    ic = elm_icon_add(parent);
    elm_icon_standard_set(ic, "delete");
    elm_icon_fill_outside_set(ic, EINA_TRUE);
    evas_object_show(ic);
    elm_button_icon_set(bt, ic);

    bt = elm_separator_add(parent);
    evas_object_size_hint_weight_set(bt, -1.0, 0.0);
    evas_object_size_hint_align_set(bt, -1.0, 0.0);
    evas_object_show(bt);
    evas_object_box_append(ebx, bt);

    bt = elm_button_add(parent);
    evas_object_size_hint_weight_set(bt, -1.0, 0.0);
    evas_object_size_hint_align_set(bt, -1.0, 0.0);
    evas_object_show(bt);
    elm_button_label_set(bt, D_("Import a GPX File"));
    evas_object_box_append(ebx, bt);
    evas_object_smart_callback_add(bt, "clicked", _geocaching_import_cb, NULL);

    ic = elm_icon_add(parent);
    elm_icon_standard_set(ic, "edit");
    elm_icon_fill_outside_set(ic, EINA_TRUE);
    evas_object_show(ic);
    elm_button_icon_set(bt, ic);


    bt = elm_separator_add(parent);
    evas_object_size_hint_weight_set(bt, -1.0, 0.0);
    evas_object_size_hint_align_set(bt, -1.0, 0.0);
    evas_object_show(bt);
    evas_object_box_append(ebx, bt);

    bt = elm_button_add(parent);
    evas_object_size_hint_weight_set(bt, -1.0, 0.0);
    evas_object_size_hint_align_set(bt, -1.0, 0.0);
    evas_object_show(bt);
    elm_button_label_set(bt, D_("Preference"));
    evas_object_box_append(ebx, bt);
    evas_object_smart_callback_add(bt, "clicked", _preferences_cb, NULL);

    ic = elm_icon_add(parent);
    elm_icon_standard_set(ic, "edit");
    elm_icon_fill_outside_set(ic, EINA_TRUE);
    evas_object_show(ic);
    elm_button_icon_set(bt, ic);
    //

    //Quit
    ebx = (Evas_Object*)edje_object_part_object_get(edje, "quit");

    bt = elm_button_add(parent);
    evas_object_size_hint_weight_set(bt, -1.0, 0.0);
    evas_object_size_hint_align_set(bt, -1.0, 1.0);
    evas_object_show(bt);
    elm_button_label_set(bt, D_("Close Enki"));
    evas_object_box_append(ebx, bt);
    evas_object_smart_callback_add(bt, "clicked", _quit_cb, NULL);

    ic = elm_icon_add(parent);
    elm_icon_standard_set(ic, "close");
    elm_icon_fill_outside_set(ic, EINA_TRUE);
    evas_object_show(ic);
    elm_button_icon_set(bt, ic);
    //


    //library list
    Eina_List *l = enlil_root_eet_path_load();
    Enlil_String *string;
    main_menu_update_libraries_list(l);
    EINA_LIST_FREE(l, string)
    {
        EINA_STRINGSHARE_DEL(string->string);
        FREE(string);
    }

    return ly;
}

void main_menu_loading_disable_set(Eina_Bool disabled)
{
   elm_object_disabled_set(bt_new_library, disabled);
   elm_object_disabled_set(libraries_list, disabled);
   elm_object_disabled_set(bt_import, disabled);
   elm_object_disabled_set(bt_album_new, disabled);
   elm_object_disabled_set(bt_slideshow, disabled);
}

void main_menu_sync_disable_set(Eina_Bool disabled)
{
   elm_object_disabled_set(bt_new_library, disabled);
   elm_object_disabled_set(libraries_list, disabled);
}

void main_menu_noroot_disabled_set(Eina_Bool disabled)
{
   main_menu_loading_disable_set(disabled);
   elm_object_disabled_set(bt_new_library, EINA_FALSE);
   elm_object_disabled_set(libraries_list, EINA_FALSE);
   elm_object_disabled_set(bt_del_bg, disabled);
}

void main_menu_update_libraries_list(Eina_List *list)
{
    const Eina_List *l;
    Enlil_String *string;

    elm_list_clear(libraries_list);
    EINA_LIST_FOREACH(list, l, string)
        elm_list_item_append(libraries_list, string->string, NULL, NULL, _library_select, NULL);
    elm_list_go(libraries_list);
}

static void _library_select(void *data, Evas_Object *obj, void *event_info)
{
    root_set(elm_list_item_label_get(elm_list_selected_item_get(obj)));
    select_list_photo();
}

static void _new_library_cb(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *fs, *vbox;

   //create inwin & file selector
   inwin = elm_win_inwin_add(enlil_data->win->win);
   evas_object_show(inwin);

   vbox = elm_box_add(inwin);
   evas_object_size_hint_weight_set(vbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(vbox);
   elm_win_inwin_content_set(inwin, vbox);

   fs = elm_fileselector_add(inwin);
   elm_fileselector_folder_only_set(fs, 1);
   elm_fileselector_is_save_set(fs, 1);
   elm_fileselector_expandable_set(fs, EINA_FALSE);
   elm_fileselector_path_set(fs,  getenv("HOME"));
   evas_object_size_hint_weight_set(fs, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(fs, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(vbox, fs);
   evas_object_show(fs);

   evas_object_smart_callback_add(fs, "done", _new_library_done_cb, NULL);
}

static void _new_library_done_cb(void *data, Evas_Object *obj, void *event_info)
{
   const char *selected = event_info;

   if (selected)
     {
	if(!ecore_file_exists(selected))
	  ecore_file_mkdir(selected);
	if(ecore_file_is_dir(selected))
        {
            root_set(selected);
            select_list_photo();
        }
     }
   evas_object_del(inwin);
}

static void _slideshow_cb(void *data, Evas_Object *obj, void *event_info)
{
   Enlil_Photo *photo = NULL;
   PL_Child_Item *item = NULL;

   Eina_List *items = photos_list_object_selected_get(enlil_data->list_photo->o_list);

   if(items)
     {
	item = eina_list_data_get(items);
	photo = photos_list_object_item_data_get(item);
     }

   slideshow_clear();
   slideshow_root_add(enlil_data->root, photo);
   slideshow_show();
}

static void _album_new_cb(void *data, Evas_Object *obj, void *event_info)
{
   inwin_album_new_new(NULL, NULL);
}

static void _import_cb(void *data, Evas_Object *obj, void *event_info)
{
   import_new(enlil_data->win->win);
}

static void _del_bg_cb(void *data, Evas_Object *obj, void *event_info)
{
   char buf[PATH_MAX];
   Enlil_String s;
   Eet_Data_Descriptor *edd;

   enlil_win_bg_set(enlil_data->win, NULL);

   s.string = NULL;
   edd = enlil_string_edd_new();

   snprintf(buf, PATH_MAX, "%s %s", APP_NAME" background", enlil_root_path_get(enlil_data->root));
   enlil_eet_app_data_save(edd, buf, &s);
   eet_data_descriptor_free(edd);
}


static void _geocaching_import_cb(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *fs, *vbox;

   inwin = elm_win_inwin_add(enlil_data->win->win);
   evas_object_show(inwin);

   vbox = elm_box_add(inwin);
   evas_object_size_hint_weight_set(vbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(vbox);
   elm_win_inwin_content_set(inwin, vbox);

   fs = elm_fileselector_add(inwin);
   elm_fileselector_expandable_set(fs, EINA_FALSE);
   elm_fileselector_path_set(fs,  getenv("HOME"));
   evas_object_size_hint_weight_set(fs, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(fs, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(vbox, fs);
   evas_object_show(fs);

   evas_object_smart_callback_add(fs, "done", _geocaching_import_done_cb, NULL);
}

static void _geocaching_import_done_cb(void *data, Evas_Object *obj, void *event_info)
{
   const char *selected = event_info;

   if (selected)
     {
	char *ext = strchr(selected, '.');
	if(ext && !strcmp(ext, ".gpx"))
	  {
	     enlil_geocaching_import(selected, geocaching_done_cb, NULL);
	  }
     }
   evas_object_del(inwin);
}


static void _preferences_cb(void *data, Evas_Object *obj, void *event_info)
{
	inwin_preferences_new();
}

static void _quit_cb(void *data, Evas_Object *obj, void *event_info)
{
   close_cb(NULL, NULL, NULL);
}


