#include "geist.h"
#include "geist_document.h"
#include "geist_object.h"
#include "geist_image.h"
#include "geist_document_gtk.h"
#include "geist_text.h"
#include "geist_rect.h"
#include "geist_layer.h"
#include "geist_document_xml.h"
#include "geist_gtk_menu.h"
#include "geist_interface.h"

GtkWidget *mainwin;
geist_document *current_doc;
GtkWidget *props_window, *obj_hbox;
GtkWidget *gen_props;
GtkWidget *table;
GtkWidget *statusbar;
int props_active = 0;

/*generic props widgets*/
GtkWidget *name;
GtkWidget *sizemode_combo;
GtkWidget *alignment_combo;
GtkWidget *vis_toggle;

void refresh_name_cb(GtkWidget * widget, gpointer * obj);
void refresh_sizemode_cb(GtkWidget * widget, gpointer * obj);
void refresh_alignment_cb(GtkWidget * widget, gpointer * obj);
void geist_update_statusbar(geist_document * doc);

typedef struct _geist_confirmation_dialog_data {
	GtkWidget *dialog;
	gboolean value;
	GMainLoop *loop;
} geist_confirmation_dialog_data;

gboolean geist_confirmation_dialog_new_with_text (char *text);


char *object_types[] = {
   "None",
   "Image",
   "Text",
   "Rect",
   "XXXXX"
};

char *object_sizemodes[] = {
   "None",
   "Zoom",
   "Stretch",
   "XXXXX"
};

char *object_alignments[] = {
   "None",
   "Center Horizontal",
   "Center Vertical",
   "Center Both",
   "Left",
   "Right",
   "Top",
   "Bottom",
   "XXXXX"
};

GtkWidget *
geist_create_main_window(void)
{
   GtkWidget *mvbox, *menubar, *menu, *menuitem;
   GtkWidget *nbook;
   GtkWidget *mainwin;


   D_ENTER(3);

   mainwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_policy(GTK_WINDOW(mainwin), TRUE, TRUE, TRUE);
   gtk_window_set_wmclass(GTK_WINDOW(mainwin), "geist", "geist");
   gtk_signal_connect(GTK_OBJECT(mainwin), "delete_event",
                      GTK_SIGNAL_FUNC(mainwin_delete_cb), NULL);
   gtk_signal_connect(GTK_OBJECT(mainwin), "destroy_event",
                      GTK_SIGNAL_FUNC(mainwin_destroy_cb), NULL);
   gtk_widget_show(mainwin);

   mvbox = gtk_vbox_new(FALSE, 0);
   gtk_widget_show(mvbox);
   gtk_container_add(GTK_CONTAINER(mainwin), mvbox);

   /* menus */
   tooltips = gtk_tooltips_new();

   menubar = gtk_menu_bar_new();
   gtk_widget_show(menubar);
   gtk_box_pack_start(GTK_BOX(mvbox), menubar, FALSE, FALSE, 0);

   menu = geist_gtk_create_submenu(menubar, "File");

   menuitem =
      geist_gtk_create_menuitem(menu, "New...", "", "New Document",
                                (GtkFunction) menu_cb, "new doc");
   menuitem =
      geist_gtk_create_menuitem(menu, "Open...", "", "Open Document",
                                (GtkFunction) menu_cb, "open doc");
   menuitem =
      geist_gtk_create_menuitem(menu, "Save", "", "Save Document",
                                (GtkFunction) menu_cb, "save doc");
   menuitem =
      geist_gtk_create_menuitem(menu, "Save as...", "", "Save Document As...",
                                (GtkFunction) menu_cb, "save doc as");

   menu = geist_gtk_create_submenu(menubar, "Add");

   menuitem =
      geist_gtk_create_menuitem(menu, "image...", "", "Add Image",
                                (GtkFunction) obj_imageadd_cb, NULL);
   menuitem =
      geist_gtk_create_menuitem(menu, "rect...", "", "Add Rectangle",
                                (GtkFunction) obj_addrect_cb, NULL);
   menuitem =
      geist_gtk_create_menuitem(menu, "text...", "", "Add Text",
                                (GtkFunction) obj_addtext_cb, NULL);

   nbook = gtk_notebook_new();
   gtk_notebook_set_tab_pos(GTK_NOTEBOOK(nbook), GTK_POS_BOTTOM);
   gtk_widget_show(nbook);
   gtk_signal_connect(GTK_OBJECT(nbook), "switch_page",
                      GTK_SIGNAL_FUNC(nbook_switch_page_cb), NULL);

   gtk_box_pack_start(GTK_BOX(mvbox), nbook, TRUE, TRUE, 0);

   gtk_object_set_data(GTK_OBJECT(mainwin), "notebook", nbook);

   /*statusbar */
   statusbar = gtk_statusbar_new();
   gtk_box_pack_end(GTK_BOX(mvbox), statusbar, FALSE, FALSE, 0);
   gtk_widget_show(statusbar);

   D_RETURN(3, mainwin);
}

void
geist_clear_statusbar(void)
{
   gint contextid;

   contextid =
      gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), "props");
   gtk_statusbar_push(GTK_STATUSBAR(statusbar), contextid,
                      "[No object selected]");
}

void
geist_update_statusbar(geist_document * doc)
{
   geist_object *obj;
   geist_list *list;
   char buff[35];

   gint contextid;

   contextid =
      gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), "props");

   list = geist_document_get_selected_list(doc);

   if (list)
   {
      if (geist_list_has_more_than_one_item(list))
      {
         gtk_statusbar_push(GTK_STATUSBAR(statusbar), contextid,
                            "[Multiple selection]");
      }
      else
      {
         obj = list->data;
         g_snprintf(buff, 35, "X:%d | Y:%d | W:%d | H:%d", obj->x, obj->y,
                    obj->w, obj->h);

         gtk_statusbar_push(GTK_STATUSBAR(statusbar), contextid, buff);
      }
   }
   else
      gtk_statusbar_push(GTK_STATUSBAR(statusbar), contextid,
                         "[No object selected]");

}



void
nbook_switch_page_cb(GtkNotebook * notebook, GtkNotebookPage * page,
                     guint page_num)
{
   D_ENTER(3);

   current_doc = GEIST_DOCUMENT((geist_list_nth(doc_list, page_num))->data);
   geist_clear_props_window();
   geist_clear_statusbar();
   geist_document_reset_object_list(current_doc);

   D_RETURN_(3);
}

GtkWidget *
geist_create_object_list(void)
{
   GtkWidget *obj_table, *obj_btn, *obj_btn_hbox, *obj_scroll;

   D_ENTER(3);

   obj_win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
   obj_table = gtk_table_new(3, 4, FALSE);
   gtk_container_set_border_width(GTK_CONTAINER(obj_win), 3);
   gtk_container_add(GTK_CONTAINER(obj_win), obj_table);
   obj_scroll = gtk_scrolled_window_new(NULL, NULL);
   gtk_table_attach(GTK_TABLE(obj_table), obj_scroll, 0, 4, 1, 2,
                    GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 2, 2);
   obj_list = gtk_clist_new(2);
   gtk_clist_set_selection_mode(GTK_CLIST(obj_list), GTK_SELECTION_EXTENDED);
   gtk_clist_column_titles_hide(GTK_CLIST(obj_list));
   gtk_clist_column_titles_passive(GTK_CLIST(obj_list));
   gtk_clist_set_column_visibility(GTK_CLIST(obj_list), 0, TRUE);
   gtk_clist_set_column_auto_resize(GTK_CLIST(obj_list), 0, TRUE);
   gtk_clist_set_column_visibility(GTK_CLIST(obj_list), 1, TRUE);
   gtk_clist_set_column_auto_resize(GTK_CLIST(obj_list), 1, TRUE);
   obj_sel_handler =
      gtk_signal_connect(GTK_OBJECT(obj_list), "select_row",
                         GTK_SIGNAL_FUNC(obj_sel_cb), NULL);
   obj_unsel_handler =
      gtk_signal_connect(GTK_OBJECT(obj_list), "unselect_row",
                         GTK_SIGNAL_FUNC(obj_unsel_cb), NULL);
   gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(obj_scroll),
                                         obj_list);
   gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(obj_scroll),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
   obj_btn_hbox = gtk_hbox_new(FALSE, 0);
   obj_btn = gtk_button_new_with_label("Add Image...");
   gtk_signal_connect(GTK_OBJECT(obj_btn), "clicked",
                      GTK_SIGNAL_FUNC(obj_imageadd_cb), NULL);
   gtk_box_pack_start(GTK_BOX(obj_btn_hbox), obj_btn, TRUE, TRUE, 2);
   gtk_widget_show(obj_btn);
   obj_btn = gtk_button_new_with_label("Add Text...");
   gtk_signal_connect(GTK_OBJECT(obj_btn), "clicked",
                      GTK_SIGNAL_FUNC(obj_addtext_cb), NULL);
   gtk_box_pack_start(GTK_BOX(obj_btn_hbox), obj_btn, TRUE, TRUE, 2);
   gtk_widget_show(obj_btn);
   obj_btn = gtk_button_new_with_label("Add Rectangle...");
   gtk_signal_connect(GTK_OBJECT(obj_btn), "clicked",
                      GTK_SIGNAL_FUNC(obj_addrect_cb), NULL);
   gtk_box_pack_start(GTK_BOX(obj_btn_hbox), obj_btn, TRUE, TRUE, 2);
   gtk_widget_show(obj_btn);

   obj_btn = gtk_button_new_with_label("Copy");
   gtk_signal_connect(GTK_OBJECT(obj_btn), "clicked",
                      GTK_SIGNAL_FUNC(obj_cpy_cb), NULL);
   gtk_box_pack_start(GTK_BOX(obj_btn_hbox), obj_btn, TRUE, TRUE, 2);
   gtk_widget_show(obj_btn);
   obj_btn = gtk_button_new_with_label("Delete");
   gtk_signal_connect(GTK_OBJECT(obj_btn), "clicked",
                      GTK_SIGNAL_FUNC(obj_del_cb), NULL);
   gtk_box_pack_start(GTK_BOX(obj_btn_hbox), obj_btn, TRUE, TRUE, 2);
   gtk_widget_show(obj_btn);
   gtk_table_attach(GTK_TABLE(obj_table), obj_btn_hbox, 0, 3, 0, 1,
                    GTK_FILL | GTK_EXPAND, 0, 2, 2);
   gtk_widget_show(obj_btn_hbox);
   gtk_window_set_default_size(GTK_WINDOW(obj_win), 125, 230);
   gtk_widget_show(obj_list);
   gtk_widget_show(obj_scroll);
   gtk_widget_show(obj_table);
   gtk_widget_show(obj_win);

   D_RETURN(3, obj_list);
}



GtkWidget *
geist_gtk_new_document_page(geist_document * doc)
{
   GtkWidget *hwid, *vwid, *darea, *viewport;
   GtkWidget *label, *scrollwin;
   GtkWidget *parent;

   D_ENTER(3);

   parent = gtk_object_get_data(GTK_OBJECT(mainwin), "notebook");

   doc_list = geist_list_add_end(doc_list, doc);

   hwid = gtk_hbox_new(TRUE, 0);
   gtk_widget_show(hwid);
   label = gtk_label_new(doc->name);
   gtk_widget_show(label);
   gtk_notebook_append_page(GTK_NOTEBOOK(parent), hwid, label);
   gtk_object_set_data(GTK_OBJECT(parent), "doc", doc);

   vwid = gtk_vbox_new(TRUE, 0);
   gtk_widget_show(vwid);
   gtk_box_pack_start(GTK_BOX(hwid), vwid, TRUE, FALSE, 0);

   scrollwin = gtk_scrolled_window_new(NULL, NULL);
   gtk_widget_set_usize(scrollwin, doc->w, doc->h);
   gtk_widget_show(scrollwin);
   gtk_box_pack_start(GTK_BOX(vwid), scrollwin, TRUE, FALSE, 0);

   gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

   viewport = gtk_viewport_new(NULL, NULL);
   gtk_widget_show(viewport);
   gtk_container_add(GTK_CONTAINER(scrollwin), viewport);

   darea = gtk_drawing_area_new();
   gtk_widget_set_events(darea,
                         GDK_EXPOSURE_MASK | GDK_LEAVE_NOTIFY_MASK |
                         GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
                         GDK_POINTER_MOTION_MASK |
                         GDK_POINTER_MOTION_HINT_MASK);

   gtk_object_set_data(GTK_OBJECT(darea), "doc", doc);
   doc->darea = darea;

   gtk_signal_connect(GTK_OBJECT(darea), "button_press_event",
                      GTK_SIGNAL_FUNC(evbox_buttonpress_cb), doc);
   gtk_signal_connect(GTK_OBJECT(darea), "button_release_event",
                      GTK_SIGNAL_FUNC(evbox_buttonrelease_cb), doc);
   gtk_signal_connect(GTK_OBJECT(darea), "motion_notify_event",
                      GTK_SIGNAL_FUNC(evbox_mousemove_cb), doc);

   gtk_container_add(GTK_CONTAINER(viewport), darea);
   gtk_signal_connect_after(GTK_OBJECT(darea), "configure_event",
                            GTK_SIGNAL_FUNC(configure_cb), doc);
   gtk_widget_show(darea);

   D_RETURN(3, darea);
}

gboolean
mainwin_delete_cb(GtkWidget * widget, GdkEvent * event, gpointer user_data)
{
   D_ENTER(3);
   gtk_exit(0);
   D_RETURN(3, FALSE);
}

gboolean
mainwin_destroy_cb(GtkWidget * widget, GdkEvent * event, gpointer user_data)
{
   D_ENTER(3);
   gtk_exit(0);
   D_RETURN(3, FALSE);
}

gboolean configure_cb(GtkWidget * widget, GdkEventConfigure * event,
                      gpointer user_data)
{
   geist_document *doc;

   D_ENTER(3);

   doc = GEIST_DOCUMENT(user_data);
   if (doc)
      geist_document_render_to_window(doc);

   D_RETURN(3, TRUE);
}

gint evbox_buttonpress_cb(GtkWidget * widget, GdkEventButton * event,
                          gpointer user_data)
{
   geist_object *obj = NULL;
   geist_document *doc;

   D_ENTER(5);

   doc = GEIST_DOCUMENT(user_data);
   if (!doc)
      D_RETURN(5, 1);

   if (event->button == 1)
   {
      geist_list *l, *list;
      int row;

      /* First check for resizes */
      list = geist_document_get_selected_list(doc);
      if (list)
      {
         int resize = 0;

         for (l = list; l; l = l->next)
         {
            obj = GEIST_OBJECT(l->data);
            if (
                (resize =
                 geist_object_check_resize_click(obj, event->x, event->y)))
               break;
         }

         if (resize)
         {
            int res_x, res_y;

            /* Click requests a resize. */
            for (l = list; l; l = l->next)
            {
               obj = GEIST_OBJECT(l->data);
               D(5, ("setting object state RESIZE\n"));
               geist_object_set_state(obj, RESIZE);
               obj->resize = resize;
               geist_object_get_resize_box_coords(obj, resize, &res_x,
                                                  &res_y);
               obj->clicked_x = res_x - event->x;
               obj->clicked_y = res_y - event->y;
               geist_object_dirty(obj);
            }
            gtk_object_set_data_full(GTK_OBJECT(mainwin), "resizelist", list,
                                     NULL);
            geist_document_render_updates(doc);
            D_RETURN(5, 1);
         }
         geist_list_free(list);
      }

      obj = geist_document_find_clicked_object(doc, event->x, event->y);
      if (!obj)
      {
         geist_document_unselect_all(doc);
         geist_clear_statusbar();
         geist_clear_props_window();
         geist_document_render_updates(doc);
         D_RETURN(5, 1);
      }
      gtk_signal_handler_block(GTK_OBJECT(obj_list), obj_sel_handler);
      gtk_signal_handler_block(GTK_OBJECT(obj_list), obj_unsel_handler);

      if (event->state & GDK_SHIFT_MASK)
      {
         /* shift click - multiple selections and selection toggling */
         row =
            gtk_clist_find_row_from_data(GTK_CLIST(obj_list), (gpointer) obj);
         if (geist_object_get_state(obj, SELECTED))
         {
            geist_object_unselect(obj);
            if (row != -1)
               gtk_clist_unselect_row(GTK_CLIST(obj_list), row, 0);
         }
         else
         {
            geist_object_select(obj);
            if (row != -1)
               gtk_clist_select_row(GTK_CLIST(obj_list), row, 0);
         }

         /* need to dirty all the other selected objects, in case they go from
          * being the only thing selected to part of a multiple selection -
          * hence have to change their selection type */
         geist_document_dirty_selection(doc);
         geist_update_props_window();
      }
      else
      {
         if (!geist_object_get_state(obj, SELECTED))
         {
            /* Single click selection */
            geist_document_unselect_all(doc);
            gtk_clist_unselect_all(GTK_CLIST(obj_list));
            geist_object_select(obj);
            row =
               gtk_clist_find_row_from_data(GTK_CLIST(obj_list),
                                            (gpointer) obj);
            if (row != -1)
               gtk_clist_select_row(GTK_CLIST(obj_list), row, 0);
            geist_update_props_window();
         }

         list = geist_document_get_selected_list(doc);
         if (list)
         {
            for (l = list; l; l = l->next)
            {
               /* set clicked_x,y */
               obj = GEIST_OBJECT(l->data);
               obj->clicked_x = event->x - obj->x;
               obj->clicked_y = event->y - obj->y;
               D(2, ("setting object state DRAG\n"));
               geist_object_set_state(obj, DRAG);
               geist_object_raise(obj);
            }
            geist_update_statusbar(doc);
         }
         gtk_object_set_data_full(GTK_OBJECT(mainwin), "draglist", list,
                                  NULL);
      }
      geist_document_render_updates(doc);

      gtk_signal_handler_unblock(GTK_OBJECT(obj_list), obj_sel_handler);
      gtk_signal_handler_unblock(GTK_OBJECT(obj_list), obj_unsel_handler);
   }
   D_RETURN(5, 1);
}

gint
evbox_buttonrelease_cb(GtkWidget * widget, GdkEventButton * event,
                       gpointer user_data)
{
   geist_list *list, *l;
   geist_object *obj;
   geist_document *doc;

   D_ENTER(5);

   doc = GEIST_DOCUMENT(user_data);
   if (!doc)
      D_RETURN(5, 1);

   list = gtk_object_get_data(GTK_OBJECT(mainwin), "draglist");
   if (list)
   {
      for (l = list; l; l = l->next)
      {
         obj = GEIST_OBJECT(l->data);

         D(2, ("unsetting object state DRAG\n"));
         geist_object_unset_state(obj, DRAG);
         geist_object_dirty(obj);
      }
      geist_list_free(list);
      gtk_object_set_data_full(GTK_OBJECT(mainwin), "draglist", NULL, NULL);
   }
   else
   {
      list = gtk_object_get_data(GTK_OBJECT(mainwin), "resizelist");
      if (list)
      {
         for (l = list; l; l = l->next)
         {
            obj = GEIST_OBJECT(l->data);

            D(2, ("unsetting object state RESIZE\n"));
            geist_object_unset_state(obj, RESIZE);
            obj->resize = RESIZE_NONE;
            geist_object_dirty(obj);
         }
         geist_list_free(list);
         gtk_object_set_data_full(GTK_OBJECT(mainwin), "resizelist", NULL,
                                  NULL);

      }
   }
   geist_document_render_updates(doc);

   D_RETURN(5, 1);
}

gint
evbox_mousemove_cb(GtkWidget * widget, GdkEventMotion * event,
                   gpointer user_data)
{
   geist_list *l, *list;
   geist_object *obj;
   geist_document *doc;
   int x, y;
   GdkModifierType state;

   D_ENTER(5);

   doc = GEIST_DOCUMENT(user_data);
   if (!doc)
      D_RETURN(5, 1);

   /* use hinted motionnotify to prevent queue backlog */
   if (event->is_hint)
      gdk_window_get_pointer(event->window, &x, &y, &state);
   else
   {
      x = event->x;
      y = event->y;
      state = event->state;
   }

   list = gtk_object_get_data(GTK_OBJECT(mainwin), "draglist");
   if (list)
   {
      for (l = list; l; l = l->next)
      {
         obj = GEIST_OBJECT(l->data);
         D(5, ("moving object to %f, %f\n", x, y));
         geist_object_move(obj, x, y);
      }
      geist_update_statusbar(doc);
      geist_document_render_updates(doc);
   }
   else
   {
      list = gtk_object_get_data(GTK_OBJECT(mainwin), "resizelist");
      if (list)
      {
         for (l = list; l; l = l->next)
         {
            obj = GEIST_OBJECT(l->data);
            D(5, ("resizing object\n"));
            geist_object_resize(obj, x + obj->clicked_x, y + obj->clicked_y);
         }
         geist_update_statusbar(doc);
         geist_document_render_updates(doc);
      }

   }

   D_RETURN(5, 1);
}

gboolean
obj_load_cb(GtkWidget * widget, gpointer data)
{
   geist_object *obj = NULL;
   char *path;
   int row;

   D_ENTER(3);

   path =
      gtk_file_selection_get_filename(GTK_FILE_SELECTION((GtkWidget *) data));
   if (path)
   {
      obj = geist_image_new_from_file(0, 0, path);
      if (obj)
      {
         geist_document_add_object(current_doc, obj);
         geist_object_show(obj);
         geist_object_raise(obj);

         geist_document_unselect_all(current_doc);

         row =
            gtk_clist_find_row_from_data(GTK_CLIST(obj_list), (gpointer) obj);
         if (row != -1)
            gtk_clist_select_row(GTK_CLIST(obj_list), row, 0);

         geist_document_render_updates(current_doc);
      }
   }
   gtk_widget_destroy((GtkWidget *) data);
   D_RETURN(3, TRUE);
}

gboolean
obj_load_cancel_cb(GtkWidget * widget, gpointer data)
{
   gtk_widget_destroy((GtkWidget *) data);
   return TRUE;
}

gboolean
obj_imageadd_cb(GtkWidget * widget, gpointer * data)
{
   GtkWidget *file_sel = gtk_file_selection_new("Add an Image");

   gtk_file_selection_show_fileop_buttons(GTK_FILE_SELECTION(file_sel));
   gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_sel)->ok_button),
                      "clicked", GTK_SIGNAL_FUNC(obj_load_cb),
                      (gpointer) file_sel);
   gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_sel)->cancel_button),
                      "clicked", GTK_SIGNAL_FUNC(obj_load_cancel_cb),
                      (gpointer) file_sel);
   gtk_widget_show(file_sel);
   return TRUE;
}


gboolean
obj_cpy_cb(GtkWidget * widget, gpointer * data)
{
   geist_object *new;
   geist_list *l, *list;
   geist_object *obj;
   int row;

   D_ENTER(3);

   list = geist_document_get_selected_list(current_doc);
   for (l = list; l; l = l->next)
   {
      obj = GEIST_OBJECT(l->data);
      new = geist_object_duplicate(obj);
      if (new)
      {
         new->x += 5;
         new->y += 5;
         geist_document_add_object(current_doc, new);

         row =
            gtk_clist_find_row_from_data(GTK_CLIST(obj_list), (gpointer) obj);
         if (row != -1)
            gtk_clist_unselect_row(GTK_CLIST(obj_list), row, 0);

         row =
            gtk_clist_find_row_from_data(GTK_CLIST(obj_list), (gpointer) new);
         if (row != -1)
            gtk_clist_select_row(GTK_CLIST(obj_list), row, 0);
      }
   }
   geist_list_free(list);
   geist_document_render_updates(current_doc);

   D_RETURN(3, TRUE);
}

gboolean
obj_del_cb(GtkWidget * widget, gpointer * data)
{
   geist_object *obj;
   geist_list *l, *list;

   D_ENTER(3);

   list = geist_document_get_selected_list(current_doc);
   for (l = list; l; l = l->next)
   {
      obj = GEIST_OBJECT(l->data);
      gtk_signal_handler_block(GTK_OBJECT(obj_list), obj_sel_handler);
      gtk_signal_handler_block(GTK_OBJECT(obj_list), obj_unsel_handler);

      if (props_active)
         geist_clear_props_window();

      geist_clear_statusbar();

      gtk_clist_remove(GTK_CLIST(obj_list),
                       gtk_clist_find_row_from_data(GTK_CLIST(obj_list),
                                                    obj));
      gtk_signal_handler_unblock(GTK_OBJECT(obj_list), obj_sel_handler);
      gtk_signal_handler_unblock(GTK_OBJECT(obj_list), obj_unsel_handler);
      geist_document_remove_object(current_doc, obj);
      geist_object_free(obj);
   }
   geist_list_free(list);
   geist_document_render_updates(current_doc);

   D_RETURN(3, TRUE);
}


gboolean
obj_sel_cb(GtkWidget * widget, int row, int column, GdkEventButton * event,
           gpointer * data)
{
   GList *selection;
   geist_object *obj;

   D_ENTER(3);

   obj = (geist_object *) gtk_clist_get_row_data(GTK_CLIST(widget), row);
   if (obj)
   {
      geist_object_select(obj);
      geist_update_props_window();
      geist_update_statusbar(GEIST_OBJECT_DOC(obj));

      selection = GTK_CLIST(widget)->selection;
      if (g_list_length(selection) > 1)
         geist_document_dirty_selection(GEIST_OBJECT_DOC(obj));

      geist_document_render_updates(GEIST_OBJECT_DOC(obj));
   }

   D_RETURN(3, TRUE);
}

gboolean
obj_unsel_cb(GtkWidget * widget, int row, int column, GdkEventButton * event,
             gpointer * data)
{
   GList *selection;
   geist_object *obj;

   D_ENTER(3);

   obj = (geist_object *) gtk_clist_get_row_data(GTK_CLIST(widget), row);
   if (obj)
   {
      geist_object_unselect(obj);
      geist_update_props_window();
      geist_update_statusbar(GEIST_OBJECT_DOC(obj));

      selection = GTK_CLIST(widget)->selection;
      if (g_list_length(selection) > 1)
         geist_document_dirty_selection(GEIST_OBJECT_DOC(obj));

      geist_document_render_updates(GEIST_OBJECT_DOC(obj));
   }

   D_RETURN(3, TRUE);
}

gboolean
obj_addtext_cb(GtkWidget * widget, gpointer * data)
{

   int row;
   geist_object *obj;

   D_ENTER(3);

   obj =
      GEIST_OBJECT(geist_text_new_with_text
                   (50, 50, "cinema.ttf", 12, "New Text", 50, 50, 255, 0));
   geist_document_add_object(current_doc, obj);

   row = gtk_clist_find_row_from_data(GTK_CLIST(obj_list), (gpointer) obj);

   geist_document_unselect_all(current_doc);
   if (row != -1)
      gtk_clist_select_row(GTK_CLIST(obj_list), row, 0);

   geist_document_render_updates(current_doc);

   D_RETURN(3, TRUE);
}



gboolean
obj_addrect_cb(GtkWidget * widget, gpointer * data)
{

   int row;
   geist_object *obj;

   D_ENTER(3);

   obj = GEIST_OBJECT(geist_rect_new_of_size(50, 50, 50, 50, 255, 0, 0, 0));
   geist_document_add_object(current_doc, obj);

   row = gtk_clist_find_row_from_data(GTK_CLIST(obj_list), (gpointer) obj);
   if (row != -1)
      gtk_clist_select_row(GTK_CLIST(obj_list), row, 0);

   geist_document_render_updates(current_doc);

   D_RETURN(3, TRUE);
}

gboolean
menu_cb(GtkWidget * widget, gpointer * data)
{
   char *item;
	
   D_ENTER(3);

   item = (char *) data;

   if (!strcmp(item, "save doc"))
   {
      if (current_doc->filename)
         geist_document_save(current_doc, current_doc->filename);
      else
         geist_document_save_as(current_doc);
   }
   else if (!strcmp(item, "save doc as"))
      geist_document_save_as(current_doc);
	else if (!strcmp(item, "new doc"))
   {		
      geist_document *doc = geist_document_new(500, 500);

      geist_gtk_new_document_page(doc);
      geist_document_render_full(doc, 1);
   }
   else if (!strcmp(item, "open doc"))
      geist_document_load();
   else
      printf("IMPLEMENT ME!\n");

   D_RETURN(3, TRUE);
}

static gboolean
props_delete_event_cb(GtkWidget * widget, GdkEvent * event, gpointer * data)
{
   D_ENTER(3);
   gtk_widget_destroy(props_window);
   props_active = 0;

   D_RETURN(3, TRUE);
}




static void
obj_vis_cb(GtkWidget * widget, gpointer * data)
{
   geist_object *obj = NULL;
   geist_list *l = NULL;
   geist_list *list = NULL;

   list = geist_document_get_selected_list(current_doc);

   D_ENTER(3);
   if (geist_list_length(list) > 1)
   {
      for (l = list; l; l = l->next)
      {
         obj = l->data;

         if (geist_object_get_state(obj, VISIBLE))
            geist_object_hide(obj);
         else
            geist_object_show(obj);
      }
   }
   else
   {
      obj = list->data;
      if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
         geist_object_show(obj);
      else
         geist_object_hide(obj);
   }
   geist_document_render_updates(GEIST_OBJECT_DOC(obj));
   efree(l);
   efree(list);
   D_RETURN_(3);
}

void
refresh_name_cb(GtkWidget * widget, gpointer * data)
{
   geist_object *obj = NULL;
   geist_list *list = NULL;

   list = geist_document_get_selected_list(current_doc);

   D_ENTER(3);

   if (geist_list_length(list) > 1)
      printf("Implement me!\n");
   else
   {
      obj = list->data;
      if (obj->name)
         efree(obj->name);

      obj->name = estrdup(gtk_entry_get_text(GTK_ENTRY(widget)));
   }
   efree(list);
   D_RETURN_(3);
}

void
refresh_sizemode_cb(GtkWidget * widget, gpointer * data)
{
   geist_object *obj = NULL;
   geist_list *l = NULL;
   geist_list *list = NULL;

   list = geist_document_get_selected_list(current_doc);

   D_ENTER(3);

   for (l = list; l; l = l->next)
   {
      obj = l->data;
      geist_object_dirty(obj);
      obj->sizemode =
         geist_object_get_sizemode_from_string(gtk_entry_get_text
                                               (GTK_ENTRY(widget)));
      geist_object_update_sizemode(obj);
      geist_object_dirty(obj);
   }
   geist_document_render_updates(GEIST_OBJECT_DOC(obj));
   efree(l);
   efree(list);
   D_RETURN_(3);
}


void
refresh_alignment_cb(GtkWidget * widget, gpointer * data)
{
   geist_object *obj = NULL;
   geist_list *l = NULL;
   geist_list *list = NULL;

   list = geist_document_get_selected_list(current_doc);

   D_ENTER(3);

   for (l = list; l; l = l->next)
   {
      obj = l->data;
      geist_object_dirty(obj);
      obj->alignment =
         geist_object_get_alignment_from_string(gtk_entry_get_text
                                                (GTK_ENTRY(widget)));
      geist_object_update_alignment(obj);
      geist_object_dirty(obj);
   }
   geist_document_render_updates(GEIST_OBJECT_DOC(obj));
   efree(l);
   efree(list);
   D_RETURN_(3);
}


void
buttons_cb(GtkWidget * widget, gpointer * data)
{
   geist_object *obj = NULL;
   geist_list *l = NULL;
   geist_list *list = NULL;

   D_ENTER(3);

   list = geist_document_get_selected_list(current_doc);

   for (l = list; l; l = l->next)
   {
      obj = l->data;
      geist_object_dirty(obj);
      switch (GPOINTER_TO_INT(data))
      {
        case 1:
           obj->y--;
           break;
        case 2:
           obj->y++;
           break;
        case 3:
           obj->x--;
           break;
        case 4:
           obj->x++;
           break;
        case 5:
           obj->h++;
           break;
        case 6:
           obj->h--;
           break;
        case 7:
           obj->w++;
           break;
        case 8:
           obj->w--;
           break;
        default:
           break;
      }
      geist_object_dirty(obj);
   }
   geist_update_statusbar(current_doc);
   geist_document_render_updates(GEIST_OBJECT_DOC(obj));
   efree(l);
   D_RETURN_(3);
}

void
geist_display_props_window(void)
{
   GtkWidget *gen_table, *name_l;
   GtkWidget *sizemode_l;
   GtkWidget *alignment_l;
   GtkWidget *up, *down, *left, *right, *width_plus, *width_minus,
      *height_plus, *height_minus;

   GList *align_list = g_list_alloc();
   GList *sizemode_list = g_list_alloc();
   int i;

   D_ENTER(3);
   props_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
   table = gtk_table_new(2, 4, FALSE);

   gtk_container_set_border_width(GTK_CONTAINER(props_window), 5);
   gtk_container_add(GTK_CONTAINER(props_window), table);

   gtk_signal_connect(GTK_OBJECT(props_window), "delete_event",
                      GTK_SIGNAL_FUNC(props_delete_event_cb), NULL);

   gen_props = gtk_hbox_new(FALSE, 0);
   gtk_table_attach(GTK_TABLE(table), gen_props, 0, 4, 0, 1,
                    GTK_FILL | GTK_EXPAND, 0, 2, 2);

   gen_table = gtk_table_new(8, 6, FALSE);

   gtk_container_set_border_width(GTK_CONTAINER(gen_props), 5);
   gtk_container_add(GTK_CONTAINER(gen_props), gen_table);

   vis_toggle = gtk_check_button_new_with_label("Visible");
   gtk_table_attach(GTK_TABLE(gen_table), vis_toggle, 0, 4, 0, 1,
                    GTK_FILL | GTK_EXPAND, 0, 2, 2);

   gtk_widget_show(vis_toggle);


   name_l = gtk_label_new("Name:");
   gtk_table_attach(GTK_TABLE(gen_table), name_l, 0, 1, 1, 2,
                    GTK_FILL | GTK_EXPAND, 0, 2, 2);
   gtk_widget_show(name_l);

   name = gtk_entry_new();
   gtk_table_attach(GTK_TABLE(gen_table), name, 1, 6, 1, 2,
                    GTK_FILL | GTK_EXPAND, 0, 2, 2);
   gtk_widget_show(name);

   sizemode_l = gtk_label_new("sizemode");
   gtk_table_attach(GTK_TABLE(gen_table), sizemode_l, 0, 1, 2, 3,
                    GTK_FILL | GTK_EXPAND, 0, 2, 2);
   gtk_widget_show(sizemode_l);

   sizemode_combo = gtk_combo_new();
   gtk_table_attach(GTK_TABLE(gen_table), sizemode_combo, 1, 6, 2, 3,
                    GTK_FILL | GTK_EXPAND, 0, 2, 2);
   gtk_container_set_border_width(GTK_CONTAINER(sizemode_combo), 5);
   gtk_widget_show(sizemode_combo);

   alignment_l = gtk_label_new("alignment");
   gtk_table_attach(GTK_TABLE(gen_table), alignment_l, 0, 1, 3, 4,
                    GTK_FILL | GTK_EXPAND, 0, 2, 2);
   gtk_widget_show(alignment_l);

   alignment_combo = gtk_combo_new();
   gtk_table_attach(GTK_TABLE(gen_table), alignment_combo, 1, 6, 3, 4,
                    GTK_FILL | GTK_EXPAND, 0, 2, 2);
   gtk_container_set_border_width(GTK_CONTAINER(alignment_combo), 5);
   gtk_widget_show(alignment_combo);

   up = gtk_button_new_with_label("Up");
   gtk_table_attach(GTK_TABLE(gen_table), up, 1, 2, 4, 5,
                    GTK_FILL | GTK_EXPAND, 0, 2, 2);
   gtk_widget_show(up);

   down = gtk_button_new_with_label("Down");
   gtk_table_attach(GTK_TABLE(gen_table), down, 1, 2, 6, 7,
                    GTK_FILL | GTK_EXPAND, 0, 2, 2);
   gtk_widget_show(down);

   left = gtk_button_new_with_label("Left");
   gtk_table_attach(GTK_TABLE(gen_table), left, 0, 1, 5, 6,
                    GTK_FILL | GTK_EXPAND, 0, 2, 2);
   gtk_widget_show(left);

   right = gtk_button_new_with_label("Right");
   gtk_table_attach(GTK_TABLE(gen_table), right, 2, 3, 5, 6,
                    GTK_FILL | GTK_EXPAND, 0, 2, 2);
   gtk_widget_show(right);


   width_plus = gtk_button_new_with_label("Width +");
   gtk_table_attach(GTK_TABLE(gen_table), width_plus, 5, 6, 5, 6,
                    GTK_FILL | GTK_EXPAND, 0, 2, 2);
   gtk_widget_show(width_plus);

   width_minus = gtk_button_new_with_label("Width -");
   gtk_table_attach(GTK_TABLE(gen_table), width_minus, 3, 4, 5, 6,
                    GTK_FILL | GTK_EXPAND, 0, 2, 2);
   gtk_widget_show(width_minus);

   height_plus = gtk_button_new_with_label("Height +");
   gtk_table_attach(GTK_TABLE(gen_table), height_plus, 4, 5, 4, 5,
                    GTK_FILL | GTK_EXPAND, 0, 2, 2);
   gtk_widget_show(height_plus);

   height_minus = gtk_button_new_with_label("Height -");
   gtk_table_attach(GTK_TABLE(gen_table), height_minus, 4, 5, 6, 7,
                    GTK_FILL | GTK_EXPAND, 0, 2, 2);
   gtk_widget_show(height_minus);

   for (i = 0; i < ALIGN_MAX; i++)
   {
      align_list = g_list_append(align_list, object_alignments[i]);
   }

   gtk_combo_set_popdown_strings(GTK_COMBO(alignment_combo), align_list);


   for (i = 0; i < SIZEMODE_MAX; i++)
   {
      align_list = g_list_append(sizemode_list, object_sizemodes[i]);
   }

   gtk_combo_set_popdown_strings(GTK_COMBO(sizemode_combo), sizemode_list);

   gtk_signal_connect(GTK_OBJECT(vis_toggle), "clicked",
                      GTK_SIGNAL_FUNC(obj_vis_cb), NULL);

   gtk_signal_connect(GTK_OBJECT(up), "clicked", GTK_SIGNAL_FUNC(buttons_cb),
                      (gpointer) 1);
   gtk_signal_connect(GTK_OBJECT(down), "clicked",
                      GTK_SIGNAL_FUNC(buttons_cb), (gpointer) 2);
   gtk_signal_connect(GTK_OBJECT(right), "clicked",
                      GTK_SIGNAL_FUNC(buttons_cb), (gpointer) 4);
   gtk_signal_connect(GTK_OBJECT(left), "clicked",
                      GTK_SIGNAL_FUNC(buttons_cb), (gpointer) 3);

   gtk_signal_connect(GTK_OBJECT(height_plus), "clicked",
                      GTK_SIGNAL_FUNC(buttons_cb), (gpointer) 5);
   gtk_signal_connect(GTK_OBJECT(height_minus), "clicked",
                      GTK_SIGNAL_FUNC(buttons_cb), (gpointer) 6);
   gtk_signal_connect(GTK_OBJECT(width_plus), "clicked",
                      GTK_SIGNAL_FUNC(buttons_cb), (gpointer) 7);
   gtk_signal_connect(GTK_OBJECT(width_minus), "clicked",
                      GTK_SIGNAL_FUNC(buttons_cb), (gpointer) 8);

   gtk_signal_connect(GTK_OBJECT(name), "changed",
                      GTK_SIGNAL_FUNC(refresh_name_cb), NULL);

   gtk_signal_connect(GTK_OBJECT(GTK_COMBO(alignment_combo)->entry),
                      "changed", GTK_SIGNAL_FUNC(refresh_alignment_cb), NULL);
   gtk_signal_connect(GTK_OBJECT(GTK_COMBO(sizemode_combo)->entry), "changed",
                      GTK_SIGNAL_FUNC(refresh_sizemode_cb), NULL);


   gtk_widget_show(gen_table);
   gtk_widget_show(gen_props);
   gtk_widget_show(table);
   gtk_widget_show(props_window);
   D_RETURN_(3);
}

void
geist_hide_props_window(void)
{
   D_ENTER(3);
   gtk_widget_destroy(props_window);
   props_active = 0;
   D_RETURN_(3);
}

void
geist_clear_props_window(void)
{
   if (obj_hbox)
   {
      gtk_widget_destroy(obj_hbox);
      obj_hbox = NULL;
   }
   if (gen_props)
      gtk_widget_hide(gen_props);
}

void
geist_update_props_window(void)
{
   geist_object *obj;
   GtkWidget *new_hbox;
   geist_list *list;
   geist_object *obj_first;
   geist_list *l;
   char *align_string = NULL;
   char *sizemode_string = NULL;

   D_ENTER(3);

   /*display props window if inactive */
   if (!props_active)
   {
      geist_display_props_window();
      props_active = 1;
   }

   if (obj_hbox)
   {
      gtk_widget_destroy(obj_hbox);
      obj_hbox = NULL;
   }

   /*show generic part */
   gtk_widget_show(gen_props);

   /*block signal handlers */
   gtk_signal_handler_block_by_func(GTK_OBJECT(vis_toggle),
                                    GTK_SIGNAL_FUNC(obj_vis_cb), NULL);
   gtk_signal_handler_block_by_func(GTK_OBJECT(name), refresh_name_cb, NULL);
   gtk_signal_handler_block_by_func(GTK_OBJECT
                                    (GTK_COMBO(alignment_combo)->entry),
                                    refresh_alignment_cb, NULL);
   gtk_signal_handler_block_by_func(GTK_OBJECT
                                    (GTK_COMBO(sizemode_combo)->entry),
                                    refresh_sizemode_cb, NULL);


   list = geist_document_get_selected_list(current_doc);

   if (list)
   {
      /*update the values in the generic part */

      if (geist_list_has_more_than_one_item(list))
      {
         /*grey out the name entry box */
         gtk_entry_set_text(GTK_ENTRY(name), "");
         gtk_widget_set_sensitive(GTK_WIDGET(name), FALSE);

         obj_first = list->data;

         /*check wether all objects have the same alignment or sizemode, and if
            so, set the combo boxes, if not leave them empty */
         for (l = list; l; l = l->next)
         {
            obj = l->data;
            if (obj->alignment == obj_first->alignment)
               align_string = geist_object_get_alignment_string(obj);
            else
               align_string = "";

            if (obj->sizemode == obj_first->sizemode)
               sizemode_string = geist_object_get_sizemode_string(obj);
            else
               sizemode_string = "";
         }

         efree(l);

         gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(alignment_combo)->entry),
                            align_string);
         gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(sizemode_combo)->entry),
                            sizemode_string);


         gtk_window_set_title(GTK_WINDOW(obj_win), "[multiple selection]");
         gtk_window_set_title(GTK_WINDOW(props_window),
                              "[multiple selection]");

      }
      else
      {

         gtk_widget_set_sensitive(GTK_WIDGET(name), TRUE);

         obj = list->data;
         if (obj->name)
            gtk_entry_set_text(GTK_ENTRY(name), obj->name);

         gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(alignment_combo)->entry),
                            geist_object_get_alignment_string(obj));
         gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(sizemode_combo)->entry),
                            geist_object_get_sizemode_string(obj));

         if (geist_object_get_state(obj, VISIBLE))
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(vis_toggle), TRUE);

         new_hbox = obj->display_props(obj);

         obj_hbox = new_hbox;

         gtk_table_attach(GTK_TABLE(table), obj_hbox, 0, 4, 1, 2,
                          GTK_FILL | GTK_EXPAND, 0, 2, 2);

         gtk_widget_show(new_hbox);
         gtk_window_set_title(GTK_WINDOW(obj_win), obj->name);
         gtk_window_set_title(GTK_WINDOW(props_window), obj->name);
      }

   }
   efree(list);
   gtk_signal_handler_unblock_by_func(GTK_OBJECT(vis_toggle),
                                      GTK_SIGNAL_FUNC(obj_vis_cb), NULL);
   gtk_signal_handler_unblock_by_func(GTK_OBJECT(name), refresh_name_cb,
                                      NULL);
   gtk_signal_handler_unblock_by_func(GTK_OBJECT
                                      (GTK_COMBO(alignment_combo)->entry),
                                      refresh_alignment_cb, NULL);
   gtk_signal_handler_unblock_by_func(GTK_OBJECT
                                      (GTK_COMBO(sizemode_combo)->entry),
                                      refresh_sizemode_cb, NULL);

   D_RETURN_(3);
}


void
conf_ok_cb (GtkWidget *widget, gpointer data)
{
	geist_confirmation_dialog_data *dialog = (geist_confirmation_dialog_data*)data;
	
	dialog->value = TRUE;
	gtk_widget_destroy(dialog->dialog);
	g_main_quit(dialog->loop);
}

void
conf_cancel_cb (GtkWidget *widget, gpointer data)
{
	geist_confirmation_dialog_data *dialog = (geist_confirmation_dialog_data*)data;
	
	dialog->value = FALSE;
	gtk_widget_destroy(dialog->dialog);
	g_main_quit(dialog->loop);
}

gboolean
geist_confirmation_dialog_new_with_text (char *text)
{
	geist_confirmation_dialog_data *data;
	GMainLoop *loop;
	gboolean ret;
	
	GtkWidget *dialog, *label, *ok_button, *cancel_button, *table;
	D_ENTER(3);
	data = (geist_confirmation_dialog_data *) emalloc
			(sizeof(geist_confirmation_dialog_data));
	
	dialog = gtk_window_new (GTK_WINDOW_DIALOG);
	table = gtk_table_new (2, 2, TRUE);
	
	loop = g_main_new(FALSE);
	
	data->dialog = dialog;
	data->value = TRUE;
	data->loop = loop;
	
	gtk_container_set_border_width(GTK_CONTAINER(dialog), 5);
	gtk_container_set_border_width(GTK_CONTAINER(table), 5);
	
	gtk_container_add(GTK_CONTAINER(dialog), table);
	gtk_widget_show(table);
	
	label = gtk_label_new(text);
	gtk_table_attach_defaults(GTK_TABLE(table), label, 0,2,0,1);
	gtk_widget_show(label);
	
	ok_button = gtk_button_new_with_label("Ok");
	gtk_table_attach_defaults(GTK_TABLE(table), ok_button, 
										0, 1, 1, 2);
	gtk_container_set_border_width(GTK_CONTAINER(ok_button), 5);
	gtk_widget_show(ok_button);
	
	cancel_button = gtk_button_new_with_label("Cancel");
	gtk_table_attach_defaults(GTK_TABLE(table), cancel_button, 
										1, 2, 1, 2);
	gtk_container_set_border_width(GTK_CONTAINER(cancel_button), 5);
	gtk_widget_show(cancel_button);
	
	gtk_signal_connect (GTK_OBJECT(ok_button), "clicked",
								GTK_SIGNAL_FUNC(conf_ok_cb),(gpointer) data);
	gtk_signal_connect (GTK_OBJECT(cancel_button), "clicked",
								GTK_SIGNAL_FUNC(conf_cancel_cb),(gpointer) data);
	
	gtk_widget_show(dialog);
	
	g_main_run(loop);
	g_main_destroy(loop);
	
	ret = data->value;
	efree(data);
	
	D_RETURN(3, ret);
	
}
