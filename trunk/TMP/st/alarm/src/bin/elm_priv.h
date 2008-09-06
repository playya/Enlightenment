#ifndef ELM_PRIV_H
#define ELM_PRIV_H

#include "els_pan.h"
#include "els_scroller.h"

// FIXME: totally disorganised. clean this up!

typedef enum _Elm_Engine
{
   ELM_SOFTWARE_X11,
     ELM_SOFTWARE_FB,
     ELM_SOFTWARE_16_X11,
     ELM_XRENDER_X11,
     ELM_OPENGL_X11
} Elm_Engine;

typedef struct _Elm_Config Elm_Config;

struct _Elm_Config
{
   int engine;
   int thumbscroll_enable;
   int thumbscroll_threshhold;
   double thumbscroll_momentum_threshhold;
   double thumbscroll_friction;
};

#define ELM_NEW(t) calloc(1, sizeof(t))

void _elm_obj_init(Elm_Obj *obj);
void _elm_obj_nest_push(void);
void _elm_obj_nest_pop(void);
int _elm_obj_del_defer(Elm_Obj *obj);
Elm_Cb *_elm_cb_new(void);
void _elm_cb_call(Elm_Obj *obj, Elm_Cb_Type, void *info);
int _elm_theme_set(Evas_Object *o, const char *clas, const char *group);
void _elm_widget_init(Elm_Widget *wid);
    
extern char *_elm_appname;

extern Elm_Config *_elm_config;

extern Elm_Obj_Class _elm_obj_class;
extern Elm_Win_Class _elm_win_class;
extern Elm_Widget_Class _elm_widget_class;
extern Elm_Bg_Class _elm_bg_class;
extern Elm_Scroller_Class _elm_scroller_class;
  
#endif
