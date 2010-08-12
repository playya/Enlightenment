#ifndef EWEBKIT_DEMO_PRIVATE_H
#define EWEBKIT_DEMO_PRIVATE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Elementary.h>
#include <EWebKit.h>

#include "favorite.h"
#include "history.h"

typedef struct _App App;
typedef struct _Browser_Window Browser_Window;
typedef struct _View_Zoom_Interactive View_Zoom_Interactive;

extern int _log_domain;
extern Fav *fav;
extern Hist *hist;
extern App app;

#define CRITICAL(...) EINA_LOG_DOM_CRIT(_log_domain, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(_log_domain, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_log_domain, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_log_domain, __VA_ARGS__)
#define DBG(...) EINA_LOG_DOM_DBG(_log_domain, __VA_ARGS__)

#define DEFAULT_URL "http://www.google.com"

/* TODO: Move all tab related stuff to its own struct */
struct _Browser_Window
{
   App *app;

   Evas_Object *win;
   Evas_Object *bg;
   Evas_Object *pager;

   Eina_List *chromes;

   Eina_List *list_history;
   Eina_List *list_history_titles;

   Evas_Object *current_chrome;
   Evas_Object *current_view;

   unsigned int current_tab;
};

struct _App
{
   Eina_List *windows;
   const char *user_agent;

   Eina_Bool is_fullscreen;
   Eina_Bool disable_plugins;
   Eina_Bool disable_mouse;
};

struct _View_Zoom_Interactive
{
   Evas_Coord x, y;
   float zoom;
};

/**
 * Create new view object.
 *
 * Signals:
 *
 *  * "zoom,interactive" View_Zoom_Interactive: zoom interactive
 *    requested a new level specified in given event information.
 *
 *  * "zoom,interactive,start" void: report zoom interactive started.
 *  * "zoom,interactive,end" void: report zoom interactive is finished.
 */

Evas_Object *window_create();
Evas_Object *view_add(Evas_Object *parent);
void         view_zoom_next_up(Evas_Object *view);
void         view_zoom_next_down(Evas_Object *view);
Eina_Bool    view_context_menu_set(Evas_Object *view, Evas_Object *widget, Ewk_Context_Menu *menu);
Evas_Object *view_context_menu_widget_get(Evas_Object *view);
Ewk_Context_Menu *view_context_menu_get(Evas_Object *view);

Evas_Object *chrome_add(Browser_Window *win, const char *url);

Eina_Bool    tab_add(Browser_Window *win, const char *url);
Eina_Bool    tab_focus_nth(Browser_Window *win, unsigned int n);
Eina_Bool    tab_focus_chrome(Browser_Window * win, Evas_Object * chrome);
Eina_Bool    tab_focus_next(Browser_Window *win);
Eina_Bool    tab_focus_prev(Browser_Window *win);
Eina_Bool    tab_close_nth(Browser_Window *win, int n);
Eina_Bool    tab_close_view(Browser_Window *win, Evas_Object * view);
Eina_Bool    tab_close_chrome(Browser_Window * win, Evas_Object * chrome);

#endif
