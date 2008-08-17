/* EINA - EFL data type library
 * Copyright (C) 2002-2008 Carsten Haitzler, Vincent Torri, Jorge Luis Zapata Muga
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

#ifndef EINA_LIST_H_
#define EINA_LIST_H_

#include <stdlib.h>

#include "eina_types.h"
#include "eina_iterator.h"
#include "eina_accessor.h"

/**
 * @defgroup List_Group List
 * @{
 */

typedef struct _Eina_List Eina_List;
typedef struct _Eina_List_Accounting Eina_List_Accounting;

struct _Eina_List /** A linked list node */
{
   void      *data; /**< Pointer to list element payload */
   Eina_List *next; /**< Next member in the list */
   Eina_List *prev; /**< Previous member in the list */
   struct _Eina_List_Accounting *accounting; /**< Private list accounting info - don't touch */
};
struct _Eina_List_Accounting
{
   Eina_List *last;
   unsigned int count;
};


EAPI int eina_list_init(void);
EAPI int eina_list_shutdown(void);

EAPI Eina_List *eina_list_append (Eina_List *list, const void *data);
EAPI Eina_List *eina_list_prepend (Eina_List *list, const void *data);
EAPI Eina_List *eina_list_append_relative (Eina_List *list, const void *data, const void *relative);
EAPI Eina_List *eina_list_append_relative_list (Eina_List *list, const void *data, Eina_List *relative);
EAPI Eina_List *eina_list_prepend_relative (Eina_List *list, const void *data, const void *relative);
EAPI Eina_List *eina_list_prepend_relative_list (Eina_List *list, const void *data, Eina_List *relative);
EAPI Eina_List *eina_list_remove (Eina_List *list, const void *data);
EAPI Eina_List *eina_list_remove_list (Eina_List *list, Eina_List *remove_list);
EAPI Eina_List *eina_list_promote_list (Eina_List *list, Eina_List *move_list);
EAPI void *eina_list_find(const Eina_List *list, const void *data);
EAPI Eina_List *eina_list_find_list (const Eina_List *list, const void *data);
EAPI Eina_List *eina_list_free (Eina_List *list);
EAPI void *eina_list_nth(const Eina_List *list, unsigned int n);
EAPI Eina_List *eina_list_nth_list (const Eina_List *list, unsigned int n);
EAPI Eina_List *eina_list_reverse (Eina_List *list);
EAPI Eina_List *eina_list_sort (Eina_List *list, unsigned int size, int(*func)(void*,void*));

static inline Eina_List *eina_list_last (const Eina_List *list);
static inline Eina_List *eina_list_next (const Eina_List *list);
static inline Eina_List *eina_list_prev (const Eina_List *list);
static inline void *eina_list_data(const Eina_List *list);
static inline unsigned int eina_list_count(const Eina_List *list);

EAPI Eina_Iterator *eina_list_iterator_new(const Eina_List *list);
EAPI Eina_Accessor *eina_list_accessor_new(const Eina_List *list);

#define EINA_LIST_ITER_NEXT(list, l, data) for (l = list, data = eina_list_data(l); l; l = eina_list_next(l), data = eina_list_data(l))

/** @} */
#include "eina_inline_list.x"

#endif /* EINA_LIST_H_ */
