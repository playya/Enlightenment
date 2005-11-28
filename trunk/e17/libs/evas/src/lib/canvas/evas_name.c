#include "evas_common.h"
#include "evas_private.h"

/**
 * @defgroup Evas_Object_Name_Group Object Name Function
 *
 * Functions that retrieve and set the name of an evas object.
 */

/**
 * Sets the name of the given evas object to the given name.
 * @param   obj  The given object.
 * @param   name The given name.
 * @ingroup Evas_Object_Name_Group
 */
void
evas_object_name_set(Evas_Object *obj, const char *name)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (obj->name)
     {
	obj->layer->evas->name_hash = evas_hash_del(obj->layer->evas->name_hash, obj->name, obj);
	evas_stringshare_del(obj->name);
     }
   if (!name) obj->name = NULL;
   else
     {
	obj->name = evas_stringshare_add(name);
	obj->layer->evas->name_hash = evas_hash_direct_add(obj->layer->evas->name_hash, obj->name, obj);
     }
}

/**
 * Retrieves the name of the given evas object.
 * @param   obj The given object.
 * @return  The name of the object.  @c NULL if no name has been given
 *          to the object.
 * @ingroup Evas_Object_Name_Group
 */
const char *
evas_object_name_get(Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   return obj->name;
}

/**
 * Retrieves the object on the given evas with the given name.
 * @param   e    The given evas.
 * @param   name The given name.
 * @return  If successful, the evas object with the given name.  Otherwise,
 *          @c NULL.
 * @ingroup Evas_Object_Name_Group
 */
Evas_Object *
evas_object_name_find(Evas *e, const char *name)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   if (!name) return NULL;
   return (Evas_Object *)evas_hash_find(e->name_hash, name);
}
