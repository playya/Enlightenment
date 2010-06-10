// vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2

#include "main.h"

int APP_LOG_DOMAIN;

const char *media_player = NULL;
Enlil_Data *enlil_data = NULL;
static Tabpanel_Item *tp_list_photo;


static const Ecore_Getopt options = {
     "Enki",
     NULL,
     VERSION,
     "(C) 2009 Photo manager, see AUTHORS.",
     "LGPL with advertisement, see COPYING",
     "A photo manager using the EFL !\n\n",
     1,
     {
	ECORE_GETOPT_VERSION('V', "version"),
	ECORE_GETOPT_COPYRIGHT('R', "copyright"),
	ECORE_GETOPT_LICENSE('L', "license"),
	ECORE_GETOPT_STORE_STR('l', "library", "Specify the location of a library"),
	ECORE_GETOPT_HELP('h', "help"),
	ECORE_GETOPT_SENTINEL
     }
};

void close_cb(void *data, Evas_Object *obj, void *event_info)
{
   slideshow_hide();
   slideshow_clear();

   photos_list_object_freeze(enlil_data->list_photo->o_list, 1);

   download_free(&(enlil_data->dl));
   upload_free(&(enlil_data->ul));
   enlil_root_free(&(enlil_data->root));
   enlil_sync_free(&(enlil_data->sync));
   if(enlil_data->load) enlil_load_free(&(enlil_data->load));

   map_free(enlil_data->map);

   tabpanel_item_del(enlil_data->library_item);
   tabpanel_del(enlil_data->tabpanel);
   free(enlil_data);
   free(enlil_data->list_photo);
   free(enlil_data->list_left);
   free(enlil_data->win);

   EINA_STRINGSHARE_DEL(media_player);

   elm_exit();
}

   static void
_notify_bt_close(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *notify = data;
   evas_object_hide(notify);
}


static void _tabpanel_select_page1_cb(void *data, Tabpanel *tabpanel, Tabpanel_Item *item)
{
   //Enlil_Data *enlil_data = data;
}

static void _photos_list_select_cb(void *data, Tabpanel *tabpanel, Tabpanel_Item *item)
{
   Enlil_Data *enlil_data = data;
   enlil_data->list_left->is_map = EINA_FALSE;
   if(enlil_data->map)
     elm_map_bubbles_close(enlil_data->map->map);
}

static void _map_select_cb(void *data, Tabpanel *tabpanel, Tabpanel_Item *item)
{
   Enlil_Data *enlil_data = data;
   enlil_data->list_left->is_map = EINA_TRUE;
}

static void _menu_select_cb(void *data, Tabpanel *tabpanel, Tabpanel_Item *item)
{
   Enlil_Data *enlil_data = data;
   enlil_data->list_left->is_map = EINA_FALSE;
   if(enlil_data->map)
     elm_map_bubbles_close(enlil_data->map->map);
}

void root_set(const char *root_path)
{
   main_menu_noroot_disabled_set(EINA_FALSE);

   photos_list_object_freeze(enlil_data->list_photo->o_list, 1);
   enlil_thumb_clear();
   slideshow_hide();
   slideshow_clear();

   enlil_flickr_job_start_cb_set(flickr_job_start_cb, NULL);
   enlil_flickr_job_done_cb_set(flickr_job_done_cb, NULL);

   if(enlil_data->sync)
     enlil_sync_free(&enlil_data->sync);
   if(enlil_data->load)
     enlil_load_free(&enlil_data->load);
   if(enlil_data->root)
     {
	enlil_root_free(&enlil_data->root);
     }

   photos_list_object_freeze(enlil_data->list_photo->o_list, 0);

   elm_label_label_set(enlil_data->list_photo->lbl_nb_albums_photos, "");

   list_left_data_set(enlil_data->list_left, enlil_data);

   //
   Enlil_Root *root = enlil_root_new(monitor_album_new_cb, monitor_album_delete_cb, monitor_enlil_delete_cb,
	 monitor_photo_new_cb, monitor_photo_delete_cb, monitor_photo_update_cb,
	 collection_new_cb, collection_delete_cb,
	 collection_album_new_cb, collection_album_delete_cb,
	 tag_new_cb, tag_delete_cb,
	 tag_photo_new_cb, tag_photo_delete_cb,
	 enlil_data);
   enlil_root_path_set(root, root_path);
   enlil_root_eet_path_save(root);
   enlil_data->root = root;
   //

   //
   Enlil_Sync *sync = enlil_sync_new(enlil_root_path_get(root),
	 sync_album_new_cb, sync_album_update_cb, sync_album_disappear_cb,
	 sync_photo_new_cb, sync_photo_update_cb, sync_photo_disappear_cb,
	 sync_done_cb, sync_start_cb, sync_error_cb, enlil_data);
   enlil_root_sync_set(root, sync);
   enlil_data->sync = sync;
   //

   //
   Enlil_Load *load = enlil_load_new(root,
	 load_album_done_cb,
	 load_done_cb, load_error_cb, enlil_data);
   enlil_root_monitor_start(root);
   enlil_data->load = load;

   notify_load_content_set(enlil_data, D_("  Loading ..."), EINA_TRUE);

   photos_list_object_freeze(enlil_data->list_photo->o_list, EINA_TRUE);
   enlil_load_run(load);
   //

   //
   Eina_List *list = enlil_root_eet_path_load();
   Enlil_String *string;
   main_menu_update_libraries_list(list);
   EINA_LIST_FREE(list, string)
     {
	EINA_STRINGSHARE_DEL(string->string);
	FREE(string);
     }
   //

   //the background
   Enlil_String *s;
   Eet_Data_Descriptor *edd;
   char buf[PATH_MAX];
   edd = enlil_string_edd_new();
   snprintf(buf, PATH_MAX, "%s %s", APP_NAME" background", enlil_root_path_get(enlil_data->root));
   s = enlil_eet_app_data_load(edd, buf);
   eet_data_descriptor_free(edd);
   if(s)
     {
	enlil_win_bg_set(enlil_data->win, s->string);
	eina_stringshare_del(s->string);
	FREE(s);
     }
   else
     enlil_win_bg_set(enlil_data->win, NULL);
   //
   main_menu_loading_disable_set(1);
}

int elm_main(int argc, char **argv)
{
   Evas_Object *panels, *tabs, *page_1, *bx;
   Tabpanel_Item *tp_item;
   unsigned char exit_option = 0;
   char *root_path = NULL;

   enlil_init();
   ecore_file_init();

   LOG_DOMAIN = eina_log_domain_register("Enki", "\033[34;1m");

   //ecore_getopt
   Ecore_Getopt_Value values[] = {
	ECORE_GETOPT_VALUE_BOOL(exit_option),
	ECORE_GETOPT_VALUE_BOOL(exit_option),
	ECORE_GETOPT_VALUE_BOOL(exit_option),
	ECORE_GETOPT_VALUE_STR(root_path),
	ECORE_GETOPT_VALUE_BOOL(exit_option),
   };
   ecore_app_args_set(argc, (const char **) argv);
   int nonargs = ecore_getopt_parse(&options, values, argc, argv);
   if (nonargs < 0)
     return 1;
   else if (nonargs != argc)
     {
	fputs("Invalid non-option argument", stderr);
	ecore_getopt_help(stderr, &options);
	return 1;
     }

   if(exit_option)
     return 0;
   //

   elm_finger_size_set(1);

   //
   enlil_data = calloc(1, sizeof(Enlil_Data));
   //

   //
   Enlil_Win *win = enlil_win_new();
   enlil_data->win = win;
   evas_object_smart_callback_add(win->win, "delete-request", close_cb, NULL);

   Evas_Object *tb = elm_table_add(win->win);
   enlil_data->tb = tb;
   evas_object_size_hint_weight_set(tb, 1.0, 1.0);
   evas_object_show(tb);

   bx = elm_box_add(win->win);
   elm_box_horizontal_set(bx, EINA_TRUE);
   evas_object_size_hint_weight_set(bx, 1.0, 0.0);
   evas_object_size_hint_align_set(bx, -1.0, -1.0);
   elm_table_pack(tb, bx, 0, 0, 1, 1);
   evas_object_show(bx);


   enlil_data->tabpanel = tabpanel_add(win->win);

   tabs = tabpanel_tabs_obj_get(enlil_data->tabpanel);
   evas_object_size_hint_weight_set(tabs, 1.0, 0.0);
   evas_object_size_hint_align_set(tabs, -1.0, 0.0);
   evas_object_show(tabs);
   elm_box_pack_end(bx, tabs);


   Evas_Object *flickr = flickr_menu_new(win->win);
   elm_box_pack_end(bx, flickr);


   panels = tabpanel_panels_obj_get(enlil_data->tabpanel);
   evas_object_size_hint_weight_set(panels, 1.0, 1.0);
   evas_object_size_hint_align_set(panels, -1.0, -1.0);
   evas_object_show(panels);
   elm_table_pack(tb, panels, 0, 2, 1, 1);


   page_1 = elm_box_add(win->win);
   elm_box_horizontal_set(page_1, 1);
   evas_object_size_hint_weight_set(page_1, 1.0, 1.0);
   evas_object_size_hint_align_set(page_1, -1.0, -1.0);
   evas_object_show(page_1);
   enlil_data->library_item =
      tabpanel_item_add(enlil_data->tabpanel, D_("Library"), page_1, _tabpanel_select_page1_cb, enlil_data);

   //HACK to have a list with a width>0
   bx = elm_box_add(win->win);
   evas_object_size_hint_weight_set(bx, 0.0, 1.0);
   evas_object_size_hint_align_set(bx, -1.0, -1.0);
   evas_object_show(bx);
   elm_box_pack_end(page_1, bx);

   Evas_Object *rect = evas_object_rectangle_add(evas_object_evas_get(win->win));
   evas_object_size_hint_weight_set(rect, 0, 0);
   evas_object_size_hint_min_set(rect, 200, 0);
   evas_object_size_hint_max_set(rect, 200, 0);
   elm_box_pack_end(bx, rect);

   List_Left *list_album = list_left_new(win->win);
   list_left_data_set(list_album, enlil_data);
   elm_box_pack_end(bx, list_album->bx);
   elm_box_pack_end(page_1, list_album->panels_map);


   Evas_Object *main_menu = main_menu_new(win->win);
   enlil_data->main_menu = main_menu;
   tp_item = tabpanel_item_add(list_album->tb_liste_map, D_("Menu"), main_menu, _menu_select_cb, enlil_data);


   List_Photo *list_photo = list_photo_new(win->win);
   list_photo_data_set(list_photo, enlil_data);
   tp_list_photo = tabpanel_item_add(list_album->tb_liste_map, D_("Liste"), list_photo->bx,
	 _photos_list_select_cb, enlil_data);

   Map *map = map_new(win->win);
   enlil_data->map = map;
   tabpanel_item_add(list_album->tb_liste_map, D_("Map"), map->bx, _map_select_cb, enlil_data);

   tabpanel_item_select(tp_item);
   //

   //
   enlil_data->notify_load = elm_notify_add(win->win);
   elm_win_resize_object_add(win->win, enlil_data->notify_load);
   evas_object_size_hint_weight_set(enlil_data->notify_load, -1.0, -1.0);
   evas_object_size_hint_align_set(enlil_data->notify_load, -1.0, -1.0);
   //

   //
   enlil_data->notify_sync = elm_notify_add(win->win);
   elm_notify_orient_set(enlil_data->notify_sync, ELM_NOTIFY_ORIENT_TOP_RIGHT);
   elm_win_resize_object_add(win->win, enlil_data->notify_sync);
   evas_object_size_hint_weight_set(enlil_data->notify_sync, -1.0, -1.0);
   evas_object_size_hint_align_set(enlil_data->notify_sync, -1.0, -1.0);
   //

   //
   enlil_data->dl = download_new(win->win);
   enlil_data->ul = upload_new(win->win);
   //

   //
   Eina_List *list = enlil_root_eet_path_load();
   Enlil_String *string;
   main_menu_update_libraries_list(list);
   EINA_LIST_FREE(list, string)
     {
	EINA_STRINGSHARE_DEL(string->string);
	FREE(string);
     }
   //

   //the media player
   Enlil_String *s;
   Eet_Data_Descriptor *edd;
   edd = enlil_string_edd_new();
   s = enlil_eet_app_data_load(edd, APP_NAME" media_player");
   eet_data_descriptor_free(edd);
   if(s)
     {
	media_player = s->string;
	FREE(s);
     }
   else
     media_player = eina_stringshare_add("vlc");
   //


   main_menu_noroot_disabled_set(EINA_TRUE);
   if(root_path)
     root_set(root_path);

   elm_win_resize_object_add(win->win, tb);

   evas_object_resize(win->win, 1024, 768);
   evas_object_show(win->win);

   elm_run();

   enlil_file_manager_flush();


   enlil_shutdown();
   eina_log_domain_unregister(LOG_DOMAIN);
   elm_shutdown();

   return 0;
}

void notify_sync_content_set(Enlil_Data *enlil_data, const char *msg, Eina_Bool loading)
{
   Evas_Object *bx, *lbl, *bt, *pb;

   bx = elm_box_add(enlil_data->win->win);
   elm_box_horizontal_set(bx, 1);
   evas_object_show(bx);

   if(loading)
     {
	pb = elm_progressbar_add(enlil_data->win->win);
	elm_object_style_set(pb, "wheel");
	elm_progressbar_label_set(pb, "");
	elm_progressbar_pulse(pb, EINA_TRUE);
	evas_object_size_hint_weight_set(pb, 1.0, 0.0);
	evas_object_size_hint_align_set(pb, -1.0, 0.5);
	evas_object_show(pb);
	elm_box_pack_end(bx, pb);
	elm_notify_timeout_set(enlil_data->notify_sync, -1);
     }
   else
     elm_notify_timeout_set(enlil_data->notify_sync, 2);


   lbl = elm_label_add(enlil_data->win->win);
   elm_label_label_set(lbl, msg);
   elm_box_pack_end(bx, lbl);
   evas_object_show(lbl);

   bt = elm_button_add(enlil_data->win->win);
   elm_button_label_set(bt, D_("Close"));
   elm_box_pack_end(bx, bt);
   evas_object_smart_callback_add(bt, "clicked", _notify_bt_close, enlil_data->notify_sync);
   evas_object_show(bt);

   elm_notify_content_set(enlil_data->notify_sync, bx);
   elm_notify_timer_init(enlil_data->notify_sync);
   evas_object_show(enlil_data->notify_sync);
}

void notify_load_content_set(Enlil_Data *enlil_data, const char *msg, Eina_Bool loading)
{
   Evas_Object *bx, *lbl, *bt, *pb;

   bx = elm_box_add(enlil_data->win->win);
   elm_box_horizontal_set(bx, 1);
   evas_object_show(bx);

   if(loading)
     {
	pb = elm_progressbar_add(enlil_data->win->win);
	elm_object_style_set(pb, "wheel");
	elm_progressbar_label_set(pb, "");
	elm_progressbar_pulse(pb, EINA_TRUE);
	evas_object_size_hint_weight_set(pb, 1.0, 0.0);
	evas_object_size_hint_align_set(pb, -1.0, 0.5);
	evas_object_show(pb);
	elm_box_pack_end(bx, pb);

	elm_notify_timeout_set(enlil_data->notify_load, -1);
     }
   else
     elm_notify_timeout_set(enlil_data->notify_load, 3);

   lbl = elm_label_add(enlil_data->win->win);
   elm_label_label_set(lbl, msg);
   elm_box_pack_end(bx, lbl);
   evas_object_show(lbl);

   bt = elm_button_add(enlil_data->win->win);
   elm_button_label_set(bt, D_("Close"));
   elm_box_pack_end(bx, bt);
   evas_object_smart_callback_add(bt, "clicked", _notify_bt_close, enlil_data->notify_load);
   evas_object_show(bt);

   elm_notify_content_set(enlil_data->notify_load, bx);
   elm_notify_timer_init(enlil_data->notify_load);
   evas_object_show(enlil_data->notify_load);
}

void enlil_album_data_free(Enlil_Album *album, void *_data)
{
   Enlil_Album_Data *data = _data;
   Enlil_Data *enlil_data = data->enlil_data;

   if(data->import_list_album_item)
     elm_genlist_item_del(data->import_list_album_item);
   if(data->photo_move_album_list_album_item)
     elm_genlist_item_del(data->photo_move_album_list_album_item);

   list_left_remove(enlil_data->list_left, album);
   photos_list_object_header_del(data->list_photo_item);

   free(data);
}

void enlil_photo_data_free(Enlil_Photo *photo, void *_data)
{
   Enlil_Photo_Data *data = _data;
   //Enlil_Data *enlil_data = data->enlil_data;

   enlil_thumb_photo_clear(photo);
   photos_list_object_item_del(data->list_photo_item);
   if(data->slideshow_item)
     elm_slideshow_item_del(data->slideshow_item);
   if(data->panel_image)
     panel_image_free(&(data->panel_image));
   if(data->marker)
     map_photo_remove(enlil_data->map, photo);
   if(data->exif_job)
     enlil_exif_job_del(data->exif_job);
   if(data->iptc_job)
     enlil_iptc_job_del(data->iptc_job);

   free(data);
}


void enlil_collection_data_free(Enlil_Collection *col, void *_data)
{
   Enlil_Collection_Data *data = _data;
   elm_genlist_item_del(data->list_col_item);
   data->list_col_item = NULL;
   free(data);
}

void enlil_tag_data_free(Enlil_Tag *tag, void *_data)
{
   Enlil_Tag_Data *data = _data;
   elm_genlist_item_del(data->list_tag_item);
   data->list_tag_item = NULL;
   free(data);
}

void enlil_geocaching_data_free(Enlil_Geocaching *gp, void *_data)
{
   Geocaching_Data *data = _data;

   if(data->panel_geocaching)
     panel_geocaching_free(&(data->panel_geocaching));

   if(data->marker)
     map_geocaching_remove(enlil_data->map, gp);
   free(data);
}

const char *album_flickr_edje_signal_get(Enlil_Album_Data *album_data)
{
   ASSERT_RETURN(album_data != NULL);
   if(album_data->flickr_sync.album_flickr_notuptodate
	 || album_data->flickr_sync.album_notinflickr
	 || album_data->flickr_sync.album_notuptodate
	 || album_data->flickr_sync.photos_notuptodate
	 || album_data->flickr_sync.photos_notinlocal)
     return "update";
   return "uptodate";
}

const char *photo_flickr_edje_signal_get(Photo_Flickr_Enum e)
{
   switch(e)
     {
      case PHOTO_FLICKR_NONE:
	 return "uptodate";
      case PHOTO_FLICKR_NOTUPTODATE:
	 return "update";
      case PHOTO_FLICKR_FLICKRNOTUPTODATE:
	 return "update";
      case PHOTO_FLICKR_NOTINFLICKR:
	 return "update";
     }
   return NULL;
}

void select_list_photo()
{
   tabpanel_item_select(tp_list_photo);
}

ELM_MAIN()

