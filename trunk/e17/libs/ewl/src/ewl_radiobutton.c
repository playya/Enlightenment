
#include <Ewl.h>

/*\
|*|
|*| The button widget is really a merge of many diffrent button types.
|*| Normal - You but whatever widget you want in here
|*| Label - You specify label for button..
|*|
\*/

static void __ewl_radiobutton_init(Ewl_RadioButton * b, const char *label);
static void __ewl_radiobutton_realize(Ewl_Widget * w, void *event_data,
				      void *user_data);
static void __ewl_radiobutton_show(Ewl_Widget * w, void *event_data,
				   void *user_data);
static void __ewl_radiobutton_hide(Ewl_Widget * w, void *event_data,
				   void *user_data);
static void __ewl_radiobutton_destroy(Ewl_Widget * w, void *event_data,
				      void *user_data);
static void __ewl_radiobutton_configure(Ewl_Widget * w, void *event_data,
					void *user_data);
static void __ewl_radiobutton_focus_in(Ewl_Widget * w, void *event_data,
				       void *user_data);
static void __ewl_radiobutton_focus_out(Ewl_Widget * w, void *event_data,
					void *user_data);
static void __ewl_radiobutton_mouse_down(Ewl_Widget * w, void *event_data,
					 void *user_data);
static void __ewl_radiobutton_mouse_up(Ewl_Widget * w, void *event_data,
				       void *user_data);
static void __ewl_radiobutton_theme_update(Ewl_Widget * w,
					   void *event_data,
					   void *user_data);


Ewl_Widget *
ewl_radiobutton_new(const char *l)
{
	Ewl_RadioButton *b;

	DENTER_FUNCTION;

	b = NEW(Ewl_RadioButton, 1);

	__ewl_radiobutton_init(b, l);

	DRETURN_PTR(EWL_WIDGET(b));
}

static void
__ewl_radiobutton_init(Ewl_RadioButton * b, const char *label)
{
	Ewl_Widget * w;

	DENTER_FUNCTION;

	/*
	 * Blank out the structure and initialize it's theme
	 */
	memset(b, 0, sizeof(Ewl_RadioButton));
	ewl_container_init(EWL_CONTAINER(b), 16, 16,
			   EWL_FILL_POLICY_NORMAL, EWL_ALIGNMENT_CENTER);

	w = EWL_WIDGET(b);

	/*
	 * Override the default recursive setting on containers. This prevents
	 * the coordinate->object mapping from searching below the button
	 * class.
	 */
	EWL_WIDGET(b)->recursive = FALSE;

	/*
	 * Add the label if desired
	 */
	if (label)
		EWL_BUTTON(b)->label = strdup(label);

	/*
	 * Attach necessary callback mechanisms
	 */
	ewl_callback_append(w, EWL_CALLBACK_REALIZE,
			    __ewl_radiobutton_realize, NULL);
	ewl_callback_append(w, EWL_CALLBACK_SHOW,
			    __ewl_radiobutton_show, NULL);
	ewl_callback_append(w, EWL_CALLBACK_HIDE,
			    __ewl_radiobutton_hide, NULL);
	ewl_callback_append(w, EWL_CALLBACK_DESTROY,
			    __ewl_radiobutton_destroy, NULL);
	ewl_callback_append(w, EWL_CALLBACK_CONFIGURE,
			    __ewl_radiobutton_configure, NULL);
	ewl_callback_append(w, EWL_CALLBACK_MOUSE_DOWN,
			    __ewl_radiobutton_mouse_down, NULL);
	ewl_callback_append(w, EWL_CALLBACK_MOUSE_UP,
			    __ewl_radiobutton_mouse_up, NULL);
	ewl_callback_append(w, EWL_CALLBACK_FOCUS_IN,
			    __ewl_radiobutton_focus_in, NULL);
	ewl_callback_append(w, EWL_CALLBACK_FOCUS_OUT,
			    __ewl_radiobutton_focus_out, NULL);
	ewl_callback_append(w, EWL_CALLBACK_THEME_UPDATE,
			    __ewl_radiobutton_theme_update, NULL);

	DLEAVE_FUNCTION;
}

/*
 * Change the state of the specified button
 */
void
ewl_radiobutton_set_checked(Ewl_Widget * w, int c)
{
	Ewl_RadioButton *b;

	DENTER_FUNCTION;
	DCHECK_PARAM_PTR("w", w);

	b = EWL_RADIOBUTTON(w);

	EWL_CHECKBUTTON(b)->checked = c;

	DLEAVE_FUNCTION;
}


static void
__ewl_radiobutton_realize(Ewl_Widget * w, void *event_data,
			  void *user_data)
{
	char * l = NULL;

	DENTER_FUNCTION;
	DCHECK_PARAM_PTR("w", w);

	ewl_fx_clip_box_create(w);

	if (w->parent && EWL_CONTAINER(w->parent)->clip_box)
		evas_set_clip(w->evas, w->fx_clip_box,
			      EWL_CONTAINER(w->parent)->clip_box);

	if (EWL_BUTTON(w)->label)
		l = strdup(EWL_BUTTON(w)->label);

	if (l)
		ewl_button_set_label(w, l);

	IF_FREE(l);
	
	ewl_widget_theme_update(w);

	DLEAVE_FUNCTION;
}

static void
__ewl_radiobutton_show(Ewl_Widget * w, void *event_data, void *user_data)
{
	DENTER_FUNCTION;
	DCHECK_PARAM_PTR("w", w);

	evas_show(w->evas, w->fx_clip_box);

	DLEAVE_FUNCTION;
}

static void
__ewl_radiobutton_hide(Ewl_Widget * w, void *event_data, void *user_data)
{
	DENTER_FUNCTION;
	DCHECK_PARAM_PTR("w", w);

	evas_hide(w->evas, w->fx_clip_box);

	DLEAVE_FUNCTION;
}

static void
__ewl_radiobutton_destroy(Ewl_Widget * w, void *event_data,
			  void *user_data)
{
	Ewl_RadioButton *b;

	DENTER_FUNCTION;
	DCHECK_PARAM_PTR("w", w);

	b = EWL_RADIOBUTTON(w);

	if (w->ebits_object) {
		ebits_hide(w->ebits_object);
		ebits_unset_clip(w->ebits_object);
		ebits_free(w->ebits_object);
	}

	if (w->fx_clip_box)
	  {
		evas_hide(w->evas, w->fx_clip_box);
		evas_unset_clip(w->evas, w->fx_clip_box);
		evas_del_object(w->evas, w->fx_clip_box);
	  }

	ewl_callback_clear(w);

	ewl_theme_deinit_widget(w);

	FREE(b);

	DLEAVE_FUNCTION;
}

static void
__ewl_radiobutton_configure(Ewl_Widget * w, void *event_data,
			    void *user_data)
{
	Ewl_Button * b;
	int req_x, req_y, req_w, req_h;
	int cur_w = 0, cur_h = 0;

	DENTER_FUNCTION;
	DCHECK_PARAM_PTR("w", w);

	b = EWL_BUTTON(w);

	ewl_object_requested_geometry(EWL_OBJECT(w), &req_x, &req_y,
				      &req_w, &req_h);

	if (w->ebits_object) {
		ebits_move(w->ebits_object, req_x, req_y);
		ebits_resize(w->ebits_object, req_w, req_h);
	}

	if (w->fx_clip_box)
	  {
		evas_move(w->evas, w->fx_clip_box, req_x, req_y);
		evas_resize(w->evas, w->fx_clip_box, req_w, req_h);
		
	  }

	if (EWL_CONTAINER(w)->clip_box)
	  {
		evas_move(w->evas, EWL_CONTAINER(w)->clip_box, req_x,
			  req_y);
		evas_resize(w->evas, EWL_CONTAINER(w)->clip_box, req_w,
			    req_h);
	  }


        if (b->label_object) {
                ewl_object_get_current_size(EWL_OBJECT(b->label_object),
                                            &cur_w, &cur_h);

                ewl_object_request_geometry(EWL_OBJECT(b->label_object),
                                            req_x + 19,
                                            req_y + ((req_h / 2) -
                                                     (cur_h / 2)), cur_w,
                                            cur_h);

                ewl_widget_configure(b->label_object);

                ewl_object_get_current_size(EWL_OBJECT(b->label_object),
                                            &cur_w, &cur_h);
        }
        
        ewl_object_set_current_geometry(EWL_OBJECT(w), req_x, req_y,
                                        19 + cur_w, req_h);

        ewl_object_set_custom_size(EWL_OBJECT(w), 19 + cur_w, req_h);

	DLEAVE_FUNCTION;
}


static void
__ewl_radiobutton_focus_in(Ewl_Widget * w, void *event_data,
			   void *user_data)
{
	DENTER_FUNCTION;
	DCHECK_PARAM_PTR("w", w);

	ewl_widget_theme_update(w);

	DLEAVE_FUNCTION;
}

static void
__ewl_radiobutton_focus_out(Ewl_Widget * w, void *event_data,
			    void *user_data)
{
	DENTER_FUNCTION;
	DCHECK_PARAM_PTR("w", w);

	ewl_widget_theme_update(w);

	DLEAVE_FUNCTION;
}

static void
__ewl_radiobutton_mouse_down(Ewl_Widget * w, void *event_data,
			     void *user_data)
{
	Ewl_RadioButton *b;

	DENTER_FUNCTION;
	DCHECK_PARAM_PTR("w", w);

	b = EWL_RADIOBUTTON(w);

	w->state |= EWL_STATE_PRESSED;

	EWL_CHECKBUTTON(b)->checked = EWL_CHECKBUTTON(b)->checked ? 0 : 1;

	ewl_widget_theme_update(w);

	ewl_callback_call(w, EWL_CALLBACK_VALUE_CHANGED);

	DLEAVE_FUNCTION;
}

static void
__ewl_radiobutton_mouse_up(Ewl_Widget * w, void *event_data,
			   void *user_data)
{
	Ewl_RadioButton *b;

	DENTER_FUNCTION;
	DCHECK_PARAM_PTR("w", w);

	b = EWL_RADIOBUTTON(w);

	w->state &= ~EWL_STATE_PRESSED;

	ewl_widget_theme_update(w);

	if (w->state & EWL_STATE_HILITED)
		ewl_callback_call(w, EWL_CALLBACK_CLICKED);

	DLEAVE_FUNCTION;
}

static void
__ewl_radiobutton_theme_update(Ewl_Widget * w, void *event_data,
			       void *user_data)
{
	char *v;
	char str[512];
	char state[512];

	DENTER_FUNCTION;
	DCHECK_PARAM_PTR("w", w);

	/*
	 * Don't want to update anything if the widget isn't realized. 
	 */
	if (!REALIZED(w))
		DRETURN;

	if (w->state & EWL_STATE_PRESSED)
		snprintf(state, 512, "clicked");
	else if (w->state & EWL_STATE_HILITED)
		snprintf(state, 512, "hilited");
	else
		snprintf(state, 512, "base");

	if (w->ebits_object)
	  {
		ebits_hide(w->ebits_object);
		ebits_unset_clip(w->ebits_object);
		ebits_free(w->ebits_object);
	  }

	snprintf(str, 512, "/appearance/button/radio/%s-checked%i/visible",
		 state, EWL_CHECKBUTTON(w)->checked);

	v = ewl_theme_data_get(w, str);

	if (v && !strncasecmp(v, "yes", 3)) {
		char *i;

		snprintf(str, 512, "/appearance/button/radio/%s-checked%i",
			 state, EWL_CHECKBUTTON(w)->checked);

		i = ewl_theme_image_get(w, str);

		if (i) {
			w->ebits_object = ebits_load(i);
			FREE(i);

			if (w->ebits_object) {
				ebits_add_to_evas(w->ebits_object,
						  w->evas);
				ebits_set_layer(w->ebits_object,
						EWL_OBJECT(w)->layer);
				ebits_set_clip(w->ebits_object,
					       w->fx_clip_box);

				ebits_show(w->ebits_object);
			}
		}
	}

	/*
	 * Finally comfigure the widget to update the changes 
	 */
	ewl_widget_configure(w);
}
