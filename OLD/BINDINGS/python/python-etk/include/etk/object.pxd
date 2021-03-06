# Copyright (C) 2007-2008 Caio Marcelo de Oliveira Filho, Gustavo Sverzut Barbieri
#
# This file is part of Python-Etk.
#
# Python-Etk is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# Python-Etk is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this Python-Etk.  If not, see <http://www.gnu.org/licenses/>.

cdef extern from "etk_object.h":
    ####################################################################
    # Signals
    int ETK_OBJECT_DESTROYED_SIGNAL

    ####################################################################
    # Enumerations
    ####################################################################
    # Structures
    ctypedef struct Etk_Notification_Callback

    ####################################################################
    # Functions
    Etk_Object* etk_object_name_find(char* name)
    Etk_Object* etk_object_new_valist(Etk_Type* object_type, char* first_property, va_list args)
    void etk_object_purge()
    void etk_object_shutdown()
    Etk_Type* etk_object_type_get()
    Etk_Object* etk_object_new(Etk_Type* object_type, char* first_property)
    Etk_Object* etk_object_check_cast(Etk_Object* __self, Etk_Type* type)
    void* etk_object_data_get(Etk_Object* __self, char* key)
    void etk_object_data_set(Etk_Object* __self, char* key, void* value)
    void etk_object_data_set_full(Etk_Object* __self, char* key, void* value)
    void etk_object_destroy(Etk_Object* __self)
    char* etk_object_name_get(Etk_Object* __self)
    void etk_object_name_set(Etk_Object* __self, char* name)
    void etk_object_notification_callback_add(Etk_Object* __self, char* property_name)
    void etk_object_notification_callback_remove(Etk_Object* __self, char* property_name)
    void etk_object_notify(Etk_Object* __self, char* property_name)
    Etk_Type* etk_object_object_type_get(Etk_Object* __self)
    void etk_object_properties_get(Etk_Object* __self, char* first_property)
    void etk_object_properties_get_valist(Etk_Object* __self, char* first_property, va_list args)
    void etk_object_properties_set(Etk_Object* __self, char* first_property)
    void etk_object_properties_set_valist(Etk_Object* __self, char* first_property, va_list args)
    void etk_object_property_reset(Etk_Object* __self, char* property_name)
    void etk_object_signal_callback_add(Etk_Object* __self, Etk_Signal_Callback* signal_callback, int after)
    void etk_object_signal_callback_remove(Etk_Object* __self, Etk_Signal_Callback* signal_callback)
    void etk_object_signal_callbacks_get(Etk_Object* __self, Etk_Signal* signal, Eina_List** callbacks)
    void etk_object_weak_pointer_add(Etk_Object* __self, void** pointer_location)
    void etk_object_weak_pointer_remove(Etk_Object* __self, void** pointer_location)

    void etk_marshaller_VOID(Etk_Callback callback, Etk_Object *object, void *data, void *return_value, va_list arguments)
    void etk_marshaller_INT(Etk_Callback callback, Etk_Object *object, void *data, void *return_value, va_list arguments)
    void etk_marshaller_INT_INT(Etk_Callback callback, Etk_Object *object, void *data, void *return_value, va_list arguments)
    void etk_marshaller_DOUBLE(Etk_Callback callback, Etk_Object *object, void *data, void *return_value, va_list arguments)
    void etk_marshaller_OBJECT(Etk_Callback callback, Etk_Object *object, void *data, void *return_value, va_list arguments)
    void etk_marshaller_POINTER(Etk_Callback callback, Etk_Object *object, void *data, void *return_value, va_list arguments)
    void etk_marshaller_POINTER_POINTER(Etk_Callback callback, Etk_Object *object, void *data, void *return_value, va_list arguments)
    void etk_marshaller_INT_POINTER(Etk_Callback callback, Etk_Object *object, void *data, void *return_value, va_list arguments)

#########################################################################
# Objects
cdef public class Object [object PyEtk_Object, type PyEtk_Object_Type]:
    cdef Etk_Object *obj
    cdef object _data
    cdef object _connections

    cdef int _unset_obj(self) except 0
    cdef object _set_obj(self, Etk_Object *obj)
