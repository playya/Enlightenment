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
#define PRIVATE(d) ((Eon_Object_Private *)((Eon_Object *)(d))->prv)

struct _Eon_Object_Private
{
	Eon_Document *doc;
	void *engine_data;
};

static void _object_ctor(Ekeko_Object *eo)
{
	Eon_Object *o = (Eon_Object *)eo;

	o->prv = ekeko_type_instance_private_get(eon_object_type_get(), o);
}

static void _object_dtor(Ekeko_Object *eo)
{
	/* TODO whenever the object is destroyed shall we delete the
	 * engine data here?
	 */
}
/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/
/**
 * Gets the engines data associated with an object
 * @param o The object to get the data from
 * @return The engine data
 */
void * eon_object_engine_data_get(Eon_Object *o)
{
	Eon_Object_Private *prv;

	prv = PRIVATE(o);
	return prv->engine_data;
}
/**
 * Sets the engine data on an object
 * @param o The object to set the data to
 * @param data The engine data
 */
void eon_object_engine_data_set(Eon_Object *o, void *data)
{
	Eon_Object_Private *prv;

	prv = PRIVATE(o);
	prv->engine_data = data;
}
/**
 * Sets the document to an object. The document is already
 * doing this by you whenever an object is created, you dont
 * need to call this yourself.
 * @param o The object to set the document to
 * @param d The document
 */
void eon_object_document_set(Eon_Object *o, Eon_Document *d)
{
	Eon_Object_Private *prv;

	prv = PRIVATE(o);
	prv->doc = d;
}

/**
 * Generic function that will only append an object if the both
 * are of type Eon_Object
 * @param parent The object you want the attch the child to
 * @param child The child object
 */
Eina_Bool eon_object_appendable(Ekeko_Object *parent, Ekeko_Object *child)
{

}
/*============================================================================*
 *                                   API                                      *
 *============================================================================*/
/**
 * Gets the type definition of an object
 * @return The type definition
 */
EAPI Ekeko_Type * eon_object_type_get(void)
{
	static Ekeko_Type *type = NULL;

	if (!type)
	{
		type = ekeko_type_new(EON_TYPE_OBJECT, sizeof(Eon_Object),
				sizeof(Eon_Object_Private),
				ekeko_object_type_get(), _object_ctor,
				_object_dtor, NULL);
	}
	return type;
}

/**
 * Gets the Eon_Document this object belongs to
 * @param o The Eon_Object to get the document from
 * @return The Eon_Document
 */
EAPI Eon_Document * eon_object_document_get(Eon_Object *o)
{
	Eon_Object_Private *prv;

	prv = PRIVATE(o);
	return prv->doc;
}
