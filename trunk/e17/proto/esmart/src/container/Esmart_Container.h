#ifndef CONTAINER_H
#define CONTAINER_H

#include <Evas.h>
#include <Ecore.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>

/*****
 Todo:

   o add a "mover" object that tracks where moving element will go

****/

typedef struct _Container Container;
typedef struct _Container_Element Container_Element;
typedef struct _Scroll_Data Scroll_Data;
typedef enum _Container_Alignment Container_Alignment;
typedef enum _Container_Fill_Policy Container_Fill_Policy;
typedef struct _Container_Layout_Plugin Container_Layout_Plugin;


enum _Container_Alignment
{
  CONTAINER_ALIGN_CENTER,
  CONTAINER_ALIGN_LEFT,
  CONTAINER_ALIGN_RIGHT,
  CONTAINER_ALIGN_BOTTOM = CONTAINER_ALIGN_LEFT,
  CONTAINER_ALIGN_TOP = CONTAINER_ALIGN_RIGHT
};

enum _Container_Fill_Policy
{
  CONTAINER_FILL_POLICY_NONE = 0,
  CONTAINER_FILL_POLICY_KEEP_ASPECT = 0x01,
  CONTAINER_FILL_POLICY_FILL_X = 0x02,
  CONTAINER_FILL_POLICY_FILL_Y = 0x04,
  CONTAINER_FILL_POLICY_FILL = 0x08, 
  CONTAINER_FILL_POLICY_HOMOGENOUS = 0x10
};

struct _Container
{
  Evas *evas;
  Evas_Object *obj;     /* the evas smart object */
  Evas_Object *clipper; /* element clip */
  Evas_Object *grabber; /* event grabber (for the container as a whole) */

  Container_Layout_Plugin *plugin;

  Evas_List *elements;  /* things contained */

  struct
  {
    double l, r, t, b;
  } padding;

  double x, y, w, h;    /* geometry */
  
  int clipper_orig_alpha;		/* original alpha value of clipper */

  int spacing;          /* space between elements */

  int direction;        /* 0 = horizontal or 1 = vertical */
  Container_Alignment align;  /* CONTAINER_ALIGN_LEFT, _CENTER, or _RIGHT */
  Container_Fill_Policy fill;

  int move_button;      /* which button to move elements with? (0 for none) */

  int scroll_offset;
  Ecore_Timer *scroll_timer;

  void (*cb_order_change) (void *data);
  void *data_order_change;
};

struct _Container_Element
{
  Container *container;
  Evas_Object *obj;
  Evas_Object *grabber;

  double orig_w, orig_h;

  struct
  {
    double x, y;
  } down, delta, current;

  int mouse_down;
  int dragging;
};

struct _Scroll_Data
{
  Container *cont;
  double start_time;
  double velocity;
  double length;
};

Evas_Object *esmart_container_new(Evas *evas);

struct _Container_Layout_Plugin{
  void *handle;

  void (*shutdown)(void);
  
  void (*layout)(Container *cont);
  
  void (*scroll_start)(Container *cont, double velocity);
  void (*scroll_stop)(Container *cont);
  void (*scroll_to)(Container *cont, Container_Element *el);

  void (*post_init)(Container *cont);
  void (*changed)(Container *cont);
};


void esmart_container_direction_set(Evas_Object *container, int direction);
int  esmart_container_direction_get(Evas_Object *container);


void esmart_container_padding_set(Evas_Object *container, double l, double r,
                             double t, double b);
void esmart_container_padding_get(Evas_Object *container, double *l, double *r,
                             double *t, double *b);


void esmart_container_fill_policy_set(Evas_Object *container,
                                 Container_Fill_Policy fill);
Container_Fill_Policy  esmart_container_fill_policy_get(Evas_Object *container);


void esmart_container_alignment_set(Evas_Object *container,
                               Container_Alignment align);
Container_Alignment esmart_container_alignment_get(Evas_Object *container);


void esmart_container_spacing_set(Evas_Object *container,
                                int spacing);
int  esmart_container_spacing_get(Evas_Object *container);


void esmart_container_move_button_set(Evas_Object *container, int move_button);
int  esmart_container_move_button_get(Evas_Object *container);


/* element adding/removing */
void esmart_container_element_append(Evas_Object *container, Evas_Object *element);
void esmart_container_element_prepend(Evas_Object *container, Evas_Object *element);
void esmart_container_element_append_relative(Evas_Object *container,
                                         Evas_Object *element,
                                         Evas_Object *relative);
void esmart_container_element_prepend_relative(Evas_Object *container,
                                          Evas_Object *element,
                                          Evas_Object *relative);
void esmart_container_element_remove(Evas_Object *container, Evas_Object *element);
void esmart_container_element_destroy(Evas_Object *container, Evas_Object *element);
void esmart_container_empty (Evas_Object *container);
void esmart_container_sort(Evas_Object *container, int (*func)(void*,void*));

Evas_List *esmart_container_elements_get(Evas_Object *container);

/* scrolling */
void esmart_container_scroll_start(Evas_Object *container, double velocity);
void esmart_container_scroll_stop(Evas_Object *container);
void esmart_container_scroll(Evas_Object *container, int val);

void esmart_container_scroll_offset_set(Evas_Object *container, int val);
int  esmart_container_scroll_offset_get(Evas_Object *container);

/* callbacks */
void esmart_container_callback_order_change_set(Evas_Object *obj, 
                                           void (*func)(void *data),
                                           void *data);

double esmart_container_elements_length_get(Evas_Object *container);
double esmart_container_elements_orig_length_get(Evas_Object *container);

int esmart_container_layout_plugin_set(Evas_Object *container, const char *name);

void esmart_container_scroll_to(Evas_Object *container, Evas_Object *element);

#endif
