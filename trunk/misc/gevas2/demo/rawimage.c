/*
 * Copyright (C) 2002 Ben Martin
 *
 * See COPYING for full details of copying & use of this software.
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include "config.h"

#include <gevas.h>
#include <gevasimage.h>
#include <gevastext.h>
#include <gevasevh_alpha.h>
#include <gevasevh_drag.h>
#include <gevasevh_to_gtk_signals.h>
#include <gevasevh_emouse_over.h>
#include <gevasevh_popup.h>
#include <gevasevh_obj_changer.h>
#include <gevasevh_clicks.h>
#include <gevasevh_group_selector.h>
#include <gevasevh_selectable.h>
#include <gevastwin.h>
#include <gevasgrad.h>

#include <gtk/gtk.h>

#include <stdio.h>

GtkgEvasImage* gimage = 0;
GtkWidget *gevas;

GtkWidget* createAndShowWindow()
{
	GtkWidget *window;
    GtkWidget *wtoy;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gevas_new_gtkscrolledwindow( &gevas, &wtoy );

    gevas_add_fontpath( gevas, PACKAGE_DATA_DIR );
    gevas_add_fontpath( gevas, g_get_current_dir() );
    gevas_add_fontpath( gevas, "/usr/X11R6/lib/X11/fonts/msttcorefonts" );

    gtk_container_add(GTK_CONTAINER(window), wtoy);
    gtk_widget_set_usize(gevas, 3000, 3000);
	gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
	gevas_set_size_request_x(gevas, 200);
	gevas_set_size_request_y(gevas, 200);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(wtoy),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    
	gevas_set_checked_bg(gevas, 1);
	gtk_window_set_title(GTK_WINDOW(window), "test window");

	gevas_set_middleb_scrolls(GTK_GEVAS(gevas), 1,
							  gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(wtoy)),
							  gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(wtoy)));
    
/*     gtk_signal_connect(GTK_OBJECT(window), */
/* 					   "delete_event", GTK_SIGNAL_FUNC(delete_event_cb), NULL); */

    GtkgEvasImage* gi = gimage = gevasimage_new();
    gevasobj_set_gevas( gi, gevas );
    GtkgEvasObj* go = GTK_GEVASOBJ( gi );

    gevasimage_set_image_name( gi, "raptor.png" );
    gevasobj_move(      go, 0, 0 );
    gevasobj_set_layer( go, 1 );
    gevasobj_show(      go );
    
    gtk_widget_show_all(window);
    return window;
}

gboolean duplicateImage( gpointer user_data )
{
    GtkWidget* widget = (GtkWidget*)user_data;
    GtkgEvasObj* go = GTK_GEVASOBJ( gimage );
    Evas_Object *eo = gevasobj_get_evasobj( GTK_OBJECT( go ));
    double w,h;
    gevasobj_get_size( go, &w, &h );

//    int w,h;
/*  w = gevasimage_get_image_fill_width(  go ); */
/*  h = gevasimage_get_image_fill_height( go ); */
    
    int* data = evas_object_image_data_get( eo, 0 );

    printf("duplicateImage() w:%f h:%f data:%p\n", w, h, data );
    
    GtkgEvasImage* gnew = gevasimage_new();
    gevasobj_set_gevas( gnew, gevas );
    go = GTK_GEVASOBJ( gnew );
    eo = gevasobj_get_evasobj( GTK_OBJECT( go ));
    
    evas_object_image_size_set( eo, w, h );
    evas_object_image_fill_set( eo, 0, 0, w, h );
    evas_object_image_data_set( eo, data );

    gevasobj_move( go, 100, 100 );
    gevasobj_set_layer( go, 2 );
    gevasobj_show( go );

    gevas_queue_redraw( gevas );
    
    return 0;
}

int main(int argc, char *argv[])
{
    GtkWidget* win1;
    
    gtk_init(&argc, &argv);

    win1 = createAndShowWindow();
    g_timeout_add( 2000, duplicateImage, &win1 );
    gtk_main();
    return 0;
}
