#include "Eon.h"
#include "eon_private.h"
/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/
#define EON_IMAGE_DEBUG 0
#define PRIVATE(d) ((Eon_Stripes_Private *)((Eon_Stripes *)(d))->private)

struct _Eon_Stripes_Private
{
	Eon_Color color1, color2;
	float thickness1, thickness2;
};

static void _ctor(void *instance)
{
	Eon_Stripes *sq;
	Eon_Stripes_Private *prv;

	sq = (Eon_Stripes *)instance;
	sq->private = prv = ekeko_type_instance_private_get(eon_stripes_type_get(), instance);
	sq->parent.create = eon_engine_stripes_create;
	sq->parent.setup = eon_engine_stripes_setup;
	sq->parent.delete = eon_engine_stripes_delete;
}

static void _dtor(void *image)
{

}
/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/
/*============================================================================*
 *                                   API                                      *
 *============================================================================*/
Ekeko_Property_Id EON_STRIPES_COLOR1;
Ekeko_Property_Id EON_STRIPES_COLOR2;
Ekeko_Property_Id EON_STRIPES_THICKNESS1;
Ekeko_Property_Id EON_STRIPES_THICKNESS2;

EAPI Ekeko_Type *eon_stripes_type_get(void)
{
	static Ekeko_Type *type = NULL;

	if (!type)
	{
		type = ekeko_type_new(EON_TYPE_STRIPES, sizeof(Eon_Stripes),
				sizeof(Eon_Stripes_Private), eon_paint_type_get(),
				_ctor, _dtor, eon_paint_appendable);
		EON_STRIPES_COLOR1 = EKEKO_TYPE_PROP_SINGLE_ADD(type, "color1", EON_PROPERTY_COLOR, OFFSET(Eon_Stripes_Private, color1));
		EON_STRIPES_COLOR2 = EKEKO_TYPE_PROP_SINGLE_ADD(type, "color2", EON_PROPERTY_COLOR, OFFSET(Eon_Stripes_Private, color2));
		EON_STRIPES_THICKNESS1 = EKEKO_TYPE_PROP_SINGLE_ADD(type, "thickness1", EKEKO_PROPERTY_FLOAT, OFFSET(Eon_Stripes_Private, thickness1));
		EON_STRIPES_THICKNESS2 = EKEKO_TYPE_PROP_SINGLE_ADD(type, "thickness2", EKEKO_PROPERTY_FLOAT, OFFSET(Eon_Stripes_Private, thickness2));
	}

	return type;
}

EAPI Eon_Stripes * eon_stripes_new(void)
{
	Eon_Stripes *sq;

	sq = ekeko_type_instance_new(eon_stripes_type_get());

	return sq;
}

EAPI void eon_stripes_thickness1_set(Eon_Stripes *s, float th)
{
	Ekeko_Value v;

	ekeko_value_float_from(&v, th);
	ekeko_object_property_value_set((Ekeko_Object *)s, "thickness1", &v);
}

EAPI float eon_stripes_thickness1_get(Eon_Stripes *s)
{
	Eon_Stripes_Private *prv = PRIVATE(s);

	return prv->thickness1;
}

EAPI void eon_stripes_thickness2_set(Eon_Stripes *s, float th)
{
	Ekeko_Value v;

	ekeko_value_float_from(&v, th);
	ekeko_object_property_value_set((Ekeko_Object *)s, "thickness2", &v);
}

EAPI float eon_stripes_thickness2_get(Eon_Stripes *s)
{
	Eon_Stripes_Private *prv = PRIVATE(s);

	return prv->thickness2;
}

EAPI Eon_Color eon_stripes_color1_get(Eon_Stripes *s)
{
	Eon_Stripes_Private *prv = PRIVATE(s);

	return prv->color1;
}

EAPI Eon_Color eon_stripes_color2_get(Eon_Stripes *s)
{
	Eon_Stripes_Private *prv = PRIVATE(s);

	return prv->color2;
}

EAPI void eon_stripes_color1_set(Eon_Stripes *s, Eon_Color color)
{
	Ekeko_Value v;

	eon_value_color_from(&v, color);
	ekeko_object_property_value_set((Ekeko_Object *)s, "color1", &v);
}

EAPI void eon_stripes_color2_set(Eon_Stripes *s, Eon_Color color)
{
	Ekeko_Value v;

	eon_value_color_from(&v, color);
	ekeko_object_property_value_set((Ekeko_Object *)s, "color2", &v);
}
