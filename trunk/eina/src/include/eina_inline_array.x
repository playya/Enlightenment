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

#ifndef EINA_INLINE_ARRAY_X_
#define EINA_INLINE_ARRAY_X_

#ifdef __GNUC__
# define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
# define UNLIKELY(x) (x)
#endif

/**
 * @cond LOCAL
 */

EAPI Eina_Bool eina_array_grow(Eina_Array *array);

/**
 * @endcond
 */

/**
 * @addtogroup Eina_Array_Group Array
 *
 * @brief These functions provide array management.
 *
 * @{
 */

/**
 * @brief Append a data to an array.
 *
 * @param array The array.
 * @param data The data to add.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * This function appends @p data to @p array. For performance
 * reasons, there is no check of @p array. If it is @c NULL or
 * invalid, the program may crash. If @p data is @c NULL, or if an
 * allocation is necessary and fails, #EINA_FALSE is returned and
 * #EINA_ERROR_OUT_OF_MEMORY is set. Otherwise, #EINA_TRUE is
 * returned.
 */
static inline Eina_Bool
eina_array_push(Eina_Array *array, const void *data)
{
   if (!data) return EINA_FALSE;

   if (UNLIKELY((array->count + 1) > array->total))
     if (!eina_array_grow(array)) return EINA_FALSE;

   array->data[array->count++] = (void*) data;
   return EINA_TRUE;
}

/**
 * @brief Remove the last data of an array.
 *
 * @param array The array.
 * @return The retrieved data.
 *
 * This function removes the last data of @p array, decreases the count
 * of @p array and returns the data. For performance reasons, there
 * is no check of @p array. If it is @c NULL or invalid, the program
 * may crash. If the count member is less or equal than 0, @c NULL is
 * returned.
 */
static inline void *
eina_array_pop(Eina_Array *array)
{
   if (array->count <= 0) return NULL;
   return array->data[--array->count];
}

/**
 * @brief Return the data at a given position in an array.
 *
 * @param array The array.
 * @param index The potition of the data to retrieve.
 * @return The retrieved data.
 *
 * This function returns the data at the position @p index in @p
 * array. For performance reasons, there is no check of @p array or @p
 * index. If it is @c NULL or invalid, the program may crash.
 */
static inline void *
eina_array_data_get(const Eina_Array *array, unsigned int index)
{
   return array->data[index];
}

/**
 * @brief Return the data at a given position in an array.
 *
 * @param array The array.
 * @param index The potition of the data to set.
 * @param data The data to set.
 *
 * This function returns the data at the position @p index in @p
 * array. For performance reasons, there is no check of @p array or @p
 * index. If it is @c NULL or invalid, the program may crash.
 */
static inline void
eina_array_data_set(const Eina_Array *array, unsigned int index, const void *data)
{
   array->data[index] = (void*) data;
}

/**
 * @brief Return the number of elements in an array.
 *
 * @param array The array.
 * @return The number of elements.
 *
 * This function returns the number of elements in @p array. For
 * performance reasons, there is no check of @p array. If it is
 * @c NULL or invalid, the program may crash.
 */
static inline unsigned int
eina_array_count_get(const Eina_Array *array)
{
   return array->count;
}

/**
 * @}
 */

#endif
