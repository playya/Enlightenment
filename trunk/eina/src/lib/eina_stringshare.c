/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
/* EINA - EFL data type library
 * Copyright (C) 2002,2003,2004,2005,2006,2007,2008,2010
 *			   Carsten Haitzler,
 *                         Jorge Luis Zapata Muga,
 *                         Cedric Bail,
 *                         Gustavo Sverzut Barbieri
 *                         Brett Nash
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
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 * Copyright (C) 2008 Peter Wehrfritz
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies of the Software and its Copyright notices. In addition publicly
 *  documented acknowledgment must be given that this software has been used if no
 *  source code of this software is made available publicly. This includes
 *  acknowledgments in either Copyright notices, Manuals, Publicity and Marketing
 *  documents or any documentation provided with any product containing this
 *  software. This License does not apply to any software that links to the
 *  libraries provided by this software (statically or dynamically), but only to
 *  the software provided.
 *
 *  Please see the OLD-COPYING.PLAIN for a plain-english explanation of this notice
 *  and it's intent.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *  THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 *  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 *  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


/**
 * @page tutorial_stringshare_page Stringshare Tutorial
 *
 * to be written...
 *
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#ifdef EFL_HAVE_POSIX_THREADS
# include <pthread.h>
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "eina_config.h"
#include "eina_private.h"
#include "eina_hash.h"
#include "eina_rbtree.h"
#include "eina_error.h"

/* undefs EINA_ARG_NONULL() so NULL checks are not compiled out! */
#include "eina_safety_checks.h"
#include "eina_stringshare.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

#define EINA_STRINGSHARE_BUCKETS 256
#define EINA_STRINGSHARE_MASK 0xFF

static const char EINA_MAGIC_STRINGSHARE_STR[] = "Eina Stringshare";
static const char EINA_MAGIC_STRINGSHARE_HEAD_STR[] = "Eina Stringshare Head";
static const char EINA_MAGIC_STRINGSHARE_NODE_STR[] = "Eina Stringshare Node";


#define EINA_MAGIC_CHECK_STRINGSHARE_HEAD(d, unlock, ...)	\
  do {								\
    if (!EINA_MAGIC_CHECK((d), EINA_MAGIC_STRINGSHARE_HEAD))	\
    {								\
        EINA_MAGIC_FAIL((d), EINA_MAGIC_STRINGSHARE_HEAD);	\
        unlock;							\
        return __VA_ARGS__;					\
    }								\
  } while (0)

#define EINA_MAGIC_CHECK_STRINGSHARE_NODE(d, unlock)		\
  do {								\
    if (!EINA_MAGIC_CHECK((d), EINA_MAGIC_STRINGSHARE_NODE))	\
    {								\
      unlock;							\
      EINA_MAGIC_FAIL((d), EINA_MAGIC_STRINGSHARE_NODE);	\
    }								\
  } while (0)

typedef struct _Eina_Stringshare             Eina_Stringshare;
typedef struct _Eina_Stringshare_Node        Eina_Stringshare_Node;
typedef struct _Eina_Stringshare_Head        Eina_Stringshare_Head;

struct _Eina_Stringshare
{
   Eina_Stringshare_Head *buckets[EINA_STRINGSHARE_BUCKETS];

   EINA_MAGIC
};

struct _Eina_Stringshare_Node
{
   Eina_Stringshare_Node *next;

   EINA_MAGIC

   unsigned int           length;
   unsigned int           references;
   char                   str[];
};

struct _Eina_Stringshare_Head
{
   EINA_RBTREE;
   EINA_MAGIC

   int                    hash;

#ifdef EINA_STRINGSHARE_USAGE
   int                    population;
#endif

   Eina_Stringshare_Node *head;
   Eina_Stringshare_Node  builtin_node;
};

static Eina_Stringshare *share = NULL;
static int _eina_stringshare_log_dom = -1;

#ifdef CRITICAL
#undef CRITICAL
#endif
#define CRITICAL(...) EINA_LOG_DOM_CRIT(_eina_stringshare_log_dom, __VA_ARGS__)

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_eina_stringshare_log_dom, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_eina_stringshare_log_dom, __VA_ARGS__)



#ifdef EFL_HAVE_THREADS
static Eina_Bool _stringshare_threads_activated = EINA_FALSE;

# ifdef EFL_HAVE_POSIX_THREADS
/* string < 4 */
static pthread_mutex_t _mutex_small = PTHREAD_MUTEX_INITIALIZER;
/* string >= 4 */
static pthread_mutex_t _mutex_big = PTHREAD_MUTEX_INITIALIZER;
#  define STRINGSHARE_LOCK_SMALL() if(_stringshare_threads_activated) pthread_mutex_lock(&_mutex_small)
#  define STRINGSHARE_UNLOCK_SMALL() if(_stringshare_threads_activated) pthread_mutex_unlock(&_mutex_small)
#  define STRINGSHARE_LOCK_BIG() if(_stringshare_threads_activated) pthread_mutex_lock(&_mutex_big)
#  define STRINGSHARE_UNLOCK_BIG() if(_stringshare_threads_activated) pthread_mutex_unlock(&_mutex_big)
# else /* EFL_HAVE_WIN32_THREADS */
static HANDLE _mutex_small = NULL;
static HANDLE _mutex_big = NULL;
#  define STRINGSHARE_LOCK_SMALL() if(_stringshare_threads_activated) WaitForSingleObject(_mutex_small, INFINITE)
#  define STRINGSHARE_UNLOCK_SMALL() if(_stringshare_threads_activated) ReleaseMutex(_mutex_small)
#  define STRINGSHARE_LOCK_BIG() if(_stringshare_threads_activated) WaitForSingleObject(_mutex_big, INFINITE)
#  define STRINGSHARE_UNLOCK_BIG() if(_stringshare_threads_activated) ReleaseMutex(_mutex_big)

# endif /* EFL_HAVE_WIN32_THREADS */
#else /* EFL_HAVE_THREADS */
# define STRINGSHARE_LOCK_SMALL() do {} while (0)
# define STRINGSHARE_UNLOCK_SMALL() do {} while (0)
# define STRINGSHARE_LOCK_BIG() do {} while (0)
# define STRINGSHARE_UNLOCK_BIG() do {} while (0)
#endif


static const unsigned char _eina_stringshare_single[512] = {
  0,0,1,0,2,0,3,0,4,0,5,0,6,0,7,0,8,0,9,0,10,0,11,0,12,0,13,0,14,0,15,0,
  16,0,17,0,18,0,19,0,20,0,21,0,22,0,23,0,24,0,25,0,26,0,27,0,28,0,29,0,30,0,
  31,0,32,0,33,0,34,0,35,0,36,0,37,0,38,0,39,0,40,0,41,0,42,0,43,0,44,0,45,0,
  46,0,47,0,48,0,49,0,50,0,51,0,52,0,53,0,54,0,55,0,56,0,57,0,58,0,59,0,60,0,
  61,0,62,0,63,0,64,0,65,0,66,0,67,0,68,0,69,0,70,0,71,0,72,0,73,0,74,0,75,0,
  76,0,77,0,78,0,79,0,80,0,81,0,82,0,83,0,84,0,85,0,86,0,87,0,88,0,89,0,90,0,
  91,0,92,0,93,0,94,0,95,0,96,0,97,0,98,0,99,0,100,0,101,0,102,0,103,0,104,0,105,0,
  106,0,107,0,108,0,109,0,110,0,111,0,112,0,113,0,114,0,115,0,116,0,117,0,118,0,119,0,120,0,
  121,0,122,0,123,0,124,0,125,0,126,0,127,0,128,0,129,0,130,0,131,0,132,0,133,0,134,0,135,0,
  136,0,137,0,138,0,139,0,140,0,141,0,142,0,143,0,144,0,145,0,146,0,147,0,148,0,149,0,150,0,
  151,0,152,0,153,0,154,0,155,0,156,0,157,0,158,0,159,0,160,0,161,0,162,0,163,0,164,0,165,0,
  166,0,167,0,168,0,169,0,170,0,171,0,172,0,173,0,174,0,175,0,176,0,177,0,178,0,179,0,180,0,
  181,0,182,0,183,0,184,0,185,0,186,0,187,0,188,0,189,0,190,0,191,0,192,0,193,0,194,0,195,0,
  196,0,197,0,198,0,199,0,200,0,201,0,202,0,203,0,204,0,205,0,206,0,207,0,208,0,209,0,210,0,
  211,0,212,0,213,0,214,0,215,0,216,0,217,0,218,0,219,0,220,0,221,0,222,0,223,0,224,0,225,0,
  226,0,227,0,228,0,229,0,230,0,231,0,232,0,233,0,234,0,235,0,236,0,237,0,238,0,239,0,240,0,
  241,0,242,0,243,0,244,0,245,0,246,0,247,0,248,0,249,0,250,0,251,0,252,0,253,0,254,0,255,0
};

typedef struct _Eina_Stringshare_Small        Eina_Stringshare_Small;
typedef struct _Eina_Stringshare_Small_Bucket Eina_Stringshare_Small_Bucket;

struct _Eina_Stringshare_Small_Bucket
{
   /* separate arrays for faster lookups */
   const char    **strings;
   unsigned char  *lengths;
   unsigned short *references;
   int count;
   int size;
};

struct _Eina_Stringshare_Small
{
   Eina_Stringshare_Small_Bucket *buckets[256];
};

#define EINA_STRINGSHARE_SMALL_BUCKET_STEP 8
static Eina_Stringshare_Small _eina_small_share;


#ifdef EINA_STRINGSHARE_USAGE
typedef struct _Eina_Stringshare_Population Eina_Stringshare_Population;
struct _Eina_Stringshare_Population
{
   int count;
   int max;
};

static Eina_Stringshare_Population population = { 0, 0 };

static Eina_Stringshare_Population population_group[4] =
  {
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 }
  };

static int max_node_population = 0;


static void
_eina_stringshare_population_init(void)
{
   unsigned int i;

   for (i = 0; i < sizeof (population_group) / sizeof (population_group[0]); ++i)
     {
	population_group[i].count = 0;
	population_group[i].max = 0;
     }
}

static void
_eina_stringshare_population_shutdown(void)
{
   unsigned int i;

   max_node_population = 0;
   population.count = 0;
   population.max = 0;

   for (i = 0; i < sizeof (population_group) / sizeof (population_group[0]); ++i)
     {
	population_group[i].count = 0;
	population_group[i].max = 0;
     }
}

static void
_eina_stringshare_population_stats(void)
{
   unsigned int i;

   fprintf(stderr, "eina stringshare statistic:\n");
   fprintf(stderr, " * maximum shared strings : %i\n", population.max);
   fprintf(stderr, " * maximum shared strings per node : %i\n", max_node_population);

   for (i = 0; i < sizeof (population_group) / sizeof (population_group[0]); ++i)
     fprintf(stderr, "DDD: %i strings of length %i, max strings: %i\n", population_group[i].count, i, population_group[i].max);
}

static void
_eina_stringshare_population_add(int slen)
{
   STRINGSHARE_LOCK_SMALL();
   STRINGSHARE_LOCK_BIG();

   population.count++;
   if (population.count > population.max)
     population.max = population.count;

   if (slen < 4)
     {
	population_group[slen].count++;
	if (population_group[slen].count > population_group[slen].max)
	  population_group[slen].max = population_group[slen].count;
     }

   STRINGSHARE_UNLOCK_BIG();
   STRINGSHARE_UNLOCK_SMALL();
}

static void
_eina_stringshare_population_del(int slen)
{
   STRINGSHARE_LOCK_SMALL();
   STRINGSHARE_LOCK_BIG();

   population.count--;
   if (slen < 4)
     population_group[slen].count--;

   STRINGSHARE_UNLOCK_BIG();
   STRINGSHARE_UNLOCK_SMALL();
}

static void
_eina_stringshare_population_head_init(Eina_Stringshare_Head *head)
{
   head->population = 1;
}

static void
_eina_stringshare_population_head_add(Eina_Stringshare_Head *head)
{
   head->population++;
   if (head->population > max_node_population)
     max_node_population = head->population;
}

static void
_eina_stringshare_population_head_del(Eina_Stringshare_Head *head)
{
   head->population--;
}

#else /* EINA_STRINGSHARE_USAGE undefined */

static void _eina_stringshare_population_init(void) {}
static void _eina_stringshare_population_shutdown(void) {}
static void _eina_stringshare_population_stats(void) {}
static void _eina_stringshare_population_add(__UNUSED__ int slen) {}
static void _eina_stringshare_population_del(__UNUSED__ int slen) {}
static void _eina_stringshare_population_head_init(__UNUSED__ Eina_Stringshare_Head *head) {}
static void _eina_stringshare_population_head_add(__UNUSED__ Eina_Stringshare_Head *head) {}
static void _eina_stringshare_population_head_del(__UNUSED__ Eina_Stringshare_Head *head) {}
#endif

static int
_eina_stringshare_cmp(const Eina_Stringshare_Head *ed, const int *hash, __UNUSED__ int length, __UNUSED__ void *data)
{
   EINA_MAGIC_CHECK_STRINGSHARE_HEAD(ed, , 0);

   return ed->hash - *hash;
}

static Eina_Rbtree_Direction
_eina_stringshare_node(const Eina_Stringshare_Head *left, const Eina_Stringshare_Head *right, __UNUSED__ void *data)
{
   EINA_MAGIC_CHECK_STRINGSHARE_HEAD(left, , 0);
   EINA_MAGIC_CHECK_STRINGSHARE_HEAD(right, , 0);

   if (left->hash - right->hash < 0)
     return EINA_RBTREE_LEFT;
   return EINA_RBTREE_RIGHT;
}

static void
_eina_stringshare_head_free(Eina_Stringshare_Head *ed, __UNUSED__ void *data)
{
   EINA_MAGIC_CHECK_STRINGSHARE_HEAD(ed, );

   while (ed->head)
     {
	Eina_Stringshare_Node *el = ed->head;

	ed->head = ed->head->next;
	if (el != &ed->builtin_node)
	  MAGIC_FREE(el);
     }
   MAGIC_FREE(ed);
}

static inline int
_eina_stringshare_small_cmp(const Eina_Stringshare_Small_Bucket *bucket, int i, const char *pstr, unsigned char plength)
{
   /* pstr and plength are from second char and on, since the first is
    * always the same.
    *
    * First string being always the same, size being between 2 and 3
    * characters (there is a check for special case length==1 and then
    * small stringshare is applied to strings < 4), we just need to
    * compare 2 characters of both strings.
    */
   const unsigned char cur_plength = bucket->lengths[i] - 1;
   const char *cur_pstr;

   if (cur_plength > plength)
     return 1;
   else if (cur_plength < plength)
     return -1;

   cur_pstr = bucket->strings[i] + 1;

   if (cur_pstr[0] > pstr[0])
     return 1;
   else if (cur_pstr[0] < pstr[0])
     return -1;

   if (plength == 1)
     return 0;

   if (cur_pstr[1] > pstr[1])
     return 1;
   else if (cur_pstr[1] < pstr[1])
     return -1;

   return 0;
}

static const char *
_eina_stringshare_small_bucket_find(const Eina_Stringshare_Small_Bucket *bucket, const char *str, unsigned char length, int *idx)
{
   const char *pstr = str + 1; /* skip first letter, it's always the same */
   unsigned char plength = length - 1;
   int i, low, high;

   if (bucket->count == 0)
     {
	*idx = 0;
	return NULL;
     }

   low = 0;
   high = bucket->count;

   while (low < high)
     {
	int r;

	i = (low + high - 1) / 2;

	r = _eina_stringshare_small_cmp(bucket, i, pstr, plength);
	if (r > 0)
	  {
	     high = i;
	  }
	else if (r < 0)
	  {
	     low = i + 1;
	  }
	else
	  {
	     *idx = i;
	     return bucket->strings[i];
	  }
     }

   *idx = low;
   return NULL;
}

static Eina_Bool
_eina_stringshare_small_bucket_resize(Eina_Stringshare_Small_Bucket *bucket, int size)
{
   void *tmp;

   tmp = realloc((void*)bucket->strings, size * sizeof(bucket->strings[0]));
   if (!tmp)
     {
	eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
	return 0;
     }
   bucket->strings = tmp;

   tmp = realloc(bucket->lengths, size * sizeof(bucket->lengths[0]));
   if (!tmp)
     {
	eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
	return 0;
     }
   bucket->lengths = tmp;

   tmp = realloc(bucket->references, size * sizeof(bucket->references[0]));
   if (!tmp)
     {
	eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
	return 0;
     }
   bucket->references = tmp;

   bucket->size = size;
   return 1;
}

static const char *
_eina_stringshare_small_bucket_insert_at(Eina_Stringshare_Small_Bucket **p_bucket, const char *str, unsigned char length, int idx)
{
   Eina_Stringshare_Small_Bucket *bucket = *p_bucket;
   int todo, off;
   char *snew;

   if (!bucket)
     {
	*p_bucket = bucket = calloc(1, sizeof(*bucket));
	if (!bucket)
	  {
	     eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
	     return NULL;
	  }
     }

   if (bucket->count + 1 >= bucket->size)
     {
	int size = bucket->size + EINA_STRINGSHARE_SMALL_BUCKET_STEP;
	if (!_eina_stringshare_small_bucket_resize(bucket, size))
	  return NULL;
     }

   snew = malloc(length + 1);
   if (!snew)
     {
	eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
	return NULL;
     }
   memcpy(snew, str, length);
   snew[length] = '\0';

   off = idx + 1;
   todo = bucket->count - idx;
   if (todo > 0)
     {
	memmove((void *)(bucket->strings + off), bucket->strings + idx,
		todo * sizeof(bucket->strings[0]));
	memmove(bucket->lengths + off, bucket->lengths + idx,
		todo * sizeof(bucket->lengths[0]));
	memmove(bucket->references + off, bucket->references + idx,
		todo * sizeof(bucket->references[0]));
     }

   bucket->strings[idx] = snew;
   bucket->lengths[idx] = length;
   bucket->references[idx] = 1;
   bucket->count++;

   return snew;
}

static void
_eina_stringshare_small_bucket_remove_at(Eina_Stringshare_Small_Bucket **p_bucket, int idx)
{
   Eina_Stringshare_Small_Bucket *bucket = *p_bucket;
   int todo, off;

   if (bucket->references[idx] > 1)
     {
	bucket->references[idx]--;
	return;
     }

   free((char *)bucket->strings[idx]);

   if (bucket->count == 1)
     {
	free((void *)bucket->strings);
	free(bucket->lengths);
	free(bucket->references);
	free(bucket);
	*p_bucket = NULL;
	return;
     }

   bucket->count--;
   if (idx == bucket->count)
     goto end;

   off = idx + 1;
   todo = bucket->count - idx;

   memmove((void *)(bucket->strings + idx), bucket->strings + off,
	   todo * sizeof(bucket->strings[0]));
   memmove(bucket->lengths + idx, bucket->lengths + off,
	   todo * sizeof(bucket->lengths[0]));
   memmove(bucket->references + idx, bucket->references + off,
	   todo * sizeof(bucket->references[0]));

 end:
   if (bucket->count + EINA_STRINGSHARE_SMALL_BUCKET_STEP < bucket->size)
     {
	int size = bucket->size - EINA_STRINGSHARE_SMALL_BUCKET_STEP;
	_eina_stringshare_small_bucket_resize(bucket, size);
     }
}

static const char *
_eina_stringshare_small_add(const char *str, unsigned char length)
{
   Eina_Stringshare_Small_Bucket **bucket;
   int i;

   bucket = _eina_small_share.buckets + (unsigned char)str[0];
   if (!*bucket)
     i = 0;
   else
     {
	const char *ret;
	ret = _eina_stringshare_small_bucket_find(*bucket, str, length, &i);
	if (ret)
	  {
	     (*bucket)->references[i]++;
	     return ret;
	  }
     }

   return _eina_stringshare_small_bucket_insert_at(bucket, str, length, i);
}

static void
_eina_stringshare_small_del(const char *str, unsigned char length)
{
   Eina_Stringshare_Small_Bucket **bucket;
   const char *ret;
   int i;

   bucket = _eina_small_share.buckets + (unsigned char)str[0];
   if (!*bucket)
     goto error;

   ret = _eina_stringshare_small_bucket_find(*bucket, str, length, &i);
   if (!ret)
     goto error;

   _eina_stringshare_small_bucket_remove_at(bucket, i);
   return;

 error:
   CRITICAL("EEEK trying to del non-shared stringshare \"%s\"", str);
}

static void
_eina_stringshare_small_init(void)
{
   memset(&_eina_small_share, 0, sizeof(_eina_small_share));
}

static void
_eina_stringshare_small_shutdown(void)
{
   Eina_Stringshare_Small_Bucket **p_bucket, **p_bucket_end;

   p_bucket = _eina_small_share.buckets;
   p_bucket_end = p_bucket + 256;

   for (; p_bucket < p_bucket_end; p_bucket++)
     {
	Eina_Stringshare_Small_Bucket *bucket = *p_bucket;
	char **s, **s_end;

	if (!bucket)
	  continue;

	s = (char **)bucket->strings;
	s_end = s + bucket->count;
	for (; s < s_end; s++)
	  free(*s);

	free((void *)bucket->strings);
	free(bucket->lengths);
	free(bucket->references);
	free(bucket);
	*p_bucket = NULL;
     }
}

static void
_eina_stringshare_node_init(Eina_Stringshare_Node *node, const char *str, int slen)
{
   EINA_MAGIC_SET(node, EINA_MAGIC_STRINGSHARE_NODE);
   node->references = 1;
   node->length = slen;
   memcpy(node->str, str, slen);
   node->str[slen] = '\0';
}

static Eina_Stringshare_Head *
_eina_stringshare_head_alloc(int slen)
{
   Eina_Stringshare_Head *head;
   const size_t head_size = offsetof(Eina_Stringshare_Head, builtin_node.str);

   head = malloc(head_size + slen + 1);
   if (!head)
     eina_error_set(EINA_ERROR_OUT_OF_MEMORY);

   return head;
}

static const char *
_eina_stringshare_add_head(Eina_Stringshare_Head **p_bucket, int hash, const char *str, int slen)
{
   Eina_Rbtree **p_tree = (Eina_Rbtree **)p_bucket;
   Eina_Stringshare_Head *head;

   head = _eina_stringshare_head_alloc(slen);
   if (!head)
     return NULL;

   EINA_MAGIC_SET(head, EINA_MAGIC_STRINGSHARE_HEAD);
   head->hash = hash;
   head->head = &head->builtin_node;
   _eina_stringshare_node_init(head->head, str, slen);
   head->head->next = NULL;

   _eina_stringshare_population_head_init(head);

   *p_tree = eina_rbtree_inline_insert
     (*p_tree, EINA_RBTREE_GET(head),
      EINA_RBTREE_CMP_NODE_CB(_eina_stringshare_node), NULL);

   return head->head->str;
}

static void
_eina_stringshare_del_head(Eina_Stringshare_Head **p_bucket, Eina_Stringshare_Head *head)
{
   Eina_Rbtree **p_tree = (Eina_Rbtree **)p_bucket;

   *p_tree = eina_rbtree_inline_remove
     (*p_tree, EINA_RBTREE_GET(head),
      EINA_RBTREE_CMP_NODE_CB(_eina_stringshare_node), NULL);

   MAGIC_FREE(head);
}


static inline Eina_Bool
_eina_stringshare_node_eq(const Eina_Stringshare_Node *node, const char *str, unsigned int slen)
{
   return ((node->length == slen) &&
	   (memcmp(node->str, str, slen) == 0));
}

static Eina_Stringshare_Node *
_eina_stringshare_head_find(Eina_Stringshare_Head *head, const char *str, unsigned int slen)
{
   Eina_Stringshare_Node *node, *prev;

   node = head->head;
   if (_eina_stringshare_node_eq(node, str, slen))
     return node;

   prev = node;
   node = node->next;
   for (; node != NULL; prev = node, node = node->next)
     if (_eina_stringshare_node_eq(node, str, slen))
       {
	  /* promote node, make hot items be at the beginning */
	  prev->next = node->next;
	  node->next = head->head;
	  head->head = node;
	  return node;
       }

   return NULL;
}

static Eina_Bool
_eina_stringshare_head_remove_node(Eina_Stringshare_Head *head, const Eina_Stringshare_Node *node)
{
   Eina_Stringshare_Node *cur, *prev;

   if (head->head == node)
     {
	head->head = node->next;
	return 1;
     }

   prev = head->head;
   cur = head->head->next;
   for (; cur != NULL; prev = cur, cur = cur->next)
     if (cur == node)
       {
	  prev->next = cur->next;
	  return 1;
       }

   return 0;
}

static Eina_Stringshare_Head *
_eina_stringshare_find_hash(Eina_Stringshare_Head *bucket, int hash)
{
   return (Eina_Stringshare_Head*) eina_rbtree_inline_lookup
     (EINA_RBTREE_GET(bucket), &hash, 0,
      EINA_RBTREE_CMP_KEY_CB(_eina_stringshare_cmp), NULL);
}

static Eina_Stringshare_Node *
_eina_stringshare_node_alloc(int slen)
{
   Eina_Stringshare_Node *node;
   const size_t node_size = offsetof(Eina_Stringshare_Node, str);

   node = malloc(node_size + slen + 1);
   if (!node)
     eina_error_set(EINA_ERROR_OUT_OF_MEMORY);

   return node;
}

static Eina_Stringshare_Node *
_eina_stringshare_node_from_str(const char *str)
{
   Eina_Stringshare_Node *node;
   const size_t offset = offsetof(Eina_Stringshare_Node, str);

   node = (Eina_Stringshare_Node *)(str - offset);
   EINA_MAGIC_CHECK_STRINGSHARE_NODE(node, );
   return node;
}

struct dumpinfo
{
   int used, saved, dups, unique;
};

static void
_eina_stringshare_small_bucket_dump(Eina_Stringshare_Small_Bucket *bucket, struct dumpinfo *di)
{
   const char **s = bucket->strings;
   unsigned char *l = bucket->lengths;
   unsigned short *r = bucket->references;
   int i;

   di->used += sizeof(*bucket);
   di->used += bucket->count * sizeof(*s);
   di->used += bucket->count * sizeof(*l);
   di->used += bucket->count * sizeof(*r);
   di->unique += bucket->count;

   for (i = 0; i < bucket->count; i++, s++, l++, r++)
     {
	int dups;
#ifdef _WIN32
	printf("DDD: %5hu %5hu '%s'\n", *l, *r, *s);
#else
	printf("DDD: %5hhu %5hu '%s'\n", *l, *r, *s);
#endif

	dups = (*r - 1);

	di->used += *l;
	di->saved += *l * dups;
	di->dups += dups;
     }
}

static void
_eina_stringshare_small_dump(struct dumpinfo *di)
{
   Eina_Stringshare_Small_Bucket **p_bucket, **p_bucket_end;

   p_bucket = _eina_small_share.buckets;
   p_bucket_end = p_bucket + 256;

   for (; p_bucket < p_bucket_end; p_bucket++)
     {
	Eina_Stringshare_Small_Bucket *bucket = *p_bucket;

	if (!bucket)
	  continue;

	_eina_stringshare_small_bucket_dump(bucket, di);
     }
}

static Eina_Bool
eina_iterator_array_check(const Eina_Rbtree *rbtree __UNUSED__, Eina_Stringshare_Head *head, struct dumpinfo *fdata)
{
   Eina_Stringshare_Node *node;

   STRINGSHARE_LOCK_SMALL();
   STRINGSHARE_LOCK_BIG();

   fdata->used += sizeof(Eina_Stringshare_Head);
   for (node = head->head; node; node = node->next)
     {
	printf("DDD: %5i %5i ", node->length, node->references);
	printf("'%s'\n", ((char *)node) + sizeof(Eina_Stringshare_Node));
	fdata->used += sizeof(Eina_Stringshare_Node);
	fdata->used += node->length;
	fdata->saved += (node->references - 1) * node->length;
	fdata->dups += node->references - 1;
	fdata->unique++;
     }

   STRINGSHARE_UNLOCK_BIG();
   STRINGSHARE_UNLOCK_SMALL();

   return EINA_TRUE;
}

/**
 * @endcond
 */


/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/

/**
 * @internal
 * @brief Initialize the stringshare module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function sets up the stringshare module of Eina. It is called by
 * eina_init().
 *
 * @see eina_init()
 */
Eina_Bool
eina_stringshare_init(void)
{
   _eina_stringshare_log_dom = eina_log_domain_register("eina_stringshare", EINA_LOG_COLOR_DEFAULT);
   if (_eina_stringshare_log_dom < 0)
     {
	EINA_LOG_ERR("Could not register log domain: eina_stringshare");
	return EINA_FALSE;
     }

   share = calloc(1, sizeof(Eina_Stringshare));
   if (!share)
     {
	eina_log_domain_unregister(_eina_stringshare_log_dom);
	_eina_stringshare_log_dom = -1;
	return EINA_FALSE;
     }

#define EMS(n) eina_magic_string_static_set(n, n##_STR)
   EMS(EINA_MAGIC_STRINGSHARE);
   EMS(EINA_MAGIC_STRINGSHARE_HEAD);
   EMS(EINA_MAGIC_STRINGSHARE_NODE);
#undef EMS
   EINA_MAGIC_SET(share, EINA_MAGIC_STRINGSHARE);

   _eina_stringshare_small_init();
   _eina_stringshare_population_init();
   return EINA_TRUE;
}

/**
 * @internal
 * @brief Shut down the stringshare module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function shuts down the stringshare module set up by
 * eina_stringshare_init(). It is called by eina_shutdown().
 *
 * @see eina_shutdown()
 */
Eina_Bool
eina_stringshare_shutdown(void)
{
   unsigned int i;

   STRINGSHARE_LOCK_SMALL();
   STRINGSHARE_LOCK_BIG();

   _eina_stringshare_population_stats();

   /* remove any string still in the table */
   for (i = 0; i < EINA_STRINGSHARE_BUCKETS; i++)
     {
	eina_rbtree_delete(EINA_RBTREE_GET(share->buckets[i]), EINA_RBTREE_FREE_CB(_eina_stringshare_head_free), NULL);
	share->buckets[i] = NULL;
     }
   MAGIC_FREE(share);

   _eina_stringshare_population_shutdown();
   _eina_stringshare_small_shutdown();
   eina_log_domain_unregister(_eina_stringshare_log_dom);
   _eina_stringshare_log_dom = -1;

   STRINGSHARE_UNLOCK_BIG();
   STRINGSHARE_UNLOCK_SMALL();


   return EINA_TRUE;
}

#ifdef EFL_HAVE_THREADS

/**
 * @internal
 * @brief Activate the stringshare mutexs.
 *
 * This function activate the mutexs in the eina stringshare module. It is called by
 * eina_thread_init().
 *
 * @see eina_thread_init()
 */
void
eina_stringshare_threads_init(void)
{
   _stringshare_threads_activated = EINA_TRUE;
}

/**
 * @internal
 * @brief Shut down the stringshare mutexs.
 *
 * This function shuts down the mutexs in the stringshare module.
 * It is called by eina_thread_shutdown().
 *
 * @see eina_thread_shutdown()
 */
void
eina_stringshare_threads_shutdown(void)
{
   _stringshare_threads_activated = EINA_FALSE;
}

#endif

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

/**
 * @addtogroup Eina_Stringshare_Group Stringshare
 *
 * These functions allow you to store one copy of a string, and use it
 * throughout your program.
 *
 * This is a method to reduce the number of duplicated strings kept in
 * memory. It's pretty common for the same strings to be dynamically
 * allocated repeatedly between applications and libraries, especially in
 * circumstances where you could have multiple copies of a structure that
 * allocates the string. So rather than duplicating and freeing these
 * strings, you request a read-only pointer to an existing string and
 * only incur the overhead of a hash lookup.
 *
 * It sounds like micro-optimizing, but profiling has shown this can have
 * a significant impact as you scale the number of copies up. It improves
 * string creation/destruction speed, reduces memory use and decreases
 * memory fragmentation, so a win all-around.
 *
 * For more information, you can look at the @ref tutorial_stringshare_page.
 *
 * @{
 */

/**
 * @brief Retrieve an instance of a string for use in a program.
 *
 * @param   str The string to retrieve an instance of.
 * @param   slen The string size (<= strlen(str)).
 * @return  A pointer to an instance of the string on success.
 *          @c NULL on failure.
 *
 * This function retrieves an instance of @p str. If @p str is
 * @c NULL, then @c NULL is returned. If @p str is already stored, it
 * is just returned and its reference counter is increased. Otherwise
 * it is added to the strings to be searched and a duplicated string
 * of @p str is returned.
 *
 * This function does not check string size, but uses the
 * exact given size. This can be used to stringshare part of a larger
 * buffer or substring.
 *
 * @see eina_stringshare_add()
 */
EAPI const char *
eina_stringshare_add_length(const char *str, unsigned int slen)
{
   Eina_Stringshare_Head **p_bucket, *ed;
   Eina_Stringshare_Node *el;
   int hash_num, hash;

   DBG("str=%p (%.*s), slen=%u", str, slen, str ? str : "", slen);
   if (!str) return NULL;

   _eina_stringshare_population_add(slen);

   if (slen <= 0)
     return "";
   else if (slen == 1)
     return (const char *)_eina_stringshare_single + ((*str) << 1);
   else if (slen < 4)
     {
	const char *s;

	STRINGSHARE_LOCK_SMALL();
	s = _eina_stringshare_small_add(str, slen);
	STRINGSHARE_UNLOCK_SMALL();
	return s;
     }

   hash = eina_hash_superfast(str, slen);
   hash_num = hash & 0xFF;
   hash = (hash >> 8) & EINA_STRINGSHARE_MASK;

   STRINGSHARE_LOCK_BIG();
   p_bucket = share->buckets + hash_num;

   ed = _eina_stringshare_find_hash(*p_bucket, hash);
   if (!ed)
     {
	const char *s =  _eina_stringshare_add_head(p_bucket, hash, str, slen);
	STRINGSHARE_UNLOCK_BIG();
	return s;
     }

   EINA_MAGIC_CHECK_STRINGSHARE_HEAD(ed, STRINGSHARE_UNLOCK_BIG(), NULL);

   el = _eina_stringshare_head_find(ed, str, slen);
   if (el)
     {
	EINA_MAGIC_CHECK_STRINGSHARE_NODE(el, STRINGSHARE_UNLOCK_BIG());
	el->references++;
	STRINGSHARE_UNLOCK_BIG();
	return el->str;
     }

   el = _eina_stringshare_node_alloc(slen);
   if (!el)
     {
	STRINGSHARE_UNLOCK_BIG();
	return NULL;
     }

   _eina_stringshare_node_init(el, str, slen);
   el->next = ed->head;
   ed->head = el;
   _eina_stringshare_population_head_add(ed);

   STRINGSHARE_UNLOCK_BIG();

   return el->str;
}

/**
 * @brief Retrieve an instance of a string for use in a program.
 *
 * @param   str The NULL terminated string to retrieve an instance of.
 * @return  A pointer to an instance of the string on success.
 *          @c NULL on failure.
 *
 * This function retrieves an instance of @p str. If @p str is
 * @c NULL, then @c NULL is returned. If @p str is already stored, it
 * is just returned and its reference counter is increased. Otherwise
 * it is added to the strings to be searched and a duplicated string
 * of @p str is returned.
 *
 * The string @p str must be NULL terminated ('@\0') and its full
 * length will be used. To use part of the string or non-null
 * terminated, use eina_stringshare_add_length() instead.
 *
 * @see eina_stringshare_add_length()
 */
EAPI const char *
eina_stringshare_add(const char *str)
{
   int slen;

   if (!str) return NULL;

   if      (str[0] == '\0') slen = 0;
   else if (str[1] == '\0') slen = 1;
   else if (str[2] == '\0') slen = 2;
   else if (str[3] == '\0') slen = 3;
   else                     slen = 3 + (int)strlen(str + 3);

   return eina_stringshare_add_length(str, slen);
}

/**
 * @brief Retrieve an instance of a string for use in a program
 * from a format string.
 *
 * @param   fmt The NULL terminated format string to retrieve an instance of.
 * @return  A pointer to an instance of the string on success.
 *          @c NULL on failure.
 *
 * This function retrieves an instance of @p fmt. If @p fmt is
 * @c NULL, then @c NULL is returned. If @p fmt is already stored, it
 * is just returned and its reference counter is increased. Otherwise
 * it is added to the strings to be searched and a duplicated string
 * is returned.
 *
 * The format string @p fmt must be NULL terminated ('@\0') and its full
 * length will be used. To use part of the format string or non-null
 * terminated, use eina_stringshare_nprintf() instead.
 *
 * @see eina_stringshare_nprintf()
 */
EAPI const char *
eina_stringshare_printf(const char *fmt, ...)
{
   va_list args;
   char *tmp;
   const char *ret;
   int len;

   if (!fmt) return NULL;

   va_start(args, fmt);
   len = vasprintf(&tmp, fmt, args);
   va_end(args);

   if (len < 1)
     return NULL;

   ret = eina_stringshare_add_length(tmp, len);
   free(tmp);

   return ret;
}

/**
 * @brief Retrieve an instance of a string for use in a program
 * from a format string.
 *
 * @param   fmt The NULL terminated format string to retrieve an instance of.
 * @param   args The va_args for @p fmt
 * @return  A pointer to an instance of the string on success.
 *          @c NULL on failure.
 *
 * This function retrieves an instance of @p fmt with @p args. If @p fmt is
 * @c NULL, then @c NULL is returned. If @p fmt with @p args is already stored, it
 * is just returned and its reference counter is increased. Otherwise
 * it is added to the strings to be searched and a duplicated string
 * is returned.
 *
 * The format string @p fmt must be NULL terminated ('@\0') and its full
 * length will be used. To use part of the format string or non-null
 * terminated, use eina_stringshare_nprintf() instead.
 *
 * @see eina_stringshare_nprintf()
 */
EAPI const char *
eina_stringshare_vprintf(const char *fmt, va_list args)
{
   char *tmp;
   const char *ret;
   int len;

   if (!fmt) return NULL;

   len = vasprintf(&tmp, fmt, args);

   if (len < 1)
     return NULL;

   ret = eina_stringshare_add_length(tmp, len);
   free(tmp);

   return ret;
}

/**
 * @brief Retrieve an instance of a string for use in a program
 * from a format string with size limitation.
 * @param   len The length of the format string to use
 * @param   fmt The format string to retrieve an instance of.
 * @return  A pointer to an instance of the string on success.
 *          @c NULL on failure.
 *
 * This function retrieves an instance of @p fmt limited by @p len. If @p fmt is
 * @c NULL or @p len is < 1, then @c NULL is returned. If the resulting string
 * is already stored, it is returned and its reference counter is increased. Otherwise
 * it is added to the strings to be searched and a duplicated string
 * is returned.
 *
 * @p len length of the format string will be used. To use the 
 * entire format string, use eina_stringshare_printf() instead.
 *
 * @see eina_stringshare_printf()
 */
EAPI const char *
eina_stringshare_nprintf(unsigned int len, const char *fmt, ...)
{
   va_list args;
   char *tmp;
   int size;

   if (!fmt) return NULL;
   if (len < 1) return NULL;

   tmp = alloca(sizeof(char) * len + 1);

   va_start(args, fmt);
   size = vsnprintf(tmp, len, fmt, args);
   va_end(args);

   if (size < 1)
     return NULL;

   return eina_stringshare_add_length(tmp, len);
}

/**
 * Increment references of the given shared string.
 *
 * @param str The shared string.
 * @return    A pointer to an instance of the string on success.
 *            @c NULL on failure.
 *
 * This is similar to eina_stringshare_add(), but it's faster since it will
 * avoid lookups if possible, but on the down side it requires the parameter
 * to be shared before, in other words, it must be the return of a previous
 * eina_stringshare_add().
 *
 * There is no unref since this is the work of eina_stringshare_del().
 */
EAPI const char *
eina_stringshare_ref(const char *str)
{
   Eina_Stringshare_Node *node;
   int slen;

   DBG("str=%p (%s)", str, str ? str : "");
   if (!str) return NULL;

   /* special cases */
   if      (str[0] == '\0') slen = 0;
   else if (str[1] == '\0') slen = 1;
   else if (str[2] == '\0') slen = 2;
   else if (str[3] == '\0') slen = 3;
   else                     slen = 4; /* handled later */

   if (slen < 2)
     {
	_eina_stringshare_population_add(slen);

	return str;
     }
   else if (slen < 4)
     {
	const char *s;
	_eina_stringshare_population_add(slen);

	STRINGSHARE_LOCK_SMALL();
	s =  _eina_stringshare_small_add(str, slen);
	STRINGSHARE_UNLOCK_SMALL();

	return s;
     }

   STRINGSHARE_LOCK_BIG();
   node = _eina_stringshare_node_from_str(str);
   node->references++;
   DBG("str=%p (%s) refs=%u", str, str, node->references);

   STRINGSHARE_UNLOCK_BIG();

   _eina_stringshare_population_add(node->length);

   return str;
}


/**
 * @brief Note that the given string has lost an instance.
 *
 * @param str string The given string.
 *
 * This function decreases the reference counter associated to @p str
 * if it exists. If that counter reaches 0, the memory associated to
 * @p str is freed. If @p str is NULL, the function returns
 * immediatly.
 *
 * Note that if the given pointer is not shared or NULL, bad things
 * will happen, likely a segmentation fault.
 */
EAPI void
eina_stringshare_del(const char *str)
{
   Eina_Stringshare_Head *ed;
   Eina_Stringshare_Head **p_bucket;
   Eina_Stringshare_Node *node;
   int hash_num, slen, hash;

   DBG("str=%p (%s)", str, str ? str : "");
   if (!str) return;

   /* special cases */
   if      (str[0] == '\0') slen = 0;
   else if (str[1] == '\0') slen = 1;
   else if (str[2] == '\0') slen = 2;
   else if (str[3] == '\0') slen = 3;
   else                     slen = 4; /* handled later */

   _eina_stringshare_population_del(slen);

   if (slen < 2)
     return;
   else if (slen < 4)
     {
	STRINGSHARE_LOCK_SMALL();
	_eina_stringshare_small_del(str, slen);
	STRINGSHARE_UNLOCK_SMALL();
	return;
     }

   STRINGSHARE_LOCK_BIG();

   node = _eina_stringshare_node_from_str(str);
   if (node->references > 1)
     {
	node->references--;
	DBG("str=%p (%s) refs=%u", str, str, node->references);
	STRINGSHARE_UNLOCK_BIG();
	return;
     }

   DBG("str=%p (%s) refs=0, delete.", str, str);
   node->references = 0;
   slen = node->length;

   hash = eina_hash_superfast(str, slen);
   hash_num = hash & 0xFF;
   hash = (hash >> 8) & EINA_STRINGSHARE_MASK;

   p_bucket = share->buckets + hash_num;
   ed = _eina_stringshare_find_hash(*p_bucket, hash);
   if (!ed)
     goto on_error;

   EINA_MAGIC_CHECK_STRINGSHARE_HEAD(ed, STRINGSHARE_UNLOCK_BIG());

   if (!_eina_stringshare_head_remove_node(ed, node))
     goto on_error;

   if (node != &ed->builtin_node)
     MAGIC_FREE(node);

   if (!ed->head)
     _eina_stringshare_del_head(p_bucket, ed);
   else
     _eina_stringshare_population_head_del(ed);

   STRINGSHARE_UNLOCK_BIG();

   return;

 on_error:
   STRINGSHARE_UNLOCK_BIG();
   /* possible segfault happened before here, but... */
   CRITICAL("EEEK trying to del non-shared stringshare \"%s\"", str);
}

/**
 * @brief Note that the given string @b must be shared.
 *
 * @param str the shared string to know the length. It is safe to
 *        give NULL, in that case -1 is returned.
 *
 * This function is a cheap way to known the length of a shared
 * string. Note that if the given pointer is not shared, bad
 * things will happen, likely a segmentation fault. If in doubt, try
 * strlen().
 */
EAPI int
eina_stringshare_strlen(const char *str)
{
   const Eina_Stringshare_Node *node;

   if (!str)
     return -1;

   /* special cases */
   if (str[0] == '\0') return 0;
   if (str[1] == '\0') return 1;
   if (str[2] == '\0') return 2;
   if (str[3] == '\0') return 3;

   node = _eina_stringshare_node_from_str(str);
   return node->length;
}

/**
 * @brief Dump the contents of the stringshare.
 *
 * This function dumps all strings in the stringshare to stdout with a
 * DDD: prefix per line and a memory usage summary.
 */
EAPI void
eina_stringshare_dump(void)
{
   Eina_Iterator *it;
   unsigned int i;
   struct dumpinfo di;

   if (!share) return;
   di.used = sizeof (_eina_stringshare_single);
   di.saved = 0;
   di.dups = 0;
   di.unique = 0;
   printf("DDD:   len   ref string\n");
   printf("DDD:-------------------\n");

   STRINGSHARE_LOCK_SMALL();
   _eina_stringshare_small_dump(&di);
   STRINGSHARE_UNLOCK_SMALL();

   STRINGSHARE_LOCK_BIG();
   for (i = 0; i < EINA_STRINGSHARE_BUCKETS; i++)
     {
	if (!share->buckets[i]) continue;
//	printf("DDD: BUCKET # %i (HEAD=%i, NODE=%i)\n", i,
//	       sizeof(Eina_Stringshare_Head), sizeof(Eina_Stringshare_Node));
	it = eina_rbtree_iterator_prefix((Eina_Rbtree *)share->buckets[i]);
	eina_iterator_foreach(it, EINA_EACH(eina_iterator_array_check), &di);
	eina_iterator_free(it);
     }
#ifdef EINA_STRINGSHARE_USAGE
   /* One character strings are not counted in the hash. */
   di.saved += population_group[0].count * sizeof(char);
   di.saved += population_group[1].count * sizeof(char) * 2;
#endif
   printf("DDD:-------------------\n");
   printf("DDD: usage (bytes) = %i, saved = %i (%3.0f%%)\n",
	  di.used, di.saved, di.used ? (di.saved * 100.0 / di.used) : 0.0);
   printf("DDD: unique: %d, duplicates: %d (%3.0f%%)\n",
	  di.unique, di.dups, di.unique ? (di.dups * 100.0 / di.unique) : 0.0);

#ifdef EINA_STRINGSHARE_USAGE
   printf("DDD: Allocated strings: %i\n", population.count);
   printf("DDD: Max allocated strings: %i\n", population.max);

   for (i = 0; i < sizeof (population_group) / sizeof (population_group[0]); ++i)
     fprintf(stderr, "DDD: %i strings of length %i, max strings: %i\n", population_group[i].count, i, population_group[i].max);
#endif

   STRINGSHARE_UNLOCK_BIG();
}

/**
 * @}
 */

