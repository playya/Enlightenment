/*
 * Initial main.c file generated by Glade. Edit as required.
 * Glade will not overwrite this file.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <gdk/gdkx.h>

#include <string.h>

#include "interface.h"
#include "support.h"
#include "utils.h"

GtkWidget *colorsel;
GtkWidget *filesel;
GtkWidget *window;

int
main (int argc, char *argv[])
{
#ifdef ENABLE_NLS
  bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
  textdomain (PACKAGE);
#endif

  gtk_set_locale ();
  gtk_init (&argc, &argv);

  add_pixmap_directory (PACKAGE_DATA_DIR "/pixmaps");
  add_pixmap_directory (PACKAGE_SOURCE_DIR "/pixmaps");

  /*
   * The following code was added by Glade to create one of each component
   * (except popup menus), just so that you see something after building
   * the project. Delete any components that you don't want shown initially.
   */

  filesel = create_filesel ();
  colorsel = create_colorsel ();
   
  window = create_window ();
     {
	GtkWidget *w;	
	
	gtk_widget_realize(GTK_WIDGET(window));
	w = gtk_object_get_data(GTK_OBJECT(window), "draw");
	gtk_widget_realize(GTK_WIDGET(w));
	e_setup_evas(GDK_WINDOW_XDISPLAY(w->window),
		     GDK_WINDOW_XWINDOW(w->window),
		     GDK_VISUAL_XVISUAL(gtk_widget_get_visual(window)),
		     GDK_COLORMAP_XCOLORMAP(gtk_widget_get_colormap(window)),
		     w->allocation.width,
		     w->allocation.height);
     }
   
     {
	int i;
	
	for (i = 1; i < argc; i++)
	  {
	     if (argv[i][0] != '-')
	       {
		  background = e_background_load(argv[i]);
		  break;
	       }
	  }
     }
   
   if (!background)
     {
	background = e_background_new();
	  {
	     E_Background_Layer *bl;
	     
	     bl = malloc(sizeof(E_Background_Layer));
	     memset(bl, 0, sizeof(E_Background_Layer));
	     bl->size.w = 1.0;
	     bl->size.h = 1.0;
	     bl->fill.w = 1.0;
	     bl->fill.h = 1.0;
	     background->layers = evas_list_append(background->layers, bl);
	     bl->obj = evas_add_image_from_file(evas, bl->file);
	     display_layer(bl);
	  }
     }
   else
     {
	if (background->layers)
	  display_layer(background->layers->data);
     }
   e_background_realize(background, evas);
   e_background_set_size(background, 320, 240);
   gtk_widget_show (window);

  gtk_main ();
  return 0;
}

