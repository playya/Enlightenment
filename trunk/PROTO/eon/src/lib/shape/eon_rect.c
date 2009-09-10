/*
 * eon_rect.c
 *
 *  Created on: 04-feb-2009
 *      Author: jl
 */
#include "Eon.h"
#include "eon_private.h"
/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/
#define PRIVATE(d) ((Eon_Rect_Private *)((Eon_Rect *)(d))->private)
struct _Eon_Rect_Private
{
	float radius;
};

static void _geometry_calc(const Ekeko_Object *o, Ekeko_Event *e, void *data)
{
	Eon_Rect *r = (Eon_Rect *)o;
	Eina_Rectangle geom;
	Eon_Coord x, y, w, h;

	eon_square_coords_get((Eon_Square *)r, &x, &y, &w, &h);
	eina_rectangle_coords_from(&geom, x.final, y.final, w.final,
			h.final);
#ifdef EON_DEBUG
	printf("[Eon_Rect] Setting geometry of size %d %d %d %d\n",
			x.final, y.final, w.final, h.final);
#endif
	eon_shape_geometry_set((Eon_Shape *)r, &geom);
}

static void _render(Eon_Shape *s, Eon_Engine *eng, void *engine_data, void *canvas_data, Eina_Rectangle *clip)
{
#ifdef EON_DEBUG
	printf("[Eon_Rect] Rendering rectangle %p into canvas\n", r);
#endif
	eon_engine_rect_render(eng, engine_data, canvas_data, clip);
}

static Eina_Bool _is_inside(Eon_Shape *s, int x, int y)
{
	Eon_Rect *r = s;
	Eon_Rect_Private *prv;
	Enesim_Shape_Draw_Mode mode;
	Enesim_Matrix_Type mtype;
	Enesim_Matrix m;
	Eon_Coord cx, cy, cw, ch;


	eon_square_coords_get((Eon_Square *)s, &cx, &cy, &cw, &ch);
        /* handle the transformation */
	eon_shape_matrix_get(s, &m);
        mtype = enesim_matrix_type_get(&m);
        if (mtype != ENESIM_MATRIX_IDENTITY)
	{
		Eina_Rectangle rrect;
		Eina_Rectangle point;

		eina_rectangle_coords_from(&rrect, cx.final, cy.final,
				cw.final, ch.final);
		eina_rectangle_coords_from(&point, x, y, 1, 1);

		if (!eina_rectangles_intersect(&rrect, &point))
		{
			printf("IS NOT INSIDE!\n");
			return EINA_FALSE;
		}
		else
		{
			printf("IS INSIDE!\n");
		}
	}

	/* TODO handle the rounded corners */
	mode = eon_shape_draw_mode_get(s);
	if (mode == ENESIM_SHAPE_DRAW_MODE_STROKE)
	{
		float sw;

		sw = eon_shape_stroke_width_get(s);

		if (x > (cx.final + sw) && (x < cx.final + cw.final - sw) &&
				y > (cy.final + sw) &&
				y < (cy.final + ch.final + sw))
			return EINA_FALSE;
		else
			return EINA_TRUE;
	}

	return EINA_TRUE;
}

static void _ctor(void *instance)
{
	Eon_Rect *r;
	Eon_Rect_Private *prv;

	r = (Eon_Rect*) instance;
	r->private = prv = ekeko_type_instance_private_get(eon_rect_type_get(), instance);
	r->parent.parent.render = _render;
	r->parent.parent.create = eon_engine_rect_create;
	r->parent.parent.is_inside = _is_inside;
	/* events */
	ekeko_event_listener_add((Ekeko_Object *)r, EON_SQUARE_X_CHANGED, _geometry_calc, EINA_FALSE, NULL);
	ekeko_event_listener_add((Ekeko_Object *)r, EON_SQUARE_Y_CHANGED, _geometry_calc, EINA_FALSE, NULL);
	ekeko_event_listener_add((Ekeko_Object *)r, EON_SQUARE_W_CHANGED, _geometry_calc, EINA_FALSE, NULL);
	ekeko_event_listener_add((Ekeko_Object *)r, EON_SQUARE_H_CHANGED, _geometry_calc, EINA_FALSE, NULL);
	ekeko_event_listener_add((Ekeko_Object *)r, EON_SHAPE_MATRIX_CHANGED, _geometry_calc, EINA_FALSE, NULL);
}

static void _dtor(void *rect)
{

}
/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/
Ekeko_Property_Id EON_RECT_CORNERS;
Ekeko_Property_Id EON_RECT_CORNER_RADIUS;

EAPI Ekeko_Type *eon_rect_type_get(void)
{
	static Ekeko_Type *type = NULL;

	if (!type)
	{
		type = ekeko_type_new(EON_TYPE_RECT, sizeof(Eon_Rect),
				sizeof(Eon_Rect_Private), eon_square_type_get(),
				_ctor, _dtor, eon_shape_appendable);
		EON_RECT_CORNER_RADIUS = EKEKO_TYPE_PROP_SINGLE_ADD(type, "radius", EKEKO_PROPERTY_FLOAT, OFFSET(Eon_Rect_Private, radius));
	}

	return type;
}

EAPI Eon_Rect * eon_rect_new(Eon_Canvas *c)
{
	Eon_Rect *r;

	r = ekeko_type_instance_new(eon_rect_type_get());
	ekeko_object_child_append((Ekeko_Object *)c, (Ekeko_Object *)r);

	return r;
}

EAPI void eon_rect_corner_radius_set(Eon_Rect *r, float rad)
{
	Ekeko_Value v;

	ekeko_value_float_from(&v, rad);
	ekeko_object_property_value_set((Ekeko_Object *)r, "radius", &v);
}

EAPI float eon_rect_corner_radius_get(Eon_Rect *r)
{
	Eon_Rect_Private *prv = PRIVATE(r);

	return prv->radius;
}

