#ifndef _EPHOTO_H_
#define _EPHOTO_H_

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Ecore_File.h>
#include <Efreet_Mime.h>
#include <Elementary.h>
#include <Eina.h>
#include <Edje.h>
#include <Evas.h>
#include <Ethumb.h>
#include <Ethumb_Client.h>
#include <Eio.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

/*Main Functions*/
void ephoto_create_main_window(const char *directory, const char *image);

/*Ephoto Flow Browser*/
void ephoto_create_flow_browser(void);
void ephoto_show_flow_browser(const char *current_image);
void ephoto_hide_flow_browser(void);
void ephoto_delete_flow_browser(void);

/*Ephoto Slideshow*/
void ephoto_create_slideshow(void);
void ephoto_show_slideshow(int view, const char *current_image);
void ephoto_hide_slideshow(void);
void ephoto_delete_slideshow(void);

/*Ephoto Thumb Browser*/
void ephoto_create_thumb_browser(const char *directory);
void ephoto_show_thumb_browser(void);
void ephoto_hide_thumb_browser(void);
void ephoto_delete_thumb_browser(void);
void ephoto_populate_thumbnails(void);

/*Ephoto Main Structure*/
struct _Ephoto
{
	Evas *e;
	Evas_Object *win;
	Evas_Object *bg;
	Evas_Object *box;
	Evas_Object *flow_browser;
	Evas_Object *slideshow;
	Evas_Object *thumb_browser;
	Eina_List   *images;
};
typedef struct _Ephoto Ephoto;
extern Ephoto *em;

#endif

