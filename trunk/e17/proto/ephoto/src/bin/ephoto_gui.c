#include "ephoto.h"

static void destroy(Ewl_Widget *w, void *event, void *data);
static void add_menu_item(Ewl_Widget *c, char *txt, char *img, void *cb);
static void add_button(Ewl_Widget *c, char *txt, char *img, void *cb);
static void create_main_gui(void);
static int destroy_boot(void *data);
static Ewl_Widget *add_menu(Ewl_Widget *c, char *txt);
static Ewl_Widget *add_tree(Ewl_Widget *c);
static Ewl_Widget *progress;
static Ecore_Timer *timer;

/*Boot Splash*/
int destroy_boot(void *data)
{
	Ewl_Widget *win;
	double val, new_val;

	val = ewl_range_value_get(EWL_RANGE(progress));
	new_val = val + 20;
	ewl_range_value_set(EWL_RANGE(progress), new_val);	

	if (new_val == 100)
	{
		win = data;
		ewl_widget_destroy(win);
		ecore_timer_del(timer);
		create_main_gui();
	}
}

void init_gui(void)
{
	Ewl_Widget *win, *vbox, *image, *text;

	get_files("/home/titan");

        win = ewl_window_new();
        ewl_window_title_set(EWL_WINDOW(win), "Ephoto!");
        ewl_window_name_set(EWL_WINDOW(win), "Ephoto!");
        ewl_window_borderless_set(EWL_WINDOW(win));
	ewl_widget_state_set(win, "splash", EWL_STATE_PERSISTENT);
	ewl_object_size_request(EWL_OBJECT(win), 325, 240);
        ewl_callback_append(win, EWL_CALLBACK_DELETE_WINDOW, destroy, NULL);
        ewl_widget_show(win);

        vbox = ewl_vbox_new();
        ewl_object_fill_policy_set(EWL_OBJECT(vbox), EWL_FLAG_FILL_ALL);
        ewl_container_child_append(EWL_CONTAINER(win), vbox);
        ewl_widget_show(vbox);

	text = ewl_text_new();
	ewl_text_text_set(EWL_TEXT(text), VERSION);
	ewl_object_alignment_set(EWL_OBJECT(text), EWL_FLAG_ALIGN_LEFT);
	ewl_object_fill_policy_set(EWL_OBJECT(text), EWL_FLAG_FILL_SHRINK);
	ewl_container_child_append(EWL_CONTAINER(vbox), text);
	ewl_widget_show(text);

	image = ewl_image_new();
	ewl_image_file_set(EWL_IMAGE(image), 
			   PACKAGE_DATA_DIR "/images/logo.png", NULL);
	ewl_object_alignment_set(EWL_OBJECT(image), EWL_FLAG_ALIGN_CENTER);
	ewl_container_child_append(EWL_CONTAINER(vbox), image);
	ewl_widget_show(image);

	text = ewl_text_new();
	ewl_text_text_set(EWL_TEXT(text), "          Ephoto    \n"
					  "By Stephen Houston");
	ewl_object_alignment_set(EWL_OBJECT(text), EWL_FLAG_ALIGN_CENTER);
	ewl_object_fill_policy_set(EWL_OBJECT(text), EWL_FLAG_FILL_SHRINK);
	ewl_container_child_append(EWL_CONTAINER(vbox), text);
	ewl_widget_show(text);

	progress = ewl_progressbar_new();
	ewl_range_value_set(EWL_RANGE(progress), 0);
	ewl_object_alignment_set(EWL_OBJECT(progress), EWL_FLAG_ALIGN_CENTER);
	ewl_container_child_append(EWL_CONTAINER(vbox), progress);
	ewl_object_maximum_size_set(EWL_OBJECT(progress), 200, 20);
	ewl_widget_show(progress);

	timer = ecore_timer_add(1, destroy_boot, win);

	ewl_main();
}

/*Main Window Calls*/
static void destroy(Ewl_Widget *w, void *event, void *data)
{
        ewl_widget_destroy(w);
        ewl_main_quit();
}

static Ewl_Widget *add_menu(Ewl_Widget *c, char *txt)
{
	Ewl_Widget *menu;

	menu = ewl_menu_new();
	if (txt)
	{
		ewl_button_label_set(EWL_BUTTON(menu), S_(txt));
	}
	ewl_object_fill_policy_set(EWL_OBJECT(menu), EWL_FLAG_FILL_NONE);
	ewl_container_child_append(EWL_CONTAINER(c), menu);
	ewl_widget_show(menu);

	return menu;
}

static void add_menu_item(Ewl_Widget *c, char *txt, char *img, void *cb)
{
	Ewl_Widget *menu_item;
	
	menu_item = ewl_menu_item_new();
	if (img)
	{
		ewl_button_image_set(EWL_BUTTON(menu_item), img, NULL);
	}
	if (txt)
	{
		ewl_button_label_set(EWL_BUTTON(menu_item), S_(txt));
	}
	ewl_object_alignment_set(EWL_OBJECT(menu_item), EWL_FLAG_ALIGN_CENTER);
	ewl_object_fill_policy_set(EWL_OBJECT(menu_item), EWL_FLAG_FILL_ALL);
	ewl_container_child_append(EWL_CONTAINER(c), menu_item);
	if (cb)
	{
		ewl_callback_append(menu_item, EWL_CALLBACK_CLICKED, cb, NULL);
	}
	ewl_widget_show(menu_item);

	return;
}

static void add_button(Ewl_Widget *c, char *txt, char *img, void *cb)
{
	Ewl_Widget *button;

	button = ewl_button_new();
	if (img)
	{
		ewl_button_image_set(EWL_BUTTON(button), img, NULL);
	}
	if (txt)
	{
		ewl_button_label_set(EWL_BUTTON(button), _(txt));
	}
	ewl_object_alignment_set(EWL_OBJECT(button), EWL_FLAG_ALIGN_CENTER);
	ewl_object_fill_policy_set(EWL_OBJECT(button), EWL_FLAG_FILL_SHRINK);
	ewl_container_child_append(EWL_CONTAINER(c), button);
	if (cb)
	{
		ewl_callback_append(button, EWL_CALLBACK_CLICKED, cb, NULL);
	}
	ewl_widget_show(button);

	return;
}

static Ewl_Widget *add_tree(Ewl_Widget *c)
{
	Ewl_Widget *tree;

	tree = ewl_tree_new(1);
	ewl_tree_headers_visible_set(EWL_TREE(tree), 0);
	ewl_tree_expandable_rows_set(EWL_TREE(tree), 0);
	ewl_object_fill_policy_set(EWL_OBJECT(tree), EWL_FLAG_FILL_ALL);
	ewl_container_child_append(EWL_CONTAINER(c), tree);
	ewl_widget_show(tree);

	return tree;
}

static void create_main_gui(void)
{
	Ewl_Widget *win, *atree, *btree, *vbox, *menu_bar, *menu, *nb, *paned;
	Ewl_Widget *ivbox, *ihbox, *sp, *image, *button;

	win = ewl_window_new();
        ewl_window_title_set(EWL_WINDOW(win), "Ephoto!");
        ewl_window_name_set(EWL_WINDOW(win), "Ephoto!");
        ewl_object_size_request(EWL_OBJECT(win), 600, 400);
        ewl_callback_append(win, EWL_CALLBACK_DELETE_WINDOW, destroy, NULL);
	ewl_widget_show(win);

	vbox = ewl_vbox_new();
	ewl_object_fill_policy_set(EWL_OBJECT(vbox), EWL_FLAG_FILL_ALL);
	ewl_container_child_append(EWL_CONTAINER(win), vbox);
	ewl_widget_show(vbox);

	menu_bar = ewl_hmenubar_new();
	ewl_object_fill_policy_set(EWL_OBJECT(menu_bar), EWL_FLAG_FILL_HFILL);
	ewl_container_child_append(EWL_CONTAINER(vbox), menu_bar);
	ewl_widget_show(menu_bar);

	menu = add_menu(menu_bar, "Menu|File");
	add_menu_item(menu, "Menu|File|Exit", PACKAGE_DATA_DIR 
		"/images/exit.png", destroy);

	menu = add_menu(menu_bar, "Menu|Help");
	add_menu_item(menu, "Menu|Help|About", NULL, NULL);

	paned = ewl_hpaned_new();
	ewl_object_fill_policy_set(EWL_OBJECT(paned), EWL_FLAG_FILL_ALL);
	ewl_container_child_append(EWL_CONTAINER(vbox), paned);
	ewl_widget_show(paned);

	nb = ewl_notebook_new();
	ewl_notebook_tabbar_alignment_set(EWL_NOTEBOOK(nb), EWL_FLAG_ALIGN_LEFT);
	ewl_object_fill_policy_set(EWL_OBJECT(nb), EWL_FLAG_FILL_ALL);
	ewl_container_child_append(EWL_CONTAINER(paned), nb);
	ewl_widget_show(nb);

	atree = add_tree(nb);
	ewl_notebook_page_tab_text_set(EWL_NOTEBOOK(nb), atree, "Albums");
	btree = add_tree(nb);
	ewl_notebook_page_tab_text_set(EWL_NOTEBOOK(nb), btree, "Browser");

	ivbox = ewl_vbox_new();
	ewl_object_fill_policy_set(EWL_OBJECT(ivbox), EWL_FLAG_FILL_ALL);
	ewl_container_child_append(EWL_CONTAINER(paned), ivbox);
	ewl_widget_show(ivbox);

	sp = ewl_scrollpane_new();
	ewl_object_fill_policy_set(EWL_OBJECT(sp), EWL_FLAG_FILL_ALL);
	ewl_container_child_append(EWL_CONTAINER(ivbox), sp);
	ewl_widget_show(sp);

	image = ewl_image_new();
	ewl_image_proportional_set(EWL_IMAGE(image), TRUE);
	ewl_object_alignment_set(EWL_OBJECT(image), EWL_FLAG_ALIGN_CENTER);
	ewl_object_fill_policy_set(EWL_OBJECT(image), EWL_FLAG_FILL_SHRINK);
	ewl_container_child_append(EWL_CONTAINER(sp), image);
	ewl_widget_show(image);

	ihbox = ewl_hbox_new();
	ewl_container_child_append(EWL_CONTAINER(ivbox), ihbox);
	ewl_object_alignment_set(EWL_OBJECT(ihbox), EWL_FLAG_ALIGN_CENTER);
	ewl_object_fill_policy_set(EWL_OBJECT(ihbox), EWL_FLAG_FILL_SHRINK);
	ewl_widget_show(ihbox);

	add_button(ihbox, "In", PACKAGE_DATA_DIR "/images/search.png", NULL);
	add_button(ihbox, "Out", PACKAGE_DATA_DIR "/images/search.png", NULL);
	add_button(ihbox, "Left", PACKAGE_DATA_DIR "/images/undo.png", NULL);
	add_button(ihbox, "Right", PACKAGE_DATA_DIR "/images/redo.png", NULL);

	return;
}

