
#include <Ewl.h>


struct _ewl_list_selection
{
	Ebits_Object *ebits_object;
	int row;
};

typedef struct _ewl_list_selection Ewl_List_Selection;

static void __ewl_list_init(Ewl_List * list);
static void __ewl_list_realize(Ewl_Widget * widget, void *event_data,
			       void *user_data);
static void __ewl_list_show(Ewl_Widget * widget, void *event_data,
			    void *user_data);
static void __ewl_list_hide(Ewl_Widget * widget, void *event_data,
			    void *user_data);
static void __ewl_list_destroy(Ewl_Widget * widget, void *event_data,
			       void *user_data);
static void __ewl_list_configure(Ewl_Widget * widget, void *event_data,
				 void *user_data);
static void __ewl_list_key_down(Ewl_Widget * widget, void *event_data,
				void *user_data);

static void __ewl_list_select_row(Ewl_Widget * l, int row);
static void __ewl_list_unselect_row(Ewl_Widget * l, int row);
static void __ewl_list_move_up_selection(Ewl_Widget * l, int multiple);
static void __ewl_list_move_down_selection(Ewl_Widget * l, int multiple);
static int __ewl_list_check_selected(Ewl_Widget * l, int row);
static void __ewl_list_unselect_all(Ewl_Widget * l);
static void __ewl_list_selection_hilit(Ewl_Widget * l, int row);
static void __ewl_list_selection_unhilit(Ewl_Widget * l, int row);


Ewl_Widget *
ewl_list_new(int columns)
{
	Ewl_List *list;
	Ewl_Widget *table;

	list = NEW(Ewl_List, 1);

	__ewl_list_init(list);

	list->selections = ewd_list_new();

	table = ewl_table_new_all(FALSE, columns, 1, 0, 3);
	ewl_container_append_child(EWL_CONTAINER(list), table);
	ewl_callback_append(table, EWL_CALLBACK_KEY_DOWN,
			    __ewl_list_key_down, table);
	ewl_table_set_alignment(table, EWL_ALIGNMENT_LEFT);

	list->table = table;

	return EWL_WIDGET(list);
}

Ewl_Widget *
ewl_list_new_all(int columns, char *titles[])
{
	Ewl_Widget *widget;

	widget = ewl_list_new(columns);

	EWL_LIST(widget)->titles = titles;

	return widget;
}

void
ewl_list_append_text(Ewl_Widget * l, char *text[])
{
	Ewl_Widget *table;
	int i;

	CHECK_PARAM_POINTER("l", l);

	table = EWL_CONTAINER(l)->children->first->data;

	ewl_table_resize(table, EWL_TABLE(table)->rows + 1,
			 EWL_TABLE(table)->columns);

	{
		Ewl_Widget *text_widgets[EWL_TABLE(table)->columns];

		for (i = 0; i < EWL_TABLE(table)->columns; i++)
		  {
			  text_widgets[i] = ewl_text_new();
			  ewl_text_set_text(text_widgets[i], text[i]);
			  ewl_text_set_font_size(text_widgets[i], 8);
			  ewl_table_attach(table, text_widgets[i],
					   EWL_ALIGNMENT_LEFT,
					   EWL_FILL_POLICY_NORMAL, i + 1,
					   i + 1, EWL_TABLE(table)->rows,
					   EWL_TABLE(table)->rows);
			  ewl_widget_show(text_widgets[i]);
		  }
	}
}

void
ewl_list_prepend_text(Ewl_Widget * l, char *text[])
{
}

void
ewl_list_insert_text(Ewl_Widget * widget, char *text[], int row)
{

}

void
ewl_list_append_widgets(Ewl_Widget * widget, Ewl_Widget * widgets[])
{

}

void
ewl_list_preppend_widgets(Ewl_Widget * widget, Ewl_Widget * widgets[])
{

}

void
ewl_list_insert_widgets(Ewl_Widget * widget, Ewl_Widget * widgets[], int row)
{

}

void
ewl_list_set_titles(Ewl_Widget * widget, char *titles[])
{
	Ewl_Widget *table;

	CHECK_PARAM_POINTER("widget", widget);

	table = EWL_CONTAINER(widget)->children->first->data;

	{
		Ewl_Widget *button[EWL_TABLE(table)->columns];
		int i;

		for (i = 0; i < EWL_TABLE(table)->columns; i++)
		  {
			  button[i] = ewl_button_new(titles[i]);
			  ewl_callback_append(button[i],
					      EWL_CALLBACK_KEY_DOWN,
					      __ewl_list_key_down, widget);
			  ewl_table_attach(table, button[i],
					   EWL_ALIGNMENT_LEFT,
					   EWL_FILL_POLICY_FILL, i + 1,
					   i + 1, 1, 1);
			  EWL_OBJECT(button[i])->custom.w = 50;
			  ewl_widget_show(button[i]);
		  }
	}
}

void
ewl_list_select_row(Ewl_Widget * list, int row)
{
	CHECK_PARAM_POINTER("list", list);
}

static void
__ewl_list_init(Ewl_List * list)
{
	CHECK_PARAM_POINTER("list", list);

	/*
	 * Initialize the list widget
	 */
	memset(list, 0, sizeof(Ewl_List));
	ewl_container_init(EWL_CONTAINER(list), EWL_WIDGET_LIST, 200, 200,
			   2048, 2048);

	ewl_callback_append(EWL_WIDGET(list),
			    EWL_CALLBACK_REALIZE, __ewl_list_realize, NULL);
	ewl_callback_append(EWL_WIDGET(list), EWL_CALLBACK_SHOW,
			    __ewl_list_show, NULL);
	ewl_callback_append(EWL_WIDGET(list), EWL_CALLBACK_HIDE,
			    __ewl_list_hide, NULL);
	ewl_callback_append(EWL_WIDGET(list), EWL_CALLBACK_DESTROY,
			    __ewl_list_destroy, NULL);
	ewl_callback_append(EWL_WIDGET(list), EWL_CALLBACK_CONFIGURE,
			    __ewl_list_configure, NULL);

}

static void
__ewl_list_realize(Ewl_Widget * widget, void *event_data, void *user_data)
{
	CHECK_PARAM_POINTER("widget", widget);
	ewl_fx_clip_box_create(widget);

	evas_set_clip(widget->evas, widget->fx_clip_box,
		      EWL_CONTAINER(widget->parent)->clip_box);

	EWL_CONTAINER(widget)->clip_box = widget->fx_clip_box;

	evas_set_color(widget->evas, widget->fx_clip_box, 255, 255, 255, 255);

	ewl_widget_realize(EWL_CONTAINER(widget)->children->first->data);
}

static void
__ewl_list_show(Ewl_Widget * widget, void *event_data, void *user_data)
{
	CHECK_PARAM_POINTER("widget", widget);

	ewl_widget_show(EWL_CONTAINER(widget)->children->first->data);

	evas_show(widget->evas, widget->fx_clip_box);

	if (EWL_LIST(widget)->titles)
		ewl_list_set_titles(widget, EWL_LIST(widget)->titles);
}

static void
__ewl_list_hide(Ewl_Widget * widget, void *event_data, void *user_data)
{
	CHECK_PARAM_POINTER("widget", widget);

	evas_hide(widget->evas, widget->fx_clip_box);
}

static void
__ewl_list_destroy(Ewl_Widget * widget, void *event_data, void *user_data)
{
	CHECK_PARAM_POINTER("widget", widget);

	ewl_widget_destroy(EWL_LIST(widget)->table);

	ewd_list_destroy(EWL_CONTAINER(widget)->children);

	evas_hide(widget->evas, widget->fx_clip_box);
	evas_unset_clip(widget->evas, widget->fx_clip_box);
	evas_del_object(widget->evas, widget->fx_clip_box);

	ewl_theme_deinit_widget(widget);

	ewl_callback_clear(widget);

	FREE(widget);
}

static void
__ewl_list_configure(Ewl_Widget * widget, void *event_data, void *user_data)
{
	Ewl_Widget *table;
	Ewl_Widget *title;
	int i, w;

	CHECK_PARAM_POINTER("widget", widget);

	EWL_OBJECT(widget)->current.x = EWL_OBJECT(widget)->request.x;
	EWL_OBJECT(widget)->current.y = EWL_OBJECT(widget)->request.y;
	EWL_OBJECT(widget)->current.w = EWL_OBJECT(widget)->request.w;
	EWL_OBJECT(widget)->current.h = EWL_OBJECT(widget)->request.h;

	ewd_list_goto_first(EWL_CONTAINER(widget)->children);
	table = ewd_list_current(EWL_CONTAINER(widget)->children);

	EWL_OBJECT(table)->request.x = EWL_OBJECT(widget)->current.x;
	EWL_OBJECT(table)->request.y = EWL_OBJECT(widget)->current.y;
	EWL_OBJECT(table)->request.w = EWL_OBJECT(widget)->current.w;
	EWL_OBJECT(table)->request.h = EWL_OBJECT(widget)->current.h;

	ewl_widget_configure(table);

	for (i = 0; i < EWL_TABLE(table)->columns + 1; i++)
	  {
		  title = ewl_table_get_child(table, 1, i + 1);
		  if (title)
		    {
			    ewl_table_get_column_width(table, i + 1, &w);
			    EWL_OBJECT(title)->custom.w = w;
			    ewl_widget_configure(title);
		    }
	  }

	ewl_fx_clip_box_resize(widget);
}

static void
__ewl_list_key_down(Ewl_Widget * widget, void *event_data, void *user_data)
{
	Ev_Key_Down *ev;
	int multiple = 0;

	CHECK_PARAM_POINTER("widget", widget);

	ev = (Ev_Key_Down *) event_data;

	if (ev->mods & EV_KEY_MODIFIER_SHIFT)
		multiple = TRUE;

	if (!strcmp(ev->key, "Up"))
	  {
		  __ewl_list_move_up_selection(widget, multiple);
	  }
	else if (!strcmp(ev->key, "Down"))
	  {
		  __ewl_list_move_down_selection(widget, multiple);
	  }
}

static void
__ewl_list_select_row(Ewl_Widget * l, int row)
{
	Ewl_List_Selection *sel;
	Ewl_Table *t;
	char *image;
	int x, y, w, h;

	CHECK_PARAM_POINTER("l", l);

	ewd_list_goto_first(EWL_CONTAINER(l)->children);
	t = ewd_list_current(EWL_CONTAINER(l)->children);

	if (row > t->rows)
		return;

	sel = NEW(Ewl_List_Selection, 1);
	sel->row = row;

	image = ewl_theme_image_get(l, "/appearance/list/default/selection");

	sel->ebits_object = ebits_load(image);
	ebits_add_to_evas(sel->ebits_object, l->evas);
	ebits_set_layer(sel->ebits_object, EWL_OBJECT(t)->layer + 5);
	ebits_set_clip(sel->ebits_object, l->fx_clip_box);
	ewl_table_get_row_geometry(EWL_WIDGET(t), row + 1, &x, &y, &w, &h);
	ebits_move(sel->ebits_object, x, y - 1);
	ebits_resize(sel->ebits_object, w, h + 1);
	ebits_set_color_class(sel->ebits_object, "Menu BG", 100, 200, 255,
			      255);
	ebits_show(sel->ebits_object);

	ewd_list_append(EWL_LIST(l)->selections, sel);
}

static void
__ewl_list_unselect_row(Ewl_Widget * l, int row)
{
	Ewl_List_Selection *sel;

	CHECK_PARAM_POINTER("l", l);

	if (row > EWL_TABLE(EWL_LIST(l)->table)->rows || row < 1)
		return;

	ewd_list_goto_first(EWL_LIST(l)->selections);

	while ((sel = ewd_list_next(EWL_LIST(l)->selections)) != NULL)
	  {
		  if (sel->row == row)
		    {
			    ebits_hide(sel->ebits_object);
			    ebits_unset_clip(sel->ebits_object);
			    ebits_free(sel->ebits_object);
			    ewd_list_remove(EWL_LIST(l)->selections);
			    FREE(sel);
			    break;
		    }
	  }
}

static void
__ewl_list_move_up_selection(Ewl_Widget * l, int multiple)
{
	CHECK_PARAM_POINTER("l", l);

	if (EWL_LIST(l)->current_selected - 1 <= 0)
		return;

	if (!multiple)
		__ewl_list_unselect_all(l);

	if (__ewl_list_check_selected(l, EWL_LIST(l)->current_selected - 1))
	  {
		  __ewl_list_selection_unhilit(l,
					       EWL_LIST(l)->current_selected);
		  EWL_LIST(l)->current_selected--;
		  __ewl_list_selection_hilit(l,
					     EWL_LIST(l)->current_selected);
		  return;
	  }

	__ewl_list_selection_unhilit(l, EWL_LIST(l)->current_selected);
	__ewl_list_select_row(l, --EWL_LIST(l)->current_selected);
	__ewl_list_selection_hilit(l, EWL_LIST(l)->current_selected);
}

static void
__ewl_list_move_down_selection(Ewl_Widget * l, int multiple)
{
	CHECK_PARAM_POINTER("l", l);

	if (EWL_LIST(l)->current_selected + 1 >=
	    EWL_TABLE(EWL_LIST(l)->table)->rows)
		return;

	if (!multiple)
		__ewl_list_unselect_all(l);

	if (__ewl_list_check_selected(l, EWL_LIST(l)->current_selected + 1))
	  {
		  __ewl_list_selection_unhilit(l,
					       EWL_LIST(l)->current_selected);
		  EWL_LIST(l)->current_selected++;
		  __ewl_list_selection_hilit(l,
					     EWL_LIST(l)->current_selected);
		  return;
	  }

	__ewl_list_selection_unhilit(l, EWL_LIST(l)->current_selected);
	__ewl_list_select_row(l, ++EWL_LIST(l)->current_selected);
	__ewl_list_selection_hilit(l, EWL_LIST(l)->current_selected);
}

static int
__ewl_list_check_selected(Ewl_Widget * l, int row)
{
	Ewl_List_Selection *sel;

	CHECK_PARAM_POINTER_RETURN("l", l, FALSE);

	ewd_list_goto_first(EWL_LIST(l)->selections);

	while ((sel = ewd_list_next(EWL_LIST(l)->selections)) != NULL)
	  {
		  if (sel->row == row)
			  return TRUE;
	  }

	return FALSE;
}

static void
__ewl_list_unselect_all(Ewl_Widget * l)
{
	Ewl_List_Selection *sel;

	CHECK_PARAM_POINTER("l", l);

	if (!EWL_LIST(l)->selections
	    || ewd_list_is_empty(EWL_LIST(l)->selections))
		return;

	ewd_list_goto_first(EWL_LIST(l)->selections);

	while ((sel = ewd_list_remove_last(EWL_LIST(l)->selections)) != NULL)
	  {
		  ebits_hide(sel->ebits_object);
		  ebits_unset_clip(sel->ebits_object);
		  ebits_free(sel->ebits_object);
		  FREE(sel);
	  }
}

static void
__ewl_list_selection_hilit(Ewl_Widget * l, int row)
{
	Ewl_List_Selection *sel;

	CHECK_PARAM_POINTER("l", l);

	ewd_list_goto_first(EWL_LIST(l)->selections);

	while ((sel = ewd_list_next(EWL_LIST(l)->selections)) != NULL)
	  {
		  if (sel->row == row)
			  ebits_set_color_class(sel->ebits_object, "Menu BG",
						100, 200, 255, 255);
	  }
}

static void
__ewl_list_selection_unhilit(Ewl_Widget * l, int row)
{
	Ewl_List_Selection *sel;

	CHECK_PARAM_POINTER("l", l);

	ewd_list_goto_first(EWL_LIST(l)->selections);

	while ((sel = ewd_list_next(EWL_LIST(l)->selections)) != NULL)
	  {
		  if (sel->row == row)
			  ebits_set_color_class(sel->ebits_object, "Menu BG",
						255, 255, 255, 255);
	  }
}
