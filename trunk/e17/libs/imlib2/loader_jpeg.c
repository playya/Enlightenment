#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "common.h"
#include <string.h>
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
#include <X11/Xutil.h>
#include "image.h"
#include <jpeglib.h>
#include <setjmp.h>

struct ImLib_JPEG_error_mgr
{
   struct jpeg_error_mgr   pub;
   sigjmp_buf              setjmp_buffer;
};
typedef struct ImLib_JPEG_error_mgr *emptr;

void _JPEGFatalErrorHandler(j_common_ptr cinfo);
char load (ImlibImage *im,
	   void (*progress)(ImlibImage *im, char percent,
			    int update_x, int update_y,
			    int update_w, int update_h),
	   char progress_granularity, char immediate_load);
char save (ImlibImage *im,
	   void (*progress)(ImlibImage *im, char percent,
			    int update_x, int update_y,
			    int update_w, int update_h),
	   char progress_granularity);
void formats (ImlibLoader *l);

void
_JPEGFatalErrorHandler(j_common_ptr cinfo)
{
   emptr               errmgr;
   
   errmgr = (emptr) cinfo->err;
/*   cinfo->err->output_message(cinfo);*/
   siglongjmp(errmgr->setjmp_buffer, 1);
   return;
}

char 
load (ImlibImage *im,
      void (*progress)(ImlibImage *im, char percent, 
		       int update_x, int update_y, 
		       int update_w, int update_h),
      char progress_granularity, char immediate_load)
{
   int                  w, h;
   struct              jpeg_decompress_struct cinfo;
   struct              ImLib_JPEG_error_mgr jerr;
   FILE               *f;
   
   if (im->data)
      return 0;
   f = fopen(im->file, "rb");
   if (!f)
      return 0;
   cinfo.err = jpeg_std_error(&(jerr.pub));
   jerr.pub.error_exit = _JPEGFatalErrorHandler;
   if (sigsetjmp(jerr.setjmp_buffer, 1))
     {
	jpeg_destroy_decompress(&cinfo);
	fclose(f);
	return 0;
     }
   jpeg_create_decompress(&cinfo);
   jpeg_stdio_src(&cinfo, f);
   jpeg_read_header(&cinfo, TRUE);
   cinfo.do_fancy_upsampling = FALSE;
   cinfo.do_block_smoothing = FALSE;
   jpeg_start_decompress(&cinfo);
   if ((!im->loader) && (!im->data))
     {
	im->w = w = cinfo.output_width;
	im->h = h = cinfo.output_height;
	UNSET_FLAG(im->flags, F_HAS_ALPHA);
	im->format = strdup("jpeg");
     }
   if (((!im->data) && (im->loader)) || (immediate_load) || (progress))
     {
	DATA8  *ptr, *line[16], *data;
	DATA32 *ptr2;
	int     x, y, l, i, scans, count, prevy;
	
	im->w = w = cinfo.output_width;
	im->h = h = cinfo.output_height;
	
	if (cinfo.rec_outbuf_height > 16)
          {	   
	     jpeg_destroy_decompress(&cinfo);
	     fclose(f);
	     return 0;
	  }	
	data = malloc(w * 16 * 3);
	if (!data)
	  {
	     jpeg_destroy_decompress(&cinfo);
	     fclose(f);
	     return 0;
	  }
	/* must set the im->data member before callign progress function */
	ptr2 = im->data = malloc(w * h * sizeof(DATA32));
	if (!im->data)
	  {
	     free(data);
	     jpeg_destroy_decompress(&cinfo);
	     fclose(f);
	     return 0;
	  }
	count = 0;
	prevy = 0;
	if (cinfo.output_components == 3)
	  {
	     for (i = 0; i < cinfo.rec_outbuf_height; i++)
		line[i] = data + (i * w * 3);
	     for (l = 0; l < h; l += cinfo.rec_outbuf_height)
	       {
		  jpeg_read_scanlines(&cinfo, line, cinfo.rec_outbuf_height);
		  scans = cinfo.rec_outbuf_height;
		  if ((h - l) < scans)
		     scans = h - l;
		  ptr = data;
		  for (y = 0; y < scans; y++)
		    {
		       for (x = 0; x < w; x++)
			 {
			    *ptr2 = (0xff000000) | 
			       ((ptr[0]) << 16) | 
			       ((ptr[1]) << 8) |
			       (ptr[2]);
			    ptr += 3;
			    ptr2 ++;
			 }
		    }
		  if (progress)
		    {
		       int per;
		       
		       per = (l * 100) / h;
		       if ((per - count) >=  progress_granularity)
			 {
			    count = per;
			    progress(im, per, 0, prevy, w, scans + l - prevy);
			    prevy = l + scans;
			 }
		    }
	       }
	  }
	else if (cinfo.output_components == 1)
	  {
	     for (i = 0; i < cinfo.rec_outbuf_height; i++)
		line[i] = data + (i * w);
	     for (l = 0; l < h; l += cinfo.rec_outbuf_height)
	       {
		  jpeg_read_scanlines(&cinfo, line, cinfo.rec_outbuf_height);
		  scans = cinfo.rec_outbuf_height;
		  if ((h - l) < scans)
		     scans = h - l;
		  ptr = data;
		  for (y = 0; y < scans; y++)
		    {
		       for (x = 0; x < w; x++)
			 {
			    *ptr2 = (0xff000000) | 
			       ((ptr[0]) << 16) | 
			       ((ptr[0]) << 8) |
			       (ptr[0]);
			    ptr ++;
			    ptr2 ++;
			 }
		    }
		  if (progress)
		    {
		       int per;
		       
		       per = (l * 100) / h;
		       if ((per - count) >=  progress_granularity)
			 {
			    count = per;
			    progress(im, per, 0, prevy, w, l + scans - prevy);
			    prevy = l + scans;
			 }
		    }
	       }
	  }	   
	free(data);	
     }
   jpeg_finish_decompress(&cinfo);
   jpeg_destroy_decompress(&cinfo);
   fclose(f);
   return 1;
}

char 
save (ImlibImage *im,
      void (*progress)(ImlibImage *im, char percent, 
		       int update_x, int update_y, 
		       int update_w, int update_h),
      char progress_granularity)
{
	im = NULL;
	progress = NULL;
   return 0;
}

void 
formats (ImlibLoader *l)
{  
   char *list_formats[] = 
     { "jpg", "jpeg", "jfif", "jfi" };

     {
	int i;
	
	l->num_formats = (sizeof(list_formats) / sizeof (char *));
	l->formats = malloc(sizeof(char *) * l->num_formats);
	for (i = 0; i < l->num_formats; i++)
	   l->formats[i] = strdup(list_formats[i]);
     }
}

