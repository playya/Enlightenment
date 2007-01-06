/* vim: set sw=8 ts=8 sts=8 noexpandtab: */
#include "Ewl_Test.h"
#include "ewl_test_private.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @addtogroup Ewl_Tree2
 * @section tree2_tut Tutorial
 *
 * Finding the Tree in the Forest
 * (originally at http://everburning.com/news/finding-the-tree-in-the-forest)
 * We've been doing a bunch of work on Ewl_Tree2 lately. Its been shaping up
 * nicely at the moment. To that end, I thought Id do a quick write up on how it
 * works and what you can do with it.
 *
 * Ewl_Tree2 is built based on an MVC (model/view/controller) framework. This
 * makes it a lot easier for developers to keep their data up to date without
 * having to go through all kinds of contortions using the tree nodes and
 * widgets as they do with the current Ewl_Tree widget.
 *
 * There are three basic items youll need to become familiar with in order to
 * use Ewl_Tree2. They are, Ewl_Tree2, of course, Ewl_Model and Ewl_View. These
 * three will allow you to setup your columns and the tree.
 *
 * The best way to show something is through an example, so thats what Ill do.
 * This is basically the tree2 test case from ewl_test ported to run in a
 * window. The case is pretty simple. We store an array of data, this array
 * contains nodes that specify the text and an image to be displayed.
 *
 * We then create a three column tree. The first column will show the text as an
 * Ewl_Label widget. The second column will display an Ewl_Image. The third will
 * be an Ewl_Button. The third column doesnt use the ewl_button code directly as
 * we want to set two pieces of information so we write our own methods to
 * handle the creating and assignment functions.
 *
 * I'll be putting the code into tree2_test.c and using the following to compile
 * the code as I go.
 *
 * @code
 * oni:~/dev/tree2_test$ gcc -o tree2_test tree2_test.c `ewl-config --cflags --libs`
 * @endcode
 *
 * With that out of the way, on with the show. Im going to start by listing all
 * of the code and then Ill go through it piece by piece.
 *
 * @code
 * #include <Ewl.h>
 * #include <stdio.h>
 * #include <stdlib.h>
 * #include <string.h>
 *
 * #define DATA_ELEMENTS 5
 *
 * typedef struct Test_Row_Data Test_Row_Data;
 * struct Test_Row_Data
 * {
 *     char *image;
 *     char *text;
 * };
 *
 * typedef struct Test_Data Test_Data;
 * struct Test_Data
 * {
 *     unsigned int count;
 *     Test_Row_Data **rows;
 * };
 *
 * static void *test_data_setup(void);
 *
 * static Ewl_Widget *test_custom_new(void);
 * static void test_custom_assign_set(Ewl_Widget *w, void *data);
 *
 * static Ewl_Widget *test_data_header_fetch(void *data, int column);
 * static void *test_data_fetch(void *data, unsigned int row, unsigned int column);
 * static void test_data_sort(void *data, unsigned int column, Ewl_Sort_Direction sort);
 * static int test_data_count_get(void *data);
 *
 * static void cb_delete_window(Ewl_Widget *w, void *ev, void *data);
 * static void cb_scroll_headers(Ewl_Widget *w, void *ev, void *data);
 * static void cb_scroll_visible(Ewl_Widget *w, void *ev, void *data);
 *
 * int
 * main(int argc, char ** argv)
 * {
 *     Ewl_Widget *tree, *box, *o, *o2;
 *     Ewl_Model *model;
 *     Ewl_View *view;
 *     void *data;
 *
 *      // make sure we can setup ewl
 *      if (!ewl_init(&argc, argv))
 *      {
 *          fprintf(stderr, "Unable to init ewl.n");
 *          return 1;
 *      }
 *
 *      // create the window
 *      o = ewl_window_new();
 *      ewl_window_title_set(EWL_WINDOW(o), "tree2 example");
 *      ewl_window_class_set(EWL_WINDOW(o), "tree2_example");
 *      ewl_window_name_set(EWL_WINDOW(o), "tree2_example");
 *      ewl_object_size_request(EWL_OBJECT(o), 640, 480);
 *      ewl_callback_append(o, EWL_CALLBACK_DELETE_WINDOW, cb_delete_window, NULL);
 *      ewl_widget_show(o);
 *
 *      box = ewl_vbox_new();
 *      ewl_container_child_append(EWL_CONTAINER(o), box);
 *      ewl_widget_show(box);
 *
 *      o2 = ewl_hbox_new();
 *      ewl_container_child_append(EWL_CONTAINER(box), o2);
 *      ewl_object_fill_policy_set(EWL_OBJECT(o2),
 *                  EWL_FLAG_FILL_VSHRINK | EWL_FLAG_FILL_HFILL);
 *      ewl_widget_show(o2);
 *
 *      // create our data
 *      data = test_data_setup();
 *
 *      // create the model that'll be used for the first two columns
 *      model = ewl_model_new();
 *      ewl_model_fetch_set(model, test_data_fetch);
 *      ewl_model_sort_set(model, test_data_sort);
 *      ewl_model_count_set(model, test_data_count_get);
 *
 *      tree = ewl_tree2_new();
 *      ewl_container_child_append(EWL_CONTAINER(box), tree);
 *      ewl_object_fill_policy_set(EWL_OBJECT(tree), EWL_FLAG_FILL_ALL);
 *      ewl_tree2_data_set(EWL_TREE2(tree), data);
 *      ewl_widget_show(tree);
 *
 *      // create a view for the first column that just has an ewl label
 *      view = ewl_view_new();
 *      ewl_view_constructor_set(view, ewl_label_new);
 *      ewl_view_assign_set(view, EWL_VIEW_ASSIGN(ewl_label_text_set));
 *      ewl_view_header_fetch_set(view, test_data_header_fetch);
 *      ewl_tree2_column_append(EWL_TREE2(tree), model, view);
 *
 *      // create a view for the second column that just has an ewl image
 *      view = ewl_view_new();
 *      ewl_view_constructor_set(view, ewl_image_new);
 *      ewl_view_assign_set(view, EWL_VIEW_ASSIGN(ewl_image_file_path_set));
 *      ewl_view_header_fetch_set(view, test_data_header_fetch);
 *      ewl_tree2_column_append(EWL_TREE2(tree), model, view);
 *
 *      // we don't want this one sortable
 *      model = ewl_model_new();
 *      ewl_model_fetch_set(model, test_data_fetch);
 *      ewl_model_count_set(model, test_data_count_get);
 *
 *      // create a view for the third column that has a custom widget
 *      view = ewl_view_new();
 *      ewl_view_constructor_set(view, test_custom_new);
 *      ewl_view_assign_set(view, test_custom_assign_set);
 *      ewl_view_header_fetch_set(view, test_data_header_fetch);
 *      ewl_tree2_column_append(EWL_TREE2(tree), model, view);
 *
 *      // create the checkbuttons for the top box
 *      o = ewl_checkbutton_new();
 *      ewl_button_label_set(EWL_BUTTON(o), "Scroll headers");
 *      ewl_container_child_append(EWL_CONTAINER(o2), o);
 *      ewl_callback_append(o, EWL_CALLBACK_CLICKED,
 *                  cb_scroll_headers, tree);
 *      ewl_widget_show(o);
 *
 *      o = ewl_checkbutton_new();
 *      ewl_button_label_set(EWL_BUTTON(o), "Scroll visible");
 *      ewl_container_child_append(EWL_CONTAINER(o2), o);
 *      ewl_checkbutton_checked_set(EWL_CHECKBUTTON(o), TRUE);
 *      ewl_callback_append(o, EWL_CALLBACK_CLICKED,
 *                  cb_scroll_visible, tree);
 *      ewl_widget_show(o);
 *
 *      ewl_main();
 *      return 0;
 * }
 *
 * // setup our data
 * static void *
 * test_data_setup(void)
 * {
 *      Test_Data *data;
 *      Test_Row_Data **dt;
 *
 *      data = calloc(1, sizeof(Test_Data));
 *      dt = calloc(DATA_ELEMENTS, sizeof(Test_Row_Data *));
 *
 *      dt[0] = calloc(1, sizeof(Test_Row_Data));
 *      dt[0]->image = strdup("/usr/local/share/ewl/images/e-logo.png");
 *      dt[0]->text = strdup("The E logo");
 *
 *      dt[1] = calloc(1, sizeof(Test_Row_Data));
 *      dt[1]->image = strdup("/usr/local/share/ewl/images/elicit.png");
 *      dt[1]->text = strdup("The Elicit image");
 *
 *      dt[2] = calloc(1, sizeof(Test_Row_Data));
 *      dt[2]->image = strdup("/usr/local/share/ewl/images/entrance.png");
 *      dt[2]->text = strdup("The Entrance image");
 *
 *      dt[3] = calloc(1, sizeof(Test_Row_Data));
 *      dt[3]->image = strdup("/usr/local/share/ewl/images/End.png");
 *      dt[3]->text = strdup("Zebra");
 *
 *      dt[4] = calloc(1, sizeof(Test_Row_Data));
 *      dt[4]->image = strdup("/usr/local/share/ewl/images/banner-top.png");
 *      dt[4]->text = strdup("Ant");
 *
 *      data->rows = dt;
 *      data->count = DATA_ELEMENTS;
 *
 *      return data;
 * }
 *
 * static Ewl_Widget *
 * test_custom_new(void)
 * {
 *      Ewl_Widget *button;
 *
 *      button = ewl_button_new();
 *
 *      return button;
 * }
 *
 * static void
 * test_custom_assign_set(Ewl_Widget *w, void *data)
 * {
 *      Test_Row_Data *d;
 *
 *      d = data;
 *      ewl_button_label_set(EWL_BUTTON(w), d->text);
 *      ewl_button_image_set(EWL_BUTTON(w), d->image, NULL);
 * }
 *
 * static Ewl_Widget *
 * test_data_header_fetch(void *data , int column)
 * {
 *      Ewl_Widget *l;
 *
 *      l = ewl_label_new();
 *      if (column == 0)
 *          ewl_label_text_set(EWL_LABEL(l), "Title");
 *      else if (column == 1)
 *          ewl_label_text_set(EWL_LABEL(l), "Image");
 *      else
 *          ewl_label_text_set(EWL_LABEL(l), "Button");
 *      ewl_widget_show(l);
 *
 *      return l;
 * }
 *
 * static void *
 * test_data_fetch(void *data, unsigned int row, unsigned int column)
 * {
 *      Test_Data *d;
 *      void *val = NULL;
 *
 *      d = data;
 *
 *      if (column == 0)
 *          val = d->rows[row]->text;
 *
 *      else if (column == 1)
 *          val = d->rows[row]->image;
 *
 *      else if (column == 2)
 *          val = d->rows[row];
 *
 *      return val;
 * }
 *
 * static void
 * test_data_sort(void *data, unsigned int column, Ewl_Sort_Direction sort)
 * {
 *      Test_Data *d;
 *      int i;
 *
 *      // just leave it if we're in sort none.
 *      if (sort == EWL_SORT_DIRECTION_NONE)
 *          return;
 *
 *      d = data;
 *
 *      for (i = (DATA_ELEMENTS - 1); i >= 0; i--)
 *      {
 *          int j;
 *
 *          for (j = 1; j <= i; j++)
 *          {
 *              char *a, *b;
 *
 *              if (column == 0)
 *              {
 *                  a = d->rows[j - 1]->text;
 *                  b = d->rows[j]->text;
 *              }
 *              else
 *              {
 *                  a = d->rows[j - 1]->image;
 *                  b = d->rows[j]->image;
 *              }
 *
 *              if (((sort == EWL_SORT_DIRECTION_ASCENDING) && strcmp(a, b) > 0)
 *                      || ((sort == EWL_SORT_DIRECTION_DESCENDING)
 *                          && strcmp(a, b) < 0))
 *              {
 *                  char *temp;
 *
 *                  temp = d->rows[j - 1]->text;
 *                  d->rows[j - 1]->text = d->rows[j]->text;
 *                  d->rows[j]->text = temp;
 *
 *                  temp = d->rows[j - 1]->image;
 *                  d->rows[j - 1]->image = d->rows[j]->image;
 *                  d->rows[j]->image = temp;
 *              }
 *          }
 *      }
 * }
 *
 * static int
 * test_data_count_get(void *data)
 * {
 *      Test_Data *d;
 *
 *      d = data;
 *
 *      return d->count;
 * }
 *
 * static void
 * cb_delete_window(Ewl_Widget *w, void *ev, void *data)
 * {
 *      ewl_widget_destroy(w);
 *      ewl_main_quit();
 * }
 *
 * static void
 * cb_scroll_headers(Ewl_Widget *w, void *ev , void *data)
 * {
 *      Ewl_Tree2 *tree;
 *
 *      tree = data;
 *      ewl_tree2_scroll_headers_set(tree,
 *              ewl_checkbutton_is_checked(EWL_CHECKBUTTON(w)));
 * }
 *
 * static void
 * cb_scroll_visible(Ewl_Widget *w, void *ev , void *data)
 * {
 *      Ewl_Tree2 *tree;
 *
 *      tree = data;
 *      ewl_tree2_scroll_visible_set(tree,
 *              ewl_checkbutton_is_checked(EWL_CHECKBUTTON(w)));
 * }
 * @endcode
 *
 * Simple enough, eh? Ok, maybe I should go through it then.
 *
 * @code
 * #include <Ewl.h>
 * #include <stdio.h>
 * #include <stdlib.h>
 * #include <string.h>
 * @endcode
 *
 * We start off with the standard set of includes. Ewl.h is obviously required
 * to do any EWL programming. The others are needed as well be using functions
 * they define throughout the application.
 *
 * @code
 * #define DATA_ELEMENTS 5
 *
 * typedef struct Test_Row_Data Test_Row_Data;
 * struct Test_Row_Data
 * {
 *      char *image;
 *      char *text;
 * };
 *
 * typedef struct Test_Data Test_Data;
 * struct Test_Data
 * {
 *      unsigned int count;
 *      Test_Row_Data **rows;
 * };
 * @endcode
 *
 * I'm not planning on doing anything fancy with my data. Just keeping an array
 * with five elements. Im using a define DATA_ELEMENTS to store the number of
 * elements as Ill be using this in a few places. The data will be stored in a
 * Test_Data structure. This struct will store the number of items in the array
 * and an array of Test_Row_Data pointers. Test_Row_Data structs just store the
 * text and image strings for each of our rows.
 *
 * @code
 * static void *test_data_setup(void);
 *
 * static Ewl_Widget *test_custom_new(void);
 * static void test_custom_assign_set(Ewl_Widget *w, void *data);
 *
 * static Ewl_Widget *test_data_header_fetch(void *data, int column);
 * static void *test_data_fetch(void *data, unsigned int row, unsigned int column);
 * static void test_data_sort(void *data, unsigned int column, Ewl_Sort_Direction sort);
 * static int test_data_count_get(void *data);
 *
 * static void cb_delete_window(Ewl_Widget *w, void *ev, void *data);
 * static void cb_scroll_headers(Ewl_Widget *w, void *ev, void *data);
 * static void cb_scroll_visible(Ewl_Widget *w, void *ev, void *data);
 * @endcode
 *
 * As you can see, a bunch of pre-declarations next. Well be seeing, and getting
 * the explanation for these as we go along.
 *
 * @code
 * int
 * main(int argc, char ** argv)
 * {
 *      Ewl_Widget *tree, *box, *o, *o2;
 *      Ewl_Model *model;
 *      Ewl_View *view;
 *      void *data;
 *
 *      // make sure we can setup ewl
 *      if (!ewl_init(&argc, argv))
 *      {
 *          fprintf(stderr, "Unable to init ewl.n");
 *          return 1;
 *      }
 * @endcode
 *
 * The first step in any EWL application is to initialize EWL itself. This is
 * done with a call to ewl_init(). ewl_init() accepts two parameters, the argc
 * and argv arguments that were passed to your application. ewl_init() will
 * return TRUE if EWL was successfully initialized or FALSE otherwise. The
 * reason to pass the args to ewl_init() is so that EWL can parse out any EWL
 * specific arguments. Things like setting the rendering engine or printing the
 * EWL help documentation. These parameters can both be safely set to NULL if
 * desired.
 *
 * With EWL setup we can get down the the fun bit of creating the UI.
 *
 * @code
 *      // create the window
 *      o = ewl_window_new();
 *      ewl_window_title_set(EWL_WINDOW(o), "tree2 example");
 *      ewl_window_class_set(EWL_WINDOW(o), "tree2_example");
 *      ewl_window_name_set(EWL_WINDOW(o), "tree2_example");
 *      ewl_object_size_request(EWL_OBJECT(o), 640, 480);
 *      ewl_callback_append(o, EWL_CALLBACK_DELETE_WINDOW, cb_delete_window, NULL);
 *      ewl_widget_show(o);
 *
 *      box = ewl_vbox_new();
 *      ewl_container_child_append(EWL_CONTAINER(o), box);
 *      ewl_widget_show(box);
 *
 *      o2 = ewl_hbox_new();
 *      ewl_container_child_append(EWL_CONTAINER(box), o2);
 *      ewl_object_fill_policy_set(EWL_OBJECT(o2), EWL_FLAG_FILL_VSHRINK | EWL_FLAG_FILL_HFILL);
 *      ewl_widget_show(o2);
 * @endcode
 *
 * This little chunks sets up our base UI. We create the Ewl_Window first using
 * ewl_window_new(). We set the title, class and name for the window and then
 * give it a default size of 640�480. Once this is done we set a callback for
 * when the window is destroyed. This is done with ewl_callback_append() call.
 * We want to get notified when the window receives the
 * EWL_CALLBACK_DELETE_WINDOW callback by having the cb_delete_window() function
 * executed.
 *
 * With the main window setup we create an Ewl_Box inside of it. An Ewl_Window
 * by default has no layout policy so if you pack several widgets into it theyll
 * all be sitting on top of each other. You have to pack a box, or something, in
 * there to handle the layout of the contents. Youll see we use
 * ewl_container_child_append() to add the box to the window. Youll be seeing
 * this a lot as we pack the UI together. There are also
 * ewl_container_child_prepend() and ewl_container_child_insert() calls that can
 * be used.
 *
 * We also create a second Ewl_Box to hold some checkbuttons that well create
 * later. Were creating the box here just because I like to keep things order.
 * Its the first item in the window so I create and pack it first. We could also
 * use ewl_container_child_prepend() later to add it to the box if we wished.
 * Were setting a custom fill policy on this box with
 * ewl_object_fill_policy_set(). The policy were setting is
 * EWL_FLAG_FILL_VSHIRNK | EWL_FLAG_FILL_HFILL. Were telling the widget that we
 * want it to be as small as possible vertically but take up as much space as
 * possible horizontally. The fill policy is just a bit mask so were bit wise
 * oring the values together.
 *
 * I guess a quick note about EWL inheritance is necessary at some point. EWL
 * uses an object oriented approach to its widgets. All widgets inherit from
 * Ewl_Widget. Ewl_Widget inherits from Ewl_Object. So any ewl_widget_* or
 * ewl_object_* call can be called on any widgets. You just need to cast to the
 * correct type first. The casts are done with the EWL_WIDGET() or EWL_OBJECT()
 * macros. All widgets in EWL have an EWL_WIDGET_NAME() macro. Theres a lot of
 * inheritance going on in EWL. The Ewl_Box code inherits from the Ewl_Container
 * code. So any ewl_container_* call will work on an Ewl_Box, you just have to
 * wrap the box variable with an EWL_CONTAINER() call. Take a look at the EWL
 * docs for a complete listing of the inheritance. (Or if youre looking at the
 * header files the inheritance is always the first item in the
 * struct.)
 *
 * @code
 *      // create our data
 *      data = test_data_setup();
 * @endcode
 *
 * This is just a convenience function to create our data array. I like to keep
 * stuff separated out whenever possible. Well see what this does later.
 *
 * @code
 *      // create the model that'll be used for the first two columns
 *      model = ewl_model_new();
 *      ewl_model_fetch_set(model, test_data_fetch);
 *      ewl_model_sort_set(model, test_data_sort);
 *      ewl_model_count_set(model, test_data_count_get);
 * @endcode
 *
 * Our first two columns in the tree will actually use the same data model. I
 * dont want the third column to be sortable so we need to use a slightly
 * different model (although its almost the same). We need to set at a of
 * minimum two pieces of data into the Ewl_Model. These are, the function to
 * fetch the model data, set with ewl_model_fetch_set(), and the function to get
 * a count of the number of rows of data, set with ewl_model_count_set(). Im
 * using the third ewl_model_sort_set() to set a sort function for the first two
 * columns. Each of these calls takes the model and a function pointer. You can
 * see the function signature at the top of the file. Well take a closer look at
 * these functions a bit later.
 *
 * @code
 *      tree = ewl_tree2_new();
 *      ewl_container_child_append(EWL_CONTAINER(box), tree);
 *      ewl_object_fill_policy_set(EWL_OBJECT(tree), EWL_FLAG_FILL_ALL);
 *      ewl_tree2_data_set(EWL_TREE2(tree), data);
 *      ewl_widget_show(tree);
 * @endcode
 *
 * Next we create the tree itself. The tree is packed into the box we created
 * earlier. We set the box with a fill policy of EWL_FLAG_FILL_ALL so it will
 * take as much, or as little space as necesary. We then set the data we created
 * into the tree with ewl_tree2_data_set() function. This data will be passed
 * around to our various functions as the tree does its work.
 *
 * @code
 *      // create a view for the first column that just has an ewl label
 *      view = ewl_view_new();
 *      ewl_view_constructor_set(view, ewl_label_new);
 *      ewl_view_assign_set(view, EWL_VIEW_ASSIGN(ewl_label_text_set));
 *      ewl_view_header_fetch_set(view, test_data_header_fetch);
 *      ewl_tree2_column_append(EWL_TREE2(tree), model, view);
 * @endcode
 *
 * Ok, with our tree created we can start to add columns. Weve already created
 * the Ewl_Model for the first two columns so we just need to create the views.
 *
 * As I mentioned before the first column is using Ewl_Label to display the text
 * of the data item. So, we create a new Ewl_View with ewl_view_new(). We then
 * set the constructor for this column with ewl_view_constructor_set(). This
 * will be used to create a new widget for the item in the column. Then we use
 * ewl_view_assign_set() to set the function that will be used to assign data
 * into our widget for a given cell. Tree columns need a header. Well set a
 * third callback function to get the header data for the tree. We use
 * ewl_view_header_fetch_set() to set this function.
 *
 * Once our view is created we call ewl_tree2_column_append() to create a new
 * column in the tree using our model and view.
 *
 * @code
 *      // create a view for the second column that just has an ewl image view =
 *      ewl_view_new();
 *      ewl_view_constructor_set(view, ewl_image_new);
 *      ewl_view_assign_set(view, EWL_VIEW_ASSIGN(ewl_image_file_path_set));
 *      ewl_view_header_fetch_set(view, test_data_header_fetch);
 *      ewl_tree2_column_append(EWL_TREE2(tree), model, view);
 * @endcode
 *
 * The second column in the tree is created the same as the first except we use
 * the Ewl_Image functions instead of the Ewl_Label functions. You can use any
 * widget within EWL to setup a view. This gives the tree the flexibility of
 * being able to display any desired widget, even custom designed widgets.
 *
 * @code
 *      // we don't want this one sortable
 *      model = ewl_model_new();
 *      ewl_model_fetch_set(model, test_data_fetch);
 *      ewl_model_count_set(model, test_data_count_get);
 *
 *      // create a view for the third column that has a custom widget
 *      view = ewl_view_new();
 *      ewl_view_constructor_set(view, test_custom_new);
 *      ewl_view_assign_set(view, test_custom_assign_set);
 *      ewl_view_header_fetch_set(view, test_data_header_fetch);
 *      ewl_tree2_column_append(EWL_TREE2(tree), model, view);
 * @endcode
 *
 * The third column in the tree is similar to the first two except we dont want
 * it sortable. So, we create a new Ewl_Model and set the fetch and count
 * functions as before but skip the sort function. We then create the view and
 * use our custom constructor and assignment functions instead of a standard EWL
 * set. With the model and view created we append the column into the tree.
 *
 * @code
 *      // create the checkbuttons for the top box
 *      o = ewl_checkbutton_new();
 *      ewl_button_label_set(EWL_BUTTON(o), "Scroll headers");
 *      ewl_container_child_append(EWL_CONTAINER(o2), o);
 *      ewl_callback_append(o, EWL_CALLBACK_CLICKED, cb_scroll_headers, tree);
 *      ewl_widget_show(o);
 *
 *      o = ewl_checkbutton_new();
 *      ewl_button_label_set(EWL_BUTTON(o), "Scroll visible");
 *      ewl_container_child_append(EWL_CONTAINER(o2), o);
 *      ewl_checkbutton_checked_set(EWL_CHECKBUTTON(o), TRUE);
 *      ewl_callback_append(o, EWL_CALLBACK_CLICKED, cb_scroll_visible, tree);
 *      ewl_widget_show(o);
 * @endcode
 *
 * The final piece of the UI to be added are the two checkbuttons. These will be
 * used to change some settings of the tree so we can see some of the different
 * options. The both respond to EWL_CALLBACK_CLICKED callbacks the first calling
 * the cb_scroll_headers() function and the second calling cb_scroll_visible()
 * function.
 *
 * @code
 *      ewl_main();
 *      return 0;
 * }
 * @endcode
 *
 * Finally we call ewl_main() to kick off the main EWL event loop. When
 * ewl_main() is done then were finished so just return. ewl_main_quit() will
 * actually call ewl_shutdown() for us so we dont have to worry about it.
 *
 * Ok, with our UI out of the way all we need to deal with are all the
 * functions weve referenced. First up is creating our data. For this
 * example were just using an array but the thing to keep in mind is you can use
 * anything to store your data. EWL never accesses this data directly it always
 * does it through the calls you specified. So, if you want to use an Evas_List
 * or an Ecore_List or a tree or some other structure its up to you. EWL doesnt
 * care.
 *
 * @code
 * // setup our data
 * static void *
 * test_data_setup(void)
 * {
 *          Test_Data *data;
 *          Test_Row_Data **dt;
 *
 *          data = calloc(1, sizeof(Test_Data));
 *          dt = calloc(DATA_ELEMENTS, sizeof(Test_Row_Data *));
 *
 *          dt[0] = calloc(1, sizeof(Test_Row_Data));
 *          dt[0]->image = strdup("/usr/local/share/ewl/images/e-logo.png");
 *          dt[0]->text = strdup("The E logo");
 *
 *          dt[1] = calloc(1, sizeof(Test_Row_Data));
 *          dt[1]->image = strdup("/usr/local/share/ewl/images/elicit.png");
 *          dt[1]->text = strdup("The Elicit image");
 *
 *          dt[2] = calloc(1, sizeof(Test_Row_Data));
 *          dt[2]->image = strdup("/usr/local/share/ewl/images/entrance.png");
 *          dt[2]->text = strdup("The Entrance image");
 *
 *          dt[3] = calloc(1, sizeof(Test_Row_Data));
 *          dt[3]->image = strdup("/usr/local/share/ewl/images/End.png");
 *          dt[3]->text = strdup("Zebra");
 *
 *          dt[4] = calloc(1, sizeof(Test_Row_Data));
 *          dt[4]->image = strdup("/usr/local/share/ewl/images/banner-top.png");
 *          dt[4]->text = strdup("Ant");
 *
 *          data->rows = dt;
 *          data->count = DATA_ELEMENTS;
 *
 *          return data;
 * }
 * @endcode
 *
 * Nothing fancy in there so Im not going to bother explaining it.
 *
 * @code
 * static Ewl_Widget *
 * test_custom_new(void)
 * {
 *      Ewl_Widget *button;
 *      button = ewl_button_new();
 *      return button;
 * }
 * @endcode
 *
 * As I mentioned for our third column were using a custom constructor and
 * assignment calls. The reason for this, since we just want a simple button, is
 * that we want to set two pieces of data into the widget instead of just one.
 * We could actually just use ewl_button_new() in the view and have a custom
 * assignment function, but this makes for a better example. For the constructor
 * all we do is create our widget and return it. Its as simple as that.
 *
 * @code
 * static void
 * test_custom_assign_set(Ewl_Widget *w, void *data)
 * {
 *      Test_Row_Data *d;
 *
 *      d = data;
 *      ewl_button_label_set(EWL_BUTTON(w), d->text);
 *      ewl_button_image_set(EWL_BUTTON(w), d->image, NULL);
 * }
 * @endcode
 *
 * For the assignment part of the custom widget EWL will provide the widget
 * created with the view, w, and the piece of data that the rows model returned
 * for the current cell in the tree, data. Using these we can setup the widget
 * however we see fit. In this case were setting the label and image of the
 * button.
 *
 * @code
 * static Ewl_Widget *
 * test_data_header_fetch(void *data , int column)
 * {
 *      Ewl_Widget *l;
 *
 *      l = ewl_label_new();
 *      if (column == 0)
 *          ewl_label_text_set(EWL_LABEL(l), "Title");
 *      else if (column == 1)
 *          ewl_label_text_set(EWL_LABEL(l), "Image");
 *      else
 *          ewl_label_text_set(EWL_LABEL(l), "Button");
 *
 *      ewl_widget_show(l);
 *
 *      return l;
 * }
 * @endcode
 *
 * The last view function we need to worry about is
 * test_data_header_fetch(). EWL will call this function for each column to get
 * the widget to display in the header. The data pointer is the data set on the
 * tree itself and the column is the column number to return the header for
 * (column numbers start at 0).
 *
 * In this case were just creating an Ewl_Label and setting and appropriate bit
 * of text to describe the column.
 *
 * Not too bad so far, right?
 *
 * With the view code out of the way lets move on to model code. We start with
 * the fetch function. This is called whenever EWL needs to know what
 * information to pass to the views assign function.
 *
 * @code
 * static void *
 * test_data_fetch(void *data, unsigned int row, unsigned int column)
 * {
 *      Test_Data *d;
 *      void *val = NULL;
 *
 *      d = data;
 *
 *      if (column == 0)
 *          val = d->rows[row]->text;
 *      else if (column == 1)
 *          val = d->rows[row]->image;
 *      else if (column == 2)
 *          val = d->rows[row]; 
 *
 *      return val;
 * }
 * @endcode
 *
 * The fetch function returns a void * as EWL is making no assumptions about
 * what type of data youll need to pass into your assign function. In this case
 * we return a char * for the first two columns and a Tree_Row_Data * struct for
 * the third column. The fetch function is passed the data set into the tree,
 * the row and the column that we are interested in.
 *
 * If youve set a sort function into your model then the tree headers become
 * clickable. When the user clicks a header the sort function is called for the
 * given column. With this app Im just using a simple bubble sort. Youll
 * possibly want to use something a bit better for a real application.
 *
 * @code
 * static void
 * test_data_sort(void *data, unsigned int column, Ewl_Sort_Direction sort)
 * {
 *      Test_Data *d;
 *      int i;
 *
 *      // just leave it if we're in sort none.
 *      if (sort == EWL_SORT_DIRECTION_NONE)
 *          return;
 *
 *      d = data;
 *
 *      for (i = (DATA_ELEMENTS - 1); i >= 0; i--)
 *      {
 *          int j;
 *
 *          for (j = 1; j <= i; j++)
 *          {
 *              char *a, *b;
 *
 *              if (column == 0)
 *              {
 *                  a = d->rows[j - 1]->text;
 *                  b = d->rows[j]->text;
 *              }
 *              else
 *              {
 *                  a = d->rows[j - 1]->image;
 *                  b = d->rows[j]->image;
 *              }
 *
 *              if (((sort == EWL_SORT_DIRECTION_ASCENDING) && strcmp(a, b) > 0)
 *                      || ((sort == EWL_SORT_DIRECTION_DESCENDING)
 *                          && strcmp(a, b) < 0))
 *              {
 *                  char *temp;
 *
 *                  temp = d->rows[j - 1]->text;
 *                  d->rows[j - 1]->text = d->rows[j]->text;
 *                  d->rows[j]->text = temp;
 *
 *                  temp = d->rows[j - 1]->image;
 *                  d->rows[j - 1]->image = d->rows[j]->image;
 *                  d->rows[j]->image = temp;
 *              }
 *          }
 *      }
 * }
 * @endcode
 *
 * The sort call has three parameters. The the data we set on the tree, the
 * column number that we are sorting and the direction of the sort. The three
 * possible sort directions are EWL_SORT_DIRECTION_NONE,
 * EWL_SORT_DIRECTION_ASCENDING and EWL_SORT_DIRECTION_DESCENDING. In this case,
 * we dont bother doing anything if we are set to a sort of
 * EWL_SORT_DIRECTION_NONE. We then sort the data as needed in either ascending
 * or descending order based on the sort parameter.
 *
 * @code
 * static int
 * test_data_count_get(void *data)
 * {
 *      Test_Data *d;
 *
 *      d = data;
 *
 *      return d->count;
 * }
 * @endcode
 *
 * The last model function we need to implement is the count function. This is
 * just a way for your application to tell EWL how many rows are in your data.
 * In this case we just return the count parameter.
 *
 * Thats it for the model code. All thats left are the three callbacks we
 * defined for the UI.
 *
 * @code
 * static void
 * cb_delete_window(Ewl_Widget *w, void *ev, void *data)
 * {
 *      ewl_widget_destroy(w);
 *      ewl_main_quit();
 * }
 * @endcode
 *
 * cb_delete_window() will be called when the window is deleted. All we do is
 * destroy the window with a call to ewl_widget_destroy() and quit the
 * application with ewl_main_quit(). Because we attached this callback to the
 * Ewl_Window widget the first parameter *w is our window. You dont technically
 * need to ewl_widget_destroy() the window but its nice to clean up after
 * yourself.
 *
 * @code
 * static void
 * cb_scroll_headers(Ewl_Widget *w, void *ev , void *data)
 * {
 *      Ewl_Tree2 *tree;
 *
 *      tree = data;
 *      ewl_tree2_scroll_headers_set(tree, ewl_checkbutton_is_checked(EWL_CHECKBUTTON(w)));
 * }
 * @endcode
 *
 * The cb_scroll_headers() function toggles if the headers in the tree should be
 * scrolled. This is done by calling ewl_tree2_scroll_headers_set() and passing
 * either TRUE or FALSE depending on if we want the headers scrolled. This is
 * conveniently the same as the return of ewl_checkbutton_is_checked().
 *
 * @code
 * static void
 * cb_scroll_visible(Ewl_Widget *w, void *ev , void *data)
 * {
 *      Ewl_Tree2 *tree;
 *
 *      tree = data;
 *      ewl_tree2_scroll_visible_set(tree, ewl_checkbutton_is_checked(EWL_CHECKBUTTON(w)));
 * }
 * @endcode
 *
 * The cb_scroll_visible() function works in the same way as the
 * cb_scroll_headers() function except that it either enables or disables the
 * display of the scrollbars in the tree.
 *
 * With that, were at the end of our code. If you use the compilation line given
 * above everything should work out and when you run the app you should seeing
 * something similar too:
 *
 * Hopefully that wasnt too painful and you can see how easily it is to work
 * with the new Ewl_Tree2 code.
 *
 * Oh, before I forget. If you're updating your model and you want EWL to redraw
 * the tree you just need to call ewl_tree2_dirty_set() and pass TRUE as the
 * second parameter. This will signal EWL that something has changed in the
 * model and the tree will be redrawn.
 */

#define TREE2_DATA_ELEMENTS 5

typedef struct Tree2_Test_Row_Data Tree2_Test_Row_Data;
struct Tree2_Test_Row_Data
{
	char *image;
	char *text;
	Tree2_Test_Row_Data *subdata;

	int expandable;
	Tree2_Test_Row_Data **rows;
};

typedef struct Tree2_Test_Data Tree2_Test_Data;
struct Tree2_Test_Data
{
	unsigned int count;
	Tree2_Test_Row_Data **rows;
};

static int create_test(Ewl_Container *win);
static void *tree2_test_data_setup(void);
static Ewl_Widget *tree2_test_custom_new(void);
static void tree2_test_custom_assign_set(Ewl_Widget *w, void *data);
static Ewl_Widget *tree2_test_data_header_fetch(void *data, 
						int column);
static void *tree2_test_data_fetch(void *data, unsigned int row, 
						unsigned int column);
static void tree2_test_data_sort(void *data, unsigned int column, 
						Ewl_Sort_Direction sort);
static int tree2_test_data_count_get(void *data);
static int tree2_test_data_expandable_get(void *data, unsigned int row);
static void *tree2_test_data_subfetch(void *data, unsigned int parent_row,
							unsigned int subrow, 
							unsigned int column);

static void ewl_tree2_cb_scroll_headers(Ewl_Widget *w, void *ev, void *data);
static void ewl_tree2_cb_plain_view(Ewl_Widget *w, void *ev, void *data);
static void ewl_tree2_cb_set_rows_clicked(Ewl_Widget *w, void *ev, void *data);
static void tree2_cb_value_changed(Ewl_Widget *w, void *ev, void *data);
static void tree2_cb_select_mode_change(Ewl_Widget *w, void *ev, void *data);

void 
test_info(Ewl_Test *test)
{
	test->name = "Tree2";
	test->tip = "Defines a widget for laying out other\n"
			"widgets in a tree or list like manner.";
	test->filename = __FILE__;
	test->func = create_test;
	test->type = EWL_TEST_TYPE_CONTAINER;
}

static int
create_test(Ewl_Container *box)
{
	Ewl_Widget *tree, *o, *o2;
	Ewl_Model *model;
	Ewl_View *view;
	void *data;

	o2 = ewl_hbox_new();
	ewl_container_child_append(box, o2);
	ewl_object_fill_policy_set(EWL_OBJECT(o2), 
				EWL_FLAG_FILL_VSHRINK | EWL_FLAG_FILL_HFILL);
	ewl_widget_show(o2);

	/* create our data */
	data = tree2_test_data_setup();

	/* the tree will only use one model. We could use a model per
	 * column, but a single model will work fine for this test */
	model = ewl_model_new();
	ewl_model_fetch_set(model, tree2_test_data_fetch);
	ewl_model_sort_set(model, tree2_test_data_sort);
	ewl_model_count_set(model, tree2_test_data_count_get);
	ewl_model_expandable_set(model, tree2_test_data_expandable_get);
	ewl_model_subfetch_set(model, tree2_test_data_subfetch);

	tree = ewl_tree2_new();
	ewl_container_child_append(EWL_CONTAINER(box), tree);
	ewl_object_fill_policy_set(EWL_OBJECT(tree), EWL_FLAG_FILL_ALL);
	ewl_callback_append(tree, EWL_CALLBACK_VALUE_CHANGED,
					tree2_cb_value_changed, NULL);
	ewl_mvc_data_set(EWL_MVC(tree), data);
	ewl_mvc_selection_mode_set(EWL_MVC(tree), EWL_SELECTION_MODE_MULTI);
	ewl_widget_name_set(tree, "tree");
	ewl_widget_show(tree);

	/* create a view for the first column that just has an ewl label */
	view = ewl_label_view_get();
	ewl_view_header_fetch_set(view, tree2_test_data_header_fetch);
	ewl_tree2_column_append(EWL_TREE2(tree), model, view);

	/* create a view for the second column that just has an ewl image */
	view = ewl_view_new();
	ewl_view_constructor_set(view, ewl_image_new);
	ewl_view_assign_set(view, EWL_VIEW_ASSIGN(ewl_image_file_path_set));
	ewl_view_header_fetch_set(view, tree2_test_data_header_fetch);
	ewl_tree2_column_append(EWL_TREE2(tree), model, view);

	/* we don't want this one sortable */
	model = ewl_model_new();
	ewl_model_fetch_set(model, tree2_test_data_fetch);
	ewl_model_count_set(model, tree2_test_data_count_get);

	/* create a view for the third column that has a custom widget */
	view = ewl_view_new();
	ewl_view_constructor_set(view, tree2_test_custom_new);
	ewl_view_assign_set(view, tree2_test_custom_assign_set);
	ewl_view_header_fetch_set(view, tree2_test_data_header_fetch);
	ewl_tree2_column_append(EWL_TREE2(tree), model, view);

	/* create the checkbuttons for the top box */
	o = ewl_checkbutton_new();
	ewl_object_alignment_set(EWL_OBJECT(o), EWL_FLAG_ALIGN_CENTER);
	ewl_button_label_set(EWL_BUTTON(o), "Scroll headers");
	ewl_container_child_append(EWL_CONTAINER(o2), o);
	ewl_callback_append(o, EWL_CALLBACK_CLICKED, 
				ewl_tree2_cb_scroll_headers, tree);
	ewl_widget_show(o);

	o = ewl_checkbutton_new();
	ewl_object_alignment_set(EWL_OBJECT(o), EWL_FLAG_ALIGN_CENTER);
	ewl_button_label_set(EWL_BUTTON(o), "Plain view");
	ewl_container_child_append(EWL_CONTAINER(o2), o);
	ewl_callback_append(o, EWL_CALLBACK_CLICKED,
				ewl_tree2_cb_plain_view, tree);
	ewl_widget_show(o);

	o = ewl_spinner_new();
	ewl_object_alignment_set(EWL_OBJECT(o), EWL_FLAG_ALIGN_CENTER);
	ewl_container_child_append(EWL_CONTAINER(o2), o);
	ewl_spinner_digits_set(EWL_SPINNER(o), 0);
	ewl_range_minimum_value_set(EWL_RANGE(o), 0);
	ewl_range_maximum_value_set(EWL_RANGE(o), 10000);
	ewl_range_value_set(EWL_RANGE(o), 5);
	ewl_range_step_set(EWL_RANGE(o), 1);
	ewl_widget_name_set(o, "rows_spinner");
	ewl_widget_show(o);

	o = ewl_button_new();
	ewl_object_alignment_set(EWL_OBJECT(o), EWL_FLAG_ALIGN_CENTER);
	ewl_button_label_set(EWL_BUTTON(o), "Set number of rows");
	ewl_container_child_append(EWL_CONTAINER(o2), o);
	ewl_callback_append(o, EWL_CALLBACK_CLICKED, 
				ewl_tree2_cb_set_rows_clicked, NULL);
	ewl_widget_show(o);

	o = ewl_button_new();
	ewl_object_alignment_set(EWL_OBJECT(o), EWL_FLAG_ALIGN_CENTER);
	ewl_button_label_set(EWL_BUTTON(o), "Row select");
	ewl_container_child_append(EWL_CONTAINER(o2), o);
	ewl_callback_append(o, EWL_CALLBACK_CLICKED, 
				tree2_cb_select_mode_change, NULL);
	ewl_widget_show(o);

	return 1;
}

static void *
tree2_test_data_setup(void)
{
	Tree2_Test_Data *data;
	Tree2_Test_Row_Data **dt;

	data = calloc(1, sizeof(Tree2_Test_Data));
	dt = calloc(TREE2_DATA_ELEMENTS, sizeof(Tree2_Test_Row_Data *));

	dt[0] = calloc(1, sizeof(Tree2_Test_Row_Data));
	dt[0]->image = strdup(PACKAGE_DATA_DIR"/ewl/images/e-logo.png");
	dt[0]->text = strdup("The E logo");
	dt[0]->expandable = 0;

	dt[1] = calloc(1, sizeof(Tree2_Test_Row_Data));
	dt[1]->image = strdup(PACKAGE_DATA_DIR"/ewl/images/elicit.png");
	dt[1]->text = strdup("The Elicit image");
	dt[1]->expandable = 1;

	dt[1]->subdata = calloc(1, sizeof(Tree2_Test_Row_Data));
	dt[1]->subdata->image = strdup(PACKAGE_DATA_DIR"/ewl/images/e-logo.png");
	dt[1]->subdata->text = strdup("The E logo");

	dt[2] = calloc(1, sizeof(Tree2_Test_Row_Data));
	dt[2]->image = strdup(PACKAGE_DATA_DIR"/ewl/images/entrance.png");
	dt[2]->text = strdup("The Entrance image");
	dt[2]->expandable = 0;

	dt[3] = calloc(1, sizeof(Tree2_Test_Row_Data));
	dt[3]->image = strdup(PACKAGE_DATA_DIR"/ewl/images/End.png");
	dt[3]->text = strdup("Zebra");
	dt[3]->expandable = 0;
	
	dt[4] = calloc(1, sizeof(Tree2_Test_Row_Data));
	dt[4]->image = strdup(PACKAGE_DATA_DIR"/ewl/images/banner-top.png");
	dt[4]->text = strdup("Ant");
	dt[4]->expandable = 0;

	data->rows = dt;
	data->count = TREE2_DATA_ELEMENTS;

	return data;
}

static Ewl_Widget *
tree2_test_custom_new(void)
{
	Ewl_Widget *button;

	button = ewl_button_new();
	ewl_widget_show(button);

	return button;
}

static void
tree2_test_custom_assign_set(Ewl_Widget *w, void *data)
{
	Tree2_Test_Row_Data *d;

	d = data;
	ewl_button_label_set(EWL_BUTTON(w), d->text);
	ewl_button_image_set(EWL_BUTTON(w), d->image, NULL);
}

static Ewl_Widget *
tree2_test_data_header_fetch(void *data __UNUSED__, int column)
{
	Ewl_Widget *l;

	l = ewl_label_new();
	if (column == 0)
		ewl_label_text_set(EWL_LABEL(l), "Title");
	else if (column == 1)
		ewl_label_text_set(EWL_LABEL(l), "Image");
	else
		ewl_label_text_set(EWL_LABEL(l), "Button");
	ewl_widget_show(l);

	return l;
}

static void *
tree2_test_data_fetch(void *data, unsigned int row, unsigned int column)
{
	Tree2_Test_Data *d;
	void *val = NULL;

	d = data;

	/* NOTE: this is just for testing purposes, should not be needed in a
	 * normal app */
	if (row >= d->count)
	{
		printf("Asking for too many rows\n");
		return NULL;
	}

	if (column == 0)
		val = d->rows[row % TREE2_DATA_ELEMENTS]->text;

	else if (column == 1)
		val = d->rows[row % TREE2_DATA_ELEMENTS]->image;

	else if (column == 2)
		val = d->rows[row % TREE2_DATA_ELEMENTS];

	else
	{
		/* NOTE: this is just for testing purposes, should not be
		 * needed in a normal app */
		printf("Unknown column %d\n", column);
	}

	return val;
}

static void
tree2_test_data_sort(void *data, unsigned int column, Ewl_Sort_Direction sort)
{
	int i;
	Tree2_Test_Data *d;

	/* just leave it if we're in sort none. */
	if (sort == EWL_SORT_DIRECTION_NONE)
		return;

	d = data;

	for (i = (TREE2_DATA_ELEMENTS - 1); i >= 0; i--)
	{
		int j;

		for (j = 1; j <= i; j++)
		{
			char *a, *b;

			if (column == 0)
			{
				a = d->rows[j - 1]->text;
				b = d->rows[j]->text;
			}
			else
			{
				a = d->rows[j - 1]->image;
				b = d->rows[j]->image;
			}

			if (((sort == EWL_SORT_DIRECTION_ASCENDING) && strcmp(a, b) > 0)
					|| ((sort == EWL_SORT_DIRECTION_DESCENDING) 
						&& strcmp(a, b) < 0))
			{
				char *temp;

				temp = d->rows[j - 1]->text;
				d->rows[j - 1]->text = d->rows[j]->text;
				d->rows[j]->text = temp;

				temp = d->rows[j - 1]->image;
				d->rows[j - 1]->image = d->rows[j]->image;
				d->rows[j]->image = temp;
			}
		}
	}
}

static int
tree2_test_data_count_get(void *data)
{
	Tree2_Test_Data *d;

	d = data;

	return d->count;
}

static int
tree2_test_data_expandable_get(void *data, unsigned int row)
{
	Tree2_Test_Data *d;
	int ret = FALSE;

	d = data;

	if (d && d->rows[row % TREE2_DATA_ELEMENTS])
		ret = d->rows[row % TREE2_DATA_ELEMENTS]->expandable;

	printf("Data %p row %d\n", d, row);
	return ret;
}

static void *
tree2_test_data_subfetch(void *data, unsigned int parent_row,
						unsigned int subrow, 
						unsigned int column)
{
	Tree2_Test_Data *d;
	void *val = NULL;

	d = data;

	if (column == 0)
		val = d->rows[parent_row % TREE2_DATA_ELEMENTS]->rows[subrow]->text;

	else if (column == 1)
		val = d->rows[parent_row % TREE2_DATA_ELEMENTS]->rows[subrow]->image;

	else if (column == 2)
		val = d->rows[parent_row % TREE2_DATA_ELEMENTS]->rows[subrow];

	return val;
}


static void
ewl_tree2_cb_scroll_headers(Ewl_Widget *w, void *ev __UNUSED__, void *data)
{
	Ewl_Tree2 *tree;
	Ewl_Widget *view;

	tree = data;
	view = ewl_tree2_view_widget_get(tree);

	if (ewl_widget_type_is(view, EWL_TREE2_VIEW_SCROLLED_TYPE))
		ewl_tree2_view_scrolled_scroll_headers_set(EWL_TREE2_VIEW(view),
			ewl_checkbutton_is_checked(EWL_CHECKBUTTON(w)));
}

static void
ewl_tree2_cb_plain_view(Ewl_Widget *w, void *ev __UNUSED__, void *data)
{
	Ewl_Tree2 *tree;
	Ewl_View *view;

	tree = data;
	if (ewl_checkbutton_is_checked(EWL_CHECKBUTTON(w)))
		view = ewl_tree2_view_plain_get();
	else
		view = ewl_tree2_view_scrolled_get();

	ewl_mvc_view_set(EWL_MVC(tree), view);
}

static void
ewl_tree2_cb_set_rows_clicked(Ewl_Widget *w __UNUSED__, void *ev __UNUSED__, 
						void *data __UNUSED__)
{
	Ewl_Widget *spinner, *tree;
	Tree2_Test_Data *d;

	tree = ewl_widget_name_find("tree");
	spinner = ewl_widget_name_find("rows_spinner");

	d = ewl_mvc_data_get(EWL_MVC(tree));
	d->count = ewl_range_value_get(EWL_RANGE(spinner));

	ewl_mvc_dirty_set(EWL_MVC(tree), TRUE);
}

static void
tree2_cb_value_changed(Ewl_Widget *w, void *ev __UNUSED__, 
					void *data __UNUSED__)
{
	Ecore_List *selected;
	Ewl_Selection *sel;

	printf("Selected:\n");
	selected = ewl_mvc_selected_list_get(EWL_MVC(w));
	ecore_list_goto_first(selected);
	while ((sel = ecore_list_next(selected)))
	{
		if (sel->type == EWL_SELECTION_TYPE_INDEX)
		{
			Ewl_Selection_Idx *idx;

			idx = EWL_SELECTION_IDX(sel);
			printf("    %d %d\n", idx->row, idx->column);
		}
		else
		{
			Ewl_Selection_Range *idx;
			int i, k;

			idx = EWL_SELECTION_RANGE(sel);
			for (i = idx->start.row; i <= idx->end.row; i++)
			{
				for (k = idx->start.column; k <=
							idx->end.column; k++)
					printf("    %d %d\n", i, k);
			}
		}
	}
}

static void
tree2_cb_select_mode_change(Ewl_Widget *w __UNUSED__, void *ev __UNUSED__, 
						void *data __UNUSED__)
{
	Ewl_Widget *tree;

	tree = ewl_widget_name_find("tree");
	if (!strcmp(ewl_button_label_get(EWL_BUTTON(w)), "Row select"))
	{
		ewl_button_label_set(EWL_BUTTON(w), "Cell select");
		ewl_tree2_selection_type_set(EWL_TREE2(tree),
					EWL_TREE_SELECTION_TYPE_ROW);
	}
	else
	{
		ewl_button_label_set(EWL_BUTTON(w), "Row select");
		ewl_tree2_selection_type_set(EWL_TREE2(tree),
					EWL_TREE_SELECTION_TYPE_CELL);
	}
}

