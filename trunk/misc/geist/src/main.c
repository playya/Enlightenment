
/*  main.c  */

#include <gtk/gtk.h>

#include "utils.h"
#include "geist.h"
#include "geist_document.h"
#include "geist_object.h"
#include "geist_image.h"
#include "geist_gtk.h"
#include "geist_text.h"
#include "layers.h"

int call_level = 0;
GtkWidget *mainwin, *darea, *evbox, *scrollwin, *viewport;
geist_document *doc;

GtkWidget *obj_win;
GtkWidget *obj_list;
gint obj_sel_handler, obj_unsel_handler;
GtkWidget *obj_name;
gint obj_name_handler;
GtkWidget *obj_text;
gint obj_text_handler;
GtkWidget *obj_vis;
gint obj_vis_handler;

gboolean mainwin_delete_cb(GtkWidget * widget, GdkEvent * event,

                           gpointer user_data);
gboolean mainwin_destroy_cb(GtkWidget * widget, GdkEvent * event,

                            gpointer user_data);
gboolean configure_cb(GtkWidget * widget, GdkEventConfigure * event,

                      gpointer user_data);
gint evbox_buttonpress_cb(GtkWidget * widget, GdkEventButton * event);
gint evbox_buttonrelease_cb(GtkWidget * widget, GdkEventButton * event);
gint evbox_mousemove_cb(GtkWidget * widget, GdkEventMotion * event);
void idle_draw_cb(GtkWidget * widget, gpointer * data);

gboolean obj_add_cb(GtkWidget * widget, gpointer * data);
gboolean obj_cpy_cb(GtkWidget * widget, gpointer * data);
gboolean obj_del_cb(GtkWidget * widget, gpointer * data);
gboolean obj_sel_cb(GtkWidget * widget, int row, int column,
                    GdkEventButton * event, gpointer * data);
gboolean obj_unsel_cb(GtkWidget * widget, int row, int column,
                      GdkEventButton * event, gpointer * data);
gboolean obj_vis_cb(GtkWidget * widget, gpointer * data);
gboolean obj_name_cb(GtkWidget * widget, gpointer * data);
gboolean obj_text_cb(GtkWidget * widget, gpointer * data);
gboolean obj_load_cancel_cb(GtkWidget * widget, gpointer data);

int
main(int argc, char *argv[])
{
   GtkWidget *hwid, *vwid;
   GtkWidget *obj_table, *obj_btn, *obj_btn_hbox, *obj_scroll, *obj_name_l,

      *obj_text_l;

   D_ENTER(3);

   opt.debug_level = 5;

   printf("%s - version %s\n", PACKAGE, VERSION);


   gtk_init(&argc, &argv);

   mainwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_policy(GTK_WINDOW(mainwin), TRUE, TRUE, TRUE);
   gtk_window_set_wmclass(GTK_WINDOW(mainwin), "geist", "geist");
   gtk_signal_connect(GTK_OBJECT(mainwin), "delete_event",
                      GTK_SIGNAL_FUNC(mainwin_delete_cb), NULL);
   gtk_signal_connect(GTK_OBJECT(mainwin), "destroy_event",
                      GTK_SIGNAL_FUNC(mainwin_destroy_cb), NULL);
   gtk_window_set_default_size(GTK_WINDOW(mainwin), 500, 500);
   gtk_widget_show(mainwin);

   hwid = gtk_hbox_new(TRUE, 0);
   gtk_widget_show(hwid);
   gtk_container_add(GTK_CONTAINER(mainwin), hwid);

   vwid = gtk_vbox_new(TRUE, 0);
   gtk_widget_show(vwid);
   gtk_box_pack_start(GTK_BOX(hwid), vwid, TRUE, FALSE, 0);

   scrollwin = gtk_scrolled_window_new(NULL, NULL);
   gtk_widget_show(scrollwin);
   gtk_box_pack_start(GTK_BOX(vwid), scrollwin, TRUE, FALSE, 0);
   gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

   viewport = gtk_viewport_new(NULL, NULL);
   gtk_widget_show(viewport);
   gtk_container_add(GTK_CONTAINER(scrollwin), viewport);

   evbox = gtk_event_box_new();
   gtk_container_add(GTK_CONTAINER(viewport), evbox);
   gtk_widget_show(evbox);
   gtk_signal_connect(GTK_OBJECT(evbox), "button_press_event",
                      GTK_SIGNAL_FUNC(evbox_buttonpress_cb), NULL);
   gtk_signal_connect(GTK_OBJECT(evbox), "button_release_event",
                      GTK_SIGNAL_FUNC(evbox_buttonrelease_cb), NULL);
   gtk_signal_connect(GTK_OBJECT(evbox), "motion_notify_event",
                      GTK_SIGNAL_FUNC(evbox_mousemove_cb), NULL);

   /* The drawing area itself */
   darea = gtk_drawing_area_new();
   gtk_container_add(GTK_CONTAINER(evbox), darea);
   gtk_signal_connect_after(GTK_OBJECT(darea), "configure_event",
                            GTK_SIGNAL_FUNC(configure_cb), NULL);
   gtk_widget_show(darea);
   imlib_init(darea);

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
   obj_btn = gtk_button_new_with_label("Add...");
   gtk_signal_connect(GTK_OBJECT(obj_btn), "clicked",
                      GTK_SIGNAL_FUNC(obj_add_cb), NULL);
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
   gtk_widget_show(obj_btn);
   gtk_table_attach(GTK_TABLE(obj_table), obj_btn_hbox, 0, 3, 0, 1,
                    GTK_FILL | GTK_EXPAND, 0, 2, 2);
   gtk_widget_show(obj_btn_hbox);
   gtk_box_pack_start(GTK_BOX(obj_btn_hbox), obj_btn, TRUE, TRUE, 2);
   obj_name_l = gtk_label_new("Name:");
   gtk_misc_set_alignment(GTK_MISC(obj_name_l), 1.0, 0.5);
   gtk_table_attach(GTK_TABLE(obj_table), obj_name_l, 0, 1, 2, 3,
                    GTK_FILL | GTK_EXPAND, 0, 2, 2);
   obj_name = gtk_entry_new();
   gtk_table_attach(GTK_TABLE(obj_table), obj_name, 1, 3, 2, 3,
                    GTK_FILL | GTK_EXPAND, 0, 2, 2);
   obj_name_handler =
      gtk_signal_connect(GTK_OBJECT(obj_name), "changed",
                         GTK_SIGNAL_FUNC(obj_name_cb), NULL);
   obj_text_l = gtk_label_new("Text:");
   gtk_misc_set_alignment(GTK_MISC(obj_text_l), 1.0, 0.5);
   gtk_table_attach(GTK_TABLE(obj_table), obj_text_l, 0, 1, 3, 4,
                    GTK_FILL | GTK_EXPAND, 0, 2, 2);
   gtk_widget_show(obj_text_l);
   obj_text = gtk_entry_new();
   gtk_table_attach(GTK_TABLE(obj_table), obj_text, 1, 3, 3, 4,
                    GTK_FILL | GTK_EXPAND, 0, 2, 2);
   obj_text_handler =
      gtk_signal_connect(GTK_OBJECT(obj_text), "changed",
                         GTK_SIGNAL_FUNC(obj_text_cb), NULL);
   gtk_widget_set_sensitive(obj_text, FALSE);
   gtk_widget_show(obj_text);
   obj_vis = gtk_check_button_new_with_label("Visible");
   gtk_table_attach(GTK_TABLE(obj_table), obj_vis, 0, 3, 4, 5,
                    GTK_FILL | GTK_EXPAND, 0, 2, 2);
   obj_vis_handler =
      gtk_signal_connect(GTK_OBJECT(obj_vis), "clicked",
                         GTK_SIGNAL_FUNC(obj_vis_cb), NULL);
   gtk_widget_show(obj_vis);
   gtk_window_set_default_size(GTK_WINDOW(obj_win), 125, 230);
   gtk_widget_show(obj_list);
   gtk_widget_show(obj_scroll);
   gtk_widget_show(obj_name_l);
   gtk_widget_show(obj_name);
   gtk_widget_show(obj_table);
   gtk_widget_show(obj_win);

   doc = geist_document_new(500, 500);

   geist_document_add_layer(doc);

   geist_document_add_object(doc,
                             geist_image_new_from_file(0, 0,
                                                       PREFIX
                                                       "/share/geist/images/laet.jpg"));
   geist_document_add_object(doc,
                             geist_text_new_with_text(0, 405, "20thcent/16",
                                                      "Some pr0n - I have to."));
   geist_document_add_object(doc,
                             geist_image_new_from_file(220, 140,
                                                       PREFIX
                                                       "/share/geist/images/elogo.png"));

   geist_document_add_object(doc,
                             geist_image_new_from_file(125, 5,
                                                       PREFIX
                                                       "/share/geist/images/globe.png"));

   geist_document_add_object(doc,
                             geist_image_new_from_file(175, 125,
                                                       PREFIX
                                                       "/share/geist/images/bulb.png"));

   geist_document_add_object(doc,
                             geist_image_new_from_file(375, 145,
                                                       PREFIX
                                                       "/share/geist/images/bulb.png"));
   geist_document_add_object(doc,
                             geist_image_new_from_file(415, 200,
                                                       PREFIX
                                                       "/share/geist/images/mail.png"));

   geist_document_add_object(doc,
                             geist_image_new_from_file(445, 305,
                                                       PREFIX
                                                       "/share/geist/images/mush.png"));

   geist_document_add_object(doc,
                             geist_image_new_from_file(315, 405,
                                                       PREFIX
                                                       "/share/geist/images/paper.png"));
   geist_document_add_object(doc,
                             geist_text_new_with_text(275, 15, "20thcent/16",
                                                      "So this is geist..."));


   geist_document_render(doc);
   geist_document_render_selection(doc);
   geist_document_render_pmap(doc);
   gtk_widget_set_usize(scrollwin, doc->w, doc->h);

   geist_document_render_to_gtk_window(doc, darea);

   gtk_main();
   D_RETURN(3, 0);
}

gboolean mainwin_delete_cb(GtkWidget * widget, GdkEvent * event,
                           gpointer user_data)
{
   D_ENTER(3);
   gtk_exit(0);
   D_RETURN(3, FALSE);
}

gboolean mainwin_destroy_cb(GtkWidget * widget, GdkEvent * event,
                            gpointer user_data)
{
   D_ENTER(3);
   gtk_exit(0);
   D_RETURN(3, FALSE);
}


gboolean
configure_cb(GtkWidget * widget, GdkEventConfigure * event,
             gpointer user_data)
{
   D_ENTER(3);

   geist_document_render_to_gtk_window(doc, darea);

   D_RETURN(3, TRUE);
}

gint
evbox_buttonpress_cb(GtkWidget * widget, GdkEventButton * event)
{
   geist_object *obj;

   D_ENTER(5);

   if (event->button == 1)
   {
      geist_list *l, *list;
      int row;


      obj = geist_document_find_clicked_object(doc, event->x, event->y);
      if (!obj)
         D_RETURN(5, 1);

      gtk_signal_handler_block(GTK_OBJECT(obj_list), obj_sel_handler);
      gtk_signal_handler_block(GTK_OBJECT(obj_list), obj_unsel_handler);

      if (event->state & GDK_SHIFT_MASK)
      {
         /* shift click - multiple selections and selection toggling */
         geist_list *ll;

         row =
            gtk_clist_find_row_from_data(GTK_CLIST(obj_list), (gpointer) obj);
         if (geist_object_get_state(obj, SELECTED))
         {
            geist_object_unset_state(obj, SELECTED);
            if (row != -1)
               gtk_clist_unselect_row(GTK_CLIST(obj_list), row, 0);
         }
         else
         {
            geist_object_set_state(obj, SELECTED);
            if (row != -1)
               gtk_clist_select_row(GTK_CLIST(obj_list), row, 0);
         }

         /* need to dirty all the other selected objects, in case they go from
          * being the only thing selected to part of a multiple selection -
          * hence have to change their selection type */
         ll = geist_document_get_selected_list(doc);
         if (ll)
         {
            geist_list *lll;

            for (lll = ll; lll; lll = lll->next)
            {
               geist_document_dirty_object_selection(doc,
                                                     GEIST_OBJECT(lll->data));
            }
            geist_list_free(ll);
         }
      }
      else
      {
         if (!geist_object_get_state(obj, SELECTED))
         {
            /* Single click selection */
            geist_document_unselect_all(doc);
            gtk_clist_unselect_all(GTK_CLIST(obj_list));
            geist_object_select(doc, obj);
            row =
               gtk_clist_find_row_from_data(GTK_CLIST(obj_list),
                                            (gpointer) obj);
            if (row != -1)
               gtk_clist_select_row(GTK_CLIST(obj_list), row, 0);

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
               geist_object_raise(doc, obj);
               geist_document_dirty_object(doc, obj);
            }
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

gint evbox_buttonrelease_cb(GtkWidget * widget, GdkEventButton * event)
{
   geist_list *list, *l;
   geist_object *obj;

   D_ENTER(5);

   list = gtk_object_get_data(GTK_OBJECT(mainwin), "draglist");
   if (list)
   {
      for (l = list; l; l = l->next)
      {
         obj = GEIST_OBJECT(l->data);

         D(2, ("unsetting object state DRAG\n"));
         geist_object_unset_state(obj, DRAG);
         geist_document_dirty_object(doc, obj);
      }
   }
   geist_list_free(list);
   gtk_object_set_data_full(GTK_OBJECT(mainwin), "draglist", NULL, NULL);
   geist_document_render_updates(doc);

   D_RETURN(5, 1);
}

gint evbox_mousemove_cb(GtkWidget * widget, GdkEventMotion * event)
{
   geist_list *l, *list;
   geist_object *obj;
   GdkEventMotion *e;

   D_ENTER(5);

   if (gdk_events_pending())
   {
      if ((e = (GdkEventMotion *) gdk_event_get()) != NULL)
      {
         if (e->type == GDK_MOTION_NOTIFY)
         {
            D(5, ("skipping event, new one coming\n"));
            event = e;
         }
         else
            gdk_event_put((GdkEvent *) e);
      }
   }

   list = gtk_object_get_data(GTK_OBJECT(mainwin), "draglist");
   if (list)
   {
      for (l = list; l; l = l->next)
      {
         obj = GEIST_OBJECT(l->data);
         D(5, ("moving object to %f, %f\n", event->x, event->y));
         geist_document_dirty_object(doc, obj);
         obj->x = event->x - obj->clicked_x;
         obj->y = event->y - obj->clicked_y;
         geist_document_dirty_object(doc, obj);
      }
      geist_document_render_updates(doc);
   }

   D_RETURN(5, 1);
}

gboolean obj_load_cb(GtkWidget * widget, gpointer data)
{
   geist_object *obj = NULL;
   char *path;

   D_ENTER(3);

   path =
      gtk_file_selection_get_filename(GTK_FILE_SELECTION((GtkWidget *) data));
   if (path)
   {
      obj = geist_image_new_from_file(0, 0, path);
      if (obj)
      {
         geist_document_add_object(doc, obj);
         geist_object_show(obj);
         geist_object_raise(doc, obj);
         geist_document_dirty_object(doc, obj);
         geist_document_render_updates(doc);
      }
   }
   gtk_widget_destroy((GtkWidget *) data);
   D_RETURN(3, TRUE);
}

gboolean obj_load_cancel_cb(GtkWidget * widget, gpointer data)
{
   gtk_widget_destroy((GtkWidget *) data);
   return TRUE;
}

gboolean obj_add_cb(GtkWidget * widget, gpointer * data)
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

gboolean obj_cpy_cb(GtkWidget * widget, gpointer * data)
{
   geist_object *new;
   geist_list *l, *list;
   geist_object *obj;
   int row;

   D_ENTER(3);

   list = geist_document_get_selected_list(doc);
   for (l = list; l; l = l->next)
   {
      obj = GEIST_OBJECT(l->data);
      new = geist_object_duplicate(obj);
      if (new)
      {
         new->x += 5;
         new->y += 5;
         geist_document_add_object(doc, new);

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
   geist_document_render_updates(doc);

   D_RETURN(3, TRUE);
}

gboolean obj_del_cb(GtkWidget * widget, gpointer * data)
{
   geist_object *obj;
   geist_list *l, *list;

   D_ENTER(3);

   list = geist_document_get_selected_list(doc);
   for (l = list; l; l = l->next)
   {
      obj = GEIST_OBJECT(l->data);
      geist_document_remove_object(doc, obj);
      geist_object_free(obj);
      gtk_clist_remove(GTK_CLIST(obj_list),
                       gtk_clist_find_row_from_data(GTK_CLIST(obj_list),
                                                    obj));
   }
   geist_list_free(list);
   geist_document_render_updates(doc);

   D_RETURN(3, TRUE);
}

gboolean obj_vis_cb(GtkWidget * widget, gpointer * data)
{
   geist_object *obj;
   geist_list *list;

   D_ENTER(3);

   list = geist_document_get_selected_list(doc);
   if (!list)
      D_RETURN(3, TRUE);

   /* TODO Work for multiple selections */
   obj = (geist_object *) list->data;
   geist_list_free(list);

   if (obj)
   {
      if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
         geist_object_show(obj);
      else
         geist_object_hide(obj);
      geist_document_dirty_object(doc, obj);
      geist_document_render_updates(doc);
   }
   D_RETURN(3, TRUE);
}

gboolean obj_text_cb(GtkWidget * widget, gpointer * data)
{
   geist_object *obj;
   geist_list *list;

   D_ENTER(3);

   list = geist_document_get_selected_list(doc);
   if (!list)
      D_RETURN(3, TRUE);

   /* TODO Work for multiple selections */
   obj = (geist_object *) list->data;

   geist_list_free(list);

   if (obj)
   {
      if (geist_object_get_type(obj) != GEIST_TYPE_TEXT)
         D_RETURN(3, TRUE);
      geist_document_dirty_object(doc, obj);
      geist_text_change_text(GEIST_TEXT(obj),
                             estrdup(gtk_entry_get_text(GTK_ENTRY(widget))));
      geist_document_dirty_object(doc, obj);
      geist_document_render_updates(doc);
   }
   D_RETURN(3, TRUE);
}

gboolean obj_name_cb(GtkWidget * widget, gpointer * data)
{
   geist_object *obj;
   geist_list *list;

   D_ENTER(3);

   list = geist_document_get_selected_list(doc);
   if (!list)
      D_RETURN(3, TRUE);

   /* TODO Work for multiple selections */
   obj = (geist_object *) list->data;

   geist_list_free(list);

   if (obj)
   {

      gtk_clist_set_text(GTK_CLIST(obj_list),
                         gtk_clist_find_row_from_data(GTK_CLIST(obj_list),
                                                      obj), 0,
                         gtk_entry_get_text(GTK_ENTRY(widget)));
      obj->name = estrdup(gtk_entry_get_text(GTK_ENTRY(widget)));
   }
   D_RETURN(3, TRUE);
}

gboolean obj_sel_cb(GtkWidget * widget, int row, int column,
                    GdkEventButton * event, gpointer * data)
{
   GList *selection;
   geist_object *obj;
   geist_list *l, *list;

   D_ENTER(3);

   obj = (geist_object *) gtk_clist_get_row_data(GTK_CLIST(widget), row);
   if (obj)
   {
      geist_object_select(doc, obj);

      selection = GTK_CLIST(widget)->selection;
      if (g_list_length(selection) > 1)
      {
         gtk_widget_set_sensitive(obj_text, FALSE);
         gtk_widget_set_sensitive(obj_vis, FALSE);
         list = geist_document_get_selected_list(doc);
         for (l = list; l; l = l->next)
         {
            geist_document_dirty_object_selection(doc, GEIST_OBJECT(l->data));
         }
         geist_list_free(list);
      }
      else
      {
         gtk_signal_handler_block(GTK_OBJECT(obj_name), obj_name_handler);
         gtk_signal_handler_block(GTK_OBJECT(obj_vis), obj_vis_handler);
         gtk_signal_handler_block(GTK_OBJECT(obj_text), obj_text_handler);
         gtk_entry_set_text(GTK_ENTRY(obj_name), obj->name ? obj->name : "");
         gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(obj_vis),
                                      obj->visible);
         if (geist_object_get_type(obj) == GEIST_TYPE_TEXT)
         {
            gtk_widget_set_sensitive(obj_text, TRUE);
            gtk_entry_set_text(GTK_ENTRY(obj_text), GEIST_TEXT(obj)->text);
         }
         else
         {
            gtk_widget_set_sensitive(obj_text, FALSE);
            gtk_entry_set_text(GTK_ENTRY(obj_text), "");
         }
         gtk_signal_handler_unblock(GTK_OBJECT(obj_name), obj_name_handler);
         gtk_signal_handler_unblock(GTK_OBJECT(obj_vis), obj_vis_handler);
         gtk_signal_handler_unblock(GTK_OBJECT(obj_text), obj_text_handler);
      }
      geist_document_render_updates(doc);
   }

   D_RETURN(3, TRUE);
}

gboolean obj_unsel_cb(GtkWidget * widget, int row, int column,
                      GdkEventButton * event, gpointer * data)
{
   GList *selection;
   geist_object *obj;
   geist_list *l, *list;

   D_ENTER(3);

   obj = (geist_object *) gtk_clist_get_row_data(GTK_CLIST(widget), row);
   if (obj)
   {
      geist_object_unselect(doc, obj);
      geist_document_dirty_object(doc, obj);

      selection = GTK_CLIST(widget)->selection;
      if (g_list_length(selection) > 1)
      {
         gtk_widget_set_sensitive(obj_text, FALSE);
         gtk_widget_set_sensitive(obj_vis, FALSE);
         list = geist_document_get_selected_list(doc);
         for (l = list; l; l = l->next)
         {
            geist_document_dirty_object_selection(doc, GEIST_OBJECT(l->data));
         }
         geist_list_free(list);
      }
      else
      {
         gtk_signal_handler_block(GTK_OBJECT(obj_name), obj_name_handler);
         gtk_signal_handler_block(GTK_OBJECT(obj_vis), obj_vis_handler);
         gtk_signal_handler_block(GTK_OBJECT(obj_text), obj_text_handler);
         gtk_entry_set_text(GTK_ENTRY(obj_name), obj->name ? obj->name : "");
         gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(obj_vis),
                                      obj->visible);
         if (geist_object_get_type(obj) == GEIST_TYPE_TEXT)
         {
            gtk_widget_set_sensitive(obj_text, TRUE);
            gtk_entry_set_text(GTK_ENTRY(obj_text), GEIST_TEXT(obj)->text);
         }
         else
         {
            gtk_widget_set_sensitive(obj_text, FALSE);
            gtk_entry_set_text(GTK_ENTRY(obj_text), "");
         }
         gtk_signal_handler_unblock(GTK_OBJECT(obj_name), obj_name_handler);
         gtk_signal_handler_unblock(GTK_OBJECT(obj_vis), obj_vis_handler);
         gtk_signal_handler_unblock(GTK_OBJECT(obj_text), obj_text_handler);
      }
      geist_document_render_updates(doc);
   }

   D_RETURN(3, TRUE);
}
