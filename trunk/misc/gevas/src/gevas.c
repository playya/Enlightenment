/*
 * A Gtk+ wrapper for an Evas.
 *
 * Developed by Ben Martin based on gtkev sample widget by Havoc Pennington from GGAD.
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

/* If this widget was in an application or library, 
 * this i18n stuff would be in some other header file.
 * (in Gtk, gtkintl.h; in the Gnome libraries, libgnome/gnome-i18nP.h; 
 *  in a Gnome application, libgnome/gnome-i18n.h)
 */

#include "config.h"

#include "project.h"

#include <Evas.h>
#include <Edb.h>

#include <gevas.h>
#include <gevasev_handler.h>
#include <gevas_util.h>

#include <gtk/gtkmain.h>
#include <gtk/gtkselection.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>



/* Always disable NLS, since we have no config.h; 
 * a real app would not do this of course.
 */
#undef ENABLE_NLS

#ifdef ENABLE_NLS
#include<libintl.h>
#define _(String) dgettext("gtk+",String)
#ifdef gettext_noop
#define N_(String) gettext_noop(String)
#else
#define N_(String) (String)
#endif
#else							/* NLS is disabled */
#define _(String) (String)
#define N_(String) (String)
#define textdomain(String) (String)
#define gettext(String) (String)
#define dgettext(Domain,String) (String)
#define dcgettext(Domain,String,Type) (String)
#define bindtextdomain(Domain,Directory) (Domain)
#endif							/* ENABLE_NLS */


#define GEVAS_CHECKED_BG_IMAGE_FILENAME    PACKAGE_DATA_DIR"/checks.png"

enum {
	ARG_0,						/* Skip 0, an invalid argument ID */
	ARG_CHECKED_BG,
	ARG_RENDER_MODE,
	ARG_EVAS,					/* READ ONLY */
	ARG_FONT_CACHE,
	ARG_IMAGE_CACHE,
	ARG_SIZE_REQUEST_X,
	ARG_SIZE_REQUEST_Y,
	ARG_DRAWABLE_SIZE_W,		/* READ ONLY */
	ARG_DRAWABLE_SIZE_H,		/* READ ONLY */
	ARG_SMOOTHNESS,				/* WRITE ONLY */
	ARG_VIEWPORT_X,
	ARG_VIEWPORT_Y,
	ARG_VIEWPORT_W,
	ARG_VIEWPORT_H
};



static void gevas_class_init(GtkgEvasClass * klass);
static void gevas_init(GtkgEvas * ev);


/* GtkObject functions */
static void gevas_destroy(GtkObject * object);


/* GtkWidget functions */

static gint gevas_event(GtkWidget * widget, GdkEvent * event);

static void gevas_realize(GtkWidget * widget);
static void gevas_unrealize(GtkWidget * widget);
static void gevas_size_request(GtkWidget * widget,
							   GtkRequisition * requisition);
static void gevas_size_allocate(GtkWidget * widget, GtkAllocation * allocation);

static void gevas_draw(GtkWidget * widget, GdkRectangle * area);
static void gevas_draw_focus(GtkWidget * widget);

static gint gevas_expose(GtkWidget * widget, GdkEventExpose * event);
static gint gevas_focus_in(GtkWidget * widget, GdkEventFocus * event);
static gint gevas_focus_out(GtkWidget * widget, GdkEventFocus * event);

static void gevas_map(GtkWidget * widget);

static void gevas_get_arg(GtkObject * object, GtkArg * arg, guint arg_id);
static void gevas_set_arg(GtkObject * object, GtkArg * arg, guint arg_id);


/* GtkgEvas-specific functions */

static void gevas_paint(GtkgEvas * ev, GdkRectangle * area);

#define EVAS(ev) ev->evas

#define EVASO_TO_GTKO(evaso) g_hash_table_lookup( ev->gevasobjs, evaso )
/* ((GtkObject*)0) */

void _register_gevasobj(GtkgEvas * thisp, GtkObject * gobj)
{

	g_hash_table_insert(thisp->gevasobjs, gevasobj_get_evasobj(gobj), gobj);

	thisp->gevasobjlist = g_slist_append(thisp->gevasobjlist, gobj);
	gtk_object_ref(gobj);
	gtk_object_sink(gobj);

}


GtkObject *gevas_get_object_under_mouse(GtkgEvas * ev)
{
	return EVASO_TO_GTKO(evas_get_object_under_mouse(EVAS(ev)));
}

GtkObject *gevas_object_in_rect(GtkgEvas * ev, double x, double y, double w,
								double h)
{
	return EVASO_TO_GTKO(evas_object_in_rect(EVAS(ev), x, y, w, h));
}

GtkObject *gevas_object_at_position(GtkgEvas * ev, double x, double y)
{
	return EVASO_TO_GTKO(evas_object_at_position(EVAS(ev), x, y));
}

GtkObject *gevas_object_get_named(GtkgEvas * ev, char *name)
{
	return EVASO_TO_GTKO(evas_object_get_named(EVAS(ev), name));
}

void gevas_add_fontpath(GtkgEvas * ev, const gchar * path)
{
/*	printf(" GEVAS add font path: %s\n", path);*/
	evas_font_add_path(EVAS(ev), (char*)path);
}

void gevas_remove_fontpath(GtkgEvas * ev, const gchar * path)
{
	evas_font_del_path(EVAS(ev), (char *)path);
}



/* internal functions */
void _show_evas_checked_bg(GtkWidget * widget, GtkgEvas * ev)
{

	if (!ev->show_checked_bg)
		return;

	if (GTK_WIDGET_REALIZED(widget)
		&& (!ev->checked_bg)) {
		int w, h;
		ev->checked_bg =
			evas_add_image_from_file(ev->evas, GEVAS_CHECKED_BG_IMAGE_FILENAME);
		if (ev->checked_bg) {
			evas_get_image_size(ev->evas, ev->checked_bg, &w, &h);
			evas_set_image_fill(ev->evas, ev->checked_bg, 0, 0, w, h);
			evas_move(ev->evas, ev->checked_bg, 0, 0);
			evas_resize(ev->evas, ev->checked_bg, 9999, 9999);
			evas_set_layer(ev->evas, ev->checked_bg, -999);
		}
	}
	if (ev->checked_bg) {
		evas_show(ev->evas, ev->checked_bg);
	}
}

/* FIXME: find a better iterator for glist. */
#define __HANDLE_EVENT_DISPATCH( func_to_call ) { \
		GSList* hans;  \
		int i=0,size=0; \
		hans = gevasobj_get_evhandlers( GTK_GEVASOBJ(_data) ); \
		size = g_slist_length( hans ); \
		for( i=0; i < size; i++ ) { \
			gpointer hdata = g_slist_nth_data( hans, i ); \
            GtkgEvasEvHClass* k = (GtkgEvasEvHClass*)GTK_OBJECT_GET_CLASS(hdata); \
            \
			if(GEVASEV_HANDLER_RET_CHOMP == k->func_to_call \
							(GTK_OBJECT(hdata), GTK_OBJECT(_data), _b, _x, _y )) \
			{ \
				break; \
			} \
		} \
	}

/* == ((GtkgEvasEvHClass*)(((GtkObject*) (hdata))->klass))->  */


void
__gevas_mouse_in(void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y)
{
	__HANDLE_EVENT_DISPATCH(handler_mouse_in)
}
void
__gevas_mouse_out(void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y)
{
	__HANDLE_EVENT_DISPATCH(handler_mouse_out)
}
void
__gevas_mouse_down(void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y)
{
	__HANDLE_EVENT_DISPATCH(handler_mouse_down)
}
void
__gevas_mouse_up(void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y)
{
	__HANDLE_EVENT_DISPATCH(handler_mouse_up)
}
void
__gevas_mouse_move(void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y)
{
	__HANDLE_EVENT_DISPATCH(handler_mouse_move)
}


#if 0
static void
gevas_drag_data_get (GtkWidget *widget,
					 GdkDragContext *context,
					 GtkSelectionData *selection_data,
					 guint info,
					 guint32 time)
{
/*	char  *image_file_name, *image_file_uri;*/
/*	gboolean is_reset;*/
    GtkgEvas* gevas = GTK_GEVAS(widget);

	
	g_return_if_fail (widget != NULL);
	g_return_if_fail (context != NULL);


/*    printf("gevas_drag_data_get() \n");*/
    
    
/*    
	switch (info) {
	case PROPERTY_TYPE:
		/* formulate the drag data based on the drag type.  Eventually, we will*/
		/*   probably select the behavior from properties in the category xml definition,*/
		/*   but for now we hardwire it to the drag_type */
		
		is_reset = FALSE;
		if (!strcmp(property_browser->details->drag_type, "property/keyword")) {
			char* keyword_str = strip_extension(property_browser->details->dragged_file);
		        gtk_selection_data_set(selection_data, selection_data->target, 8, keyword_str, strlen(keyword_str));
			g_free(keyword_str);
			return;	
		}
		else if (!strcmp(property_browser->details->drag_type, "application/x-color")) {
		        GdkColor color;
			guint16 colorArray[4];
			
			/* handle the "reset" case as an image */
			if (nautilus_strcmp (property_browser->details->dragged_file, RESET_IMAGE_NAME) != 0) {
				gdk_color_parse(property_browser->details->dragged_file, &color);
				colorArray[0] = color.red;
				colorArray[1] = color.green;
				colorArray[2] = color.blue;
				colorArray[3] = 0xffff;
				
				gtk_selection_data_set(selection_data,
				selection_data->target, 16, (const char *) &colorArray[0], 8);
				return;	
			} else {
				is_reset = TRUE;
			}

		}
		
		image_file_name = g_strdup_printf ("%s/%s/%s",
						   NAUTILUS_DATADIR,
						   is_reset ? "patterns" : property_browser->details->category,
						   property_browser->details->dragged_file);
		
		if (!g_file_exists (image_file_name)) {
			char *user_directory;
			g_free (image_file_name);

			user_directory = nautilus_get_user_directory ();
			image_file_name = g_strdup_printf ("%s/%s/%s",
							   user_directory,
							   property_browser->details->category, 
							   property_browser->details->dragged_file);	

			g_free (user_directory);
		}

		image_file_uri = gnome_vfs_get_uri_from_local_path (image_file_name);
		gtk_selection_data_set (selection_data, selection_data->target, 8, image_file_uri, strlen (image_file_uri));
		g_free (image_file_name);
		g_free (image_file_uri);
		
		break;
	default:
		g_assert_not_reached ();
	}
*/
    
}
#endif



static GtkWidgetClass *parent_class = NULL;

guint gevas_get_type(void)
{
	static guint ev_type = 0;

	if (!ev_type) {
		static const GtkTypeInfo ev_info = {
			"GtkgEvas",
			sizeof(GtkgEvas),
			sizeof(GtkgEvasClass),
			(GtkClassInitFunc) gevas_class_init,
			(GtkObjectInitFunc) gevas_init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL,
		};

		ev_type = gtk_type_unique(gtk_widget_get_type(), &ev_info);
	}

	return ev_type;
}

static void gevas_finalize(GObject* object);



static void gevas_class_init(GtkgEvasClass * klass)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;
    GObjectClass   *go;

    printf("gevas_class_init()\n");

    
    go           = (GObjectClass   *) klass;
    object_class = (GtkObjectClass *) klass;
	widget_class = (GtkWidgetClass *) klass;

	parent_class = gtk_type_class(gtk_widget_get_type());

	object_class->destroy = gevas_destroy;
	go->finalize = gevas_finalize;

	widget_class->realize = gevas_realize;
	widget_class->unrealize = gevas_unrealize;

	widget_class->size_request = gevas_size_request;

	widget_class->size_allocate = gevas_size_allocate;

    // FIXME: check it draws ok still...
//	widget_class->draw = gevas_draw;
//	widget_class->draw_focus = gevas_draw_focus;

	widget_class->map = gevas_map;

	widget_class->event = gevas_event;


	widget_class->expose_event = gevas_expose;

	widget_class->focus_in_event = gevas_focus_in;
	widget_class->focus_out_event = gevas_focus_out;
/*    widget_class->drag_data_get  = gevas_drag_data_get;*/
    
	object_class->get_arg = gevas_get_arg;
	object_class->set_arg = gevas_set_arg;

	klass->get_object_under_mouse = gevas_get_object_under_mouse;
	klass->object_in_rect = gevas_object_in_rect;
	klass->object_at_position = gevas_object_at_position;
	klass->object_get_named = gevas_object_get_named;

    printf("gevas_class_init() 3\n");

	gtk_object_add_arg_type(GTK_GEVAS_CHECKED_BG,
							GTK_TYPE_BOOL, GTK_ARG_READWRITE, ARG_CHECKED_BG);

	gtk_object_add_arg_type(GTK_GEVAS_RENDER_MODE,
							GTK_TYPE_INT, GTK_ARG_READWRITE, ARG_RENDER_MODE);

	gtk_object_add_arg_type(GTK_GEVAS_EVAS,
							GTK_TYPE_POINTER, GTK_ARG_READABLE, ARG_EVAS);

	gtk_object_add_arg_type(GTK_GEVAS_FONT_CACHE,
							GTK_TYPE_INT, GTK_ARG_READWRITE, ARG_FONT_CACHE);

	gtk_object_add_arg_type(GTK_GEVAS_SIZE_REQUEST_X,
							GTK_TYPE_INT,
							GTK_ARG_READWRITE, ARG_SIZE_REQUEST_X);

	gtk_object_add_arg_type(GTK_GEVAS_SIZE_REQUEST_Y,
							GTK_TYPE_INT,
							GTK_ARG_READWRITE, ARG_SIZE_REQUEST_Y);

	gtk_object_add_arg_type(GTK_GEVAS_DRAWABLE_SIZE_W,
							GTK_TYPE_INT,
							GTK_ARG_READABLE, ARG_DRAWABLE_SIZE_W);

	gtk_object_add_arg_type(GTK_GEVAS_DRAWABLE_SIZE_H,
							GTK_TYPE_INT,
							GTK_ARG_READABLE, ARG_DRAWABLE_SIZE_H);

	gtk_object_add_arg_type(GTK_GEVAS_SMOOTHNESS,
							GTK_TYPE_INT, GTK_ARG_WRITABLE, ARG_SMOOTHNESS);

	gtk_object_add_arg_type(GTK_GEVAS_VIEWPORT_X,
							GTK_TYPE_DOUBLE, GTK_ARG_READWRITE, ARG_VIEWPORT_X);
	gtk_object_add_arg_type(GTK_GEVAS_VIEWPORT_Y,
							GTK_TYPE_DOUBLE, GTK_ARG_READWRITE, ARG_VIEWPORT_Y);
	gtk_object_add_arg_type(GTK_GEVAS_VIEWPORT_W,
							GTK_TYPE_DOUBLE, GTK_ARG_READABLE, ARG_VIEWPORT_W);
	gtk_object_add_arg_type(GTK_GEVAS_VIEWPORT_H,
							GTK_TYPE_DOUBLE, GTK_ARG_READABLE, ARG_VIEWPORT_H);


    printf("gevas_class_init() end\n");
}

static void gevas_init(GtkgEvas * ev)
{
    printf("gevas_init()\n");
    
	GTK_WIDGET_SET_FLAGS(GTK_WIDGET(ev), GTK_CAN_FOCUS);

	ev->show_checked_bg = 0;
	ev->evas = NULL;
	ev->checked_bg = NULL;
	ev->size_request_x = 100;
	ev->size_request_y = 100;
	ev->render_method = RENDER_METHOD_ALPHA_SOFTWARE;
	ev->middleb_scrolls = 0;
	ev->middleb_scrolls_pgate_event = 0;
	ev->middleb_scrolls_yplane = 0;
	ev->middleb_scrolls_xplane = 0;
	ev->gevasobjs = g_hash_table_new(NULL, NULL);
	ev->gevasobjlist = NULL;
    printf("gevas_init() end\n");
}

GtkWidget *gevas_new(void)
{
	GtkgEvas *ev;

    ev = gtk_type_new(gevas_get_type());

	gevas_set_render_mode(ev, RENDER_METHOD_ALPHA_SOFTWARE);
	gevas_set_size_request_x(ev, 200);
	gevas_set_size_request_y(ev, 200);

    ev->scrolledwindow = 0;

    return GTK_WIDGET(ev);
}

/* GtkObject functions */
static void gevas_destroy(GtkObject * object)
{
	GtkgEvas *ev;

/*	printf("gevas_destroy() 1\n"); */

	g_return_if_fail(object != NULL);
	g_return_if_fail(GTK_IS_GEVAS(object));
	ev = GTK_GEVAS(object);



	/* Chain up */
	if (GTK_OBJECT_GET_CLASS(parent_class)->destroy)
		(*GTK_OBJECT_GET_CLASS(parent_class)->destroy) (object);
}

static void gevas_finalize(GObject* object)
{
	GtkgEvas *ev;
	g_return_if_fail(object != NULL);
	g_return_if_fail(GTK_IS_GEVAS(object));
	ev = GTK_GEVAS(object);

/*	printf("gevas_finalize!\n"); */

	/* FIXME: better list delete... glib :-( */
	while (1) {
		GSList *pitem;
		gint ref_count;

		pitem = g_slist_nth(ev->gevasobjlist, 0);
		if (!pitem)
			break;
		ref_count = G_OBJECT(pitem->data)->ref_count;
/*		printf("gevas_finalize wave... before unref... ref_count:%d\n",ref_count); */
		gtk_object_destroy(GTK_OBJECT(pitem->data));
		gtk_object_unref(GTK_OBJECT(pitem->data));
		ev->gevasobjlist = g_slist_remove(ev->gevasobjlist, pitem->data);
	}

	if (ev->gevasobjlist) {
		g_slist_free(ev->gevasobjlist);
		ev->gevasobjlist = NULL;
	}

	if (ev->gevasobjs) {
		g_hash_table_destroy(ev->gevasobjs);
		ev->gevasobjs = NULL;
	}

	if (ev->checked_bg) {
		evas_del_object(ev->evas, ev->checked_bg);
		ev->checked_bg = NULL;
	}

	if (ev->evas) {
/*		printf("FREE() destroy evas\n");*/
		evas_free(ev->evas);
		ev->evas = NULL;
	}

	/* Chain up */
	if (G_OBJECT_CLASS(parent_class)->finalize)
		(*G_OBJECT_CLASS(parent_class)->finalize) (object);

}


void gevas_adjust_scrollarea(GtkgEvas * ev, GtkAdjustment * plane, gint * delta)
{
	if (delta) {
		gfloat value = plane->value;
		gfloat lower = plane->lower;
		gfloat upper = plane->upper;
		gfloat page = plane->page_size;

		if (value - (*delta) < lower) {
			(*delta) = value - lower;
		}
		if (value - (*delta) + page > upper) {
			(*delta) = upper - value - page;
		}

		gtk_adjustment_set_value(plane, value - (*delta));
		gtk_adjustment_value_changed(plane);
	}
}


/* GtkWidget functions */

static gint gevas_event(GtkWidget * widget, GdkEvent * event)
{
	GtkgEvas *ev;
    
    g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(GTK_IS_GEVAS(widget), FALSE);

	ev = GTK_GEVAS(widget);

/*      printf("gevas_event() ev:%p type:%d\n",ev,event->type);  */
    
    
	ev->current_event = event;
	switch (event->type) {

    case GDK_LEAVE_NOTIFY:
    {
        GdkEventCrossing* e = (GdkEventCrossing*)event;

/*        printf("GDK_LEAVE_NOTIFY\n");*/
        
        break;
    }
    
		case GDK_MOTION_NOTIFY:
			{
				gint x = 0, y = 0;

/*				x=(int)event->motion.x;
				y=(int)event->motion.y; */
				/* If it was a motion event, make sure we get more */
				gdk_window_get_pointer(widget->window, &x, &y, NULL);

				if (ev->scrolling) {
					int dx, dy;
					int max_middleb_pixels = GEVAS_MAX_MIDDLEB_SCROLL_PIXELS;

					int middleb_scale_x =
						(ev->middleb_scrolls_xplane->upper -
						 ev->middleb_scrolls_xplane->lower) / 1000;
					int middleb_scale_y =
						(ev->middleb_scrolls_yplane->upper -
						 ev->middleb_scrolls_yplane->lower) / 1000;

					dx = ev->scrolling_x - x;
					dy = ev->scrolling_y - y;
					dx *= middleb_scale_x;
					dy *= middleb_scale_y;

					if (dx > max_middleb_pixels)
						dx = max_middleb_pixels;
					if (dx < -(max_middleb_pixels))
						dx = -(max_middleb_pixels);
					if (dy > max_middleb_pixels)
						dy = max_middleb_pixels;
					if (dy < -(max_middleb_pixels))
						dy = -(max_middleb_pixels);

					if (dy) {
						gevas_adjust_scrollarea(ev,
												ev->middleb_scrolls_yplane,
												&dy);
						ev->scrolling_y = y - dy;
					}
					if (dx) {
						gevas_adjust_scrollarea(ev,
												ev->middleb_scrolls_xplane,
												&dx);
						ev->scrolling_x = x - dx;
					}

				} else {
					evas_event_move(ev->evas, x, y);


/*                    printf("evas_event_move() x:%d y:%d \n",x,y);*/
                    
                    /*
                    {
                        static dr = 0;
                        GtkTargetList *target_list;	
                        GdkDragContext *context;
                        enum {
                            TARGET_URI_LIST,
                            TARGET_TEXT_PLAIN
                        };
                        static GtkTargetEntry file_d_types[] = {
                            { "text/uri-list", 0, TARGET_URI_LIST }
                        };

                        GtkAdjustment* a = 0;
                        a = gtk_scrolled_window_get_hadjustment( ev->scrolledwindow );

                        printf("moving. X:%d a->x:%d a->w:%d\n",
                               x, a->value, a->page_size);
                        
                        if( x > 700 )
                        {
                            printf("starting a drag!\n");
                            dr=1;
                            target_list = gtk_target_list_new (file_d_types, 1);
                            context = gtk_drag_begin (GTK_WIDGET (ev),
                                                      target_list,
                                                      GDK_ACTION_MOVE | GDK_ACTION_COPY,
                                                      1,
                                                      (GdkEvent *) event);
                        }
                    }
                    */
				}

			}
			break;


		case GDK_BUTTON_PRESS:
		case GDK_2BUTTON_PRESS:
		case GDK_3BUTTON_PRESS:
			{
				int x = 0, y = 0, b = 0;

				x = (int) event->button.x;
				y = (int) event->button.y;
				b = (int) event->button.button;


/*				gdk_pointer_grab( widget->window, 1, 0,0,0, GDK_CURRENT_TIME);
 */


				if (ev->middleb_scrolls && b == 2) {
					ev->scrolling = 1;
					ev->scrolling_x = x;
					ev->scrolling_y = y;

					if( ev->middleb_scrolls_pgate_event )
					{
						evas_event_button_down(ev->evas, x, y, b);
					}
				}
				else 
				{
					evas_event_button_down(ev->evas, x, y, b);
				}
			}
			break;
            
		case GDK_BUTTON_RELEASE:
			{
				int x = 0, y = 0, b = 0;

				x = (int) event->button.x;
				y = (int) event->button.y;
				b = (int) event->button.button;

/*                printf("GDK_BUTTON_RELEASE: x:%d y:%d\n",x,y); */
                
                
/*				gdk_pointer_ungrab( GDK_CURRENT_TIME );
 */

				if (ev->middleb_scrolls && b == 2) {
					ev->scrolling = 0;
					if( ev->middleb_scrolls_pgate_event )
					{
						evas_event_button_up(ev->evas, x, y, b);
					}
				}
				else 
				{
					evas_event_button_up(ev->evas, x, y, b);
				}
			break;
			}
            
            
		default:
			break;
	}
	ev->current_event = 0;

    
	if (event->type == GDK_BUTTON_RELEASE
        && (!event->button.x)
        && (!event->button.y)
        )
    {
/*        printf("drag synthetic event being ignored...\n"); */
        return FALSE;
    }
    

    
	if (event->any.window == widget->window) {
		if (GTK_WIDGET_CLASS(parent_class)->event)
        {
            return (*GTK_WIDGET_CLASS(parent_class)->event) (widget, event);
        }
		else
        {
            return FALSE;
        }
	}
    else
    {
	}

    return FALSE;
}

static void gevas_map(GtkWidget * widget)
{
	GtkgEvas *ev;

	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_GEVAS(widget));
	ev = GTK_GEVAS(widget);

	if (GTK_WIDGET_CLASS(parent_class)->map)
		(*GTK_WIDGET_CLASS(parent_class)->map) (widget);

	/* Keep origin, make a given with and height shown */
	evas_set_output_size(ev->evas, widget->allocation.width,
						 widget->allocation.height);
	evas_set_output_viewport(ev->evas, 0, 0, widget->allocation.width,
							 widget->allocation.height);
}


static void gevas_realize(GtkWidget * widget)
{
	GdkWindowAttr attributes;
	gint attributes_mask;
	GtkgEvas *ev;
/*  GdkCursor* cursor; */
	Visual *vis;
	Colormap cmap;
	GdkVisual *gdk_vis;
	GdkColormap *gdk_cmap;

    printf("gevas_realize()\n");
    
    
	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_GEVAS(widget));

	ev = GTK_GEVAS(widget);

	/* Set realized flag */
	GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);

     
	/* Evas window */

	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = widget->allocation.x;
	attributes.y = widget->allocation.y;
	attributes.width = widget->allocation.width;
	attributes.height = widget->allocation.height;
	attributes.wclass = GDK_INPUT_OUTPUT;

	ev->evas = evas_new();

/*  evas_set_image_cache(ev->evas, 8 * 1024 * 1024); */
	evas_set_output_method(ev->evas, ev->render_method);

	vis =
		evas_get_optimal_visual(ev->evas,
								GDK_WINDOW_XDISPLAY(GDK_ROOT_PARENT()));
	cmap =
		evas_get_optimal_colormap(ev->evas,
								  GDK_WINDOW_XDISPLAY(GDK_ROOT_PARENT()));
	gdk_vis = gdkx_visual_get(XVisualIDFromVisual(vis));
	gdk_cmap = gdkx_colormap_get(cmap);

///	((GdkColormapPrivate *) gdk_cmap)->visual = gdk_vis;  // This was not commented before.
/*  gtk_widget_push_visual(gdk_vis);*/
/*  gtk_widget_push_colormap(gdk_cmap);*/
 
	attributes.visual = gdk_vis;	/*gtk_widget_get_visual (widget); */
	attributes.colormap = gdk_cmap;	/*gtk_widget_get_colormap (widget); */
	attributes.event_mask = gtk_widget_get_events(widget) | GDK_EXPOSURE_MASK;
	attributes.event_mask |= GDK_EXPOSURE_MASK
        | GDK_BUTTON_PRESS_MASK
		| GDK_BUTTON_RELEASE_MASK
        | GDK_ENTER_NOTIFY_MASK
        | GDK_LEAVE_NOTIFY_MASK
        | GDK_POINTER_MOTION_MASK
        | GDK_POINTER_MOTION_HINT_MASK;
/*  attributes.event_mask = GDK_ALL_EVENTS_MASK;  */

	attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

	widget->window = gdk_window_new(gtk_widget_get_parent_window(widget),
									&attributes, attributes_mask);
	gdk_window_set_user_data(widget->window, widget);

  /** ** **/

	/* Style */

	widget->style = gtk_style_attach(widget->style, widget->window);
	gtk_style_set_background(widget->style, widget->window, GTK_STATE_NORMAL);

	gdk_window_set_back_pixmap(widget->window, NULL, FALSE);
	evas_set_output(ev->evas,
					GDK_WINDOW_XDISPLAY(widget->window),
					GDK_WINDOW_XWINDOW(widget->window),
					GDK_VISUAL_XVISUAL(gtk_widget_get_visual(widget)),
					GDK_COLORMAP_XCOLORMAP(gtk_widget_get_colormap(widget)));
	evas_set_output_size(ev->evas, widget->allocation.width,
						 widget->allocation.height);
	evas_set_output_viewport(ev->evas, 0, 0, widget->allocation.width,
							 widget->allocation.height);


	/* only shows if the user has requested it */
	_show_evas_checked_bg(widget, ev);

    // FIXME:
    gtk_widget_set_double_buffered( widget, 0);
    

    
    printf("gevas_realize() end\n");
}

static void gevas_unrealize(GtkWidget * widget)
{
	GtkgEvas *ev;

	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_GEVAS(widget));

	ev = GTK_GEVAS(widget);

	/* Hide all windows */

	if (GTK_WIDGET_MAPPED(widget))
		gtk_widget_unmap(widget);

	GTK_WIDGET_UNSET_FLAGS(widget, GTK_MAPPED);


	/* This destroys widget->window and unsets the realized flag
	 */
	if (GTK_WIDGET_CLASS(parent_class)->unrealize)
		(*GTK_WIDGET_CLASS(parent_class)->unrealize) (widget);
}

static void gevas_size_request(GtkWidget * widget, GtkRequisition * requisition)
{
	GtkgEvas *ev;

	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_GEVAS(widget));
	ev = GTK_GEVAS(widget);

	/* 
	 * GtkgEvas always wants to be the same fixed size.
	 */

	requisition->width = ev->size_request_x;
	requisition->height = ev->size_request_y;



}

static void gevas_size_allocate(GtkWidget * widget, GtkAllocation * allocation)
{
	GtkgEvas *ev;

	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_GEVAS(widget));


	ev = GTK_GEVAS(widget);


	widget->allocation = *allocation;

	if (GTK_WIDGET_REALIZED(widget)) {
		double x, y, w, h;

		/* Keep origin, make a given with and height shown */
		evas_get_viewport(ev->evas, &x, &y, &w, &h);
		evas_set_output_size(ev->evas, widget->allocation.width,
							 widget->allocation.height);
		evas_set_output_viewport(ev->evas, x, y, widget->allocation.width,
								 widget->allocation.height);

		gdk_window_move_resize(widget->window,
							   allocation->x,
							   allocation->y,
							   allocation->width, allocation->height);
	}
}

static void gevas_draw(GtkWidget * widget, GdkRectangle * area)
{
	/* GdkRectangle event_window_area;
	 * GdkRectangle intersection;
	 */
	GtkgEvas *ev;

	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_GEVAS(widget));

	ev = GTK_GEVAS(widget);

/*
  printf("gevas_draw() area: x:%d y:%d w:%d h:%d\n", area->x, area->y,
           area->width, area->height );
*/
    
    
	gevas_paint(ev, area);
}

static void gevas_draw_focus(GtkWidget * widget)
{
	GdkRectangle rect;
	GtkgEvas *ev;

	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_GEVAS(widget));

	ev = GTK_GEVAS(widget);

	rect.x = 0;
	rect.y = 0;
	rect.width = widget->allocation.width;
	rect.height = widget->allocation.height;

	if (GTK_WIDGET_DRAWABLE(ev))
		gevas_paint(ev, &rect);
}

static gint gevas_expose(GtkWidget * widget, GdkEventExpose * event)
{
	GtkgEvas *ev;

	g_return_val_if_fail(widget != NULL, 0);
	g_return_val_if_fail(GTK_IS_GEVAS(widget), 0);

	ev = GTK_GEVAS(widget);

    if (event->window == widget->window)
    {
        gevas_paint(GTK_GEVAS(widget), &event->area);
	} else
		g_assert_not_reached();

	return TRUE;
}

static gint gevas_focus_in(GtkWidget * widget, GdkEventFocus * event)
{
	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(GTK_IS_GEVAS(widget), FALSE);

	GTK_WIDGET_SET_FLAGS(widget, GTK_HAS_FOCUS);
//	gtk_widget_draw_focus(widget);

	return FALSE;
}

static gint gevas_focus_out(GtkWidget * widget, GdkEventFocus * event)
{
	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(GTK_IS_GEVAS(widget), FALSE);

	GTK_WIDGET_UNSET_FLAGS(widget, GTK_HAS_FOCUS);
//	gtk_widget_draw_focus(widget);

	return FALSE;
}

/* GtkgEvas-specific functions */

static void gevas_paint(GtkgEvas * ev, GdkRectangle * area)
{
	GtkWidget *widget;

	g_return_if_fail(ev != NULL);
	g_return_if_fail(GTK_IS_GEVAS(ev));

    printf("gevas_paint()\n");

    widget = GTK_WIDGET(ev);

	if (!GTK_WIDGET_DRAWABLE(widget))
		return;

  
/* XXX: doesn't work without this line.*/
	evas_update_rect(ev->evas, area->x, area->y, area->width, area->height);

    
/*	printf("evas_update_rect() x:%d y:%d w:%d h:%d\n",*/
/*		area->x, area->y, area->width, area->height);*/

	ev->evas_r = *area;

    /**/
    /* Now we carve up the area outside of the current expose event so*/
    /* that evas doesn't waste time rendering stuff that already is drawn ok.*/
    /**/
    evas_clear_obscured_rects(ev->evas);
    {
        int x = ev->evas_r.x;
        int y = ev->evas_r.y;
        int w = ev->evas_r.width;
        int h = ev->evas_r.height;
        int aw = widget->allocation.width;
        int ah = widget->allocation.height;

/*        printf("paint() x:%d y:%d w:%d h:%d aw:%d ah:%d\n",*/
/*               x,y,w,h,aw,ah);*/

        /* Left and right blocks*/
        evas_add_obscured_rect(ev->evas, 0, 0, x, ah );
        evas_add_obscured_rect(ev->evas, x+w, 0, aw - x+w, ah );
        
        /* Top and bottom blocks*/
        evas_add_obscured_rect(ev->evas, x, 0, x+w, y );
        evas_add_obscured_rect(ev->evas, x, y+h, x+w, ah );
    }
  



    
    evas_render(ev->evas);


    if (GTK_WIDGET_HAS_FOCUS(widget)) {
		gtk_paint_focus(widget->style, widget->window,
						area, widget, "ev",
						widget->allocation.x, widget->allocation.y,
						widget->allocation.width - 1,
						widget->allocation.height - 1);
	}

    
}


void gevas_get_viewport_area( GtkgEvas* gevas, gint* x, gint* y, gint* w, gint* h )
{
    GtkAdjustment* a = 0;
    GtkScrolledWindow* swin = GTK_SCROLLED_WINDOW(gevas->scrolledwindow);
    
    if( gevas->scrolledwindow )
    {
        if( a = gtk_scrolled_window_get_hadjustment( swin ))
        {
            *x = a->value;
            *w = a->page_size;
        }
        
        if( a = gtk_scrolled_window_get_vadjustment( swin ))
        {
            *y = a->value;
            *h = a->page_size;
        }
    }
    
    
}


gint gevas_view_redraw_cb(gpointer data)
{
	GtkgEvas* 	gevas = (GtkgEvas *) data;
	GtkgEvas* 	ev = gevas;
	GdkRectangle* 	area = &ev->evas_r;
    GdkRectangle rect;

    evas_clear_obscured_rects(ev->evas);
    evas_render(ev->evas);

    printf("gevas_view_redraw_cb!\n");
    
    
#if 0
  /* Although the below code *seems* faster (due to calling paint, and thus having*/
  /* obscures working to block parts of the evas) the dumber code above seems faster.*/

/*    printf("gevas_view_redraw_cb %p %p\n", ev->scrolledwindow, ev->scrolledwindow_viewport);*/
    {
        GtkAdjustment* a = 0;
    
        a = gtk_scrolled_window_get_hadjustment( ev->scrolledwindow );
        
/*        printf("scrolledwindow_viewport2... low:%f hi:%f val:%f pagesize:%f\n",*/
/*               a->lower, a->upper, a->value, a->page_size );*/

        rect.x     = a->value;
        rect.width = a->page_size;

        a = gtk_scrolled_window_get_vadjustment( ev->scrolledwindow );
        rect.y      = a->value;
        rect.height = a->page_size;
    }
    gevas_paint( ev, &rect );
#endif
    
	gevas->current_idle = 0;
	return FALSE;
}



void gevas_queue_redraw(GtkgEvas * gevas)
{
    /* This call seems to be much slower to use.*/
    /*gtk_widget_queue_draw( gevas );*/

	if (gevas->current_idle)
		gtk_idle_remove(gevas->current_idle);

	gevas->current_idle = gtk_idle_add(gevas_view_redraw_cb, gevas);

}



Evas gevas_get_evas(GtkgEvas * gevas)
{
	return gevas->evas;
}

GdkEvent *gevas_get_current_event(GtkgEvas * gevas)
{
	return gevas->current_event;
}

void 
gevas_set_middleb_scrolls_pgate_event( GtkgEvas* gevas, gboolean v )
{
	gevas->middleb_scrolls_pgate_event = v;
}

void
gevas_set_middleb_scrolls(GtkgEvas * gevas,
						  gboolean v, GtkAdjustment * ah, GtkAdjustment * av)
{
	if (v) {
		gevas->middleb_scrolls = 1;
		gevas->middleb_scrolls_yplane = av;
		gevas->middleb_scrolls_xplane = ah;
	} else {
		gevas->middleb_scrolls = 0;
		gevas->middleb_scrolls_yplane = 0;
		gevas->middleb_scrolls_xplane = 0;
	}
}


typedef gboolean (*gevas_GtkSignal_BOOL__POINTER_POINTER_INT_INT_INT) (GtkObject *
							     object,
							     gpointer arg1,
							     gpointer arg2,
							     gint arg3,
							     gint arg4,
							     gint arg5,
							     gpointer
							     user_data);
void
gevas_gtk_marshal_BOOL__POINTER_POINTER_INT_INT_INT (GtkObject * object,
				GtkSignalFunc func, gpointer func_data, GtkArg * args)
{
  gevas_GtkSignal_BOOL__POINTER_POINTER_INT_INT_INT rfunc;
  gboolean *return_val;
  return_val = GTK_RETLOC_BOOL (args[5]);
  rfunc = (gevas_GtkSignal_BOOL__POINTER_POINTER_INT_INT_INT) func;
  *return_val = (*rfunc) (object,
			  GTK_VALUE_POINTER (args[0]),
			  GTK_VALUE_POINTER (args[1]),
			  GTK_VALUE_INT (args[2]),
			  GTK_VALUE_INT (args[3]), 
			  GTK_VALUE_INT (args[4]), func_data);
}


void gevas_get_drawable_size( GtkgEvas *object, int* w, int *h ) {
  GtkgEvas* ev;

  g_return_if_fail(object != NULL);
  g_return_if_fail(GTK_IS_GEVAS(object));

  ev = GTK_GEVAS(object);
  evas_get_drawable_size( ev->evas, w, h );
}



void gevas_new_gtkscrolledwindow(GtkgEvas** gevas , GtkWidget** scrolledwindow )
{
    GtkBin *bin;

	*scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
	*gevas = GTK_GEVAS(gevas_new()); // errs on this line

	gtk_scrolled_window_add_with_viewport(
        GTK_SCROLLED_WINDOW(*scrolledwindow), GTK_WIDGET(*gevas));

    
    bin = GTK_BIN (*scrolledwindow);
    if (bin->child != NULL)
    {
      g_return_if_fail (GTK_IS_VIEWPORT (bin->child));
      (*gevas)->scrolledwindow_viewport = GTK_VIEWPORT(bin->child);
    }
    
    (*gevas)->scrolledwindow = *scrolledwindow;


/*
  printf("gevas_new_gtkscrolledwindow() %p %p\n",
           (*gevas)->scrolledwindow,
           (*gevas)->scrolledwindow_viewport);
*/
    
}





static void gevas_set_arg(GtkObject * object, GtkArg * arg, guint arg_id)
{
	GtkgEvas *ev;

	g_return_if_fail(object != NULL);
	g_return_if_fail(GTK_IS_GEVAS(object));

	ev = GTK_GEVAS(object);


	switch (arg_id) {
		case ARG_CHECKED_BG:
			if (GTK_VALUE_BOOL(*arg) != ev->show_checked_bg) {
				gboolean oldv = ev->show_checked_bg;
				ev->show_checked_bg = GTK_VALUE_BOOL(*arg);

				if (oldv) {
					evas_hide(ev->evas, ev->checked_bg);
				} else {
					_show_evas_checked_bg(GTK_WIDGET(object), ev);
				}
			}
			break;

		case ARG_RENDER_MODE:
			ev->render_method = GTK_VALUE_INT(*arg);
			evas_set_output_method(ev->evas, ev->render_method);
			break;

		case ARG_FONT_CACHE:
			evas_set_font_cache(ev->evas, GTK_VALUE_INT(*arg));
			break;

		case ARG_IMAGE_CACHE:
			evas_set_image_cache(ev->evas, GTK_VALUE_INT(*arg));
			break;

		case ARG_SIZE_REQUEST_X:
			ev->size_request_x = GTK_VALUE_INT(*arg);
			break;

		case ARG_SIZE_REQUEST_Y:
			ev->size_request_y = GTK_VALUE_INT(*arg);
			break;

		case ARG_SMOOTHNESS:
			evas_set_scale_smoothness(ev->evas, GTK_VALUE_INT(*arg));
			break;
		case ARG_VIEWPORT_X:
			{
				double x = 0, y = 0, w = 0, h = 0;
				evas_get_viewport(ev->evas, &x, &y, &w, &h);
				x = GTK_VALUE_DOUBLE(*arg);
				evas_set_output_viewport(ev->evas, x, y, w, h);
			}
			break;
		case ARG_VIEWPORT_Y:
			{
				double x = 0, y = 0, w = 0, h = 0;
				evas_get_viewport(ev->evas, &x, &y, &w, &h);
				y = GTK_VALUE_DOUBLE(*arg);
				evas_set_output_viewport(ev->evas, x, y, w, h);
			}
			break;

		default:
			break;
	}
}

static void gevas_get_arg(GtkObject * object, GtkArg * arg, guint arg_id)
{
	GtkgEvas *ev;

	g_return_if_fail(object != NULL);
	g_return_if_fail(GTK_IS_GEVAS(object));

	ev = GTK_GEVAS(object);

	switch (arg_id) {
		case ARG_CHECKED_BG:
			GTK_VALUE_BOOL(*arg) = ev->show_checked_bg;
			break;
		case ARG_RENDER_MODE:
			GTK_VALUE_INT(*arg) = evas_get_render_method(ev->evas);
			break;
		case ARG_EVAS:
			GTK_VALUE_POINTER(*arg) = ev->evas;
			break;
		case ARG_FONT_CACHE:
			GTK_VALUE_INT(*arg) = evas_get_font_cache(ev->evas);
			break;
		case ARG_IMAGE_CACHE:
			GTK_VALUE_INT(*arg) = evas_get_image_cache(ev->evas);
			break;
		case ARG_SIZE_REQUEST_X:
			GTK_VALUE_INT(*arg) = ev->size_request_x;
			break;
		case ARG_SIZE_REQUEST_Y:
			GTK_VALUE_INT(*arg) = ev->size_request_y;
			break;
		case ARG_DRAWABLE_SIZE_W:
			{
				int w = 0, h = 0;
				evas_get_drawable_size(ev->evas, &w, &h);
				GTK_VALUE_INT(*arg) = w;
			}
			break;
		case ARG_DRAWABLE_SIZE_H:
			{
				int w = 0, h = 0;
				evas_get_drawable_size(ev->evas, &w, &h);
				GTK_VALUE_INT(*arg) = h;
			}
			break;
		case ARG_VIEWPORT_X:
			{
				double x = 0, y = 0, w = 0, h = 0;
				evas_get_viewport(ev->evas, &x, &y, &w, &h);
				GTK_VALUE_DOUBLE(*arg) = x;
			}
			break;
		case ARG_VIEWPORT_Y:
			{
				double x = 0, y = 0, w = 0, h = 0;
				evas_get_viewport(ev->evas, &x, &y, &w, &h);
				GTK_VALUE_DOUBLE(*arg) = y;
			}
			break;
		case ARG_VIEWPORT_W:
			{
				double x = 0, y = 0, w = 0, h = 0;
				evas_get_viewport(ev->evas, &x, &y, &w, &h);
				GTK_VALUE_DOUBLE(*arg) = w;
			}
			break;
		case ARG_VIEWPORT_H:
			{
				double x = 0, y = 0, w = 0, h = 0;
				evas_get_viewport(ev->evas, &x, &y, &w, &h);
				GTK_VALUE_DOUBLE(*arg) = h;
			}
			break;

		default:
			arg->type = GTK_TYPE_INVALID;
			break;
	}
}


static void
add_string_to_list( GtkgEvas * ev, const char* p, GList **li )
{
	g_return_if_fail(ev != NULL);
	g_return_if_fail(p  != NULL);
	g_return_if_fail(GTK_IS_GEVAS(ev));

    if(!strlen(p))
        return;

    *li = g_list_append(*li, g_strdup(p));
}

static void
remove_string_from_list( GtkgEvas * ev, const char* p, GList **li )
{
    GList *t = 0;
    
    g_return_if_fail(ev != NULL);
	g_return_if_fail(p  != NULL);
	g_return_if_fail(GTK_IS_GEVAS(ev));

    if(!strlen(p))
        return;

    if(t = g_list_find(*li, (gpointer)p))
    {
        g_free(t->data);
        *li = g_list_remove_link(*li, t);
    }
}



void gevas_add_metadata_prefix( GtkgEvas * ev, const char* p )
{
	g_return_if_fail(ev != NULL);
	g_return_if_fail(p  != NULL);
	g_return_if_fail(GTK_IS_GEVAS(ev));

    if(!strlen(p))
        return;

    add_string_to_list( ev, p, &ev->metadata_prefix_list );
}

void gevas_remove_metadata_prefix ( GtkgEvas * ev, const char* p )
{
    GList *li = 0;
    
    g_return_if_fail(ev != NULL);
	g_return_if_fail(p  != NULL);
	g_return_if_fail(GTK_IS_GEVAS(ev));

    if(!strlen(p))
        return;

    remove_string_from_list( ev, p, &ev->metadata_prefix_list );
    
}





void gevas_add_image_prefix( GtkgEvas * ev, const char* p )
{
    g_return_if_fail(ev != NULL);
	g_return_if_fail(p  != NULL);
	g_return_if_fail(GTK_IS_GEVAS(ev));
    if(!strlen(p))
        return;
    add_string_to_list( ev, p, &ev->image_prefix_list );
}

void gevas_remove_image_prefix ( GtkgEvas * ev, const char* p )
{
    g_return_if_fail(ev != NULL);
	g_return_if_fail(p  != NULL);
	g_return_if_fail(GTK_IS_GEVAS(ev));
    if(!strlen(p))
        return;
    remove_string_from_list( ev, p, &ev->image_prefix_list );
}



GList* gevas_get_metadata_prefix_list( GtkgEvas *ev )
{
    g_return_if_fail(ev != NULL);
	g_return_if_fail(GTK_IS_GEVAS(ev));
    return ev->metadata_prefix_list;
}


GList* gevas_get_image_prefix_list   ( GtkgEvas *ev )
{
    g_return_if_fail(ev != NULL);
	g_return_if_fail(GTK_IS_GEVAS(ev));
    return ev->image_prefix_list;
}


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/** These can be used to find an edb file using prefixs and optionally call ***/
/** a function when a valid edb file is found *********************************/
/******************************************************************************/


gboolean
gevas_file_exists(const char* fmt, ... )
{
  gchar *full_filename;
  struct stat s;
  gint status;
  va_list args;
  gchar* fn = NULL;

  va_start (args, fmt);
  fn = g_strdup_vprintf( fmt, args );
  va_end (args);
  
  status = stat (fn, &s);
  g_free(fn);
  if (status == 0 && S_ISREG (s.st_mode))
  {
      return 1;
  }
  
  return 0;
}




enum _gevas_metadata_type
{
    MD_STRING=1,
    MD_INT,
    MD_DATA,
    MD_DUMMY,
};
typedef enum _gevas_metadata_type gevas_metadata_type;




static void
metadata_find_edb_cb( 
    gpointer data,
    gpointer user_data
    )
{
    const char* fully_qualified_prefix = data;
    gevas_metadata_find_edb_data* d = (gevas_metadata_find_edb_data*)user_data;
    GtkgEvas* ev      = 0;
    E_DB_File* edb    = 0;
    char* k           = 0;
    char* filen       = 0;
    char* full_buffer = 0;
    char* edb_prefix  = 0;
    char* strbuf1     = 0;
    GHashTable* hash_args = 0;
    gboolean has_imlib2_colon_in_name = 0;
    gboolean ok       = 1;

    /* Assert and init */
    g_return_if_fail(d != NULL);
	g_return_if_fail(fully_qualified_prefix!= NULL);
    g_return_if_fail(GTK_IS_GEVAS(d->gevas));
    if( d->loaded )
    {
        return;
    }
    ev = d->gevas;

    /* create the filename buffer */
    if( strlen( fully_qualified_prefix ))
    {
        full_buffer = g_strconcat( fully_qualified_prefix,
                                   "/", d->edb_postfix, 0 );
    }
    else
    {
        full_buffer = g_strdup(d->edb_postfix);
    }

    
    /* get filename and extra args */
    printf("full_buffer:%s\n",full_buffer );
    filen = strbuf1 = url_file_name_part_new( full_buffer );
    d->hash_args    = url_args_to_hash( full_buffer );
    g_free(full_buffer);

    printf("metadata_find_edb_cb() checking for file exists:%s\n",filen);

    if( strstr( filen, ":" ))
    {
        struct stat s;
        char* p = strstr( filen, ":" );

        has_imlib2_colon_in_name = 1;
        *p = '\0';
        if(!stat (filen, &s) && S_ISREG (s.st_mode))
        {
            *p = ':';
        }
        else
        {
            ok = 0;
        }
    }
    

    /* If we have a winner, call the callback and get out of here */
    if(ok && (has_imlib2_colon_in_name || gevas_file_exists( filen )))
    {
        printf("metadata_find_edb_cb() file exists!! :%s\n",filen);

        d->edb_full_path = g_strdup(filen);

        if( d->edb_found_f )
        {
            printf("metadata_find_edb_cb() file exists calling f\n");
            d->edb_found_f( d );
        }
        

        d->loaded=1;
    }
    
    hash_str_str_clean( d->hash_args );
    g_free(strbuf1);
}


gchar*
gevas_metadata_find_edb_with_data( GtkgEvas *ev, gevas_metadata_find_edb_data* data)
{
    char* no_prefix = "";

    g_return_if_fail(ev   != NULL);
    g_return_if_fail(data != NULL);
	g_return_if_fail(GTK_IS_GEVAS(ev));

    data->loaded        = 0;
    data->gevas         = ev;
    data->edb_full_path = 0;
    data->hash_args     = 0;
    
    if( gevas_get_metadata_prefix_list(ev))
    {
        g_list_foreach( gevas_get_metadata_prefix_list(ev),
                        metadata_find_edb_cb, data);
    }
    metadata_find_edb_cb((gpointer)no_prefix, data);

    return data->loaded ? data->edb_full_path : g_strdup(data->edb_postfix);
}

gchar*
gevas_metadata_find_edb( GtkgEvas *ev, const gchar* loc )
{
    char* no_prefix = "";
    gevas_metadata_find_edb_data data;

    g_return_if_fail(ev  != NULL);
    g_return_if_fail(loc != NULL);
	g_return_if_fail(GTK_IS_GEVAS(ev));

    memset( &data, 0, sizeof(gevas_metadata_find_edb_data));
    data.edb_postfix   = loc;
    return gevas_metadata_find_edb_with_data( ev, &data );
}




/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/** These can be used to lookup a key in a metadata chain *********************/
/******************************************************************************/



/*
 * Note that we can pass extra args as long as gevas_metadata_find_edb_data
 * is the first static arg in the struct, then the edb finding functions
 * will work ok, and we get to pass in our own data.
 */
typedef struct _metadata_lookup_x_data metadata_lookup_x_data;
struct _metadata_lookup_x_data 
{
    gevas_metadata_find_edb_data d;

    gpointer    def;
    const char* key;
    gboolean    loaded;
    gpointer    loaded_data;
    
    gevas_metadata_type md_type;

};



void
gevas_metadata_lookup_x(gevas_metadata_find_edb_data* d)
{
    metadata_lookup_x_data* data = (metadata_lookup_x_data*)d;
    E_DB_File* edb    = 0;

    printf("gevas_metadata_lookup_x() %d\n",data->loaded);

    g_return_if_fail(d != NULL);
    if(data->loaded)
        return;
    

    printf("gevas_metadata_lookup_x() key:%s edb_full_path:%s\n", data->key,
           d->edb_full_path);
    
    
    /* load the data */
    if( edb = e_db_open( d->edb_full_path ))
    {
        switch(data->md_type)
        {
        case MD_STRING:
            data->loaded_data = edb_lookup_str( edb, (char*)data->def, "%s", data->key );
            if( data->loaded_data && data->def && strcmp( data->loaded_data, data->def ))
            {
                printf("gevas_metadata_lookup_x() loaded_data:%s\n",data->loaded_data);
                data->loaded=1;
            }
            break;

        case MD_INT:
            data->loaded_data = (gpointer)edb_lookup_int( edb,
                                                          (gint)data->def,
                                                          "%s", data->key );
            if( data->loaded_data != data->def && data->def )
            {
                data->loaded=1;
            }
            break;

        default:
            fprintf(stderr,"metadata type not coded yet!\n");
            
        }
        
        e_db_close(edb);
    }
    
    
}



gchar*
gevas_metadata_lookup_string(
    GtkgEvas *ev,
    const char* loc,
    const char* def,
    const char* fmt, ... )
{
    char* no_prefix = "";
    metadata_lookup_x_data data;
    gevas_metadata_find_edb_data* d = (gevas_metadata_find_edb_data*)&data;
    va_list args;
    gchar* key = NULL;

    g_return_if_fail(ev  != NULL);
    g_return_if_fail(loc != NULL);
	g_return_if_fail(GTK_IS_GEVAS(ev));

    va_start (args, fmt);
    key = g_strdup_vprintf( fmt, args );
    va_end (args);

    d->  edb_postfix   = loc;
    d->  edb_found_f   = gevas_metadata_lookup_x;
    d->  loaded        = 0;
    data.key           = key;
    data.def           = g_strdup(def);
    data.md_type       = MD_STRING;
    data.loaded        = 0;

    gevas_metadata_find_edb_with_data( ev, d );
    g_free(key);
    
    if(!data.loaded)
    {
        return data.def;
    }
    g_free(data.def);
    return data.loaded_data;
}


gint
gevas_metadata_lookup_int(
    GtkgEvas *ev,
    const char* loc,
    const char* def,
    const char* fmt, ... )
{
    char* no_prefix = "";
    metadata_lookup_x_data data;
    gevas_metadata_find_edb_data* d = (gevas_metadata_find_edb_data*)&data;
    va_list args;
    gchar* key = NULL;

    g_return_if_fail(ev  != NULL);
    g_return_if_fail(loc != NULL);
	g_return_if_fail(GTK_IS_GEVAS(ev));

    va_start (args, fmt);
    key = g_strdup_vprintf( fmt, args );
    va_end (args);

    d->  edb_postfix   = loc;
    d->  edb_found_f   = gevas_metadata_lookup_x;
    data.key           = key;
    data.def           = g_strdup(def);
    data.md_type       = MD_INT;

    gevas_metadata_find_edb_with_data( ev, d );
    g_free(key);
    
    if(!data.loaded)
    {
        return (gint)data.def;
    }
    g_free(data.def);
    return (gint)data.loaded_data;
}






/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/


#if 0 

struct _metadata_lookup_x_data 
{
    gpointer    def;
    char*       edb_postfix;
    const char* key;

    gboolean loaded;
    gpointer loaded_data;
    gevas_metadata_type md_type;

    GtkgEvas* gevas;
};
typedef struct _metadata_lookup_x_data metadata_lookup_x_data;


static void
metadata_lookup_x_cb( 
    gpointer data,
    gpointer user_data
    )
{
    const char* fully_qualified_prefix = data;
    metadata_lookup_x_data* d = (metadata_lookup_x_data*)user_data;
    GtkgEvas* ev      = 0;
    E_DB_File* edb    = 0;
    char* k           = 0;
    char* full_buffer = 0;
    char* filen       = 0;
    char* edb_prefix  = 0;
    char* strbuf1     = 0;
    GHashTable* hash_args = 0;

    /* Assert and init */
    g_return_if_fail(d != NULL);
	g_return_if_fail(fully_qualified_prefix!= NULL);
    g_return_if_fail(GTK_IS_GEVAS(d->gevas));
    if( d->loaded )
    {
        return;
    }
    ev = d->gevas;

    /* create the filename buffer */
    if( strlen( fully_qualified_prefix ))
    {
        full_buffer = g_strconcat( fully_qualified_prefix,
                                   "/", d->edb_postfix, 0 );
    }
    else
    {
        full_buffer = g_strdup(d->edb_postfix);
    }

    
    /* get filename and extra args */
    printf("full_buffer:%s\n",full_buffer );
    filen = strbuf1 = url_file_name_part_new( full_buffer );
    hash_args       = url_args_to_hash( full_buffer );
    g_free(full_buffer);

    
    /* load the data */
    if( edb = e_db_open(filen) )
    {
        switch(d->md_type)
        {
        case MD_STRING:
            d->loaded_data = edb_lookup_str( edb, (char*)d->def, "%s", d->key );
            if( d->loaded_data && d->def && strcmp( d->loaded_data, d->def ))
            {
                d->loaded=1;
            }
            break;

        case MD_INT:
            d->loaded_data = (gpointer)edb_lookup_int( edb, (gint)d->def, "%s", d->key );
            if( d->loaded_data != d->def && d->def )
            {
                d->loaded=1;
            }
            break;

        default:
            fprintf(stderr,"metadata type not coded yet!\n");
            
        }
        
        e_db_close(edb);
    }
        
    hash_str_str_clean( hash_args );
    g_free(strbuf1);

}


gchar*
gevas_metadata_lookup_x( GtkgEvas *ev, metadata_lookup_x_data* d )
{
    char* no_prefix = "";

    g_return_if_fail(ev != NULL);
    g_return_if_fail(d  != NULL);
	g_return_if_fail(GTK_IS_GEVAS(ev));

    d->loaded      = 0;
    d->loaded_data = 0;
    d->gevas       = ev;
    
    if( gevas_get_metadata_prefix_list(ev))
    {
        g_list_foreach( gevas_get_metadata_prefix_list(ev),
                        metadata_lookup_x_cb, d);
    }
    metadata_lookup_x_cb((gpointer)no_prefix, d);

    return d->loaded ? d->loaded_data : d->def;
}







gchar*
gevas_metadata_lookup_string(
    GtkgEvas *ev,
    const char* loc,
    const char* def,
    const char* fmt, ... )
{
    metadata_lookup_x_data data;
    gchar* ret;
    va_list args;
    gchar* key = NULL;
    
    g_return_if_fail(ev != NULL);
	g_return_if_fail(GTK_IS_GEVAS(ev));

    va_start (args, fmt);
    key = g_strdup_vprintf( fmt, args );
    va_end (args);

    data.edb_postfix = loc;
    data.key         = key;
    data.def         = g_strdup(def);
    data.md_type     = MD_STRING;
    
    ret = gevas_metadata_lookup_x( ev, &data );
    if(ret != data.def)
    {
        g_free(def);
    }
    
    g_free(key);
    return ret;
}


#endif
