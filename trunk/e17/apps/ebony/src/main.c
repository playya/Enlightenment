/*
 * Initial main.c file generated by Glade. Edit as required.
 * Glade will not overwrite this file.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#include "interface.h"
#include "support.h"
#include "ebony.h"
#include "gtk_util.h"

void
setup_evas(Display * disp, Window win, Visual * vis, Colormap cm, int w,
           int h)
{
   Evas_Object o;
   int colors[] = { 255, 255, 255, 255 };

   evas = evas_new();
   evas_set_output_method(evas, RENDER_METHOD_ALPHA_SOFTWARE);
   evas_set_output(evas, disp, win, vis, cm);
   evas_set_output_size(evas, w, h);
   evas_set_output_viewport(evas, 0, 0, w, h);
   evas_set_font_cache(evas, ((1024 * 1024) * 1));
   evas_set_image_cache(evas, ((1024 * 1024) * 4));
   evas_font_add_path(evas, PACKAGE_DATA_DIR "/fnt/");
   o = evas_add_rectangle(evas);
   evas_move(evas, o, 0, 0);
   evas_resize(evas, o, 999999, 999999);
   evas_set_color(evas, o, 255, 255, 255, 255);
   evas_set_layer(evas, o, -100);
   evas_show(evas, o);

   o = evas_add_line(evas);
   evas_object_set_name(evas, o, "top_line");
   evas_set_color(evas, o, colors[0], colors[1], colors[2], colors[3]);
   evas_show(evas, o);

   o = evas_add_line(evas);
   evas_object_set_name(evas, o, "bottom_line");
   evas_set_color(evas, o, colors[0], colors[1], colors[2], colors[3]);
   evas_show(evas, o);

   o = evas_add_line(evas);
   evas_object_set_name(evas, o, "left_line");
   evas_set_color(evas, o, colors[0], colors[1], colors[2], colors[3]);
   evas_show(evas, o);

   o = evas_add_line(evas);
   evas_object_set_name(evas, o, "right_line");
   evas_set_color(evas, o, colors[0], colors[1], colors[2], colors[3]);
   evas_show(evas, o);
}

void
init_globals(void)
{
   win_ref = NULL;
   ebony_status = NULL;

   recent_bgs = NULL;

   evas = NULL;
   bg = NULL;
   bl = NULL;
   idle = 0;
}

int
main(int argc, char *argv[])
{
   GtkWidget *win, *w;
   int rx, ry, rw, rh, rd;
   char bgfile[PATH_MAX];

   bgfile[0] = '\0';

   init_globals();
   gtk_set_locale();
   gtk_init(&argc, &argv);

   add_pixmap_directory(PACKAGE_DATA_DIR "/pixmaps");
   add_pixmap_directory(PACKAGE_SOURCE_DIR "/pixmaps");

   if (argv[1])
      snprintf(bgfile, PATH_MAX, "%s", argv[1]);
   /* 
    * The following code was added by Glade to create one of each component
    * (except popup menus), just so that you see something after building
    * the project. Delete any components that you don't want shown initially.
    */
   win = create_main_win();
   win_ref = win;

   gdk_window_get_geometry(GDK_ROOT_PARENT(), &rx, &ry, &rw, &rh, &rd);
   gtk_widget_realize(GTK_WIDGET(win));

   /* realize the drwaing areas */
   w = gtk_object_get_data(GTK_OBJECT(win), "evas");
   gtk_widget_realize(w);


   /* setup the evas stuffs */
   setup_evas(GDK_WINDOW_XDISPLAY(w->window), GDK_WINDOW_XWINDOW(w->window),
              GDK_VISUAL_XVISUAL(gtk_widget_get_visual(win)),
              GDK_COLORMAP_XCOLORMAP(gtk_widget_get_colormap(win)),
              w->allocation.width, w->allocation.height);

   gtk_widget_show(win);
   win_ref = win;

   /* drawing area requests initialization */
   gdk_rgb_init();

   w = gtk_object_get_data(GTK_OBJECT(win), "color_box");
   if (w)
      gtk_widget_realize(w);

   w = gtk_object_get_data(GTK_OBJECT(win), "gradient_one_color_box");
   if (w)
      gtk_widget_realize(w);

   w = gtk_object_get_data(GTK_OBJECT(win), "gradient_two_color_box");
   if (w)
      gtk_widget_realize(w);

   w = gtk_object_get_data(GTK_OBJECT(win), "_ebony_statusbar");
   if (w)
      ebony_status = w;

   /* load the file passed as argv[1] or create a new bg */
   if (!strlen(bgfile))
      open_bg_named(PACKAGE_DATA_DIR "/pixmaps/ebony.bg.db");
   else
      open_bg_named(bgfile);
      
   set_spin_value("layer_num", 0);

   gtk_widget_set_usize(win, 600, 380);

   gtk_main();
   return 0;
}
