
#include <Ewl.h>


void ewl_checkbutton_init(Ewl_CheckButton * cb, char *label);

void __ewl_checkbutton_configure(Ewl_Widget * w, void *ev_data,
				 void *user_data);
void __ewl_checkbutton_mouse_down(Ewl_Widget * w, void *ev_data,
				  void *user_data);
void __ewl_checkbutton_update_check(Ewl_Widget * w);

void __ewl_box_configure(Ewl_Widget * w, void *ev_data, void *user_data);

void ewl_button_init(Ewl_Button * b, char *l);


Ewl_Widget *
ewl_checkbutton_new(char *label)
{
	Ewl_CheckButton *b;

	DENTER_FUNCTION;

	b = NEW(Ewl_CheckButton, 1);
	if (!b)
		return NULL;

	memset(b, 0, sizeof(Ewl_CheckButton));
	ewl_checkbutton_init(b, label);

	DRETURN_PTR(EWL_WIDGET(b));
}

void
ewl_checkbutton_set_checked(Ewl_Widget * w, int c)
{
	Ewl_CheckButton *cb;

	DENTER_FUNCTION;
	DCHECK_PARAM_PTR("w", w);

	cb = EWL_CHECKBUTTON(w);

	if (c)
		cb->checked = 1;
	else
		cb->checked = 0;

	__ewl_checkbutton_update_check(w);

	DLEAVE_FUNCTION;
}

int
ewl_checkbutton_is_checked(Ewl_Widget * w)
{
	Ewl_CheckButton *cb;

	DENTER_FUNCTION;
	DCHECK_PARAM_PTR_RET("w", w, -1);

	cb = EWL_CHECKBUTTON(w);

	DRETURN_INT(cb->checked);
}

void
ewl_checkbutton_set_label_position(Ewl_Widget * w, Ewl_Position p)
{
	Ewl_CheckButton *cb;

	DENTER_FUNCTION;
	DCHECK_PARAM_PTR("w", w);

	cb = EWL_CHECKBUTTON(w);

	cb->label_position = p;

	ewl_widget_configure(w);

	DLEAVE_FUNCTION;
}

void
ewl_checkbutton_init(Ewl_CheckButton * cb, char *label)
{
	Ewl_Button *b;
	Ewl_Widget *w;

	DENTER_FUNCTION;

	b = EWL_BUTTON(cb);
	w = EWL_WIDGET(cb);

	ewl_button_init(b, label);
	ewl_widget_set_appearance(w, "/appearance/button/check");

	ewl_callback_del(w, EWL_CALLBACK_CONFIGURE, __ewl_box_configure);
	ewl_callback_append(w, EWL_CALLBACK_CONFIGURE,
			    __ewl_checkbutton_configure, NULL);

	ewl_callback_append(w, EWL_CALLBACK_MOUSE_DOWN,
			    __ewl_checkbutton_mouse_down, NULL);

	cb->label_position = EWL_POSITION_RIGHT;

	DLEAVE_FUNCTION;
}


void
__ewl_checkbutton_configure(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Button *b;
	Ewl_CheckButton *cb;

	DENTER_FUNCTION;
	DCHECK_PARAM_PTR("w", w);

	b = EWL_BUTTON(w);
	cb = EWL_CHECKBUTTON(w);

	if (b->label_object)
	  {
		  if (MINIMUM_H(b->label_object) > CURRENT_H(w))
		    {
			    REQUEST_Y(b->label_object) = CURRENT_Y(w) + 17;
			    MINIMUM_H(w) = MINIMUM_H(b->label_object);
			    MAXIMUM_H(w) = MINIMUM_H(b->label_object);
		    }
		  else
		    {
			    REQUEST_Y(b->label_object) = CURRENT_Y(w);
			    REQUEST_Y(b->label_object) +=
				    (CURRENT_H(w) / 2) -
				    (CURRENT_H(b->label_object) / 2);
			    MINIMUM_H(w) = 17;
			    MAXIMUM_H(w) = 17;
		    }

		  MINIMUM_W(w) = 17 + CURRENT_W(b->label_object);
		  MAXIMUM_W(w) = 17 + CURRENT_W(b->label_object);

		  if (cb->label_position == EWL_POSITION_LEFT)
			  REQUEST_X(b->label_object) = REQUEST_X(w);
		  else
			  REQUEST_X(b->label_object) = CURRENT_X(w) + 17;

		  ewl_widget_configure(b->label_object);

		  if (w->ebits_object)
		    {
			    if (cb->label_position == EWL_POSITION_LEFT)
				    ebits_move(w->ebits_object,
					       REQUEST_X(w) +
					       CURRENT_W(b->label_object),
					       REQUEST_Y(w));
			    else
				    ebits_move(w->ebits_object, REQUEST_X(w),
					       REQUEST_Y(w));

		    }
	  }
	else
	  {
		  MAXIMUM_W(w) = 17;
		  MAXIMUM_H(w) = 17;
	  }

	DLEAVE_FUNCTION;
}

void
__ewl_checkbutton_mouse_down(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_CheckButton *cb;
	int oc;

	DENTER_FUNCTION;
	DCHECK_PARAM_PTR("w", w);

	cb = EWL_CHECKBUTTON(w);
	oc = cb->checked;

	cb->checked ^= 1;

	__ewl_checkbutton_update_check(w);

	if (oc != cb->checked)
		ewl_callback_call(w, EWL_CALLBACK_VALUE_CHANGED);

	DLEAVE_FUNCTION;
}

void
__ewl_checkbutton_update_check(Ewl_Widget * w)
{
	Ewl_CheckButton *cb;

	DENTER_FUNCTION;
	DCHECK_PARAM_PTR("w", w);

	cb = EWL_CHECKBUTTON(w);

	if (w->ebits_object)
	  {
		  if (cb->checked)
			  ebits_set_named_bit_state(w->ebits_object, "Check",
						    "clicked");
		  else
			  ebits_set_named_bit_state(w->ebits_object, "Check",
						    "normal");
	  }

	DLEAVE_FUNCTION;
}
