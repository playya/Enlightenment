/* EKEKO - Object and property system
 * Copyright (C) 2007-2009 Jorge Luis Zapata
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
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef EKEKO_PRIVATE_H
#define EKEKO_PRIVATE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DBG(...) EINA_LOG_DOM_DBG(ekeko_dom, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(ekeko_dom, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(ekeko_dom, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(ekeko_dom, __VA_ARGS__)

extern int ekeko_dom;

#define OFFSET(type, mem) ((size_t) ((char *)&((type *) 0)->mem - (char *)((type *) 0)))

#include <private/object.h>
#include <private/renderable.h>
#include <private/input.h>
#include <private/type.h>
#include <private/value.h>
#include <private/event.h>
#include <private/property.h>

#endif
