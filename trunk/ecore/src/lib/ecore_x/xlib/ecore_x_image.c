/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_x_private.h"
#include "Ecore_X.h"

#include <X11/extensions/XShm.h>
#include <X11/Xutil.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>

static int _composite_available;

static int _ecore_x_image_shm_can = -1;
static int _ecore_x_image_err = 0;

static void
_ecore_x_image_error_handler(Display * d, XErrorEvent * ev)
{
   _ecore_x_image_err = 1;
}

static void
_ecore_x_image_shm_check(void)
{
   XErrorHandler ph;
   XShmSegmentInfo shminfo;
   XImage *xim;
   
   if (_ecore_x_image_shm_can != -1) return;
   
   XSync(_ecore_x_disp, False);
   _ecore_x_image_err = 0;
   
   xim = XShmCreateImage(_ecore_x_disp, 
                         DefaultVisual(_ecore_x_disp, 
                                       DefaultScreen(_ecore_x_disp)), 
                         DefaultDepth(_ecore_x_disp, 
                                      DefaultScreen(_ecore_x_disp)),
                         ZPixmap, NULL, 
                         &shminfo, 1, 1);
   if (!xim)
     {
        _ecore_x_image_shm_can = 0;
        return;
     }

   shminfo.shmid = shmget(IPC_PRIVATE, xim->bytes_per_line * xim->height,
                          IPC_CREAT | 0666);
   if (shminfo.shmid == -1)
     {
        XDestroyImage(xim);
        _ecore_x_image_shm_can = 0;
        return;
     }
   
   shminfo.readOnly = False;
   shminfo.shmaddr  = shmat(shminfo.shmid, 0, 0);
   xim->data = shminfo.shmaddr;
   
   if (xim->data == (char *)-1)
     {
        XDestroyImage(xim);
        _ecore_x_image_shm_can = 0;
        return;
     }
   
   ph = XSetErrorHandler((XErrorHandler)_ecore_x_image_error_handler);
   XShmAttach(_ecore_x_disp, &shminfo);
   XShmGetImage(_ecore_x_disp, DefaultRootWindow(_ecore_x_disp), 
                xim, 0, 0, 0xffffffff);
   XSync(_ecore_x_disp, False);
   XSetErrorHandler((XErrorHandler)ph);
   if (_ecore_x_image_err)
     {
        XShmDetach(_ecore_x_disp, &shminfo);
        shmdt(shminfo.shmaddr); 
        shmctl(shminfo.shmid, IPC_RMID, 0);
        XDestroyImage(xim);
        _ecore_x_image_shm_can = 0;
        return;
     }
   
   XShmDetach(_ecore_x_disp, &shminfo);
   shmdt(shminfo.shmaddr);
   shmctl(shminfo.shmid, IPC_RMID, 0);
   XDestroyImage(xim);
   
   _ecore_x_image_shm_can = 1;
}

struct _Ecore_X_Image
{
   XShmSegmentInfo shminfo;
   Ecore_X_Visual vis;
   XImage *xim;
   int depth;
   int w, h;
   int bpl, bpp, rows;
   unsigned char *data;
   Eina_Bool shm : 1;
};

EAPI Ecore_X_Image *
ecore_x_image_new(int w, int h, Ecore_X_Visual vis, int depth)
{
   Ecore_X_Image *im;
   
   im = calloc(1, sizeof(Ecore_X_Image));
   if (!im) return NULL;
   im->w = w;
   im->h = h;
   im->vis = vis;
   im->depth = depth;
   _ecore_x_image_shm_check();
   im->shm = _ecore_x_image_shm_can;
   return im;
}

EAPI void
ecore_x_image_free(Ecore_X_Image *im)
{
   if (im->shm)
     {
        XShmDetach(_ecore_x_disp, &(im->shminfo));
        shmdt(im->shminfo.shmaddr);
        shmctl(im->shminfo.shmid, IPC_RMID, 0);
     }
   else
     {
        if (im->xim) free(im->xim->data);
     }
   if (im->xim)
     {
        im->xim->data = NULL;
        XDestroyImage(im->xim);
     }
   free(im);
}

static void
_ecore_x_image_shm_create(Ecore_X_Image *im)
{
   im->xim = XShmCreateImage(_ecore_x_disp, im->vis, im->depth,
                             ZPixmap, NULL, &(im->shminfo), 
                             im->w, im->h);
   if (!im->xim) return;

   im->shminfo.shmid = shmget(IPC_PRIVATE, 
                              im->xim->bytes_per_line * im->xim->height,
                              IPC_CREAT | 0666);
   if (im->shminfo.shmid == -1)
     {
        XDestroyImage(im->xim);
        return;
     }
   im->shminfo.readOnly = False;
   im->shminfo.shmaddr  = shmat(im->shminfo.shmid, 0, 0);
   im->xim->data = im->shminfo.shmaddr;
   if ((im->xim->data == (char *)-1) ||
       (im->xim->data == NULL))
     {
        shmdt(im->shminfo.shmaddr);
        shmctl(im->shminfo.shmid, IPC_RMID, 0);
        XDestroyImage(im->xim);
        return;
     } 
   XShmAttach(_ecore_x_disp, &im->shminfo);
   
   im->data = im->xim->data;

   im->bpl = im->xim->bytes_per_line;
   im->rows = im->xim->height;
   if (im->xim->bits_per_pixel <= 8) im->bpp = 1;
   else if (im->xim->bits_per_pixel <= 16) im->bpp = 2;
   else im->bpp = 4;
}

EAPI void
ecore_x_image_get(Ecore_X_Image *im, Ecore_X_Drawable draw, 
                  int x, int y, int sx, int sy, int w, int h)
{
   if (im->shm)
     {
        if (!im->xim) _ecore_x_image_shm_create(im);
        if (!im->xim) return;
        // optimised path
        if ((sx == 0) && (w == im->w))
          {
             im->xim->data = 
               im->data + (im->xim->bytes_per_line * sy) + (sx * im->bpp);
             im->xim->width = w;
             im->xim->height = h;
             XShmGetImage(_ecore_x_disp, draw, im->xim, x, y, 0xffffffff);
             ecore_x_sync();
          }
        // unavoidable thanks to mit-shm get api - tmp shm buf + copy into it
        else
          {
             Ecore_X_Image *tim;
             unsigned char *spixels, *sp, *pixels, *p;
             int bpp, bpl, rows, sbpp, sbpl, srows;
             int r;
             
             tim = ecore_x_image_new(w, h, im->vis, im->depth);
             ecore_x_image_get(tim, draw, x, y, 0, 0, w, h);
             spixels = ecore_x_image_data_get(tim, &sbpl, &srows, &sbpp);
             pixels = ecore_x_image_data_get(im, &bpl, &rows, &bpp);
             p = pixels + (sy * bpl) + (sx * bpp);
             sp = spixels;
             for (r = srows; r > 0; r--)
               {
                  memcpy(p, sp, sbpl);
                  p += bpl;
                  sp += sbpl;
               }
             ecore_x_image_free(tim);
          }
     }
   else
     {
        printf("currently unimplemented ecore_x_image_get without shm\n");
     }
}

EAPI void
ecore_x_image_put(Ecore_X_Image *im, Ecore_X_Drawable draw, 
                  int x, int y, int sx, int sy, int w, int h)
{
   printf("ecore_x_image_put: unimplemented!\n");
}

EAPI void *
ecore_x_image_data_get(Ecore_X_Image *im, int *bpl, int *rows, int *bpp)
{
   if (!im->xim) _ecore_x_image_shm_create(im);
   if (!im->xim) return NULL;
   
   if (bpl) *bpl = im->bpl;
   if (rows) *rows = im->rows;
   if (bpp) *bpp = im->bpp;
   return im->data;
}
