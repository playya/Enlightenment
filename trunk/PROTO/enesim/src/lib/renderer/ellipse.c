/* ENESIM - Direct Rendering Library
 * Copyright (C) 2007-2008 Jorge Luis Zapata
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
#include "Enesim.h"
#include "enesim_private.h"
/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/
typedef struct _Ellipse Ellipse;
struct _Ellipse {
	Enesim_Renderer base;

	float x, y;
	float rx, ry;

	struct {
		unsigned int color;
		Enesim_Renderer *paint;
		float weight;
	} stroke;

	struct {
		unsigned int color;
		Enesim_Renderer *paint;
	} fill;

	int draw_mode;

	int xx0, yy0;
	int rr0_x, rr0_y;
	int irr0_x, irr0_y;
	int cc0, icc0;
	int fxxp, fyyp;
	int ifxxp, ifyyp;
	unsigned char do_inner :1;
};

static void _span_color_outlined_paint_filled_affine(Enesim_Renderer *p, int x, int y, unsigned int len, uint32_t *dst)
{
	Ellipse *ellipse = (Ellipse *) p;
	int axx = p->matrix.axx, axy = p->matrix.axy, axz = p->matrix.axz;
	int ayx = p->matrix.ayx, ayy = p->matrix.ayy, ayz = p->matrix.ayz;
	int do_inner = ellipse->do_inner;
	unsigned int ocolor = ellipse->stroke.color;
	unsigned int icolor = ellipse->fill.color;
	int xx0 = ellipse->xx0, yy0 = ellipse->yy0;
	int rr0_x = ellipse->rr0_x, rr1_x = rr0_x + 65536;
	int rr0_y = ellipse->rr0_y, rr1_y = rr0_y + 65536;
	int irr0_x = ellipse->irr0_x, irr1_x = irr0_x + 65536;
	int irr0_y = ellipse->irr0_y, irr1_y = irr0_y + 65536;
	int cc0 = ellipse->cc0, cc1 = cc0 + 65536;
	int icc0 = ellipse->icc0, icc1 = icc0 + 65536;
	int fxxp = xx0 + ellipse->fxxp, fyyp = yy0 + ellipse->fyyp;
	int fxxm = xx0 - ellipse->fxxp, fyym = yy0 - ellipse->fyyp;
	int ifxxp = xx0 + ellipse->ifxxp, ifyyp = yy0 + ellipse->ifyyp;
	int ifxxm = xx0 - ellipse->ifxxp, ifyym = yy0 - ellipse->ifyyp;
	Enesim_Renderer *fpaint = ellipse->fill.paint;
	unsigned int *d = dst, *e = d + len;
	int xx, yy;
	int fill_only = 0;

	if (ellipse->draw_mode == ENESIM_FIGURE_DRAW_MODE_STROKE)
	  {
		icolor = 0;
		fpaint = NULL;
	  }
	if (ellipse->draw_mode == ENESIM_FIGURE_DRAW_MODE_FILL)
	  {
		ocolor = icolor;
		fill_only = 1;
		do_inner = 0;
		if (fpaint)
			fpaint->span(fpaint, x, y, len, dst);
	  }
	if ((ellipse->draw_mode == ENESIM_FIGURE_DRAW_MODE_STROKE_FILL) && do_inner && fpaint)
		fpaint->span(fpaint, x, y, len, dst);

	xx = (axx * x) + (axy * y) + axz;
	yy = (ayx * x) + (ayy * y) + ayz;

	while (d < e)
	{
		unsigned int q0 = 0;

		if ((abs(xx - xx0) <= rr1_x) && (abs(yy - yy0) <= rr1_y))
		{
			unsigned int op0 = ocolor, p0;
			int a = 256;

			if (fill_only && fpaint)
			{
				op0 = *d;
				if (ocolor != 0xffffffff)
					op0 = argb8888_mul4_sym(op0, ocolor);
			}

			if (((abs(xx - xx0) >= (rr0_x / 2))) || ((abs(yy - yy0) >= (rr0_y
					/ 2))))
			{
				int rr = hypot(xx - fxxp, yy - fyyp) + hypot(xx - fxxm, yy
						- fyym);

				a = 0;
				if (rr < cc1)
				{
					a = 256;
					if (rr > cc0)
						a -= ((rr - cc0) >> 8);
				}
			}

			if (a < 256)
				op0 = argb8888_mul_256(a, op0);

			p0 = op0;
			if (do_inner && (abs(xx - xx0) <= irr1_x) && (abs(yy - yy0)
					<= irr1_y))
			{
				p0 = icolor;
				if (fpaint)
				{
					p0 = *d;
					if (icolor != 0xffffffff)
						p0 = argb8888_mul4_sym(icolor, p0);
				}

				a = 256;
				if (((abs(xx - xx0) >= (irr0_x / 2))) || ((abs(yy - yy0)
						>= (irr0_y / 2))))
				{
					int rr = hypot(xx - ifxxp, yy - ifyyp) + hypot(xx - ifxxm,
							yy - ifyym);

					a = 0;
					if (rr < icc1)
					{
						a = 256;
						if (rr > icc0)
							a -= ((rr - icc0) >> 8);
					}
				}

				if (a < 256)
					p0 = INTERP_256(a, p0, op0);
			}
			q0 = p0;
		}
		*d++ = q0;
		xx += axx;
		yy += ayx;
	}
}

static void _span_color_outlined_paint_filled_proj(Enesim_Renderer *p, int x, int y,
		unsigned int len, uint32_t *dst)
{
	Ellipse *ellipse = (Ellipse *) p;
	int axx = p->matrix.axx, axy = p->matrix.axy, axz = p->matrix.axz;
	int ayx = p->matrix.ayx, ayy = p->matrix.ayy, ayz = p->matrix.ayz;
	int azx = p->matrix.azx, azy = p->matrix.azy, azz = p->matrix.azz;
	int do_inner = ellipse->do_inner;
	unsigned int ocolor = ellipse->stroke.color;
	unsigned int icolor = ellipse->fill.color;
	int xx0 = ellipse->xx0, yy0 = ellipse->yy0;
	int rr0_x = ellipse->rr0_x, rr1_x = rr0_x + 65536;
	int rr0_y = ellipse->rr0_y, rr1_y = rr0_y + 65536;
	int irr0_x = ellipse->irr0_x, irr1_x = irr0_x + 65536;
	int irr0_y = ellipse->irr0_y, irr1_y = irr0_y + 65536;
	int cc0 = ellipse->cc0, cc1 = cc0 + 65536;
	int icc0 = ellipse->icc0, icc1 = icc0 + 65536;
	int fxxp = xx0 + ellipse->fxxp, fyyp = yy0 + ellipse->fyyp;
	int fxxm = xx0 - ellipse->fxxp, fyym = yy0 - ellipse->fyyp;
	int ifxxp = xx0 + ellipse->ifxxp, ifyyp = yy0 + ellipse->ifyyp;
	int ifxxm = xx0 - ellipse->ifxxp, ifyym = yy0 - ellipse->ifyyp;
	Enesim_Renderer *fpaint = ellipse->fill.paint;
	unsigned int *d = dst, *e = d + len;
	int xx, yy, zz;
	int fill_only = 0;

	if (ellipse->draw_mode == ENESIM_FIGURE_DRAW_MODE_STROKE)
	  {
		icolor = 0;
		fpaint = NULL;
	  }
	if (ellipse->draw_mode == ENESIM_FIGURE_DRAW_MODE_FILL)
	  {
		ocolor = icolor;
		fill_only = 1;
		do_inner = 0;
		if (fpaint)
			fpaint->span(fpaint, x, y, len, dst);
	  }
	if ((ellipse->draw_mode == ENESIM_FIGURE_DRAW_MODE_STROKE_FILL) && do_inner && fpaint)
		fpaint->span(fpaint, x, y, len, dst);

	xx = (axx * x) + (axy * y) + axz;
	yy = (ayx * x) + (ayy * y) + ayz;
	zz = (azx * x) + (azy * y) + azz;

	while (d < e)
	{
		unsigned int q0 = 0;

		if (zz)
		{
			int sxx = ((((long long int)xx) << 16) / zz);
			int syy = ((((long long int)yy) << 16) / zz);

			if ((abs(sxx - xx0) <= rr1_x) && (abs(syy - yy0) <= rr1_y))
			{
				unsigned int op0 = ocolor, p0;
				int a = 256;

				if (fill_only && fpaint)
				{
					op0 = *d;
					if (ocolor != 0xffffffff)
						op0 = argb8888_mul4_sym(op0, ocolor);
				}

				if (((abs(sxx - xx0) >= (rr0_x / 2))) || ((abs(syy - yy0) >= (rr0_y / 2))))
				{
					int rr = hypot(sxx - fxxp, syy - fyyp) + hypot(sxx - fxxm, syy - fyym);

					a = 0;
					if (rr < cc1)
					{
						a = 256;
						if (rr > cc0)
							a -= ((rr - cc0) >> 8);
					}
				}

				if (a < 256)
					op0 = argb8888_mul_256(a, op0);

				p0 = op0;
				if (do_inner && (abs(sxx - xx0) <= irr1_x) && (abs(syy - yy0) <= irr1_y))
				{
					p0 = icolor;
					if (fpaint)
					{
						p0 = *d;
						if (icolor != 0xffffffff)
							p0 = argb8888_mul4_sym(icolor, p0);
					}

					a = 256;
					if (((abs(sxx - xx0) >= (irr0_x / 2))) || ((abs(syy - yy0) >= (irr0_y / 2))))
					{
						int rr = hypot(sxx - ifxxp, syy - ifyyp) + hypot(sxx - ifxxm, syy - ifyym);

						a = 0;
						if (rr < icc1)
						{
							a = 256;
							if (rr > icc0)
								a -= ((rr - icc0) >> 8);
						}
					}

					if (a < 256)
						p0 = INTERP_256(a, p0, op0);
				}
				q0 = p0;
			}
		}
		*d++ = q0;
		xx += axx;
		yy += ayx;
		zz += azx;
	}
}

static int _state_setup(Enesim_Renderer *p)
{
	Ellipse *ellipse = (Ellipse *) p;
	float rx, ry;
	float sw;

	if (!ellipse || (ellipse->rx < 1) || (ellipse->ry < 1))
		return EINA_FALSE;

	if (p->changed)
	{
		ellipse->rr0_x = 65536 * (ellipse->rx - 1);
		ellipse->rr0_y = 65536 * (ellipse->ry - 1);
		ellipse->xx0 = 65536 * (ellipse->x - 0.5);
		ellipse->yy0 = 65536 * (ellipse->y - 0.5);

		rx = ellipse->rx - 1;
		ry = ellipse->ry - 1;
		if (rx > ry)
		{
			ellipse->fxxp = 65536 * sqrt(fabs((rx * rx) - (ry * ry)));
			ellipse->fyyp = 0;
			ellipse->cc0 = 2 * ellipse->rr0_x;
		} else
		{
			ellipse->fxxp = 0;
			ellipse->fyyp = 65536 * sqrt(fabs((ry * ry) - (rx * rx)));
			ellipse->cc0 = 2 * ellipse->rr0_y;
		}
		sw = ellipse->stroke.weight;
		ellipse->do_inner = 1;
		if ((sw >= (ellipse->rx - 1)) || (sw >= (ellipse->ry - 1)))
		{
			sw = 0;
			ellipse->do_inner = 0;
		}
		rx = ellipse->rx - 1 - sw;
		if (rx < 0.0039)
			rx = 0;
		ellipse->irr0_x = rx * 65536;
		ry = ellipse->ry - 1 - sw;
		if (ry < 0.0039)
			ry = 0;
		ellipse->irr0_y = ry * 65536;

		if (rx > ry)
		{
			ellipse->ifxxp = 65536 * sqrt(fabs((rx * rx) - (ry * ry)));
			ellipse->ifyyp = 0;
			ellipse->icc0 = 2 * ellipse->irr0_x;
		} else
		{
			ellipse->ifxxp = 0;
			ellipse->ifyyp = 65536 * sqrt(fabs((ry * ry) - (rx * rx)));
			ellipse->icc0 = 2 * ellipse->irr0_y;
		}
	}

	if (ellipse->fill.paint &&
	    ((ellipse->draw_mode == ENESIM_FIGURE_DRAW_MODE_FILL) ||
	     (ellipse->draw_mode == ENESIM_FIGURE_DRAW_MODE_STROKE_FILL)))
	{
		if (!enesim_renderer_state_setup(ellipse->fill.paint))
			return EINA_FALSE;
	}

	p->span = ENESIM_RENDERER_SPAN_DRAW(_span_color_outlined_paint_filled_proj);
	if (p->matrix.is_affine)
		p->span = ENESIM_RENDERER_SPAN_DRAW(_span_color_outlined_paint_filled_affine);

	p->changed = EINA_FALSE;
	return EINA_TRUE;
}

static void _state_cleanup(Enesim_Renderer *p)
{
	Ellipse *ellipse = (Ellipse *) p;

	if (ellipse->fill.paint &&
	    ((ellipse->draw_mode == ENESIM_FIGURE_DRAW_MODE_FILL) ||
	     (ellipse->draw_mode == ENESIM_FIGURE_DRAW_MODE_STROKE_FILL)))
		enesim_renderer_state_cleanup(ellipse->fill.paint);
}

static void _free(Enesim_Renderer *p)
{
}

/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/
/*============================================================================*
 *                                   API                                      *
 *============================================================================*/
EAPI Enesim_Renderer * enesim_renderer_ellipse_new(void)
{
	Enesim_Renderer *p;
	Ellipse *ellipse;

	ellipse = calloc(1, sizeof(Ellipse));
	if (!ellipse)
		return NULL;
	ellipse->fill.color = 0xffffffff;
	ellipse->stroke.color = 0xffffffff;
	p = (Enesim_Renderer *) ellipse;

	p->type_id = ELLIPSE_RENDERER;
	p->matrix.axx = p->matrix.ayy = p->matrix.azz = 65536;
	p->free = ENESIM_RENDERER_DELETE(_free);
	p->state_cleanup = ENESIM_RENDERER_STATE_CLEANUP(_state_cleanup);
	p->state_setup = ENESIM_RENDERER_STATE_SETUP(_state_setup);
	p->matrix.is_identity = p->matrix.is_affine = 1;
	p->changed = EINA_TRUE;
	//   if (!ellipse_setup_state(p, 0)) { free(ellipse); return NULL; }
	return p;
}

EAPI void enesim_renderer_ellipse_center_set(Enesim_Renderer *p, float x, float y)
{
	Ellipse *ellipse;

	ellipse = (Ellipse *) p;
	if (!ellipse) return;
	if ((ellipse->x == x) && (ellipse->y == y))
		return;
	ellipse->x = x;
	ellipse->y = y;
	p->changed = EINA_TRUE;
}

EAPI void enesim_renderer_ellipse_radii_set(Enesim_Renderer *p, float radius_x, float radius_y)
{
	Ellipse *ellipse;

	ellipse = (Ellipse *) p;
	if (!ellipse) return;
	if (radius_x < 0.9999999)
		radius_x = 1;
	if (radius_y < 0.9999999)
		radius_y = 1;
	if ((ellipse->rx == radius_x) && (ellipse->ry == radius_y))
		return;
	ellipse->rx = radius_x;
	ellipse->ry = radius_y;
	p->changed = EINA_TRUE;
}

EAPI void enesim_renderer_ellipse_outline_weight_set(Enesim_Renderer *p, float weight)
{
	Ellipse *ellipse;

	ellipse = (Ellipse *) p;
	if (!ellipse) return;
	if (weight < 0.000009)
		weight = 0;
	if (ellipse->stroke.weight == weight)
		return;
	ellipse->stroke.weight = weight;
	p->changed = EINA_TRUE;
}

EAPI void enesim_renderer_ellipse_outline_color_set(Enesim_Renderer *p, unsigned int stroke_color)
{
	Ellipse *ellipse;

	ellipse = (Ellipse *) p;
	if (!ellipse) return;
	if (ellipse->stroke.color == stroke_color)
		return;
	ellipse->stroke.color = stroke_color;
//	p->changed = EINA_TRUE;
}

EAPI void enesim_renderer_ellipse_outline_paint_set(Enesim_Renderer *p, Enesim_Renderer *paint)
{
	Ellipse *ellipse;

	ellipse = (Ellipse *) p;
	if (!ellipse) return;
	if (p == paint)
		return;
	if (ellipse->stroke.paint == paint)
		return;
	ellipse->stroke.paint = paint;
//	p->changed = EINA_TRUE;
}

EAPI void enesim_renderer_ellipse_fill_color_set(Enesim_Renderer *p, unsigned int fill_color)
{
	Ellipse *ellipse;

	ellipse = (Ellipse *) p;
	if (!ellipse) return;
	if (ellipse->fill.color == fill_color)
		return;
	ellipse->fill.color = fill_color;
//	p->changed = EINA_TRUE;
}

EAPI void enesim_renderer_ellipse_fill_paint_set(Enesim_Renderer *p, Enesim_Renderer *paint)
{
	Ellipse *ellipse;

	ellipse = (Ellipse *) p;
	if (!ellipse) return;
	if (p == paint)
		return;
	if (ellipse->fill.paint == paint)
		return;
	ellipse->fill.paint = paint;
//	p->changed = EINA_TRUE;
}

EAPI void enesim_renderer_ellipse_draw_mode_set(Enesim_Renderer *p, Enesim_Figure_Draw_Mode draw_mode)
{
	Ellipse *ellipse;

	ellipse = (Ellipse *) p;
	if (!ellipse) return;
	if ((draw_mode != ENESIM_FIGURE_DRAW_MODE_STROKE) &&
		(draw_mode != ENESIM_FIGURE_DRAW_MODE_FILL) &&
		(draw_mode != ENESIM_FIGURE_DRAW_MODE_STROKE_FILL))
		draw_mode = ENESIM_FIGURE_DRAW_MODE_FILL;
	if (ellipse->draw_mode == draw_mode)
		return;
	ellipse->draw_mode = draw_mode;
//	r->changed = EINA_TRUE;
}
