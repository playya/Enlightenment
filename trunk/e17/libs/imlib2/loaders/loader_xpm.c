#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "common.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
#include <X11/Xutil.h>
#include "image.h"
#include "Imlib2.h"

char load(ImlibImage *im, ImlibProgressFunction progress,char progress_granularity, char immediate_load);
char save(ImlibImage *im, ImlibProgressFunction progress, char progress_granularity);
void formats(ImlibLoader *l);

static FILE *rgb_txt = NULL;

static void
xpm_parse_color(char *color, int *r, int *g, int *b)
{
   char buf[4096];
   
   /* is a #ff00ff like color */
   if (color[0] == '#')
     {
	int len;
	char val[32];
	
	len = strlen(color) - 1;
	if (len < 96)
	  {
	     int i;
	     
	     len /= 3;
	     for (i = 0; i < len; i++) val[i] = color[1 + i + (0 * len)];
	     val[i] = 0;
	     sscanf(val, "%x", r);
	     for (i = 0; i < len; i++) val[i] = color[1 + i + (1 * len)];
	     val[i] = 0;
	     sscanf(val, "%x", g);
	     for (i = 0; i < len; i++) val[i] = color[1 + i + (2 * len)];
	     val[i] = 0;
	     sscanf(val, "%x", b);
	     if (len == 1) 
		{
		   *r = (*r << 4) | *r;
		   *g = (*g << 4) | *g;
		   *b = (*b << 4) | *b;
		}
	     else if (len > 2)
	       {
		  *r >>= (len / 3) * 4;
		  *g >>= (len / 3) * 4;
		  *b >>= (len / 3) * 4;
	       }
	  }
	return;
     }
   /* look in rgb txt database */
   if (!rgb_txt)
      rgb_txt = fopen("/usr/X11R6/lib/X11/rgb.txt", "r");
   if (!rgb_txt)
      return;
   fseek(rgb_txt, 0, SEEK_SET);
   while (fgets(buf, 4000, rgb_txt))
     {
	if (buf[0] != '!')
	  {
	     int rr, gg, bb;
	     char name[4096];
	     
	     sscanf(buf, "%i %i %i %[^\n]", &rr, &gg, &bb, name);
	     if (!strcasecmp(name, color))
	       {
		  *r = rr;
		  *g = gg;
		  *b = bb;
		  return;
	       }
	  }
     }
}

static void
xpm_parse_done(void)
{
   if (rgb_txt)
      fclose(rgb_txt);
   rgb_txt = NULL;
}

char 
load(ImlibImage *im, ImlibProgressFunction progress, char progress_granularity, char immediate_load)
{
  DATA32             *ptr, *end;
  FILE               *f;

  int                 pc, c, i, j, k, w, h, ncolors, cpp, comment, transp, quote,
                      context, len, done, r, g, b;
  char                *line, s[256], tok[128], col[256];
  XColor              xcol;
  int                 lsz = 256;
  struct _cmap
  {
    unsigned char     str[6];
    unsigned char     transp;
    short             r, g, b;
  } *cmap;

  short               lookup[128 - 32][128 - 32];
  float               per = 0.0, per_inc;
  int                 last_per = 0, last_y = 0;

  done = 0;
  transp = -1;

  /* if immediate_load is 1, then dont delay image laoding as below, or */
  /* already data in this image - dont load it again */

  if (im->data)
     {
	xpm_parse_done();
	return 0;
     }
  f = fopen(im->file, "rb");
  if (!f)
     {
	xpm_parse_done();
	return 0;
     }
  
  i = 0;
  j = 0;
  cmap = NULL;
  w = 10;
  h = 10;
  ptr = NULL;
  end = NULL;
  c = ' ';
  comment = 0;
  quote = 0;
  context = 0;
  line = malloc(lsz);
  while (!done)
    {
      pc = c;
      c = fgetc(f);
      if (c == EOF)
	break;
      if (!quote)
	{
	  if ((pc == '/') && (c == '*'))
	    comment = 1;
	  else if ((pc == '*') && (c == '/') && (comment))
	    comment = 0;
	}
      if (!comment)
	{
	  if ((!quote) && (c == '"'))
	    {
	      quote = 1;
	      i = 0;
	    }
	  else if ((quote) && (c == '"'))
	    {
	      line[i] = 0;
	      quote = 0;
	      if (context == 0)
		{
		  /* Header */
		  sscanf(line, "%i %i %i %i", &w, &h, &ncolors, &cpp);
                  if (ncolors > 32766)
		    {
		      fprintf(stderr, "IMLIB ERROR: XPM files with colors > 32766 not supported\n");
		      free(line);
		      fclose(f);
		       xpm_parse_done();
		      return 0;
		    }
		  if (cpp > 5)
		    {
		      fprintf(stderr, "IMLIB ERROR: XPM files with characters per pixel > 5 not supported\n");
		      free(line);
		      fclose(f);
		       xpm_parse_done();
		      return 0;
		    }
		  if (w > 32767)
		    {
		      fprintf(stderr, "IMLIB ERROR: Image width > 32767 pixels for file\n");
		      free(line);
		      fclose(f);
		       xpm_parse_done();
		      return 0;
		    }
		  if (h > 32767)
		    {
		      fprintf(stderr, "IMLIB ERROR: Image height > 32767 pixels for file\n");
		      free(line);
		      fclose(f);
		       xpm_parse_done();
		      return 0;
		    }
		  cmap = malloc(sizeof(struct _cmap) * ncolors);

		  if (!cmap)
		    {
		      free(line);
		      fclose(f);
		       xpm_parse_done();
		      return 0;
		    }

		  im->w = w;
		  im->h = h;
		  
		  if (!im->format)
		    im->format = strdup("xpm");
		  
		  per_inc = 100.0 / (((float) w) * h);
		  
		  if (im->loader || immediate_load || progress)
		    {
		      im->data = (DATA32 *) malloc(sizeof(DATA32) * w * h);
		      if (!im->data)
			{
			  free(cmap);
			  free(line);
			  fclose(f);
			   xpm_parse_done();
			  return 0;
			}
		      ptr = im->data;
		      end = ptr + (sizeof(DATA32) * w * h);
		    }
		  j = 0;
		  context++;
		}
	      else if (context == 1)
		{
		  /* Color Table */
		  if (j < ncolors)
		    {
		      int                 slen;
		      int                 hascolor, iscolor;

		      iscolor = 0;
		      hascolor = 0;
		      tok[0] = 0;
		      col[0] = 0;
		      s[0] = 0;
		      len = strlen(line);
		      strncpy(cmap[j].str, line, cpp);
		      cmap[j].str[cpp] = 0;
		      cmap[j].r = -1;
		      cmap[j].transp = 0;
		      for (k = cpp; k < len; k++)
			{
			  if (line[k] != ' ')
			    {
			      s[0] = 0;
			      sscanf(&line[k], "%65535s", s);
			      slen = strlen(s);
			      k += slen;
			      if (!strcmp(s, "c"))
				iscolor = 1;
			      if ((!strcmp(s, "m")) || (!strcmp(s, "s")) ||
				  (!strcmp(s, "g4")) || (!strcmp(s, "g")) ||
				  (!strcmp(s, "c")) || (k >= len))
				{
				  if (k >= len)
				    {
				      if (col[0])
					strcat(col, " ");
                                      if (strlen(col) + strlen(s) < sizeof(col))
					strcat(col, s);
				    }
				  if (col[0])
				    {
				      if (!strcasecmp(col, "none"))
					{
					  transp = 1;
					  cmap[j].transp = 1;
					}
				      else
					{
					  if ((((cmap[j].r < 0) ||
						(!strcmp(tok, "c"))) &&
					       (!hascolor)))
					    {
					       r = 0; g = 0; b = 0;
					      xpm_parse_color(col, &r, &g, &b);
					      cmap[j].r = r;
					      cmap[j].g = g;
					      cmap[j].b = b;
					      if (iscolor)
						hascolor = 1;
					    }
					}
				    }
				  strcpy(tok, s);
				  col[0] = 0;
				}
			      else
				{
				  if (col[0])
				    strcat(col, " ");
				  strcat(col, s);
				}
			    }
			}
		    }
		  j++;
		  if (j >= ncolors)
		    {
		      if (cpp == 1)
			for (i = 0; i < ncolors; i++)
			  lookup[(int)cmap[i].str[0] - 32][0] = i;
		      if (cpp == 2)
			for (i = 0; i < ncolors; i++)
			  lookup[(int)cmap[i].str[0] - 32][(int)cmap[i].str[1] - 32] = i;
		      context++;
		    }
		}
	      else
		{
		  /* Image Data */
		  i = 0;
		  if (cpp == 0)
		    {
		      /* Chars per pixel = 0? well u never know */
		    }
		  if (cpp == 1)
		    {
		      if (transp)
			{
			  for (i = 0; ((i < 65536) && (ptr < end) && (line[i])); i++)
			    {
			      col[0] = line[i];
			      if (cmap[lookup[(int)col[0] - 32][0]].transp)
				{
				  r = (unsigned char)cmap[lookup[(int)col[0] - 32][0]].r;
				  g = (unsigned char)cmap[lookup[(int)col[0] - 32][0]].g;
				  b = (unsigned char)cmap[lookup[(int)col[0] - 32][0]].b;
				  *ptr++ = 0x00ffffff & ((r << 16) | (g << 8) | b);
				}
			      else
				{
				  r = (unsigned char)cmap[lookup[(int)col[0] - 32][0]].r;
				  g = (unsigned char)cmap[lookup[(int)col[0] - 32][0]].g;
				  b = (unsigned char)cmap[lookup[(int)col[0] - 32][0]].b;
				  *ptr++ = (0xff << 24) | (r << 16) | (g << 8) | b;
				}
			    }
			}
		      else
			{
			  for (i = 0; ((i < 65536) && (ptr < end) && (line[i])); i++)
			    {
			      col[0] = line[i];
			      r = (unsigned char)cmap[lookup[(int)col[0] - 32][0]].r;
			      g = (unsigned char)cmap[lookup[(int)col[0] - 32][0]].g;
			      b = (unsigned char)cmap[lookup[(int)col[0] - 32][0]].b;
			      *ptr++ = (0xff << 24) | (r << 16) | (g << 8) | b;
			    }
			}
		    }
		  else if (cpp == 2)
		    {
		      if (transp)
			{
			  for (i = 0; ((i < 65536) && (ptr < end) && (line[i])); i++)
			    {
			      col[0] = line[i++];
			      col[1] = line[i];
			      if (cmap[lookup[(int)col[0] - 32][(int)col[1] - 32]].transp)
				{
				  r = (unsigned char)cmap[lookup[(int)col[0] - 32][(int)col[1] - 32]].r;
				  g = (unsigned char)cmap[lookup[(int)col[0] - 32][(int)col[1] - 32]].g;
				  b = (unsigned char)cmap[lookup[(int)col[0] - 32][(int)col[1] - 32]].b;
				  *ptr++ = 0x00ffffff & ((r << 16) | (g << 8) | b);
				}
			      else
				{
				  r = (unsigned char)cmap[lookup[(int)col[0] - 32][(int)col[1] - 32]].r;
				  g = (unsigned char)cmap[lookup[(int)col[0] - 32][(int)col[1] - 32]].g;
				  b = (unsigned char)cmap[lookup[(int)col[0] - 32][(int)col[1] - 32]].b;
				  *ptr++ = (0xff << 24) | (r << 16) | (g << 8) | b;
				}
			    }
			}
		      else
			{
			  for (i = 0; ((i < 65536) && (ptr < end) && (line[i])); i++)
			    {
			      col[0] = line[i++];
			      col[1] = line[i];
			      r = (unsigned char)cmap[lookup[(int)col[0] - 32][(int)col[1] - 32]].r;
			      g = (unsigned char)cmap[lookup[(int)col[0] - 32][(int)col[1] - 32]].g;
			      b = (unsigned char)cmap[lookup[(int)col[0] - 32][(int)col[1] - 32]].b;
			      *ptr++ = (0xff << 24) | (r << 16) | (g << 8) | b;
			    }
			}
		    }
		  else
		    {
		      if (transp)
			{
			  for (i = 0; ((i < 65536) && (ptr < end) && (line[i])); i++)
			    {
			      for (j = 0; j < cpp; j++, i++)
				{
				  col[j] = line[i];
				}
			      col[j] = 0;
			      i--;
			      for (j = 0; j < ncolors; j++)
				{
				  if (!strcmp(col, cmap[j].str))
				    {
				      if (cmap[j].transp)
					{
					  r = (unsigned char)cmap[lookup[(int)col[0] - 32][0]].r;
					  g = (unsigned char)cmap[lookup[(int)col[0] - 32][0]].g;
					  b = (unsigned char)cmap[lookup[(int)col[0] - 32][0]].b;
					  *ptr++ = 0x00ffffff & ((r << 16) | (g << 8) | b);
					}
				      else
					{
					  r = (unsigned char)cmap[j].r;
					  g = (unsigned char)cmap[j].g;
					  b = (unsigned char)cmap[j].b;
					  *ptr++ = (0xff << 24) | (r << 16) | (g << 8) | b;
					}
				      j = ncolors;
				    }
				}
			    }
			}
		      else
			{
			  for (i = 0; ((i < 65536) && (ptr < end) && (line[i])); i++)
			    {
			      for (j = 0; j < cpp; j++, i++)
				{
				  col[j] = line[i];
				}
			      col[j] = 0;
			      i--;
			      for (j = 0; j < ncolors; j++)
				{
				  if (!strcmp(col, cmap[j].str))
				    {
				      r = (unsigned char)cmap[j].r;
				      g = (unsigned char)cmap[j].g;
				      b = (unsigned char)cmap[j].b;
				      *ptr++ = (0xff << 24) | (r << 16) | (g << 8) | b;
				      j = ncolors;
				    }
				}
			    }
			}
		    }
		  per += per_inc;
		  if (progress && (((int) per) != last_per) && (((int) per) % progress_granularity == 0))
		    {
		      last_per = (int) per;
		      if (!(progress(im, (int) per, 0, last_y, w, i)))
			{
			  fclose(f);
			  free(cmap);
			  free(line);
			   xpm_parse_done();
			  return 2;
			}
		      last_y = i;
		    }
		}
	    }
	}
      /* Scan in line from XPM file */
      if ((!comment) && (quote) && (c != '"'))
	{
	  if (c < 32)
	    c = 32;
	  else if (c > 127)
	    c = 127;
	  line[i++] = c;
	}
      if (i >= lsz)
	{
	  lsz += 256;
	  line = realloc(line, lsz);
	}
      if ((ptr) && ((ptr - im->data) >= w * h * sizeof(DATA32)))
	done = 1;
    }

  if (transp >= 0) {
    SET_FLAG(im->flags, F_HAS_ALPHA);
  } else {
    UNSET_FLAG(im->flags, F_HAS_ALPHA);
  }

  if (progress)
    {
      progress(im, 100, 0, last_y, w, h);
    }

  free(cmap);
  free(line);

   xpm_parse_done();

  return 1;
}

char 
save(ImlibImage *im, ImlibProgressFunction progress, char progress_granularity)
{
   return 0;
   im = NULL;
   progress = NULL;
   progress_granularity = 0;
}

/* fills the ImlibLoader struct with a strign array of format file */
/* extensions this loader can load. eg: */
/* loader->formats = { "jpeg", "jpg"}; */
/* giving permutations is a good idea. case sensitivity is irrelevant */
/* your laoder CAN load more than one format if it likes - like: */
/* loader->formats = { "gif", "png", "jpeg", "jpg"} */
/* if it can load those formats. */
void 
formats (ImlibLoader *l)
{  
   /* this is the only bit you have to change... */
   char *list_formats[] = 
     { "xpm" };

   /* don't bother changing any of this - it just reads this in and sets */
   /* the struct values and makes copies */
     {
	int i;
	
	l->num_formats = (sizeof(list_formats) / sizeof (char *));
	l->formats = malloc(sizeof(char *) * l->num_formats);
	for (i = 0; i < l->num_formats; i++)
	   l->formats[i] = strdup(list_formats[i]);
     }
}

