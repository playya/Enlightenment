/********* some functions stolen from imlib2 and modified :) ***********/

#include "Elicit.h"
#include "util.h"

void
elicit_action_color_get(int *r, int *g, int *b)
{
  Imlib_Image *im;
  Imlib_Color col;
  int red, green, blue;
  int h, s, v;
  int x, y;
  int tr;

  /* where are we pointing? */
  XQueryPointer(ecore_x_display_get(), RootWindow(ecore_x_display_get(),0), &tr, &tr, &tr, &tr, &x, &y, &tr);

  /* setup the imlib context */
  imlib_context_set_display(ecore_x_display_get());
  imlib_context_set_drawable(RootWindow(ecore_x_display_get(),0));
  imlib_context_set_visual( DefaultVisual(ecore_x_display_get(), DefaultScreen(ecore_x_display_get() ) ));

  /* get the color of the current pixel */
  im = imlib_create_image_from_drawable(0, x-1, y-1, 1, 1, 0);
  imlib_context_set_image(im);
  imlib_image_query_pixel(0, 0, &col);

  //printf("(%d, %d, %d)\n", col.red, col.green, col.blue);
  //evas_object_color_set(swatch.obj, col.red, col.green, col.blue, 255);
  /* set the color values */
  if (r) *r = col.red;
  if (g) *g = col.green;
  if (b) *b = col.blue;

  /* update the other formats */
  /*
  _rgb_to_hsv();
  _rgb_to_hex();

  elicit_ui_update_text(el);
  changed = 1;
  */
  imlib_free_image(); 

}

void
elicit_action_shoot(Evas_Object *shot, int w, int h)
{
  Imlib_Image *im;
  int x, y;
  int px, py;
  int dw, dh;
  int tr;
  double sw, sh;

  XQueryPointer(ecore_x_display_get(), RootWindow(ecore_x_display_get(),0), &tr, &tr, &tr, &tr, &px, &py, &tr);

  printf("shooting (%d x %d)!\n", w, h);
  x = px - .5 * w;
  y = py - .5 * h;

  ecore_x_window_size_get(RootWindow(ecore_x_display_get(),0), &dw, &dh);
  if (x < 0) x = 0;
  if (y < 0) y = 0;
  if (x + w > dw) x = dw - w;
  if (y + h > dh) y = dh - h;

  /* setup the imlib context */
  imlib_context_set_display(ecore_x_display_get());
  imlib_context_set_drawable(RootWindow(ecore_x_display_get(),0));
  imlib_context_set_visual( DefaultVisual(ecore_x_display_get(), DefaultScreen(ecore_x_display_get() ) ));

  /* copy the correct part of the screen */
  im = imlib_create_image_from_drawable(0, x, y, w, h, 1);
  imlib_context_set_image(im);
  imlib_image_set_format("argb");

  /* get the object ready, copy the data in */
  evas_object_image_alpha_set(shot, 0);
  evas_object_image_size_set(shot, w, h);
  evas_object_image_smooth_scale_set(shot, 0);

  evas_object_image_data_copy_set(shot, imlib_image_get_data_for_reading_only());
  
  /* tell evas that we changed part of the image data */
  evas_object_image_data_update_add(shot, 0, 0, w, h);

  /* set it to fill the whole object */
  evas_object_geometry_get(shot, NULL, NULL, &sw, &sh);
  evas_object_image_fill_set(shot, 0, 0, sw, sh);

  imlib_free_image();
}




void
elicit_color_rgb_to_hsv(int rr, int gg, int bb, double *hh, double *ss, double *vv)
{
   int r, g, b;
   int f;
   float i,j,k,max,min,d;
   float h, s, v;

   r = rr;
   g = gg;
   b = bb;

   i = ((float)r)/255.0;
   j = ((float)g)/255.0;
   k = ((float)b)/255.0;

   f = 0;
   max = min = i;
   if (j>max) { max = j; f = 1; } else min = j;
   if (k>max) { max = k; f = 2; } else if (k<min) min = k;
   d = max - min;

   v = max;
   if (max!=0) s = d/max; else s = 0;
   if (s==0)
      h = 0;
   else
   {
      switch (f)
      {
         case 0:
           h = (j - k)/d;
           break;
         case 1:
           h = 2 + (k - i)/d;
           break;
         case 2:
           h = 4 + (i - j)/d;
           break;
      }
      h *= 60.0;
      if (h<0) h += 360.0;
   }

   if (hh) *hh = h;
   if (ss) *ss = s;
   if (vv) *vv = v;

   //printf("%i %i %i %f %f %f\n", r, g, b, hue, saturation, value);
}

void
elicit_color_hsv_to_rgb(double hh, double ss, double vv, int *rr, int *gg, int *bb)
{
   int i,p,q,t;
   float vs,vsf;
   int r, g, b;
   float h, s, v;

   h = hh;
   s = ss;
   v = vv;

   i = (int)(v*255.0);
   if (s==0)
      r = g = b = i;
   else
   {
      if (h==360) h = 0;
      h = h/60.0;
      vs = v * s;
                vsf = vs * (h - (int)h);
      p = (int)(255.0 * (v - vs));
      q = (int)(255.0 * (v - vsf));
      t = (int)(255.0 * (v - vs + vsf));
      switch ((int)h)
      {
         case 0:
           r = i;
                          g = t;
                          b = p;
                          break;
         case 1:
           r = q;
                          g = i;
                          b = p;
           break;
         case 2:
           r = p;
                          g = i;
                          b = t;
           break;
         case 3:
           r = p;
                          g = q;
                          b = i;
           break;
         case 4:
           r = t;
                          g = p;
                          b = i;
           break;
         case 5:
           r = i;
                          g = p;
                          b = q;
           break;
      }
   }

   if (rr) *rr = r;
   if (gg) *gg = g;
   if (bb) *bb = b;
} 

char * 
elicit_color_rgb_to_hex(int rr, int gg, int bb)
{
  char buf[8];

  sprintf(buf, "#%.2x%.2x%.2x\0", rr, gg, bb);
  return (char *)strdup(buf);
}

int
elicit_glob_match(char *str, char *glob)
{
   if (!strcmp(glob, "*")) return 1;
   if (!fnmatch(glob, str, 0)) return 1;
   return 0;
}

