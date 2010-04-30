/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "Evry.h"
#include "e_mod_main.h"

#define IMPORT_STRETCH 0
#define IMPORT_TILE 1
#define IMPORT_CENTER 2
#define IMPORT_SCALE_ASPECT_IN 3
#define IMPORT_SCALE_ASPECT_OUT 4

typedef struct _Import Import;

struct _Import
{
  const char *file;
  int method;
  int external;
  int quality;

  Ecore_Exe *exe;
  Ecore_Event_Handler *exe_handler;
  char *tmpf;
  char *fdest;
};


static void _import_edj_gen(Import *import);
static int _import_cb_edje_cc_exit(void *data, int type, void *event);
static Import *import = NULL;

static Evry_Action *_act;

static char _module_icon[] = "preferences-desktop-wallpaper";

static int
_action(Evry_Action *act)
{
   if (!CHECK_TYPE(act->it1.item, EVRY_TYPE_FILE))
     return 0;

   GET_FILE(file, act->it1.item);

   if (!(evry_file_path_get(file)))
     return 0;
   
   if (import)
     {
	if (import->exe_handler)
	  ecore_event_handler_del(import->exe_handler);
	E_FREE(import);
     }

   import = E_NEW(Import, 1);
   import->method = EVRY_ITEM_DATA_INT_GET(act);
   import->file = file->path;
   import->quality = 100;
   import->external = 0;
   _import_edj_gen(import);

   return 1;
}

static int
_check(Evry_Action *act, const Evry_Item *it)
{
   GET_FILE(file, it);

   if (file->mime && (!strncmp(file->mime, "image/", 6)))
     return 1;

   return 0;
}

static void
_item_add(Evry_Item *it, const char *name, int method, const char *icon)
{
   Evry_Action *act;
   act = EVRY_ACTION_NEW(name, EVRY_TYPE_FILE, 0, icon, _action, NULL);
   EVRY_ITEM_DATA_INT_SET(act, method);

   it->items = eina_list_append(it->items, act);
}

static Eina_List *
_fetch(Evry_Action *act)
{
   Evry_Item *it = (Evry_Item *) act;

   it->items = NULL;

   _item_add(it, _("Stretch"), IMPORT_STRETCH, "enlightenment/wallpaper_stretch");
   _item_add(it, _("Center"),  IMPORT_CENTER,  "enlightenment/wallpaper_center");
   _item_add(it, _("Tile"),    IMPORT_TILE,    "enlightenment/wallpaper_tile");
   _item_add(it, _("Within"),  IMPORT_SCALE_ASPECT_IN,  "enlightenment/wallpaper_scale_aspect_in");
   _item_add(it, _("Fill"),    IMPORT_SCALE_ASPECT_OUT, "enlightenment/wallpaper_stretch");

   return it->items;
}

static Eina_Bool
_plugins_init(void)
{
   if (!evry_api_version_check(EVRY_API_VERSION))
     return EINA_FALSE;

   _act = EVRY_ACTION_NEW(_("Set as Wallpaper"),
			  EVRY_TYPE_FILE, 0,
			  _module_icon,
			  NULL, _check);
   _act->fetch = _fetch;
   EVRY_ITEM(_act)->browseable = EINA_TRUE;

   evry_action_register(_act, 2);

   return EINA_TRUE;
}

static void
_plugins_shutdown(void)
{
   evry_action_free(_act);
}

/* taken from e_int_config_wallpaper_import.c */
static void
_import_edj_gen(Import *import)
{
   Ecore_Evas *ee = ecore_evas_buffer_new(100, 100);
   Evas *evas = ecore_evas_get(ee);
   Evas_Object *img;
   int fd, num = 1;
   int w = 0, h = 0;
   const char *file, *locale;
   char buf[4096], cmd[4096], tmpn[4096], ipart[4096], enc[128];
   char *imgdir = NULL, *fstrip;
   int cr = 255, cg = 255, cb = 255, ca = 255;
   FILE *f;
   size_t len, off;

   file = ecore_file_file_get(import->file);
   fstrip = ecore_file_strip_ext(file);
   if (!fstrip) return;
   len = e_user_dir_snprintf(buf, sizeof(buf), "backgrounds/%s.edj", fstrip);
   if (len >= sizeof(buf)) return;
   off = len - (sizeof(".edj") - 1);
   while (ecore_file_exists(buf))
     {
	snprintf(buf + off, sizeof(buf) - off, "-%d.edj", num);
	num++;
     }
   free(fstrip);
   strcpy(tmpn, "/tmp/e_bgdlg_new.edc-tmp-XXXXXX");
   fd = mkstemp(tmpn);
   if (fd < 0)
     {
	printf("Error Creating tmp file: %s\n", strerror(errno));
	return;
     }
   close(fd);

   f = fopen(tmpn, "w");
   if (!f)
     {
	printf("Cannot open %s for writing\n", tmpn);
	return;
     }

   imgdir = ecore_file_dir_get(import->file);
   if (!imgdir) ipart[0] = '\0';
   else
     {
	snprintf(ipart, sizeof(ipart), "-id %s", e_util_filename_escape(imgdir));
	free(imgdir);
     }

   img = evas_object_image_add(evas);
   evas_object_image_file_set(img, import->file, NULL);
   evas_object_image_size_get(img, &w, &h);
   evas_object_del(img);
   ecore_evas_free(ee);

   printf("w%d h%d\n", w, h);

   if (import->external)
     {
	fstrip = strdup(e_util_filename_escape(import->file));
	snprintf(enc, sizeof(enc), "USER");
     }
   else
     {
	fstrip = strdup(e_util_filename_escape(file));
	if (import->quality == 100)
	  snprintf(enc, sizeof(enc), "COMP");
	else
	  snprintf(enc, sizeof(enc), "LOSSY %i", import->quality);
     }
   switch (import->method)
     {
      case IMPORT_STRETCH:
	 fprintf(f,
		 "images { image: \"%s\" %s; }\n"
		 "collections {\n"
		 "group { name: \"e/desktop/background\";\n"
		 "data { item: \"style\" \"0\"; }\n"
		 "max: %i %i;\n"
		 "parts {\n"
		 "part { name: \"bg\"; mouse_events: 0;\n"
		 "description { state: \"default\" 0.0;\n"
		 "image { normal: \"%s\"; scale_hint: STATIC; }\n"
		 "} } } } }\n"
		 , fstrip, enc, w, h, fstrip);
	 break;
      case IMPORT_TILE:
	 fprintf(f,
		 "images { image: \"%s\" %s; }\n"
		 "collections {\n"
		 "group { name: \"e/desktop/background\";\n"
		 "data { item: \"style\" \"1\"; }\n"
		 "max: %i %i;\n"
		 "parts {\n"
		 "part { name: \"bg\"; mouse_events: 0;\n"
		 "description { state: \"default\" 0.0;\n"
		 "image { normal: \"%s\"; }\n"
		 "fill { size {\n"
		 "relative: 0.0 0.0;\n"
		 "offset: %i %i;\n"
		 "} } } } } } }\n"
		 , fstrip, enc, w, h, fstrip, w, h);
	 break;
      case IMPORT_CENTER:
	 fprintf(f,
		 "images { image: \"%s\" %s; }\n"
		 "collections {\n"
		 "group { name: \"e/desktop/background\";\n"
		 "data { item: \"style\" \"2\"; }\n"
		 "max: %i %i;\n"
		 "parts {\n"
		 "part { name: \"col\"; type: RECT; mouse_events: 0;\n"
		 "description { state: \"default\" 0.0;\n"
		 "color: %i %i %i %i;\n"
		 "} }\n"
		 "part { name: \"bg\"; mouse_events: 0;\n"
		 "description { state: \"default\" 0.0;\n"
		 "min: %i %i; max: %i %i;\n"
		 "image { normal: \"%s\"; }\n"
		 "} } } } }\n"
		 , fstrip, enc, w, h, cr, cg, cb, ca, w, h, w, h, fstrip);
	 break;
      case IMPORT_SCALE_ASPECT_IN:
	 locale = e_intl_language_get();
	 setlocale(LC_NUMERIC, "C");
	 fprintf(f,
		 "images { image: \"%s\" %s; }\n"
		 "collections {\n"
		 "group { name: \"e/desktop/background\";\n"
		 "data { item: \"style\" \"3\"; }\n"
		 "max: %i %i;\n"
		 "parts {\n"
		 "part { name: \"col\"; type: RECT; mouse_events: 0;\n"
		 "description { state: \"default\" 0.0;\n"
		 "color: %i %i %i %i;\n"
		 "} }\n"
		 "part { name: \"bg\"; mouse_events: 0;\n"
		 "description { state: \"default\" 0.0;\n"
		 "aspect: %1.9f %1.9f; aspect_preference: BOTH;\n"
		 "image { normal: \"%s\";  scale_hint: STATIC; }\n"
		 "} } } } }\n"
		 , fstrip, enc, w, h, cr, cg, cb, ca, (double)w / (double)h, (double)w / (double)h, fstrip);
	 setlocale(LC_NUMERIC, locale);
	 break;
      case IMPORT_SCALE_ASPECT_OUT:
	 locale = e_intl_language_get();
	 setlocale(LC_NUMERIC, "C");
	 fprintf(f,
		 "images { image: \"%s\" %s; }\n"
		 "collections {\n"
		 "group { name: \"e/desktop/background\";\n"
		 "data { item: \"style\" \"4\"; }\n"
		 "max: %i %i;\n"
		 "parts {\n"
		 "part { name: \"bg\"; mouse_events: 0;\n"
		 "description { state: \"default\" 0.0;\n"
		 "aspect: %1.9f %1.9f; aspect_preference: NONE;\n"
		 "image { normal: \"%s\";  scale_hint: STATIC; }\n"
		 "} } } } }\n"
		 , fstrip, enc, w, h, (double)w / (double)h, (double)w / (double)h, fstrip);
	 setlocale(LC_NUMERIC, locale);
	 break;
      default:
	 /* won't happen */
	 break;
     }
   free(fstrip);

   fclose(f);

   snprintf(cmd, sizeof(cmd), "edje_cc -v %s %s %s",
	    ipart, tmpn, e_util_filename_escape(buf));

   import->tmpf = strdup(tmpn);
   import->fdest = strdup(buf);
   import->exe_handler =
     ecore_event_handler_add(ECORE_EXE_EVENT_DEL,
			     _import_cb_edje_cc_exit, import);
   import->exe = ecore_exe_run(cmd, NULL);
}

static int
_import_cb_edje_cc_exit(void *data, int type, void *event)
{
   Import *import;
   Ecore_Exe_Event_Del *ev;
   char *fdest;
   int r = 1;

   ev = event;
   import = data;

   if (!ev->exe) return 1;

   if (ev->exe != import->exe) return 1;

   if (ev->exit_code != 0)
     {
	e_util_dialog_show(_("Picture Import Error"),
			   _("Enlightenment was unable to import the picture<br>"
			     "due to conversion errors."));
	r = 0;
     }


   fdest = strdup(import->fdest);
   if (r)
     {
	e_bg_default_set(fdest);
	e_bg_update();
     }

   E_FREE(fdest);

   return 0;
}

/***************************************************************************/

static E_Module *module = NULL;
static Eina_Bool active = EINA_FALSE;

EAPI E_Module_Api e_modapi =
{
   E_MODULE_API_VERSION,
   "everything-wallpaper"
};

EAPI void *
e_modapi_init(E_Module *m)
{
   module = m;

   if (e_datastore_get("everything_loaded"))
     active = _plugins_init();

   e_module_delayed_set(m, 1);

   return m;
}

EAPI int
e_modapi_shutdown(E_Module *m)
{
   if (active && e_datastore_get("everything_loaded"))
     _plugins_shutdown();

   if (import)
     {
	if (import->exe_handler)
	  ecore_event_handler_del(import->exe_handler);
	E_FREE(import);
     }

   module = NULL;

   return 1;
}

EAPI int
e_modapi_save(E_Module *m)
{
   return 1;
}

/***************************************************************************/
