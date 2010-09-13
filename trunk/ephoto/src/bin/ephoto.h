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
Evas_Object *ephoto_create_flow_browser(Evas_Object *parent);
void ephoto_flow_browser_image_set(Evas_Object *obj, const char *current_image);
 /* smart callbacks called:
  * "delete,request" - the user requested to delete the flow browser, typically called when go_back button is pressed or Escape key is typed.
  */


/*Ephoto Slideshow*/
void ephoto_create_slideshow(void);
void ephoto_show_slideshow(int view, const char *current_image);
void ephoto_hide_slideshow(void);
void ephoto_delete_slideshow(void);

/*Ephoto Thumb Browser*/
Evas_Object *ephoto_create_thumb_browser(Evas_Object *parent, const char *directory);
void ephoto_populate_thumbnails(Evas_Object *obj);
/* smart callbacks called:
 * "selected" - an item in the thumb browser is selected. The selected file is passed as event_info argument.
 */

typedef enum _Ephoto_State Ephoto_State;

/* Enum for the state machine */
enum _Ephoto_State
{
        EPHOTO_STATE_THUMB,
        EPHOTO_STATE_FLOW,
        EPHOTO_STATE_SLIDESHOW
};

/*Ephoto Main Structure*/
struct _Ephoto
{
	Evas *e;
	Evas_Object *win;
	Evas_Object *bg;
	Evas_Object *layout;
	Evas_Object *flow_browser;
	Evas_Object *slideshow;
	Evas_Object *thumb_browser;
	Eina_List   *images;
        Ephoto_State state;
};
typedef struct _Ephoto Ephoto;

extern Ephoto *em;

#endif

