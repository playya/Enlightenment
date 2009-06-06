/* vim: set sw=8 ts=8 sts=8 expandtab: */
#include "Ewl_Test.h"
#include "ewl_test_private.h"
#include "ewl_button.h"
#include "ewl_entry.h"
#include "ewl_filedialog.h"
#include "ewl_image.h"
#include "ewl_icon.h"
#include "ewl_toolbar.h"
#include "ewl_scrollpane.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Ecore_File.h>

static Ecore_DList    *images;

static Ewl_Widget     *image;
static Ewl_Widget     *entry_path;

static Ewl_Widget     *fd;
char                  *last_dir;

static void create_image_fd_window_response (Ewl_Widget *w, void *ev, void *data);
static void create_image_fd_cb(Ewl_Widget *w, void *ev_data, void *user_data);

static int create_test(Ewl_Container *box);

extern Ewl_Unit_Test image_unit_tests[];

void
test_info(Ewl_Test *test)
{
        test->name = "Image";
        test->tip = "Provides a widget for displaying evas\n"
                                "loadable images, and edjes.";
        test->filename = __FILE__;
        test->func = create_test;
        test->type = EWL_TEST_TYPE_SIMPLE;
        test->unit_tests = image_unit_tests;
}

static void
destroy_image_test(Ewl_Widget * w __UNUSED__, void *ev_data __UNUSED__,
                                                void *user_data __UNUSED__)
{
        ecore_dlist_destroy(images);
        if (last_dir)
                free(last_dir);
}

static void
image_goto_prev_cb(Ewl_Widget * w __UNUSED__, void *ev_data __UNUSED__,
                                        void *user_data __UNUSED__)
{
        char *img = NULL;

        ecore_dlist_previous(images);
        img = ecore_dlist_current(images);

        if (!img) img = ecore_dlist_last_goto(images);

        ewl_text_text_set(EWL_TEXT(entry_path), img);
        ewl_image_file_set(EWL_IMAGE(image), img, NULL);
}

static void
image_remove_cb(Ewl_Widget * w __UNUSED__, void *ev_data __UNUSED__,
                                        void *user_data __UNUSED__)
{
        char *img = NULL;

        ecore_dlist_remove_destroy(images);
        
        img = ecore_dlist_current(images);

        if (!img) img = ecore_dlist_last_goto(images);

        ewl_text_text_set(EWL_TEXT(entry_path), img);
        ewl_image_file_set(EWL_IMAGE(image), img, NULL);
}

static void
image_load(const char *img)
{
        if (img && ecore_file_exists(img)) {
                ecore_dlist_append(images, strdup(img));
                ecore_dlist_last_goto(images);
                ewl_image_file_set(EWL_IMAGE(image), img, NULL);
        } else
                printf("ERROR: %s does not exist\n", img);
}

static void
image_goto_next_cb(Ewl_Widget * w __UNUSED__, void *ev_data __UNUSED__,
                                        void *user_data __UNUSED__)
{
        char *img = NULL;

        ecore_dlist_next(images);
        img = ecore_dlist_current(images);

        if (!img)
                img = ecore_dlist_first_goto(images);

        ewl_text_text_set(EWL_TEXT(entry_path), img);
        ewl_image_file_set(EWL_IMAGE(image), img, NULL);
}

static void
image_cb_rotate_left(Ewl_Widget *w __UNUSED__, void *ev __UNUSED__,
                void *data __UNUSED__)
{
        if (image)
                ewl_image_rotate(EWL_IMAGE(image), EWL_ROTATE_CC_90);
}

static void
image_cb_rotate_right(Ewl_Widget *w __UNUSED__, void *ev __UNUSED__,
                void *data __UNUSED__)
{
        if (image)
                ewl_image_rotate(EWL_IMAGE(image), EWL_ROTATE_CW_90);
}


static void
image_cb_flip_horizontal(Ewl_Widget *w __UNUSED__, void *ev __UNUSED__,
                void *data __UNUSED__)
{
        if (image)
                ewl_image_flip(EWL_IMAGE(image), EWL_ORIENTATION_HORIZONTAL);
}

static void
image_cb_flip_vertical(Ewl_Widget *w __UNUSED__, void *ev __UNUSED__,
                void *data __UNUSED__)
{
        if (image)
                ewl_image_flip(EWL_IMAGE(image), EWL_ORIENTATION_VERTICAL);
}

void
entry_path_cb_value_changed(Ewl_Widget *w, void *ev_data __UNUSED__, 
                                                        void *data __UNUSED__)
{
        char *img;

        img = ewl_text_text_get(EWL_TEXT(w));
        image_load(img);
        free(img);
}

int
create_test(Ewl_Container *image_box)
{
        Ewl_Widget *scrollpane;
        Ewl_Widget *box;
        Ewl_Widget *button;
        Ewl_Widget *note;
        char *image_file = NULL;

        images = ecore_dlist_new();
        ecore_dlist_free_cb_set(images, free);

        box = ewl_htoolbar_new();
        ewl_container_child_append(image_box, box);
        ewl_toolbar_icon_part_hide(EWL_TOOLBAR(box), EWL_ICON_PART_LABEL);
        ewl_widget_show(box);
        
        /* the previous icon */
        button = ewl_icon_simple_new();
        ewl_stock_type_set(EWL_STOCK(button), EWL_STOCK_ARROW_LEFT);
        ewl_container_child_append(EWL_CONTAINER(box), button);
        ewl_callback_append(button, EWL_CALLBACK_CLICKED,
                            image_goto_prev_cb, NULL);
        ewl_widget_show(button);

        /* the next icon */
        button = ewl_icon_simple_new();
        ewl_stock_type_set(EWL_STOCK(button), EWL_STOCK_ARROW_RIGHT);
        ewl_container_child_append(EWL_CONTAINER(box), button);
        ewl_callback_append(button, EWL_CALLBACK_CLICKED,
                            image_goto_next_cb, NULL);
        ewl_widget_show(button);
        
        /* the remove icon */
        button = ewl_icon_simple_new();
        ewl_stock_type_set(EWL_STOCK(button), EWL_STOCK_REMOVE);
        ewl_container_child_append(EWL_CONTAINER(box), button);
        ewl_callback_append(button, EWL_CALLBACK_CLICKED,
                            image_remove_cb, NULL);
        ewl_widget_show(button);

        /* the rotate left icon */
        button = ewl_icon_simple_new();
        ewl_stock_type_set(EWL_STOCK(button), EWL_STOCK_ROTATE_LEFT);
        ewl_container_child_append(EWL_CONTAINER(box), button);
        ewl_callback_append(button, EWL_CALLBACK_CLICKED, image_cb_rotate_left,
                        NULL);
        ewl_widget_show(button);

        /* the rotate right icon */
        button = ewl_icon_simple_new();
        ewl_stock_type_set(EWL_STOCK(button), EWL_STOCK_ROTATE_RIGHT);
        ewl_container_child_append(EWL_CONTAINER(box), button);
        ewl_callback_append(button, EWL_CALLBACK_CLICKED, image_cb_rotate_right,
                        NULL);
        ewl_widget_show(button);

        /* the flip vertical icon */
        button = ewl_icon_simple_new();
        ewl_stock_type_set(EWL_STOCK(button), EWL_STOCK_FLIP_VERTICAL);
        ewl_container_child_append(EWL_CONTAINER(box), button);
        ewl_callback_append(button, EWL_CALLBACK_CLICKED,
                        image_cb_flip_vertical, NULL);
        ewl_widget_show(button);
        
        /* the flip horizontal icon */
        button = ewl_icon_simple_new();
        ewl_stock_type_set(EWL_STOCK(button), EWL_STOCK_FLIP_HORIZONTAL);
        ewl_container_child_append(EWL_CONTAINER(box), button);
        ewl_callback_append(button, EWL_CALLBACK_CLICKED,
                        image_cb_flip_horizontal, NULL);
        ewl_widget_show(button);
        
        entry_path = ewl_entry_new();
        ewl_container_child_append(EWL_CONTAINER(box), entry_path);
        ewl_callback_append(entry_path, EWL_CALLBACK_VALUE_CHANGED,
                        entry_path_cb_value_changed, NULL);
        ewl_widget_show(entry_path);

        button = ewl_icon_simple_new();
        ewl_stock_type_set(EWL_STOCK(button), EWL_STOCK_OPEN);
        ewl_callback_append(button, EWL_CALLBACK_CLICKED,
                            create_image_fd_cb, entry_path);
        ewl_container_child_append(EWL_CONTAINER(box), button);
        ewl_widget_show(button);

        scrollpane = ewl_scrollpane_new();
        ewl_callback_append(scrollpane, EWL_CALLBACK_DELETE_WINDOW,
                        destroy_image_test, NULL);
        ewl_container_child_append(image_box, scrollpane);
        ewl_widget_show(scrollpane);

        if ((ecore_file_exists(PACKAGE_DATA_DIR "/ewl/images/e-logo.png")))
                image_file = strdup(PACKAGE_DATA_DIR "/ewl/images/e-logo.png");
        else if ((ecore_file_exists(PACKAGE_SOURCE_DIR "/data/images/e-logo.png")))
                image_file = strdup(PACKAGE_SOURCE_DIR "/data/images/e-logo.png");
        else if ((ecore_file_exists("./data/images/e-logo.png")))
                image_file = strdup("./data/images/e-logo.png");
        else if ((ecore_file_exists("../data/images/e-logo.png")))
                image_file = strdup("../data/images/e-logo.png");

        /* now that we have the fullpath we need to update the entry */
        ewl_text_text_set(EWL_TEXT(entry_path), image_file);

        box = ewl_cell_new();
        ewl_container_child_append(EWL_CONTAINER(scrollpane), box);
        ewl_widget_show(box);

        image = ewl_image_new();
        ewl_image_file_set(EWL_IMAGE(image), image_file, NULL);
        ewl_object_padding_type_top_set(EWL_OBJECT(image), EWL_PADDING_MEDIUM);
        ewl_object_alignment_set(EWL_OBJECT(image), EWL_FLAG_ALIGN_CENTER);
        ewl_container_child_append(EWL_CONTAINER(box), image);
        ewl_widget_show(image);

        if (image_file)
                ecore_dlist_append(images, image_file);

        note = ewl_text_new();
        ewl_object_fill_policy_set(EWL_OBJECT(note), EWL_FLAG_FILL_HFILL);
        ewl_text_text_set(EWL_TEXT(note), "Simple image viewer, load"
                       " up images and page through them.");
        ewl_container_child_append(EWL_CONTAINER(image_box), note);
        ewl_widget_show(note);

        ewl_widget_show(image);

        return 1;
}

static void
create_image_fd_cb(Ewl_Widget *w __UNUSED__, void *ev_data __UNUSED__,
                                    void *user_data)
{
        if (fd)
                return;

        fd = ewl_filedialog_new();
        ewl_window_title_set(EWL_WINDOW(fd), "Select an Image...");
        ewl_window_name_set(EWL_WINDOW(fd), "EWL Image Test");
        ewl_window_class_set(EWL_WINDOW(fd), "EWL Filedialog");
        ewl_callback_append(fd, EWL_CALLBACK_VALUE_CHANGED,
                            create_image_fd_window_response, user_data);
        if (last_dir)
                ewl_filedialog_directory_set(EWL_FILEDIALOG(fd), last_dir);
        ewl_widget_show(fd);
}

static void
create_image_fd_window_response(Ewl_Widget *w, void *ev, void *data)
{
        Ewl_Event_Action_Response *e;
        Ewl_Widget *entry = data;

        e = ev;

        if (e->response == EWL_STOCK_OPEN) {
                char *filename;
                const char *dir;

                filename = ewl_filedialog_selected_file_get(EWL_FILEDIALOG (w));
                printf("File open from image test: %s\n", filename);
                if (filename) {
                        ewl_text_text_set(EWL_TEXT(entry), filename);
                        image_load(filename);
                        free (filename);
                }
                if (last_dir) {
                        free(last_dir);
                        last_dir = NULL;
                }
                
                dir = ewl_filedialog_directory_get(EWL_FILEDIALOG(fd));
                if (dir)
                        last_dir = strdup(dir);
        }
        else {
                printf("Test program says bugger off.\n");
        }

        ewl_widget_destroy(fd);
        fd = NULL;
}
