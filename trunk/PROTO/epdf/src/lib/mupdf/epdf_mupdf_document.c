#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>

#include <fitz.h>
#include <mupdf.h>

#include "Epdf.h"
#include "epdf_mupdf_private.h"


static void *pdfdoc_error(Epdf_Document *doc)
{
   if(doc)
     free(doc);

   return NULL;
}

Epdf_Document*
epdf_document_new(const char *filename)
{
   Epdf_Document *doc;
   fz_error error;

   doc = (Epdf_Document *)malloc(sizeof(Epdf_Document));
   if(!doc)
     return NULL;

   doc->xref = NULL;
   doc->outline = NULL;
   doc->rast = NULL;
   doc->pagecount = 0;
   doc->zoom = 1.0;
   doc->rotate = 0;
   doc->locked = 0;

   error = fz_newrenderer(&doc->rast, pdf_devicergb, 0, 1024 * 512);
   if (error)
     return pdfdoc_error(doc);

   fz_obj *obj;

   /* Open PDF and load xref table */

   doc->xref = pdf_newxref();
   if (!doc->xref)
     return pdfdoc_error(doc);

   error = pdf_loadxref(doc->xref, (char *)filename);
   if (error)
     {
        fz_catch(error, "trying to repair");
        fprintf(stderr, "There was a problem with file \"%s\".\nIt may be corrupted or generated by faulty software.\nTrying to repair the file.\n", filename);
        error = pdf_repairxref(doc->xref, (char *)filename);
        if(error)
          return pdfdoc_error(doc);
     }

   error = pdf_decryptxref(doc->xref);
   if (error)
     pdfdoc_error(doc);

   /* Handle encrypted PDF files */

   if (pdf_needspassword(doc->xref))
     doc->locked = 1;

   /*
    * Load meta information
    * TODO: move this into mupdf library
    */

   obj = fz_dictgets(doc->xref->trailer, (char *)"Root");
   doc->xref->root = fz_resolveindirect(obj);
   if (!doc->xref->root)
     {
        fz_throw("syntaxerror: missing Root object");
        pdfdoc_error(doc);
     }
   fz_keepobj(doc->xref->root);

   obj = fz_dictgets(doc->xref->trailer, "Info");
   doc->xref->info = fz_resolveindirect(obj);
   if (!doc->xref->info)
     fprintf(stderr, "Could not load PDF meta information.\n");
   if (doc->xref->info)
     fz_keepobj(doc->xref->info);

   doc->outline = pdf_loadoutline(doc->xref);

   doc->filename = strdup(filename);
   if (doc->xref->info)
     {
        obj = fz_dictgets(doc->xref->info, "Title");
        if(obj)
          doc->doctitle = pdf_toutf8(obj);
     }

   /*
    * Start at first page
    */

   doc->pagecount = pdf_getpagecount(doc->xref);

   if(doc->zoom < 0.1)
     doc->zoom = 0.1;
   if(doc->zoom > 3.0)
     doc->zoom = 3.0;

   return doc;
}

void
epdf_document_delete(Epdf_Document *doc)
{
  if (!doc)
    return; 

  if (doc->outline)
    {
       pdf_dropoutline(doc->outline);
       doc->outline = NULL;
    }

  if (doc->xref->store)
    {
       pdf_dropstore(doc->xref->store);
       doc->xref->store = NULL;
    }

  pdf_closexref(doc->xref);
  doc->xref = NULL;

  if (doc->rast)
    {
       fz_droprenderer(doc->rast);
       doc->rast = NULL;
    }

  free(doc);
}

unsigned char
epdf_document_is_locked(const Epdf_Document *doc)
{
  if (!doc)
    return 0;

  return doc->locked ? 1 : 0;
}

unsigned char
epdf_document_unlock(Epdf_Document *doc, const char *password)
{
  if (!doc)
    return 0;

  if (doc->locked)
    {
       if (!pdf_authenticatepassword(doc->xref, (char *)password))
         fprintf(stderr, "Invalid password.\n");
       else
         doc->locked = 0;
    }

  return doc->locked ? 1 : 0;
}

const char *
epdf_document_info_get (const Epdf_Document *document __UNUSED__, const char *data __UNUSED__)
{
  return NULL;
}

unsigned char
epdf_document_is_encrypted (const Epdf_Document *document)
{
  return 0;
}

unsigned char
epdf_document_is_linearized (const Epdf_Document *document)
{
  return 0;
}

unsigned char
epdf_document_is_printable (const Epdf_Document *document)
{
  return 0;
}

unsigned char
epdf_document_is_changeable (const Epdf_Document *document)
{
  return 0;
}

unsigned char
epdf_document_is_copyable (const Epdf_Document *document)
{
  return 0;
}

unsigned char
epdf_document_is_notable (const Epdf_Document *document)
{
  return 0;
}

const char *
epdf_document_filename_get (const Epdf_Document *document)
{
   if (!document)
     return NULL;

   return document->filename;
}

int
epdf_document_page_count_get(const Epdf_Document *doc)
{
   if (!doc)
     return 0;

   return doc->pagecount;
}

static char *
epdf_document_property_get(const Epdf_Document *doc, const char *property)
{
   fz_obj *obj;
   char *ret;

   if (doc->xref->info)
     {
        obj = fz_dictgets(doc->xref->info, (char *)property);
        if (obj)
          {
             if ((ret = pdf_toutf8(obj)))
               return ret;
             else
               return fz_tostrbuf(obj);
          }
     }

   return NULL;
}

char *
epdf_document_title_get(const Epdf_Document *doc)
{
   if (!doc)
     return NULL;

   return epdf_document_property_get(doc, "Title");
}

char *
epdf_document_author_get(const Epdf_Document *doc)
{
   if (!doc)
     return NULL;

   return epdf_document_property_get(doc, "Author");
}

char *
epdf_document_subject_get(const Epdf_Document *doc)
{
   if (!doc)
     return NULL;

   return epdf_document_property_get(doc, "Subject");
}

char *
epdf_document_keywords_get(const Epdf_Document *doc)
{
   if (!doc)
     return NULL;

   return epdf_document_property_get(doc, "Keywords");
}

char *
epdf_document_creator_get(const Epdf_Document *doc)
{
   if (!doc)
     return NULL;

   return epdf_document_property_get(doc, "Creator");
}

char *
epdf_document_producer_get(const Epdf_Document *doc)
{
   if (!doc)
     return NULL;

   return epdf_document_property_get(doc, "Producer");
}

static char *
epdf_document_date_get(const Epdf_Document *doc, const char *type)
{
   fz_obj *obj;

   if (doc->xref->info)
     {
       obj = fz_dictgets(doc->xref->info, (char *)type);
       if (obj)
         {
            int year, month, day, hour, minute, second;
            char *date = fz_tostrbuf(obj);

            if (date[0] == 'D')
              date += 2;
            sscanf(date, "%4d%2d%2d%2d%2d%2d", &year, &month, &day, &hour, &minute, &second);
            asprintf(&date, "%d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, minute, second);

            return date;
         }
     }

   return NULL;
}

char *
epdf_document_creation_date_get(const Epdf_Document *doc)
{
   if (!doc)
     return NULL;

   return epdf_document_date_get(doc, "CreationDate");
}

char *
epdf_document_mod_date_get(const Epdf_Document *doc)
{
   if (!doc)
     return NULL;

   return epdf_document_date_get(doc, "ModDate");
}

const char *
epdf_document_linearized_get (const Epdf_Document *document)
{
   if (!document)
     return NULL;

   if (epdf_document_is_linearized (document))
     return "yes";
   else
     return "no";
}


void
epdf_document_pdf_version_get (const Epdf_Document *document, int *major, int *minor)
{
   if (!document)
     {
        if (major) *major = 0;
        if (minor) *minor = 0;
     }

   if (major) *major = document->xref->version / 10;
   if (minor) *minor = document->xref->version % 10;
}

Eina_List *
epdf_document_fonts_get (const Epdf_Document *document)
{
  return NULL;
}

const char *
epdf_document_page_mode_string_get (const Epdf_Document *document)
{
  return "none";
}

const char *
epdf_document_page_layout_string_get (const Epdf_Document *document)
{
  return "none";
}
