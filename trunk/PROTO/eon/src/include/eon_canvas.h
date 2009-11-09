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
#ifndef EON_CANVAS_H_
#define EON_CANVAS_H_
/*============================================================================*
 *                                 Events                                     *
 *============================================================================*/
#define EON_CANVAS_X_CHANGED "xChanged"
#define EON_CANVAS_Y_CHANGED "yChanged"
#define EON_CANVAS_W_CHANGED "wChanged"
#define EON_CANVAS_H_CHANGED "hChanged"
#define EON_CANVAS_MATRIX_CHANGED "matrixChanged"
/*============================================================================*
 *                               Properties                                   *
 *============================================================================*/
extern Ekeko_Property_Id EON_CANVAS_X;
extern Ekeko_Property_Id EON_CANVAS_Y;
extern Ekeko_Property_Id EON_CANVAS_W;
extern Ekeko_Property_Id EON_CANVAS_H;
extern Ekeko_Property_Id EON_CANVAS_MATRIX;
/*============================================================================*
 *                                 Class                                      *
 *============================================================================*/
typedef struct _Eon_Canvas_Private Eon_Canvas_Private;
struct _Eon_Canvas
{
	Eon_Layout base;
	Eon_Canvas_Private *prv;
};
/*============================================================================*
 *                                Functions                                   *
 *============================================================================*/
EAPI Eon_Canvas * eon_canvas_new(Eon_Document *c);
EAPI Eon_Document * eon_canvas_document_get(Eon_Canvas *c);
EAPI void eon_canvas_x_get(Eon_Canvas *c, Eon_Coord *x);
EAPI void eon_canvas_x_rel_set(Eon_Canvas *c, int x);
EAPI void eon_canvas_x_set(Eon_Canvas *c, int x);
EAPI void eon_canvas_y_get(Eon_Canvas *c, Eon_Coord *x);
EAPI void eon_canvas_y_set(Eon_Canvas *c, int y);
EAPI void eon_canvas_y_rel_set(Eon_Canvas *c, int y);
EAPI void eon_canvas_w_get(Eon_Canvas *c, Eon_Coord *w);
EAPI void eon_canvas_w_set(Eon_Canvas *c, int w);
EAPI void eon_canvas_w_rel_set(Eon_Canvas *c, int w);
EAPI void eon_canvas_h_get(Eon_Canvas *c, Eon_Coord *w);
EAPI void eon_canvas_h_set(Eon_Canvas *c, int h);
EAPI void eon_canvas_h_rel_set(Eon_Canvas *c, int h);
EAPI void eon_canvas_matrix_set(Eon_Canvas *c, Enesim_Matrix *m);
EAPI Eon_Document * eon_canvas_document_get(Eon_Canvas *c);
/* renderable wrappers */
#define eon_canvas_show(c) ekeko_renderable_show(EKEKO_RENDERABLE((c)))
#define eon_canvas_hide(c) ekeko_renderable_hide(EKEKO_RENDERABLE((c)))

#endif /* EON_CANVAS_H_ */
