#ifndef _WINDOW_H_
#define _WINDOW_H_

#include "widget.h"
#include "container.h"
#include "x.h"
#include "font.h"
#include "api.h"

#define EWL_WINDOW(a) ((EwlWindow*)a)

typedef struct _EwlWindow EwlWindow;

struct _EwlWindow {
	EwlContainer widget;
	Evas         evas;
};

/* WINDOW NEW/FREE FUNCTIONS */
EwlWidget *ewl_window_new(char *type);
void       ewl_window_init(EwlWidget *widget);
void       ewl_window_free(EwlWidget *widget);

/* WINDOW EVAS FUNCTIONS */
Evas       ewl_window_get_evas(EwlWidget *widget);

/* WINDOW MISC FUNCTIONS */
EwlWidget *ewl_window_find_by_evas(Window xwin);

/* WINDOW EVENT CALLBACKS */
void       ewl_window_configure_callback(void     *object,
                                       EwlEvent *event,
                                       void     *data);
void       ewl_window_expose_callback(void     *object,
                                    EwlEvent *event,
                                    void     *data);
void       ewl_window_realize_callback(void     *object,
                                     EwlEvent *event,
                                     void     *data);
void       ewl_window_unrealize_callback(void     *object,
                                       EwlEvent *event,
                                       void     *data);
void       ewl_window_show_callback(void     *object,
                                  EwlEvent *event,
                                  void     *data);
void       ewl_window_hide_callback(void     *object,
                                  EwlEvent *event,
                                  void     *data);
#endif /* _WINDOW_H_ */
