#include "geist_document_gtk.h"
#include "geist_document_xml.h"
gboolean file_save_ok_cb(GtkWidget * widget, gpointer * data);

void
geist_document_render_to_window(geist_document * doc)
{
   Window xwin;

   D_ENTER(3);

   xwin = GDK_WINDOW_XWINDOW(doc->darea->window);

   XSetWindowBackgroundPixmap(disp, xwin, doc->pmap);

   XClearWindow(disp, xwin);

   D_RETURN_(3);
}

void
geist_document_render_to_window_partial(geist_document * doc, int x, int y,
                                        int w, int h)
{
   Window xwin;

   D_ENTER(3);

   xwin = GDK_WINDOW_XWINDOW(doc->darea->window);

   XSetWindowBackgroundPixmap(disp, xwin, doc->pmap);

   XClearArea(disp, xwin, x, y, w, h, False);

   D_RETURN_(3);
}

void
geist_document_save_as(geist_document * doc)
{
   GtkWidget *fs;

   D_ENTER(3);

   fs = gtk_file_selection_new("Save document as....");
   gtk_signal_connect(GTK_OBJECT(fs), "destroy",
                      (GtkSignalFunc) gtk_widget_destroy, GTK_OBJECT(fs));

   gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(fs)->ok_button),
                      "clicked", (GtkSignalFunc) file_save_ok_cb, fs);

   gtk_signal_connect_object(GTK_OBJECT
                             (GTK_FILE_SELECTION(fs)->cancel_button),
                             "clicked", (GtkSignalFunc) gtk_widget_destroy,
                             GTK_OBJECT(fs));

   gtk_file_selection_set_filename(GTK_FILE_SELECTION(fs), "test.xml");
   gtk_object_set_data(GTK_OBJECT(fs), "doc", doc);
   gtk_widget_show(fs);

   D_RETURN_(3);
}

gboolean file_save_ok_cb(GtkWidget * widget, gpointer * data)
{
   char *filename;
   geist_document *doc;

   D_ENTER(3);

   filename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(data));
   doc = gtk_object_get_data(GTK_OBJECT(data), "doc");
   if (doc)
   {
      if (doc->filename)
         efree(doc->filename);
      doc->filename = estrdup(filename);
      geist_project_save_xml(doc, filename);
   }
   gtk_widget_destroy(GTK_WIDGET(data));

   D_RETURN(3, TRUE);
}
