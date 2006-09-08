#include "ephoto.h"
Ecore_Timer *timer;

typedef struct _Slide_Config Slide_Config;

struct _Slide_Config
{
 Ewl_Widget *win;
 Ewl_Widget *vbox;
 Ewl_Widget *hbox;
 Ewl_Widget *window_border;
 Ewl_Widget *transition_border;
 Ewl_Widget *order_border;
 Ewl_Widget *fullscreen;
 Ewl_Widget *custom;
 Ewl_Widget *spinner;
 Ewl_Widget *show_name;
 Ewl_Widget *random;
 Ewl_Widget *loop;
 Ewl_Widget *text;
 Ewl_Widget *wentry;
 Ewl_Widget *hentry;
 Ewl_Widget *save;
 Ewl_Widget *cancel;
 int full_size;
 int custom_size;
 int length;
 int name_show;
 int random_order;
 int loop_slide;
 int w_size;
 int h_size;
};

void destroy_slideshow(Ewl_Widget *w, void *event, void *data)
{
 ecore_timer_del(timer);
 ewl_widget_destroy(w);
}

int change_picture(void *data)
{
 char *image_path;
 Ewl_Widget *w;
 
 w = data;
 ecore_dlist_next(current_thumbs);
 image_path = ecore_dlist_current(current_thumbs);
 if(image_path)
 {
  ewl_image_file_set(EWL_IMAGE(w), image_path, NULL);
 }
 else 
 {
  ecore_timer_del(timer);
  ewl_widget_destroy(w->parent->parent);
 }
}
 
void start_slideshow(Ewl_Widget *w, void *event, void *data)
{
 Ewl_Widget *window;
 Ewl_Widget *vbox;
 Ewl_Widget *image;
 char *image_path;
 
 image_path = ecore_dlist_goto_first(current_thumbs);
 
 if (!image_path) return;
 
 window = ewl_window_new();
 ewl_window_fullscreen_set(EWL_WINDOW(window), 1);
 ewl_callback_append(window, EWL_CALLBACK_DELETE_WINDOW, destroy_slideshow, NULL);
 ewl_callback_append(window, EWL_CALLBACK_CLICKED, destroy_slideshow, NULL); 
 ewl_widget_show(window);

 vbox = ewl_vbox_new();
 ewl_object_fill_policy_set(EWL_OBJECT(vbox), EWL_FLAG_FILL_ALL);
 ewl_container_child_append(EWL_CONTAINER(window), vbox);
 ewl_widget_show(vbox);

 image = ewl_image_new();
 ewl_image_file_set(EWL_IMAGE(image), image_path, NULL);
 ewl_object_fill_policy_set(EWL_OBJECT(image), EWL_FLAG_FILL_ALL);
 ewl_container_child_append(EWL_CONTAINER(vbox), image);
 ewl_widget_show(image);

 timer = ecore_timer_add(3, change_picture, image);
}

void config_cancel(Ewl_Widget *w, void *event, void *data)
{
 Ewl_Widget *win;
 win = data;
 ewl_widget_destroy(win);
}

void save_config(Ewl_Widget *w, void *event, void *data)
{
 FILE *file;
 Slide_Config *sc;
 char path[PATH_MAX];
 char temp[PATH_MAX];

 snprintf(path, PATH_MAX, "%s/.ephoto/slideshow_config", getenv("HOME"));
 sc = data;
 
 file = fopen(path, "w");
 if (file != NULL)
 {
  if (ewl_checkbutton_is_checked(EWL_CHECKBUTTON(sc->fullscreen)))
  {
   fputs("Fullscreen=1\n", file);
   fputs("Custom=0\n", file);
  }
  else
  {
   fputs("Fullscreen=0\n", file);
   fputs("Custom=1\n", file);
  }
  snprintf(temp, PATH_MAX, "Width=%s\n", ewl_text_text_get(EWL_TEXT(sc->wentry)));
  fputs(temp, file);
  snprintf(temp, PATH_MAX, "Height=%s\n", ewl_text_text_get(EWL_TEXT(sc->hentry)));
  fputs(temp, file);
  if (ewl_checkbutton_is_checked(EWL_CHECKBUTTON(sc->random)))
  {
   fputs("Random=1\n", file);
  }
  else
  {
   fputs("Random=0\n", file);
  }
  if (ewl_checkbutton_is_checked(EWL_CHECKBUTTON(sc->loop)))
  {
   fputs("Loop=1\n", file);
  }
  else
  {
   fputs("Loop=0\n", file);
  }
  snprintf(temp, PATH_MAX, "Length=%d\n", ewl_range_value_get(EWL_RANGE(sc->spinner)));
  fputs(temp, file);
  if (ewl_checkbutton_is_checked(EWL_CHECKBUTTON(sc->show_name)))
  {
   fputs("FileName=1\n", file);
  }
  else
  {
   fputs("FileName=0\n", file);
  }
 fclose(file);
 }
}
	
void create_slideshow_config(Ewl_Widget *w, void *event, void *data)
{
 Slide_Config *sc;
 sc = calloc(1, sizeof(Slide_Config));
 
 sc->win = ewl_window_new();
 ewl_window_title_set(EWL_WINDOW(sc->win), "Slideshow Configuration");
 ewl_window_name_set(EWL_WINDOW(sc->win), "Slideshow Configuration");
 ewl_window_dialog_set(EWL_WINDOW(sc->win), 1);
 ewl_object_size_request(EWL_OBJECT(sc->win), 400, 200);
 ewl_callback_append(sc->win, EWL_CALLBACK_DELETE_WINDOW, config_cancel, sc->win);
 ewl_widget_show(sc->win);

 sc->vbox = ewl_vbox_new();
 ewl_object_fill_policy_set(EWL_OBJECT(sc->vbox), EWL_FLAG_FILL_ALL);
 ewl_container_child_append(EWL_CONTAINER(sc->win), sc->vbox);
 ewl_widget_show(sc->vbox);

 sc->hbox = ewl_hbox_new();
 ewl_object_fill_policy_set(EWL_OBJECT(sc->hbox), EWL_FLAG_FILL_ALL);
 ewl_container_child_append(EWL_CONTAINER(sc->vbox), sc->hbox);
 ewl_object_alignment_set(EWL_OBJECT(sc->hbox), EWL_FLAG_ALIGN_CENTER);
 ewl_widget_show(sc->hbox);
      
 sc->window_border = ewl_border_new();
 ewl_border_text_set(EWL_BORDER(sc->window_border), "Window Size");
 ewl_border_label_alignment_set(EWL_BORDER(sc->window_border), EWL_FLAG_ALIGN_CENTER);
 ewl_container_child_append(EWL_CONTAINER(sc->hbox), sc->window_border);
 ewl_object_alignment_set(EWL_OBJECT(sc->window_border), EWL_FLAG_ALIGN_CENTER);
 ewl_widget_show(sc->window_border);

 sc->order_border = ewl_border_new();
 ewl_border_text_set(EWL_BORDER(sc->order_border), "Order");
 ewl_border_label_alignment_set(EWL_BORDER(sc->order_border), EWL_FLAG_ALIGN_CENTER);
 ewl_container_child_append(EWL_CONTAINER(sc->hbox), sc->order_border);
 ewl_object_alignment_set(EWL_OBJECT(sc->order_border), EWL_FLAG_ALIGN_CENTER);
 ewl_widget_show(sc->order_border);
 
 sc->transition_border = ewl_border_new();
 ewl_border_text_set(EWL_BORDER(sc->transition_border), "Transitions");
 ewl_border_label_alignment_set(EWL_BORDER(sc->transition_border), EWL_FLAG_ALIGN_CENTER);
 ewl_container_child_append(EWL_CONTAINER(sc->vbox), sc->transition_border);
 ewl_object_alignment_set(EWL_OBJECT(sc->transition_border), EWL_FLAG_ALIGN_CENTER);
 ewl_widget_show(sc->transition_border);
 
 sc->hbox = ewl_hbox_new();
 ewl_object_fill_policy_set(EWL_OBJECT(sc->hbox), EWL_FLAG_FILL_HFILL);
 ewl_container_child_append(EWL_CONTAINER(sc->window_border), sc->hbox);
 ewl_object_alignment_set(EWL_OBJECT(sc->hbox), EWL_FLAG_ALIGN_CENTER);
 ewl_widget_show(sc->hbox);

 sc->fullscreen = ewl_radiobutton_new();
 ewl_button_label_set(EWL_BUTTON(sc->fullscreen), "Fullscreen");
 ewl_container_child_append(EWL_CONTAINER(sc->hbox), sc->fullscreen);
 ewl_radiobutton_checked_set(EWL_RADIOBUTTON(sc->fullscreen), TRUE);
 ewl_object_alignment_set(EWL_OBJECT(sc->fullscreen), EWL_FLAG_ALIGN_LEFT);
 ewl_widget_show(sc->fullscreen);	
 
 sc->custom = ewl_radiobutton_new();
 ewl_button_label_set(EWL_BUTTON(sc->custom), "Custom");
 ewl_container_child_append(EWL_CONTAINER(sc->hbox), sc->custom);
 ewl_radiobutton_checked_set(EWL_RADIOBUTTON(sc->custom), FALSE);
 ewl_radiobutton_chain_set(EWL_RADIOBUTTON(sc->fullscreen), 
		 	   EWL_RADIOBUTTON(sc->custom));
 ewl_object_alignment_set(EWL_OBJECT(sc->custom), EWL_FLAG_ALIGN_RIGHT);
 ewl_widget_show(sc->custom);

 sc->hbox = ewl_hbox_new();
 ewl_object_fill_policy_set(EWL_OBJECT(sc->hbox), EWL_FLAG_FILL_HFILL);
 ewl_container_child_append(EWL_CONTAINER(sc->window_border), sc->hbox);
 ewl_widget_show(sc->hbox);
 
 sc->text = ewl_text_new();
 ewl_text_text_set(EWL_TEXT(sc->text), "Width");
 ewl_object_fill_policy_set(EWL_OBJECT(sc->text), EWL_FLAG_FILL_SHRINK);
 ewl_object_alignment_set(EWL_OBJECT(sc->text), EWL_FLAG_ALIGN_CENTER);
 ewl_container_child_append(EWL_CONTAINER(sc->hbox), sc->text);
 ewl_widget_show(sc->text);

 sc->wentry = ewl_entry_new();
 ewl_text_text_set(EWL_TEXT(sc->wentry), "640");
 ewl_container_child_append(EWL_CONTAINER(sc->hbox), sc->wentry);
 ewl_object_size_request(EWL_OBJECT(sc->wentry), 35, 15);
 ewl_widget_disable(sc->wentry);
 ewl_widget_show(sc->wentry);

 sc->text = ewl_text_new();
 ewl_text_text_set(EWL_TEXT(sc->text), "Height");
 ewl_object_fill_policy_set(EWL_OBJECT(sc->text), EWL_FLAG_FILL_SHRINK);
 ewl_object_alignment_set(EWL_OBJECT(sc->text), EWL_FLAG_ALIGN_CENTER);
 ewl_container_child_append(EWL_CONTAINER(sc->hbox), sc->text);
 ewl_widget_show(sc->text);

 sc->hentry = ewl_entry_new();
 ewl_text_text_set(EWL_TEXT(sc->hentry), "480");
 ewl_container_child_append(EWL_CONTAINER(sc->hbox), sc->hentry);
 ewl_object_size_request(EWL_OBJECT(sc->hentry), 35, 15);
 ewl_widget_disable(sc->hentry);
 
 sc->loop = ewl_checkbutton_new();
 ewl_button_label_set(EWL_BUTTON(sc->loop), "Loop Slideshow");
 ewl_checkbutton_checked_set(EWL_CHECKBUTTON(sc->loop), FALSE);
 ewl_container_child_append(EWL_CONTAINER(sc->order_border), sc->loop);
 ewl_object_alignment_set(EWL_OBJECT(sc->loop), EWL_FLAG_ALIGN_CENTER);
 ewl_widget_show(sc->loop);

 sc->random = ewl_checkbutton_new();
 ewl_button_label_set(EWL_BUTTON(sc->random), "Random Order");
 ewl_checkbutton_checked_set(EWL_CHECKBUTTON(sc->random), FALSE);
 ewl_container_child_append(EWL_CONTAINER(sc->order_border), sc->random);
 ewl_object_alignment_set(EWL_OBJECT(sc->random), EWL_FLAG_ALIGN_CENTER);
 ewl_widget_show(sc->random);
 
 sc->hbox = ewl_hbox_new();
 ewl_object_fill_policy_set(EWL_OBJECT(sc->hbox), EWL_FLAG_FILL_HFILL);
 ewl_object_alignment_set(EWL_OBJECT(sc->hbox), EWL_FLAG_ALIGN_CENTER);
 ewl_container_child_append(EWL_CONTAINER(sc->transition_border), sc->hbox);
 ewl_widget_show(sc->hbox);
 
 sc->spinner = ewl_spinner_new();
 ewl_spinner_digits_set(EWL_SPINNER(sc->spinner), 0);
 ewl_range_value_set(EWL_RANGE(sc->spinner), 3);
 ewl_range_step_set(EWL_RANGE(sc->spinner), 1);
 ewl_range_minimum_value_set(EWL_RANGE(sc->spinner), 1.0);
 ewl_range_maximum_value_set(EWL_RANGE(sc->spinner), 1000);
 ewl_container_child_append(EWL_CONTAINER(sc->hbox), sc->spinner);
 ewl_object_alignment_set(EWL_OBJECT(sc->spinner), EWL_FLAG_ALIGN_CENTER);
 ewl_widget_show(sc->spinner);
 
 sc->show_name = ewl_checkbutton_new();
 ewl_button_label_set(EWL_BUTTON(sc->show_name), "Show File Name On Image Change");
 ewl_checkbutton_checked_set(EWL_CHECKBUTTON(sc->show_name), FALSE);
 ewl_container_child_append(EWL_CONTAINER(sc->hbox), sc->show_name);
 ewl_object_alignment_set(EWL_OBJECT(sc->show_name), EWL_FLAG_ALIGN_CENTER);
 ewl_widget_show(sc->show_name);
      
 sc->hbox = ewl_hbox_new();
 ewl_object_fill_policy_set(EWL_OBJECT(sc->hbox), EWL_FLAG_FILL_SHRINK);
 ewl_object_alignment_set(EWL_OBJECT(sc->hbox), EWL_FLAG_ALIGN_CENTER);
 ewl_container_child_append(EWL_CONTAINER(sc->vbox), sc->hbox);
 ewl_widget_show(sc->hbox);
 
 sc->save = ewl_button_new();
 ewl_button_stock_type_set(EWL_BUTTON(sc->save), EWL_STOCK_SAVE);
 ewl_container_child_append(EWL_CONTAINER(sc->hbox), sc->save);
 ewl_object_fill_policy_set(EWL_OBJECT(sc->save), EWL_FLAG_FILL_SHRINK);
 ewl_object_alignment_set(EWL_OBJECT(sc->save), EWL_FLAG_ALIGN_CENTER);
 ewl_callback_append(sc->save, EWL_CALLBACK_CLICKED, save_config, sc);
 ewl_widget_show(sc->save);

 sc->cancel = ewl_button_new();
 ewl_button_stock_type_set(EWL_BUTTON(sc->cancel), EWL_STOCK_CANCEL);
 ewl_container_child_append(EWL_CONTAINER(sc->hbox), sc->cancel);
 ewl_object_fill_policy_set(EWL_OBJECT(sc->cancel), EWL_FLAG_FILL_SHRINK);
 ewl_object_alignment_set(EWL_OBJECT(sc->cancel), EWL_FLAG_ALIGN_CENTER);
 ewl_callback_append(sc->cancel, EWL_CALLBACK_CLICKED, config_cancel, sc->win);
 ewl_widget_show(sc->cancel);   
}
