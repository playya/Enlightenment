#include <stdio.h>
#include <Ewl.h>

void destroy_cb(Ewl_Widget *w, void *event, void *data) {
    ewl_widget_destroy(w);
    ewl_main_quit();
}

void text_update_cb(Ewl_Widget *w, void *event, void *data) {
    char *s = NULL;
    Ewl_Widget *label = NULL;
    char buf[BUFSIZ];

    s = ewl_entry_get_text(EWL_ENTRY(w));
    label = (Ewl_Widget *)data;

    snprintf(buf, BUFSIZ, "Hello %s", s);
    ewl_text_text_set(EWL_TEXT(label), buf);

    free(s);
    return;
}

int main(int argc, char ** argv) {
    Ewl_Widget *win = NULL;
    Ewl_Widget *box = NULL;
    Ewl_Widget *label = NULL;
    Ewl_Widget *o = NULL;

    /* init the library */
    if (!ewl_init(&argc, argv)) {
        printf("Unable to initialize EWL\n");
        return 1;
    }

    /* create the window */
    win = ewl_window_new();
    ewl_window_set_title(EWL_WINDOW(win), "Hello world");
    ewl_window_set_class(EWL_WINDOW(win), "hello");
    ewl_window_set_name(EWL_WINDOW(win), "hello");
    ewl_object_request_size(EWL_OBJECT(win), 200, 50);
    ewl_callback_append(win, EWL_CALLBACK_DELETE_WINDOW, destroy_cb, NULL);
    ewl_widget_show(win);
 
    /* create the container */
    box = ewl_vbox_new();
    ewl_container_append_child(EWL_CONTAINER(win), box);
    ewl_object_set_fill_policy(EWL_OBJECT(box), EWL_FLAG_FILL_ALL);
    ewl_widget_show(box);
 
    /* create text label */
    label = ewl_text_new(NULL);
    ewl_container_append_child(EWL_CONTAINER(box), label);
    ewl_object_set_alignment(EWL_OBJECT(label), EWL_FLAG_ALIGN_CENTER);
    ewl_text_style_set(EWL_TEXT(label), "soft_shadow");
    ewl_text_color_set(EWL_TEXT(label), 255, 0, 0, 255);
    ewl_text_text_set(EWL_TEXT(label), "Hello");
    ewl_widget_show(label);

    /* create the entry */ 
    o = ewl_entry_new("");
    ewl_container_append_child(EWL_CONTAINER(box), o);
    ewl_object_set_alignment(EWL_OBJECT(o), EWL_FLAG_ALIGN_CENTER);
    ewl_object_set_padding(EWL_OBJECT(o), 5, 5, 5, 0);
    ewl_text_color_set(EWL_TEXT(EWL_ENTRY(o)->text), 0, 0, 0, 255);
    ewl_callback_append(o, EWL_CALLBACK_VALUE_CHANGED, text_update_cb, label);
    ewl_widget_show(o);

    ewl_main();
    return 0;
}


