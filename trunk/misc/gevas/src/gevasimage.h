/*
 * Gtk abstraction of Evas_Object showing an image.
 *
 *
 *
 * Copyright (C) 2000 Ben Martin.
 *
 * Original author: Ben Martin
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
 *
 *
 */


#ifndef INC_GTK_GEVASIMAGE_H
#define INC_GTK_GEVASIMAGE_H

#include <gtk/gtkobject.h>
#include "gevasobj.h"
#include "gevas.h"

#ifdef __cplusplus
extern "C" {
#endif							/* __cplusplus */
#define GTK_GEVASIMAGE(obj)          GTK_CHECK_CAST (obj, gevasimage_get_type (), GtkgEvasImage)
#define GTK_GEVASIMAGE_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, gevasimage_get_type (), GtkgEvasImageClass)
#define GTK_IS_GEVASIMAGE(obj)       GTK_CHECK_TYPE (obj, gevasimage_get_type ())
#define GTK_GEVASIMAGE_IMAGENAME		"GtkgEvasImage::image_name"
#define gevasimage_set_image_name( gevaso, val ) \
	  gtk_object_set(GTK_OBJECT(gevaso), \
                  GTK_GEVASIMAGE_IMAGENAME, (gchar*) val, NULL);
	 typedef struct _GtkgEvasImage GtkgEvasImage;
	typedef struct _GtkgEvasImageClass GtkgEvasImageClass;

	struct _GtkgEvasImage {
		GtkgEvasObj gobj;
	};

	struct _GtkgEvasImageClass {
		GtkgEvasObjClass parent_class;

		 Evas(*_gevas_evas) (GtkObject * object);

		void (*set_image_fill) (GtkgEvasObj * object, double x, double y,
								double w, double h);
		void (*get_image_size) (GtkgEvasObj * object, int *w, int *h);
		void (*set_image_border) (GtkgEvasObj * object, int l, int r, int t,
								  int b);
		void (*get_image_border) (GtkgEvasObj * object, int *l, int *r, int *t,
								  int *b);

	};


	guint gevasimage_get_type(void);
	GtkgEvasImage *gevasimage_new(void);


	void gevasimage_set_image_fill(GtkgEvasObj * object, double x, double y,
								   double w, double h);
	void gevasimage_set_image_border(GtkgEvasObj * object, int l, int r, int t,
									 int b);
	void gevasimage_get_image_size(GtkgEvasObj * object, int *w, int *h);
	void gevasimage_get_image_border(GtkgEvasObj * object, int *l, int *r,
									 int *t, int *b);


#ifdef __cplusplus
}
#endif							/* __cplusplus */
#endif							/* __GTK_GEVASOBJ_H__ */
