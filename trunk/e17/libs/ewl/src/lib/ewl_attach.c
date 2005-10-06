#include <Ewl.h>
#include "ewl_debug.h"
#include "ewl_macros.h"
#include "ewl_private.h"

typedef struct Ewl_Attach_Tooltip Ewl_Attach_Tooltip;
struct Ewl_Attach_Tooltip
{
    Ewl_Widget *embed;

	Ewl_Widget *win;
	Ewl_Widget *box;

	Ewl_Attach *attach;

	Evas_Coord x;
	Evas_Coord y;

	Ecore_Timer *timer;
};

static void ewl_attach_parent_setup(Ewl_Widget *w);
static void ewl_attach_cb_parent_destroy(Ewl_Widget *w, void *ev, void *data);
static void ewl_attach_attach_type_setup(Ewl_Widget *w, Ewl_Attach *attach);

static void ewl_attach_tooltip_attach(Ewl_Widget *w, Ewl_Attach *attach);
static void ewl_attach_tooltip_detach(Ewl_Attach *attach);

static void ewl_attach_cb_tooltip_win_destroy(Ewl_Widget *w, void *ev, void *data);

static void ewl_attach_cb_tooltip_mouse_move(Ewl_Widget *w, void *ev, void *data);
static void ewl_attach_cb_tooltip_mouse_down(Ewl_Widget *w, void *ev, void *data);
static void ewl_attach_cb_tooltip_focus_out(Ewl_Widget *w, void *ev, void *data);
static int ewl_attach_cb_tooltip_timer(void *data);

static Ewl_Attach_Tooltip *ewl_attach_tooltip = NULL;

void
ewl_attach_text_set(Ewl_Widget *w, Ewl_Attach_Type t, const char *data)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	if (!w->attach)
		ewl_attach_parent_setup(w);

	if (data)
	{
		Ewl_Attach *attach;

		attach = ewl_attach_new(t, EWL_ATTACH_DATA_TYPE_TEXT, (void *)data);
		if (attach)
			ewl_attach_list_add(w->attach, w, attach);
	}
	else
		ewl_attach_list_del(w->attach, t);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void
ewl_attach_widget_set(Ewl_Widget *w, Ewl_Attach_Type t, Ewl_Widget *data)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	if (!w->attach)
		ewl_attach_parent_setup(w);

	if (data)
	{
		Ewl_Attach *attach;

		attach = ewl_attach_new(t, EWL_ATTACH_DATA_TYPE_WIDGET, (void *)data);
		if (attach)
			ewl_attach_list_add(w->attach, w, attach);
	}
	else
		ewl_attach_list_del(w->attach, t);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void
ewl_attach_other_set(Ewl_Widget *w, Ewl_Attach_Type t, void *data)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	if (!w->attach)
		ewl_attach_parent_setup(w);

	if (data)
	{
		Ewl_Attach *attach;

		attach = ewl_attach_new(t, EWL_ATTACH_DATA_TYPE_OTHER, data);
		if (attach)
			ewl_attach_list_add(w->attach, w, attach);
	}
	else
		ewl_attach_list_del(w->attach, t);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void *
ewl_attach_get(Ewl_Widget *w, Ewl_Attach_Type t)
{
	Ewl_Attach *attach;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("w", w, NULL);

	if (!w->attach) 
	{
		DRETURN_PTR(NULL, DLEVEL_STABLE);
	}

	attach = ewl_attach_list_get(w->attach, t);
	if (attach)
	{
		DRETURN_PTR(attach->data, DLEVEL_STABLE);
	}
	DRETURN_PTR(NULL, DLEVEL_STABLE);
}

Ewl_Attach_List *
ewl_attach_list_new(void)
{
	Ewl_Attach_List *list;

	DENTER_FUNCTION(DLEVEL_STABLE);

	list = NEW(Ewl_Attach_List, 1);

	DRETURN_PTR(list, DLEVEL_STABLE);
}

void
ewl_attach_list_free(Ewl_Attach_List *list)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("list", list);

	if (list->direct)
		ewl_attach_list_del(list, EWL_ATTACH(list)->type);
	else
	{
		while (list->len)
		{
			if (!list->direct)
				ewl_attach_list_del(list, EWL_ATTACH(list->list[0])->type);
			else
				ewl_attach_list_del(list, EWL_ATTACH(list->list)->type);
		}
	}
	FREE(list);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void
ewl_attach_list_add(Ewl_Attach_List *list, Ewl_Widget *parent, Ewl_Attach *attach)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("list", list);
	DCHECK_PARAM_PTR("attach", attach);

	if (!list->len)
	{
		list->len = 1;
		list->direct = 1;
		list->list = (void *)attach;

		ewl_attach_attach_type_setup(parent, attach);
		DRETURN(DLEVEL_STABLE);
	}
	else if (list->direct)
	{
		Ewl_Attach *tmp;
		list->direct = 0;

		tmp = EWL_ATTACH(list->list);

		/* replace if the same type */
		if (tmp->type == attach->type)
		{
			ewl_attach_free(tmp);
			list->list = (void *)attach;

			ewl_attach_attach_type_setup(parent, attach);
			DRETURN(DLEVEL_STABLE);
		}

		list->list = malloc(sizeof(void *));
		list->list[0] = tmp;
	}
	else
	{
		int i;
		Ewl_Attach *tmp;

		/* replace if in list already */
		for (i = 0; i < list->len; i++)
		{
			tmp = EWL_ATTACH(list->list[i]);
			if (tmp->type == attach->type)
			{
				ewl_attach_free(tmp);
				list->list[i] = attach;

				ewl_attach_attach_type_setup(parent, attach);
				DRETURN(DLEVEL_STABLE);
			}
		}
	}

	list->len ++;
	list->list = realloc(list->list, list->len * sizeof(void *));

	ewl_attach_attach_type_setup(parent, attach);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ewl_attach_attach_type_setup(Ewl_Widget *w, Ewl_Attach *attach)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_PARAM_PTR("attach", attach);

	switch (attach->type)
	{
		case EWL_ATTACH_TYPE_TOOLTIP:
			ewl_attach_tooltip_attach(w, attach);
			break;

		case EWL_ATTACH_TYPE_COLOR:
		case EWL_ATTACH_TYPE_NAME:
		default:
			break;
	}

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void
ewl_attach_list_del(Ewl_Attach_List *list, Ewl_Attach_Type type)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("list", list);

	if (!list->len)
	{
		DRETURN(DLEVEL_STABLE);
	}
	else if (list->direct)
	{
		Ewl_Attach *tmp;

		tmp = EWL_ATTACH(list->list);
		if (tmp->type == type)
		{
			ewl_attach_free(tmp);
			list->len --;
			list->direct = 0;
			list->list = NULL;
		}
		DRETURN(DLEVEL_STABLE);
	}
	else
	{
		int i;
		Ewl_Attach *tmp;

		for (i = 0; i < list->len; i++)
		{
			tmp = EWL_ATTACH(list->list[i]);

			if (tmp->type == type)
			{
				ewl_attach_free(tmp);
				list->len --;

				/* if not the last entry */
				if (i != list->len)
					memmove(list->list + i, list->list + i + 1, list->len * sizeof(void *));

				list->list = realloc(list->list, list->len * sizeof(void *));
			}
		}
	}

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void *
ewl_attach_list_get(Ewl_Attach_List *list, Ewl_Attach_Type type)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("list", list, NULL);

	if (!list->len)
	{
		DRETURN_PTR(NULL, DLEVEL_STABLE);
	}
	else if (list->direct)
	{
		Ewl_Attach *tmp;

		tmp = EWL_ATTACH(list->list);
		if (tmp->type == type)
		{
			DRETURN_PTR(tmp, DLEVEL_STABLE);
		}
		DRETURN_PTR(NULL, DLEVEL_STABLE);
	}
	else
	{
		int i;
		Ewl_Attach *tmp;

		for (i = 0; i < list->len; i++)
		{
			tmp = EWL_ATTACH(list->list[i]);

			if (tmp->type == type)
			{
				DRETURN_PTR(tmp, DLEVEL_STABLE);
			}
		}
	}
	DRETURN_PTR(NULL, DLEVEL_STABLE);
}

Ewl_Attach *
ewl_attach_new(Ewl_Attach_Type t, Ewl_Attach_Data_Type dt, void *data)
{
	Ewl_Attach *attach;

	DENTER_FUNCTION(DLEVEL_STABLE);

	attach = NEW(Ewl_Attach, 1);
	if (!attach)
	{
		DRETURN_PTR(NULL, DLEVEL_STABLE);
	}

	if (!ewl_attach_init(attach, t, dt, data))
	{
		FREE(attach);
	}

	DRETURN_PTR(attach, DLEVEL_STABLE);
}

int
ewl_attach_init(Ewl_Attach *attach, Ewl_Attach_Type t, Ewl_Attach_Data_Type dt, void *data)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("attach", attach, FALSE);

	attach->type = t;

	if (dt == EWL_ATTACH_DATA_TYPE_TEXT)
		attach->data = strdup(data);
	else
		attach->data = data;
	attach->data_type = dt;

	DRETURN_INT(TRUE, DLEVEL_STABLE);
}

void
ewl_attach_free(Ewl_Attach *attach)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("attach", attach);

	/* XXX should we clean up _WIDGET in here? */

	if ((attach->type == EWL_ATTACH_DATA_TYPE_TEXT)
			|| (attach->type == EWL_ATTACH_DATA_TYPE_OTHER))
		IF_FREE(attach->data);

	IF_FREE(attach);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ewl_attach_parent_setup(Ewl_Widget *w)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	w->attach = ewl_attach_list_new();
	ewl_callback_append(w, EWL_CALLBACK_DESTROY, ewl_attach_cb_parent_destroy, NULL);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ewl_attach_cb_parent_destroy(Ewl_Widget *w, void *ev, void *data)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	if (w->attach)
		ewl_attach_list_free(w->attach);

	w->attach = NULL;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ewl_attach_tooltip_attach(Ewl_Widget *w, Ewl_Attach *attach)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("attach", attach);

	if (!ewl_attach_tooltip)
		ewl_attach_tooltip = NEW(Ewl_Attach_Tooltip, 1);

	ewl_attach_tooltip->attach = attach;

	ewl_callback_append(w, EWL_CALLBACK_MOUSE_MOVE,
				ewl_attach_cb_tooltip_mouse_move, attach);
	ewl_callback_append(w, EWL_CALLBACK_MOUSE_DOWN,
				ewl_attach_cb_tooltip_mouse_down, attach);
	ewl_callback_append(w, EWL_CALLBACK_FOCUS_OUT,
				ewl_attach_cb_tooltip_focus_out, attach);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ewl_attach_tooltip_detach(Ewl_Attach *attach)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("attach", attach);

	/* make sure the display attach is our attach */
	if (ewl_attach_tooltip->attach != attach)
	{
		DRETURN(DLEVEL_STABLE);
	}

	if (ewl_attach_tooltip)
	{
		if (ewl_attach_tooltip->timer)
			ecore_timer_del(ewl_attach_tooltip->timer);

		ewl_attach_tooltip->timer = NULL;
		ewl_attach_tooltip->x = 0;
		ewl_attach_tooltip->y = 0;

		/* hide window if needed */
		if (ewl_attach_tooltip->win && (VISIBLE(ewl_attach_tooltip->win)))
			ewl_widget_hide(ewl_attach_tooltip->win);

		/* cleanup the display window */
		if (ewl_attach_tooltip->box)
		{
			if (attach->data_type == EWL_ATTACH_DATA_TYPE_TEXT)
			{
				ewl_widget_destroy(ewl_attach_tooltip->box);
				ewl_attach_tooltip->box = NULL;
			}
			else
				ewl_container_child_remove(EWL_CONTAINER(ewl_attach_tooltip->box), 
									EWL_WIDGET(attach->data));
		}
		ewl_attach_tooltip->attach = NULL;
	}

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ewl_attach_cb_tooltip_mouse_move(Ewl_Widget *w, void *ev, void *data)
{
	Ewl_Attach *attach;
	Ewl_Embed *emb;
	Ewl_Event_Mouse_Move *e;
	int x, y;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("ev", ev);
	DCHECK_PARAM_PTR("data", data);

	e = ev;
	attach = data;

	if (!ewl_attach_tooltip)
		ewl_attach_tooltip = NEW(Ewl_Attach_Tooltip, 1);

	ewl_attach_tooltip_detach(attach);

	emb = ewl_embed_widget_find(w);
	ewl_window_position_get(EWL_WINDOW(emb), &x, &y);

	/* XXX the 15 should come from the theme (offset off of the cursor) */
	/* XXX the 1.0 shoudl come from the theme (delay before firing timer) */
	ewl_attach_tooltip->attach = attach;
	ewl_attach_tooltip->x = e->x + 15;
	ewl_attach_tooltip->y = e->y + 15;
#if 0 
	ewl_attach_tooltip->x = x + CURRENT_X(w) + e->x + 15;
	ewl_attach_tooltip->y = y + CURRENT_Y(w) + e->y + 15;
#endif
	ewl_attach_tooltip->timer = ecore_timer_add(1.0, ewl_attach_cb_tooltip_timer, w);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void ewl_attach_cb_tooltip_mouse_down(Ewl_Widget *w __UNUSED__, 
						void *ev __UNUSED__, void *data)
{
	Ewl_Attach *attach;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("data", data);

	attach = data;
	ewl_attach_tooltip_detach(attach);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ewl_attach_cb_tooltip_focus_out(Ewl_Widget *w __UNUSED__, 
						void *ev __UNUSED__, void *data)
{
	Ewl_Attach *attach;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("data", data);

	attach = data;
	ewl_attach_tooltip_detach(attach);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static int
ewl_attach_cb_tooltip_timer(void *data)
{
	Ewl_Widget *w;
	Ewl_Embed *emb;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("data", data, FALSE);

	w = data;
	emb = ewl_embed_widget_find(w);

	/* XXX i think there is a race condition with the detach code? 
	 * ie, we get the focus_out (or something) stuck in the event queue
	 * and then we get the timer trigger in the event queue. the focus
	 * out is processed, then the trigger is processed ... blam ->attach
	 * is null */
	if (!(ewl_attach_tooltip->attach))
	{
		DRETURN_INT(FALSE, DLEVEL_STABLE);
	}

	if (!(ewl_attach_tooltip->win))
	{
		ewl_attach_tooltip->embed = EWL_WIDGET(emb);
#if 0
		ewl_attach_tooltip->win = ewl_window_new();
		ewl_window_title_set(EWL_WINDOW(ewl_attach_tooltip->win), "Tooltip");
		ewl_window_name_set(EWL_WINDOW(ewl_attach_tooltip->win), "Tooltip");
		ewl_window_class_set(EWL_WINDOW(ewl_attach_tooltip->win), "Tooltip");
		ewl_window_transient_for(EWL_WINDOW(emb), EWL_WINDOW(ewl_attach_tooltip->win));
		ewl_window_borderless_set(EWL_WINDOW(ewl_attach_tooltip->win));
		ewl_window_raise(EWL_WINDOW(ewl_attach_tooltip->win));
#endif

		/* XXX this should really be in it's own window */
		ewl_attach_tooltip->win = ewl_hbox_new();
		ewl_container_child_append(EWL_CONTAINER(emb), ewl_attach_tooltip->win);
		ewl_widget_layer_set(ewl_attach_tooltip->win, 1000);

		ewl_callback_append(ewl_attach_tooltip->win, EWL_CALLBACK_DESTROY,
						ewl_attach_cb_tooltip_win_destroy, NULL);
		ewl_object_fill_policy_set(EWL_OBJECT(ewl_attach_tooltip->win), EWL_FLAG_FILL_NONE);

		ewl_widget_appearance_set(ewl_attach_tooltip->win, "tooltip");
		ewl_widget_inherit(ewl_attach_tooltip->win, "tooltip");
	}
	else
	{
		if (ewl_attach_tooltip->embed)
			ewl_container_child_remove(EWL_CONTAINER(ewl_attach_tooltip->embed), 
									ewl_attach_tooltip->win);

		ewl_attach_tooltip->embed = EWL_WIDGET(emb);
		ewl_container_child_append(EWL_CONTAINER(emb), ewl_attach_tooltip->win);
	}

	if (!(ewl_attach_tooltip->box))
	{
		ewl_attach_tooltip->box = ewl_hbox_new();
		ewl_container_child_append(EWL_CONTAINER(ewl_attach_tooltip->win), 
							ewl_attach_tooltip->box);
	}

	if (ewl_attach_tooltip->attach->data_type == EWL_ATTACH_DATA_TYPE_WIDGET)
	{
		ewl_container_child_append(EWL_CONTAINER(ewl_attach_tooltip->box), 
						EWL_WIDGET(ewl_attach_tooltip->attach->data));
		ewl_widget_show(EWL_WIDGET(ewl_attach_tooltip->attach->data));
	}
	else
	{
		Ewl_Widget *o;

		o = ewl_text_new();
		ewl_text_text_set(EWL_TEXT(o), ewl_attach_tooltip->attach->data);
		ewl_container_child_append(EWL_CONTAINER(ewl_attach_tooltip->box), o);
		ewl_widget_show(o);
	}

	ewl_object_position_request(EWL_OBJECT(ewl_attach_tooltip->win), 
					ewl_attach_tooltip->x, ewl_attach_tooltip->y);

	ewl_widget_show(ewl_attach_tooltip->win);
	ewl_widget_show(ewl_attach_tooltip->box);

	/* we are returning false so that will shutdown the timer, just need
	 * to make sure we don't try to del it a second time so set it NULL */
	ewl_attach_tooltip->timer = NULL;

	DRETURN_INT(FALSE, DLEVEL_STABLE);
}

static void
ewl_attach_cb_tooltip_win_destroy(Ewl_Widget *w, void *ev, void *data)
{
	DENTER_FUNCTION(DLEVEL_STABLE);

	ewl_attach_tooltip->embed = NULL;
	ewl_attach_tooltip->win = NULL;
	ewl_attach_tooltip->box = NULL;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

