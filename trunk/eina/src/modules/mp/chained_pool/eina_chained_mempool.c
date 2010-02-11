/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
/* EINA - EFL data type library
 * Copyright (C) 2008 Cedric BAIL
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
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#ifdef EFL_HAVE_PTHREAD
#include <pthread.h>
#endif

#include "eina_inlist.h"
#include "eina_error.h"
#include "eina_module.h"
#include "eina_mempool.h"
#include "eina_trash.h"

#include "eina_private.h"

#ifdef DEBUG
#include "eina_log.h"

static int _eina_mempool_log_dom = -1;

#ifdef INF
#undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_eina_mempool_log_dom, __VA_ARGS__)
#endif

typedef struct _Chained_Mempool Chained_Mempool;
struct _Chained_Mempool
{
   Eina_Inlist  *first;
   const char *name;
   int item_size;
   int pool_size;
   int usage;
#ifdef EFL_HAVE_PTHREAD
   pthread_mutex_t mutex;
#endif
};

typedef struct _Chained_Pool Chained_Pool;
struct _Chained_Pool
{
   EINA_INLIST;
   Eina_Trash *base;
   int usage;
};

static inline Chained_Pool *
_eina_chained_mp_pool_new(Chained_Mempool *pool)
{
   Chained_Pool *p;
   unsigned char *ptr;
   int item_alloc, i;

   item_alloc = ((pool->item_size + sizeof(void *) - 1) / sizeof(void *)) * sizeof(void *);
   p = malloc(sizeof(Chained_Pool) + (pool->pool_size * item_alloc));
   ptr = (unsigned char *) (p + 1);
   p->usage = 0;
   p->base = NULL;
   for (i = 0; i < pool->pool_size; ++i, ptr += item_alloc)
     eina_trash_push(&p->base, ptr);
   return p;
}

static inline void
_eina_chained_mp_pool_free(Chained_Pool *p)
{
   free(p);
}

static void *
eina_chained_mempool_malloc(void *data, __UNUSED__ unsigned int size)
{
   Chained_Mempool *pool = data;
   Chained_Pool *p = NULL;
   void *mem;

#ifdef EFL_HAVE_PTHREAD
   pthread_mutex_lock(&pool->mutex);
#endif

   // look 4 pool from 2nd bucket on
   EINA_INLIST_FOREACH(pool->first, p)
     {
	// base is not NULL - has a free slot
	if (p->base)
	  {
	     pool->first = eina_inlist_demote(pool->first, EINA_INLIST_GET(p));
	     break;
	  }
     }

   // we have reached the end of the list - no free pools
   if (!p)
     {
	p = _eina_chained_mp_pool_new(pool);
	if (!p)
	  {
#ifdef EFL_HAVE_PTHREAD
	     pthread_mutex_unlock(&pool->mutex);
#endif
	     return NULL;
	  }
	pool->first = eina_inlist_prepend(pool->first, EINA_INLIST_GET(p));
     }
   // Request a free pointer
   mem = eina_trash_pop(&p->base);
   // move to end - it just filled up
   if (!p->base)
     {
	pool->first = eina_inlist_demote(pool->first, EINA_INLIST_GET(p));
     }
   p->usage++;
   pool->usage++;

#ifdef EFL_HAVE_PTHREAD
   pthread_mutex_unlock(&pool->mutex);
#endif
   return mem;
}

static void
eina_chained_mempool_free(void *data, void *ptr)
{
   Chained_Mempool *pool = data;
   Chained_Pool *p;
   void *pmem;
   int item_alloc, psize;

   item_alloc = ((pool->item_size + sizeof(void *) - 1) / sizeof(void *)) * sizeof(void *);
   psize = item_alloc * pool->pool_size;
   // look 4 pool

#ifdef EFL_HAVE_PTHREAD
   pthread_mutex_lock(&pool->mutex);
#endif

   EINA_INLIST_FOREACH(pool->first, p)
     {
	// pool mem base
	pmem = (void *)(((unsigned char *)p) + sizeof(Chained_Pool));
	// is it in pool mem?
	if ((ptr >= pmem) && ((unsigned char *)ptr < (((unsigned char *)pmem) + psize)))
	  {
	     // freed node points to prev free node
	     eina_trash_push(&p->base, ptr);
	     // next free node is now the one we freed
	     p->usage--;
	     pool->usage--;
	     if (p->usage == 0)
	       {
		  // free bucket
		  pool->first = eina_inlist_remove(pool->first, EINA_INLIST_GET(p));
		  _eina_chained_mp_pool_free(p);
	       }
	     else
	       {
		  // move to front
		  pool->first = eina_inlist_promote(pool->first, EINA_INLIST_GET(p));
	       }
	     break;
	  }
     }

#ifdef EFL_HAVE_PTHREAD
   pthread_mutex_unlock(&pool->mutex);
#endif
}

static void*
eina_chained_mempool_realloc(__UNUSED__ void *data, __UNUSED__ void *element, __UNUSED__ unsigned int size)
{
   return NULL;
}

static void*
eina_chained_mempool_init(const char *context, __UNUSED__ const char *option, va_list args)
{
   Chained_Mempool *mp;
   size_t length;

   length = context ? strlen(context) + 1 : 0;

   mp = calloc(1, sizeof(Chained_Mempool) + length);
   if (!mp) return NULL;

   mp->item_size = va_arg(args, int);
   mp->pool_size = va_arg(args, int);

   if (length)
     {
	mp->name = (const char*) (mp + 1);
	memcpy((char*) mp->name, context, length);
     }

#ifdef EFL_HAVE_PTHREAD
   pthread_mutex_init(&mp->mutex, NULL);
#endif

   return mp;
}

static void
eina_chained_mempool_shutdown(void *data)
{
   Chained_Mempool *mp;

   mp = (Chained_Mempool *) data;

   while (mp->first)
     {
	Chained_Pool *p = (Chained_Pool *) mp->first;

#ifdef DEBUG
	if (p->usage > 0)
	  INF("Bad news we are destroying not an empty mempool [%s]\n", mp->name);
#endif

	mp->first = eina_inlist_remove(mp->first, mp->first);
	_eina_chained_mp_pool_free(p);
     }

#ifdef EFL_HAVE_PTHREAD
   pthread_mutex_destroy(&mp->mutex);
#endif

   free(mp);
}

static Eina_Mempool_Backend _eina_chained_mp_backend = {
   "chained_mempool",
   &eina_chained_mempool_init,
   &eina_chained_mempool_free,
   &eina_chained_mempool_malloc,
   &eina_chained_mempool_realloc,
   NULL,
   NULL,
   &eina_chained_mempool_shutdown
};

Eina_Bool chained_init(void)
{
#ifdef DEBUG
   _eina_mempool_log_dom = eina_log_domain_register("eina_mempool", EINA_LOG_COLOR_DEFAULT);
   if (_eina_mempool_log_dom < 0)
     {
	EINA_LOG_ERR("Could not register log domain: eina_mempool");
	return EINA_FALSE;
     }
#endif
   return eina_mempool_register(&_eina_chained_mp_backend);
}

void chained_shutdown(void)
{
   eina_mempool_unregister(&_eina_chained_mp_backend);
#ifdef DEBUG
   eina_log_domain_unregister(_eina_mempool_log_dom);
   _eina_mempool_log_dom = -1;
#endif
}

#ifndef EINA_STATIC_BUILD_CHAINED_POOL

EINA_MODULE_INIT(chained_init);
EINA_MODULE_SHUTDOWN(chained_shutdown);

#endif /* ! EINA_STATIC_BUILD_CHAINED_POOL */
