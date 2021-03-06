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
#ifndef EON_COLOR_H_
#define EON_COLOR_H_

typedef uint32_t Eon_Color;

static inline void eon_color_set(Eon_Color *c, uint8_t a, uint8_t r, uint8_t g, uint8_t b)
{
	*c = (a << 24) | (r << 16) | (g << 8) | b;
}

#endif /* EON_COLOR_H_ */
