/**
 * Corey Donohoe <atmos@atmos.org>
 * Filename: entice_image.c
 * Smart Object: ;
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <Ecore.h>
#include "image.h"

#define DEBUG 0

static void entice_image_resize(Evas_Object * o, double w, double h);
static int _entice_image_scroll_timer(void *data);
static void _entice_image_mouse_down_translate(void *data, Evas * e,
                                               Evas_Object * obj,
                                               void *event_info);
static void _entice_image_mouse_in_translate(void *data, Evas * e,
                                             Evas_Object * obj,
                                             void *event_info);
static void _entice_image_mouse_out_translate(void *data, Evas * e,
                                              Evas_Object * obj,
                                              void *event_info);
static void _entice_image_mouse_up_translate(void *data, Evas * e,
                                             Evas_Object * obj,
                                             void *event_info);
static void _entice_image_mouse_wheel_translate(void *data, Evas * e,
                                                Evas_Object * obj,
                                                void *event_info);

const char *
entice_image_format_get(Evas_Object * o)
{
   char *result = NULL;
   Entice_Image *im = NULL;

   if ((im = evas_object_smart_data_get(o)))
      result = im->format;
   return (result);
}

void
entice_image_format_set(Evas_Object * o, const char *format)
{
   char buf[PATH_MAX];
   Entice_Image *im = NULL;

   if ((im = evas_object_smart_data_get(o)))
   {
      if (im->format)
         free(im->format);
      snprintf(buf, PATH_MAX, "%s", format);
      im->format = strdup(buf);
   }
}

/**
 * entice_image_rotate - rotate the image using imlib2
 * @o - the Entice Image Object
 * @direction - 1 to flip clockwise, 3 to flip counter clockwise
 */
int
entice_image_rotate(Evas_Object * o, int orientation)
{
   int iw, ih;
   double w, h, x, y;
   Entice_Image *im = NULL;
   Imlib_Image imlib_im = NULL;

   if ((im = evas_object_smart_data_get(o)))
   {
      evas_object_image_size_get(im->obj, &iw, &ih);
      evas_object_geometry_get(o, &x, &y, &w, &h);

      if (imlib_im =
          imlib_create_image_using_copied_data(iw, ih,
                                               evas_object_image_data_get(im->
                                                                          obj,
                                                                          1)))
      {
         imlib_context_set_image(imlib_im);
         imlib_image_orientate(orientation);
         im->iw = imlib_image_get_width();
         im->ih = imlib_image_get_height();
         evas_object_image_size_set(im->obj, im->iw, im->ih);
         evas_object_image_data_copy_set(im->obj,
                                         imlib_image_get_data_for_reading_only
                                         ());
         evas_object_resize(o, w, h);
         /* if we're fitting, it'll need to be recalculated */
         if (entice_image_zoom_fit_get(o))
            entice_image_zoom_fit(o);
         evas_damage_rectangle_add(evas_object_evas_get(o), x, y, w, h);
         imlib_free_image();
         return (1);
      }
   }
   return (0);
}

/**
 * entice_image_flip - flip the image using imlib2
 * @o - the Entice Image Object
 * @direction - non-zero to flip vertical, zero to flip horizontal
 */
int
entice_image_flip(Evas_Object * o, int orientation)
{
   int iw, ih;
   double w, h, x, y;
   Entice_Image *im = NULL;
   Imlib_Image imlib_im = NULL;

   if ((im = evas_object_smart_data_get(o)))
   {
      evas_object_image_size_get(im->obj, &iw, &ih);
      evas_object_geometry_get(o, &x, &y, &w, &h);

      if (imlib_im =
          imlib_create_image_using_copied_data(iw, ih,
                                               evas_object_image_data_get(im->
                                                                          obj,
                                                                          1)))
      {
         imlib_context_set_image(imlib_im);
         if (orientation)
            imlib_image_flip_horizontal();
         else
            imlib_image_flip_vertical();

         im->iw = imlib_image_get_width();
         im->ih = imlib_image_get_height();
         evas_object_image_size_set(im->obj, im->iw, im->ih);
         evas_object_image_data_copy_set(im->obj,
                                         imlib_image_get_data_for_reading_only
                                         ());
         evas_object_resize(o, w, h);
         /* if we're fitting, it'll need to be recalculated */
         if (entice_image_zoom_fit_get(o))
            entice_image_zoom_fit(o);
         evas_damage_rectangle_add(evas_object_evas_get(o), x, y, w, h);
         imlib_free_image();
         return (1);
      }
   }
   return (0);
}

/**
 * entice_image_save - save the image using imlib2
 * @o - the Entice Image Object
 */
int
entice_image_save(Evas_Object * o)
{
   int iw, ih;
   double w, h;
   Entice_Image *im = NULL;
   Imlib_Image imlib_im = NULL;

   if ((im = evas_object_smart_data_get(o)))
   {
      evas_object_image_size_get(im->obj, &iw, &ih);
      evas_object_geometry_get(o, NULL, NULL, &w, &h);

      if (imlib_im =
          imlib_create_image_using_copied_data(iw, ih,
                                               evas_object_image_data_get(im->
                                                                          obj,
                                                                          1)))
      {
         imlib_context_set_image(imlib_im);
         if (im->format && im->filename)
         {
            imlib_image_set_format(im->format);
            imlib_save_image(im->filename);
         }
         imlib_free_image();
         return (1);
      }
   }
   return (0);
}

void
entice_image_file_set(Evas_Object * o, const char *filename)
{
   char buf[PATH_MAX];
   Entice_Image *im = NULL;

   if ((filename) && (im = evas_object_smart_data_get(o)))
   {
      snprintf(buf, PATH_MAX, "%s", filename);
      if (im->filename)
         free(im->filename);
      im->filename = strdup(buf);
   }
}
const char *
entice_image_file_get(Evas_Object * o)
{
   Entice_Image *im = NULL;

   if ((im = evas_object_smart_data_get(o)))
      return (im->filename);
   return (NULL);
}

/**
 * entice_image_zoom_fit_get - find out whether we're fitting this obj
 * @o - the Entice_Image object
 * Return 1 if it is being fitted, 0 if it's not
 */
int
entice_image_zoom_fit_get(Evas_Object * o)
{
   int result = 0;
   Entice_Image *im = NULL;

   if ((im = evas_object_smart_data_get(o)))
      result = im->fit;
   return (result);
}

/**
 * entice_image_scroll_stop - stop scrolling in any direction
 * @o - the Entice_Image object
 */
void
entice_image_scroll_stop(Evas_Object * o)
{
   Entice_Image *im = NULL;

   if ((im = evas_object_smart_data_get(o)))
   {
      if (im->scroll.timer)
         ecore_timer_del(im->scroll.timer);
      im->scroll.timer = NULL;
   }
}

/**
 * entice_image_scroll_start - start scrolling in a given direction
 * @o - the Entice_Image object
 * @d - the Entice_Image_Scroll_Direction
 */
void
entice_image_scroll_start(Evas_Object * o, Entice_Scroll_Direction d)
{
   Entice_Image *im = NULL;

   if ((im = evas_object_smart_data_get(o)))
   {
      if (im->scroll.timer)
         return;
      im->scroll.direction = d;
      im->scroll.velocity = 1.0 * im->zoom;
      im->scroll.start_time = ecore_time_get();
      im->scroll.timer = ecore_timer_add(0.03, _entice_image_scroll_timer, o);
   }
}

/**
 * entice_image_scroll - scrolling in a given direction, a given value
 * @o - the Entice_Image object
 * @d - the Entice_Image_Scroll_Direction
 * @val - the number of pixels to scroll
 */
void
entice_image_scroll(Evas_Object * o, Entice_Scroll_Direction d, int val)
{
   Entice_Image *im = NULL;

   if ((im = evas_object_smart_data_get(o)))
   {
      switch (d)
      {
        case ENTICE_SCROLL_NORTH:
           if (im->scroll.y > 0)
              im->scroll.y -= val;
           if (im->scroll.y < 0)
              im->scroll.y = 0;
           break;
        case ENTICE_SCROLL_EAST:
           if (im->scroll.x > 0)
              im->scroll.x += val;
           if (im->scroll.x < 0)
              im->scroll.x = 0;
           break;
        case ENTICE_SCROLL_SOUTH:
           if (im->scroll.y > 0)
              im->scroll.y += val;
           if (im->scroll.y < 0)
              im->scroll.y = 0;
           break;
        case ENTICE_SCROLL_WEST:
           if (im->scroll.x > 0)
              im->scroll.x -= val;
           if (im->scroll.x < 0)
              im->scroll.x = 0;
           break;
        default:
#if DEBUG
           fprintf(stderr, "SCrolling WTF\n");
#endif
           break;
      }
      entice_image_resize(o, im->w, im->h);
   }
}

/**
 * entice_image_zoom_get - get the current zoom value for the image
 * @o - The Entice_Image we're curious about
 */
double
entice_image_zoom_get(Evas_Object * o)
{
   double result = 1.0;
   Entice_Image *im = NULL;

   if ((im = evas_object_smart_data_get(o)))
      result = im->zoom;
   return (result);
}

/**
 * entice_image_zoom_set - set the current zoom value for the image
 * @o - The Entice_Image we're curious about
 * @val - the new zoom value for our Image
 */
void
entice_image_zoom_set(Evas_Object * o, double val)
{
   Entice_Image *im = NULL;

   if ((im = evas_object_smart_data_get(o)))
   {
      im->zoom = val;
      im->fit = 0;
      evas_object_resize(o, im->w, im->h);
   }
}

/**
 * entice_image_zoom_fit - fit the current image to the clip
 * @o - The Entice_Image we're fitting
 */
void
entice_image_zoom_fit(Evas_Object * o)
{
   Entice_Image *im = NULL;

   if ((im = evas_object_smart_data_get(o)))
   {
      if (im->iw > im->ih)
         im->zoom = ((double) (im->iw) / (double) im->w);
      else
         im->zoom = ((double) (im->ih) / (double) im->h);
      im->fit = 1;
      entice_image_resize(o, im->w, im->h);
   }
}

/**
 * entice_image_zoom_reset - set the scale to be 1:1
 * @o - The Entice_Image we're resetting zoom for
 */
void
entice_image_zoom_reset(Evas_Object * o)
{
   Entice_Image *im = NULL;

   if ((im = evas_object_smart_data_get(o)))
   {
      im->zoom = 1.0;
      im->fit = 0;
      entice_image_resize(o, im->w, im->h);
   }
}

/**
 * entice_image_zoom_out - zoom out by a factor of zoom *= 1.414
 * @o - The Entice_Image we're zooming
 */
void
entice_image_zoom_out(Evas_Object * o)
{
   Entice_Image *im = NULL;

   if ((im = evas_object_smart_data_get(o)))
   {
#if DEBUG
      fprintf(stderr, "Zooming Out!! %0.2f\n", im->zoom);
#endif
      im->zoom *= 1.414;
      im->fit = 0;
      entice_image_resize(o, im->w, im->h);
   }

}

/**
 * entice_image_zoom_in - zoom in by a factor of zoom *= 1.414
 * @o - The Entice_Image we're zooming
 */
void
entice_image_zoom_in(Evas_Object * o)
{
   Entice_Image *im = NULL;

   if ((im = evas_object_smart_data_get(o)))
   {
#if DEBUG
      fprintf(stderr, "Zooming In!! %0.2f\n", im->zoom);
#endif
      im->zoom /= 1.414;
      if (im->zoom < 0.03125)
         im->zoom = 0.03125;
      im->fit = 0;
      entice_image_resize(o, im->w, im->h);
   }
}
void
entice_image_edje_set(Evas_Object * o, Evas_Object * edje)
{
   Entice_Image *im = NULL;

   if ((edje) && (im = evas_object_smart_data_get(o)))
   {
      evas_object_event_callback_add(im->obj, EVAS_CALLBACK_MOUSE_IN,
                                     _entice_image_mouse_in_translate, edje);
      evas_object_event_callback_add(im->obj, EVAS_CALLBACK_MOUSE_OUT,
                                     _entice_image_mouse_out_translate, edje);
      evas_object_event_callback_add(im->obj, EVAS_CALLBACK_MOUSE_UP,
                                     _entice_image_mouse_up_translate, edje);
      evas_object_event_callback_add(im->obj, EVAS_CALLBACK_MOUSE_DOWN,
                                     _entice_image_mouse_down_translate,
                                     edje);
      evas_object_event_callback_add(im->obj, EVAS_CALLBACK_MOUSE_WHEEL,
                                     _entice_image_mouse_wheel_translate,
                                     edje);
   }
}

/**
 * _entice_image_scroll_timer - our ecore timer to do continuous
 * scrolling
 * @data - a pointer to the object we're scrolling
 * Returns 1 until the image's boundaries are reached, or the timer is
 * manually deleted elsewhere
 */
static int
_entice_image_scroll_timer(void *data)
{
   Entice_Image *im = NULL;
   int ok = 1;

   if (data && ((im = evas_object_smart_data_get((Evas_Object *) data))))
   {
      double dt, dx;
      double ix, iy, iw, ih;
      double offset;

      dt = ecore_time_get() - im->scroll.start_time;
      dx = 10 * (1 - exp(-dt));
      offset = dx * im->scroll.velocity;

      evas_object_geometry_get(im->obj, &ix, &iy, &iw, &ih);


      switch (im->scroll.direction)
      {
        case ENTICE_SCROLL_NORTH:
           if (ih > im->h)
           {
              iy += offset;
              im->scroll.y += offset;
              if (im->scroll.y > ((ih - im->h) / 2))
              {
                 im->scroll.y = ((ih - im->h) / 2);
                 ok = 0;
              }
           }
           break;
        case ENTICE_SCROLL_SOUTH:
           if (ih > im->h)
           {
              iy -= offset;
              im->scroll.y -= offset;
              if (im->scroll.y < -((ih - im->h) / 2))
              {
                 im->scroll.y = -((ih - im->h) / 2);
                 ok = 0;
              }

           }
           break;
        case ENTICE_SCROLL_EAST:
           if (iw > im->w)
           {
              ix -= offset;
              im->scroll.x -= offset;
              if (im->scroll.x < -((iw - im->w) / 2))
              {
                 im->scroll.x = -((iw - im->w) / 2);
                 ok = 0;
              }
           }
           break;
        case ENTICE_SCROLL_WEST:
           if (iw > im->w)
           {
              ix += offset;
              im->scroll.x += offset;
              if (im->scroll.x > ((iw - im->w) / 2))
              {
                 im->scroll.x = ((iw - im->w) / 2);
                 ok = 0;
              }
           }
           break;
        default:
#if DEBUG
           fprintf(stderr, "Scrolling\n");
#endif
           break;
      }
      evas_object_resize((Evas_Object *) data, im->w, im->h);
      if (!ok)
      {
         ecore_timer_del(im->scroll.timer);
         im->scroll.timer = NULL;
      }
      /* 
         evas_object_move(im->obj, ix, iy); */
   }
   return (ok);
}

/*=========================================================================
 * Entice_Image smart object definitions
 *=======================================================================*/
static void
entice_image_add(Evas_Object * o)
{
   Entice_Image *im = NULL;

   im = (Entice_Image *) malloc(sizeof(Entice_Image));
   memset(im, 0, sizeof(Entice_Image));
   im->zoom = 1.0;

   im->clip = evas_object_rectangle_add(evas_object_evas_get(o));
   evas_object_color_set(im->clip, 255, 255, 255, 255);
   evas_object_layer_set(im->clip, 0);

   im->zoom = 1.0;
   im->fit = 1;
   evas_object_smart_data_set(o, im);
}
static void
entice_image_del(Evas_Object * o)
{
   Entice_Image *im = NULL;

   if ((im = evas_object_smart_data_get(o)))
   {
      if (im->filename)
         free(im->filename);
      evas_object_del(im->obj);
      free(im);
   }
}
static void
entice_image_layer_set(Evas_Object * o, int layer)
{
   Entice_Image *im = NULL;

   if ((im = evas_object_smart_data_get(o)))
   {
      evas_object_layer_set(im->obj, layer);
   }
}
static void
entice_image_raise(Evas_Object * o)
{
   Entice_Image *im = NULL;

   if ((im = evas_object_smart_data_get(o)))
   {
      evas_object_raise(im->obj);
   }
}
static void
entice_image_lower(Evas_Object * o)
{
   Entice_Image *im = NULL;

   if ((im = evas_object_smart_data_get(o)))
   {
      evas_object_lower(im->obj);
   }
}
static void
entice_image_stack_above(Evas_Object * o, Evas_Object * above)
{
   Entice_Image *im = NULL;

   if ((im = evas_object_smart_data_get(o)))
   {
      evas_object_stack_above(im->obj, above);
   }
}
static void
entice_image_stack_below(Evas_Object * o, Evas_Object * below)
{
   Entice_Image *im = NULL;

   if ((im = evas_object_smart_data_get(o)))
   {
      evas_object_stack_below(im->obj, below);
   }
}
static void
entice_image_move(Evas_Object * o, double x, double y)
{
   Entice_Image *im = NULL;

   if ((im = evas_object_smart_data_get(o)))
   {
      evas_object_move(im->clip, x, y);
      im->x = x;
      im->y = y;
   }
}
static void
entice_image_resize(Evas_Object * o, double w, double h)
{
   Entice_Image *im = NULL;

   if ((im = evas_object_smart_data_get(o)))
   {
      double ww = 0.0, hh = 0.0;

      im->w = w;
      im->h = h;
      evas_object_resize(im->clip, im->w, im->h);

      if (w < 5 || h < 5)
         return;
      if (im->zoom > w || im->zoom > h)
         im->zoom = w < h ? w : h;

      ww = (int) ((double) im->iw / im->zoom);
      hh = (int) ((double) im->ih / im->zoom);

      if (ww > w)
      {
         if (im->scroll.x > ((ww - w) / 2))
            im->scroll.x = ((ww - w) / 2);
         else if (im->scroll.x < -((ww - w + 1) / 2))
            im->scroll.x = -((ww - w + 1) / 2);
      }
      else
         im->scroll.x = 0;
      if (hh > h)
      {
         if (im->scroll.y > ((hh - h) / 2))
            im->scroll.y = ((hh - h) / 2);
         else if (im->scroll.y < -((hh - h + 1) / 2))
            im->scroll.y = -((hh - h + 1) / 2);
      }
      else
         im->scroll.y = 0;
      evas_object_move(im->obj, im->scroll.x + im->x + (im->w - ww) / 2,
                       im->scroll.y + im->y + (im->h - hh) / 2);
      evas_object_resize(im->obj, ww, hh);
      evas_object_image_fill_set(im->obj, 0, 0, ww, hh);
   }
}
static void
entice_image_show(Evas_Object * o)
{
   Entice_Image *im = NULL;

   if ((im = evas_object_smart_data_get(o)))
   {
      evas_object_show(im->clip);
   }
}
static void
entice_image_hide(Evas_Object * o)
{
   Entice_Image *im = NULL;

   if ((im = evas_object_smart_data_get(o)))
   {
      evas_object_hide(im->clip);
   }
}
static void
entice_image_color_set(Evas_Object * o, int r, int g, int b, int a)
{
   Entice_Image *im = NULL;

   if ((im = evas_object_smart_data_get(o)))
   {
      evas_object_color_set(im->clip, r, g, b, a);
   }
}
static void
entice_image_clip_set(Evas_Object * o, Evas_Object * clip)
{
   Entice_Image *im = NULL;

   if ((im = evas_object_smart_data_get(o)))
   {
      evas_object_clip_set(im->clip, clip);
   }
}
static void
entice_image_clip_unset(Evas_Object * o)
{
   Entice_Image *im = NULL;

   if ((im = evas_object_smart_data_get(o)))
   {
      evas_object_clip_unset(im->clip);
   }
}
static Evas_Smart *
entice_image_get(void)
{
   Evas_Smart *s = NULL;

   s = evas_smart_new("EnticeImage", entice_image_add, entice_image_del,
                      entice_image_layer_set, entice_image_raise,
                      entice_image_lower, entice_image_stack_above,
                      entice_image_stack_below, entice_image_move,
                      entice_image_resize, entice_image_show,
                      entice_image_hide, entice_image_color_set,
                      entice_image_clip_set, entice_image_clip_unset, NULL);
   return (s);
}

Evas_Object *
entice_image_new(Evas_Object * image)
{
   int w, h;
   Evas_Object *o = NULL;
   Entice_Image *im = NULL;

   if (image)
   {
      o = evas_object_smart_add(evas_object_evas_get(image),
                                entice_image_get());

      im = evas_object_smart_data_get(o);
      im->obj = image;

      evas_object_image_size_get(im->obj, &w, &h);
      evas_object_clip_set(im->obj, im->clip);
      evas_object_show(im->obj);
      im->iw = w;
      im->ih = h;
   }
   return (o);
}

/*==========================================================================
 * Test app for entice_image by itself
 * #define TESTING to 1 at the top of the file
 *========================================================================*/
#if TESTING
#include<Ecore.h>
#include<Ecore_Evas.h>

static Evas_Object *bg = NULL;

/**
 * exit_cb - called when the app exits(window is killed)
 * @ev_type -
 * @ev - 
 * @data -
 */
static int
exit_cb(int ev_type, void *ev, void *data)
{
   ecore_main_loop_quit();
   return (0);
}

/**
 * window_resize_cb - when the ecore_evas is resized by the user
 * @ee - the Ecore_Evas that was resized 
 */
static void
window_resize_cb(Ecore_Evas * ee)
{
   int w, h;

   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
   evas_object_resize(bg, (double) w, (double) h);
}

/**
 * window_del_cb - callback for when the ecore_evas is deleted
 * @ee - the Ecore_Evas that was deleted
 */
static void
window_del_cb(Ecore_Evas * ee)
{
   ecore_main_loop_quit();
}

/**
 * main - your C apps start here, duh.
 * @argc - unused
 * @argv - unused
 */
int
main(int argc, const char *argv[])
{
   Evas *evas = NULL;
   Ecore_Evas *e = NULL;
   Evas_Object *o = NULL, *oo = NULL;

   if (argc < 2)
      return (1);
   ecore_init();
   ecore_app_args_set(argc, argv);

   ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, exit_cb, NULL);

   if (ecore_evas_init())
   {
      e = ecore_evas_software_x11_new(NULL, 0, 0, 0, 300, 120);
      ecore_evas_title_set(e, "Entice Image Test");
      ecore_evas_callback_delete_request_set(e, window_del_cb);
      ecore_evas_callback_resize_set(e, window_resize_cb);

      evas = ecore_evas_get(e);
      bg = evas_object_rectangle_add(evas);
      evas_object_move(bg, 0, 0);
      evas_object_resize(bg, 300, 120);
      evas_object_color_set(bg, 89, 94, 97, 255);
      evas_object_layer_set(bg, 0);
      evas_object_show(bg);

      o = evas_object_image_add(evas);
      evas_object_image_file_set(o, argv[1], NULL);

      oo = entice_image_new(o);
      evas_object_move(oo, 0, 0);
      evas_object_resize(oo, 300, 120);
      evas_object_show(oo);
      ecore_evas_show(e);
   }
   ecore_main_loop_begin();
   return (0);
}
#endif
/**
 * The following five functions translate evas mouse events into edje
 * mouse event signal emissions
 */
static void
_entice_image_mouse_wheel_translate(void *data, Evas * e, Evas_Object * obj,
                                    void *event_info)
{
   Evas_Event_Mouse_Wheel *ev = NULL;
   Evas_Object *o = NULL;

   if ((ev = (Evas_Event_Mouse_Wheel *) event_info))
   {
      if ((o = (Evas_Object *) data))
      {
         char buf[PATH_MAX];

         snprintf(buf, PATH_MAX, "mouse,wheel,%i,%i", (int) ev->direction,
                  (int) ev->z);
         edje_object_signal_emit(o, buf, "EnticeImage");
      }
   }
#if DEBUG
   fprintf(stderr, "MouseWheel\n");
#endif
}

static void
_entice_image_mouse_in_translate(void *data, Evas * e, Evas_Object * obj,
                                 void *event_info)
{
   Evas_Event_Mouse_In *ev = NULL;
   Evas_Object *o = NULL;

   if ((ev = (Evas_Event_Mouse_In *) event_info))
      if ((o = (Evas_Object *) data))
         edje_object_signal_emit(o, "mouse,in", "EnticeImage");
#if DEBUG
   fprintf(stderr, "MouseIn\n");
#endif
}

static void
_entice_image_mouse_out_translate(void *data, Evas * e, Evas_Object * obj,
                                  void *event_info)
{
   Evas_Event_Mouse_Out *ev = NULL;
   Evas_Object *o = NULL;

   if ((ev = (Evas_Event_Mouse_Out *) event_info))
      if ((o = (Evas_Object *) data))
         edje_object_signal_emit(o, "mouse,out", "EnticeImage");
#if DEBUG
   fprintf(stderr, "MouseOut\n");
#endif
}

static void
_entice_image_mouse_up_translate(void *data, Evas * e, Evas_Object * obj,
                                 void *event_info)
{
   Evas_Event_Mouse_Up *ev = NULL;
   Evas_Object *o = NULL;

   if ((ev = (Evas_Event_Mouse_Up *) event_info))
   {
      if ((o = (Evas_Object *) data))
      {
         char buf[PATH_MAX];

         snprintf(buf, PATH_MAX, "mouse,up,%i", (int) ev->button);
         edje_object_signal_emit(o, buf, "EnticeImage");
      }
   }
#if DEBUG
   fprintf(stderr, "MouseUp");
#endif
}

static void
_entice_image_mouse_down_translate(void *data, Evas * e, Evas_Object * obj,
                                   void *event_info)
{
   Evas_Event_Mouse_Down *ev = NULL;
   Evas_Object *o = NULL;

   if ((ev = (Evas_Event_Mouse_Down *) event_info))
   {
      if ((o = (Evas_Object *) data))
      {
         char buf[PATH_MAX];

         snprintf(buf, PATH_MAX, "mouse,down,%i", (int) ev->button);
         edje_object_signal_emit(o, buf, "EnticeImage");
      }
   }
#if DEBUG
   fprintf(stderr, "MouseDown\n");
#endif
}
