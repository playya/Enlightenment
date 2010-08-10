/* EINA - EFL data type library
 * Copyright (C) 2008 Cedric Bail
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

#ifndef EINA_ARRAY_H_
#define EINA_ARRAY_H_

#include <stdlib.h>

#include "eina_config.h"

#include "eina_types.h"
#include "eina_error.h"
#include "eina_iterator.h"
#include "eina_accessor.h"
#include "eina_magic.h"

/**
 * @addtogroup Eina_Data_Types_Group Data Types
 *
 * @{
 */

/**
 * @addtogroup Eina_Containers_Group Containers
 *
 * @{
 */

/**
 * @defgroup Eina_Array_Group Array
 *
 * @{
 */

/**
 * @typedef Eina_Array
 * Type for a generic vector.
 */
typedef struct _Eina_Array Eina_Array;

/**
 * @typedef Eina_Array_Iterator
 * Type for an iterator on arrays, used with #EINA_ARRAY_ITER_NEXT.
 */
typedef void **Eina_Array_Iterator;

/**
 * @struct _Eina_Array
 * Type for an array of data.
 */
struct _Eina_Array
{
   void **data; /**< Pointer to a vector of pointer to payload */
   unsigned int total; /**< Total number of slots in the vector */
   unsigned int count; /**< Number of active slots in the vector */
   unsigned int step; /**< How much must we grow the vector when it is full */
#ifdef EINA_RWLOCKS_ENABLED
   pthread_rwlock_t lock;
#endif

   EINA_MAGIC
};

EAPI Eina_Array *               eina_array_new(unsigned int step) EINA_WARN_UNUSED_RESULT EINA_MALLOC EINA_WARN_UNUSED_RESULT;
EAPI void                       eina_array_free(Eina_Array *array) EINA_ARG_NONNULL(1);
EAPI void                       eina_array_step_set(Eina_Array *array, unsigned int step) EINA_ARG_NONNULL(1);
EAPI void                       eina_array_clean(Eina_Array *array) EINA_ARG_NONNULL(1);
EAPI void                       eina_array_flush(Eina_Array *array) EINA_ARG_NONNULL(1);
EAPI Eina_Bool                  eina_array_remove(Eina_Array *array, Eina_Bool(*keep)(void *data, void *gdata), void *gdata) EINA_ARG_NONNULL(1, 2);

static inline Eina_Bool         eina_array_push(Eina_Array *array, const void *data) EINA_ARG_NONNULL(1, 2);
static inline void *            eina_array_pop(Eina_Array *array) EINA_ARG_NONNULL(1);
static inline void *            eina_array_data_get(Eina_Array *array, unsigned int idx) EINA_ARG_NONNULL(1);
static inline void              eina_array_data_set(Eina_Array *array, unsigned int idx, const void *data) EINA_ARG_NONNULL(1, 3);
static inline unsigned int      eina_array_count_get(Eina_Array *array) EINA_ARG_NONNULL(1);

EAPI Eina_Iterator *            eina_array_iterator_new(Eina_Array *array) EINA_MALLOC EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
EAPI Eina_Accessor *            eina_array_accessor_new(Eina_Array *array) EINA_MALLOC EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @def EINA_ARRAY_ITER_NEXT
 * @brief Macro to iterate over an array easily.
 *
 * @param array The array to iterate over.
 * @param index The integer number that is increased while itareting.
 * @param item The data
 * @param iterator The iterator
 *
 * This macro allows the iteration over @p array in an easy way. It
 * iterates from the first element to the last one. @p index is an
 * integer that increases from 0 to the number of elements. @p item is
 * the data of each element of @p array, so it is a pointer to a type
 * chosen by the user. @p iterator is of type #Eina_Array_Iterator.
 *
 * This macro can be used for freeing the data of an array, like in
 * the following example:
 *
 * @code
 * Eina_Array         *array;
 * char               *item;
 * Eina_Array_Iterator iterator;
 * unsigned int        i;
 *
 * // array is already filled,
 * // its elements are just duplicated strings,
 * // EINA_ARRAY_ITER_NEXT will be used to free those strings
 *
 * EINA_ARRAY_ITER_NEXT(array, i, item, iterator)
 *   free(item);
 * @endcode
 */
#define EINA_ARRAY_ITER_NEXT(array, index, item, iterator)              \
   for (index = 0, iterator = (array)->data; \
        (index < eina_array_count_get(array)) && ((item = *((iterator)++))); \
                                                   ++(index))

#ifdef EINA_RWLOCKS_ENABLED
/**
 * @def EINA_ARRAY_THREADSAFE_ITER_NEXT
 * @brief Macro to iterate over an array easily while mutexing.
 *
 * @param array The array to iterate over.
 * @param index The integer number that is increased while itareting.
 * @param item The data
 * @param iterator The iterator
 * @param code The code in the iterator loop
 *
 * This macro allows the iteration over @p array in an easy way. It
 * iterates from the first element to the last one. @p index is an
 * integer that increases from 0 to the number of elements. @p item is
 * the data of each element of @p array, so it is a pointer to a type
 * chosen by the user. @p iterator is of type #Eina_Array_Iterator.
 * @p code is the entire chunk of code which will be in the iterator loop,
 * terminated by a semicolon.
 *
 * This macro can be used for safely freeing the data of an array in a thread,
 * like in the following example:
 *
 * @code
 * Eina_Array         *array;
 * char               *item;
 * Eina_Array_Iterator iterator;
 * unsigned int        i;
 *
 * // array is already filled,
 * // its elements are just duplicated strings,
 * // EINA_ARRAY_ITER_NEXT will be used to free those strings
 *
 * EINA_ARRAY_THREADSAFE_ITER_NEXT(array, i, item, iterator,
 *   {
 *      if (item)
 *        free(item);
 *   }
 * );
 * @endcode
 */
#define EINA_ARRAY_THREADSAFE_ITER_NEXT(array, index, item, iterator, code...)   \
   if (_eina_array_threadsafety)    \
     pthread_rwlock_wrlock(&(array)->lock); \
   for (index = 0, iterator = (array)->data; \
        (index < (array)->count) && ((item = *((iterator)++))); \
                                                   ++(index)) \
        code \
   if (_eina_array_threadsafety)    \
     pthread_rwlock_unlock(&(array)->lock)

#else
#define EINA_ARRAY_THREADSAFE_ITER_NEXT(array, index, item, iterator, code...)   \
  do \
    { \
       for (index = 0, iterator = (array)->data; \
            (index < (array)->count) && ((item = *((iterator)++))); \
                                                       ++(index)) \
            code \
    } \
  while (0)
#endif

#ifdef EINA_RWLOCKS_ENABLED

/**
 * @def EINA_ARRAY_THREADSAFE_ITER_RETURN
 * @brief Macro to perform a return while using EINA_ARRAY_THREADSAFE_ITER_NEXT
 *
 * @param array The array being iterated over.
 * @param retval The value to be returned
 *
 * This macro should be used any time the user wishes to perform a return
 * statement while using EINA_ARRAY_THREADSAFE_ITER_RETURN to unlock any mutexes
 * which may have been locked while iterating.  Failure to use this will likely
 * result in a deadlock.
 *
 * example:
 *
 * @code
 * Eina_Array         *array;
 * char               *item;
 * Eina_Array_Iterator iterator;
 * unsigned int        i;
 *
 * // array is already filled,
 * // its elements are just duplicated strings,
 * // EINA_ARRAY_ITER_NEXT will be used to free those strings
 *
 * EINA_ARRAY_THREADSAFE_ITER_NEXT(array, i, item, iterator,
 *   {
 *      if (item)
 *        free(item);
 *      else
 *        EINA_ARRAY_THREADSAFE_ITER_RETURN(array, NULL);
  *   }
 * );
 * @endcode
 */
#define EINA_ARRAY_THREADSAFE_ITER_RETURN(array, retval) \
do \
  { \
     if (_eina_array_threadsafety)    \
       pthread_rwlock_unlock(&(array)->lock); \
     return (retval); \
  } \
while (0)

#else

#define EINA_ARRAY_THREADSAFE_ITER_RETURN(array, retval) \
     return (retval)

#endif

#include "eina_inline_array.x"

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

#endif
