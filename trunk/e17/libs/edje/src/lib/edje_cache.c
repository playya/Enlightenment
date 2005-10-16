/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "Edje.h"
#include "edje_private.h"

static Evas_Hash   *_edje_file_hash = NULL;
static int          _edje_file_cache_size = 16;
static Evas_List   *_edje_file_cache = NULL;

static int          _edje_collection_cache_size = 16;

static Edje_Part_Collection *
_edje_file_coll_open(Edje_File *edf, Eet_File *ef, char *coll)
{
   Edje_Part_Collection *edc = NULL;
   Evas_List *l = NULL;
   int id = -1, close_eet = 0, size = 0;
   char buf[256];
   void *data;
   
   for (l = edf->collection_dir->entries; l; l = l->next)
     {
	Edje_Part_Collection_Directory_Entry *ce;
	
	ce = l->data;
	if ((ce->entry) && (!strcmp(ce->entry, coll)))
	  {
	     id = ce->id;
	     break;
	  }
     }
   if (id < 0) return NULL;
   
   if (!ef)
     {
	ef = eet_open(edf->path, EET_FILE_MODE_READ);
	if (!ef) return NULL;
	close_eet = 1;
     }
   snprintf(buf, sizeof(buf), "collections/%i", id);
   edc = eet_data_read(ef, _edje_edd_edje_part_collection, buf);
   if (!edc)
     {
	if (close_eet) eet_close(ef);
	return NULL;
     }
   
   snprintf(buf, sizeof(buf), "scripts/%i", id);
   data = eet_read(ef, buf, &size);
   if (close_eet) eet_close(ef);
   
   if (data)
     {
	edc->script = embryo_program_new(data, size);
	free(data);
     }
   
   edc->part = strdup(coll);
   edc->references = 1;
   edf->collection_hash = evas_hash_add(edf->collection_hash, coll, edc);
   return edc;
}

static Edje_File *
_edje_file_open(char *file, char *coll, int *error_ret, Edje_Part_Collection **edc_ret)
{
   Edje_File *edf;
   Edje_Part_Collection *edc;
   Eet_File *ef;
   
   ef = eet_open((char *)file, EET_FILE_MODE_READ);
   if (!ef)
     {
	*error_ret = EDJE_LOAD_ERROR_UNKNOWN_FORMAT;
	return NULL;
     }
   edf = eet_data_read(ef, _edje_edd_edje_file, "edje_file");
   if (!edf)
     {
	*error_ret = EDJE_LOAD_ERROR_CORRUPT_FILE;
	eet_close(ef);
	return NULL;
     }
   if (edf->version != EDJE_FILE_VERSION)
     {
	*error_ret = EDJE_LOAD_ERROR_INCOMPATIBLE_FILE;
	_edje_file_free(edf);
	eet_close(ef);
	return NULL;
     }
   if (!edf->collection_dir)
     {
	*error_ret = EDJE_LOAD_ERROR_CORRUPT_FILE;
	_edje_file_free(edf);
	eet_close(ef);
	return NULL;
     }
   
   edf->path = strdup(file);
   edf->references = 1;

   _edje_textblock_style_parse_and_fix(edf);
   
   if (!coll)
     {
	eet_close(ef);
	return edf;
     }
   
   edc = _edje_file_coll_open(edf, ef, coll);
   if (!edc)
     {
	*error_ret = EDJE_LOAD_ERROR_UNKNOWN_COLLECTION;
     }
   if (edc_ret) *edc_ret = edc;

   eet_close(ef);
   return edf;
}

Edje_File *
_edje_cache_file_coll_open(char *file, char *coll, int *error_ret, Edje_Part_Collection **edc_ret)
{
   Edje_File *edf;
   Evas_List *l;
   Edje_Part_Collection *edc;

   edf = evas_hash_find(_edje_file_hash, file);
   if (edf)
     {
	edf->references++;
     }
   else
     {
	for (l = _edje_file_cache; l; l = l->next)
	  {
	     edf = l->data;
	     if (!strcmp(edf->path, file))
	       {
		  edf->references = 1;
		  _edje_file_cache = evas_list_remove_list(_edje_file_cache, l);
		  _edje_file_hash = evas_hash_add(_edje_file_hash, file, edf);
		  break;
	       }
	     edf = NULL;
	  }
     }
   if (!edf)
     {
	edf = _edje_file_open(file, coll, error_ret, edc_ret);
	if (!edf) return NULL;
	_edje_file_hash = evas_hash_add(_edje_file_hash, file, edf);
	return edf;
     }
   
   if (!coll) return edf;

   edc = evas_hash_find(edf->collection_hash, coll);
   if (edc)
     {
	edc->references++;
     }
   else
     {
	for (l = edf->collection_cache; l; l = l->next)
	  {
	     edc = l->data;
	     if (!strcmp(edc->part, coll))
	       {
		  edc->references = 1;
		  edf->collection_cache = evas_list_remove_list(edf->collection_cache, l);
		  edf->collection_hash = evas_hash_add(edf->collection_hash, coll, edc);
		  break;
	       }
	     edc = NULL;
	  }
     }
   if (!edc)
     {
	edc = _edje_file_coll_open(edf, NULL, coll);
	if (!edc)
	  {
	     *error_ret = EDJE_LOAD_ERROR_UNKNOWN_COLLECTION;
	  }
     }
   if (edc_ret) *edc_ret = edc;
   
   return edf;
}

void
_edje_cache_coll_clean(Edje_File *edf)
{
   int count;
   
   count = evas_list_count(edf->collection_cache);
   while ((edf->collection_cache) && (count > _edje_collection_cache_size))
     {
	Edje_Part_Collection *edc;
	
	edc = evas_list_last(edf->collection_cache)->data;
	edf->collection_cache = evas_list_remove_list(edf->collection_cache, evas_list_last(edf->collection_cache));
	_edje_collection_free(edf, edc);
	count = evas_list_count(edf->collection_cache);
     }
}

void
_edje_cache_coll_unref(Edje_File *edf, Edje_Part_Collection *edc)
{
   edc->references--;
   if (edc->references != 0) return;
   edf->collection_hash = evas_hash_del(edf->collection_hash, edc->part, edc);
   edf->collection_cache = evas_list_prepend(edf->collection_cache, edc);
   _edje_cache_coll_clean(edf);
}

static void
_edje_cache_file_clean(void)
{
   int count;
   
   count = evas_list_count(_edje_file_cache);
   while ((_edje_file_cache) && (count > _edje_file_cache_size))
     {
	Edje_File *edf;
	
	edf = evas_list_last(_edje_file_cache)->data;
	_edje_file_cache = evas_list_remove_list(_edje_file_cache, evas_list_last(_edje_file_cache));
	_edje_file_free(edf);
	count = evas_list_count(_edje_file_cache);
     }
}

void
_edje_cache_file_unref(Edje_File *edf)
{
   edf->references--;
   if (edf->references != 0) return;
   _edje_file_hash = evas_hash_del(_edje_file_hash, edf->path, edf);
   _edje_file_cache = evas_list_prepend(_edje_file_cache, edf);
   _edje_cache_file_clean();
}

void
_edje_file_cache_shutdown(void)
{
   edje_file_cache_flush();
}






void
edje_file_cache_set(int count)
{
   if (count < 0) count = 0;
   _edje_file_cache_size = count;
   _edje_cache_file_clean();
}

int
edje_file_cache_get(void)
{
   return _edje_file_cache_size;
}

void
edje_file_cache_flush(void)
{
   int ps;
   
   ps = _edje_file_cache_size;
   _edje_file_cache_size = 0;
   _edje_cache_file_clean();
   _edje_file_cache_size = ps;
}

void
edje_collection_cache_set(int count)
{
   Evas_List *l;
   
   if (count < 0) count = 0;
   _edje_collection_cache_size = count;
   for (l = _edje_file_cache; l; l = l->next)
     {
	Edje_File *edf;
	
	edf = l->data;
	_edje_cache_coll_clean(edf);
     }
   /* FIXME: freach in file hash too! */
}

int
edje_collection_cache_get(void)
{
   return _edje_collection_cache_size;
}

void
edje_collection_cache_flush(void)
{
   int ps;
   Evas_List *l;
   
   ps = _edje_collection_cache_size;
   _edje_collection_cache_size = 0;
   for (l = _edje_file_cache; l; l = l->next)
     {
	Edje_File *edf;
	
	edf = l->data;
	_edje_cache_coll_clean(edf);
     }
   /* FIXME: freach in file hash too! */
   _edje_collection_cache_size = ps;
}
