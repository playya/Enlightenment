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

#ifndef EINA_ITERATOR_H__
#define EINA_ITERATOR_H__

#include "eina_config.h"

#include "eina_types.h"
#include "eina_magic.h"


/**
 * @addtogroup Eina_Iterator_Group Iterator Functions
 *
 * @brief These functions manage iterators on containers.
 *
 * These functions allow to access elements of a container in a
 * generic way, without knowing which container is used (a bit like
 * iterators in the C++ STL). Iterators only allows sequential access
 * (that is, from an element to the next one). For random access, see
 * @ref Eina_Accessor_Group.
 *
 * An iterator is created from container data types, so no creation
 * function is available here. An iterator is deleted with
 * eina_iterator_free(). To get the data and iterate, use
 * eina_iterator_next(). To call a function on all the elements of a
 * container, use eina_iterator_foreach().
 *
 * @{
 */

/**
 * @addtogroup Eina_Content_Access_Group Content Access
 *
 * @{
 */

/**
 * @defgroup Eina_Iterator_Group Iterator Functions
 *
 * @{
 */

/**
 * @typedef Eina_Iterator
 * Abstract type for iterators.
 */
typedef struct _Eina_Iterator Eina_Iterator;

/**
 * @typedef Eina_Iterator_Next_Callback
 * Type for a callback that returns the next element in a container.
 */
typedef Eina_Bool           (*Eina_Iterator_Next_Callback)(Eina_Iterator *it, void **data);

/**
 * @typedef Eina_Iterator_Get_Container_Callback
 * Type for a callback that returns the container.
 */
typedef void               *(*Eina_Iterator_Get_Container_Callback)(Eina_Iterator *it);

/**
 * @typedef Eina_Iterator_Free_Callback
 * Type for a callback that frees the container.
 */
typedef void                (*Eina_Iterator_Free_Callback)(Eina_Iterator *it);

/**
 * @typedef Eina_Iterator_Lock_Callback
 * Type for a callback that lock the container.
 */
typedef Eina_Bool           (*Eina_Iterator_Lock_Callback)(Eina_Iterator *it);

/**
 * @struct _Eina_Iterator
 * structure of an iterator
 */
struct _Eina_Iterator
{
#define EINA_ITERATOR_VERSION 1
   int                                  version; /**< Version of the Iterator API. */

   Eina_Iterator_Next_Callback          next          EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT; /**< Callback called when a next element is requested. */
   Eina_Iterator_Get_Container_Callback get_container EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT; /**< Callback called when the container is requested. */
   Eina_Iterator_Free_Callback          free          EINA_ARG_NONNULL(1); /**< Callback called when the container is freed. */

   Eina_Iterator_Lock_Callback          lock          EINA_WARN_UNUSED_RESULT; /**< Callback called when the container is locked. */
   Eina_Iterator_Lock_Callback          unlock        EINA_WARN_UNUSED_RESULT; /**< Callback called when the container is unlocked. */

#define EINA_MAGIC_ITERATOR 0x98761233
   EINA_MAGIC
};

/**
 * @def FUNC_ITERATOR_NEXT(Function)
 * Helper macro to cast @p Function to a Eina_Iterator_Next_Callback.
 */
#define FUNC_ITERATOR_NEXT(Function)          ((Eina_Iterator_Next_Callback)Function)

/**
 * @def FUNC_ITERATOR_GET_CONTAINER(Function)
 * Helper macro to cast @p Function to a Eina_Iterator_Get_Container_Callback.
 */
#define FUNC_ITERATOR_GET_CONTAINER(Function) ((Eina_Iterator_Get_Container_Callback)Function)

/**
 * @def FUNC_ITERATOR_FREE(Function)
 * Helper macro to cast @p Function to a Eina_Iterator_Free_Callback.
 */
#define FUNC_ITERATOR_FREE(Function)          ((Eina_Iterator_Free_Callback)Function)

/**
 * @def FUNC_ITERATOR_LOCK(Function)
 * Helper macro to cast @p Function to a Eina_Iterator_Lock_Callback.
 */
#define FUNC_ITERATOR_LOCK(Function)          ((Eina_Iterator_Lock_Callback)Function)


/**
 * @brief Free an iterator.
 *
 * @param iterator The iterator to free.
 *
 * This function frees @p iterator if it is not @c NULL;
 */
EAPI void      eina_iterator_free(Eina_Iterator *iterator) EINA_ARG_NONNULL(1);


/**
 * @brief Return the container of an iterator.
 *
 * @param iterator The iterator.
 * @return The container which created the iterator.
 *
 * This function returns the container which created @p iterator. If
 * @p iterator is @c NULL, this function returns @c NULL.
 */
EAPI void     *eina_iterator_container_get(Eina_Iterator *iterator) EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * @brief Return the value of the current element and go to the next one.
 *
 * @param iterator The iterator.
 * @param data The data of the element.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * This function returns the value of the current element pointed by
 * @p iterator in @p data, then goes to the next element. If @p
 * iterator is @c NULL or if a problem occurred, #EINA_FALSE is
 * returned, otherwise #EINA_TRUE is returned.
 */
EAPI Eina_Bool eina_iterator_next(Eina_Iterator *iterator,
                                  void         **data) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Iterate over the container and execute a callback on each element.
 *
 * @param iterator The iterator.
 * @param cb The callback called on each iteration.
 * @param fdata The data passed to the callback.
 *
 * This function iterates over the elements pointed by @p iterator,
 * beginning from the current element. For Each element, the callback
 * @p cb is called with the data @p fdata. If @p iterator is @c NULL,
 * the function returns immediately. Also, if @p cb returns @c
 * EINA_FALSE, the iteration stops at that point.
 */
EAPI void eina_iterator_foreach(Eina_Iterator *iterator,
                                Eina_Each_Cb   callback,
                                const void    *fdata) EINA_ARG_NONNULL(1, 2);


/**
 * @brief Lock the container of the iterator.
 *
 * @param iterator The iterator.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * If the container of the @p iterator permit it, it will be locked.
 * If @p iterator is @c NULL or if a problem occurred, #EINA_FALSE is
 * returned, otherwise #EINA_TRUE is returned. If the container
 * is not lockable, it will return EINA_TRUE.
 */
EAPI Eina_Bool eina_iterator_lock(Eina_Iterator *iterator) EINA_ARG_NONNULL(1);

/**
 * @brief Unlock the container of the iterator.
 *
 * @param iterator The iterator.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * If the container of the @p iterator permit it and was previously
 * locked, it will be unlocked. If @p iterator is @c NULL or if a
 * problem occurred, #EINA_FALSE is returned, otherwise #EINA_TRUE
 * is returned. If the container is not lockable, it will return
 * EINA_TRUE.
 */
EAPI Eina_Bool eina_iterator_unlock(Eina_Iterator *iterator) EINA_ARG_NONNULL(1);

/**
 * @def EINA_ITERATOR_FOREACH
 * @brief Macro to iterate over all elements easily.
 *
 * @param itr The iterator to use.
 * @param data Where to store * data, must be a pointer support getting
 *        its address since * eina_iterator_next() requires a pointer
 *        to pointer!
 *
 * This macro is a convenient way to use iterators, very similar to
 * EINA_LIST_FOREACH().
 *
 * This macro can be used for freeing the data of a list, like in the
 * following example. It has the same goal as the one documented in
 * EINA_LIST_FOREACH(), but using iterators:
 *
 * @code
 * Eina_List     *list;
 * Eina_Iterator *itr;
 * char          *data;
 *
 * // list is already filled,
 * // its elements are just duplicated strings
 *
 * itr = eina_list_iterator_new(list);
 * EINA_ITERATOR_FOREACH(itr, data)
 *   free(data);
 * eina_iterator_free(itr);
 * eina_list_free(list);
 * @endcode
 *
 * @note this example is not optimal algorithm to release a list since
 *    it will walk the list twice, but it serves as an example. For
 *    optimized version use EINA_LIST_FREE()
 *
 * @warning unless explicitly stated in functions returning iterators,
 *    do not modify the iterated object while you walk it, in this
 *    example using lists, do not remove list nodes or you might
 *    crash!  This is not a limitiation of iterators themselves,
 *    rather in the iterators implementations to keep them as simple
 *    and fast as possible.
 */
#define EINA_ITERATOR_FOREACH(itr,                                   \
                              data) while (eina_iterator_next((itr), \
                                                              (void **)(void *)&(data)))

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
