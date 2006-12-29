#include "Ewl_Test.h"
#include "ewl_test_private.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <Ecore.h>
#include <Ecore_File.h>
#include <Ecore_Data.h>

#define MAIN_WIDTH 640
#define MAIN_HEIGHT 320

static char *ewl_test_about_body = 
		"The EWL Test application services two purposes\n"
		"The first is to allow us to test the different\n"
		"pices of EWL as we develop and work on them.\n\n"
		"The second piece is to allow deveopers to see\n"
		"tutorials and source listings for the different\n"
		"widgets in the system.\n\n"
		"If you develop your own EWL widget you can add\n"
		"a test to the test widget without needing to\n"
		"recompile or edit the test application.\n";

static int ewl_test_setup_tests(void);
static void ewl_test_print_tests(void);

static void run_test_boxed(Ewl_Widget *w, void *ev, void *data);
static void run_window_test(Ewl_Test *test, int width, int height);
static int run_unit_tests(Ewl_Test *test);
static int create_main_test_window(Ewl_Container *win);
static void fill_source_text(Ewl_Test *test);
static void text_parse(char *str);
static void tutorial_parse(Ewl_Text *tutorial, char *str);
static void setup_unit_tests(Ewl_Test *test);

static int ewl_test_cb_unit_test_timer(void *data);
static void ewl_test_cb_delete_window(Ewl_Widget *w, void *ev, void *data);
static void ewl_test_cb_exit(Ewl_Widget *w, void *ev, void *data);
static void cb_run_unit_tests(Ewl_Widget *w, void *ev, void *data);

static void ewl_test_cb_help(Ewl_Widget *w, void *ev, void *data);
static void ewl_test_cb_about(Ewl_Widget *w, void *ev, void *data);

static Ecore_List *tests = NULL;
static int window_count = 0;
static int current_unit_test = 0;
static Ecore_Timer *unit_test_timer = NULL;

static Ewl_Test *current_test = NULL;

int
main(int argc, char **argv)
{
	int i, unit_test = 0, ran_test = 0, ret = 0;

	/* initialize the ewl library */
	if (!ewl_init(&argc, argv))
	{
		printf("Unable to init EWL.\n");
		return 1;
	}

	if (!ewl_test_setup_tests())
	{
		printf("Unable to setup tests.\n");
		return 1;
	}

	/* check for any flags */
	for (i = 0; i < argc; i++)
	{
		Ewl_Test *t;

		if (!strncmp(argv[i], "-list", 5))
		{
			ewl_test_print_tests();
			exit(0);
		}
		else if (!strncmp(argv[i], "-all", 4))
		{
			Ewl_Test *t;

			ecore_list_goto_first(tests);
			while ((t = ecore_list_next(tests)))
				run_window_test(t, MAIN_WIDTH, MAIN_HEIGHT);
		}
		else if (!strncmp(argv[i], "-unit", 5))
			unit_test = 1;

		/* see if this thing was a test to run */
		ecore_list_goto_first(tests);
		while ((t = ecore_list_next(tests)))
		{
			if (!strcasecmp(argv[i], t->name))
			{
				if (unit_test)
					ret = run_unit_tests(t);
				else
					run_window_test(t, MAIN_WIDTH, 
								MAIN_HEIGHT);

				ran_test ++;
				break;
			}
		}
	}

	/* no passed in tests, run the main test app */
	if (!ran_test)
	{
		Ewl_Test test;

		test.name = "The Enlightened Widget Library Test App";
		test.func = create_main_test_window;
		test.filename = NULL;
		test.tip = NULL;

		run_window_test(&test, MAIN_WIDTH, MAIN_HEIGHT);
	}

	if (!unit_test)
		ewl_main();

	return ret;
}

static int
ewl_test_cb_unit_test_timer(void *data)
{
	char buf[1024];
	char *entries[3];
	Ewl_Unit_Test *unit_tests;
	int ret = 1;

	unit_tests = data;
	if (unit_tests[current_unit_test].func)
	{
		int val;
		Ewl_Widget *tree, *progress, *stat;

		stat = ewl_widget_name_find("statusbar");
		ewl_statusbar_push(EWL_STATUSBAR(stat), (char *)unit_tests[current_unit_test].name);
		val = unit_tests[current_unit_test].func(buf, sizeof(buf));
		ewl_statusbar_pop(EWL_STATUSBAR(stat));

		tree = ewl_widget_name_find("unit_test_tree");
		progress = ewl_widget_name_find("unit_test_progress");

		entries[0] = (char *)unit_tests[current_unit_test].name;
		entries[1] = (val ? "PASS" : "FAIL");
		entries[2] = (val ? "" : buf);
		ewl_tree_text_row_add(EWL_TREE(tree), NULL, entries);

		ewl_range_value_set(EWL_RANGE(progress),
				(double)(++current_unit_test));
	}
	else
	{
		unit_test_timer = NULL;
		current_unit_test = 0;
		ret = 0;
	}

	return ret;
}

static void
ewl_test_cb_delete_window(Ewl_Widget *w, void *ev __UNUSED__, 
				void *data __UNUSED__)
{
	ewl_widget_destroy(w);

	if ((--window_count) < 1)
		ewl_main_quit();
}

static void
ewl_test_cb_exit(Ewl_Widget *w __UNUSED__, void *ev __UNUSED__,
		 void *data __UNUSED__)
{
	ewl_main_quit();
}

static void
ewl_test_print_tests(void)
{
	Ewl_Test *t;

	printf("Ewl_Test Test List:\n");
	ecore_list_goto_first(tests);
	while ((t = ecore_list_next(tests)))
		printf("  %s\n", t->name);
}

static void
run_window_test(Ewl_Test *test, int width, int height)
{
	Ewl_Widget *win, *box;

	/* nothing to do if there is no ui test */
	if (!test->func) return;

	win = ewl_window_new();
	ewl_window_title_set(EWL_WINDOW(win), test->name);
	ewl_window_name_set(EWL_WINDOW(win), test->name);
	ewl_window_class_set(EWL_WINDOW(win), "Ewl Test Window");
	ewl_callback_append(win, EWL_CALLBACK_DELETE_WINDOW,
					ewl_test_cb_delete_window, NULL);	
	if ((width > 0) && (height > 0))
		ewl_object_size_request(EWL_OBJECT(win), width, height);

	ewl_widget_show(win);

	box = ewl_vbox_new();
	ewl_container_child_append(EWL_CONTAINER(win), box);
	ewl_widget_show(box);
	window_count ++;

	test->func(EWL_CONTAINER(box));
}

/* Return 0 if all tests pased, other wise return the number of failures */
static int
run_unit_tests(Ewl_Test *test)
{
	int i, failures = 0;
	char buf[1024];
	buf[0] = '\0';

	/* no unit tests, nothign to do */
	if (!test->unit_tests) return 0;

	for (i = 0; test->unit_tests[i].func; i++)
	{
		int ret;

		printf("Running %s: ", test->unit_tests[i].name);
		ret = test->unit_tests[i].func(buf, sizeof(buf));
		printf("%s %s\n", (ret ? "PASSED" : "FAILED"), 
						(ret ? "" : buf));

		buf[0] = '\0';

		if (!ret) failures++;
	}

	return failures;
}

static void
run_test_boxed(Ewl_Widget *w __UNUSED__, void *ev __UNUSED__,
							void *data)
{
	Ewl_Test *t;
	Ewl_Widget *c, *n, *stat;
	char info[1024];

	t = data;

	/* make sure we have a function if we aren't a straight unit test */
	if ((t->type != EWL_TEST_TYPE_UNIT) && (!t->func))
	{
		printf("WARNING: No UI test function defined for (%s).\n", 
								t->name);
		return;
	}

	stat = ewl_widget_name_find("statusbar");
	snprintf(info, sizeof(info), "%s Information/Tests\n", t->name);
	ewl_statusbar_push(EWL_STATUSBAR(stat), info);

	fill_source_text(t);
	setup_unit_tests(t);

	c = ewl_widget_name_find("execute_box");
	ewl_container_reset(EWL_CONTAINER(c));

	if (t->type != EWL_TEST_TYPE_UNIT)
		t->func(EWL_CONTAINER(c));
	else
		c = ewl_widget_name_find("unit_test_box");

	n = ewl_widget_name_find("notebook");
	ewl_notebook_visible_page_set(EWL_NOTEBOOK(n), c);
}

static int
ewl_test_setup_tests(void)
{
	char buf[PATH_MAX], buf2[PATH_MAX];
	Ecore_List *list = NULL, *list2;
	char *file = NULL;

	tests = ecore_list_new();
	if (!tests) return 0;

	ecore_list_set_free_cb(tests, ECORE_FREE_CB(free));

	list2 = ecore_list_new();
	if (!list2) return 0;

	snprintf(buf, sizeof(buf), "%s", PACKAGE_LIB_DIR "/ewl/tests");
	list = ecore_file_ls(buf);
	if (list && ecore_list_nodes(list) > 0)
	{
		ecore_list_goto_first(list);
		while ((file = ecore_list_remove_first(list)))
		{
			int len;

			/* see if this is a .so file */
			len = strlen(file);
			if (!strncmp(file + (len - 3), ".so", 3))
			{
				snprintf(buf2, sizeof(buf2), "%s/%s", 
								buf, file);
				ecore_list_append(list2, strdup(buf2));
			}
			free(file);
		}
		ecore_list_destroy(list);
	}

	/* no tests found ... */
	if (ecore_list_nodes(list2) == 0) return 0;

	/* open each test file and get the Test struct from it */
	while ((file = ecore_list_remove_first(list2)))
	{
		void *handle;
		
		handle = dlopen(file, RTLD_LAZY | RTLD_GLOBAL);
		if (handle)
		{
			void (*func_info)(Ewl_Test *test);

			/* the UI test info */
			func_info = dlsym(handle, "test_info");
			if (func_info)
			{
				Ewl_Test *t;
				
				t = calloc(1, sizeof(Ewl_Test));
				func_info(t);
				t->handle = handle;
				ecore_list_append(tests, t);
			}
		}
		free(file);
	}
	ecore_list_destroy(list2);

	return 1;
}

static int
create_main_test_window(Ewl_Container *box)
{
	Ewl_Test *t;
	Ewl_Widget *menubar, *note, *tree, *o, *o2;
	Ewl_Widget *sim, *adv, *misc, *container, *unit;
	char *entries[1];
	char *headers[] = {"Test", "Status", "Failure Reason"};

	menubar = ewl_hmenubar_new();
	ewl_container_child_append(EWL_CONTAINER(box), menubar);
	ewl_widget_show(menubar);

	o = ewl_menu_new();
	ewl_container_child_append(EWL_CONTAINER(menubar), o);
	ewl_button_label_set(EWL_BUTTON(o), "File");
	ewl_object_fill_policy_set(EWL_OBJECT(o), EWL_FLAG_FILL_NONE);
	ewl_widget_show(o);

	o2 = ewl_menu_item_new();
	ewl_button_label_set(EWL_BUTTON(o2), "Exit");
	ewl_callback_append(o2, EWL_CALLBACK_CLICKED, ewl_test_cb_exit, NULL);
	ewl_container_child_append(EWL_CONTAINER(o), o2);
	ewl_widget_show(o2);

	o = ewl_spacer_new();
	ewl_container_child_append(EWL_CONTAINER(menubar), o);
	ewl_widget_show(o);

	o = ewl_menu_new();
	ewl_container_child_append(EWL_CONTAINER(menubar), o);
	ewl_button_label_set(EWL_BUTTON(o), "Help");
	ewl_object_fill_policy_set(EWL_OBJECT(o), EWL_FLAG_FILL_NONE);
	ewl_widget_show(o);

	o2 = ewl_menu_item_new();
	ewl_button_label_set(EWL_BUTTON(o2), "About EWL...");
	ewl_container_child_append(EWL_CONTAINER(o), o2);
	ewl_callback_append(o2, EWL_CALLBACK_CLICKED, ewl_test_cb_about, NULL);
	ewl_widget_show(o2);

	o2 = ewl_menu_item_new();
	ewl_button_label_set(EWL_BUTTON(o2), "EWL Test Help");
	ewl_container_child_append(EWL_CONTAINER(o), o2);
	ewl_callback_append(o2, EWL_CALLBACK_CLICKED, ewl_test_cb_help, NULL);
	ewl_widget_show(o2);

	note = ewl_notebook_new();
	ewl_container_child_append(box, note);
	ewl_widget_name_set(note, "notebook");
	ewl_notebook_tabbar_alignment_set(EWL_NOTEBOOK(note),
					EWL_FLAG_ALIGN_CENTER);
	ewl_widget_show(note);

	tree = ewl_tree_new(1);
	ewl_container_child_append(EWL_CONTAINER(note), tree);
	ewl_notebook_page_tab_text_set(EWL_NOTEBOOK(note), tree, "Tests");
	ewl_tree_headers_visible_set(EWL_TREE(tree), FALSE);
	ewl_tree_mode_set(EWL_TREE(tree), EWL_SELECTION_MODE_SINGLE);
	ewl_widget_show(tree);

	/* create the test category rows */
	entries[0] = "Simple";
	sim = ewl_tree_text_row_add(EWL_TREE(tree), NULL, entries);

	entries[0] = "Advanced";
	adv = ewl_tree_text_row_add(EWL_TREE(tree), NULL, entries);

	entries[0] = "Container";
	container = ewl_tree_text_row_add(EWL_TREE(tree), NULL, entries);

	entries[0] = "Misc";
	misc = ewl_tree_text_row_add(EWL_TREE(tree), NULL, entries);

	entries[0] = "Unit";
	unit = ewl_tree_text_row_add(EWL_TREE(tree), NULL, entries);

	/* add the tests to the category rows */
	ecore_list_goto_first(tests);
	while ((t = ecore_list_next(tests)))
	{
		Ewl_Widget *parent = NULL, *w;
		entries[0] = (char *)t->name;

		if (t->type == EWL_TEST_TYPE_SIMPLE)
			parent = sim;
		else if (t->type == EWL_TEST_TYPE_ADVANCED)
			parent = adv;
		else if (t->type == EWL_TEST_TYPE_CONTAINER)
			parent = container;
		else if (t->type == EWL_TEST_TYPE_MISC)
			parent = misc;
		else if (t->type == EWL_TEST_TYPE_UNIT)
			parent = unit;

		w = ewl_tree_text_row_add(EWL_TREE(tree), 
					EWL_ROW(parent), entries);
		ewl_callback_append(w, EWL_CALLBACK_CLICKED,
						run_test_boxed, t);
	}

	o = ewl_vbox_new();
	ewl_container_child_append(EWL_CONTAINER(note), o);
	ewl_object_fill_policy_set(EWL_OBJECT(o), EWL_FLAG_FILL_ALL);
	ewl_notebook_page_tab_text_set(EWL_NOTEBOOK(note), o, "UI Test");
	ewl_widget_name_set(o, "execute_box");
	ewl_widget_show(o);

	o = ewl_vbox_new();
	ewl_container_child_append(EWL_CONTAINER(note), o);
	ewl_notebook_page_tab_text_set(EWL_NOTEBOOK(note), o, "Unit Tests");
	ewl_widget_name_set(o, "unit_test_box");
	ewl_widget_show(o);

	o2 = ewl_tree_new(3);
	ewl_container_child_append(EWL_CONTAINER(o), o2);
	ewl_tree_headers_set(EWL_TREE(o2), headers);
	ewl_tree_mode_set(EWL_TREE(o2), EWL_SELECTION_MODE_SINGLE);
	ewl_widget_name_set(o2, "unit_test_tree");
	ewl_widget_show(o2);

	o2 = ewl_hbox_new();
	ewl_container_child_append(EWL_CONTAINER(o), o2);
	ewl_object_fill_policy_set(EWL_OBJECT(o2), EWL_FLAG_FILL_HFILL);
	ewl_widget_show(o2);

	o = o2;

	o2 = ewl_button_new();
	ewl_container_child_append(EWL_CONTAINER(o), o2);
	ewl_button_label_set(EWL_BUTTON(o2), "Run unit tests");
	ewl_callback_append(o2, EWL_CALLBACK_CLICKED, cb_run_unit_tests, NULL);
	ewl_object_fill_policy_set(EWL_OBJECT(o2), EWL_FLAG_FILL_SHRINK);
	ewl_widget_name_set(o2, "unit_test_button");
	ewl_widget_show(o2);

	o2 = ewl_progressbar_new();
	ewl_widget_name_set(o2, "unit_test_progress");
	ewl_container_child_append(EWL_CONTAINER(o), o2);
	ewl_widget_show(o2);

	o2 = ewl_scrollpane_new();
	ewl_container_child_append(EWL_CONTAINER(note), o2);
	ewl_notebook_page_tab_text_set(EWL_NOTEBOOK(note), o2, "Source");
	ewl_widget_show(o2);

	o = ewl_text_new();
	ewl_container_child_append(EWL_CONTAINER(o2), o);
	ewl_widget_name_set(o, "source_text");
	ewl_text_selectable_set(EWL_TEXT(o), TRUE);
	ewl_widget_show(o);

	o2 = ewl_scrollpane_new();
	ewl_container_child_append(EWL_CONTAINER(note), o2);
	ewl_notebook_page_tab_text_set(EWL_NOTEBOOK(note), o2, "Tutorial");
	ewl_widget_show(o2);

	o = ewl_text_new();
	ewl_container_child_append(EWL_CONTAINER(o2), o);
	ewl_widget_name_set(o, "tutorial_text");
	ewl_text_selectable_set(EWL_TEXT(o), TRUE);
	ewl_widget_show(o);

	o = ewl_statusbar_new();
	ewl_container_child_append(EWL_CONTAINER(box), o);
	ewl_statusbar_push(EWL_STATUSBAR(o), "Select Test");
	ewl_widget_name_set(o, "statusbar");
	ewl_widget_show(o);

	return 1;
}

static void
fill_source_text(Ewl_Test *test)
{
	FILE *file;
	struct stat buf;
	char filename[PATH_MAX];

	snprintf(filename, sizeof(filename), PACKAGE_DATA_DIR "/ewl/examples/%s", 
								test->filename);
	file = fopen(filename, "r");
	if (file)
	{
		char *str;

		stat(filename, &buf);

		str = malloc(sizeof(char) * (buf.st_size + 1));
		fread(str, buf.st_size, 1, file);
		str[buf.st_size] = '\0';
		fclose(file);

		text_parse(str);
		free(str);
	}
}

static void
setup_unit_tests(Ewl_Test *test)
{
	Ewl_Widget *button, *tree, *progress;
	char *entries[3];
	int i;

	button = ewl_widget_name_find("unit_test_button");
	tree = ewl_widget_name_find("unit_test_tree");

	ewl_container_reset(EWL_CONTAINER(tree));

	current_test = test;

	/* just clean up if no tests */
	if (!test->unit_tests) return;

	for (i = 0; test->unit_tests[i].func; i++)
	{
		entries[0] = (char *)test->unit_tests[i].name;
		entries[1] = "";
		entries[2] = "";

		ewl_tree_text_row_add(EWL_TREE(tree), NULL, entries);
	}

	progress = ewl_widget_name_find("unit_test_progress");
	ewl_range_maximum_value_set(EWL_RANGE(progress), (double)(i));
	ewl_range_value_set(EWL_RANGE(progress), 0.0);

}

static void
cb_run_unit_tests(Ewl_Widget *w __UNUSED__, void *ev __UNUSED__, 
						void *data __UNUSED__)
{
	int i;
	Ewl_Test *test;
	Ewl_Widget *tree;
	Ewl_Widget *progress;

	tree = ewl_widget_name_find("unit_test_tree");
	ewl_container_reset(EWL_CONTAINER(tree));

	test = current_test;
	if ((!test) || (!test->unit_tests)) return;

	for (i = 0; test->unit_tests[i].func; i++)
		;

	progress = ewl_widget_name_find("unit_test_progress");
	ewl_range_maximum_value_set(EWL_RANGE(progress), (double)(i));
	ewl_range_value_set(EWL_RANGE(progress), 0.0);

	if (unit_test_timer) 
	{
		ecore_timer_del(unit_test_timer);
		unit_test_timer = NULL;
		current_unit_test = 0;
	}

	unit_test_timer = ecore_timer_add(0.1, ewl_test_cb_unit_test_timer, 
							test->unit_tests);
}

static void
text_parse(char *str)
{
	Ewl_Widget *txt, *tutorial;
	char *start, *end, tmp;

	txt = ewl_widget_name_find("source_text");
	tutorial = ewl_widget_name_find("tutorial_text");

	start = strstr(str, "/**");
	if (!start)
	{
		ewl_text_text_set(EWL_TEXT(txt), str);
		ewl_text_text_set(EWL_TEXT(tutorial),  "");
		return;
	}

	end = strstr(str, "*/");
	end++;

	while (*(start - 1) == '\n') start --;

	tmp = *start;
	*start = '\0';
	
	ewl_text_text_set(EWL_TEXT(txt), str);
	ewl_text_text_append(EWL_TEXT(txt), end + 1);

	*start = tmp;
	tmp = *(end + 1);
	*(end + 1) = '\0';

	ewl_text_text_set(EWL_TEXT(tutorial), "");
	tutorial_parse(EWL_TEXT(tutorial), start);

	*(end + 1) = tmp;
}

static void
tutorial_parse(Ewl_Text *tutorial, char *str)
{
	char *ptr, *ptr2;

	ptr = str;

	/* move past the comment stuff to the start of the tutorial */
	while ((*ptr == '/') || (*ptr == '*') || (*ptr == '\n')
			|| (*ptr == ' '))
		ptr ++;

	/* while we've still got data */
	while (ptr != NULL)
	{
		ptr2 = ptr;
		while ((*ptr2 != '*') && (*ptr2 != '@') && (*ptr2 != '\0'))
			ptr2 ++;

		if (ptr2 == NULL)
		{
			ewl_text_text_append(tutorial, ptr);
			break;
		}
		else if (*ptr2 == '*')
		{
			char *prev, tmp;

			prev = ptr2;
			while ((*ptr2 == '*') || (*ptr2 == ' ')) ptr2 --;

			if (*ptr2 != '\n')
				ptr2 = prev;

			tmp = *(++ptr2);
			*ptr2 = '\0';

			if (ptr2 > ptr)
				ewl_text_text_append(tutorial, ptr);

			*ptr2 = tmp;
			ptr2 = prev;

			/* we're done if we have a / */
			if (*(ptr2 + 1) == '/')
				break;

			while ((*ptr2 == '*') || (*ptr2 == ' ')) ptr2 ++;
		}
		else if (*ptr2 == '@')
		{
			char *tmp;

			*ptr2 = '\0';
			ewl_text_text_append(tutorial, ptr);

			ptr2++;
			if ((!strncasecmp(ptr2, "addtogroup", 10))
					|| (!strncasecmp(ptr2, "section", 7)))
			{
				int size = 14;
				char t;

				if (!strncasecmp(ptr2, "addtogroup", 10))
					size = 18;

				ptr2 = strstr(ptr2, "\n");
				tmp = ptr2;

				while (*tmp != ' ') tmp --;
				tmp ++;
				t = *ptr2;
				*ptr2 = '\0';

				ewl_text_font_size_set(tutorial, size);
				ewl_text_text_append(tutorial, tmp);
				ewl_text_text_append(tutorial, "\n");
				ewl_text_font_size_set(tutorial, 10);
			
				*ptr2 = t;
				ptr2 ++;
			}
			else if (!strncasecmp(ptr2, "code", 4))
			{
				ptr2 += strlen("code\n");
				ewl_text_color_set(tutorial, 
						128, 128, 128, 255);
			}
			else if (!strncasecmp(ptr2, "endcode", 7))
			{
				ptr2 += strlen("endcode\n");
				ewl_text_color_set(tutorial,
						0, 0, 0, 255);
			}
			else if (!strncasecmp(ptr2, "note", 4))
			{
				ewl_text_font_size_set(tutorial, 12);
				ewl_text_text_append(tutorial, "Note");
				ewl_text_text_append(tutorial, "\n");
				ewl_text_font_size_set(tutorial, 10);

				ptr2 += strlen("note");
			}
			else
				printf("Didn't match (%s)\n", ptr2);
		}
		ptr = ptr2;
	}
}

static void
ewl_test_cb_help(Ewl_Widget *w __UNUSED__, void *ev __UNUSED__, 
						void *data __UNUSED__)
{
	Ewl_Widget *win, *vbox, *o;

	window_count++;

	win = ewl_window_new();
	ewl_window_title_set(EWL_WINDOW(win), "EWL Test Help");
	ewl_window_class_set(EWL_WINDOW(win), "ewl_test");
	ewl_window_name_set(EWL_WINDOW(win), "ewl_test");
	ewl_callback_append(win, EWL_CALLBACK_DELETE_WINDOW,
					ewl_test_cb_delete_window, NULL);
	ewl_widget_show(win);

	vbox = ewl_vbox_new();
	ewl_container_child_append(EWL_CONTAINER(win), vbox);
	ewl_widget_show(vbox);

	o = ewl_text_new();
	ewl_text_font_size_set(EWL_TEXT(o), 22);
	ewl_text_align_set(EWL_TEXT(o), EWL_FLAG_ALIGN_CENTER);
	ewl_text_text_set(EWL_TEXT(o), "Ewl_ Test App Help");
	ewl_text_text_append(EWL_TEXT(o), "\n\n");
	ewl_text_align_set(EWL_TEXT(o), EWL_FLAG_ALIGN_LEFT);
	ewl_text_font_size_set(EWL_TEXT(o), 12);
	ewl_text_text_append(EWL_TEXT(o), "Still need to write this, eh.");
	ewl_container_child_append(EWL_CONTAINER(vbox), o);
	ewl_widget_show(o);
}

static void
ewl_test_cb_about(Ewl_Widget *w __UNUSED__, void *ev __UNUSED__, 
						void *data __UNUSED__)
{
	Ewl_Widget *win, *vbox, *o;

	window_count++;

	win = ewl_window_new();
	ewl_window_title_set(EWL_WINDOW(win), "EWL Test About");
	ewl_window_class_set(EWL_WINDOW(win), "ewl_test");
	ewl_window_name_set(EWL_WINDOW(win), "ewl_test");
	ewl_callback_append(win, EWL_CALLBACK_DELETE_WINDOW,
					ewl_test_cb_delete_window, NULL);
	ewl_widget_show(win);

	vbox = ewl_vbox_new();
	ewl_container_child_append(EWL_CONTAINER(win), vbox);
	ewl_widget_show(vbox);

	o = ewl_text_new();
	ewl_text_font_size_set(EWL_TEXT(o), 22);
	ewl_text_align_set(EWL_TEXT(o), EWL_FLAG_ALIGN_CENTER);
	ewl_text_text_set(EWL_TEXT(o), "Ewl_ Test App");
	ewl_text_text_append(EWL_TEXT(o), "\n\n");
	ewl_text_align_set(EWL_TEXT(o), EWL_FLAG_ALIGN_LEFT);
	ewl_text_font_size_set(EWL_TEXT(o), 12);
	ewl_text_text_append(EWL_TEXT(o), ewl_test_about_body);
	ewl_container_child_append(EWL_CONTAINER(vbox), o);
	ewl_widget_show(o);
}


