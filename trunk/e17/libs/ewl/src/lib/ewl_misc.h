#ifndef __EWL_MISC_H__
#define __EWL_MISC_H__

/**
 * @file ewl_misc.h
 * @defgroup Ewl_Misc Misc: Miscellaneous Utility Functions
 * Provides important miscellaneous functionality such as manipulation of the
 * main loop.
 */

typedef struct Ewl_Options Ewl_Options;

struct Ewl_Options
{
	int             debug_level;
	char           *xdisplay;
};

int             ewl_init(int *argc, char **argv);
int             ewl_shutdown(void);
void            ewl_main(void);
void            ewl_main_quit(void);
void            ewl_configure_request(Ewl_Widget * w);
void            ewl_configure_queue(void);
void            ewl_configure_cancel_request(Ewl_Widget *w);
void            ewl_realize_request(Ewl_Widget *w);
void            ewl_realize_cancel_request(Ewl_Widget *w);
void            ewl_realize_queue(void);
void            ewl_destroy_request(Ewl_Widget *w);
void            ewl_garbage_collect(void);
void            ewl_realize_phase_enter(void);
void            ewl_realize_phase_exit(void);
int             ewl_in_realize_phase(void);
unsigned int    ewl_engine_mask_get();
void            ewl_evas_destroy(Evas *evas);
void            ewl_evas_object_destroy(Evas_Object *obj);

/* #define DEBUG_MALLOCDEBUG 1 */
#ifdef DEBUG_MALLOCDEBUG
char *strdup(const char *str);
#endif

#endif				/* __EWL_MISC_H__ */
