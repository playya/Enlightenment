/*
 Copyright (C) 2000 Carsten Haitzler, Geoff Harrison and various contributors
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to
 deal in the Software without restriction, including without limitation the
 rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies of the Software, its documentation and marketing & publicity
 materials, and acknowledgment shall be given in the documentation, materials
 and software packages that this Software was used.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "dox.h"

static int          num_pages = 0;
static Page        *page = NULL;
static char        *fdat_ptr = NULL;
static int          fdat_size = 0;
static char        *fdat = NULL;

static int          fdgetc(void);
static void         fdjump(int count);

static int
fdgetc(void)
{
   int                 val;

   if (fdat_ptr >= (fdat + fdat_size))
      return EOF;
   val = (int)(*fdat_ptr);
   fdat_ptr++;
   return val;
}

static void
fdjump(int count)
{
   fdat_ptr += count;
   if (fdat_ptr < fdat)
      fdat_ptr = fdat;
   if (fdat_ptr >= (fdat + fdat_size))
      fdat_ptr = (fdat + fdat_size) - 1;
}

void
AddPage(Object * obj)
{
   num_pages++;
   page = realloc(page, sizeof(Page) * (num_pages));
   page[num_pages - 1].name = NULL;
   page[num_pages - 1].count = 0;
   page[num_pages - 1].obj = NULL;
   page[num_pages - 1].columns = 1;
   page[num_pages - 1].background = NULL;
   page[num_pages - 1].padding = 2;
   page[num_pages - 1].linkr = DEFAULT_LINKCOLOR_R;
   page[num_pages - 1].linkg = DEFAULT_LINKCOLOR_G;
   page[num_pages - 1].linkb = DEFAULT_LINKCOLOR_B;

   if ((obj) && (obj->type == PAGE))
     {
	Page               *pg;

	pg = (Page *) (obj->object);
	if (pg->name)
	   page[num_pages - 1].name = pg->name;
	page[num_pages - 1].columns = pg->columns;
	page[num_pages - 1].padding = pg->padding;
	page[num_pages - 1].linkr = pg->linkr;
	page[num_pages - 1].linkg = pg->linkg;
	page[num_pages - 1].linkb = pg->linkb;
	if (pg->background)
	   page[num_pages - 1].background = pg->background;
     }
}

void
AddObject(Object * obj)
{
   page[num_pages - 1].count++;
   page[num_pages - 1].obj =
      realloc(page[num_pages - 1].obj,
	      sizeof(Object) * (page[num_pages - 1].count));
   page[num_pages - 1].obj[page[num_pages - 1].count - 1].type =
      obj->type;
   page[num_pages - 1].obj[page[num_pages - 1].count - 1].object =
      obj->object;
}

void
BuildObj(Object * obj, char *var, char *param)
{
   static Page        *pg = NULL;
   static P_          *p = NULL;
   static Font_       *fn = NULL;
   static Img_        *img = NULL;

   switch (obj->type)
     {
     case IMG:
	if (!obj->object)
	  {
	     img = obj->object = malloc(sizeof(Img_));
	     img->src = NULL;
	     img->src2 = NULL;
	     img->src3 = NULL;
	     img->x = 0;
	     img->y = 0;
	     img->link = NULL;
	     img->w = 0;
	     img->h = 0;
	  }
	if (!strcmp(var, "x"))
	   img->x = atoi(param);
	else if (!strcmp(var, "y"))
	   img->y = atoi(param);
	else if (!strcmp(var, "src"))
	   img->src = strdup(param);
	else if (!strcmp(var, "src2"))
	   img->src2 = strdup(param);
	else if (!strcmp(var, "src3"))
	   img->src3 = strdup(param);
	else if (!strcmp(var, "href"))
	   img->link = strdup(param);
	break;
     case BR:
	break;
     case FONT:
	if (!obj->object)
	  {
	     fn = obj->object = malloc(sizeof(Font_));
	     fn->face = NULL;
	     fn->r = 0;
	     fn->g = 0;
	     fn->b = 0;
	  }
	if (!strcmp(var, "face"))
	   fn->face = strdup(param);
	else if (!strcmp(var, "color"))
	  {
	     char                hex[3] = "00";

	     if (param[0] == '#')
	       {
		  hex[0] = param[1];
		  hex[1] = param[2];
		  sscanf(hex, "%x", &(fn->r));
		  hex[0] = param[3];
		  hex[1] = param[4];
		  sscanf(hex, "%x", &(fn->g));
		  hex[0] = param[5];
		  hex[1] = param[6];
		  sscanf(hex, "%x", &(fn->b));
	       }
	  }
	break;
     case P:
	if (!obj->object)
	  {
	     p = obj->object = malloc(sizeof(P_));
	     p->align = 0;
	  }
	if (!strcmp(var, "align"))
	  {
	     if ((strlen(param) > 0) && (param[strlen(param) - 1] == '%'))
		param[strlen(param) - 1] = 0;
	     p->align = atof(param);
	  }
	break;
     case TEXT:
	break;
     case PAGE:
	if (!obj->object)
	  {
	     pg = obj->object = malloc(sizeof(Page));
	     pg->columns = 1;
	     pg->padding = 1;
	     pg->name = NULL;
	     pg->background = NULL;
	     pg->linkr = DEFAULT_LINKCOLOR_R;
	     pg->linkg = DEFAULT_LINKCOLOR_G;
	     pg->linkb = DEFAULT_LINKCOLOR_B;
	  }
	if (!strcmp(var, "columns"))
	   pg->columns = atoi(param);
	else if (!strcmp(var, "padding"))
	   pg->padding = atoi(param);
	else if (!strcmp(var, "name"))
	   pg->name = strdup(param);
	else if (!strcmp(var, "background"))
	   pg->background = strdup(param);
	else if (!strcmp(var, "linkcolor"))
	  {
	     char                hex[3] = "00";

	     if (param[0] == '#')
	       {
		  hex[0] = param[1];
		  hex[1] = param[2];
		  sscanf(hex, "%x", &(pg->linkr));
		  hex[0] = param[3];
		  hex[1] = param[4];
		  sscanf(hex, "%x", &(pg->linkg));
		  hex[0] = param[5];
		  hex[1] = param[6];
		  sscanf(hex, "%x", &(pg->linkb));
	       }
	  }
	break;
     default:
	break;
     }
}

int
GetNextTag(Object * obj)
{
   char                s[65536];
   int                 i = 0, wd = 0;
   int                 val;
   char                intag = 0;
   char                havobj = 0;

   for (;;)
     {
	val = fdgetc();
	if (val == EOF)
	   return 0;
	if (intag)
	  {
	     if (val == '>')
		intag = 0;
	     s[i++] = (char)val;
	     if (s[i - 1] == '\n')
		s[i - 1] = ' ';
	     if (s[i - 1] == '>')
		s[i - 1] = ' ';
	     if (s[i - 1] == ' ')
	       {
		  if (i == 1)
		     i = 0;
		  else
		    {
		       s[i - 1] = 0;
		       if (!havobj)
			 {
			    if (wd == 0)
			      {
				 if (!strcmp(s, "page"))
				    obj->type = PAGE;
				 else if (!strcmp(s, "img"))
				    obj->type = IMG;
				 else if (!strcmp(s, "br"))
				    obj->type = BR;
				 else if (!strcmp(s, "font"))
				    obj->type = FONT;
				 else if (!strcmp(s, "p"))
				    obj->type = P;
				 havobj = 1;
			      }
			    i = 0;
			 }
		       else
			 {
			    char                w1[1024];
			    char                w2[1024];
			    int                 j = 0;

			    w1[0] = 0;
			    w2[0] = 0;
			    while ((s[j]) && (s[j] != '='))
			      {
				 w1[j] = s[j];
				 j++;
			      }
			    w1[j] = 0;
			    if (j < (int)strlen(s))
			       strcpy(w2, &(s[j + 1]));
			    BuildObj(obj, w1, w2);
			    i = 0;
			 }
		       wd++;
		    }
	       }
	     if (!intag)
		return 1;
	  }
	if (val == '<')
	   intag = 1;
     }
   return 1;
}

char               *
GetTextUntilTag(void)
{
   char                s[65536];
   int                 i = 0;
   int                 val;

   for (;;)
     {
	val = fdgetc();
	if (val == EOF)
	  {
	     s[i] = 0;
	     if (strlen(s) < 1)
		return NULL;
	     return strdup(s);
	  }
	s[i++] = (char)val;
	if (s[i - 1] == '\n')
	   s[i - 1] = ' ';
	if ((i == 1) && (s[0] == ' '))
	   i--;
	else if (s[i - 1] == '<')
	  {
	     s[i - 1] = 0;
	     fdjump(-1);
	     if (strlen(s) < 1)
		return NULL;
	     return strdup(s);
	  }
	if ((i > 2) && (s[i - 2] == ' ') && (s[i - 1] == ' '))
	   i--;
	if (i > 65530)
	   return NULL;
     }
   return NULL;
}

int
GetObjects(FILE * f)
{
   static char         have_font = 0;
   static char         in_para = 0;
   Object              obj;
   char               *txt;
   char                buf[4096];
   int                 count;

   fdat = NULL;
   fdat_size = 0;
   while ((count = fread(buf, 1, 4096, f)) > 0)
     {
	if (!fdat)
	   fdat = malloc(count);
	else
	   fdat = realloc(fdat, (fdat_size + count));
	memcpy(fdat + fdat_size, buf, count);
	fdat_size += count;
     }
   fdat_ptr = fdat;

   if (page)
     {
	int                 i;

	for (i = 0; i < num_pages; i++)
	  {
	     int                 j;

	     if (page[i].name)
		free(page[i].name);
	     if (page[i].background)
		free(page[i].background);
	     for (j = 0; j < page[i].count; j++)
	       {
		  switch (page[i].obj[j].type)
		    {
		    case IMG:
		       if (((Img_ *) page[i].obj[j].object)->src)
			  free(((Img_ *) page[i].obj[j].object)->src);
		       if (((Img_ *) page[i].obj[j].object)->src2)
			  free(((Img_ *) page[i].obj[j].object)->src2);
		       if (((Img_ *) page[i].obj[j].object)->src3)
			  free(((Img_ *) page[i].obj[j].object)->src3);
		       if (((Img_ *) page[i].obj[j].object)->link)
			  free(((Img_ *) page[i].obj[j].object)->link);
		       break;
		    case BR:
		       break;
		    case FONT:
		       if (((Font_ *) page[i].obj[j].object)->face)
			  free(((Font_ *) page[i].obj[j].object)->face);
		       break;
		    case P:
		       break;
		    case TEXT:
		       break;
		    case PAGE:
		       break;
		    }
		  if (page[i].obj[j].object)
		     free(page[i].obj[j].object);
	       }
	     if (page[i].obj)
		free(page[i].obj);
	  }
	free(page);
	num_pages = 0;
	page = NULL;
	have_font = 0;
	in_para = 0;
     }

   obj.object = NULL;
   for (;;)
     {
	if ((have_font) && (in_para))
	  {
	     txt = GetTextUntilTag();
	     if (txt)
	       {
		  obj.type = TEXT;
		  obj.object = (void *)txt;
	       }
	     else
	       {
		  if (!GetNextTag(&obj))
		    {
		       if (fdat)
			  free(fdat);
		       return 0;
		    }
	       }
	  }
	else
	  {
	     if (!GetNextTag(&obj))
	       {
		  if (fdat)
		     free(fdat);
		  return 0;
	       }
	  }
	if (obj.type == PAGE)
	  {
	     in_para = 0;
	     have_font = 0;
	     AddPage(&obj);
	  }
	else if (page)
	   AddObject(&obj);
	if (obj.type == IMG)
	   in_para = 0;
	if (obj.type == P)
	   in_para = 1;
	if (obj.type == FONT)
	   have_font = 1;
	obj.object = NULL;
     }
   free(fdat);
}

int
FixPage(int p)
{
   if (p < 0)
      return 0;
   if (p >= num_pages)
      return num_pages - 1;
   return p;
}

int
GetPage(char *name)
{
   int                 i;

   for (i = 0; i < num_pages; i++)
     {
	if ((page[i].name) && (!strcmp(name, page[i].name)))
	   return i;
     }
   return -1;
}

void
GetLinkColors(int page_num, int *r, int *g, int *b)
{
   if (page_num < 0)
     {
	*r = DEFAULT_LINKCOLOR_R;
	*g = DEFAULT_LINKCOLOR_G;
	*b = DEFAULT_LINKCOLOR_B;
     }
   else
     {
	*r = page[page_num].linkr;
	*g = page[page_num].linkg;
	*b = page[page_num].linkb;
     }
}

Link               *
RenderPage(Window win, int page_num, int w, int h)
{
   Link               *ll = NULL;
   Page               *pg;
   TextState           ts;
   int                 i, col_w, col_h;
   int                 x, y;
   int                 justification = 0;
   int                 firstp = 1;
   ImlibImage         *im;
   int                 wastext = 0;

   ts.fontname = NULL;
   ts.style.orientation = FONT_TO_RIGHT;
   ts.style.mode = MODE_WRAP_WORD;
   ts.style.justification = 0;
   ts.style.spacing = 0;
   ts.font = NULL;
   ts.fg_col.r = 0;
   ts.fg_col.g = 0;
   ts.fg_col.b = 0;
   ts.bg_col.r = 0;
   ts.bg_col.g = 0;
   ts.bg_col.b = 0;
   ts.effect = 0;
   ts.efont = NULL;
   ts.xfont = NULL;
   ts.xfontset = 0;
   ts.xfontset_ascent = 0;
   ts.height = 0;
   pg = &(page[page_num]);
   x = pg->padding;
   y = pg->padding;
   col_w = ((w - (pg->padding * (pg->columns + 1))) / pg->columns);
   col_h = h - (pg->padding * 2);
   if (pg->background)
     {
	char                tmp[4096];

	sprintf(tmp, "%s/%s", docdir, pg->background);
	findLocalizedFile(tmp);
	im = Imlib_load_image(id, tmp);
	if (im)
	  {
	     Imlib_paste_image(id, im, win, 0, 0, w, h);
	     Imlib_destroy_image(id, im);
	  }
     }
   for (i = 0; i < pg->count; i++)
     {
	char                s[32768], ss[32768], wd[4096], *txt;
	Img_               *img;
	Font_              *fn;
	P_                 *p;
	int                 wc, eol, eot;
	int                 link = 0, lx, lw;

	switch (pg->obj[i].type)
	  {
	  case IMG:
	     img = pg->obj[i].object;
	     if (img->src)
	       {
		  char                tmp[4096];

		  sprintf(tmp, "%s/%s", docdir, img->src);
		  im = Imlib_load_image(id, tmp);
		  if (im)
		    {
		       img->w = im->rgb_width;
		       img->h = im->rgb_height;
		       Imlib_paste_image(id, im, win, img->x, img->y,
					 im->rgb_width, im->rgb_height);
		       Imlib_destroy_image(id, im);
		    }
		  if (img->link)
		    {
		       Link               *l;

		       l = malloc(sizeof(Link));
		       l->name = strdup(img->link);
		       l->x = img->x;
		       l->y = img->y;
		       l->w = img->w;
		       l->h = img->h;
		       l->next = ll;
		       ll = l;
		    }
	       }
	     break;
	  case BR:
	     if (!wastext)
		y += ts.height;
	     wastext = 0;
	     break;
	  case FONT:
	     fn = pg->obj[i].object;
	     ts.fontname = NULL;
	     ts.style.orientation = FONT_TO_RIGHT;
	     ts.style.mode = MODE_WRAP_WORD;
	     ts.style.justification = 0;
	     ts.style.spacing = 0;
	     if (ts.font)
		Fnlib_free_font(fd, ts.font);
	     ts.font = NULL;
	     ts.fg_col.r = 0;
	     ts.fg_col.g = 0;
	     ts.fg_col.b = 0;
	     ts.bg_col.r = 0;
	     ts.bg_col.g = 0;
	     ts.bg_col.b = 0;
	     ts.effect = 0;
	     if (ts.efont)
		Efont_free(ts.efont);
	     ts.efont = NULL;
	     if (ts.xfont)
		XFreeFont(disp, ts.xfont);
	     ts.xfont = NULL;
	     if (ts.xfontset)
		XFreeFontSet(disp, ts.xfontset);
	     ts.xfontset = NULL;
	     ts.xfontset_ascent = 0;
	     ts.height = 0;
	     ts.fontname = fn->face;
	     ts.fg_col.r = fn->r;
	     ts.fg_col.g = fn->g;
	     ts.fg_col.b = fn->b;
	     TextStateLoadFont(&ts);
	     break;
	  case P:
	     p = pg->obj[i].object;
	     if (p)
		justification = (int)((p->align / 100) * 1024);
	     else
		justification = 0;
	     if (!firstp)
		y += ts.height;
	     else
		firstp = 0;
	     break;
	  case TEXT:
	     txt = pg->obj[i].object;
	     wc = 1;
	     ss[0] = 0;
	     s[0] = 0;
	     eol = 0;
	     eot = 0;
	     for (;;)
	       {
		  char               *txt_disp;
		  int                 tw, th, xspace;
		  int                 off, j;
		  int                 sx, sy, ssx, ssy;
		  char                link_txt[1024];
		  char                link_link[1024];
		  int                 spaceflag, oldwc=0;

		  wd[0] = 0;
#ifdef HAVE_WCTYPE_H
		  if ( MB_CUR_MAX > 1 )                 /* If multibyte locale,... */
		    word_mb(txt, wc, wd, &spaceflag);
		  else
#endif
		    {
		      word(txt, wc, wd);
		      spaceflag = 1;
		    }
		  if (!wd[0])	eol = 1;

		  wc++;
		  eot++;
		  strcpy(ss, s);
		  if ( (eot != 1) && spaceflag)
		     strcat(s, " ");

		  if (wd[0] == '_')
		     {
			link_txt[0] = '\0';
			link_link[0] = '\0';
			link = 1;
			oldwc = wc;
		 	TextSize(&ts, s, &lx, &th, 17);
		     }

		  if ( link == 1 )
		    {
		      if ( eol || ( (wd[0] != '_') && spaceflag ) )	/* if NO link tag, ... */
			{
			  link_txt[0] = '\0';
			  link_link[0] = '\0';
			  link = 0;
			  wc = oldwc;
#ifdef HAVE_WCTYPE_H
			  if ( MB_CUR_MAX > 1 )
			    word_mb(txt, wc - 1, wd, &spaceflag);
			  else
#endif
			    {
			      word(txt, wc - 1, wd);
			      spaceflag = 1;
			    }
			}
		      else
		        {
			  int	k, linkflg;

			  j = 0;
			  linkflg = 0;
			  if ( wd[0] == '_' )	{ j++; linkflg++; }

			  k = strlen( link_txt );
			  for ( ; wd[j] != '(' && wd[j] != '\0'; j++, k++)
			    {
			      if (wd[j] == '_')	link_txt[k] = ' ';
			      else		link_txt[k] = wd[j];
			      if ( linkflg )	wd[ j - 1 ] = link_txt[k];
			      else		wd[j]       = link_txt[k];
			    }
			  link_txt[k] = '\0';
			  if ( linkflg )	wd[ j - 1 ] = '\0';

			  if ( wd[j] == '(' )
			    {
			      wd[j++] = '\0';
			      strcpy( link_link, wd + j);
			      link_link[ strlen(link_link) - 1 ] = '\0';
			      strcpy( wd, link_txt );
			      link = 2;
			    }
			  else
			    continue;
			}
		    }

		  strcat(s, wd);
		  xspace = col_w;
		  off = 0;
		  sx = x + off;
		  sy = y;
		  ssx = sx + col_w - 1;
		  ssy = sy + ts.height - 1;
		  for (j = 0; j < pg->count; j++)
		    {
		       if (pg->obj[j].type == IMG)
			 {
			    img = pg->obj[j].object;
			    if ((img->w > 0) && (img->h > 0))
			      {
				 int                 ix, iy, iix, iiy;

				 ix = img->x;
				 iy = img->y;
				 iix = img->x + img->w - 1;
				 iiy = img->y + img->h - 1;

				 if ((iy <= ssy) && (iiy >= sy))
				   {
				      if ((ix >= sx) && (ix <= ssx))
					{
					   if ((iix >= sx) && (iix <= ssx))
					     {
						if (((ix + iix) / 2) > ((sx + ssx) / 2))
						   ssx = ix - 1;
						else
						   sx = iix + 1;
					     }
					   else
					     {
						ssx = ix - 1;
					     }
					}
				      else if ((iix >= sx) && (iix <= ssx))
					{
					   sx = iix + 1;
					}
				   }
			      }
			 }
		    }
		  off = sx - x;
		  xspace = (ssx - sx) + 1;
		  if (xspace < 0)
		     xspace = 0;
		  TextSize(&ts, s, &tw, &th, 17);
		  txt_disp = ss;
		  if (eot == 1)
		     txt_disp = s;
		  if (((tw > xspace) || (eol)) && (strlen(txt_disp) > 0))
		    {
		       if ( txt_disp[strlen(txt_disp) - 1] == ' ' )
		         txt_disp[strlen(txt_disp) - 1] = 0;

		       if ((eot == 1) && (tw > xspace))
			 {
			    char                p1[4096];
			    int                 point = 0, cnt = 0, i, len;

			    while (txt_disp[(point + cnt)])
			      {
				len = mblen( txt_disp + point + cnt, MB_CUR_MAX);
				if ( len < 0 )
				  {
				     cnt++;
				     continue;
				  }
				else
				  for ( i = 0; i < len; i++, cnt++ )
					p1[cnt] = txt_disp[point + cnt];
				 p1[cnt] = 0;
				 TextSize(&ts, p1, &tw, &th, 17);
				 if ((tw > xspace) || (!txt_disp[(point + cnt)]))
				   {
				      if (txt_disp[(point + cnt)])
					{
					   point = point + cnt - len;
					   p1[cnt - len] = 0;
					   cnt = 0;
					}
				      else
					{
					   point = point + cnt;
					   p1[cnt] = 0;
					   cnt = 0;
					}
				      wastext = 1;
				      TextDraw(&ts, win, p1, x + off, y,
					    xspace, 99999, 17, justification);
				      y += ts.height;
				      if (y >= (h - (pg->padding + ts.height - (ts.height - ts.xfontset_ascent))))
					{
					   y = pg->padding;
					   x += col_w + pg->padding;
					}
				      xspace = col_w;
				      off = 0;
				      sx = x + off;
				      sy = y;
				      ssx = sx + col_w - 1;
				      ssy = sy + ts.height - 1;
				      for (j = 0; j < pg->count; j++)
					{
					   if (pg->obj[j].type == IMG)
					     {
						img = pg->obj[j].object;
						if ((img->w > 0) && (img->h > 0))
						  {
						     int                 ix,
						                         iy,
						                         iix,
						                         iiy;

						     ix = img->x;
						     iy = img->y;
						     iix = img->x + img->w - 1;
						     iiy = img->y + img->h - 1;

						     if ((iy <= ssy) && (iiy >= sy))
						       {
							  if ((ix >= sx) && (ix <= ssx))
							    {
							       if ((iix >= sx) && (iix <= ssx))
								 {
								    if (((ix + iix) / 2) > ((sx + ssx) / 2))
								       ssx = ix - 1;
								    else
								       sx = iix + 1;
								 }
							       else
								 {
								    ssx = ix - 1;
								 }
							    }
							  else if ((iix >= sx) && (iix <= ssx))
							    {
							       sx = iix + 1;
							    }
						       }
						  }
					     }
					}
				      off = sx - x;
				      xspace = (ssx - sx) + 1;
				      if (xspace < 0)
					 xspace = 0;
				   }
			      }
			 }
		       else
			 {
			    if ((tw > xspace) && (eot != 1))
			       wc--;
			    wastext = 1;
			    TextDraw(&ts, win, txt_disp, x + off, y,
				     xspace, 99999, 17, justification);
			    if ( link > 1 && !strcmp( wd, link_txt) )
			      {
				 link = 0;
				 link_link[0] = '\0';
				 link_txt[0] = '\0';
				 wc = oldwc - 1;
			      }

			    if (link > 1)
			      {
				 int                 rr, gg, bb;
				 int                 r, g, b;
				 int                 extra;
				 GC                  gc;
				 XGCValues           gcv;

				 gc = XCreateGC(disp, win, 0, &gcv);
				 rr = ts.fg_col.r;
				 gg = ts.fg_col.g;
				 bb = ts.fg_col.b;
				 r = ts.fg_col.r = pg->linkr;
				 g = ts.fg_col.g = pg->linkg;
				 b = ts.fg_col.b = pg->linkb;
				 XSetForeground(disp, gc,
				      Imlib_best_color_match(id, &r, &g, &b));
				 TextSize(&ts, txt_disp, &tw, &th, 17);
				 extra = ((xspace - tw) * justification) >> 10;
				 TextDraw(&ts, win, link_txt, x + off + lx + extra, y,
					  99999, 99999, 17, 0);
				 TextSize(&ts, link_txt, &lw, &th, 17);
				 XDrawLine(disp, win, gc,
					   x + off + lx + extra,
					   y + ts.xfontset_ascent,
					   x + off + lx + lw + extra,
					   y + ts.xfontset_ascent);
				 ts.fg_col.r = rr;
				 ts.fg_col.g = gg;
				 ts.fg_col.b = bb;
				 link = 0;
				 XFreeGC(disp, gc);
				 {
				    Link               *l;

				    l = malloc(sizeof(Link));
				    l->name = strdup(link_link);
				    l->x = x + off + lx + extra;
				    l->y = y;
				    l->w = lw;
				    l->h = ts.height;
				    l->next = ll;
				    ll = l;
				 }
				 link_link[0] = '\0';
				 link_txt[0] = '\0';
			      }
			    y += ts.height;
			    if (y >= (h - (pg->padding + ts.height - (ts.height - ts.xfontset_ascent))))
			      {
				 y = pg->padding;
				 x += col_w + pg->padding;
			      }
			 }
		       eot = 0;
		       s[0] = 0;
		    }
		  if (eol)
		     break;
	       }

	     break;
	  default:
	     break;
	  }
	if (y >= (h - (pg->padding + ts.height - (ts.height - ts.xfontset_ascent))))
	  {
	     y = pg->padding;
	     x += col_w + pg->padding;
	  }
     }

   if (ts.font)
      Fnlib_free_font(fd, ts.font);
   if (ts.efont)
      Efont_free(ts.efont);
   if (ts.xfont)
      XFreeFont(disp, ts.xfont);
   if (ts.xfontset)
      XFreeFontSet(disp, ts.xfontset);

   return ll;
}
