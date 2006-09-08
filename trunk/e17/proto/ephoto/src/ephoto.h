#ifndef _EPHOTO_H_
#define _EPHOTO_H_

#include <Ecore_File.h>
#include <Ewl.h>
#include <fnmatch.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void destroy_cb(Ewl_Widget *w, void *event, void *data);
void populate_albums(Ewl_Widget *w, void *event, void *data);
void populate_browser(Ewl_Widget *w, void *event, void *data);
void populate_images(Ewl_Widget *w, void *event, void *data);
void go_up(Ewl_Widget *w, void *event, void *data);
void go_home(Ewl_Widget *w, void *event, void *data);
void entry_change(Ewl_Widget *w, void *event, void *data);
void start_slideshow(Ewl_Widget *w, void *event, void *data);

typedef struct _Main Main;

struct _Main
{
 Ewl_Widget *win;
 Ewl_Widget *vbox;
 Ewl_Widget *hbox;
 Ewl_Widget *groups;
 Ewl_Widget *menubar;
 Ewl_Widget *menu;
 Ewl_Widget *menu_item;
 Ewl_Widget *hpaned;
 Ewl_Widget *hseparator;
 Ewl_Widget *albums;
 Ewl_Widget *albums_border;
 Ewl_Widget *browser_border;
 Ewl_Widget *browser;
 Ewl_Widget *viewer;
 Ewl_Widget *viewer_border;
 Ewl_Widget *viewer_freebox;
 Ewl_Widget *vsep;
 Ewl_Widget *image;
 Ewl_Widget *icon;
 Ewl_Widget *text;
 Ewl_Widget *entry;
};

extern Main *m;
extern char *current_directory;
extern Ecore_List *current_thumbs;

#endif
