/* EINA - EFL data type library
 * Copyright (C) 2002-2008 Carsten Haitzler, Vincent Torri
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
#include <assert.h>

#include "eina_inlist.h"
#include "eina_error.h"
#include "eina_private.h"
#include "eina_safety_checks.h"

/* FIXME: TODO please, refactor this :) */

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

typedef struct _Eina_Iterator_Inlist Eina_Iterator_Inlist;
typedef struct _Eina_Accessor_Inlist Eina_Accessor_Inlist;

struct _Eina_Iterator_Inlist
{
	Eina_Iterator iterator;
	const Eina_Inlist *head;
	const Eina_Inlist *current;
};

struct _Eina_Accessor_Inlist
{
	Eina_Accessor accessor;

	const Eina_Inlist *head;
	const Eina_Inlist *current;

	unsigned int index;
};

static Eina_Bool
eina_inlist_iterator_next(Eina_Iterator_Inlist *it, void **data) {
	if (it->current == NULL) return EINA_FALSE;
	if (data) *data = (void*) it->current;

	it->current = it->current->next;

	return EINA_TRUE;
}

static Eina_Inlist *
eina_inlist_iterator_get_container(Eina_Iterator_Inlist *it) {
	return (Eina_Inlist*) it->head;
}

static void
eina_inlist_iterator_free(Eina_Iterator_Inlist *it) {
	free(it);
}

static Eina_Bool
eina_inlist_accessor_get_at(Eina_Accessor_Inlist *it, unsigned int index, void **data) {
	const Eina_Inlist *over;
	unsigned int middle;
	unsigned int i;

	if (it->index == index) {
		over = it->current;
	} else if (index > it->index) {
		/* Looking after current. */
		for (i = it->index, over = it->current;
		     i < index && over != NULL;
		     ++i, over = over->next)
			;

	} else {
		middle = it->index >> 1;

		if (index > middle) {
			/* Looking backward from current. */
			for (i = it->index, over = it->current;
			     i > index && over != NULL;
			     --i, over = over->prev)
				;
		} else {
			/* Looking from the start. */
			for (i = 0, over = it->head;
			     i < index && over != NULL;
			     ++i, over = over->next)
				;
		}
	}

	if (over == NULL) return EINA_FALSE;

	it->current = over;
	it->index = index;

	if (data) *data = (void*) over;
	return EINA_TRUE;
}

static Eina_Inlist *
eina_inlist_accessor_get_container(Eina_Accessor_Inlist *it) {
	return (Eina_Inlist *) it->head;
}

static void
eina_inlist_accessor_free(Eina_Accessor_Inlist *it) {
	free(it);
}

/**
 * @endcond
 */


/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

/**
 * @addtogroup Eina_Inline_List_Group Inline List
 *
 * @brief These functions provide inline list management.
 *
 * @{
 */

EAPI Eina_Inlist *
eina_inlist_append(Eina_Inlist *list, Eina_Inlist *new_l)
{
   Eina_Inlist *l;

   EINA_SAFETY_ON_NULL_RETURN_VAL(new_l, list);

   new_l->next = NULL;
   if (!list) {
      new_l->prev = NULL;
      new_l->last = new_l;
      return new_l;
   }
   if (list->last)
     l = list->last;
   else
     for (l = list; (l) && (l->next); l = l->next)
       ;
   l->next = new_l;
   new_l->prev = l;
   list->last = new_l;
   return list;
}

EAPI Eina_Inlist *
eina_inlist_prepend(Eina_Inlist *list, Eina_Inlist *new_l)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(new_l, list);

   new_l->prev = NULL;
   if (!list) {
      new_l->next = NULL;
      new_l->last = new_l;
      return new_l;
   }
   new_l->next = list;
   list->prev = new_l;
   new_l->last = list->last;
   list->last = NULL;
   return new_l;
}

EAPI Eina_Inlist *
eina_inlist_append_relative(Eina_Inlist *list,
			    Eina_Inlist *new_l,
			    Eina_Inlist *relative)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(new_l, list);

   if (relative) {
      if (relative->next) {
	 new_l->next = relative->next;
	 relative->next->prev = new_l;
      } else
	new_l->next = NULL;
      relative->next = new_l;
      new_l->prev = relative;
      if (!new_l->next)
	list->last = new_l;
      return list;
   }
   return eina_inlist_append(list, new_l);
}

EAPI Eina_Inlist *
eina_inlist_prepend_relative(Eina_Inlist *list,
			     Eina_Inlist *new_l,
			     Eina_Inlist *relative)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(new_l, list);

   if (relative) {
      new_l->prev = relative->prev;
      new_l->next = relative;
      relative->prev = new_l;
      if (new_l->prev) {
	 new_l->prev->next = new_l;
	 /* new_l->next could not be NULL, as it was set to 'relative' */
	 assert(new_l->next);
	 return list;
      } else {
	 /* new_l->next could not be NULL, as it was set to 'relative' */
	 assert(new_l->next);

	 new_l->last = list->last;
	 list->last = NULL;
	 return new_l;
      }
   }
   return eina_inlist_prepend(list, new_l);
}

EAPI Eina_Inlist *
eina_inlist_remove(Eina_Inlist *list, Eina_Inlist *item)
{
   Eina_Inlist *return_l;

   /* checkme */
   EINA_SAFETY_ON_NULL_RETURN_VAL(list, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(item, list);

   if (item->next)
     item->next->prev = item->prev;

   if (item->prev) {
      item->prev->next = item->next;
      return_l = list;
   } else {
      return_l = item->next;
      if (return_l)
	return_l->last = list->last;
   }
   if (item == list->last)
     list->last = item->prev;
   item->next = NULL;
   item->prev = NULL;
   return return_l;
}

EAPI Eina_Inlist *
eina_inlist_promote(Eina_Inlist *list, Eina_Inlist *item)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(list, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(item, list);

   if (item == list) return list;

   if (item->next)
     item->next->prev = item->prev;
   item->prev->next = item->next;

   if (list->last == item)
     list->last = item->prev;

   item->next = list;
   item->prev = NULL;
   item->last = list->last;

   list->prev = item;
   list->last = NULL;

   return item;
}

EAPI Eina_Inlist *
eina_inlist_demote(Eina_Inlist *list, Eina_Inlist *item)
{
   Eina_Inlist *l;

   EINA_SAFETY_ON_NULL_RETURN_VAL(list, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(item, list);

   if (list->last == item) return list;

   if (!list->last)
     {
	for (l = list; l->next; l = l->next)
	  ;
	list->last = l;
     }

   l = list;
   if (item->prev)
     item->prev->next = item->next;
   else
     l = item->next;
   item->next->prev = item->prev;

   list->last->next = item;
   item->prev = list->last;
   item->next = NULL;

   l->last = item;
   return l;
}

EAPI Eina_Inlist *
eina_inlist_find(Eina_Inlist *list, Eina_Inlist *item)
{
   Eina_Inlist *l;

   for (l = list; l; l = l->next) {
      if (l == item)
	return item;
   }
   return NULL;
}

/**
 * @brief Get the count of the number of items in a list.
 *
 * @param list The list whose count to return.
 * @return The number of members in the list.
 *
 * This function returns how many members @p list contains. If the
 * list is @c NULL, 0 is returned.
 *
 * @warning This is an order-N operation and so the time will depend
 *    on the number of elements on the list, that is, it might become
 *    slow for big lists!
 */
EAPI unsigned int
eina_inlist_count(const Eina_Inlist *list)
{
   const Eina_Inlist *l;
   unsigned int i = 0;

   for (l = list; l; l = l->next)
     i++;

   return i;
}

EAPI Eina_Iterator *
eina_inlist_iterator_new(const Eina_Inlist *list)
{
   Eina_Iterator_Inlist *it;

   eina_error_set(0);
   it = calloc(1, sizeof (Eina_Iterator_Inlist));
   if (!it) {
      eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
      return NULL;
   }

   it->head = list;
   it->current = list;

   it->iterator.next = FUNC_ITERATOR_NEXT(eina_inlist_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(eina_inlist_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(eina_inlist_iterator_free);

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   return &it->iterator;
}

EAPI Eina_Accessor *
eina_inlist_accessor_new(const Eina_Inlist *list)
{
   Eina_Accessor_Inlist *it;

   eina_error_set(0);
   it = calloc(1, sizeof (Eina_Accessor_Inlist));
   if (!it) {
      eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
      return NULL;
   }

   it->head = list;
   it->current = list;
   it->index = 0;

   it->accessor.get_at = FUNC_ACCESSOR_GET_AT(eina_inlist_accessor_get_at);
   it->accessor.get_container = FUNC_ACCESSOR_GET_CONTAINER(eina_inlist_accessor_get_container);
   it->accessor.free = FUNC_ACCESSOR_FREE(eina_inlist_accessor_free);

   EINA_MAGIC_SET(&it->accessor, EINA_MAGIC_ACCESSOR);

   return &it->accessor;
}

/**
 * @}
 */
