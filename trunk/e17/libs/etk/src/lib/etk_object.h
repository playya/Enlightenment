/** @file etk_object.h */
#ifndef _ETK_OBJECT_H_
#define _ETK_OBJECT_H_

#include <Ecore_Data.h>
#include <stdarg.h>
#include "etk_type.h"
#include "etk_types.h"

/**
 * @defgroup Etk_Object Etk_Object
 * @{
 */

#ifndef ETK_DISABLE_CAST_CHECKS
#  define ETK_OBJECT_CAST(obj, etk_type, c_type)      ((c_type *)etk_object_check_cast((Etk_Object *)(obj), (etk_type)))
#else
#  define ETK_OBJECT_CAST(obj, etk_type, c_type)      ((c_type *)(obj))
#endif
#define ETK_OBJECT_CHECK_TYPE(obj, etk_type)          (etk_type_inherits_from(((Etk_Object *)(obj))->type, (etk_type)))

/** @brief Gets the type of an object */
#define ETK_OBJECT_TYPE       (etk_object_type_get())
/** @brief Casts the object to an Etk_Object */
#define ETK_OBJECT(obj)       (ETK_OBJECT_CAST((obj), ETK_OBJECT_TYPE, Etk_Object))
/** @brief Checks if the object is an Etk_Object (should be always true) */
#define ETK_IS_OBJECT(obj)    (ETK_OBJECT_CHECK_TYPE((obj), ETK_OBJECT_TYPE))

enum _Etk_Object_Signal_Id
{
   ETK_OBJECT_DESTROY_SIGNAL,
   ETK_OBJECT_NUM_SIGNALS
};

/**
 * @struct Etk_Notification_Callback
 * @brief A callback called each time the associated property is changed
 */
struct _Etk_Notification_Callback
{
   Etk_Notification_Callback_Function callback;
   void *data;
};

/**
 * @struct Etk_Object
 * @brief An Etk_Object is the base of all the widgets: it implements features such as inheritance, constructors, properties, signals.
 */
struct _Etk_Object
{
   /* private: */
   Etk_Type *type;
   Ecore_Hash *data_hash;
   Ecore_List *before_signal_callbacks_list;
   Ecore_List *after_signal_callbacks_list;
   Ecore_Hash *notification_callbacks_hash;
   Ecore_List *weak_pointers_list;
};

Etk_Bool etk_object_init();
void etk_object_shutdown();

Etk_Type *etk_object_type_get();
Etk_Object *etk_object_new(Etk_Type *object_type, const char *first_property, ...);
Etk_Object *etk_object_new_valist(Etk_Type *object_type, const char *first_property, va_list args);
void etk_object_destroy(Etk_Object *object);
void etk_object_destroy_all_objects();

Etk_Object *etk_object_check_cast(Etk_Object *object, Etk_Type *type);
Etk_Type *etk_object_object_type_get(Etk_Object *object);

void etk_object_signal_callback_add(Etk_Object *object, Etk_Signal_Callback *signal_callback, Etk_Bool after);
void etk_object_signal_callback_remove(Etk_Object *object, Etk_Signal_Callback *signal_callback);
void etk_object_signal_callbacks_get(Etk_Object *object, Etk_Signal *signal, Ecore_List *callbacks, Etk_Bool after);

void etk_object_weak_pointer_add(Etk_Object *object, void **pointer_location);
void etk_object_weak_pointer_remove(Etk_Object *object, void **pointer_location);

void etk_object_data_set(Etk_Object *object, const char *key, void *value);
void *etk_object_data_get(Etk_Object *object, const char *key);

void etk_object_property_reset(Etk_Object *object, const char *property_name);
void etk_object_properties_set(Etk_Object *object, const char *first_property, ...);
void etk_object_properties_set_valist(Etk_Object *object, const char *first_property, va_list args);
void etk_object_properties_get(Etk_Object *object, const char *first_property, ...);
void etk_object_properties_get_valist(Etk_Object *object, const char *first_property, va_list args);

void etk_object_notify(Etk_Object *object, const char *property_name);
void etk_object_notification_callback_add(Etk_Object *object, const char *property_name, Etk_Notification_Callback_Function callback, void *data);
void etk_object_notification_callback_remove(Etk_Object *object, const char *property_name, Etk_Notification_Callback_Function callback);

/** @} */

#endif
