#include "Ethumb.h"
#include "ethumb_private.h"
#include "Ethumb_Plugin.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <Eina.h>
#include <Evas.h>
#include <Epdf.h>

static int
_generate_thumb(Ethumb *e)
{
   Epdf_Document *document;
   Epdf_Page *page;
   Evas_Object *o;
   int w, h, ww, hh;
   int fx, fy, fw, fh;
   int npages;

   document = epdf_document_new(e->src_path);
   if (!document)
     {
	fprintf(stderr, "ERROR: could not read document: %s\n", e->src_path);
	return 0;
     }

   page = epdf_page_new(document);
   if (!page)
     {
	fprintf(stderr, "ERROR: could not read document: %s\n", e->src_path);
	epdf_document_delete(document);
	return 0;
     }

   npages = epdf_document_page_count_get(document);
   if (e->document.page < npages)
     epdf_page_page_set(page, e->document.page);
   epdf_page_size_get(page, &w, &h);
   ethumb_calculate_aspect(e, w, h, &ww, &hh);
   ethumb_plugin_image_resize(e, ww, hh);

   o = evas_object_image_add(e->sub_e);
   epdf_page_render(page, o);
   evas_object_resize(o, ww, hh);
   evas_object_move(o, 0, 0);

   ethumb_calculate_fill(e, w, h, &fx, &fy, &fw, &fh);
   evas_object_image_fill_set(o, fx, fy, fw, fh);

   evas_object_show(o);
   ethumb_image_save(e);

   evas_object_del(o);
   epdf_page_delete(page);
   epdf_document_delete(document);

   ethumb_finished_callback_call(e);

   return 1;
}

Ethumb_Plugin *
ethumb_plugin_get(void)
{
   static const char *extensions[] = { "pdf", NULL };
   static Ethumb_Plugin plugin =
     {
	extensions,
	_generate_thumb,
     };

   return &plugin;
}

Eina_Bool
_module_init(void)
{
   epdf_init();

   return EINA_TRUE;
}

static void
_module_shutdown(void)
{
   epdf_shutdown();
}

EINA_MODULE_INIT(_module_init);
EINA_MODULE_SHUTDOWN(_module_shutdown);
