/* EON - Canvas and Toolkit library
 * Copyright (C) 2008-2009 Jorge Luis Zapata
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
#ifndef EON_COORD_H_
#define EON_COORD_H_

typedef enum
{
	EON_COORD_ABSOLUTE,
	EON_COORD_RELATIVE,
} Eon_Coord_Type;

typedef struct _Eon_Coord
{
	int final;
	Eon_Coord_Type type;
	int value;
} Eon_Coord;

static inline void eon_coord_set(Eon_Coord *c, int value, Eon_Coord_Type type)
{
	c->value = value;
	c->type = type;
}

#endif /* EON_COORD_H_ */
