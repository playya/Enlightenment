#include <stdio.h>
#include <stdlib.h>
#include "ewl.h"

char cb_test_option(int argc, char *argv[]);
char cb_mouse(EwlWidget *w, EwlEvent *ev, EwlData *d);
char cb_keydown(EwlWidget *w, EwlEvent *ev, EwlData *d);

int main(int argc, char *argv[])
{
	EwlWidget *win;
	EwlWidget *box;
	EwlWidget *btn;
	int        t = 0;

	/* declare command line options */
	ewl_option_add("t", "test", "This is a test option.",cb_test_option);

	/* initialize ewl */
	/*ewl_debug_enable();*/
	ewl_init(&argc, &argv);
	ewl_state_set_application_name("ewltest");

	/* configure widgets  and connect callbacks */
	win = ewl_window_new_with_values(EWL_WINDOW_TOPLEVEL, "Sampel window",
									320, 240);
	ewl_window_move(win,800,600);
	ewl_window_resize(win,640,480);
	ewl_window_set_class_hints(win,"blah", "blah");
	ewl_window_set_decoration_hint(win, TRUE);

	ewl_callback_add(win, EWL_EVENT_MOUSEDOWN, cb_mouse, NULL);

	/* pack widget(s) into container */
	box = ewl_hbox_new(FALSE);
	btn = ewl_button_new_with_label("Test Button");
	ewl_callback_add(btn, EWL_EVENT_MOUSEDOWN, cb_mouse, NULL);
	ewl_callback_add(btn, EWL_EVENT_MOUSEUP, cb_mouse, NULL);
	ewl_box_pack_end(box,btn);
	ewl_widget_show(btn);

	/* pack container into window */
	ewl_window_pack(win,box);
	ewl_widget_show(box);
	ewl_widget_show(win);

	/* DEBUGGING */
	fprintf(stderr, "win = 0x%08x\nbox = 0x%08x\nbtn = 0x%08x\n",
	        (unsigned int) win, (unsigned int) box, (unsigned int) btn);

	fprintf(stderr,"win: ");
	ewl_rect_dump(win->layout->rect);
	fprintf(stderr,"box: ");
	ewl_rect_dump(box->layout->rect);
	fprintf(stderr,"btn: ");
	ewl_rect_dump(btn->layout->rect);
	/* call the ewl_main() routine */
	ewl_main();

	return 0;
}


char cb_test_option(int argc, char *argv[])
{
	fprintf(stderr,"wahoo!\n");
	return 1;
}

char cb_mouse(EwlWidget *w, EwlEvent *ev, EwlData *d)
{
	char evtype[8] = "";

	if (ev->type==EWL_EVENT_MOUSEDOWN)
		sprintf(evtype,"down");
	else if (ev->type==EWL_EVENT_MOUSEUP)
		sprintf(evtype,"up");
	else if (ev->type==EWL_EVENT_MOUSEMOVE)
		sprintf(evtype,"move");

	fprintf(stderr,"mouse%s in widget 0x%08x\n", evtype, (unsigned int) w);
	return TRUE;
}

char cb_keydown(EwlWidget *w, EwlEvent *ev, EwlData *d)
{
	fprintf(stderr,"keydown in widget 0x%08x\n", (unsigned int) w);
	ewl_quit();
	return TRUE;
}
