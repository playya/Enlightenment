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
#include "Eon.h"
#include "eon_private.h"
/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/
#define BOUNDING_DEBUG 0
#define PRIVATE(d) ((Eon_Canvas_Private *)((Eon_Canvas *)(d))->private)

static Ekeko_Type *_type;
struct _Eon_Canvas_Private
{
	Eon_Coord x, y, w, h;
	/* how to handle the geometry of an object based on the matrix and the coords
	 * looks that child objects with relative coordinates use the geometry
	 */
	Enesim_Matrix matrix; /* user matrix S -> D */
	Enesim_Matrix inverse; /* inverse matrix D -> S*/ /* FIXME do we actually need this? */

	/* track parent canvas, if parent canvas == NULL then this is the
	 * main canvas and we should treat it differently
	 */
	Eina_Bool root;
	/* TODO overflow property */
	/* FIXME change this later */
	Eon_Document *doc;
	void *engine_data;
};

/* in case the subcanvas has another canvas as parent it will blt to the
 * parent canvas
 */
static void _subcanvas_render(Ekeko_Renderable *r, Eina_Rectangle *rect)
{
	Eina_Rectangle sgeom, srect;
	Eon_Canvas *c;
	Eon_Canvas_Private *sprv, *cprv;
	Eon_Engine *eng;

	sprv = PRIVATE(r);
	c = (Eon_Canvas *)ekeko_renderable_canvas_get(r);
	cprv = PRIVATE(c);

	eng = eon_document_engine_get(sprv->doc);
#if BOUNDING_DEBUG
	{
#if 0
		func->context->color_set(ctx, 0xffaaaaaa);
		func->context->rop_set(ctx, ENESIM_FILL);
		func->canvas->lock(cprv->s);
		func->shape->rect(cprv->s, ctx, rect->x, rect->y, rect->w, rect->h);
		func->canvas->unlock(cprv->s);
		func->context->delete(ctx);
#endif
	}
#endif
	{
		Enesim_Quad q;

		/* get the largest rectangle that fits on the matrix */
		enesim_matrix_rect_transform(&sprv->inverse, &sgeom, &q);
		enesim_quad_rectangle_to(&q, &srect);
	}
	srect.x = srect.y = -1;
	srect.w = 1 + sprv->w.final;
	srect.h = 1 + sprv->h.final;
	/* blt there */
#ifdef EON_DEBUG
	printf("[Eon_Canvas] Subcanvas render %d %d %d %d (%d %d %d %d)\n", srect.x, srect.y, srect.w, srect.h, rect->x, rect->y, rect->w, rect->h);
#endif
	eon_engine_canvas_blit(eng, sprv->engine_data, rect, cprv->engine_data, &srect);
}

static Eina_Bool _subcanvas_is_inside(Ekeko_Canvas *c, int x, int y)
{
	printf("CAAAAAAAAAAALED\n");
	return EINA_TRUE;
}
static inline Eina_Bool _subcanvas_flush(Ekeko_Canvas *c, Eina_Rectangle *r)
{
	Eina_Rectangle rscaled;
	Eina_Rectangle cgeom;
	Ekeko_Canvas *dc; /* the canvas this subcanvas has */

	/* this canvas doesnt have a parent canvas? */
	dc = ekeko_renderable_canvas_get((Ekeko_Renderable *)c);
	if (!dc)
		return EINA_TRUE;
	ekeko_renderable_geometry_get((Ekeko_Renderable *)c, &cgeom);
	/* transform the rectangle relative to the upper canvas */
	eina_rectangle_rescale_out(&cgeom, r, &rscaled);
	printf("[Eon_Canvas] subcanvas adding a new damage %d %d %d %d (%d %d %d %d)\n",
			rscaled.x, rscaled.y, rscaled.w, rscaled.h,
			r->x, r->y, r->w, r->h);
	printf("[Eon_Canvas] subcanvas = %p, canvas = %p\n", c, dc);
	ekeko_canvas_damage_add(dc, &rscaled);
	return EINA_FALSE;
}

static Eina_Bool _flush(Ekeko_Canvas *c, Eina_Rectangle *r)
{
	Eon_Canvas_Private *prv;

	prv = PRIVATE(c);
	/* if root flip */
	if (prv->root)
	{
		Eon_Engine *eng;

#ifdef EON_DEBUG
		printf("[Eon_Canvas] flipping root surface\n");
#endif
		eng = eon_document_engine_get(prv->doc);
		return eon_engine_canvas_flush(eng, prv->engine_data, r);
	}
	/* otherwise blt */
	else
	{
		return _subcanvas_flush(c, r);
	}
}

static void _geometry_change(const Ekeko_Object *o, Ekeko_Event *e, void *data)
{
	Ekeko_Event_Mutation *em = (Ekeko_Event_Mutation *)e;
	Eon_Canvas_Private *prv = PRIVATE(o);
	Eon_Engine *eng;
	void *doc_data;
	int w, h;

	/* check if the change is the rectangle */
	if (em->state != EVENT_MUTATION_STATE_POST)
		return;

	if (!prv->doc)
	{
		printf("EEEEEEEEEEEEERRRRRRRRRRRRRRROOOOOOOOOOR\n");
		exit(1);
	}

	/* TODO add the x and y too */
	/* TODO check that the w and h have changed */
	w = prv->w.final;
	h = prv->h.final;
	eng = eon_document_engine_get(prv->doc);
	doc_data = eon_document_engine_data_get(prv->doc);
	if (prv->engine_data)
		eon_engine_canvas_delete(eng, prv->engine_data);
	prv->engine_data = eon_engine_canvas_create(eng, doc_data, (Eon_Canvas *)o, prv->root, w, h);
}

/* Once the matrix or the coordinates have changed, update the renderable
 * geometry */
static inline void _geometry_calc(Eon_Canvas *c)
{
	Eon_Canvas_Private *prv = PRIVATE(c);
	Eina_Rectangle r;

	eina_rectangle_coords_from(&r, prv->x.final, prv->y.final, prv->w.final,
			prv->h.final);
	/* in case of a subcanvas use the matrix */
	if (!prv->root)
	{
		/* compute the final geometry multiplying by the context matrix */
		Enesim_Quad qm, qi;
		float x1, y1, x2, y2, x3, y3, x4, y4;

		/* get the largest rectangle that fits on the matrix */
		printf("[Eon_Canvas] Geometry is %d %d %d %d\n", r.x, r.y, r.w, r.h);
		printf("[Eon_Canvas] Transforming with matrix = \n");
		{
			Enesim_Matrix *m = &prv->matrix;
			printf("[Eon_Canvas] Subcanvas matrix =\n            %f %f %f\n            %f %f %f\n            %f %f %f\n", m->xx, m->xy, m->xz, m->yx, m->yy, m->yz, m->zx, m->zy, m->zz);
		}
		enesim_matrix_rect_transform(&prv->matrix, &r, &qm);
		enesim_quad_coords_get(&qm, &x1, &y1, &x2, &y2, &x3, &y3, &x4, &y4);
		enesim_quad_rectangle_to(&qm, &r);
		printf("[Eon_Canvas] Destination quad =\n            %fx%f\n            %fx%f\n            %fx%f\n            %fx%f\n", x1, y1, x2, y2, x3, y3, x4, y4);
		enesim_matrix_rect_transform(&prv->inverse, &r, &qi);
		enesim_quad_coords_get(&qi, &x1, &y1, &x2, &y2, &x3, &y3, &x4, &y4);
		printf("[Eon_Canvas] Source quad =\n            %fx%f\n            %fx%f\n            %fx%f\n            %fx%f\n", x1, y1, x2, y2, x3, y3, x4, y4);
	}
	printf("[Eon_Canvas] Setting geometry of size %d %d %d %d\n",
			r.x, r.y, r.w, r.h);
	ekeko_renderable_geometry_set((Ekeko_Renderable *)c, &r);
}

/* Just informs that the x.final property has to be recalculated */
static void _x_inform(const Ekeko_Object *o, Ekeko_Event *e, void *data)
{
	Eon_Canvas_Private *prv = PRIVATE(data);
	Ekeko_Value v;

	printf("[Eon_Canvas] Informing X change\n");
	eon_value_coord_from(&v, &prv->x);
	ekeko_object_property_value_set((Ekeko_Object *)data, "x", &v);
}

/* Just informs that the y.final property has to be recalculated */
static void _y_inform(const Ekeko_Object *o, Ekeko_Event *e, void *data)
{
	Eon_Canvas_Private *prv = PRIVATE(data);
	Ekeko_Value v;

	printf("[Eon_Canvas] Informing Y change\n");
	eon_value_coord_from(&v, &prv->y);
	ekeko_object_property_value_set((Ekeko_Object *)data, "y", &v);
}

/* Just informs that the w.final property has to be recalculated */
static void _w_inform(const Ekeko_Object *o, Ekeko_Event *e, void *data)
{
	Eon_Canvas_Private *prv = PRIVATE(data);
	Ekeko_Value v;

	printf("[Eon_Canvas] Informing W change\n");
	eon_value_coord_from(&v, &prv->w);
	ekeko_object_property_value_set((Ekeko_Object *)data, "w", &v);
}

/* Just informs that the h.final property has to be recalculated */
static void _h_inform(const Ekeko_Object *o, Ekeko_Event *e, void *data)
{
	Eon_Canvas_Private *prv = PRIVATE(data);
	Ekeko_Value v;

	printf("[Eon_Canvas] Informing H change\n");
	eon_value_coord_from(&v, &prv->h);
	ekeko_object_property_value_set((Ekeko_Object *)data, "h", &v);
}

/* Called whenever the x property changes */
static void _x_change(const Ekeko_Object *o, Ekeko_Event *e, void *data)
{
	Ekeko_Event_Mutation *em = (Ekeko_Event_Mutation *)e;
	Eon_Canvas *c = (Eon_Canvas *)o;
	Eon_Canvas_Private *prv = PRIVATE(o);
	Ekeko_Object *parent;

	if (em->state == EVENT_MUTATION_STATE_POST)
		return;
	printf("setting x value\n");
	parent = ekeko_object_parent_get(o);
	if (prv->root)
	{
		int w;

		eon_document_size_get((Eon_Document *)parent, &w, NULL);
		printf("w = %d\n", w);
		eon_coord_change(o, &prv->x, em->curr->value.pointer_value,
				em->prev->value.pointer_value, 0, w, parent,
				NULL, EON_DOCUMENT_SIZE_CHANGED, _x_inform);
		{
			Eon_Coord *coord = em->curr->value.pointer_value;
			printf("X Change = %d %d %d\n", coord->final, coord->value, coord->type);
		}
	}
	else
	{
		Eon_Canvas *cp = (Eon_Canvas *)parent;
		Eon_Canvas_Private *cprv = PRIVATE(cp);

		eon_coord_change(o, &prv->x, em->curr->value.pointer_value,
				em->prev->value.pointer_value, cprv->x.final, cprv->w.final, parent,
				EON_CANVAS_X_CHANGED, EON_CANVAS_W_CHANGED,
				_x_inform);
	}
	_geometry_calc(c);
}

/* Called whenever the y property changes */
static void _y_change(const Ekeko_Object *o, Ekeko_Event *e, void *data)
{
	Ekeko_Event_Mutation *em = (Ekeko_Event_Mutation *)e;
	Eon_Canvas *c = (Eon_Canvas *)o;
	Eon_Canvas_Private *prv = PRIVATE(o);
	Ekeko_Object *parent;

	if (em->state == EVENT_MUTATION_STATE_POST)
		return;

	parent = ekeko_object_parent_get(o);
	if (prv->root)
	{
		int h;

		eon_document_size_get((Eon_Document *)parent, NULL, &h);
		eon_coord_change(o, &prv->y, em->curr->value.pointer_value,
				em->prev->value.pointer_value, 0, h, parent,
				NULL, EON_DOCUMENT_SIZE_CHANGED, _y_inform);
	}
	else
	{
		Eon_Canvas *cp = (Eon_Canvas *)parent;
		Eon_Canvas_Private *cprv = PRIVATE(cp);

		eon_coord_change(o, &prv->y, em->curr->value.pointer_value,
				em->prev->value.pointer_value, cprv->y.final, cprv->h.final, parent,
				EON_CANVAS_Y_CHANGED, EON_CANVAS_H_CHANGED, _y_inform);
	}
	_geometry_calc(c);
}

/* Called whenever the w property changes */
static void _w_change(const Ekeko_Object *o, Ekeko_Event *e, void *data)
{
	Ekeko_Event_Mutation *em = (Ekeko_Event_Mutation *)e;
	Eon_Canvas *c = (Eon_Canvas *)o;
	Eon_Canvas_Private *prv = PRIVATE(o);
	Ekeko_Object *parent;

	if (em->state == EVENT_MUTATION_STATE_POST)
		return;

	parent = ekeko_object_parent_get(o);
	if (prv->root)
	{
		int w;

		eon_document_size_get((Eon_Document *)parent, &w, NULL);
		eon_coord_length_change(o, &prv->w, em->curr->value.pointer_value,
				em->prev->value.pointer_value, w, parent,
				EON_DOCUMENT_SIZE_CHANGED, _w_inform);
	}
	else
	{
		Eon_Canvas *cp = (Eon_Canvas *)parent;
		Eon_Canvas_Private *cprv = PRIVATE(cp);

		eon_coord_length_change(o, &prv->w, em->curr->value.pointer_value,
				em->prev->value.pointer_value, cprv->w.final, parent,
				EON_CANVAS_W_CHANGED, _w_inform);
	}
	{
		printf("[Eon_Canvas] W Change = %d\n", prv->w.final);
	}
	_geometry_calc(c);
}

/* Called whenever the h property changes */
static void _h_change(const Ekeko_Object *o, Ekeko_Event *e, void *data)
{
	Ekeko_Event_Mutation *em = (Ekeko_Event_Mutation *)e;
	Eon_Canvas *c = (Eon_Canvas *)o;
	Eon_Canvas_Private *prv = PRIVATE(o);
	Ekeko_Object *parent;

	if (em->state == EVENT_MUTATION_STATE_POST)
		return;

	parent = ekeko_object_parent_get(o);
	if (prv->root)
	{
		int h;

		eon_document_size_get((Eon_Document *)parent, NULL, &h);
		eon_coord_length_change(o, &prv->h, em->curr->value.pointer_value,
				em->prev->value.pointer_value, h, parent,
				EON_DOCUMENT_SIZE_CHANGED, _h_inform);
	}
	else
	{
		Eon_Canvas *cp = (Eon_Canvas *)parent;
		Eon_Canvas_Private *cprv = PRIVATE(cp);

		eon_coord_length_change(o, &prv->h, em->curr->value.pointer_value,
				em->prev->value.pointer_value, cprv->h.final, parent,
				EON_CANVAS_H_CHANGED, _h_inform);
	}
	{
		printf("[Eon_Canvas] H Change = %d\n", prv->h.final);
	}
	_geometry_calc(c);
}

static void _matrix_change(const Ekeko_Object *o, Ekeko_Event *e, void *data)
{
	Ekeko_Event_Mutation *em = (Ekeko_Event_Mutation *)e;
	Eon_Canvas *c = (Eon_Canvas *)o;
	Eon_Canvas_Private *prv = PRIVATE(o);
	Enesim_Matrix *m;

	m = em->curr->value.pointer_value;
	enesim_matrix_inverse(m, &prv->inverse);
	/* update the geometry */
	_geometry_calc(c);
}

static void _child_append_cb(const Ekeko_Object *o, Ekeko_Event *e, void *data)
{
	Eon_Canvas *c;
	Eon_Canvas_Private *prv;
	Ekeko_Event_Mutation *em = (Ekeko_Event_Mutation *)e;

	/* FIXME the canvas can only have one parent, either a document
	 * or a canvas
	 */
	c = (Eon_Canvas *)o;
	prv = PRIVATE(c);

	/* TODO whenever we attach to any parent, update the final oordinates
	 * in case the user has already set them up before attaching
	 */
	/* in case the parent is a document retrieve the engine
	 * in case the parent is a canvas retrieve the engine from the relative
	 * document
	 */
	/* parent is a document */
	if (!ekeko_type_instance_is_of(em->related, EON_TYPE_CANVAS))
	{
		prv->root = EINA_TRUE;
	}
}

static Eina_Bool _appendable(void *instance, void *child)
{
	if ((!ekeko_type_instance_is_of(child, EON_TYPE_CANVAS)) &&
			(!ekeko_type_instance_is_of(child, EON_TYPE_PAINT)) &&
			(!ekeko_type_instance_is_of(child, EON_TYPE_EXTERNAL)) &&
			(!ekeko_type_instance_is_of(child, EON_TYPE_ANIMATION)))
		return EINA_FALSE;
	return EINA_TRUE;
}

static void _ctor(Ekeko_Object *o)
{
	Eon_Canvas *c;
	Eon_Canvas_Private *prv;

	c = (Eon_Canvas *)o;
	c->private = prv = ekeko_type_instance_private_get(_type, o);
	c->parent.flush = _flush;
	c->parent.parent.render = _subcanvas_render;
	c->parent.parent.is_inside= _subcanvas_is_inside;
	enesim_matrix_identity(&prv->matrix);
	enesim_matrix_inverse(&prv->matrix, &prv->inverse);
	ekeko_event_listener_add(o, EON_CANVAS_X_CHANGED, _x_change, EINA_FALSE, NULL);
	ekeko_event_listener_add(o, EON_CANVAS_Y_CHANGED, _y_change, EINA_FALSE, NULL);
	ekeko_event_listener_add(o, EON_CANVAS_W_CHANGED, _w_change, EINA_FALSE, NULL);
	ekeko_event_listener_add(o, EON_CANVAS_H_CHANGED, _h_change, EINA_FALSE, NULL);
	ekeko_event_listener_add(o, EON_CANVAS_MATRIX_CHANGED, _matrix_change, EINA_FALSE, NULL);
	ekeko_event_listener_add(o, EKEKO_RENDERABLE_GEOMETRY_CHANGED, _geometry_change, EINA_FALSE, NULL);
	ekeko_event_listener_add(o, EKEKO_EVENT_OBJECT_APPEND, _child_append_cb, EINA_FALSE, NULL);
}

static void _dtor(void *canvas)
{

}
/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/
void * eon_canvas_engine_data_get(Eon_Canvas *c)
{
	Eon_Canvas_Private *prv;

	prv = PRIVATE(c);
	return prv->engine_data;
}

void eon_canvas_init(void)
{
	_type = ekeko_type_new(EON_TYPE_CANVAS, sizeof(Eon_Canvas),
			sizeof(Eon_Canvas_Private), ekeko_canvas_type_get(),
			_ctor, _dtor, _appendable);

	EON_CANVAS_X = EKEKO_TYPE_PROP_SINGLE_ADD(_type, "x",
			EON_PROPERTY_COORD, OFFSET(Eon_Canvas_Private, x));
	EON_CANVAS_Y = EKEKO_TYPE_PROP_SINGLE_ADD(_type, "y",
			EON_PROPERTY_COORD, OFFSET(Eon_Canvas_Private, y));
	EON_CANVAS_W = EKEKO_TYPE_PROP_SINGLE_ADD(_type, "w",
			EON_PROPERTY_COORD, OFFSET(Eon_Canvas_Private, w));
	EON_CANVAS_H = EKEKO_TYPE_PROP_SINGLE_ADD(_type, "h",
			EON_PROPERTY_COORD, OFFSET(Eon_Canvas_Private, h));
	EON_CANVAS_MATRIX = EKEKO_TYPE_PROP_SINGLE_ADD(_type, "matrix",
			EON_PROPERTY_MATRIX,
			OFFSET(Eon_Canvas_Private, matrix));

	eon_type_register(_type, EON_TYPE_CANVAS);
}

void eon_canvas_shutdown(void)
{
	eon_type_unregister(_type);
}


/*============================================================================*
 *                                   API                                      *
 *============================================================================*/
Ekeko_Property_Id EON_CANVAS_X;
Ekeko_Property_Id EON_CANVAS_Y;
Ekeko_Property_Id EON_CANVAS_W;
Ekeko_Property_Id EON_CANVAS_H;
Ekeko_Property_Id EON_CANVAS_MATRIX;

EAPI Eon_Canvas * eon_canvas_new(Eon_Document *d)
{
	Eon_Canvas *c;
	Eon_Canvas_Private *prv;


	c = eon_document_object_new(d, EON_TYPE_CANVAS);
	if (!c) return NULL;

	prv = PRIVATE(c);
	prv->doc = d;

	return c;
}

EAPI Eon_Document * eon_canvas_document_get(Eon_Canvas *c)
{
	Eon_Canvas_Private *prv;

	prv = PRIVATE(c);
	return prv->doc;
}

EAPI void eon_canvas_x_rel_set(Eon_Canvas *c, int x)
{
	Eon_Coord coord;
	Ekeko_Value v;

	eon_coord_set(&coord, x, EON_COORD_RELATIVE);
	eon_value_coord_from(&v, &coord);
	printf("x set %d %d\n", x, coord.value);
	ekeko_object_property_value_set((Ekeko_Object *)c, "x", &v);
}

EAPI void eon_canvas_x_set(Eon_Canvas *c, int x)
{
	Eon_Coord coord;
	Ekeko_Value v;

	eon_coord_set(&coord, x, EON_COORD_ABSOLUTE);
	eon_value_coord_from(&v, &coord);
	ekeko_object_property_value_set((Ekeko_Object *)c, "x", &v);
}

EAPI void eon_canvas_y_set(Eon_Canvas *c, int y)
{
	Eon_Coord coord;
	Ekeko_Value v;

	eon_coord_set(&coord, y, EON_COORD_ABSOLUTE);
	eon_value_coord_from(&v, &coord);
	ekeko_object_property_value_set((Ekeko_Object *)c, "y", &v);
}

EAPI void eon_canvas_y_rel_set(Eon_Canvas *c, int y)
{
	Eon_Coord coord;
	Ekeko_Value v;

	eon_coord_set(&coord, y, EON_COORD_RELATIVE);
	eon_value_coord_from(&v, &coord);
	ekeko_object_property_value_set((Ekeko_Object *)c, "y", &v);
}

EAPI void eon_canvas_w_set(Eon_Canvas *c, int w)
{
	Eon_Coord coord;
	Ekeko_Value v;

	eon_coord_set(&coord, w, EON_COORD_ABSOLUTE);
	eon_value_coord_from(&v, &coord);
	ekeko_object_property_value_set((Ekeko_Object *)c, "w", &v);
}

EAPI void eon_canvas_w_rel_set(Eon_Canvas *c, int w)
{
	Eon_Coord coord;
	Ekeko_Value v;

	eon_coord_set(&coord, w, EON_COORD_RELATIVE);
	eon_value_coord_from(&v, &coord);
	ekeko_object_property_value_set((Ekeko_Object *)c, "w", &v);
}

EAPI void eon_canvas_h_set(Eon_Canvas *c, int h)
{
	Eon_Coord coord;
	Ekeko_Value v;

	eon_coord_set(&coord, h, EON_COORD_ABSOLUTE);
	eon_value_coord_from(&v, &coord);
	ekeko_object_property_value_set((Ekeko_Object *)c, "h", &v);
}

EAPI void eon_canvas_h_rel_set(Eon_Canvas *c, int h)
{
	Eon_Coord coord;
	Ekeko_Value v;

	eon_coord_set(&coord, h, EON_COORD_RELATIVE);
	eon_value_coord_from(&v, &coord);
	ekeko_object_property_value_set((Ekeko_Object *)c, "h", &v);
}

EAPI void eon_canvas_x_get(Eon_Canvas *c, Eon_Coord *x)
{
	Eon_Canvas_Private *prv;

	prv = PRIVATE(c);
	*x = prv->x;
}

EAPI void eon_canvas_y_get(Eon_Canvas *c, Eon_Coord *y)
{
	Eon_Canvas_Private *prv;

	prv = PRIVATE(c);
	*y = prv->y;
}

EAPI void eon_canvas_w_get(Eon_Canvas *c, Eon_Coord *w)
{
	Eon_Canvas_Private *prv;

	prv = PRIVATE(c);
	*w = prv->w;
}

EAPI void eon_canvas_h_get(Eon_Canvas *c, Eon_Coord *h)
{
	Eon_Canvas_Private *prv;

	prv = PRIVATE(c);
	*h = prv->h;
}

EAPI void eon_canvas_matrix_set(Eon_Canvas *c, Enesim_Matrix *m)
{
	Ekeko_Value v;

	eon_value_matrix_from(&v, m);
	ekeko_object_property_value_set((Ekeko_Object *)c, "matrix", &v);
}

