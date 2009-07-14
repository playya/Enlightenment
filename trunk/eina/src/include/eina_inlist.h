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

#ifndef EINA_INLIST_H_
#define EINA_INLIST_H_

#include "eina_types.h"
#include "eina_iterator.h"
#include "eina_accessor.h"
#include <stddef.h>

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
 * @defgroup Eina_Inline_List_Group Inline List
 *
 * @{
 */

typedef struct _Eina_Inlist Eina_Inlist;
struct _Eina_Inlist
{
   Eina_Inlist *next;
   Eina_Inlist *prev;
   Eina_Inlist *last;
};

#define EINA_INLIST Eina_Inlist __in_list
#define EINA_INLIST_GET(Inlist) (&((Inlist)->__in_list))
#define EINA_INLIST_CONTAINER_GET(ptr, type) ((type *) ((Eina_Inlist *) ptr - offsetof(type, __in_list)))

EAPI Eina_Inlist * eina_inlist_append(Eina_Inlist *in_list, Eina_Inlist *in_item) EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;
EAPI Eina_Inlist * eina_inlist_prepend(Eina_Inlist *in_list, Eina_Inlist *in_item) EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;
EAPI Eina_Inlist * eina_inlist_append_relative(Eina_Inlist *in_list, Eina_Inlist *in_item, Eina_Inlist *in_relative) EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;
EAPI Eina_Inlist * eina_inlist_prepend_relative(Eina_Inlist *in_list, Eina_Inlist *in_item, Eina_Inlist *in_relative) EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;
EAPI Eina_Inlist * eina_inlist_remove(Eina_Inlist *in_list, Eina_Inlist *in_item) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;
EAPI Eina_Inlist * eina_inlist_find(Eina_Inlist *in_list, Eina_Inlist *in_item) EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;
EAPI Eina_Inlist * eina_inlist_promote(Eina_Inlist *list, Eina_Inlist *item) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;
EAPI Eina_Inlist * eina_inlist_demote(Eina_Inlist *list, Eina_Inlist *item) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;
EAPI unsigned int eina_inlist_count(const Eina_Inlist *list) EINA_WARN_UNUSED_RESULT;

EAPI Eina_Iterator *eina_inlist_iterator_new(const Eina_Inlist *in_list) EINA_MALLOC EINA_WARN_UNUSED_RESULT;
EAPI Eina_Accessor *eina_inlist_accessor_new(const Eina_Inlist *in_list) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

#define EINA_INLIST_FOREACH(list, l) for (l = (void*)list; l; l = (void*)(l->__in_list.next))
#define EINA_INLIST_REVERSE_FOREACH(list, l) for (l = (list ? (void*)(list->last) : NULL); l; l = (void*)(l->__in_list.prev))

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

#endif /*EINA_INLIST_H_*/
