/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "evas_common.h"
#include "evas_private.h"

#ifdef EVAS_CSERVE
// FIXME: cache server and threaded preload clash badly atm - disable
//#undef BUILD_ASYNC_PRELOAD
#endif

#ifdef BUILD_ASYNC_PRELOAD
#include <pthread.h>

typedef struct _Evas_Cache_Preload Evas_Cache_Preload;

struct _Evas_Cache_Preload
{
   EINA_INLIST;
   Image_Entry *ie;
};

static Eina_Inlist *preload = NULL;
static Image_Entry *current = NULL;
static Eina_List   *pending = NULL;

static pthread_cond_t cond_done = PTHREAD_COND_INITIALIZER;
static pthread_cond_t cond_new = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mutex_new = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_surface_alloc = PTHREAD_MUTEX_INITIALIZER;
static pthread_t tid = 0;

static Eina_Bool running = EINA_FALSE;

static void *_evas_cache_background_load(void *);
#endif

#define FREESTRC(Var)              \
  if (Var)                         \
    {                              \
       eina_stringshare_del(Var);  \
       Var = NULL;                 \
    }

static void _evas_cache_image_entry_delete(Evas_Cache_Image *cache, Image_Entry *ie);

static void
_evas_cache_image_make_dirty(Evas_Cache_Image *cache,
                             Image_Entry *im)
{
   im->flags.cached = 1;
   im->flags.dirty = 1;
   im->flags.activ = 0;
   im->flags.lru_nodata = 0;
   cache->dirty = eina_inlist_prepend(cache->dirty, EINA_INLIST_GET(im));

   if (im->cache_key)
     {
        eina_stringshare_del(im->cache_key);
        im->cache_key = NULL;
     }
}

static void
_evas_cache_image_make_activ(Evas_Cache_Image *cache,
                             Image_Entry *im,
                             const char *key)
{
#ifdef BUILD_ASYNC_PRELOAD
   pthread_mutex_lock(&mutex);
   if (im->flags.pending)
     {
        im->flags.pending = 0;
        pending = eina_list_remove(pending, im);
     }
   pthread_mutex_unlock(&mutex);
#endif

   im->cache_key = key;
   if (key != NULL)
     {
        im->flags.cached = 1;
        im->flags.activ = 1;
        im->flags.lru_nodata = 0;
        im->flags.dirty = 0;
	eina_hash_direct_add(cache->activ, key, im);
     }
   else
     {
        _evas_cache_image_make_dirty(cache, im);
     }
}

static void
_evas_cache_image_make_inactiv(Evas_Cache_Image *cache,
                               Image_Entry *im,
                               const char *key)
{
   if (im->cache_key)
     {
	im->flags.activ = 0;
	im->flags.dirty = 0;
	im->flags.cached = 1;
	eina_hash_direct_add(cache->inactiv, key, im);
	cache->lru = eina_inlist_prepend(cache->lru, EINA_INLIST_GET(im));
	cache->usage += cache->func.mem_size_get(im);
     }
   else
     {
	_evas_cache_image_entry_delete(cache, im);
     }
 }

static void
_evas_cache_image_remove_lru_nodata(Evas_Cache_Image *cache,
                                    Image_Entry *im)
{
   if (im->flags.lru_nodata)
     {
        im->flags.lru_nodata = 0;
        cache->lru_nodata = eina_inlist_remove(cache->lru_nodata, EINA_INLIST_GET(im));
        cache->usage -= cache->func.mem_size_get(im);
     }
}

static void
_evas_cache_image_activ_lru_nodata(Evas_Cache_Image *cache,
                                   Image_Entry *im)
{
   im->flags.need_data = 0;
   im->flags.lru_nodata = 1;
   cache->lru_nodata = eina_inlist_prepend(cache->lru_nodata, EINA_INLIST_GET(im));
   cache->usage += cache->func.mem_size_get(im);
}

static void
_evas_cache_image_remove_activ(Evas_Cache_Image *cache,
                               Image_Entry *ie)
{
   if (ie->flags.cached)
     {
        if (ie->flags.activ)
          {
	     eina_hash_del(cache->activ, ie->cache_key, ie);
             _evas_cache_image_remove_lru_nodata(cache, ie);
          }
        else
          {
             if (ie->flags.dirty)
               {
 		  cache->dirty = eina_inlist_remove(cache->dirty, EINA_INLIST_GET(ie));
               }
             else
               {
		  eina_hash_del(cache->inactiv, ie->cache_key, ie);
                  cache->lru = eina_inlist_remove(cache->lru, EINA_INLIST_GET(ie));
                  cache->usage -= cache->func.mem_size_get(ie);
               }
          }
        ie->flags.cached = 0;
        ie->flags.dirty = 0;
        ie->flags.activ = 0;
     }
}

static void
_evas_cache_image_entry_delete(Evas_Cache_Image *cache, Image_Entry *ie)
{
   if (!ie) return ;

   if (cache->func.debug)
     cache->func.debug("deleting", ie);

   cache->func.destructor(ie);

   _evas_cache_image_remove_activ(cache, ie);

   if (ie->cache_key)
     {
        eina_stringshare_del(ie->cache_key);
        ie->cache_key = NULL;
     }

   FREESTRC(ie->file);
   FREESTRC(ie->key);

   cache->func.surface_delete(ie);

#ifdef BUILD_ASYNC_PRELOAD
   LKD(ie->lock);
#endif

   cache->func.dealloc(ie);
}

static Image_Entry *
_evas_cache_image_entry_new(Evas_Cache_Image *cache,
                            const char *hkey,
                            time_t timestamp,
                            const char *file,
                            const char *key,
                            RGBA_Image_Loadopts *lo,
                            int *error)
{
   Image_Entry  *ie;
   const char   *cache_key;

   ie = cache->func.alloc();
   if (!ie)
     return NULL;

   cache_key = hkey ? eina_stringshare_add(hkey) : NULL;

   ie->flags.loaded = 0;
   ie->flags.need_data = 1;

   _evas_cache_image_make_activ(cache, ie, cache_key);

   ie->space = EVAS_COLORSPACE_ARGB8888;
   ie->w = -1;
   ie->h = -1;
   ie->allocated.w = 0;
   ie->allocated.h = 0;

   ie->references = 0;
   ie->cache = cache;

   ie->file = file ? eina_stringshare_add(file) : NULL;
   ie->key = key ? eina_stringshare_add(key) : NULL;

   ie->timestamp = timestamp;
   ie->laststat = time(NULL);

   ie->load_opts.scale_down_by = 0;
   ie->load_opts.dpi = 0;
   ie->load_opts.w = 0;
   ie->load_opts.h = 0;
   ie->load_opts.region.x = 0;
   ie->load_opts.region.y = 0;
   ie->load_opts.region.w = 0;
   ie->load_opts.region.h = 0;
   ie->scale = 1;

#ifdef BUILD_ASYNC_PRELOAD
   LKI(ie->lock);
   ie->targets = NULL;
#endif

   if (lo)
     ie->load_opts = *lo;

   if (file)
     {
        *error = cache->func.constructor(ie);
        if (*error != 0)
          {
             _evas_cache_image_entry_delete(cache, ie);
             return NULL;
          }
     }
   if (cache->func.debug)
     cache->func.debug("build", ie);

   return ie;
}

static void
_evas_cache_image_entry_surface_alloc__locked(Evas_Cache_Image *cache,
					      Image_Entry *ie,
					      int wmin,
					      int hmin)
{
   if (ie->allocated.w == wmin && ie->allocated.h == hmin)
     return ;

   if (cache->func.surface_alloc(ie, wmin, hmin))
     {
        wmin = 0;
        hmin = 0;
     }

   ie->w = wmin;
   ie->h = hmin;
   ie->allocated.w = wmin;
   ie->allocated.h = hmin;
}

static void
_evas_cache_image_entry_surface_alloc(Evas_Cache_Image *cache,
                                      Image_Entry *ie,
                                      int w,
                                      int h)
{
   int  wmin;
   int  hmin;

   wmin = w > 0 ? w : 1;
   hmin = h > 0 ? h : 1;

#ifdef BUILD_ASYNC_PRELOAD
   pthread_mutex_lock(&mutex_surface_alloc);
#endif
   _evas_cache_image_entry_surface_alloc__locked(cache, ie, wmin, hmin);
#ifdef BUILD_ASYNC_PRELOAD
   pthread_mutex_unlock(&mutex_surface_alloc);
#endif
}

#ifdef BUILD_ASYNC_PRELOAD
static void
_evas_cache_image_async_call__locked(Image_Entry *im)
{
   while (im->targets)
     {
	Evas_Cache_Target *tmp = im->targets;

        // FIXME: bug bug bug bug. hrrrm.
        // FIXME: this is a little bad if obj gets deleted before preload
        // async event comes in... boom!
	evas_async_events_put(tmp->target, EVAS_CALLBACK_IMAGE_PRELOADED, NULL,
			      (void (*)(void*, Evas_Callback_Type, void*))evas_object_event_callback_call);
	im->targets = (Evas_Cache_Target*) eina_inlist_remove(EINA_INLIST_GET(im->targets), EINA_INLIST_GET(im->targets));
	free(tmp);
     }
}

static void
_evas_cache_image_async_call(Image_Entry *im)
{
   pthread_mutex_lock(&mutex);
   _evas_cache_image_async_call__locked(im);
   pthread_mutex_unlock(&mutex);
}

static int
_evas_cache_image_entry_preload_add(Image_Entry *ie,
				    const void *target)
{
   Evas_Cache_Target *tg;
   int ret = 0;

   pthread_mutex_lock(&mutex);

   if (!ie->flags.loaded)
     {
	tg = malloc(sizeof (Evas_Cache_Target));
	if (!tg) goto end;

	tg->target = target;
	ie->targets = (Evas_Cache_Target*) eina_inlist_append(EINA_INLIST_GET(ie->targets), EINA_INLIST_GET(tg));

	if (!ie->flags.preload)
	  {
	     Evas_Cache_Preload *tmp;

	     tmp = malloc(sizeof (Evas_Cache_Preload));
	     if (!tmp) goto end;

	     tmp->ie = ie;
	     preload = eina_inlist_append(preload, EINA_INLIST_GET(tmp));

	     ie->flags.preload = 1;
             if (ie->flags.pending)
               {
                  ie->flags.pending = 0;
                  pending = eina_list_remove(pending, ie);
               }

	     if (!running)
	       {
		  if (tid)
		    {
		       running = EINA_TRUE;
		       pthread_cond_signal(&cond_new);
		    }
		  else
		    {
		       if (pthread_create(&tid, NULL, _evas_cache_background_load, NULL) == 0)
			 running = EINA_TRUE;
		    }
	       }

	     ret = 2;
	  }
	ret = 1;
     }

 end:
   pthread_mutex_unlock(&mutex);

   return ret;
}

static int
_evas_cache_image_entry_preload_remove(Image_Entry *ie, const void *target)
{
   int ret = 0;

   if (running)
     {
	pthread_mutex_lock(&mutex);
	if (ie->flags.preload)
	  {
	     if (current == ie)
	       {
// dont wait. simply handle "ie->flags.preload" nicely
//		  /* Wait until ie is processed. */
//		  pthread_cond_wait(&cond_done, &mutex);
	       }
	     else
	       {
		  Evas_Cache_Preload *l;
		  EINA_INLIST_FOREACH(preload, l)
		    {
		       if (l->ie == ie)
			 {
			    Evas_Cache_Target *tg;
                            
			    if (target)
                              {
                                 EINA_INLIST_FOREACH(ie->targets, tg)
                                   {
                                      if (tg->target == target)
                                        {
                                           ie->targets = (Evas_Cache_Target*) eina_inlist_remove(EINA_INLIST_GET(ie->targets), EINA_INLIST_GET(tg));
                                           free(tg);
                                           break;
                                        }
                                   }
                              }
                            else
                              {
                                 _evas_cache_image_async_call__locked(ie);
                                 
                                 while (ie->targets)
                                   {
                                      tg = ie->targets;
                                      ie->targets = (Evas_Cache_Target*) eina_inlist_remove(EINA_INLIST_GET(ie->targets), EINA_INLIST_GET(tg));
                                      free(tg);
                                   }
                              }
                            
			    if (!ie->targets)
			      {
				 preload = eina_inlist_remove(preload,
							      EINA_INLIST_GET(l));
				 free(l);
			      }
                            
			    break;
			 }
		    }
		  ie->flags.preload = 0;
		  ret = 1;
	       }
	  }
	pthread_mutex_unlock(&mutex);
     }

   return ret;
}

static void*
_evas_cache_background_load(void *data)
{
   (void) data;
   
   restart:
   while (preload)
     {
	pthread_mutex_lock(&mutex);
	if (preload)
	  {
	     Evas_Cache_Preload *tmp = (Evas_Cache_Preload*) preload;
             
	     current = tmp->ie;
	     preload = eina_inlist_remove(preload, preload);

	     free(tmp);
	  }

	pthread_mutex_unlock(&mutex);

	if (current)
	  {
	     Evas_Cache_Image *cache;
	     int error;
             int pchannel;

	     LKL(current->lock);
             pchannel = current->channel;
             current->channel++;
	     cache = current->cache;

             if (!current->flags.loaded)
               {
                  error = cache->func.load(current);
                  if (cache->func.debug)
                    cache->func.debug("load", current);
                  if (error)
                    {
                       _evas_cache_image_entry_surface_alloc
                         (cache, current, current->w, current->h);
                       current->flags.loaded = 0;
                    }
                  else
                    current->flags.loaded = 1;
               }
             
             pthread_mutex_lock(&mutex);
	     current->flags.preload = 0;
             current->channel = pchannel;
	     LKU(current->lock);
             pthread_mutex_unlock(&mutex);
             
	     _evas_cache_image_async_call(current);
             
             pthread_mutex_lock(&mutex);
	     current = NULL;
             pthread_mutex_unlock(&mutex);
	  }

	pthread_cond_signal(&cond_done);
     }

   pthread_mutex_lock(&mutex);
   if (preload)
     {
	pthread_mutex_unlock(&mutex);
	goto restart;
     }

   running = EINA_FALSE;
   pthread_mutex_unlock(&mutex);

   pthread_mutex_lock(&mutex_new);
   pthread_cond_wait(&cond_new, &mutex_new);
   pthread_mutex_unlock(&mutex_new);
   goto restart;

   return NULL;
}
#endif

EAPI int
evas_cache_image_usage_get(Evas_Cache_Image *cache)
{
   assert(cache != NULL);

   return cache->usage;
}

EAPI int
evas_cache_image_get(Evas_Cache_Image *cache)
{
   assert(cache != NULL);

   return cache->limit;
}

EAPI void
evas_cache_image_set(Evas_Cache_Image *cache, int limit)
{
   assert(cache != NULL);
   if (cache->limit == limit) return;
   cache->limit = limit;
   evas_cache_image_flush(cache);
}

EAPI Evas_Cache_Image *
evas_cache_image_init(const Evas_Cache_Image_Func *cb)
{
   Evas_Cache_Image    *new;

   new = malloc(sizeof (Evas_Cache_Image));
   if (!new)
     return NULL;

   new->func = *cb;

   new->limit = 0;
   new->usage = 0;

   new->dirty = NULL;
   new->lru = NULL;
   new->lru_nodata = NULL;
   new->inactiv = eina_hash_string_superfast_new(NULL);
   new->activ = eina_hash_string_superfast_new(NULL);

   new->references = 1;

   return new;
}

static Eina_Bool
_evas_cache_image_free_cb(__UNUSED__ const Eina_Hash *hash, __UNUSED__ const void *key, void *data, void *fdata)
{
   Eina_List **delete_list = fdata;

   *delete_list = eina_list_prepend(*delete_list, data);

   return EINA_TRUE;
}

#ifdef BUILD_ASYNC_PRELOAD
static void
_evas_cache_image_entry_clear_preloaders(Image_Entry *ie)
{
   while (ie->targets)
     {
	Evas_Cache_Target *t = ie->targets;
	ie->targets = (Evas_Cache_Target *)
	  eina_inlist_remove(EINA_INLIST_GET(ie->targets),
			     EINA_INLIST_GET(ie->targets));
	free(t);
     }
}
#endif

EAPI void
evas_cache_image_shutdown(Evas_Cache_Image *cache)
{
   Image_Entry  *im;
   Eina_List *delete_list;

   assert(cache != NULL);
   cache->references--;

   if (cache->references > 0)
     return ;

#ifdef BUILD_ASYNC_PRELOAD
   pthread_mutex_lock(&mutex);
   if (running)
     {
	Eina_Inlist *l, *l_next;
	for (l = preload; l != NULL; l = l_next)
	  {
	     Evas_Cache_Preload *tmp = (Evas_Cache_Preload *)l;
	     Image_Entry *ie = tmp->ie;

	     l_next = l->next;

	     if (ie->cache != cache)
	       continue;

	     preload = eina_inlist_remove(preload, l);
	     _evas_cache_image_entry_clear_preloaders(ie);
	     free(l);
	  }
     }
   pthread_mutex_unlock(&mutex);
#endif

   while (cache->lru)
     {
        im = (Image_Entry *) cache->lru;
        _evas_cache_image_entry_delete(cache, im);
     }

   while (cache->lru_nodata)
     {
        im = (Image_Entry *) cache->lru_nodata;
        _evas_cache_image_entry_delete(cache, im);
     }

   /* This is mad, I am about to destroy image still alive, but we need to prevent leak. */
   while (cache->dirty)
     {
        im = (Image_Entry *) cache->dirty;
        _evas_cache_image_entry_delete(cache, im);
     }

   delete_list = NULL;
   eina_hash_foreach(cache->activ, _evas_cache_image_free_cb, &delete_list);

   while (delete_list)
     {
	_evas_cache_image_entry_delete(cache, eina_list_data_get(delete_list));
	delete_list = eina_list_remove_list(delete_list, delete_list);
     }

   eina_hash_free(cache->activ);
   eina_hash_free(cache->inactiv);

   free(cache);
}

#define STAT_GAP 2

EAPI Image_Entry *
evas_cache_image_request(Evas_Cache_Image *cache, const char *file, const char *key, RGBA_Image_Loadopts *lo, int *error)
{
   const char           *ckey = "(null)";
   const char           *format;
   char                 *hkey;
   Image_Entry          *im;
   Evas_Image_Load_Opts  prevent = { 0, 0, 0, 0, 0, 0, 0, 0 };
   size_t                size;
   int                   stat_done = 0;
   size_t                file_length;
   size_t                key_length;
   struct stat           st;

   assert(cache != NULL);

   if (!file && !key) return NULL;
   if (!file) return NULL;

   file_length = strlen(file);
   key_length = key ? strlen(key) : 6;

   size = file_length + key_length + 128;
   hkey = alloca(sizeof (char) * size);

   memcpy(hkey, file, file_length);
   size = file_length;

   memcpy(hkey + size, "//://", 5);
   size += 5;

   if (key) ckey = key;
   memcpy(hkey + size, ckey, key_length);
   size += key_length;

   if ((!lo) ||
       (lo &&
        (lo->scale_down_by == 0) &&
        (lo->dpi == 0.0) &&
        ((lo->w == 0) || (lo->h == 0)) &&
        ((lo->region.w == 0) || (lo->region.w == 0))
        ))
     {
        lo = &prevent;
//        if (key)
//          format = "%s//://%s";
//        else
//          format = "%s//://%p";
     }
   else
     {
	memcpy(hkey + size, "//@/", 4);
	size += 4;

	size += eina_convert_xtoa(lo->scale_down_by, hkey + size);

	hkey[size] = '/';
	size += 1;

	size += eina_convert_dtoa(lo->dpi, hkey + size);

	hkey[size] = '/';
	size += 1;

	size += eina_convert_xtoa(lo->w, hkey + size);

	hkey[size] = 'x';
	size += 1;

	size += eina_convert_xtoa(lo->h, hkey + size);
        
	hkey[size] = '/';
	size += 1;

	size += eina_convert_xtoa(lo->region.x, hkey + size);
        
	hkey[size] = '+';
	size += 1;
        
	size += eina_convert_xtoa(lo->region.y, hkey + size);
        
	hkey[size] = '.';
	size += 1;
        
	size += eina_convert_xtoa(lo->region.w, hkey + size);
        
	hkey[size] = 'x';
	size += 1;
        
	size += eina_convert_xtoa(lo->region.h, hkey + size);
     }

   hkey[size] = '\0';

   im = eina_hash_find(cache->activ, hkey);
   if (im)
     {
        time_t  t;
        int     ok;

        ok = 1;
        t = time(NULL);
        if ((t - im->laststat) > STAT_GAP)
          {
             stat_done = 1;
             if (stat(file, &st) < 0) goto on_error;

             im->laststat = t;
             if (st.st_mtime != im->timestamp) ok = 0;
          }
        if (ok) goto on_ok;

        _evas_cache_image_remove_activ(cache, im);
	_evas_cache_image_make_dirty(cache, im);
     }

   im = eina_hash_find(cache->inactiv, hkey);
   if (im)
     {
        int     ok;

        ok = 1;
        if (!stat_done)
          {
             time_t  t;

             t = time(NULL);
             if ((t - im->laststat) > STAT_GAP)
               {
                  stat_done = 1;
                  if (stat(file, &st) < 0) goto on_error;

                  im->laststat = t;
                  if (st.st_mtime != im->timestamp) ok = 0;
               }
          }
        else
          if (st.st_mtime != im->timestamp) ok = 0;

        if (ok)
          {
             _evas_cache_image_remove_activ(cache, im);
             _evas_cache_image_make_activ(cache, im, im->cache_key);
             goto on_ok;
          }

        _evas_cache_image_entry_delete(cache, im);
     }

   if (!stat_done)
     {
        if (stat(file, &st) < 0) return NULL;
     }

   im = _evas_cache_image_entry_new(cache, hkey, st.st_mtime, file, key, lo, error);
   if (!im) return NULL;

   if (cache->func.debug)
     cache->func.debug("request", im);

 on_ok:
   *error = 0;
   im->references++;
   if (im->references > 1 && im->flags.lru_nodata)
     _evas_cache_image_remove_lru_nodata(cache, im);

   return im;

 on_error:
   _evas_cache_image_entry_delete(cache, im);
   return NULL;
}

EAPI void
evas_cache_pending_process(void)
{
   Image_Entry *im;
   
#ifdef BUILD_ASYNC_PRELOAD
   pthread_mutex_lock(&mutex);
   EINA_LIST_FREE(pending, im)
     {
        Evas_Cache_Image *cache = im->cache;

        if (!im->flags.pending) continue;
        im->flags.pending = 0;
        if (im->flags.preload) continue;
        
        if (im->flags.activ)
          {
             if (im->flags.dirty)
               {
                  _evas_cache_image_entry_delete(cache, im);
               }
             else
               {
                  _evas_cache_image_remove_activ(cache, im);
                  _evas_cache_image_make_inactiv(cache, im, im->cache_key);
                  evas_cache_image_flush(cache);
               }
          }
        else if (im->flags.cached)
          {
             evas_cache_image_flush(cache);
          }
     }
   pthread_mutex_unlock(&mutex);
#endif   
}

EAPI void
evas_cache_image_drop(Image_Entry *im)
{
   Evas_Cache_Image    *cache;

   assert(im);
   assert(im->cache);

   im->references--;
   cache = im->cache;

   if (im->references == 0)
     {
#ifdef BUILD_ASYNC_PRELOAD
	pthread_mutex_lock(&mutex);
        if (im->flags.preload)
          {
             pthread_mutex_unlock(&mutex);
             _evas_cache_image_entry_preload_remove(im, NULL);
             pthread_mutex_lock(&mutex);
             if (!im->flags.pending)
               {
                  im->flags.pending = 1;
                  pending = eina_list_append(pending, im);
               }
             pthread_mutex_unlock(&mutex);
             return;
          }
	pthread_mutex_unlock(&mutex);
#endif

	if (im->flags.dirty)
	  {
	     _evas_cache_image_entry_delete(cache, im);
	     return;
	  }
        
        _evas_cache_image_remove_activ(cache, im);
	_evas_cache_image_make_inactiv(cache, im, im->cache_key);
	evas_cache_image_flush(cache);
     }
}

EAPI void
evas_cache_image_data_not_needed(Image_Entry *im)
{
   Evas_Cache_Image    *cache;

   assert(im);
   assert(im->cache);

   cache = im->cache;

   if (im->references > 1) return ;
   if (im->flags.dirty || !im->flags.need_data) return ;

   _evas_cache_image_activ_lru_nodata(cache, im);
}

EAPI Image_Entry *
evas_cache_image_dirty(Image_Entry *im, int x, int y, int w, int h)
{
   Image_Entry          *im_dirty = im;
   Evas_Cache_Image     *cache;

   assert(im);
   assert(im->cache);

   cache = im->cache;
   if (!(im->flags.dirty))
     {
#ifndef EVAS_CSERVE
// if ref 1 also copy if using shared cache as its read-only
        if (im->references == 1) im_dirty = im;
        else
#endif
          {
             int        error;

             im_dirty = evas_cache_image_copied_data
               (cache, im->w, im->h, 
                evas_cache_image_pixels(im), 
                im->flags.alpha, 
                im->space);
             if (!im_dirty) goto on_error;
             if (cache->func.debug)
               cache->func.debug("dirty-src", im);
             error = cache->func.dirty(im_dirty, im);
             if (cache->func.debug)
               cache->func.debug("dirty-out", im_dirty);
/*             
             im_dirty = _evas_cache_image_entry_new(cache, NULL, im->timestamp, im->file, im->key, &im->load_opts, &error);
             if (!im_dirty) goto on_error;

             if (cache->func.debug)
               cache->func.debug("dirty-src", im);
             error = cache->func.dirty(im_dirty, im);
             if (cache->func.debug)
               cache->func.debug("dirty-out", im_dirty);

             if (error != 0) goto on_error;
 */
             im_dirty->references = 1;

             evas_cache_image_drop(im);
          }

	_evas_cache_image_remove_activ(cache, im_dirty);
        _evas_cache_image_make_dirty(cache, im_dirty);
     }

   if (cache->func.debug)
     cache->func.debug("dirty-region", im_dirty);
   if (cache->func.dirty_region)
     cache->func.dirty_region(im_dirty, x, y, w, h);

   return im_dirty;

 on_error:
   if (im_dirty) _evas_cache_image_entry_delete(cache, im_dirty);
   evas_cache_image_drop(im);
   return NULL;
}

EAPI Image_Entry *
evas_cache_image_alone(Image_Entry *im)
{
   Evas_Cache_Image     *cache;
   Image_Entry          *im_dirty = im;

   assert(im);
   assert(im->cache);

   cache = im->cache;
   if (im->references == 1)
     {
        if (!(im->flags.dirty))
          {
             _evas_cache_image_remove_activ(cache, im);
             _evas_cache_image_make_dirty(cache, im);
          }
     }
   else
     {
        int     error;

        im_dirty = evas_cache_image_copied_data
          (cache, im->w, im->h, 
           evas_cache_image_pixels(im), 
           im->flags.alpha, 
           im->space);
        if (!im_dirty) goto on_error;
        if (cache->func.debug)
          cache->func.debug("dirty-src", im);
        error = cache->func.dirty(im_dirty, im);
        if (cache->func.debug)
          cache->func.debug("dirty-out", im_dirty);
/*        
        im_dirty = _evas_cache_image_entry_new(cache, NULL, im->timestamp, im->file, im->key, &im->load_opts, &error);
        if (!im_dirty) goto on_error;

        if (cache->func.debug)
          cache->func.debug("dirty-src", im);
        error = cache->func.dirty(im_dirty, im);
        if (cache->func.debug)
          cache->func.debug("dirty-out", im_dirty);

        if (error != 0) goto on_error;
 */
        im_dirty->references = 1;

        evas_cache_image_drop(im);
     }

   return im_dirty;
   
   on_error:
   if (im_dirty) _evas_cache_image_entry_delete(cache, im_dirty);
   evas_cache_image_drop(im);
   return NULL;
}

EAPI Image_Entry *
evas_cache_image_copied_data(Evas_Cache_Image *cache, int w, int h, DATA32 *image_data, int alpha, int cspace)
{
   Image_Entry  *im;

   assert(cache);

   if ((cspace == EVAS_COLORSPACE_YCBCR422P601_PL) ||
       (cspace == EVAS_COLORSPACE_YCBCR422P709_PL))
     w &= ~0x1;

   im = _evas_cache_image_entry_new(cache, NULL, 0, NULL, NULL, NULL, NULL);
   if (!im) return NULL;

   im->space = cspace;
   im->flags.alpha = alpha;

   _evas_cache_image_entry_surface_alloc(cache, im, w, h);

   if (cache->func.copied_data(im, w, h, image_data, alpha, cspace) != 0)
     {
        _evas_cache_image_entry_delete(cache, im);
        return NULL;
     }
   im->references = 1;

   if (cache->func.debug)
     cache->func.debug("copied-data", im);
   return im;
}

EAPI Image_Entry *
evas_cache_image_data(Evas_Cache_Image *cache, int w, int h, DATA32 *image_data, int alpha, int cspace)
{
   Image_Entry  *im;

   assert(cache);

   if ((cspace == EVAS_COLORSPACE_YCBCR422P601_PL) ||
       (cspace == EVAS_COLORSPACE_YCBCR422P709_PL))
     w &= ~0x1;

   im = _evas_cache_image_entry_new(cache, NULL, 0, NULL, NULL, NULL, NULL);
   im->w = w;
   im->h = h;
   im->flags.alpha = alpha;

   if (cache->func.data(im, w, h, image_data, alpha, cspace) != 0)
     {
        _evas_cache_image_entry_delete(cache, im);
        return NULL;
     }
   im->references = 1;

   if (cache->func.debug)
     cache->func.debug("data", im);
   return im;
}

EAPI void
evas_cache_image_surface_alloc(Image_Entry *im, int w, int h)
{
   Evas_Cache_Image     *cache;

   assert(im);
   assert(im->cache);

   cache = im->cache;

   if ((im->space == EVAS_COLORSPACE_YCBCR422P601_PL) ||
       (im->space == EVAS_COLORSPACE_YCBCR422P709_PL))
     w &= ~0x1;

   _evas_cache_image_entry_surface_alloc(cache, im, w, h);

   if (cache->func.debug)
     cache->func.debug("surface-alloc", im);
}

EAPI Image_Entry *
evas_cache_image_size_set(Image_Entry *im, int w, int h)
{
   Evas_Cache_Image    *cache;
   Image_Entry         *new;
   int                  error;

   assert(im);
   assert(im->cache);
   assert(im->references > 0);

   if ((im->space == EVAS_COLORSPACE_YCBCR422P601_PL) ||
       (im->space == EVAS_COLORSPACE_YCBCR422P709_PL))
     w &= ~0x1;

   if ((im->w == w) && (im->h == h))
     return im;

   cache = im->cache;

   new = _evas_cache_image_entry_new(cache, NULL, 0, NULL, NULL, NULL, &error);
   if (!new) goto on_error;

   new->flags.alpha = im->flags.alpha;
   new->space = im->space;
   new->load_opts = im->load_opts;

   _evas_cache_image_entry_surface_alloc(cache, new, w, h);

   error = cache->func.size_set(new, im, w, h);
   if (error != 0) goto on_error;

   new->references = 1;

   evas_cache_image_drop(im);

   if (cache->func.debug)
     cache->func.debug("size_set", new);

   return new;

 on_error:
   if (new) _evas_cache_image_entry_delete(cache, new);
   evas_cache_image_drop(im);
   return NULL;
}

EAPI void
evas_cache_image_load_data(Image_Entry *im)
{
   Evas_Cache_Image    *cache;
   int                  error, preload;

   assert(im);
   assert(im->cache);
   cache = im->cache;

   if (im->flags.loaded) return;
#ifdef BUILD_ASYNC_PRELOAD
   pthread_mutex_lock(&mutex);
   preload = im->flags.preload;
   /* We check a first time, to prevent useless lock. */
   if (preload)
     {
        if (current == im)
          {
             /* Wait until ie is processed. */
             pthread_cond_wait(&cond_done, &mutex);
          }
     }
   pthread_mutex_unlock(&mutex);
//   _evas_cache_image_entry_preload_remove(im, NULL);
   if (im->flags.loaded) return ;
   LKL(im->lock);
#endif

   error = cache->func.load(im);
#ifdef BUILD_ASYNC_PRELOAD
   LKU(im->lock);
#endif

   if (cache->func.debug)
     cache->func.debug("load", im);
#ifdef BUILD_ASYNC_PRELOAD
   if (preload)
     _evas_cache_image_async_call(im);
#endif

   if (error)
     {
        _evas_cache_image_entry_surface_alloc(cache, im, im->w, im->h);
        im->flags.loaded = 0;
        return ;
     }

   im->flags.loaded = 1;
}

EAPI void
evas_cache_image_preload_data(Image_Entry *im, const void *target)
{
#ifdef BUILD_ASYNC_PRELOAD
   Evas_Cache_Image    *cache;

   assert(im);
   assert(im->cache);

   if (im->flags.loaded)
     {
	evas_async_events_put(target, EVAS_CALLBACK_IMAGE_PRELOADED, NULL,
			      (void (*)(void*, Evas_Callback_Type, void*))evas_object_event_callback_call);
	return ;
     }

   cache = im->cache;

   if (!_evas_cache_image_entry_preload_add(im, target))
     evas_async_events_put(target, EVAS_CALLBACK_IMAGE_PRELOADED, NULL,
			   (void (*)(void*, Evas_Callback_Type, void*))evas_object_event_callback_call);
#else
   evas_cache_image_load_data(im);

   evas_async_events_put(target, EVAS_CALLBACK_IMAGE_PRELOADED, NULL,
			 (void (*)(void*, Evas_Callback_Type, void*))evas_object_event_callback_call);
#endif
}

EAPI void
evas_cache_image_preload_cancel(Image_Entry *im, const void *target)
{
#ifdef BUILD_ASYNC_PRELOAD
   Evas_Cache_Image    *cache;

   assert(im);
   assert(im->cache);
   cache = im->cache;

   _evas_cache_image_entry_preload_remove(im, target);
#else
   (void) im;
#endif
}

EAPI int
evas_cache_image_flush(Evas_Cache_Image *cache)
{
   assert(cache);

   if (cache->limit == -1)
     return -1;

   while ((cache->lru) && (cache->limit < cache->usage))
     {
        Image_Entry     *im;

        im = (Image_Entry *) cache->lru->last;
        _evas_cache_image_entry_delete(cache, im);
     }

   while ((cache->lru_nodata) && (cache->limit < cache->usage))
     {
        Image_Entry     *im;

        im = (Image_Entry *) cache->lru_nodata->last;
        _evas_cache_image_remove_lru_nodata(cache, im);

        cache->func.surface_delete(im);

        im->flags.loaded = 0;
     }

   return cache->usage;
}

EAPI Image_Entry *
evas_cache_image_empty(Evas_Cache_Image *cache)
{
   Image_Entry  *new;

   new = _evas_cache_image_entry_new(cache, NULL, 0, NULL, NULL, NULL, NULL);
   if (!new) return NULL;

   new->references = 1;

   return new;
}

EAPI void
evas_cache_image_colorspace(Image_Entry *im, int cspace)
{
   Evas_Cache_Image    *cache;

   assert(im);
   assert(im->cache);

   cache = im->cache;

   if (im->space == cspace) return ;

   im->space = cspace;
   cache->func.color_space(im, cspace);
}

EAPI void *
evas_cache_private_from_image_entry_get(Image_Entry *im)
{
   Evas_Cache_Image	*cache;

   assert(im);
   assert(im->cache);

   cache = im->cache;

   return (void*) cache->data;
}

EAPI void *
evas_cache_private_get(Evas_Cache_Image *cache)
{
   assert(cache);

   return cache->data;
}

EAPI void
evas_cache_private_set(Evas_Cache_Image *cache, const void *data)
{
   assert(cache);

   cache->data = (void *)data;
}

EAPI DATA32 *
evas_cache_image_pixels(Image_Entry *im)
{
   Evas_Cache_Image    *cache;

   assert(im);
   assert(im->cache);

   cache = im->cache;

   return cache->func.surface_pixels(im);
}
