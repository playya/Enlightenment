
#include <Ewl.h>

static Ewl_Widget		* last_selected;
static Ewl_Widget		* last_key;
static Ewl_Widget		* last_focused;
static Ewl_Widget		* dnd_widget;

static void				ewl_ev_window_expose(Eevent * _ev);
static void				ewl_ev_window_configure(Eevent * _ev);
static void				ewl_ev_window_delete(Eevent * _ev);
static void				ewl_ev_key_down(Eevent * _ev);
static void				ewl_ev_key_up(Eevent * _ev);
static void				ewl_ev_mouse_down(Eevent * _ev);
static void				ewl_ev_mouse_up(Eevent * _ev);
static void				ewl_ev_mouse_move(Eevent * _ev);

void
ewl_ev_init(void)
{
	e_event_filter_handler_add(EV_WINDOW_EXPOSE,	ewl_ev_window_expose);
	e_event_filter_handler_add(EV_WINDOW_CONFIGURE,	ewl_ev_window_configure);
	e_event_filter_handler_add(EV_WINDOW_DELETE,	ewl_ev_window_delete);
	e_event_filter_handler_add(EV_KEY_DOWN,			ewl_ev_key_down);
	e_event_filter_handler_add(EV_KEY_UP,			ewl_ev_key_up);
	e_event_filter_handler_add(EV_MOUSE_DOWN,		ewl_ev_mouse_down);
	e_event_filter_handler_add(EV_MOUSE_UP,			ewl_ev_mouse_up);
	e_event_filter_handler_add(EV_MOUSE_MOVE,		ewl_ev_mouse_move);
}

static void
ewl_ev_window_expose(Eevent * _ev)
{
	Ev_Window_Expose * ev;
	Ewl_Window * window;

	ev = _ev->event;

	window = ewl_window_find_window_by_evas_window(ev->win);
	if (window)
	  {
		evas_update_rect(EWL_WIDGET(window)->evas, ev->x, ev->y, ev->w, ev->h);
	  }
}

static void
ewl_ev_window_configure(Eevent * _ev)
{
	Ev_Window_Configure * ev;
	Ewl_Window * window;

	ev = _ev->event;

	window = ewl_window_find_window(ev->win);
	if (window)
	  {
		EWL_OBJECT(window)->request.x = ev->x;
		EWL_OBJECT(window)->request.y = ev->y;
		EWL_OBJECT(window)->request.w = ev->w;
		EWL_OBJECT(window)->request.h = ev->h;
		ewl_callback_call(EWL_WIDGET(window), EWL_CALLBACK_CONFIGURE);
	  }
}

static void
ewl_ev_window_delete(Eevent * _ev)
{
	Ev_Window_Delete * ev;
	Ewl_Window * window;

	ev = _ev->event;

	window = ewl_window_find_window(ev->win);
	if (window)
	  {
		ewl_callback_call(EWL_WIDGET(window), EWL_CALLBACK_DELETE_WINDOW);
	  }
}

static void
ewl_ev_key_down(Eevent * _ev)
{
	Ewl_Window * window;
	Ev_Key_Down * ev;

	ev = _ev->event;

	window = ewl_window_find_window_by_evas_window(ev->win);
	if (window)
	  {
		if (last_selected)
			ewl_callback_call_with_data(last_selected,EWL_CALLBACK_KEY_DOWN,ev);
			last_key = last_selected;
	  }
}

static void
ewl_ev_key_up(Eevent * _ev)
{
	Ewl_Window * window;
	Ev_Key_Up * ev;

	ev = _ev->event;

	window = ewl_window_find_window_by_evas_window(ev->win);
	if (window)
	  {
		if (last_key)
			ewl_callback_call_with_data(last_key, EWL_CALLBACK_KEY_UP, ev);
	  }
}

static void
ewl_ev_mouse_down(Eevent * _ev)
{
	Ewl_Widget * widget;
	Ewl_Window * window;
	Ev_Mouse_Down * ev;

	ev = _ev->event;

	window = ewl_window_find_window_by_evas_window(ev->win);
	if (window)
	  {
		widget = ewl_container_get_child_at_recursive(EWL_WIDGET(window),
								ev->x, ev->y);

		if (widget != last_selected)
		  {
			if (last_selected)
			  {
				last_selected->state = last_selected->state&!EWL_STATE_SELECTED;
				ewl_callback_call(last_selected, EWL_CALLBACK_UNSELECT);
			  }
			if (widget)
			  {
				widget->state = widget->state | EWL_STATE_SELECTED;
				ewl_callback_call(widget, EWL_CALLBACK_SELECT);
			  }
		  }

		if (widget)
			ewl_callback_call_with_data(widget, EWL_CALLBACK_MOUSE_DOWN, ev);

		last_selected = widget;
	  }
}

static void
ewl_ev_mouse_up(Eevent * _ev)
{
	Ewl_Window * window;
	Ev_Mouse_Up * ev;

	ev = _ev->event;

	window = ewl_window_find_window_by_evas_window(ev->win);
	if (window)
		if (last_selected)
			ewl_callback_call_with_data(last_selected,
										EWL_CALLBACK_MOUSE_UP, ev);
}

static void
ewl_ev_mouse_move(Eevent * _ev)
{
	Ewl_Widget * widget;
	Ewl_Window * window;
	Ev_Mouse_Move * ev;

	ev = _ev->event;

	window = ewl_window_find_window_by_evas_window(ev->win);
	if (window)
	  {
		widget = ewl_container_get_child_at_recursive(EWL_WIDGET(window),
										ev->x, ev->y);

		if (widget)
		  {
			widget->state = widget->state | EWL_STATE_HILITED;
			ewl_callback_call(widget, EWL_CALLBACK_FOCUS_IN);
			DPRINT(8, "Focus In on %p", widget);
			ewl_callback_call_with_data(widget, EWL_CALLBACK_MOUSE_MOVE,ev);
		  }
		if (last_focused != widget && last_focused)
		  {
			last_focused->state = widget->state & !EWL_STATE_HILITED;
			ewl_callback_call(last_focused, EWL_CALLBACK_FOCUS_OUT);
			DPRINT(8, "Focus Out off %p", last_focused);
		  }

		if (last_focused && last_focused->state & EWL_STATE_DND)
			dnd_widget = last_focused;

		if (dnd_widget && dnd_widget->state & EWL_STATE_DND)
			ewl_callback_call_with_data(dnd_widget,
						EWL_CALLBACK_MOUSE_MOVE, ev);
		else
			dnd_widget = NULL;

		last_focused = widget;
	  }
}

