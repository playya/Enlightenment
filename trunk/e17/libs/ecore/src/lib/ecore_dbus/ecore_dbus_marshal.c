/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "ecore_private.h"
#include "Ecore_Con.h"
#include "Ecore_DBus.h"
#include "ecore_dbus_private.h"

Ecore_DBus_Message_Field_Byte *
_ecore_dbus_message_marshal_byte(Ecore_DBus_Message *msg, unsigned char c)
{
   Ecore_DBus_Message_Field_Byte *f;

   f = _ecore_dbus_message_field_new(msg, ECORE_DBUS_DATA_TYPE_BYTE);
   f->value = c;
   _ecore_dbus_message_append_byte(msg, c);
   return f;
}

Ecore_DBus_Message_Field_UInt32 *
_ecore_dbus_message_marshal_uint32(Ecore_DBus_Message *msg, unsigned int i)
{
   Ecore_DBus_Message_Field_UInt32 *f;
   unsigned char                   *c;

   _ecore_dbus_message_padding(msg, 4);
   f = _ecore_dbus_message_field_new(msg, ECORE_DBUS_DATA_TYPE_UINT32);
   f->value = i;
   c = (unsigned char *)&i;
   _ecore_dbus_message_append_bytes(msg, c, 4);
   return f;
}

Ecore_DBus_Message_Field_String *
_ecore_dbus_message_marshal_string(Ecore_DBus_Message *msg, char *str)
{
   Ecore_DBus_Message_Field_String *f;
   unsigned int                     str_len;

   _ecore_dbus_message_padding(msg, 4);
   f = _ecore_dbus_message_field_new(msg, ECORE_DBUS_DATA_TYPE_STRING);
   f->value = str;

   str_len = strlen(str);
   _ecore_dbus_message_append_uint32(msg, str_len);

   /* + 1 for \0 */
   _ecore_dbus_message_append_bytes(msg, (unsigned char *)str, str_len + 1);
   return f;
}

Ecore_DBus_Message_Field_Object_Path *
_ecore_dbus_message_marshal_object_path(Ecore_DBus_Message *msg, char *str)
{
   Ecore_DBus_Message_Field_Object_Path *f;
   unsigned int                          str_len;

   _ecore_dbus_message_padding(msg, 4);
   f = _ecore_dbus_message_field_new(msg, ECORE_DBUS_DATA_TYPE_OBJECT_PATH);
   f->value = str;

   str_len = strlen(str);
   _ecore_dbus_message_append_uint32(msg, str_len);

   /* + 1 for \0 */
   _ecore_dbus_message_append_bytes(msg, (unsigned char *)str, str_len + 1);
   return f;
}

Ecore_DBus_Message_Field_Signature *
_ecore_dbus_message_marshal_signature(Ecore_DBus_Message *msg, char *str)
{
   Ecore_DBus_Message_Field_Signature *f;
   unsigned int                        str_len;

   f = _ecore_dbus_message_field_new(msg, ECORE_DBUS_DATA_TYPE_SIGNATURE);
   f->value = str;

   str_len = strlen(str);
   _ecore_dbus_message_append_byte(msg, str_len);

   _ecore_dbus_message_append_bytes(msg, (unsigned char *)str, str_len + 1);
   return f;
}

Ecore_DBus_Message_Field_Array *
_ecore_dbus_message_marshal_array_begin(Ecore_DBus_Message *msg,
					Ecore_DBus_Data_Type contained_type)
{
   Ecore_DBus_Message_Field_Array *arr;

   _ecore_dbus_message_padding(msg, 4);

   arr = _ecore_dbus_message_field_new(msg, ECORE_DBUS_DATA_TYPE_ARRAY);
   /* leave room for the array length value, gets filled in on array_end() */
   _ecore_dbus_message_append_uint32(msg, 0);
   arr->contained_type = contained_type;
   ecore_list_prepend(msg->recurse, arr);

   /* pad for contained type */
   _ecore_dbus_message_padding(msg, _ecore_dbus_alignment_get(contained_type));
   arr->start = msg->length;

   return arr;
}

void
_ecore_dbus_message_marshal_array_end(Ecore_DBus_Message *msg, Ecore_DBus_Message_Field_Array *arr)
{
   ecore_list_remove_first(msg->recurse);
   arr->end = msg->length;
   *(unsigned int *)ECORE_DBUS_MESSAGE_FIELD(arr)->buffer = arr->end - arr->start;
}

Ecore_DBus_Message_Field_Array *
_ecore_dbus_message_marshal_array(Ecore_DBus_Message *msg, char *contained_type, Ecore_List *data)
{
   Ecore_DBus_Message_Field_Array *arr;
   void *el;

   printf("[ecore_dbus] marshal array %c\n", *contained_type);
   arr = _ecore_dbus_message_marshal_array_begin(msg, *contained_type);
   ecore_list_goto_first(data);
   while ((el = ecore_list_next(data)))
	_ecore_dbus_message_marshal(msg, contained_type, el);
   _ecore_dbus_message_marshal_array_end(msg, arr);

   return arr;
}

Ecore_DBus_Message_Field_Struct *
_ecore_dbus_message_marshal_struct_begin(Ecore_DBus_Message *msg)
{
   Ecore_DBus_Message_Field_Struct *s;

   _ecore_dbus_message_padding(msg, 8);
   s = _ecore_dbus_message_field_new(msg, ECORE_DBUS_DATA_TYPE_STRUCT);
   ecore_list_prepend(msg->recurse, s);

   return s;
}

void
_ecore_dbus_message_marshal_struct_end(Ecore_DBus_Message *msg, Ecore_DBus_Message_Field_Struct *s)
{
   ecore_list_remove_first(msg->recurse);
}

Ecore_DBus_Message_Field_Variant *
_ecore_dbus_message_marshal_variant(Ecore_DBus_Message *msg, Ecore_DBus_Data_Type type, void *data)
{
   Ecore_DBus_Message_Field_Variant *f = NULL;

   f = _ecore_dbus_message_field_new(msg, ECORE_DBUS_DATA_TYPE_VARIANT);
   ecore_list_prepend(msg->recurse, f);
   f->contained_type = type;

   /* signature length */
   _ecore_dbus_message_append_byte(msg, 1);
   /* signature */
   _ecore_dbus_message_append_byte(msg, type);
   _ecore_dbus_message_append_byte(msg, '\0');

   f->value = _ecore_dbus_message_marshal(msg, (char *)&type, data);
   ecore_list_remove_first(msg->recurse);
   return f;
}



Ecore_DBus_Message_Field *
_ecore_dbus_message_marshal(Ecore_DBus_Message *msg, char *type, void *data)
{

   switch (*type)
     {
     case ECORE_DBUS_DATA_TYPE_UINT32:
	return (Ecore_DBus_Message_Field *)_ecore_dbus_message_marshal_uint32(msg, *(unsigned int *)data);
     case ECORE_DBUS_DATA_TYPE_STRING:
	return (Ecore_DBus_Message_Field *)_ecore_dbus_message_marshal_string(msg, (char *)data);
     case ECORE_DBUS_DATA_TYPE_OBJECT_PATH:
	return (Ecore_DBus_Message_Field *)_ecore_dbus_message_marshal_object_path(msg, (char *)data);
     case ECORE_DBUS_DATA_TYPE_SIGNATURE:
	return (Ecore_DBus_Message_Field *)_ecore_dbus_message_marshal_signature(msg, (char *)data);
     case ECORE_DBUS_DATA_TYPE_BYTE:
	return (Ecore_DBus_Message_Field *)_ecore_dbus_message_marshal_byte(msg, *(char *)data);
     case ECORE_DBUS_DATA_TYPE_ARRAY:
	return (Ecore_DBus_Message_Field *)_ecore_dbus_message_marshal_array(msg, type + 1, (Ecore_List *)data);  // we need to let the caller know how many fields were marshalled (e.g. how far to skip ahead in the type list)
     case ECORE_DBUS_DATA_TYPE_BOOLEAN:
     case ECORE_DBUS_DATA_TYPE_INT16:
     case ECORE_DBUS_DATA_TYPE_UINT16:
     case ECORE_DBUS_DATA_TYPE_INT32:
     case ECORE_DBUS_DATA_TYPE_INT64:
     case ECORE_DBUS_DATA_TYPE_UINT64:
     case ECORE_DBUS_DATA_TYPE_DOUBLE:
     case ECORE_DBUS_DATA_TYPE_VARIANT:
     case ECORE_DBUS_DATA_TYPE_STRUCT:
     case ECORE_DBUS_DATA_TYPE_STRUCT_BEGIN:
     case ECORE_DBUS_DATA_TYPE_STRUCT_END:
     case ECORE_DBUS_DATA_TYPE_DICT_ENTRY:
     case ECORE_DBUS_DATA_TYPE_DICT_ENTRY_BEGIN:
     case ECORE_DBUS_DATA_TYPE_DICT_ENTRY_END:
     case ECORE_DBUS_DATA_TYPE_INVALID:
	printf("[ecore_dbus] unhandled data type %c\n", *type);
	return NULL;
     default:
	printf("[ecore_dbus] unknown data type %c\n", *type);
	return NULL;
     }

}
