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

#ifndef EINA_CONFIG_H_
#define EINA_CONFIG_H_

#undef EINA_MAGIC_DEBUG
#undef EINA_DEFAULT_MEMPOOL
#undef EINA_SAFETY_CHECKS

#ifndef NDEBUG
# define EINA_MAGIC_DEBUG
# define EINA_SAFETY_CHECKS
#endif

#define MODULE_ARCH "win32-msc-x86"
#define SHARED_LIB_SUFFIX ".dll"

#endif /* EINA_CONFIG_H_ */
