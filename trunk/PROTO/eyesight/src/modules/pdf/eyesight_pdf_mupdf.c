/*
 * Eyesight - EFL-based document renderer
 * Copyright (C) 2010 Vincent Torri <vtorri at univ-evry dot fr>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <fitz.h>
#include <mupdf.h>

#include <Evas.h>

#include "Eyesight_Module_Pdf.h"
#include "Eyesight.h"
#include "eyesight_private.h"
#include "eyesight_pdf_mupdf.h"


#define DBG(...) EINA_LOG_DOM_DBG(_eyesight_pdf_log_domain, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_eyesight_pdf_log_domain, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_eyesight_pdf_log_domain, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(_eyesight_pdf_log_domain, __VA_ARGS__)
#define CRIT(...) EINA_LOG_DOM_CRIT(_eyesight_pdf_log_domain, __VA_ARGS__)


static int _eyesight_pdf_log_domain = -1;

static char *
_eyesight_document_property_get(fz_obj *info, char *prop)
{
  fz_obj *obj;

  obj = fz_dictgets(info, prop);
  if (obj)
    return pdf_toutf8(obj);
  else
    return NULL;
}

/* FIXME: handle minutes and seconds ? */
static char *
_eyesight_document_date_get(fz_obj *info, char *prop)
{
  char *date;
  char *fmt;

  date = _eyesight_document_property_get(info, prop);
  if (!date)
    return NULL;

  fmt = (char *)malloc(21);
  if (fmt)
    {
      memcpy(fmt, date + 2, 4);
      fmt[4] = '-';
      memcpy(fmt + 5, date + 6, 2);
      fmt[7] = '-';
      memcpy(fmt + 8, date + 8, 2);
      fmt[10] = ',';
      fmt[11] = ' ';
      memcpy(fmt + 12, date + 10, 2);
      fmt[14] = ':';
      memcpy(fmt + 15, date + 12, 2);
      fmt[17] = ':';
      memcpy(fmt + 18, date + 14, 2);
      fmt[20] = '\0';
      free(date);
      return fmt;
    }
}

static Eina_Bool
_eyesight_document_permission(pdf_crypt *crypt, int perm)
{
  if (!crypt)
    return EINA_TRUE;
  if (crypt->p & perm)
    return EINA_TRUE;
  return EINA_FALSE;
}

static Eina_List *
_eyesight_index_fill(pdf_xref *xref,
                     pdf_outline *outline,
                     Eina_List   *items)
{
  if (!outline)
    return items;

  while (outline)
    {
      Eyesight_Index_Item *item;

      item = eyesight_index_item_new();
      if (!item) goto next_item;

      item ->title = strdup(outline->title);
      /* if title is NULL, we still continue */
      switch (outline->link->kind)
        {
        case PDF_LGOTO:
          item->action = EYESIGHT_LINK_ACTION_GOTO;
          break;
        case PDF_LURI:
          item->action = EYESIGHT_LINK_ACTION_URI;
          break;
        default:
          /* this case should not be reached */
          item->action = EYESIGHT_LINK_ACTION_UNKNOWN;
          break;
        }

      /* FIXME: page outlines are allowed to have a /A key (action) which can be a URL */
      if (outline->link && (outline->link->kind == PDF_LGOTO))
        item->page = pdf_findpageobject(xref, outline->link->dest) - 1;
      /* FIXME: not implemented in mupdf yet */
      item->is_open = EINA_TRUE;

      if (outline->child)
        item->children = _eyesight_index_fill(xref, outline->child, item->children);

      items = eina_list_append (items, item);

    next_item:
      outline = outline->next;
    }

  return items;
}

static void
_eyesight_index_unfill (Eina_List *items)
{
  Eyesight_Index_Item *item;

  if (!items)
    return;

  while (items)
    {
      Eyesight_Index_Item *item;

      item = (Eyesight_Index_Item *)eina_list_data_get(items);

      if (item->title)
        free (item->title);

      if (item->children)
        _eyesight_index_unfill (item->children);

      free (item);

      items = eina_list_remove_list(items, items);
    }
}

static Eyesight_Link_Action_Goto
_eyesight_link_action_goto_fill(pdf_xref *xref, pdf_link *link)
{
  Eyesight_Link_Action_Goto action_goto;

  action_goto.page = pdf_findpageobject(xref, link->dest) - 1;

  /* FIXME: to complete */

  return action_goto;
}

static Eyesight_Link_Action_Uri
_eyesight_link_action_uri_fill(pdf_link *link)
{
  Eyesight_Link_Action_Uri action_uri;
action_uri.uri = NULL;

  action_uri.uri = strdup(fz_tostrbuf(link->dest));
  /* we don't need any check on the success of strdup() */

  return action_uri;
}

static Eyesight_Link_Action_Unknown
_eyesight_link_action_unknown_fill(pdf_link *link)
{
  Eyesight_Link_Action_Unknown action_unknown;

  action_unknown.action = NULL;

  return action_unknown;
}

static Eina_List *
_eyesight_page_links_get(Eyesight_Backend_Pdf *ebp)
{
  Eina_List *links_list = NULL;
  pdf_link *link;

  if (!ebp->page.page)
    return NULL;

  /* FIXME: mupdf should provide the number of links */
  link = ebp->page.page->links;
  while (link)
    {
      Eyesight_Link *link_item;

      link_item = (Eyesight_Link *)calloc(1, sizeof(Eyesight_Link));
      if (!link_item)
        goto next_link;

      switch (link->kind)
        {
        case PDF_LGOTO:
          link_item->action = EYESIGHT_LINK_ACTION_GOTO;
          link_item->dest.action_goto = _eyesight_link_action_goto_fill(ebp->doc.xref, link);
          break;
        case PDF_LURI:
          link_item->action = EYESIGHT_LINK_ACTION_URI;
          link_item->dest.action_uri = _eyesight_link_action_uri_fill(link);
          break;
        default:
          /* this case should not be reached */
          link_item->action = EYESIGHT_LINK_ACTION_UNKNOWN;
          link_item->dest.action_unknown = _eyesight_link_action_unknown_fill(link);
          break;
        }

      switch ((ebp->page.page->rotate + ebp->page.orientation) % 360)
        {
/*         case 90: */
/*           link_item->rect.x = (int)x1; */
/*           link_item->rect.y = (int)(ebp->page.page->getCropWidth() - x2); */
/*           link_item->rect.w = (int)(y2 - y1); */
/*           link_item->rect.h = (int)(x2 - x1); */
/*           break; */
/*         case 180: */
/*           link_item->rect.x = (int)(ebp->page.page->getCropWidth() - x2); */
/*           link_item->rect.y = (int)(ebp->page.page->getCropHeight() - y2); */
/*           link_item->rect.w = (int)(x2 - x1); */
/*           link_item->rect.h = (int)(y2 - y1); */
/*           break; */
/*         case 270: */
/*           link_item->rect.x = (int)(ebp->page.page->getCropHeight() - y2); */
/*           link_item->rect.y = (int)x1; */
/*           link_item->rect.w = (int)(y2 - y1); */
/*           link_item->rect.h = (int)(x2 - x1); */
/*           break; */
        default:
          link_item->rect.x = round(link->rect.x0);
          link_item->rect.y = round(ebp->page.height - link->rect.y1);
          link_item->rect.w = round(link->rect.x1 - link->rect.x0);
          link_item->rect.h = round(link->rect.y1 - link->rect.y0);
          break;
        }

      links_list = eina_list_prepend(links_list, link_item);

    next_link:
      link = link->next;
    }

  return links_list;
}

static void
_eyesight_page_links_free(Eyesight_Backend_Pdf *ebp)
{
  Eina_List *l = NULL;
  void *data;

  if (!ebp->page.links)
    return;

  EINA_LIST_FOREACH(ebp->page.links, l, data)
    {
      Eyesight_Link *link = (Eyesight_Link *)data;

      switch (link->action)
        {
        case EYESIGHT_LINK_ACTION_GOTO:
          /* nothing */
          break;
        case EYESIGHT_LINK_ACTION_URI:
          if (link->dest.action_uri.uri)
            free(link->dest.action_uri.uri);
          break;
        case EYESIGHT_LINK_ACTION_UNKNOWN:
          if (link->dest.action_unknown.action)
            free(link->dest.action_unknown.action);
          break;
        }
      free(link);
    }
}

static Eina_Bool
em_init(Evas *evas, Evas_Object **obj, void **eyesight_backend)
{
  Eyesight_Backend_Pdf *ebp;

  if (!eyesight_backend)
    return EINA_FALSE;

  ebp = (Eyesight_Backend_Pdf *)calloc(1, sizeof(Eyesight_Backend_Pdf));
  if (!ebp)
    return EINA_FALSE;

  ebp->obj = evas_object_image_add(evas);
  if (!ebp->obj)
    goto free_ebp;

  ebp->doc.cache = fz_newglyphcache();
  if (!ebp->doc.cache)
    {
      ERR("Could not create glyph cache");
      goto del_obj;
    }

  *obj = ebp->obj;
  *eyesight_backend = ebp;

  return EINA_TRUE;

 del_obj:
  evas_object_del(ebp->obj);
 free_ebp:
  free(ebp);

  return EINA_FALSE;
}

static void
em_shutdown(void *eb)
{
  Eyesight_Backend_Pdf *ebp;

  if (!eb)
    return;

  ebp = (Eyesight_Backend_Pdf *)eb;
  fz_freeglyphcache(ebp->doc.cache);
  evas_object_del(ebp->obj);
  free(eb);
}

static void *
em_file_open(void *eb, const char *filename)
{
  Eyesight_Backend_Pdf  *ebp;
  Eyesight_Document_Pdf *doc;
  fz_stream *file;
  fz_obj *obj;
  fz_device *dev;
  fz_matrix ctm;
  fz_bbox bbox;
  float zoom;
  float resolution = 72.0f;
  fz_error error;
  int fd;

  if (!eb || !filename || !*filename)
    return NULL;

  DBG("Open file %s", filename);

  ebp = (Eyesight_Backend_Pdf *)eb;
  ebp->filename = strdup(filename);
  if (!ebp->filename)
    return NULL;

  fd = open(ebp->filename, O_BINARY | O_RDONLY, 0666);
  if (fd == -1)
    {
      ERR("Could not open file %s", ebp->filename);
      goto free_filename;
    }

  file = fz_openfile(fd);
  error = pdf_openxrefwithstream(&ebp->doc.xref, file, NULL);
  if (error)
    {
      ERR("PDF file %s illformed", filename);
      goto close_fd;
    }

  fz_close(file);

  obj = fz_dictgets(ebp->doc.xref->trailer, "Info");
  if (!obj)
    goto free_xref;

  doc = (Eyesight_Document_Pdf *)malloc(sizeof(Eyesight_Document_Pdf));
  if (!doc)
    goto free_xref;

  error = pdf_loadpagetree(ebp->doc.xref);
  if (error)
    {
       ERR("Could not load page tree");
       goto free_xref;
    }

  doc->version_maj = ebp->doc.xref->version / 10;
  doc->version_min = ebp->doc.xref->version % 10;
  doc->filename = ebp->filename;

  doc->title = _eyesight_document_property_get(obj, "Title");
  doc->author = _eyesight_document_property_get(obj, "Author");
  doc->subject = _eyesight_document_property_get(obj, "Subject");
  doc->keywords = _eyesight_document_property_get(obj, "Keywords");
  doc->creator = _eyesight_document_property_get(obj, "Creator");
  doc->producer = _eyesight_document_property_get(obj, "Producer");
  doc->date_creation = _eyesight_document_date_get(obj, "CreationDate");
  doc->date_modification = _eyesight_document_date_get(obj, "ModDate");

/*   doc->mode = _eyesight_document_page_mode_get(ebp->doc.pdfdoc); */
/*   doc->layout = _eyesight_document_page_layout_get(ebp->doc.pdfdoc); */

/*   doc->locked = (ebp->doc.pdfdoc->getErrorCode() == errEncrypted) ? EINA_TRUE : EINA_FALSE; */
  doc->encrypted = pdf_needspassword(ebp->doc.xref);
/*   doc->linearized = ebp->doc.pdfdoc->isLinearized(); */
  doc->printable = _eyesight_document_permission(ebp->doc.xref->crypt, PDF_PERM_PRINT);
  doc->changeable = _eyesight_document_permission(ebp->doc.xref->crypt, PDF_PERM_CHANGE);
  doc->copyable = _eyesight_document_permission(ebp->doc.xref->crypt, PDF_PERM_COPY);
  doc->notable = _eyesight_document_permission(ebp->doc.xref->crypt, PDF_PERM_NOTES);
  ebp->document = doc;

  obj = pdf_getpageobject(ebp->doc.xref, 1);
  error = pdf_loadpage(&ebp->page.page, ebp->doc.xref, obj);
  if (error)
    {
       ERR("Could not retrieve first page from the document");
       goto free_xref;
    }

  ebp->page.page->list = fz_newdisplaylist();
  dev = fz_newlistdevice(ebp->page.page->list);
  error = pdf_runpage(ebp->doc.xref, ebp->page.page, dev, fz_identity);
  if (error)
    {
      ERR("Could not draw page");
    }
  fz_freedevice(dev);

  zoom = resolution / 72.0f;
  ctm = fz_translate(0, -ebp->page.page->mediabox.y1);
  ctm = fz_concat(ctm, fz_scale(zoom, -zoom));
  ctm = fz_concat(ctm, fz_rotate(ebp->page.orientation + ebp->page.page->rotate));
  bbox = fz_roundrect(fz_transformrect(ctm, ebp->page.page->mediabox));
  ebp->page.width = bbox.x1 - bbox.x0;
  ebp->page.height = bbox.y1 - bbox.y0;

  ebp->page.links = _eyesight_page_links_get(ebp);

  ebp->page.page_nbr = 0;
  ebp->page.hscale = 1.0;
  ebp->page.vscale = 1.0;
  ebp->page.orientation = EYESIGHT_ORIENTATION_PORTRAIT;

  return doc;

 free_xref:
  pdf_freexref(ebp->doc.xref);
 close_fd:
  close(fd);
 free_filename:
  free(ebp->filename);
  ebp->filename = NULL;

  return NULL;
}

static void
em_file_close(void *eb)
{
  Eyesight_Backend_Pdf *ebp;

  if (!eb)
    return;

  ebp = (Eyesight_Backend_Pdf *)eb;

  DBG("Close file %s", ebp->filename);

  if (ebp->page.page)
    {
      pdf_freepage(ebp->page.page);
      ebp->page.page = NULL;
    }

  _eyesight_page_links_free(ebp);
  _eyesight_index_unfill(ebp->doc.toc);
  if (ebp->document)
    {
      if (ebp->document->title) free(ebp->document->title);
      if (ebp->document->author) free(ebp->document->author);
      if (ebp->document->subject) free(ebp->document->subject);
      if (ebp->document->keywords) free(ebp->document->keywords);
      if (ebp->document->creator) free(ebp->document->creator);
      if (ebp->document->producer) free(ebp->document->producer);
      if (ebp->document->date_creation) free(ebp->document->date_creation);
      if (ebp->document->date_modification) free(ebp->document->date_modification);
      free(ebp->document);
      ebp->document = NULL;
    }
  if (ebp->doc.xref)
    {
      pdf_freexref(ebp->doc.xref);
      ebp->doc.xref = NULL;
    }
  if (ebp->filename)
    {
      free(ebp->filename);
      ebp->filename = NULL;
    }
}

static const Eina_List *
em_toc_get(void *eb)
{
  Eyesight_Backend_Pdf *ebp;
  pdf_outline *outline;

  if (!eb)
    return NULL;

  ebp = (Eyesight_Backend_Pdf *)eb;

  outline = pdf_loadoutline(ebp->doc.xref);
  if (outline)
    {
      ebp->doc.toc = _eyesight_index_fill (ebp->doc.xref, outline, NULL);
      pdf_freeoutline(outline);
    }

  return ebp->doc.toc;
}

static int
em_page_count(void *eb)
{
  Eyesight_Backend_Pdf *ebp;

  if (!eb)
    return 0;

  ebp = (Eyesight_Backend_Pdf *)eb;

  return pdf_getpagecount(ebp->doc.xref);
}

static void
em_page_set(void *eb, int page)
{
  Eyesight_Backend_Pdf *ebp;
  pdf_page *p;
  fz_obj *obj;
  fz_device *dev;
  fz_matrix ctm;
  fz_bbox bbox;
  float zoom;
  float resolution = 72.0f;
  fz_error error;

  if (!eb || (page < 0))
    return;

  ebp = (Eyesight_Backend_Pdf *)eb;

  obj = pdf_getpageobject(ebp->doc.xref, page + 1);
  error = pdf_loadpage(&p, ebp->doc.xref, obj);
  if (error)
    {
      ERR("Could not retrieve page %d from the document", page);
      return;
    }

  p->list = fz_newdisplaylist();
  dev = fz_newlistdevice(p->list);
  error = pdf_runpage(ebp->doc.xref, p, dev, fz_identity);
  if (error)
    {
      ERR("Could not retrieve page %d from the document", page);
      pdf_freepage(p);
      return;
    }
  fz_freedevice(dev);

  zoom = resolution / 72.0f;
  ctm = fz_translate(0, -ebp->page.page->mediabox.y1);
  ctm = fz_concat(ctm, fz_scale(zoom, -zoom));
  ctm = fz_concat(ctm, fz_rotate(ebp->page.orientation + ebp->page.page->rotate));
  bbox = fz_roundrect(fz_transformrect(ctm, ebp->page.page->mediabox));
  ebp->page.width = bbox.x1 - bbox.x0;
  ebp->page.height = bbox.y1 - bbox.y0;

  if (ebp->page.page)
    pdf_freepage(ebp->page.page);

  ebp->page.page = p;
  ebp->page.page_nbr = page;

  ebp->page.links = _eyesight_page_links_get(ebp);
}

static int
em_page_get(void *eb)
{
  Eyesight_Backend_Pdf *ebp;

  if (!eb)
    return 0;

  return ebp->page.page_nbr;
}

static void
em_page_scale_set(void *eb, double hscale, double vscale)
{
  Eyesight_Backend_Pdf *ebp;

  if (!eb)
    return;

  DBG("horizontal scale=%f vertical scale=%f", hscale, vscale);

  ebp = (Eyesight_Backend_Pdf *)eb;

  ebp->page.hscale = hscale;
  ebp->page.vscale = vscale;
}

static void
em_page_scale_get(void *eb, double *hscale, double *vscale)
{
  Eyesight_Backend_Pdf *ebp;

  if (!eb)
    {
      if (hscale) *hscale = 1.0;
      if (vscale) *vscale = 1.0;
    }

  ebp = (Eyesight_Backend_Pdf *)eb;

  if (hscale) *hscale = ebp->page.hscale;
  if (vscale) *vscale = ebp->page.vscale;
}

static void
em_page_orientation_set(void *eb, Eyesight_Orientation orientation)
{
  Eyesight_Backend_Pdf *ebp;

  if (!eb)
    return;

  DBG("orientation=%d", orientation);

  ebp = (Eyesight_Backend_Pdf *)eb;

  ebp->page.orientation = orientation;
}

static Eyesight_Orientation
em_page_orientation_get(void *eb)
{
  Eyesight_Backend_Pdf *ebp;

  if (!eb)
    return EYESIGHT_ORIENTATION_PORTRAIT;

  ebp = (Eyesight_Backend_Pdf *)eb;

  return ebp->page.orientation;
}

static void
em_page_size_get(void *eb, int *width, int *height)
{
  Eyesight_Backend_Pdf *ebp;

  if (!eb)
    {
      if (width) *width = 0;
      if (height) *height = 0;
      return;
    }

  ebp = (Eyesight_Backend_Pdf *)eb;

  if (width) *width = ebp->page.width;
  if (height) *height = ebp->page.height;
}

static void
em_page_render(void *eb)
{
  Eyesight_Backend_Pdf *ebp;
  unsigned int    *m = NULL;
  fz_matrix ctm;
  fz_bbox bbox;
  fz_device *dev;
  fz_pixmap *image;
  float resolution = 72.0f;

  if (!eb)
    return;

  ebp = (Eyesight_Backend_Pdf *)eb;

  ctm = fz_identity;
  ctm = fz_concat(ctm, fz_translate(0, -ebp->page.page->mediabox.y1));
  ctm = fz_concat(ctm, fz_scale(resolution / 72.0f, -resolution/72.0f));
  ctm = fz_concat(ctm, fz_rotate(ebp->page.orientation + ebp->page.page->rotate));
  bbox = fz_roundrect(fz_transformrect(ctm, ebp->page.page->mediabox));

  ebp->page.page->text = fz_newtextspan();
  dev = fz_newtextdevice(ebp->page.page->text);
  fz_executedisplaylist(ebp->page.page->list, dev, ctm);
  fz_freedevice(dev);

  /* FIXME: use fz_newpixmapwithdata(fz_devicergb, bbox.x0, bbox.y0, bbox.x1 - bbox.x0, bbox.y1 - bbox.y0, m) instead to avoid a memcpy */
  image = fz_newpixmapwithrect(fz_devicergb, bbox);
  fz_clearpixmap(image, 0xFF);
  dev = fz_newdrawdevice(ebp->doc.cache, image);
  fz_executedisplaylist(ebp->page.page->list, dev, ctm);
  fz_freedevice(dev);

  evas_object_image_size_set(ebp->obj, image->w, image->h);
  evas_object_image_fill_set(ebp->obj, 0, 0, image->w, image->h);
  m = (unsigned int *)evas_object_image_data_get(ebp->obj, 1);
  if (!m)
    {
       ERR("Could not retrieve data from the Evas Object");
       return;
    }

  memcpy(m, image->samples, image->w * image->h * 4);
  evas_object_image_data_set(ebp->obj, m);
  evas_object_image_data_update_add(ebp->obj, 0, 0, image->w, image->h);
  evas_object_resize(ebp->obj, image->w, image->h);
}

static char *
em_page_text_get(void *eb, Eina_Rectangle rect)
{
  /* FIXME: todo */
  return NULL;
}

static Eina_List *
em_page_text_find(void *eb, const char *text, Eina_Bool is_case_sensitive, Eina_Bool backward)
{
  /* FIXME: todo */
  return NULL;
}

static Eina_List *
em_page_links_get(void *eb)
{
  Eyesight_Backend_Pdf *ebp;

  if (!eb)
    return NULL;

  ebp = (Eyesight_Backend_Pdf *)eb;

  return ebp->page.links;
}

static Eyesight_Module _eyesight_module_pdf =
{
  em_init,
  em_shutdown,
  em_file_open,
  em_file_close,
  em_toc_get,
  em_page_count,
  em_page_set,
  em_page_get,
  em_page_scale_set,
  em_page_scale_get,
  em_page_orientation_set,
  em_page_orientation_get,
  em_page_size_get,
  em_page_render,
  em_page_text_get,
  em_page_text_find,
  em_page_links_get,

  NULL
};

static Eina_Bool
module_open(Evas *evas, Evas_Object **obj, const Eyesight_Module **module, void **backend)
{
   if (!module)
      return EINA_FALSE;

   if (_eyesight_pdf_log_domain < 0)
     {
        _eyesight_pdf_log_domain = eina_log_domain_register("eyesight-pdf", EINA_COLOR_LIGHTCYAN);
        if (_eyesight_pdf_log_domain < 0)
          {
             EINA_LOG_CRIT("Could not register log domain 'eyesight-pdf'");
             return EINA_FALSE;
          }
     }

   if (!_eyesight_module_pdf.init(evas, obj, backend))
     {
        ERR("Could not initialize module");
        eina_log_domain_unregister(_eyesight_pdf_log_domain);
        _eyesight_pdf_log_domain = -1;
       return EINA_FALSE;
     }

   *module = &_eyesight_module_pdf;
   return EINA_TRUE;
}

static void
module_close(Eyesight_Module *module, void *backend)
{
   eina_log_domain_unregister(_eyesight_pdf_log_domain);
   _eyesight_pdf_log_domain = -1;
   _eyesight_module_pdf.shutdown(backend);
}

Eina_Bool
pdf_module_init(void)
{
   return _eyesight_module_register("pdf", module_open, module_close);
}

void
pdf_module_shutdown(void)
{
   _eyesight_module_unregister("pdf");
}

#ifndef EYESIGHT_STATIC_BUILD_PDF

EINA_MODULE_INIT(pdf_module_init);
EINA_MODULE_SHUTDOWN(pdf_module_shutdown);

#endif
