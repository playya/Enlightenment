/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

// Da usare per importare l'immagine : e_int_config_wallpaper_import(NULL);

#include "e.h"
#include "e_mod_main.h"
#define D(x)  do {printf("### DBG line %d ### ", __LINE__); printf x; fflush(stdout);} while (0)

typedef struct _Import Import;

struct _Import {
   E_Config_Dialog *parent;
   E_Config_Dialog_Data *cfdata;
   E_Dialog *dia;
};

struct _E_Config_Dialog_Data 
{
   Evas_Object *ofm;
   Evas_Object *o;
   Evas_Object *osfm;
   Evas_Object *ol;
   Ecore_List *thumbs;
   Ecore_List *medias;
   Ecore_Con_Url *ecu;
   Ecore_Event_Handler *hdata;
   Ecore_Event_Handler *hcomplete;
   FILE *feed;
   int ready_for_edj;
   int pending_downloads;
   int busy;
   char *edj;
   char *ol_val;
   char *tmpdir;
   const char *source;
};

char tmpdir_tpl[17] = "/tmp/wallpXXXXXX";

static void _file_double_click_cb (void *data, Evas_Object *obj, void *ev_info);
static void _file_click_cb (void *data, Evas_Object *obj, void *ev_info);
static int  _feed_complete (void *data, int type, void *event);
static int  _feed_data (void *data, int type, void *event);
static void _get_feed (char *url, void *data);
static void _parse_feed (void *data);
static void _get_thumbs (void *data);
void        _get_thumb_complete (void *data, const char *file, int status);
static void _source_sel_cb (void *data);
static void _reset (void *data);
static int  _list_find (const char *str1, const char *str2);
static void _dia_del_cb (void *obj);
static void _dia_close_cb (void *data, E_Dialog *dia);
static void _dia_ok_cb (void *data, E_Dialog *dia);
static void _download_media (Import *import);
int         _download_media_progress_cb (void *data, const char *file, long int dltotal, long int dlnow, long int ultotal, long int ulnow);
void        _download_media_complete_cb (void *data, const char *file, int status);

EAPI E_Dialog *
e_int_config_wallpaper_web (E_Config_Dialog *parent)
{
   Evas *evas;
   E_Dialog *dia;
   Import *import;
   E_Config_Dialog_Data *cfdata;

   Evas_Object *o, *ol, *of, *ofm, *osfm, *otl;
   Evas_Coord mw, mh;
   E_Fm2_Config fmc;

   import = E_NEW (Import, 1);
   if (!import) 
      return NULL;

   dia = e_dialog_new (parent->con, "E", "_wallpaper_web_dialog");
   if (!dia) 
   {
      E_FREE (import);
      return NULL;
   }

   dia->data = import;
   e_object_del_attach_func_set (E_OBJECT(dia), _dia_del_cb);
   e_win_centered_set (dia->win, 1);

   evas = e_win_evas_get (dia->win);

   cfdata = E_NEW (E_Config_Dialog_Data, 1);

   ecore_con_url_init ();
   cfdata->ecu = ecore_con_url_new ("http://fake.url");

   cfdata->ready_for_edj = 0;
   cfdata->pending_downloads = 0;
   cfdata->busy = 0;
   import->cfdata = cfdata;
   import->dia = dia;

   import->parent = parent;

   e_dialog_title_set (dia, _("Choose a website from list..."));

   o = e_widget_list_add (evas, 0, 1);
   cfdata->o = o;
   cfdata->thumbs = ecore_list_new ();
   cfdata->medias = ecore_list_new ();
   of = e_widget_framelist_add (evas, "Sources", 1);
   ol = e_widget_ilist_add (evas, 24, 24, &cfdata->ol_val);
   cfdata->ol = ol;
   e_widget_ilist_append (ol, NULL, _("get-e.org - Static"),
                         _source_sel_cb, import,
                         "http://www.get-e.org/Backgrounds/Static/feed.xml");
   e_widget_ilist_append (ol, NULL, _("get-e.org  - Animated"),
                         _source_sel_cb, import,
                         "http://www.get-e.org/Backgrounds/Animated/feed.xml");
   /*e_widget_ilist_append (ol, NULL, _("get-e.org - Local copy"),
                         _source_sel_cb, import,
                         "http://localhost/get_e_feed.xml");*/
   /*e_widget_ilist_append (ol, NULL, "Flickr test",
                         _source_sel_cb, import,
                         "http://api.flickr.com/services/feeds/photos_public.gne?tags=birro&lang=it-it&format=rss_200_enc");*/
   e_widget_ilist_go (ol);


   e_widget_framelist_object_append (of, ol);
   e_widget_list_object_append (o, of, 1, 1, 0.5);

   ofm = e_fm2_add (evas);
   memset (&fmc, 0, sizeof (E_Fm2_Config));
   cfdata->ofm = ofm;
   fmc.view.mode = E_FM2_VIEW_MODE_GRID_ICONS;
   fmc.view.open_dirs_in_place = 1;
   fmc.view.selector = 1;
   fmc.view.single_click = 0;
   fmc.view.no_subdir_jump = 0;
   fmc.icon.list.w = 48;
   fmc.icon.list.h = 48;
   fmc.icon.icon.w = 96;
   fmc.icon.icon.h = 96;
   fmc.icon.fixed.w = 0;
   fmc.icon.fixed.h = 0;
   fmc.icon.extension.show = 0;
   fmc.icon.key_hint = NULL;
   fmc.list.sort.no_case = 1;
   fmc.list.sort.dirs.first = 0;
   fmc.list.sort.dirs.last = 1;
   fmc.selection.single = 1;
   fmc.selection.windows_modifiers = 0;
   e_fm2_config_set (ofm, &fmc);
   e_fm2_icon_menu_flags_set (ofm, E_FM2_MENU_NO_SHOW_HIDDEN);

   evas_object_smart_callback_add (ofm, 
                                   "selected",
                                   _file_double_click_cb,
                                   import);
   evas_object_smart_callback_add (ofm, 
                                   "selection_change",
                                   _file_click_cb,
                                   import);

   osfm = e_widget_scrollframe_pan_add (evas, ofm,
                                     e_fm2_pan_set,
                                     e_fm2_pan_get,
                                     e_fm2_pan_max_get,
                                     e_fm2_pan_child_size_get);
   cfdata->osfm = osfm;
   e_widget_list_object_append (cfdata->o, cfdata->osfm, 1, 1, 0.5);
   e_widget_min_size_set (osfm, 320, 320);

   e_widget_min_size_set (o, 580, 370);
   e_widget_min_size_get (o, &mw, &mh);
   e_dialog_content_set (dia, o, mw, mh);

   e_dialog_button_add (dia, _("OK"), NULL, _dia_ok_cb, import);
   e_dialog_button_add (dia, _("Cancel"), NULL, _dia_close_cb, import);
   e_dialog_button_disable_num_set (dia, 0, 1);

   e_dialog_resizable_set (dia, 1);
   e_dialog_show (dia);

   e_dialog_border_icon_set (dia, "enlightenment/background");

   return dia;
}

void
e_int_config_wallpaper_web_del (E_Dialog *dia)
{
   Import *import;
   E_Config_Dialog_Data *cfdata;

   import = dia->data;
   cfdata = import->cfdata;

   if (cfdata->pending_downloads == 1)
   {
      ecore_file_download_abort_all ();
   }

   if (cfdata->hdata)
   {
      ecore_event_handler_del (cfdata->hdata);
   }
   if (cfdata->hcomplete)
   {
      ecore_event_handler_del (cfdata->hcomplete);
   }
   ecore_con_url_shutdown ();

   if (cfdata->tmpdir)
   {
      if (ecore_file_is_dir (cfdata->tmpdir))
      {
         ecore_file_recursive_rm (cfdata->tmpdir);
         ecore_file_rmdir (cfdata->tmpdir);
      }
   }

   e_int_config_wallpaper_web_done (import->parent);
   E_FREE (import->cfdata);
   E_FREE (import);
   e_object_unref (E_OBJECT(dia));
}

static int 
_feed_complete (void *data, int type, void *event)
{
   Ecore_Con_Event_Url_Complete *euc;
   E_Config_Dialog_Data *cfdata;
   Import *import;
   char *title;

   euc = (Ecore_Con_Event_Url_Complete *)event;
   import = data;
   cfdata = import->cfdata;
   fclose (cfdata->feed);
   ecore_event_handler_del (cfdata->hdata);
   ecore_event_handler_del (cfdata->hcomplete);
   cfdata->hdata = NULL;
   cfdata->hcomplete = NULL;
   if (euc->status == 200) {
      asprintf(&title, _("[%s] Getting feed... DONE!"), cfdata->source);
      e_dialog_title_set (import->dia, title);
      _parse_feed (data);
      return 0;
   }
   else
   {
      asprintf(&title, _("[%s] Getting feed... FAILED!"), cfdata->source);
      e_dialog_title_set (import->dia, title);
   }
   return 0;
}

static int
_feed_data (void *data, int type, void *event)
{
   Ecore_Con_Event_Url_Data *eud;
   E_Config_Dialog_Data *cfdata;
   Import *import;

   eud = (Ecore_Con_Event_Url_Data *)event;
   import = data;
   cfdata = import->cfdata;
   fwrite (eud->data, sizeof(unsigned char), eud->size, cfdata->feed);
   return 0;
}

static void
_source_sel_cb (void *data)
{
   Import *import;
   E_Config_Dialog_Data *cfdata;

   import = data;
   cfdata = import->cfdata;
   if ((cfdata->busy == 0) && (cfdata->pending_downloads == 0))
   {
      cfdata->source = e_widget_ilist_selected_label_get (cfdata->ol);
      cfdata->busy = 1;
      _reset (import);
      _get_feed (cfdata->ol_val, import);
   }
   else 
   {
      e_widget_ilist_unselect(cfdata->ol);
   }
}

static void
_parse_feed (void *data)
{
   Import *import;
   E_Config_Dialog_Data *cfdata;
   FILE *fh;
   char instr[255];
   char *edj;
   char *img;
   char *tmpstr;
   char *tinstr;
   char *timg;
   char *title;

   int state = -1;

   import = data;
   cfdata = import->cfdata;

   cfdata->pending_downloads = 0;
   fh = fopen ("/tmp/feed.xml", "r");
   while (fgets (instr, 255, fh) != NULL)
   {
      if (strstr (instr, "<rss version") != NULL)
      {
         state = 0;
      }

      if ((strstr (instr, "<item>") != NULL) && (state == 0))
      {
         edj = NULL;
         img = NULL;
         state = 1;
      }

      if ((strstr (instr, "<link>") !=  NULL) && (state == 1))
      {
         tinstr = strdup (instr); 
         edj = strtok (tinstr, ">");
         edj = strtok (NULL, "<");
         tmpstr = strrchr (ecore_file_file_get (edj), '.');
         if (strstr (tmpstr, "edj") != NULL)
            state = 2;
      }

      if ((strstr (instr, "<enclosure") != NULL) && (state == 2))
      {
         tinstr = strdup (instr); 
         img = strtok (tinstr, "\"");
         img = strtok (NULL, "\"");
         strcat (img, "\n");
         state = 3;
      }

      if ((strstr (instr, "</item>") != NULL) && (state == 3))
      {
         timg = strdup (img);
         timg[strlen(timg) - 1] = 0;
         ecore_list_append (cfdata->thumbs, strdup (timg));
         ecore_list_append (cfdata->medias, strdup (edj));
         state = 0;
      }
   }

   if (timg)
      free (timg);
   if (tinstr)
      free (tinstr);
   fclose(fh);

   if ((state != -1) && (state == 0))
   {
      asprintf(&title, _("[%s] Parsing feed... DONE!"), cfdata->source);
      e_dialog_title_set (import->dia, title);
      e_fm2_path_set (cfdata->ofm, cfdata->tmpdir, "/");
      _get_thumbs (import);
   } 
   else
   {
      asprintf(&title, _("[%s] Parsing feed... FAILED!"), cfdata->source);
      cfdata->busy = 0;
      e_dialog_title_set (import->dia, title);
   }
}

static void
_get_thumbs (void *data)
{
   Import *import;
   E_Config_Dialog_Data *cfdata;
   char *src;
   char *dest;
   char *dtmp;

   import = data;
   cfdata = import->cfdata;
   cfdata->pending_downloads = 1;
   asprintf(&dtmp, "%s/.tmp", cfdata->tmpdir);
   ecore_file_mkdir(dtmp);
   ecore_list_first_goto(cfdata->thumbs);
   while (src = ecore_list_next (cfdata->thumbs))
   {
      asprintf(&dest, "%s/%s", dtmp, ecore_file_file_get(src));
      ecore_file_download (src,
                           dest,
                           _get_thumb_complete,
                           NULL,
                           import);
   }
}

static void
_dia_del_cb (void *obj)
{
   E_Dialog *dia = obj;

   e_int_config_wallpaper_web_del (dia);
}

static void 
_file_double_click_cb (void *data, Evas_Object *obj, void *ev_info)
{
   /*E_Config_Dialog_Data *cfdata;
   Evas_List *sels;
   E_Fm2_Icon_Info *icon_info;

   cfdata = data;
   sels = e_fm2_selected_list_get(cfdata->ofm);
   if (!sels)
      return;
   icon_info = sels->data;
   printf("[double click] %s\n", icon_info->file);*/
   
   // Unused atm, interesting to simulate click on Ok button
}

static void 
_file_click_cb (void *data, Evas_Object *obj, void *ev_info)
{
   Import *import;
   E_Config_Dialog_Data *cfdata;
   Evas_List *sels;
   E_Fm2_Icon_Info *icon_info;

   import = data;
   cfdata = import->cfdata;
   sels = e_fm2_selected_list_get (cfdata->ofm);
   if (!sels)
      return;
   if (cfdata->ready_for_edj == 0)
      return;
   icon_info = sels->data;
   if (ecore_list_find (cfdata->thumbs, ECORE_COMPARE_CB (_list_find), icon_info->file))
      cfdata->edj = ecore_list_index_goto(cfdata->medias, ecore_list_index(cfdata->thumbs));
}

static int
_list_find (const char *str1, const char *str2)
{
   char *tmp;

   tmp = strdup (str1);
   return strcmp(ecore_file_file_get (tmp), str2);
}

static void 
_dia_close_cb (void *data, E_Dialog *dia) 
{
   e_int_config_wallpaper_web_del (dia);
}

static void 
_dia_ok_cb (void *data, E_Dialog *dia)
{
   Import *import;
   E_Config_Dialog_Data *cfdata;
   Evas_List *sels;

   import = data;
   cfdata = import->cfdata;
   sels = e_fm2_selected_list_get (cfdata->ofm);
   if (sels)
   {
      _download_media (import);
      return;
   }

   e_int_config_wallpaper_web_del (dia);
}

static void 
_download_media (Import *import)
{
   Import *i;
   E_Config_Dialog_Data *cfdata;
   int num = 0;
   const char *file;
   const char *homedir;
   char *buf;
   char *title;

   i = import;
   cfdata = i->cfdata;
   
   cfdata->pending_downloads = 1;
   file = ecore_file_file_get (cfdata->edj);
   homedir = e_user_homedir_get ();
   asprintf(&buf, "%s/.e/e/backgrounds/%s", homedir, file);
   asprintf(&title, _("[%s] Downloading of edje file..."), cfdata->source);
   e_dialog_title_set (i->dia, title);
   ecore_file_download (cfdata->edj, buf,
                       _download_media_complete_cb,
                       _download_media_progress_cb,
                       i);
}

void 
_download_media_complete_cb (void *data, const char *file, int status)
{
   Import *import;
   char *dest;

   import = data;
   import->cfdata->pending_downloads = 0;
   asprintf(&dest, "%s/.e/e/backgrounds/%s",
            e_user_homedir_get (),
            ecore_file_file_get (import->cfdata->edj));
   e_int_config_wallpaper_update (import->parent, dest);
   e_int_config_wallpaper_web_del (import->dia);
}

void
_get_thumb_complete (void *data, const char *file, int status)
{
   Import *import;
   E_Config_Dialog_Data *cfdata;
   char *title;
   static int got = 1;
   char *dst;

   import = data;
   cfdata = import->cfdata;
   if (got != ecore_list_count (cfdata->thumbs))
   {
      asprintf (&title,
                _("[%s] Download %d images of %d"),
                cfdata->source,
                got,
                ecore_list_index (cfdata->thumbs));
      e_dialog_title_set (import->dia, title);
      cfdata->ready_for_edj = 0;
      asprintf(&dst, "%s/%s", cfdata->tmpdir, ecore_file_file_get (file));
      ecore_file_mv (file, dst);
      got++;
   }
   else
   {
      got = 1;
      cfdata->busy = 0;
      cfdata->ready_for_edj = 1;
      asprintf(&title, _("[%s] Choose an image from list"), cfdata->source);
      e_dialog_title_set (import->dia, title);
      e_dialog_button_disable_num_set (import->dia, 0, 0);
      cfdata->pending_downloads = 0;
   }
}

int
_download_media_progress_cb (void *data, const char *file, long int dltotal, long int dlnow, long int ultotal, long int ulnow)
{
   Import *import;
   double status;
   char *title;
   static long int last;

   import = data;

   if (dlnow == 0 || dltotal == 0)
      return 0;

   if (last)
   {
      status = (double) ((double) dlnow) / ((double) dltotal);
      asprintf (&title,
                _("[%s] Downloading of edje file... %d%% done"),
                import->cfdata->source,
                (int) (status * 100.0));
      e_dialog_title_set (import->dia, title);
   }

   last = dlnow;

   return 0;
}

static void
_get_feed (char *url, void *data)
{
   Import *import;
   E_Config_Dialog_Data *cfdata;
   extern int errno;
   char *title;

   import = data;
   cfdata = import->cfdata;

   cfdata->tmpdir = mkdtemp (strdup (tmpdir_tpl));

   ecore_con_url_url_set (cfdata->ecu, url);

   cfdata->hdata = ecore_event_handler_add (ECORE_CON_EVENT_URL_DATA,
                                            _feed_data, 
                                            import);
   cfdata->hcomplete = ecore_event_handler_add (ECORE_CON_EVENT_URL_COMPLETE,
                                                _feed_complete, 
                                                import);

   asprintf(&title, _("[%s] Getting feed..."), cfdata->source);
   e_dialog_title_set (import->dia, title); //
   cfdata->feed = fopen ("/tmp/feed.xml", "w+");
   ecore_con_url_send (cfdata->ecu, NULL, 0, NULL);
}

static void
_reset (void *data)
{
   Import *import;
   E_Config_Dialog_Data *cfdata;

   import = data;
   cfdata = import->cfdata;

   // If there's pending downloads, stop it
   if (cfdata->pending_downloads == 1)
   {
      ecore_file_download_abort_all ();
      //ecore_file_download_shutdown ();
   }
   cfdata->pending_downloads = 0;

   // Reset busy state
   cfdata->busy = 0;

   // Clean lists
   if (!ecore_list_empty_is (cfdata->thumbs))
      ecore_list_clear (cfdata->thumbs);
   if (!ecore_list_empty_is (cfdata->medias))
      ecore_list_clear (cfdata->medias);

   // Clean existing data
   if (ecore_file_exists ("/tmp/feed.xml"))
      ecore_file_unlink ("/tmp/feed.xml");

   // Remove temporary data
   if (cfdata->tmpdir)
   {
      if (ecore_file_is_dir (cfdata->tmpdir))
      {
         ecore_file_recursive_rm (cfdata->tmpdir);
         ecore_file_rmdir (cfdata->tmpdir);
      }
   }

   // Disable OK button
   e_dialog_button_disable_num_set (import->dia, 0, 1);
}
