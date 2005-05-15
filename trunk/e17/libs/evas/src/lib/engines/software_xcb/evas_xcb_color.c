#include "evas_common.h"
#include "evas_engine.h"

#include <X11/XCB/xcb.h>

typedef struct _Convert_Pal_Priv Convert_Pal_Priv;

struct _Convert_Pal_Priv
{
   XCBConnection *conn;
   XCBCOLORMAP    cmap;
   XCBVISUALTYPE *vis;
};

typedef DATA8 * (*Xcb_Func_Alloc_Colors) (XCBConnection *conn, XCBCOLORMAP cmap, XCBVISUALTYPE *v);

static Xcb_Func_Alloc_Colors x_color_alloc[PAL_MODE_LAST + 1];
static int                   x_color_count[PAL_MODE_LAST + 1];
static Evas_List            *palettes = NULL;

static DATA8 * x_color_alloc_rgb(int nr, int ng, int nb, XCBConnection *conn, XCBCOLORMAP cmap, XCBVISUALTYPE *v);
static DATA8 * x_color_alloc_gray(int ng, XCBConnection *conn, XCBCOLORMAP cmap, XCBVISUALTYPE *v);

static DATA8 * x_color_alloc_rgb_332  (XCBConnection *conn, XCBCOLORMAP cmap, XCBVISUALTYPE *v);
static DATA8 * x_color_alloc_rgb_666  (XCBConnection *conn, XCBCOLORMAP cmap, XCBVISUALTYPE *v);
static DATA8 * x_color_alloc_rgb_232  (XCBConnection *conn, XCBCOLORMAP cmap, XCBVISUALTYPE *v);
static DATA8 * x_color_alloc_rgb_222  (XCBConnection *conn, XCBCOLORMAP cmap, XCBVISUALTYPE *v);
static DATA8 * x_color_alloc_rgb_221  (XCBConnection *conn, XCBCOLORMAP cmap, XCBVISUALTYPE *v);
static DATA8 * x_color_alloc_rgb_121  (XCBConnection *conn, XCBCOLORMAP cmap, XCBVISUALTYPE *v);
static DATA8 * x_color_alloc_rgb_111  (XCBConnection *conn, XCBCOLORMAP cmap, XCBVISUALTYPE *v);
static DATA8 * x_color_alloc_gray_256 (XCBConnection *conn, XCBCOLORMAP cmap, XCBVISUALTYPE *v);
static DATA8 * x_color_alloc_gray_64  (XCBConnection *conn, XCBCOLORMAP cmap, XCBVISUALTYPE *v);
static DATA8 * x_color_alloc_gray_16  (XCBConnection *conn, XCBCOLORMAP cmap, XCBVISUALTYPE *v);
static DATA8 * x_color_alloc_gray_4   (XCBConnection *conn, XCBCOLORMAP cmap, XCBVISUALTYPE *v);
static DATA8 * x_color_alloc_mono     (XCBConnection *conn, XCBCOLORMAP cmap, XCBVISUALTYPE *v);

static DATA8 *
x_color_alloc_rgb(int            nr,
		  int            ng,
		  int            nb,
		  XCBConnection *conn,
		  XCBCOLORMAP    cmap,
		  XCBVISUALTYPE *v)
{
   int    r, g, b, i;
   DATA8 *color_lut;
   int    sig_mask = 0;

   for (i = 0; i < v->bits_per_rgb_value; i++) sig_mask |= (0x1 << i);
   sig_mask <<= (16 - v->bits_per_rgb_value);
   i = 0;
   color_lut = malloc((nr) * (ng) * (nb));
   if (!color_lut) return NULL;
   for (r = 0; r < (nr); r++)
     {
	for (g = 0; g < (ng); g++)
	  {
	     for (b = 0; b < (nb); b++)
	       {
		  XCBCOLORITEM xcl;
		  XCBCOLORITEM xcl_in;
		  int val;
		  XCBAllocColorRep *rep;

                  val = (int)((((double)r) / ((nr) - 1)) * 65535);
		  xcl.red = (unsigned short)(val);
		  val = (int)((((double)g) / ((ng) - 1)) * 65535);
		  xcl.green = (unsigned short)(val);
		  val = (int)((((double)b) / ((nb) - 1)) * 65535);
		  xcl.blue = (unsigned short)(val);
		  xcl_in = xcl;
		  rep = XCBAllocColorReply(conn,
					   XCBAllocColor(conn, cmap,
							 xcl.red, xcl.green, xcl.blue),
					   0);
		  /* TODO: XAllocColor tries to approach the color */
		  /* in case the allocation fails */
		  /* XCB does not that (i think). It should be done */
		  /* So if rep == NULL, the other following tests */
		  /* should be always satisfied */
		  if ((!rep) ||
		      ((xcl_in.red   & sig_mask) != (xcl.red   & sig_mask)) ||
		      ((xcl_in.green & sig_mask) != (xcl.green & sig_mask)) ||
		      ((xcl_in.blue  & sig_mask) != (xcl.blue  & sig_mask)))
		    {
		       unsigned long pixels[256];
		       int j;

		       if (i > 0)
			 {
			    for(j = 0; j < i; j++)
			      pixels[j] = (unsigned long)color_lut[j];
			    XCBFreeColors(conn, cmap, 0, i, pixels);
			 }
		       free(color_lut);
		       return NULL;
		    }
		  color_lut[i] = rep->pixel;
		  i++;
		  free(rep);
	       }
	  }
     }
   return color_lut;
}

static DATA8 *
x_color_alloc_gray(int            ng,
		   XCBConnection *conn,
		   XCBCOLORMAP    cmap,
		   XCBVISUALTYPE *v)
{
   int g, i;
   DATA8 *color_lut;
   int sig_mask = 0;

   for (i = 0; i < v->bits_per_rgb_value; i++) sig_mask |= (0x1 << i);
   sig_mask <<= (16 - v->bits_per_rgb_value);
   i = 0;
   color_lut = malloc(ng);
   if (!color_lut) return NULL;
   for (g = 0; g < (ng); g++)
     {
	XCBCOLORITEM xcl;
	XCBCOLORITEM xcl_in;
	int val;
	XCBAllocColorRep *rep;
	
	val = (int)((((double)g) / ((ng) - 1)) * 65535);
	xcl.red = (unsigned short)(val);
	xcl.green = (unsigned short)(val);
	xcl.blue = (unsigned short)(val);
	xcl_in = xcl;
	rep = XCBAllocColorReply(conn,
				 XCBAllocColor(conn, cmap,
					       xcl.red, xcl.green, xcl.blue),
				 0);
	/* TODO: XAllocColor tries to approach the color */
	/* in case the allocation fails */
	/* XCB does not that (i think). It should be done */
	/* So if rep == NULL, the other following tests */
	/* should be always satisfied */
	if ((!rep) ||
	    ((xcl_in.red   & sig_mask) != (xcl.red   & sig_mask)) ||
	    ((xcl_in.green & sig_mask) != (xcl.green & sig_mask)) ||
	    ((xcl_in.blue  & sig_mask) != (xcl.blue  & sig_mask)))
	  {
	     unsigned long pixels[256];
	     int j;
	     
	     if (i > 0)
	       {
		  for(j = 0; j < i; j++)
		    pixels[j] = (unsigned long) color_lut[j];
		  XCBFreeColors(conn, cmap, 0, i, pixels);
	       }
	     free(color_lut);
	     return NULL;
	  }
	color_lut[i] = xcl.pixel;
	i++;
	free(rep);
     }
   return color_lut;
}

static DATA8 *
x_color_alloc_rgb_332(XCBConnection *conn,
		      XCBCOLORMAP    cmap,
		      XCBVISUALTYPE *v)
{
   return x_color_alloc_rgb(8, 8, 4, conn, cmap, v);
}

static DATA8 *
x_color_alloc_rgb_666(XCBConnection *conn,
		      XCBCOLORMAP    cmap,
		      XCBVISUALTYPE *v)
{
   return x_color_alloc_rgb(6, 6, 6, conn, cmap, v);
}

static DATA8 *
x_color_alloc_rgb_232(XCBConnection *conn,
		      XCBCOLORMAP    cmap,
		      XCBVISUALTYPE *v)
{
   return x_color_alloc_rgb(4, 8, 4, conn, cmap, v);
}

static DATA8 *
x_color_alloc_rgb_222(XCBConnection *conn,
		      XCBCOLORMAP    cmap,
		      XCBVISUALTYPE *v)
{
   return x_color_alloc_rgb(4, 4, 4, conn, cmap, v);
}

static DATA8 *
x_color_alloc_rgb_221(XCBConnection *conn,
				 XCBCOLORMAP    cmap,
				 XCBVISUALTYPE *v)
{
   return x_color_alloc_rgb(4, 4, 2, conn, cmap, v);
}

static DATA8 *
x_color_alloc_rgb_121(XCBConnection *conn,
		      XCBCOLORMAP    cmap,
		      XCBVISUALTYPE *v)
{
   return x_color_alloc_rgb(2, 4, 2, conn, cmap, v);
}

static DATA8 *
x_color_alloc_rgb_111(XCBConnection *conn,
		      XCBCOLORMAP    cmap,
		      XCBVISUALTYPE *v)
{
   return x_color_alloc_rgb(2, 2, 2, conn, cmap, v);
}

static DATA8 *
x_color_alloc_gray_256(XCBConnection *conn,
		       XCBCOLORMAP    cmap,
		       XCBVISUALTYPE *v)
{
   return x_color_alloc_gray(256, conn, cmap, v);
}

static DATA8 *
x_color_alloc_gray_64(XCBConnection *conn,
		      XCBCOLORMAP    cmap,
		      XCBVISUALTYPE *v)
{
   return x_color_alloc_gray(64, conn, cmap, v);
}

static DATA8 *
x_color_alloc_gray_16(XCBConnection *conn,
		      XCBCOLORMAP    cmap,
		      XCBVISUALTYPE *v)
{
   return x_color_alloc_gray(32, conn, cmap, v);
}

static DATA8 *
x_color_alloc_gray_4(XCBConnection *conn,
		     XCBCOLORMAP    cmap,
		     XCBVISUALTYPE *v)
{
   return x_color_alloc_gray(16, conn, cmap, v);
}

static DATA8 *
x_color_alloc_mono(XCBConnection *conn,
		   XCBCOLORMAP    cmap,
		   XCBVISUALTYPE *v)
{
   return x_color_alloc_gray(2, conn, cmap, v);
}

void
evas_software_xcb_x_color_init(void)
{
   static int initialised = 0;
   
   if (initialised) return;
   x_color_alloc[PAL_MODE_NONE]    = NULL;
   x_color_count[PAL_MODE_NONE]    = 0;
   
   x_color_alloc[PAL_MODE_MONO]    = x_color_alloc_mono;
   x_color_count[PAL_MODE_MONO]    = 2;
   
   x_color_alloc[PAL_MODE_GRAY4]   = x_color_alloc_gray_4;
   x_color_count[PAL_MODE_GRAY4]   = 4;
   
   x_color_alloc[PAL_MODE_GRAY16]  = x_color_alloc_gray_16;
   x_color_count[PAL_MODE_GRAY16]  = 16;
   
   x_color_alloc[PAL_MODE_GRAY64]  = x_color_alloc_gray_64;
   x_color_count[PAL_MODE_GRAY64]  = 64;
   
   x_color_alloc[PAL_MODE_GRAY256] = x_color_alloc_gray_256;
   x_color_count[PAL_MODE_GRAY256] = 256;
   
   x_color_alloc[PAL_MODE_RGB111]  = x_color_alloc_rgb_111;
   x_color_count[PAL_MODE_RGB111]  = 2 * 2 * 2;
   
   x_color_alloc[PAL_MODE_RGB121]  = x_color_alloc_rgb_121;
   x_color_count[PAL_MODE_RGB121]  = 2 * 4 * 2;
   
   x_color_alloc[PAL_MODE_RGB221]  = x_color_alloc_rgb_221;
   x_color_count[PAL_MODE_RGB221]  = 4 * 4 * 2;
   
   x_color_alloc[PAL_MODE_RGB222]  = x_color_alloc_rgb_222;
   x_color_count[PAL_MODE_RGB222]  = 4 * 4 * 4;
   
   x_color_alloc[PAL_MODE_RGB232]  = x_color_alloc_rgb_232;
   x_color_count[PAL_MODE_RGB232]  = 4 * 8 * 4;
   
   x_color_alloc[PAL_MODE_RGB666]  = x_color_alloc_rgb_666;
   x_color_count[PAL_MODE_RGB666]  = 6 * 6 * 6;
   
   x_color_alloc[PAL_MODE_RGB332]  = x_color_alloc_rgb_332;
   x_color_count[PAL_MODE_RGB332]  = 8 * 8 * 4;
   
   x_color_alloc[PAL_MODE_LAST]    = NULL;
   x_color_count[PAL_MODE_LAST]    = 0;
   initialised = 1;
}

Convert_Pal *
evas_software_xcb_x_color_allocate(XCBConnection   *conn,
				   XCBCOLORMAP      cmap,
				   XCBVISUALTYPE   *vis,
				   Convert_Pal_Mode colors)
{
   Convert_Pal_Priv *palpriv;
   Convert_Pal      *pal;
   Convert_Pal_Mode  c;
   Evas_List        *l;

   for (l = palettes; l; l = l->next)
     {
	pal = l->data;
	palpriv = pal->data;
	if ((conn == palpriv->conn) &&
	    (vis  == palpriv->vis)  &&
	    (cmap.xid == palpriv->cmap.xid))
	  {
	     pal->references++;
	     return pal;
	  }	    
     }
   pal = calloc(1, sizeof(struct _Convert_Pal));
   if (!pal) return NULL;
   for (c = colors; c > PAL_MODE_NONE; c--)
     {
	if (x_color_alloc[c])
	  {
	     pal->lookup = (x_color_alloc[c])(conn, cmap, vis);
	     if (pal->lookup) break;
	  }
     }
   pal->references = 1;
   pal->colors = c;
   pal->count = x_color_count[c];
   palpriv = calloc(1, sizeof(Convert_Pal_Priv));
   pal->data = palpriv;
   if (!palpriv)
     {
	if (pal->lookup) free(pal->lookup);
	free(pal);
	return NULL;
     }
   palpriv->conn = conn;
   palpriv->vis = vis;
   palpriv->cmap = cmap;
   if (pal->colors == PAL_MODE_NONE)
     {
	if (pal->lookup) free(pal->lookup);
	free(pal);
	return NULL;
     }
   palettes = evas_list_append(palettes, pal);
   return pal;
}

void
evas_software_xcb_x_color_deallocate(XCBConnection *conn,
				     XCBCOLORMAP    cmap,
				     XCBVISUALTYPE *vis, 
				     Convert_Pal   *pal)
{
   unsigned long pixels[256];
   int           j;
   
   pal->references--;
   if (pal->references > 0) return;
   if (pal->lookup) 
     {	
	for(j = 0; j < pal->count; j++) 
	  pixels[j] = (unsigned long) pal->lookup[j];
	XCBFreeColors(conn, cmap, 0, pal->count, pixels);
	free(pal->lookup);
     }
   free(pal->data);
   palettes = evas_list_remove(palettes, pal);
   free(pal);
}
