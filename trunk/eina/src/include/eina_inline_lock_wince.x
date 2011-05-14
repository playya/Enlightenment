/* EINA - EFL data type library
 * Copyright (C) 2011 Vincent Torri
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

#ifndef EINA_INLINE_LOCK_WIN32_X_
#define EINA_INLINE_LOCK_WIN32_X_

#include <windows.h>

EAPI extern Eina_Bool _threads_activated;

typedef HANDLE Eina_Lock;

static inline Eina_Bool
eina_lock_new(Eina_Lock *mutex)
{
   Eina_Lock m;

   m = CreateMutex(NULL, FALSE, NULL);
   if (m) *mutex = m;
   return (m != NULL);
}

static inline void
eina_lock_free(Eina_Lock *mutex)
{
   CloseHandle(*mutex);
}

static inline Eina_Lock_Result
eina_lock_take(Eina_Lock *mutex)
{
   DWORD res;

#ifdef EINA_HAVE_ON_OFF_THREADS
   if (!_eina_threads_activated) return EINA_LOCK_FAIL;
#endif

   res = WaitForSingleObject(*mutex, INFINITE);
   if ((res == WAIT_ABANDONED) || (res == WAIT_FAILED))
     return EINA_LOCK_FAIL;

   return EINA_LOCK_SUCCEED;
}

static inline Eina_Lock_Result
eina_lock_take_try(Eina_Lock *mutex)
{
   return eina_lock_take(*mutex);
}

static inline Eina_Lock_Result
eina_lock_release(Eina_Lock *mutex)
{
#ifdef EINA_HAVE_ON_OFF_THREADS
   if (!_eina_threads_activated) return ;
#endif

   return ReleaseMutex(*mutex) ? EINA_LOCK_SUCCEED : EINA_LOCK_FAIL;
}

static inline void
eina_lock_debug(const Eina_Lock *mutex)
{
}

static inline Eina_Bool
eina_condition_new(Eina_Condition *cond, Eina_Lock *mutex)
{
   return EINA_FALSE;
}

static inline void
eina_condition_free(Eina_Condition *cond)
{
}

static inline Eina_Bool
eina_condition_wait(Eina_Condition *cond)
{
   return EINA_FALSE;
}

static inline Eina_Bool
eina_condition_broadcast(Eina_Condition *cond)
{
   return EINA_FALSE;
}

static inline Eina_Bool
eina_condition_signal(Eina_Condition *cond)
{
   return EINA_FALSE;
}


#endif
