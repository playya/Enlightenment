/*
 * Copyright (C) 2000-2005 Carsten Haitzler, Geoff Harrison and various contributors
 * Copyright (C) 2004-2005 Kim Woelders
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies of the Software, its documentation and marketing & publicity
 * materials, and acknowledgment shall be given in the documentation, materials
 * and software packages that this Software was used.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "E.h"
#include "conf.h"
#include "emodule.h"
#include "xwin.h"

#ifdef ENABLE_THEME_TRANSPARENCY

static Imlib_Color_Modifier *icm = NULL;
static DATA8        gray[256];
static DATA8        alpha[256];

static int          prev_alpha = -1;

int
TransparencyEnabled(void)
{
   return Conf.trans.alpha;
}

int
TransparencyUpdateNeeded(void)
{
   return Conf.trans.alpha || prev_alpha;
}

static void
TransparencyMakeColorModifier(void)
{
   int                 i;

   for (i = 0; i < 256; i++)
     {
	gray[i] = i;
	alpha[i] = 255 - Conf.trans.alpha;
     }

   if (icm == NULL)
      icm = imlib_create_color_modifier();
   imlib_context_set_color_modifier(icm);
#if 0				/* Useful in this context? */
   imlib_modify_color_modifier_gamma(0.5);
   imlib_modify_color_modifier_brightness(0.5);
   imlib_modify_color_modifier_contrast(0.5);
#endif
   imlib_set_color_modifier_tables(gray, gray, gray, alpha);
   imlib_context_set_color_modifier(NULL);
}

void
TransparencySet(int transparency)
{
   int                 changed;

   if (transparency < 0)
      transparency = 0;
   else if (transparency > 255)
      transparency = 255;

   /*  This will render the initial startup stuff correctly since !changed  */
   if (prev_alpha == -1)
     {
	prev_alpha = Conf.trans.alpha = transparency;
	changed = 1;
     }
   else
     {
	changed = Conf.trans.alpha != transparency;
	prev_alpha = Conf.trans.alpha;
	Conf.trans.alpha = transparency;
     }
   /* Generate the color modifier tables */
   TransparencyMakeColorModifier();
   if (changed)
      ModulesSignal(ESIGNAL_THEME_TRANS_CHANGE, NULL);
}

#endif /* ENABLE_THEME_TRANSPARENCY */

static ImageState  *
ImagestateCreate(void)
{
   ImageState         *is;

   is = Emalloc(sizeof(ImageState));
   if (!is)
      return NULL;

   is->im_file = NULL;
   is->real_file = NULL;
   is->unloadable = 0;
   is->transparent = 0;
   is->im = NULL;
   is->border = NULL;
   is->pixmapfillstyle = FILL_STRETCH;
   ESetColor(&(is->bg), 160, 160, 160);
   ESetColor(&(is->hi), 200, 200, 200);
   ESetColor(&(is->lo), 120, 120, 120);
   ESetColor(&(is->hihi), 255, 255, 255);
   ESetColor(&(is->lolo), 64, 64, 64);
   is->bevelstyle = BEVEL_NONE;
#if ENABLE_COLOR_MODIFIERS
   is->colmod = NULL;
#endif

   return is;
}

static void
FreeImageState(ImageState * i)
{

   Efree(i->im_file);
   Efree(i->real_file);

   if (i->im)
     {
	imlib_context_set_image(i->im);
	imlib_free_image();
	i->im = NULL;
     }

   if (i->border)
      Efree(i->border);

#if ENABLE_COLOR_MODIFIERS
   if (i->colmod)
      i->colmod->ref_count--;
#endif
}

static void
FreeImageStateArray(ImageStateArray * isa)
{
   FreeImageState(isa->normal);
   Efree(isa->normal);
   FreeImageState(isa->hilited);
   Efree(isa->hilited);
   FreeImageState(isa->clicked);
   Efree(isa->clicked);
   FreeImageState(isa->disabled);
   Efree(isa->disabled);
}

static void
ImagestatePopulate(ImageState * is)
{
   if (!is)
      return;

   EAllocColor(&is->bg);
   EAllocColor(&is->hi);
   EAllocColor(&is->lo);
   EAllocColor(&is->hihi);
   EAllocColor(&is->lolo);
}

static void
ImagestateRealize(ImageState * is)
{
   if (is == NULL || is->im_file == NULL)
      return;

   /* has bg pixmap */
   if (is->im)
      return;

   /* not loaded, load and setup */
   if (!is->real_file)
      is->real_file = ThemeFileFind(is->im_file, 0);

   is->im = ELoadImage(is->real_file);
   imlib_context_set_image(is->im);
   if (is->im == NULL)
      Eprintf
	 ("ImagestateRealize: Hmmm... is->im is NULL (im_file=%s real_file=%s\n",
	  is->im_file, is->real_file);

   if (is->border)
      imlib_image_set_border(is->border);

#if 0				/* To be implemented? */
   if (is->colmod)
     {
	Imlib_set_image_red_curve(pImlib_Context, is->im, is->colmod->red.map);
	Imlib_set_image_green_curve(pImlib_Context, is->im,
				    is->colmod->green.map);
	Imlib_set_image_blue_curve(pImlib_Context, is->im,
				   is->colmod->blue.map);
     }
#endif
}

static ImageClass  *
ImageclassCreate(const char *name)
{
   ImageClass         *ic;

   ic = Ecalloc(1, sizeof(ImageClass));
   if (!ic)
      return NULL;

   ic->name = Estrdup(name);
   ic->norm.normal = ic->norm.hilited = ic->norm.clicked = ic->norm.disabled =
      NULL;
   ic->active.normal = ic->active.hilited = ic->active.clicked =
      ic->active.disabled = NULL;
   ic->sticky.normal = ic->sticky.hilited = ic->sticky.clicked =
      ic->sticky.disabled = NULL;
   ic->sticky_active.normal = ic->sticky_active.hilited =
      ic->sticky_active.clicked = ic->sticky_active.disabled = NULL;
   ic->padding.left = 0;
   ic->padding.right = 0;
   ic->padding.top = 0;
   ic->padding.bottom = 0;
#if ENABLE_COLOR_MODIFIERS
   ic->colmod = NULL;
#endif
   ic->ref_count = 0;

   return ic;
}

static void
ImageclassDestroy(ImageClass * ic)
{
   if (!ic)
      return;

   if (ic->ref_count > 0)
     {
	DialogOK(_("Imageclass Error!"), _("%u references remain\n"),
		 ic->ref_count);
	return;
     }
   while (RemoveItemByPtr(ic, LIST_TYPE_ICLASS));

   if (ic->name)
      Efree(ic->name);

   FreeImageStateArray(&(ic->norm));
   FreeImageStateArray(&(ic->active));
   FreeImageStateArray(&(ic->sticky));
   FreeImageStateArray(&(ic->sticky_active));

#if ENABLE_COLOR_MODIFIERS
   if (ic->colmod)
      ic->colmod->ref_count--;
#endif
}

ImageClass         *
ImageclassFind(const char *name, int fallback)
{
   ImageClass         *ic;

   if (name)
     {
	ic = FindItem(name, 0, LIST_FINDBY_NAME, LIST_TYPE_ICLASS);
	if (ic || !fallback)
	   return ic;
     }

   ic = FindItem("__FALLBACK_ICLASS", 0, LIST_FINDBY_NAME, LIST_TYPE_ICLASS);

   return ic;
}

#define ISTATE_SET_STATE(which, fallback) \
   if (ic->which) ImagestatePopulate(ic->which); \
   else ic->which = ic->fallback;

#if ENABLE_COLOR_MODIFIERS
#define ISTATE_SET_CM(which, fallback) \
   if (!ic->which->colmod) { \
      ic->which->colmod = fallback; \
      if (fallback) fallback->ref_count++; \
     }
#endif

static void
ImageclassPopulate(ImageClass * ic)
{
#if ENABLE_COLOR_MODIFIERS
   ColorModifierClass *cm;
#endif

   if (!ic)
      return;

   if (!ic->norm.normal)
      return;

   ImagestatePopulate(ic->norm.normal);
   ISTATE_SET_STATE(norm.hilited, norm.normal);
   ISTATE_SET_STATE(norm.clicked, norm.normal);
   ISTATE_SET_STATE(norm.disabled, norm.normal);

   ISTATE_SET_STATE(active.normal, norm.normal);
   ISTATE_SET_STATE(active.hilited, active.normal);
   ISTATE_SET_STATE(active.clicked, active.normal);
   ISTATE_SET_STATE(active.disabled, active.normal);

   ISTATE_SET_STATE(sticky.normal, norm.normal);
   ISTATE_SET_STATE(sticky.hilited, sticky.normal);
   ISTATE_SET_STATE(sticky.clicked, sticky.normal);
   ISTATE_SET_STATE(sticky.disabled, sticky.normal);

   ISTATE_SET_STATE(sticky_active.normal, norm.normal);
   ISTATE_SET_STATE(sticky_active.hilited, sticky_active.normal);
   ISTATE_SET_STATE(sticky_active.clicked, sticky_active.normal);
   ISTATE_SET_STATE(sticky_active.disabled, sticky_active.normal);

#if ENABLE_COLOR_MODIFIERS
   if (!ic->colmod)
     {
	cm = FindItem("ICLASS", 0, LIST_FINDBY_NAME, LIST_TYPE_COLORMODIFIER);
	if (!cm)
	   cm =
	      FindItem("DEFAULT", 0, LIST_FINDBY_NAME, LIST_TYPE_COLORMODIFIER);
	ic->colmod = cm;
     }

   cm = FindItem("NORMAL", 0, LIST_FINDBY_NAME, LIST_TYPE_COLORMODIFIER);
   if (!cm)
      cm = ic->colmod;

   ISTATE_SET_CM(norm.normal, cm);
   ISTATE_SET_CM(norm.hilited, cm);
   ISTATE_SET_CM(norm.clicked, cm);
   ISTATE_SET_CM(norm.disabled, cm);

   cm = FindItem("ACTIVE", 0, LIST_FINDBY_NAME, LIST_TYPE_COLORMODIFIER);
   if (!cm)
      cm = ic->colmod;

   ISTATE_SET_CM(active.normal, cm);
   ISTATE_SET_CM(active.hilited, cm);
   ISTATE_SET_CM(active.clicked, cm);
   ISTATE_SET_CM(active.disabled, cm);

   cm = FindItem("STICKY", 0, LIST_FINDBY_NAME, LIST_TYPE_COLORMODIFIER);
   if (!cm)
      cm = ic->colmod;

   ISTATE_SET_CM(sticky.normal, cm);
   ISTATE_SET_CM(sticky.hilited, cm);
   ISTATE_SET_CM(sticky.clicked, cm);
   ISTATE_SET_CM(sticky.disabled, cm);

   cm = FindItem("STICKY_ACTIVE", 0, LIST_FINDBY_NAME, LIST_TYPE_COLORMODIFIER);
   if (!cm)
      cm = ic->colmod;

   ISTATE_SET_CM(sticky_active.normal, cm);
   ISTATE_SET_CM(sticky_active.hilited, cm);
   ISTATE_SET_CM(sticky_active.clicked, cm);
   ISTATE_SET_CM(sticky_active.disabled, cm);
#endif
}

int
ImageclassConfigLoad(FILE * fs)
{
   int                 err = 0;
   char                s[FILEPATH_LEN_MAX];
   char                s2[FILEPATH_LEN_MAX];
   int                 i1;
   ImageClass         *ic = NULL;
   ImageState         *ICToRead = NULL;
   int                 fields;
   int                 l, r, t, b;

#if ENABLE_COLOR_MODIFIERS
   ColorModifierClass *cm = NULL;
#endif

   while (GetLine(s, sizeof(s), fs))
     {
	s2[0] = 0;
	i1 = CONFIG_INVALID;
	fields = sscanf(s, "%i %4000s", &i1, s2);

	if (fields < 1)
	   i1 = CONFIG_INVALID;
	else if (i1 == CONFIG_CLOSE)
	  {
	     if (fields != 1)
		Alert(_("CONFIG: ignoring extra data in \"%s\"\n"), s);
	  }
	else if (i1 != CONFIG_INVALID)
	  {
	     if (fields != 2)
		Alert(_("CONFIG: missing required data in \"%s\"\n"), s);
	  }

	switch (i1)
	  {
	  case CONFIG_CLOSE:
	     ImageclassPopulate(ic);
	     AddItem(ic, ic->name, 0, LIST_TYPE_ICLASS);
	     goto done;
	  case ICLASS_LRTB:
	     {
		ICToRead->border = Emalloc(sizeof(Imlib_Border));

		l = r = t = b = 0;
		sscanf(s, "%*s %i %i %i %i", &l, &r, &t, &b);
		ICToRead->border->left = l;
		ICToRead->border->right = r;
		ICToRead->border->top = t;
		ICToRead->border->bottom = b;
		/* Hmmm... imlib2 works better with this */
		ICToRead->border->right++;
		ICToRead->border->bottom++;
	     }
	     break;
	  case ICLASS_FILLRULE:
	     ICToRead->pixmapfillstyle = atoi(s2);
	     break;
	  case ICLASS_TRANSPARENT:
	     ICToRead->transparent = strtoul(s2, NULL, 0);
	     break;
	  case CONFIG_INHERIT:
	     {
		ImageClass         *ICToInherit;

		ICToInherit = ImageclassFind(s2, 0);
		ic->norm = ICToInherit->norm;
		ic->active = ICToInherit->active;
		ic->sticky = ICToInherit->sticky;
		ic->sticky_active = ICToInherit->sticky_active;
		ic->padding = ICToInherit->padding;
#if ENABLE_COLOR_MODIFIERS
		ic->colmod = ICToInherit->colmod;
#endif
	     }
	     break;
	  case CONFIG_COLORMOD:
	  case ICLASS_COLORMOD:
#if ENABLE_COLOR_MODIFIERS
	     cm = FindItem(s2, 0, LIST_FINDBY_NAME, LIST_TYPE_COLORMODIFIER);
	     if (cm)
	       {
		  if (ICToRead)
		    {
		       ICToRead->colmod = cm;
		    }
		  else
		    {
		       ic->colmod = cm;
		    }
		  cm->ref_count++;
	       }
#endif
	     break;
	  case ICLASS_PADDING:
	     {
		l = r = t = b = 0;
		sscanf(s, "%*s %i %i %i %i", &l, &r, &t, &b);
		ic->padding.left = l;
		ic->padding.right = r;
		ic->padding.top = t;
		ic->padding.bottom = b;
	     }
	     break;
	  case CONFIG_CLASSNAME:
	  case ICLASS_NAME:
	     if (ConfigSkipIfExists(fs, s2, LIST_TYPE_ICLASS))
		goto done;
	     ic = ImageclassCreate(s2);
	     break;
	  case CONFIG_DESKTOP:
	     /* don't ask... --mandrake */
	  case ICLASS_NORMAL:
	     ic->norm.normal = ImagestateCreate();
	     ic->norm.normal->im_file = Estrdup(s2);
	     ICToRead = ic->norm.normal;
	     break;
	  case ICLASS_CLICKED:
	     ic->norm.clicked = ImagestateCreate();
	     ic->norm.clicked->im_file = Estrdup(s2);
	     ICToRead = ic->norm.clicked;
	     break;
	  case ICLASS_HILITED:
	     ic->norm.hilited = ImagestateCreate();
	     ic->norm.hilited->im_file = Estrdup(s2);
	     ICToRead = ic->norm.hilited;
	     break;
	  case ICLASS_DISABLED:
	     ic->norm.disabled = ImagestateCreate();
	     ic->norm.disabled->im_file = Estrdup(s2);
	     ICToRead = ic->norm.disabled;
	     break;
	  case ICLASS_STICKY_NORMAL:
	     ic->sticky.normal = ImagestateCreate();
	     ic->sticky.normal->im_file = Estrdup(s2);
	     ICToRead = ic->sticky.normal;
	     break;
	  case ICLASS_STICKY_CLICKED:
	     ic->sticky.clicked = ImagestateCreate();
	     ic->sticky.clicked->im_file = Estrdup(s2);
	     ICToRead = ic->sticky.clicked;
	     break;
	  case ICLASS_STICKY_HILITED:
	     ic->sticky.hilited = ImagestateCreate();
	     ic->sticky.hilited->im_file = Estrdup(s2);
	     ICToRead = ic->sticky.hilited;
	     break;
	  case ICLASS_STICKY_DISABLED:
	     ic->sticky.disabled = ImagestateCreate();
	     ic->sticky.disabled->im_file = Estrdup(s2);
	     ICToRead = ic->sticky.disabled;
	     break;
	  case ICLASS_ACTIVE_NORMAL:
	     ic->active.normal = ImagestateCreate();
	     ic->active.normal->im_file = Estrdup(s2);
	     ICToRead = ic->active.normal;
	     break;
	  case ICLASS_ACTIVE_CLICKED:
	     ic->active.clicked = ImagestateCreate();
	     ic->active.clicked->im_file = Estrdup(s2);
	     ICToRead = ic->active.clicked;
	     break;
	  case ICLASS_ACTIVE_HILITED:
	     ic->active.hilited = ImagestateCreate();
	     ic->active.hilited->im_file = Estrdup(s2);
	     ICToRead = ic->active.hilited;
	     break;
	  case ICLASS_ACTIVE_DISABLED:
	     ic->active.disabled = ImagestateCreate();
	     ic->active.disabled->im_file = Estrdup(s2);
	     ICToRead = ic->active.disabled;
	     break;
	  case ICLASS_STICKY_ACTIVE_NORMAL:
	     ic->sticky_active.normal = ImagestateCreate();
	     ic->sticky_active.normal->im_file = Estrdup(s2);
	     ICToRead = ic->sticky_active.normal;
	     break;
	  case ICLASS_STICKY_ACTIVE_CLICKED:
	     ic->sticky_active.clicked = ImagestateCreate();
	     ic->sticky_active.clicked->im_file = Estrdup(s2);
	     ICToRead = ic->sticky_active.clicked;
	     break;
	  case ICLASS_STICKY_ACTIVE_HILITED:
	     ic->sticky_active.hilited = ImagestateCreate();
	     ic->sticky_active.hilited->im_file = Estrdup(s2);
	     ICToRead = ic->sticky_active.hilited;
	     break;
	  case ICLASS_STICKY_ACTIVE_DISABLED:
	     ic->sticky_active.disabled = ImagestateCreate();
	     ic->sticky_active.disabled->im_file = Estrdup(s2);
	     ICToRead = ic->sticky_active.disabled;
	     break;
	  default:
	     Alert(_("Warning: unable to determine what to do with\n"
		     "the following text in the middle of current "
		     "ImageClass definition:\n"
		     "%s\nWill ignore and continue...\n"), s);
	     break;
	  }
     }
   err = -1;

 done:
   return err;
}

ImageClass         *
ImageclassCreateSimple(const char *name, const char *image)
{
   ImageClass         *ic;

   ic = ImageclassCreate(name);
   ic->norm.normal = ImagestateCreate();
   ic->norm.normal->im_file = Estrdup(image);
   ic->norm.normal->unloadable = 1;
   ImageclassPopulate(ic);
   AddItem(ic, ic->name, 0, LIST_TYPE_ICLASS);

   ImagestateRealize(ic->norm.normal);

   return ic;
}

#ifdef ENABLE_THEME_TRANSPARENCY
int
ImageclassIsTransparent(ImageClass * ic)
{
   return ic && ic->norm.normal && ic->norm.normal->transparent;
}
#endif

static ImageState  *
ImageclassGetImageState1(ImageStateArray * pisa, int state)
{
   ImageState         *is;

   switch (state)
     {
     case STATE_NORMAL:
	is = pisa->normal;
	break;
     case STATE_HILITED:
	is = pisa->hilited;
	break;
     case STATE_CLICKED:
	is = pisa->clicked;
	break;
     case STATE_DISABLED:
	is = pisa->disabled;
	break;
     default:
	is = NULL;
	break;
     }

   return is;
}

ImageState         *
ImageclassGetImageState(ImageClass * ic, int state, int active, int sticky)
{
   ImageState         *is;

   if (active)
     {
	if (sticky)
	   is = ImageclassGetImageState1(&ic->sticky_active, state);
	else
	   is = ImageclassGetImageState1(&ic->active, state);
     }
   else
     {
	if (sticky)
	   is = ImageclassGetImageState1(&ic->sticky, state);
	else
	   is = ImageclassGetImageState1(&ic->norm, state);
     }

   return is;
}

Imlib_Image        *
ImageclassGetImage(ImageClass * ic, int active, int sticky, int state)
{
   Imlib_Image        *im;
   ImageState         *is;

   if (!ic)
      return NULL;

   is = ImageclassGetImageState(ic, state, active, sticky);
   if (!is)
      return NULL;

   if (is->im == NULL && is->im_file)
      ImagestateRealize(is);

   im = is->im;
   if (!im)
      return NULL;
   is->im = NULL;

   return im;
}

static void
ImagestateMakePmapMask(ImageState * is, Drawable win, PmapMask * pmm,
		       int make_mask, int w, int h, int image_type)
{
   int                 trans;
   int                 ww, hh;

#ifdef ENABLE_TRANSPARENCY
   Imlib_Image        *ii = NULL;
   int                 flags;

   flags = ICLASS_ATTR_OPAQUE;
   if (Conf.trans.alpha > 0)
     {
	switch (image_type)
	  {
	  default:
	  case ST_UNKNWN:
	  case ST_BUTTON:
	     flags = ICLASS_ATTR_OPAQUE;
	     break;
	  case ST_BORDER:
	     flags = Conf.trans.border;
	     break;
	  case ST_WIDGET:
	     flags = Conf.trans.widget;
	     break;
	  case ST_ICONBOX:
	     flags = Conf.trans.iconbox;
	     break;
	  case ST_MENU:
	     flags = Conf.trans.menu;
	     break;
	  case ST_MENU_ITEM:
	     flags = Conf.trans.menu_item;
	     break;
	  case ST_TOOLTIP:
	     flags = Conf.trans.tooltip;
	     break;
	  case ST_DIALOG:
	     flags = Conf.trans.dialog;
	     break;
	  case ST_HILIGHT:
	     flags = Conf.trans.hilight;
	     break;
	  case ST_PAGER:
	     flags = Conf.trans.pager;
	     break;
	  case ST_WARPLIST:
	     flags = Conf.trans.warplist;
	     break;
	  }
     }
   if (flags != ICLASS_ATTR_OPAQUE)
      flags |= ICLASS_ATTR_USE_CM;
#endif

   imlib_context_set_drawable(win);
   imlib_context_set_image(is->im);

   ww = imlib_image_get_width();
   hh = imlib_image_get_height();

   pmm->type = 1;
   pmm->pmap = pmm->mask = 0;

#ifdef ENABLE_TRANSPARENCY
   /*
    * is->transparent flags:
    *   0x01: Use desktop background pixmap as base
    *   0x02: Use root window as base (use only for transients, if at all)
    *   0x04: Don't apply image mask to result
    */
   if (is->transparent && imlib_image_has_alpha())
      flags = is->transparent;

   trans = (flags != ICLASS_ATTR_OPAQUE);

   if (trans)
     {
	Window              cr, dummy;
	Drawable            bg;
	int                 xx, yy;

	bg = BackgroundGetPixmap(DeskGetBackground(DesksGetCurrent()));
	if ((flags & ICLASS_ATTR_GLASS) || (bg == None))
	  {
	     cr = VRoot.win;
	     bg = VRoot.win;
	  }
	else
	  {
	     cr = DeskGetCurrentRoot();
	  }
	XTranslateCoordinates(disp, win, cr, 0, 0, &xx, &yy, &dummy);
#if 0
	Eprintf("ImagestateMakePmapMask %#lx %d %d %d %d\n", win, xx, yy, w, h);
#endif
	if (xx < VRoot.w && yy < VRoot.h && xx + w >= 0 && yy + h >= 0)
	  {
	     /* Create the background base image */
	     imlib_context_set_drawable(bg);
	     ii = imlib_create_image_from_drawable(0, xx, yy, w, h,
						   !EServerIsGrabbed());
	     imlib_context_set_image(ii);
	     imlib_context_set_drawable(win);
	  }
     }
   else
     {
#if 0
	Eprintf("ImagestateMakePmapMask %#lx %d %d\n", win, w, h);
#endif
     }
#else
   trans = 0;
#endif

   if (is->pixmapfillstyle == FILL_STRETCH || trans)
     {
#ifdef ENABLE_TRANSPARENCY
	if (ii)
	  {
	     imlib_context_set_blend(1);
#ifdef ENABLE_THEME_TRANSPARENCY
	     if (flags & ICLASS_ATTR_USE_CM)
	       {
		  imlib_context_set_color_modifier(icm);
	       }
#endif
	     imlib_context_set_operation(IMLIB_OP_COPY);
	     imlib_blend_image_onto_image(is->im, 0, 0, 0, ww, hh, 0, 0, w, h);
	     imlib_context_set_blend(0);
#ifdef ENABLE_THEME_TRANSPARENCY
	     if (flags & ICLASS_ATTR_USE_CM)
	       {
		  imlib_context_set_color_modifier(NULL);
	       }
#if 0				/* Do we ever need to free it? */
	     imlib_free_color_modifier();
#endif
#endif
	  }

	pmm->type = 1;
	imlib_render_pixmaps_for_whole_image_at_size(&pmm->pmap, &pmm->mask,
						     w, h);
	if (ii && make_mask && !(flags & ICLASS_ATTR_NO_CLIP))
	  {
	     Pixmap              pmap = 0, mask = 0;
	     GC                  gc;

	     imlib_context_set_image(is->im);
	     if (imlib_image_has_alpha())
	       {
		  /* Due to the blending the mask will always be 0 here */

		  /* Make the scaled clip mask to be used */
		  imlib_render_pixmaps_for_whole_image_at_size(&pmap, &mask, w,
							       h);

		  /* And now some uglyness to make a single "Imlib2 pixmap/mask" thing */

		  /* Replace the pmap with the previously blended one */
		  gc = ECreateGC(pmm->pmap, 0, NULL);
		  XCopyArea(disp, pmm->pmap, pmap, gc, 0, 0, w, h, 0, 0);
		  EFreeGC(gc);

		  /* Free the old pixmap without associated mask */
		  imlib_free_pixmap_and_mask(pmm->pmap);

		  /* We now have the copied pixmap with proper mask */
		  pmm->pmap = pmap;
		  pmm->mask = mask;
	       }
	  }
#else
	pmm->type = 1;
	imlib_render_pixmaps_for_whole_image_at_size(&pmm->pmap, &pmm->mask,
						     w, h);
#endif /* ENABLE_TRANSPARENCY */
	pmm->w = w;
	pmm->h = h;
     }
   else
     {
	int                 cw, ch, pw, ph;

	pw = w;
	ph = h;
	if (is->pixmapfillstyle & FILL_TILE_H)
	   pw = ww;
	if (is->pixmapfillstyle & FILL_TILE_V)
	   ph = hh;
	if (is->pixmapfillstyle & FILL_INT_TILE_H)
	  {
	     cw = w / ww;
	     if (cw * ww < w)
		cw++;
	     if (cw < 1)
		cw = 1;
	     pw = w / cw;
	  }
	if (is->pixmapfillstyle & FILL_INT_TILE_V)
	  {
	     ch = h / hh;
	     if (ch * hh < h)
		ch++;
	     if (ch < 1)
		ch = 1;
	     ph = h / ch;
	  }
	imlib_render_pixmaps_for_whole_image_at_size(&pmm->pmap, &pmm->mask,
						     pw, ph);
	pmm->w = pw;
	pmm->h = ph;
     }

#ifdef ENABLE_TRANSPARENCY
   if (ii)
     {
	imlib_context_set_image(ii);
	imlib_free_image();
     }
#else
   make_mask = 0;
   image_type = 0;
#endif
}

static void
ImagestateDrawBevel(ImageState * is, Drawable win, GC gc, int w, int h)
{
   switch (is->bevelstyle)
     {
     case BEVEL_AMIGA:
	XSetForeground(disp, gc, is->hihi.pixel);
	XDrawLine(disp, win, gc, 0, 0, w - 2, 0);
	XDrawLine(disp, win, gc, 0, 0, 0, h - 2);
	XSetForeground(disp, gc, is->lolo.pixel);
	XDrawLine(disp, win, gc, 1, h - 1, w - 1, h - 1);
	XDrawLine(disp, win, gc, w - 1, 1, w - 1, h - 1);
	break;
     case BEVEL_MOTIF:
	XSetForeground(disp, gc, is->hi.pixel);
	XDrawLine(disp, win, gc, 0, 0, w - 1, 0);
	XDrawLine(disp, win, gc, 0, 0, 0, h - 1);
	XDrawLine(disp, win, gc, 1, 1, w - 2, 1);
	XDrawLine(disp, win, gc, 1, 1, 1, h - 2);
	XSetForeground(disp, gc, is->lo.pixel);
	XDrawLine(disp, win, gc, 0, h - 1, w - 1, h - 1);
	XDrawLine(disp, win, gc, w - 1, 1, w - 1, h - 1);
	XDrawLine(disp, win, gc, 1, h - 2, w - 2, h - 2);
	XDrawLine(disp, win, gc, w - 2, 2, w - 2, h - 2);
	break;
     case BEVEL_NEXT:
	XSetForeground(disp, gc, is->hihi.pixel);
	XDrawLine(disp, win, gc, 0, 0, w - 1, 0);
	XDrawLine(disp, win, gc, 0, 0, 0, h - 1);
	XSetForeground(disp, gc, is->hi.pixel);
	XDrawLine(disp, win, gc, 1, 1, w - 2, 1);
	XDrawLine(disp, win, gc, 1, 1, 1, h - 2);
	XSetForeground(disp, gc, is->lolo.pixel);
	XDrawLine(disp, win, gc, 1, h - 1, w - 1, h - 1);
	XDrawLine(disp, win, gc, w - 1, 1, w - 1, h - 1);
	XSetForeground(disp, gc, is->lo.pixel);
	XDrawLine(disp, win, gc, 2, h - 2, w - 2, h - 2);
	XDrawLine(disp, win, gc, w - 2, 2, w - 2, h - 2);
	break;
     case BEVEL_DOUBLE:
	XSetForeground(disp, gc, is->hi.pixel);
	XDrawLine(disp, win, gc, 0, 0, w - 2, 0);
	XDrawLine(disp, win, gc, 0, 0, 0, h - 2);
	XSetForeground(disp, gc, is->lo.pixel);
	XDrawLine(disp, win, gc, 1, 1, w - 3, 1);
	XDrawLine(disp, win, gc, 1, 1, 1, h - 3);
	XSetForeground(disp, gc, is->lo.pixel);
	XDrawLine(disp, win, gc, 1, h - 1, w - 1, h - 1);
	XDrawLine(disp, win, gc, w - 1, 1, w - 1, h - 1);
	XSetForeground(disp, gc, is->hi.pixel);
	XDrawLine(disp, win, gc, 2, h - 2, w - 2, h - 2);
	XDrawLine(disp, win, gc, w - 2, 2, w - 2, h - 2);
	break;
     case BEVEL_WIDEDOUBLE:
	XSetForeground(disp, gc, is->hihi.pixel);
	XDrawLine(disp, win, gc, 0, 0, w - 1, 0);
	XDrawLine(disp, win, gc, 0, 0, 0, h - 1);
	XSetForeground(disp, gc, is->hi.pixel);
	XDrawLine(disp, win, gc, 1, 1, w - 2, 1);
	XDrawLine(disp, win, gc, 1, 1, 1, h - 2);
	XDrawLine(disp, win, gc, 3, h - 4, w - 4, h - 4);
	XDrawLine(disp, win, gc, w - 4, 3, w - 4, h - 4);
	XSetForeground(disp, gc, is->lolo.pixel);
	XDrawLine(disp, win, gc, 1, h - 1, w - 1, h - 1);
	XDrawLine(disp, win, gc, w - 1, 1, w - 1, h - 1);
	XSetForeground(disp, gc, is->lo.pixel);
	XDrawLine(disp, win, gc, 2, h - 2, w - 2, h - 2);
	XDrawLine(disp, win, gc, w - 2, 2, w - 2, h - 2);
	XDrawLine(disp, win, gc, 3, 3, w - 4, 3);
	XDrawLine(disp, win, gc, 3, 3, 3, h - 4);
	break;
     case BEVEL_THINPOINT:
	XSetForeground(disp, gc, is->hi.pixel);
	XDrawLine(disp, win, gc, 0, 0, w - 2, 0);
	XDrawLine(disp, win, gc, 0, 0, 0, h - 2);
	XSetForeground(disp, gc, is->lo.pixel);
	XDrawLine(disp, win, gc, 1, h - 1, w - 1, h - 1);
	XDrawLine(disp, win, gc, w - 1, 1, w - 1, h - 1);
	XSetForeground(disp, gc, is->hihi.pixel);
	XDrawLine(disp, win, gc, 0, 0, 1, 0);
	XDrawLine(disp, win, gc, 0, 0, 0, 1);
	XSetForeground(disp, gc, is->lolo.pixel);
	XDrawLine(disp, win, gc, w - 2, h - 1, w - 1, h - 1);
	XDrawLine(disp, win, gc, w - 1, h - 2, w - 1, h - 1);
	break;
     case BEVEL_THICKPOINT:
	XSetForeground(disp, gc, is->hi.pixel);
	XDrawRectangle(disp, win, gc, 0, 0, w - 1, h - 1);
	break;
     default:
	break;
     }
}

void
ITApply(Window win, ImageClass * ic, ImageState * is, int w, int h, int state,
	int active, int sticky, char expose, int image_type, TextClass * tc,
	TextState * ts, const char *text)
{
   if (win == None || !ic)
      return;

   /* FIXME - Why? */
   if (w <= 0 || h <= 0)
      EGetGeometry(win, NULL, NULL, NULL, &w, &h, NULL, NULL);
   if (w <= 0 || h <= 0)
      return;

   if (!is)
      is = ImageclassGetImageState(ic, state, active, sticky);
   if (!is)
      return;

   if (tc && text)
     {
	if (!ts)
	   ts = TextclassGetTextState(tc, state, active, sticky);
     }

   if (!expose)			/* FIXME - Hmmm */
     {
	if (is->im == NULL && is->im_file)
	   ImagestateRealize(is);

	/* Imlib2 will not render pixmaps with dimensions > 8192 */
	if (is->im && w <= 8192 && h <= 8192)
	  {
	     PmapMask            pmm;
	     int                 decache = 1;

	     ImagestateMakePmapMask(is, win, &pmm, 1, w, h, image_type);

	     if (pmm.pmap)
	       {
		  if (ts && text)
		    {
		       TextstateDrawText(ts, pmm.pmap, text, ic->padding.left,
					 ic->padding.top,
					 w - (ic->padding.left +
					      ic->padding.right),
					 h - (ic->padding.top +
					      ic->padding.bottom),
					 0, tc->justification);
		       decache = 1;
		    }

		  /* Set window pixmap */
		  if (pmm.w == w && pmm.h == h)
		    {
		       ESetWindowBackgroundPixmap(win, pmm.pmap);
		       EShapeCombineMask(win, ShapeBounding, 0, 0,
					 pmm.mask, ShapeSet);
		    }
		  else
		    {
		       /* Tiled */
		       ESetWindowBackgroundPixmap(win, pmm.pmap);
		       if (pmm.mask)
			  EShapeCombineMaskTiled(win, ShapeBounding, 0, 0,
						 pmm.mask, ShapeSet, w, h);
		    }
	       }

	     FreePmapMask(&pmm);
	     EClearWindow(win);

	     if ((is->unloadable) || (Conf.memory_paranoia))
	       {
		  imlib_context_set_image(is->im);
		  if (decache)
		     imlib_free_image_and_decache();
		  else
		     imlib_free_image();
		  is->im = NULL;
	       }
	  }
	else
	  {
	     /* FIXME - No text */
	     ESetWindowBackground(win, is->bg.pixel);
	     EClearWindow(win);
	  }
     }

   if (is->bevelstyle != BEVEL_NONE)
     {
	GC                  gc;

	gc = ECreateGC(win, 0, NULL);
	ImagestateDrawBevel(is, win, gc, w, h);
	EFreeGC(gc);
     }
}

void
ImageclassApply(ImageClass * ic, Window win, int w, int h, int active,
		int sticky, int state, char expose, int image_type)
{
   ITApply(win, ic, NULL, w, h, state, active, sticky, expose, image_type,
	   NULL, NULL, NULL);
}

void
ImageclassApplyCopy(ImageClass * ic, Window win, int w, int h, int active,
		    int sticky, int state, PmapMask * pmm, int make_mask,
		    int image_type)
{
   ImageState         *is;
   GC                  gc;

   if (pmm == NULL)
      return;

   pmm->type = 0;
   pmm->pmap = pmm->mask = 0;

   if ((!ic) || (!win) || (w <= 0) || (h <= 0))
      return;

   is = ImageclassGetImageState(ic, state, active, sticky);
   if (!is)
      return;

   if (is->im == NULL && is->im_file)
      ImagestateRealize(is);

   /* Imlib2 will not render pixmaps with dimensions > 8192 */
   if (is->im && w <= 8192 && h <= 8192)
     {
	ImagestateMakePmapMask(is, win, pmm, make_mask, w, h, image_type);

	if (pmm->pmap)
	  {
	     if (pmm->w != w || pmm->h != h)
	       {
		  /* Create new full sized pixmaps and fill them with the */
		  /* pmap and mask tiles                                  */
		  Pixmap              tp = 0, tm = 0;
		  XGCValues           gcv;

		  tp = ECreatePixmap(win, w, h, VRoot.depth);
		  gcv.fill_style = FillTiled;
		  gcv.tile = pmm->pmap;
		  gcv.ts_x_origin = 0;
		  gcv.ts_y_origin = 0;
		  gc = ECreateGC(tp, GCFillStyle | GCTile |
				 GCTileStipXOrigin | GCTileStipYOrigin, &gcv);
		  XFillRectangle(disp, tp, gc, 0, 0, w, h);
		  EFreeGC(gc);
		  if (pmm->mask)
		    {
		       tm = ECreatePixmap(win, w, h, 1);
		       gcv.fill_style = FillTiled;
		       gcv.tile = pmm->mask;
		       gcv.ts_x_origin = 0;
		       gcv.ts_y_origin = 0;
		       gc = ECreateGC(tm, GCFillStyle | GCTile |
				      GCTileStipXOrigin | GCTileStipYOrigin,
				      &gcv);
		       XFillRectangle(disp, tm, gc, 0, 0, w, h);
		       EFreeGC(gc);
		    }
		  FreePmapMask(pmm);
		  pmm->type = 0;
		  pmm->pmap = tp;
		  pmm->mask = tm;
	       }
	  }

	if ((is->unloadable) || (Conf.memory_paranoia))
	  {
	     imlib_context_set_image(is->im);
	     imlib_free_image();
	     is->im = NULL;
	  }
     }
   else
     {
	Pixmap              pmap;

	if (pmm->pmap)
	   Eprintf("ImageclassApplyCopy: Hmm... pmm->pmap already set\n");

	pmap = ECreatePixmap(win, w, h, VRoot.depth);
	pmm->type = 0;
	pmm->pmap = pmap;
	pmm->mask = 0;

	gc = ECreateGC(pmap, 0, NULL);
	/* bg color */
	XSetForeground(disp, gc, is->bg.pixel);
	XFillRectangle(disp, pmap, gc, 0, 0, w, h);
	/* if there is a bevel to draw, draw it */
	if (is->bevelstyle != BEVEL_NONE)
	   ImagestateDrawBevel(is, pmap, gc, w, h);
	EFreeGC(gc);
	/* FIXME - No text */
     }
}

/*
 */
void
FreePmapMask(PmapMask * pmm)
{
   /* type !=0: Created by imlib_render_pixmaps_for_whole_image... */
   if (pmm->pmap)
     {
	if (pmm->type == 0)
	   EFreePixmap(pmm->pmap);
	else
	   imlib_free_pixmap_and_mask(pmm->pmap);
	pmm->pmap = 0;
     }

   if (pmm->mask)
     {
	if (pmm->type == 0)
	   EFreePixmap(pmm->mask);
	pmm->mask = 0;
     }
}

static void
ImageclassSetupFallback(void)
{
   ImageClass         *ic;

   /* create a fallback imageclass in case no imageclass can be found */
   ic = ImageclassCreate("__FALLBACK_ICLASS");

   ic->norm.normal = ImagestateCreate();
   ESetColor(&(ic->norm.normal->hihi), 255, 255, 255);
   ESetColor(&(ic->norm.normal->hi), 255, 255, 255);
   ESetColor(&(ic->norm.normal->bg), 160, 160, 160);
   ESetColor(&(ic->norm.normal->lo), 0, 0, 0);
   ESetColor(&(ic->norm.normal->lolo), 0, 0, 0);
   ic->norm.normal->bevelstyle = BEVEL_AMIGA;

   ic->norm.hilited = ImagestateCreate();
   ESetColor(&(ic->norm.hilited->hihi), 255, 255, 255);
   ESetColor(&(ic->norm.hilited->hi), 255, 255, 255);
   ESetColor(&(ic->norm.hilited->bg), 192, 192, 192);
   ESetColor(&(ic->norm.hilited->lo), 0, 0, 0);
   ESetColor(&(ic->norm.hilited->lolo), 0, 0, 0);
   ic->norm.hilited->bevelstyle = BEVEL_AMIGA;

   ic->norm.clicked = ImagestateCreate();
   ESetColor(&(ic->norm.clicked->hihi), 0, 0, 0);
   ESetColor(&(ic->norm.clicked->hi), 0, 0, 0);
   ESetColor(&(ic->norm.clicked->bg), 192, 192, 192);
   ESetColor(&(ic->norm.clicked->lo), 255, 255, 255);
   ESetColor(&(ic->norm.clicked->lolo), 255, 255, 255);
   ic->norm.clicked->bevelstyle = BEVEL_AMIGA;

   ic->active.normal = ImagestateCreate();
   ESetColor(&(ic->active.normal->hihi), 255, 255, 255);
   ESetColor(&(ic->active.normal->hi), 255, 255, 255);
   ESetColor(&(ic->active.normal->bg), 180, 140, 160);
   ESetColor(&(ic->active.normal->lo), 0, 0, 0);
   ESetColor(&(ic->active.normal->lolo), 0, 0, 0);
   ic->active.normal->bevelstyle = BEVEL_AMIGA;

   ic->active.hilited = ImagestateCreate();
   ESetColor(&(ic->active.hilited->hihi), 255, 255, 255);
   ESetColor(&(ic->active.hilited->hi), 255, 255, 255);
   ESetColor(&(ic->active.hilited->bg), 230, 190, 210);
   ESetColor(&(ic->active.hilited->lo), 0, 0, 0);
   ESetColor(&(ic->active.hilited->lolo), 0, 0, 0);
   ic->active.hilited->bevelstyle = BEVEL_AMIGA;

   ic->active.clicked = ImagestateCreate();
   ESetColor(&(ic->active.clicked->hihi), 0, 0, 0);
   ESetColor(&(ic->active.clicked->hi), 0, 0, 0);
   ESetColor(&(ic->active.clicked->bg), 230, 190, 210);
   ESetColor(&(ic->active.clicked->lo), 255, 255, 255);
   ESetColor(&(ic->active.clicked->lolo), 255, 255, 255);
   ic->active.clicked->bevelstyle = BEVEL_AMIGA;

   ic->padding.left = 8;
   ic->padding.right = 8;
   ic->padding.top = 8;
   ic->padding.bottom = 8;

   ImageclassPopulate(ic);
   AddItem(ic, ic->name, 0, LIST_TYPE_ICLASS);

   /* Create all black image class for filler borders */
   ic = ImageclassCreate("__BLACK");

   ic->norm.normal = ImagestateCreate();
   ESetColor(&(ic->norm.normal->hihi), 0, 0, 0);
   ESetColor(&(ic->norm.normal->hi), 0, 0, 0);
   ESetColor(&(ic->norm.normal->bg), 0, 0, 0);
   ESetColor(&(ic->norm.normal->lo), 0, 0, 0);
   ESetColor(&(ic->norm.normal->lolo), 0, 0, 0);

   ImageclassPopulate(ic);
   AddItem(ic, ic->name, 0, LIST_TYPE_ICLASS);
}

/*
 * Imageclass Module
 */

static void
ImageclassSighan(int sig, void *prm __UNUSED__)
{
   switch (sig)
     {
     case ESIGNAL_INIT:
	ImageclassSetupFallback();
	break;
     }
}

static void
ImageclassIpc(const char *params, Client * c __UNUSED__)
{
   char                param1[FILEPATH_LEN_MAX];
   char                param2[FILEPATH_LEN_MAX];
   char                param3[FILEPATH_LEN_MAX];
   ImageClass         *ic;

   if (!params)
     {
	IpcPrintf("Please specify...\n");
	return;
     }

   param1[0] = 0;
   param2[0] = 0;
   param3[0] = 0;

   word(params, 1, param1);
   word(params, 2, param2);

   if (!strncmp(param1, "list", 2))
     {
	ImageClass        **lst;
	int                 num, i;

	lst = (ImageClass **) ListItemType(&num, LIST_TYPE_ICLASS);
	for (i = 0; i < num; i++)
	   IpcPrintf("%s\n", lst[i]->name);
	if (lst)
	   Efree(lst);
	return;
     }

   if (!strcmp(param2, "create"))
     {
     }
   else if (!strcmp(param2, "delete"))
     {
	ic = ImageclassFind(param1, 0);
	if (ic)
	   ImageclassDestroy(ic);
     }
   else if (!strcmp(param2, "modify"))
     {
     }
   else if (!strcmp(param2, "free_pixmap"))
     {
	Pixmap              p;

	word(params, 3, param3);
	p = (Pixmap) strtol(param3, (char **)NULL, 0);
	imlib_free_pixmap_and_mask(p);
     }
   else if (!strcmp(param2, "get_padding"))
     {
	ic = ImageclassFind(param1, 0);
	if (ic)
	   IpcPrintf("%i %i %i %i\n",
		     ic->padding.left, ic->padding.right,
		     ic->padding.top, ic->padding.bottom);
	else
	   IpcPrintf("Error: Imageclass does not exist\n");
     }
   else if (!strcmp(param2, "get_image_size"))
     {
	ic = ImageclassFind(param1, 0);
	if (ic)
	  {
	     ImagestateRealize(ic->norm.normal);
	     if (ic->norm.normal->im)
	       {
		  imlib_context_set_image(ic->norm.normal->im);
		  IpcPrintf("%i %i\n", imlib_image_get_width(),
			    imlib_image_get_height());
		  imlib_free_image();
	       }
	  }
	else
	   IpcPrintf("Error: Imageclass does not exist\n");
     }
   else if (!strcmp(param2, "apply"))
     {
	ic = ImageclassFind(param1, 0);
	if (ic)
	  {
	     Window              win;
	     char                state[20];
	     const char         *winptr, *hptr;
	     int                 st, w = -1, h = -1;

	     winptr = atword(params, 3);
	     word(params, 4, state);
	     win = (Window) strtol(winptr, (char **)NULL, 0);
	     if (!strcmp(state, "hilited"))
		st = STATE_HILITED;
	     else if (!strcmp(state, "clicked"))
		st = STATE_CLICKED;
	     else if (!strcmp(state, "disabled"))
		st = STATE_DISABLED;
	     else
		st = STATE_NORMAL;
	     if ((hptr = atword(params, 6)))
	       {
		  w = (int)strtol(atword(params, 5), (char **)NULL, 0);
		  h = (int)strtol(hptr, (char **)NULL, 0);
	       }
	     ImageclassApply(ic, win, w, h, 0, 0, st, 0, ST_UNKNWN);
	  }
     }
   else if (!strcmp(param2, "apply_copy"))
     {
	ic = ImageclassFind(param1, 0);
	if (ic)
	  {
	     Window              win;
	     char                state[20];
	     const char         *winptr, *hptr;
	     int                 st, w = -1, h = -1;

	     winptr = atword(params, 3);
	     word(params, 4, state);
	     win = (Window) strtol(winptr, (char **)NULL, 0);
	     if (!strcmp(state, "hilited"))
		st = STATE_HILITED;
	     else if (!strcmp(state, "clicked"))
		st = STATE_CLICKED;
	     else if (!strcmp(state, "disabled"))
		st = STATE_DISABLED;
	     else
		st = STATE_NORMAL;
	     if (!(hptr = atword(params, 6)))
		IpcPrintf("Error:  missing width and/or height\n");
	     else
	       {
		  PmapMask            pmm;

		  w = (int)strtol(atword(params, 5), (char **)NULL, 0);
		  h = (int)strtol(hptr, (char **)NULL, 0);
		  ImageclassApplyCopy(ic, win, w, h, 0, 0, st,
				      &pmm, 1, ST_UNKNWN);
		  IpcPrintf("0x%08x 0x%08x\n",
			    (unsigned)pmm.pmap, (unsigned)pmm.mask);
/*			    FreePmapMask(&pmm);		??? */
	       }
	  }
     }
   else if (!strcmp(param2, "ref_count"))
     {
	ic = ImageclassFind(param1, 0);
	if (ic)
	   IpcPrintf("%u references remain\n", ic->ref_count);
     }
   else if (!strcmp(param2, "query"))
     {
	ic = ImageclassFind(param1, 0);
	if (ic)
	   IpcPrintf("ImageClass %s found\n", ic->name);
	else
	   IpcPrintf("ImageClass %s not found\n", param1);
     }
   else
     {
	IpcPrintf("Error: unknown operation specified\n");
     }
}

IpcItem             ImageclassIpcArray[] = {
   {
    ImageclassIpc,
    "imageclass", NULL,
    "List imageclasses, create/delete/modify/apply an imageclass",
    NULL}
   ,
};
#define N_IPC_FUNCS (sizeof(ImageclassIpcArray)/sizeof(IpcItem))

/*
 * Module descriptor
 */
EModule             ModImageclass = {
   "imageclass", "ic",
   ImageclassSighan,
   {N_IPC_FUNCS, ImageclassIpcArray}
   ,
   {0, NULL}
};
