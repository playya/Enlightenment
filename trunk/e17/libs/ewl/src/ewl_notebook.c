#include <Ewl.h>

/**
 * ewl_notebook_new - create a new notebook
 * 
 * Returns a newly allocated notebook on success, NULL on failure.
 */
Ewl_Widget     *ewl_notebook_new(void)
{
	Ewl_Notebook   *n;

	DENTER_FUNCTION(DLEVEL_STABLE);

	n = NEW(Ewl_Notebook, 1);
	if (!n)
		DRETURN_PTR(NULL, DLEVEL_STABLE);

	ewl_notebook_init(n);

	DRETURN_PTR(EWL_WIDGET(n), DLEVEL_TESTING);
}

/**
 * ewl_notebook_init - initialize a notebook to default values and callbacks
 * @n: the notebook to initialize
 *
 * Returns no value. Sets the fields and callbacks of @n to their defaults.
 */
int ewl_notebook_init(Ewl_Notebook * n)
{
	Ewl_Widget     *w;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("n", n, FALSE);

	w = EWL_WIDGET(n);

	/*
	 * Initialize the container portion of the notebook and set the fill
	 * policy to fill the area available.
	 */
	if (!ewl_container_init(EWL_CONTAINER(w), "tnotebook"))
		DRETURN_INT(FALSE, DLEVEL_STABLE);

	ewl_container_show_notify_set(EWL_CONTAINER(w),
				  ewl_notebook_child_show_cb);
	ewl_container_resize_notify_set(EWL_CONTAINER(w),
				    ewl_notebook_child_resize_cb);
	ewl_container_hide_notify_set(EWL_CONTAINER(w),
				  ewl_notebook_child_show_cb);

	ewl_object_fill_policy_set(EWL_OBJECT(w), EWL_FLAG_FILL_FILL);

	/*
	 * Set the default position of the tabs.
	 */
	n->flags |= EWL_POSITION_TOP;

	/*
	 * Create the box to hold tabs and make the box fill the area of the
	 * notebook.
	 */
	n->tab_box = ewl_hbox_new();
	if (n->tab_box) {
		ewl_widget_internal_set(n->tab_box, TRUE);
		ewl_object_fill_policy_set(EWL_OBJECT(n->tab_box),
					   EWL_FLAG_FILL_NONE);
		ewl_object_alignment_set(EWL_OBJECT(n->tab_box),
					 EWL_FLAG_ALIGN_LEFT |
					 EWL_FLAG_ALIGN_TOP);
		ewl_container_child_append(EWL_CONTAINER(n), n->tab_box);
		ewl_widget_show(n->tab_box);
	}
	else
		DRETURN_INT(FALSE, DLEVEL_STABLE);

	/*
	 * Attach the necessary callbacks for the notebook
	 */
	ewl_callback_append(w, EWL_CALLBACK_CONFIGURE,
			    ewl_notebook_configure_top_cb, NULL);

	DRETURN_INT(TRUE, DLEVEL_STABLE);
}

/**
 * ewl_notebook_page_append - append a page to the notebook
 * @n: the notebook to append the page
 * @t: the tab of the new page added
 * @p: the contents of the page added
 *
 * Returns no value. Appends a page to the list of available pages that will
 * be available for display.
 */
void ewl_notebook_page_append(Ewl_Notebook * n, Ewl_Widget * t, Ewl_Widget * p)
{
	Ewl_Widget     *w;
	Ewl_Widget     *b;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("n", n);
	DCHECK_PARAM_PTR("p", p);

	w = EWL_WIDGET(n);

	b = ewl_button_new(NULL);
	if (t)
		ewl_container_child_append(EWL_CONTAINER(b), t);
	ewl_callback_append(b, EWL_CALLBACK_CLICKED, 
					ewl_notebook_tab_click_cb, p);
	ewl_widget_show(b);

	ewl_container_child_append(EWL_CONTAINER(n->tab_box), b);
	ewl_container_child_append(EWL_CONTAINER(w), p);
	ewl_widget_data_set(p, n, b);

	ewl_callback_append(p, EWL_CALLBACK_REPARENT, 
			    ewl_notebook_page_reparent_cb, n);

	if (!n->visible_page)
		n->visible_page = p;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * ewl_notebook_page_prepend - prepend a page to the notebook
 * @n: the notebook to prepend the page
 * @t: the tab of the new page added
 * @p: the contents of the page added
 *
 * Returns no value. Prepends a page to the list of available pages that will
 * be available for display.
 */
void ewl_notebook_page_prepend(Ewl_Notebook * n, Ewl_Widget * t, Ewl_Widget * p)
{
	Ewl_Widget     *w;
	Ewl_Widget     *b;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("n", n);
	DCHECK_PARAM_PTR("p", p);

	w = EWL_WIDGET(n);

	b = ewl_button_new(NULL);
	if (t)
		ewl_container_child_append(EWL_CONTAINER(b), t);
	ewl_callback_append(b, EWL_CALLBACK_CLICKED, 
					ewl_notebook_tab_click_cb, p);
	ewl_widget_show(b);

	ewl_container_child_prepend(EWL_CONTAINER(n->tab_box), b);
	ewl_container_child_prepend(EWL_CONTAINER(w), p);
	ewl_widget_data_set(p, n, b);

	ewl_callback_append(p, EWL_CALLBACK_REPARENT, 
			    ewl_notebook_page_reparent_cb, n);

	if (!n->visible_page)
		n->visible_page = p;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * ewl_notebook_page_insert - insert a page to the notebook
 * @n: the notebook to insert the page
 * @t: the tab of the new page added
 * @p: the contents of the page added
 * @pos: the position in the list of tabs to add the page
 *
 * Returns no value. Insert a page to the list of available pages that will
 * be available for display.
 */
void
ewl_notebook_page_insert(Ewl_Notebook * n, Ewl_Widget * t, Ewl_Widget * p,
			 int pos)
{
	Ewl_Widget     *w;
	Ewl_Widget     *b;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("n", n);
	DCHECK_PARAM_PTR("p", p);

	w = EWL_WIDGET(n);

	b = ewl_button_new(NULL);
	if (t)
		ewl_container_child_append(EWL_CONTAINER(b), t);
	ewl_callback_append(b, EWL_CALLBACK_CLICKED, 
					ewl_notebook_tab_click_cb, p);
	ewl_widget_show(b);

	ewl_container_child_insert(EWL_CONTAINER(n->tab_box), b, pos);
	ewl_container_child_insert(EWL_CONTAINER(w), p, pos);
	ewl_widget_data_set(p, n, b);

	ewl_callback_append(p, EWL_CALLBACK_REPARENT, 
			    ewl_notebook_page_reparent_cb, n);

	if (!n->visible_page)
		n->visible_page = p;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * ewl_notebook_first_page_remove - remove the first page from the notebook
 * @n: the notebook to remove the first page
 *
 * Returns a pointer to the removed page on success, NULL on failure.
 */
void ewl_notebook_first_page_remove(Ewl_Notebook * n)
{
	Ewl_Widget     *w;
	Ewl_Widget     *page = NULL;
	Ewl_Widget     *tab;
	Ewl_Container  *c;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("n", n);

	w = EWL_WIDGET(n);
	c = EWL_CONTAINER(n);

	if (!ecore_list_nodes(c->children))
		DRETURN(DLEVEL_STABLE);

	ecore_list_goto_first(c->children);
	while ((page = ecore_list_next(c->children)) && (page == n->tab_box));

	if (page) {
		ewl_container_child_remove(c, page);
		tab = ewl_widget_data_get(page, n);
		if (tab)
			ewl_widget_destroy(tab);
		if (page == n->visible_page) {
			n->visible_page = NULL;
			ewl_notebook_visible_page_set(n, 0);
		}
		ewl_widget_destroy(page);
	}

	DRETURN(DLEVEL_STABLE);
}

/**
 * ewl_notebook_first_page_remove - remove the last page from the notebook
 * @n: the notebook to remove the last page
 *
 * Returns a pointer to the removed page on success, NULL on failure.
 */
void ewl_notebook_last_page_remove(Ewl_Notebook * n)
{
	Ewl_Widget     *w;
	Ewl_Widget     *page = NULL;
	Ewl_Widget     *last = NULL;
	Ewl_Widget     *tab;
	Ewl_Container  *c;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("n", n);

	w = EWL_WIDGET(n);
	c = EWL_CONTAINER(n);

	if (!ecore_list_nodes(c->children))
		DRETURN(DLEVEL_STABLE);

	ecore_list_goto_first(c->children);
	while ((last = ecore_list_next(c->children))) {
		if (page != n->tab_box) {
			page = last;
		}
	}

	if (page) {
		ewl_container_child_remove(c, page);
		tab = ewl_widget_data_get(page, n);
		if (tab)
			ewl_widget_destroy(tab);
		if (page == n->visible_page) {
			n->visible_page = NULL;
			ewl_notebook_visible_page_set(n, 0);
		}
		ewl_widget_destroy(page);
	}

	DRETURN(DLEVEL_STABLE);
}

/**
 * ewl_notebook_page_remove - remove the specified page from the notebook
 * @n: the notebook to remove the specified page
 * @i: the position in the list of pages to remove from @n
 *
 * Returns a pointer to the removed page on success, NULL on failure.
 */
void ewl_notebook_page_remove(Ewl_Notebook * n, int i)
{
	int j = 1;
	Ewl_Widget     *w;
	Ewl_Widget     *page = NULL;
	Ewl_Widget     *tab;
	Ewl_Container  *c;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("n", n);

	w = EWL_WIDGET(n);
	c = EWL_CONTAINER(n);

	if (!ecore_list_nodes(c->children))
		DRETURN(DLEVEL_STABLE);

	ecore_list_goto_first(c->children);
	while ((page = ecore_list_next(c->children)) && j < i) {
		if (page != n->tab_box)
			j++;
	}

	if (page) {
		ewl_container_child_remove(c, page);
		tab = ewl_widget_data_get(page, n);
		if (tab)
			ewl_widget_destroy(tab);
		if (page == n->visible_page) {
			n->visible_page = NULL;
			ewl_notebook_visible_page_set(n, 0);
		}
		ewl_widget_destroy(page);
	}

	DRETURN(DLEVEL_STABLE);
}

/**
 * ewl_notebook_visible_page_remove - remove the visible page from the notebook
 * @n: the notebook to remove the visible page
 *
 * Returns no value.
 */
void ewl_notebook_visible_page_remove(Ewl_Notebook * n)
{
	Ewl_Widget     *w;
	Ewl_Widget     *page = NULL;
	Ewl_Widget     *tab;
	Ewl_Container  *c;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("n", n);

	w = EWL_WIDGET(n);
	c = EWL_CONTAINER(n);

	/*
	 * No visible page? What the hell are we doing here then?!?
	 */
	if (!n->visible_page)
		DRETURN(DLEVEL_STABLE);

	/*
	 * Search out the page in the notebook, and the tab that goes with it.
	 */
	ecore_list_goto_first(c->children);
	while ((page = ecore_list_next(c->children)) && page != n->visible_page);

	/*
	 * We found a page, sweet, kick it to the curb!
	 */
	if (page) {

		/*
		 * Remove from the list of pages in this notebook, and set it
		 * to be freed.
		 */
		ewl_container_child_remove(c, page);
		tab = ewl_widget_data_get(page, n);
		if (tab)
			ewl_widget_destroy(tab);
		ewl_widget_destroy(page);

		/*
		 * Set a usable visible page.
		 */
		n->visible_page = NULL;
		ewl_notebook_visible_page_set(n, 0);
	}

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * ewl_notebook_tabs_alignment_set - set the alignment of a notebooks tabs
 * @n: the notebook to change tab alignment
 * @a: the new alignment for the tabs of @n
 *
 * Returns no value. Changes the alignment of the tabs on @n to @a, and
 * updates the display.
 */
void ewl_notebook_tabs_alignment_set(Ewl_Notebook * n, unsigned int a)
{
	Ewl_Widget     *w;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("n", n);

	w = EWL_WIDGET(n);

	ewl_object_alignment_set(EWL_OBJECT(n->tab_box), a);

	ewl_widget_configure(w);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * ewl_notebook_tabs_alignment_get - get the alignment of a notebooks tabs
 * @n: the notebook to get tab alignment
 *
 * Returns the tab alignment of the notebook @n on success, 0 on failure.
 */
unsigned int ewl_notebook_tabs_alignment_get(Ewl_Notebook * n)
{
	Ewl_Widget     *w;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("n", n, 0);

	w = EWL_WIDGET(n);

	DRETURN_INT(ewl_object_alignment_get(EWL_OBJECT(n->tab_box)),
		    DLEVEL_TESTING);
}

/**
 * ewl_notebook_tabs_position_set - set a notebooks tab position
 * @n: the notebook to change tab position
 * @p: the new position for the tabs of @n
 *
 * Returns no value. Changes the tab position of @n to @p and updates the
 * display.
 */
void ewl_notebook_tabs_position_set(Ewl_Notebook * n, Ewl_Position p)
{
	Ewl_Widget     *w;
	char            file[PATH_MAX];

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("n", n);

	w = EWL_WIDGET(n);

	if (n->flags & p)
		DRETURN(DLEVEL_STABLE);

	switch (n->flags & EWL_POSITION_MASK) {
		case EWL_POSITION_LEFT:
			ewl_callback_del(w, EWL_CALLBACK_CONFIGURE,
					 ewl_notebook_configure_left_cb);
			break;
		case EWL_POSITION_RIGHT:
			ewl_callback_del(w, EWL_CALLBACK_CONFIGURE,
					 ewl_notebook_configure_right_cb);
			break;
		case EWL_POSITION_BOTTOM:
			ewl_callback_del(w, EWL_CALLBACK_CONFIGURE,
					 ewl_notebook_configure_bottom_cb);
			break;
		case EWL_POSITION_TOP:
		default:
			ewl_callback_del(w, EWL_CALLBACK_CONFIGURE,
					 ewl_notebook_configure_bottom_cb);
			break;
	}

	n->flags = (n->flags & ~EWL_POSITION_MASK) | p;

	switch (n->flags & EWL_POSITION_MASK) {
		case EWL_POSITION_LEFT:
			snprintf(file, PATH_MAX, "lnotebook");
			ewl_box_orientation_set(EWL_BOX(n->tab_box),
					EWL_ORIENTATION_VERTICAL);
			ewl_callback_append(w, EWL_CALLBACK_CONFIGURE,
					    ewl_notebook_configure_left_cb,
					    NULL);
			break;
		case EWL_POSITION_RIGHT:
			snprintf(file, PATH_MAX, "rnotebook");
			ewl_box_orientation_set(EWL_BOX(n->tab_box),
					EWL_ORIENTATION_VERTICAL);
			ewl_callback_append(w, EWL_CALLBACK_CONFIGURE,
					    ewl_notebook_configure_right_cb,
					    NULL);
			break;
		case EWL_POSITION_BOTTOM:
			snprintf(file, PATH_MAX, "bnotebook");
			ewl_box_orientation_set(EWL_BOX(n->tab_box),
					EWL_ORIENTATION_HORIZONTAL);
			ewl_callback_append(w, EWL_CALLBACK_CONFIGURE,
					    ewl_notebook_configure_bottom_cb,
					    NULL);
			break;
		case EWL_POSITION_TOP:
		default:
			snprintf(file, PATH_MAX, "tnotebook");
			ewl_box_orientation_set(EWL_BOX(n->tab_box),
					EWL_ORIENTATION_HORIZONTAL);
			ewl_callback_append(w, EWL_CALLBACK_CONFIGURE,
					    ewl_notebook_configure_top_cb,
					    NULL);
			break;
	}

	ewl_widget_appearance_set(w, file);
	ewl_widget_configure(w);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * ewl_notebook_tabs_position_get - get the position of a notebooks tabs
 * @n: the notebook to retrieve the tab position
 *
 * Returns the position of the tabs in @n on success, 0 on failure.
 */
Ewl_Position ewl_notebook_tabs_position_get(Ewl_Notebook * n)
{
	Ewl_Widget     *w;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("n", n, 0);

	w = EWL_WIDGET(n);

	DRETURN_INT(n->flags, DLEVEL_TESTING);
}

/**
 * ewl_notebook_tabs_visible_set - set the visibility of the tabs
 * @n: the notebook to change flags
 * @show: the sets if the tabs should be shown (0) or not (1)
 *
 * Returns no value. Sets the visiblity for the tabs
 */
void ewl_notebook_tabs_visible_set(Ewl_Notebook * n, int show)
{
	Ewl_Widget     *w;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("n", n);

	w = EWL_WIDGET(n);

	if (show) {
		n->flags &= EWL_POSITION_MASK;
		ewl_widget_show(n->tab_box);
	}
	else {
		n->flags |= EWL_NOTEBOOK_FLAG_TABS_HIDDEN;
		ewl_widget_hide(n->tab_box);
	}

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param n: the notebook to switch tabs
 * @param t: the tab number to switch to
 * @return Returns no value.
 * @brief Switches to the tab number @a t in the notebook widget @a n.
 *
 * The notebook @a n switches to the tab number @a t where @a t is between 0
 * and the number of widgets in the notebook.
 */
void ewl_notebook_visible_page_set(Ewl_Notebook *n, int t)
{
	int i = 0;
	Ewl_Container *c;
	Ewl_Widget *child = NULL;

	DENTER_FUNCTION(DLEVEL_STABLE);

	c = EWL_CONTAINER(n);

	if (!c->children || !ecore_list_nodes(c->children) 
			|| t > ecore_list_nodes(c->children))
		DRETURN(DLEVEL_STABLE);

	i = 0;
	ecore_list_goto_first(c->children);
	while ((child = ecore_list_next(c->children))) {
		if (child != n->tab_box &&
				ewl_object_flags_get(EWL_OBJECT(child),
						EWL_FLAG_QUEUED_DSCHEDULED)) {
			if (i == t)
				break;
			i++;
		}
	}

	if (!child || child == n->visible_page)
		DRETURN(DLEVEL_STABLE);

	if (n->visible_page)
		ewl_widget_hide(n->visible_page);
	n->visible_page = child;
	ewl_widget_show(child);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param n: the notebook to retrieve the current visible page number
 * @return Returns the current page number on success.
 * @brief Retrieves the position of the current page in the notebook @a n.
 */
int ewl_notebook_visible_page_get(Ewl_Notebook *n)
{
	int i = 0;
	Ewl_Container *c;
	Ewl_Widget *child = NULL;

	DENTER_FUNCTION(DLEVEL_STABLE);

	c = EWL_CONTAINER(n);

	ecore_list_goto_first(c->children);
	while (child != n->visible_page &&
			(child = ecore_list_next(c->children))) {
		if (child != n->tab_box)
			i++;
	}

	DRETURN_INT(i, DLEVEL_STABLE);
}

void
ewl_notebook_tab_click_cb(Ewl_Widget *widget, void *ev_data, void *user_data)
{
	Ewl_Widget *page;
	Ewl_Notebook *nb;

	page = EWL_WIDGET(user_data);

	/*
	 * We need to find the notebook containing the tabbox containing this
	 * widget.
	 */
	nb = EWL_NOTEBOOK(widget->parent->parent);

	if (nb->visible_page)
		ewl_widget_hide(nb->visible_page);
	nb->visible_page = page;
	ewl_widget_show(nb->visible_page);
}

void
ewl_notebook_configure_top_cb(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Notebook   *n;
	int x, y;
	int width, height;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	n = EWL_NOTEBOOK(w);

	width = CURRENT_W(n);
	height = CURRENT_H(n) / 2;

	ewl_object_size_request(EWL_OBJECT(n->tab_box), width, height);
	height = ewl_object_current_h_get(EWL_OBJECT(n->tab_box));

	ewl_object_place(EWL_OBJECT(n->tab_box), CURRENT_X(w), CURRENT_Y(w),
			 width, height);
	x = CURRENT_X(w);
	y = CURRENT_Y(w) + height;
	height = CURRENT_H(w) - height;

	if (n->visible_page)
		ewl_object_geometry_request(EWL_OBJECT(n->visible_page),
				x, y, width, height);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void
ewl_notebook_configure_bottom_cb(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Notebook   *n;
	int x, y;
	int width, height;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	n = EWL_NOTEBOOK(w);

	width = CURRENT_W(n);
	height = CURRENT_H(n) / 2;

	ewl_object_size_request(EWL_OBJECT(n->tab_box), width, height);
	height = ewl_object_current_h_get(EWL_OBJECT(n->tab_box));

	x = CURRENT_X(w);
	y = CURRENT_Y(w) + height;
	height = CURRENT_H(w) - height;

	ewl_object_place(EWL_OBJECT(n->tab_box), CURRENT_X(w),
			 CURRENT_Y(w) + CURRENT_H(w) - height, width, height);
	x = CURRENT_X(w);
	y = CURRENT_Y(w);
	height = CURRENT_H(w) - height;

	if (n->visible_page)
		ewl_object_geometry_request(EWL_OBJECT(n->visible_page),
				x, y, width, height);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void
ewl_notebook_configure_left_cb(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Notebook   *n;
	int x, y;
	int width, height;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	n = EWL_NOTEBOOK(w);

	width = CURRENT_W(n) / 2;
	height = CURRENT_H(n);

	ewl_object_size_request(EWL_OBJECT(n->tab_box), width, height);
	width = ewl_object_current_w_get(EWL_OBJECT(n->tab_box));

	ewl_object_place(EWL_OBJECT(n->tab_box), CURRENT_X(w), CURRENT_Y(w),
			 width, height);
	x = CURRENT_X(w) + width;
	y = CURRENT_Y(w);
	width = CURRENT_W(w) - width;

	if (n->visible_page)
		ewl_object_geometry_request(EWL_OBJECT(n->visible_page),
				x, y, width, height);
}

void
ewl_notebook_configure_right_cb(Ewl_Widget * w, void *ev_data, void *user_data)
{
	Ewl_Notebook   *n;
	int x, y;
	int width, height;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);

	n = EWL_NOTEBOOK(w);

	width = CURRENT_W(n) / 2;
	height = CURRENT_H(n);

	ewl_object_size_request(EWL_OBJECT(n->tab_box), width, height);
	width = ewl_object_current_w_get(EWL_OBJECT(n->tab_box));

	ewl_object_place(EWL_OBJECT(n->tab_box),
			 CURRENT_X(w) + CURRENT_W(w) - width, CURRENT_Y(w),
			 width, height);
	x = CURRENT_X(w);
	y = CURRENT_Y(w);
	width = CURRENT_W(w) - width;

	if (n->visible_page)
		ewl_object_geometry_request(EWL_OBJECT(n->visible_page),
				x, y, width, height);
}

void
ewl_notebook_child_show_cb(Ewl_Container *c, Ewl_Widget *w)
{
	Ewl_Notebook *n;

	DENTER_FUNCTION(DLEVEL_STABLE);

	n = EWL_NOTEBOOK(c);

	/*
	 * Sizing depends on the placement of the tabs
	 */
	if (n->flags & EWL_POSITION_LEFT || n->flags & EWL_POSITION_RIGHT) {
		ewl_container_largest_prefer(EWL_CONTAINER(c),
				EWL_ORIENTATION_VERTICAL);
		if (w != n->tab_box) {
			ewl_container_largest_prefer(EWL_CONTAINER(c),
					EWL_ORIENTATION_HORIZONTAL);
		}

		if (VISIBLE(n->tab_box))
			ewl_object_preferred_inner_w_set(EWL_OBJECT(c),
					PREFERRED_W(c) +
					ewl_object_preferred_w_get(
						EWL_OBJECT(n->tab_box)));
	}
	else {
		ewl_container_largest_prefer(EWL_CONTAINER(c),
				EWL_ORIENTATION_HORIZONTAL);
		if (w != n->tab_box) {
			ewl_container_largest_prefer(EWL_CONTAINER(c),
					EWL_ORIENTATION_VERTICAL);
		}

		if (VISIBLE(n->tab_box))
			ewl_object_preferred_inner_h_set(EWL_OBJECT(c),
					PREFERRED_H(c) +
					ewl_object_preferred_w_get(
						EWL_OBJECT(n->tab_box)));
	}

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void
ewl_notebook_child_resize_cb(Ewl_Container *c, Ewl_Widget *w, int size,
		Ewl_Orientation o)
{
	Ewl_Notebook *n;

	DENTER_FUNCTION(DLEVEL_STABLE);

	n = EWL_NOTEBOOK(c);

	/*
	 * Sizing depends on the placement of the tabs
	 */
	if (n->flags & EWL_POSITION_LEFT || n->flags & EWL_POSITION_RIGHT) {
		ewl_container_largest_prefer(EWL_CONTAINER(c),
				EWL_ORIENTATION_VERTICAL);
		if (w != n->tab_box) {
			ewl_container_largest_prefer(EWL_CONTAINER(c),
					EWL_ORIENTATION_HORIZONTAL);
		}

		if (VISIBLE(n->tab_box))
			ewl_object_preferred_inner_w_set(EWL_OBJECT(c),
					PREFERRED_W(c) +
					ewl_object_preferred_w_get(
						EWL_OBJECT(n->tab_box)));
	}
	else {
		ewl_container_largest_prefer(EWL_CONTAINER(c),
				EWL_ORIENTATION_HORIZONTAL);
		if (w != n->tab_box) {
			ewl_container_largest_prefer(EWL_CONTAINER(c),
					EWL_ORIENTATION_VERTICAL);
		}

		if (VISIBLE(n->tab_box))
			ewl_object_preferred_inner_h_set(EWL_OBJECT(c),
					PREFERRED_H(c) +
					ewl_object_preferred_w_get(
						EWL_OBJECT(n->tab_box)));
	}

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void
ewl_notebook_page_reparent_cb(Ewl_Widget *w, void *ev_data, void *user_data) 
{
	Ewl_Notebook *n = NULL;
	Ewl_Widget *tab = NULL;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_PARAM_PTR("user_data", user_data);

	n = EWL_NOTEBOOK(user_data);

	if (EWL_WIDGET(ev_data) == EWL_WIDGET(n))
		DRETURN(DLEVEL_STABLE);

	tab = ewl_widget_data_get(w, n);
	if (tab)
		ewl_widget_destroy(tab);

	if (n->visible_page == w) {
		n->visible_page = NULL;
		ewl_notebook_visible_page_set(n, 1);
	}

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}


