
#ifndef __EWL_FX_H
#define __EWL_FX_H


enum _ewl_fx_type {
	EWL_FX_TYPE_FADE_IN,
	EWL_FX_TYPE_FADE_OUT,
	EWL_FX_TYPE_GLOW,
	EWL_FX_TYPE_MAX
};

typedef enum _ewl_fx_type Ewl_FX_Type;

struct _ewl_fx_timer {
	Ewl_Widget * widget; /* What widget is it we want to do an effect on ? */
	Ewl_FX_Type type; /* What type ? */
	int	repeat; /* How many times should we repeat ? */
	int completed; /* Keep track on how many times we have doon the fx */
	double timeout; /* The initial timeout */
	int start_val;
	int increase;
	char * name;
	void (*func) (Ewl_Widget * widget, void * func_data);
	void * func_data;
};

typedef struct _ewl_fx_timer Ewl_FX_Timer;

#define EWL_FX_TIMER(timer) ((Ewl_FX_Timer *) timer)

void ewl_fx_init();
void ewl_fx_add(Ewl_Widget * widget, Ewl_FX_Type type,
				void (*func) (Ewl_Widget * widget, void * func_data),
				void * func_data);
void ewl_fx_clip_box_create(Ewl_Widget * widget);
void ewl_fx_clip_box_resize(Ewl_Widget * widget);

#endif
