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
#ifndef EON_STYLE_APPLIER_H_
#define EON_STYLE_APPLIER_H_
/*============================================================================*
 *                                 Events                                     *
 *============================================================================*/
/*============================================================================*
 *                               Properties                                   *
 *============================================================================*/
/*============================================================================*
 *                                 Class                                      *
 *============================================================================*/
typedef struct _Eon_Style_Applier_Private Eon_Style_Applier_Private;
struct _Eon_Style_Applier
{
	Ekeko_Object parent;
	Eon_Style_Applier_Private *prv;
	void (*set)(Ekeko_Object *self, Ekeko_Object *ref);
	void (*unset)(Ekeko_Object *self, Ekeko_Object *ref);
};
/*============================================================================*
 *                                Functions                                   *
 *============================================================================*/
EAPI Ekeko_Type * eon_style_applier_type_get(void);

#endif
