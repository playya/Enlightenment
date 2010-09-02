/*
 * eve
 *
 * Copyright (C) 2009, Gustavo Sverzut Barbieri <barbieri@profusion.mobi>
 *
 * License LGPL-3, see COPYING file at project folder.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH

#include "favorite.h"
#include "private.h"

#include <Ecore_Getopt.h>
#include <Ecore_File.h>
#include <stdlib.h>
#include "gettext.h"

int _log_domain = -1;
Fav *fav = NULL;
Hist *hist = NULL;
App app;

static void
del_win(App *app, Evas_Object *win)
{
   Browser_Window *win_data;
   Eina_List *l;

   EINA_LIST_FOREACH(app->windows, l, win_data) if (win_data->win == win)
      break;

   evas_object_del(win);
   app->windows = eina_list_remove(app->windows, win_data);
   free(win_data);

   if (!app->windows)
      elm_exit();
}

static void
on_win_del_req(void *data, Evas_Object *win, void *event_info __UNUSED__)
{
   del_win(data, win);
}

/* this should be in elm_win... */
static void
win_mouse_disable(Evas_Object *win)
{
   Evas *e = evas_object_evas_get(win);
   Ecore_Evas *ee = evas_data_attach_get(e);
   Evas_Object *cursor = evas_object_rectangle_add(e);

   evas_object_color_set(cursor, 0, 0, 0, 0);
   evas_object_resize(cursor, 1, 1);
   ecore_evas_object_cursor_set(ee, cursor, EVAS_LAYER_MIN, 0, 0);
}

Eina_Bool
tab_add(Browser_Window *win, const char *url)
{
   Evas_Object *chrome = chrome_add(win, url);

   if (!chrome)
     {
        CRITICAL("Could not create chrome.");
        goto error_chrome_create;
     }

   evas_object_size_hint_weight_set(chrome, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(chrome);

   win->chromes = eina_list_append(win->chromes, chrome);
   win->current_chrome = chrome;
   win->current_view = evas_object_data_get(chrome, "view");
   win->current_tab++;

   return EINA_TRUE;

error_chrome_create:
   evas_object_del(evas_object_data_get(chrome, "view"));

   return EINA_FALSE;
}

Eina_Bool
tab_focus_chrome(Browser_Window *win, Evas_Object *chrome)
{
   Eina_List *itr;
   int n;

   if (!chrome)
      return EINA_FALSE;

   for (n = 0, itr = win->chromes; itr->data != chrome; n++, itr = itr->next) ;

   evas_object_hide(win->current_chrome);

   win->current_chrome = chrome;
   win->current_view = evas_object_data_get(chrome, "view");
   win->current_tab = n;

   evas_object_show(win->current_chrome);
   evas_object_focus_set(win->current_view, EINA_TRUE);
   elm_pager_content_promote(win->pager, win->current_chrome);

   return EINA_TRUE;
}

Eina_Bool
tab_focus_nth(Browser_Window *win, unsigned int n)
{
   return tab_focus_chrome(win, eina_list_nth(win->chromes, n));
}

Eina_Bool
tab_focus_next(Browser_Window *win)
{
   unsigned int n_tabs = eina_list_count(win->chromes);

   if (win->current_tab > n_tabs)
      return EINA_FALSE;

   return tab_focus_nth(win, win->current_tab + 1);
}

Eina_Bool
tab_focus_prev(Browser_Window *win)
{
   if (win->current_tab == 0)
      return EINA_FALSE;

   return tab_focus_nth(win, win->current_tab - 1);
}

Eina_Bool
tab_close_chrome(Browser_Window *win, Evas_Object *chrome)
{
   Evas_Object *edje;

   EINA_SAFETY_ON_TRUE_RETURN_VAL(!win, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(!chrome, EINA_FALSE);

   if (!win->chromes->next)
     {
        del_win(win->app, win->win);
        return EINA_TRUE;
     }

   evas_object_del(chrome);
   win->chromes = eina_list_remove(win->chromes, chrome);
   if (win->current_chrome == chrome)
      tab_focus_nth(win, 0);

   edje = elm_layout_edje_get(win->current_chrome);
   edje_object_signal_emit(edje, "hide,tab", "");

   return EINA_TRUE;
}

Eina_Bool
tab_close_nth(Browser_Window *win, int n)
{
   return tab_close_chrome(win, eina_list_nth(win->chromes, n));
}

Eina_Bool
tab_close_view(Browser_Window *win, Evas_Object *view)
{
   return tab_close_chrome(win, evas_object_data_get(view, "chrome"));
}

static Browser_Window *
add_win(App *app, const char *url)
{
   Browser_Window *win = malloc(sizeof(*win));

   if (!win)
     {
        CRITICAL("Could not create window data.");
        goto error_win_data;
     }

   win->app = app;
   win->chromes = NULL;
   win->current_chrome = NULL;
   win->current_view = NULL;
   win->current_tab = 0;
   win->list_history = NULL;
   win->list_history_titles = NULL;

   win->win = elm_win_add(NULL, "eve", ELM_WIN_BASIC);
   if (!win->win)
     {
        CRITICAL("Could not create window.");
        goto error_win_create;
     }

   elm_win_title_set(win->win, PACKAGE_STRING);
   elm_win_rotation_set(win->win, app->rotate);
   elm_win_fullscreen_set(win->win, app->is_fullscreen);
   if (app->disable_mouse)
      win_mouse_disable(win->win);

   win->bg = edje_object_add(evas_object_evas_get(win->win));
   if (!win->bg)
     {
        CRITICAL("Could not create background.");
        goto error_bg_create;
     }

   if (!edje_object_file_set(win->bg, PACKAGE_DATA_DIR "/default.edj", "bg"))
     {
        int err = edje_object_load_error_get(win->bg);

        const char *msg = edje_load_error_str(err);

        CRITICAL("Could not load background theme: %s", msg);
        goto error_bg_theme_set;
     }

   evas_object_size_hint_weight_set(win->bg, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win->win, win->bg);
   evas_object_layer_set(win->bg, EVAS_LAYER_MIN);
   evas_object_show(win->bg);

   win->pager = elm_pager_add(win->win);
   if (!win->pager)
     {
        CRITICAL("Could not create pager");
        goto error_pager_create;
     }

   elm_object_style_set(win->pager, "ewebkit");
   evas_object_size_hint_weight_set(win->pager, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win->win, win->pager);
   evas_object_show(win->pager);

   if (!tab_add(win, url))
      goto error_tab_add;

   app->windows = eina_list_append(app->windows, win);
   evas_object_smart_callback_add
      (win->win, "delete-request", on_win_del_req, app);

   evas_object_resize(win->win, 480, 800);
   evas_object_show(win->win);

   return win;

error_bg_theme_set:
   evas_object_del(win->bg);
error_bg_create:
   evas_object_del(win->win);
error_win_create:
   free(win);
error_win_data:
error_tab_add:
error_pager_create:
   return NULL;
}

/**
 * Creates a new window, without any url to load, calling add_win().
 *
 * @return If a window was successfully created, it returns the correspondent view
 * object.
 */
Evas_Object *
window_create()
{
   Browser_Window *win = add_win(&app, NULL);

   if (!win)
      return NULL;

   return win->current_view;
}

#define stringify(X) #X

static const Ecore_Getopt options = {
   PACKAGE_NAME,
   "%prog [options] [url]",
   PACKAGE_VERSION "Revision:" stringify(VREV),
   "(C) 2010 ProFUSION embedded systems",
   "LGPL-3",
   "WebKit-EFL demo browser for mobile systems with touchscreen.",
   EINA_TRUE,
   {
      ECORE_GETOPT_STORE_DEF_BOOL('F', "fullscreen", "start in fullscreen.", 1),
      ECORE_GETOPT_STORE_DEF_BOOL('P', "disable-plugins",
                                  "disable plugins (flash, etc).", 1),
      ECORE_GETOPT_STORE_DEF_BOOL('M', "disable-mouse",
                                  "disable mouse (hide it).", 1),
      ECORE_GETOPT_STORE_STR('U', "user-agent",
                             "user agent string to use. Special case=iphone."),
      ECORE_GETOPT_STORE_DEF_UINT('R', "rotate", "Screen Rotation in degrees", 0),
      ECORE_GETOPT_VERSION('V', "version"),
      ECORE_GETOPT_COPYRIGHT('C', "copyright"),
      ECORE_GETOPT_LICENSE('L', "license"),
      ECORE_GETOPT_HELP('h', "help"),
      ECORE_GETOPT_SENTINEL
   }
};

EAPI int
elm_main(int argc, char **argv)
{
   int r = 0, args;
   const char *home;
   const char *url;
   char path[PATH_MAX];
   Eina_Bool quit_option = EINA_FALSE;
   char *user_agent = NULL;

   Ecore_Getopt_Value values[] = {
      ECORE_GETOPT_VALUE_BOOL(app.is_fullscreen),
      ECORE_GETOPT_VALUE_BOOL(app.disable_plugins),
      ECORE_GETOPT_VALUE_BOOL(app.disable_mouse),
      ECORE_GETOPT_VALUE_STR(user_agent),
      ECORE_GETOPT_VALUE_UINT(app.rotate),
      ECORE_GETOPT_VALUE_BOOL(quit_option),
      ECORE_GETOPT_VALUE_BOOL(quit_option),
      ECORE_GETOPT_VALUE_BOOL(quit_option),
      ECORE_GETOPT_VALUE_BOOL(quit_option),
      ECORE_GETOPT_VALUE_NONE
   };

#if ENABLE_NLS
   setlocale(LC_ALL, "");
   bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
   bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
   textdomain(GETTEXT_PACKAGE);
#endif

   _log_domain = eina_log_domain_register("eve", NULL);
   if (_log_domain < 0)
     {
        EINA_LOG_CRIT("could not create log domain 'eve'.");
        return -1;
     }

   args = ecore_getopt_parse(&options, values, argc, argv);
   if (args < 0)
     {
        ERR("Could not parse command line options.");
        return -1;
     }

   if (quit_option)
     {
        DBG("Command lines option requires quit.");
        return 0;
     }

   if (args < argc)
      url = argv[args];
   else
      url = DEFAULT_URL;

   if (user_agent && strcasecmp(user_agent, "iphone") == 0)
      user_agent =
         "Mozilla/5.0 (iPhone; U; CPU like Mac OS X; en) AppleWebKit/420+ (KHTML, like Gecko) Version/3.0 Mobile/1A543a Safari/419.3";

   app.user_agent = eina_stringshare_add(user_agent);

   elm_theme_extension_add(NULL, PACKAGE_DATA_DIR "/default.edj");
   ewk_init();
   favorite_init();
   history_init();

   home = getenv("HOME");
   if (!home || !home[0])
     {
        CRITICAL("Could not get $HOME");
        r = -1;
        goto end;
     }

   snprintf(path, sizeof(path), "%s/.config/ewebkit", home);
   if (!ecore_file_mkpath(path))
     {
        ERR("Could not create %s", path);
        r = -1;
        goto end;
     }

   if (!ewk_settings_icon_database_path_set(path))
     {
        ERR("Could not set icon database path to %s", path);
        r = -1;
        goto end;
     }

   snprintf(path, sizeof(path), "%s/.config/ewebkit/favorites.db", home);
   fav = fav_load(path);
   if (!fav)
     {
        fav = fav_new(0);
        fav_save(fav, path);
     }

   snprintf(path, sizeof(path), "%s/.config/ewebkit/history.db", home);
   hist = hist_load(path);
   if (!hist)
     {
        hist = hist_new(0);
        hist_save(hist, path);
     }

   if (!add_win(&app, url))
     {
        r = -1;
        goto end;
     }

   elm_run();
end:
   fav_save(fav, NULL);
   fav_free(fav);

   hist_save(hist, NULL);
   hist_free(hist);

   eina_stringshare_del(app.user_agent);

   eina_log_domain_unregister(_log_domain);
   _log_domain = -1;
   elm_shutdown();
   ewk_shutdown();
   favorite_shutdown();
   history_shutdown();
   return r;
}

#endif
ELM_MAIN()
