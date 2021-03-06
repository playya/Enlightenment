/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "ecore_private.h"
#include "Ecore_Con.h"
#include "Ecore_DBus.h"
#include "ecore_dbus_private.h"

typedef void *(*Ecore_DBus_Unmarshal_Func)(Ecore_DBus_Message *msg, int *size);

static void *
_ecore_dbus_message_unmarshal_byte(Ecore_DBus_Message *msg, int *size)
{
   Ecore_DBus_Message_Field_Byte *f;
   unsigned int                   old_length;

   old_length = msg->length;
   f = _ecore_dbus_message_field_new(msg, ECORE_DBUS_DATA_TYPE_BYTE);
   f->value = _ecore_dbus_message_read_byte(msg);
   if (*size < (int)(msg->length - old_length))
     {
	/* TODO: Free message field */
	printf("Ecore_DBus: To few bytes for byte: got %d need %d\n",
	       *size, (msg->length - old_length));
	return NULL;
     }
   *size -= (msg->length - old_length);
   return f;
}

static void *
_ecore_dbus_message_unmarshal_boolean(Ecore_DBus_Message *msg, int *size)
{
   Ecore_DBus_Message_Field_Boolean *f;
   unsigned int                      old_length;

   old_length = msg->length;
   f = _ecore_dbus_message_field_new(msg, ECORE_DBUS_DATA_TYPE_BOOLEAN);
   f->value = _ecore_dbus_message_read_uint32(msg);
   if (*size < (int)(msg->length - old_length))
     {
	/* TODO: Free message field */
	printf("Ecore_DBus: To few bytes for boolean: got %d need %d\n",
	       *size, (msg->length - old_length));
	return NULL;
     }
   *size -= (msg->length - old_length);
   return f;
}

static void *
_ecore_dbus_message_unmarshal_int32(Ecore_DBus_Message *msg, int *size)
{
   Ecore_DBus_Message_Field_Int32 *f;
   unsigned int                    old_length;

   old_length = msg->length;
   _ecore_dbus_message_padding(msg, 4);
   f = _ecore_dbus_message_field_new(msg, ECORE_DBUS_DATA_TYPE_INT32);
   f->value = _ecore_dbus_message_read_uint32(msg);
   if (*size < (int)(msg->length - old_length))
     {
	/* TODO: Free message field */
	printf("Ecore_DBus: To few bytes for uint32: got %d need %d\n",
	       *size, (msg->length - old_length));
	return NULL;
     }
   *size -= (msg->length - old_length);
   return f;
}

static void *
_ecore_dbus_message_unmarshal_uint32(Ecore_DBus_Message *msg, int *size)
{
   Ecore_DBus_Message_Field_UInt32 *f;
   unsigned int                     old_length;

   old_length = msg->length;
   _ecore_dbus_message_padding(msg, 4);
   f = _ecore_dbus_message_field_new(msg, ECORE_DBUS_DATA_TYPE_UINT32);
   f->value = _ecore_dbus_message_read_uint32(msg);
   if (*size < (int)(msg->length - old_length))
     {
	/* TODO: Free message field */
	printf("Ecore_DBus: To few bytes for uint32: got %d need %d\n",
	       *size, (msg->length - old_length));
	return NULL;
     }
   *size -= (msg->length - old_length);
   return f;
}

static void *
_ecore_dbus_message_unmarshal_string(Ecore_DBus_Message *msg, int *size)
{
   Ecore_DBus_Message_Field_String *f;
   unsigned int                     str_len, old_length;

   old_length = msg->length;
   _ecore_dbus_message_padding(msg, 4);
   f = _ecore_dbus_message_field_new(msg, ECORE_DBUS_DATA_TYPE_STRING);
   str_len = _ecore_dbus_message_read_uint32(msg);
   f->value = (char *)msg->buffer + msg->length;
   msg->length += str_len + 1;
   if (*size < (int)(msg->length - old_length))
     {
	/* TODO: Free message field */
	printf("Ecore_DBus: To few bytes for string: got %d need %d\n",
	       *size, (msg->length - old_length));
	return NULL;
     }
   *size -= (msg->length - old_length);
   return f;
}

static void *
_ecore_dbus_message_unmarshal_object_path(Ecore_DBus_Message *msg, int *size)
{
   Ecore_DBus_Message_Field_Object_Path *f;
   unsigned int                          str_len, old_length;

   old_length = msg->length;
   _ecore_dbus_message_padding(msg, 4);
   f = _ecore_dbus_message_field_new(msg, ECORE_DBUS_DATA_TYPE_OBJECT_PATH);
   str_len = _ecore_dbus_message_read_uint32(msg);
   f->value = (char *)msg->buffer + msg->length;
   msg->length += str_len + 1;
   if (*size < (int)(msg->length - old_length))
     {
	/* TODO: Free message field */
	printf("Ecore_DBus: To few bytes for object path: got %d need %d\n",
	       *size, (msg->length - old_length));
	return NULL;
     }
   *size -= (msg->length - old_length);
   return f;
}

static void *
_ecore_dbus_message_unmarshal_signature(Ecore_DBus_Message *msg, int *size)
{
   Ecore_DBus_Message_Field_Signature *f;
   unsigned int                        str_len, old_length;

   old_length = msg->length;
   f = _ecore_dbus_message_field_new(msg, ECORE_DBUS_DATA_TYPE_SIGNATURE);
   str_len = _ecore_dbus_message_read_byte(msg);
   f->value = (char *)msg->buffer + msg->length;
   msg->length += str_len + 1;
   if (*size < (int)(msg->length - old_length))
     {
	/* TODO: Free message field */
	printf("Ecore_DBus: To few bytes for signature: got %d need %d\n",
	       *size, (msg->length - old_length));
	return NULL;
     }
   *size -= (msg->length - old_length);
   return f;
}

static void *
_ecore_dbus_message_unmarshal_array_begin(Ecore_DBus_Message *msg,
					  Ecore_DBus_Data_Type contained_type, int *size)
{
   Ecore_DBus_Message_Field_Array *arr;
   unsigned int                    old_length;

   old_length = msg->length;
   _ecore_dbus_message_padding_skip(msg, 4);

   arr = _ecore_dbus_message_field_new(msg, ECORE_DBUS_DATA_TYPE_ARRAY);
   arr->contained_type = contained_type;
   arr->start = 0;
   arr->end = _ecore_dbus_message_read_uint32(msg);

   _ecore_dbus_message_padding_skip(msg, _ecore_dbus_alignment_get(contained_type));
   arr->start = msg->length;
   arr->end += msg->length;

   ecore_list_prepend(msg->recurse, arr);

   if (*size < (int)(arr->end - old_length))
     {
	/* TODO: Free message field */
	printf("Ecore_DBus: To few bytes for array: got %d need %d\n",
	       *size, (arr->end - old_length));
	return NULL;
     }
   *size -= (msg->length - old_length);
   return arr;
}

static void
_ecore_dbus_message_unmarshal_array_end(Ecore_DBus_Message *msg,
					Ecore_DBus_Message_Field_Array *arr __UNUSED__)
{
   ecore_list_first_remove(msg->recurse);
}

static void *
_ecore_dbus_message_unmarshal_struct_begin(Ecore_DBus_Message *msg,
					   int *size)
{
   Ecore_DBus_Message_Field_Struct *s;
   unsigned int                     old_length;

   old_length = msg->length;
   _ecore_dbus_message_padding_skip(msg, 8);

   s = _ecore_dbus_message_field_new(msg, ECORE_DBUS_DATA_TYPE_STRUCT);
   ecore_list_prepend(msg->recurse, s);

   if (*size < (int)(msg->length - old_length))
     {
	/* TODO: Free message field */
	printf("Ecore_DBus: To few bytes for struct: got %d need %d\n",
	       *size, (msg->length - old_length));
	return NULL;
     }
   *size -= (msg->length - old_length);
   return s;
}

static void
_ecore_dbus_message_unmarshal_struct_end(Ecore_DBus_Message *msg,
					 Ecore_DBus_Message_Field_Struct *s __UNUSED__)
{
   ecore_list_first_remove(msg->recurse);
}

static void *
_ecore_dbus_message_unmarshal_variant(Ecore_DBus_Message *msg, int *size)
{
   Ecore_DBus_Message_Field_Variant *f = NULL;
   unsigned int                      old_length;
   unsigned char                     length;
   Ecore_DBus_Data_Type              type;

   old_length = msg->length;
   f = _ecore_dbus_message_field_new(msg, ECORE_DBUS_DATA_TYPE_VARIANT);
   ecore_list_prepend(msg->recurse, f);

   /* signature length */
   length = _ecore_dbus_message_read_byte(msg);
   if (length != 1)
     {
	/* TODO: Free message field */
	printf("Ecore_DBus: Wrong length for variant signature: %d\n", length);
	return NULL;
     }
   /* signature */
   type = _ecore_dbus_message_read_byte(msg);
   f->contained_type = type;
   /* Read '\0' from signature */
   _ecore_dbus_message_read_byte(msg);
   if (*size < (int)(msg->length - old_length))
     {
	/* TODO: Free message field */
	printf("Ecore_DBus: To few bytes for variant header: got %d need %d\n",
	       *size, (msg->length - old_length));
	return NULL;
     }
   *size -= (msg->length - old_length);

   switch (type)
     {
     case ECORE_DBUS_DATA_TYPE_UINT32:
	f->value = _ecore_dbus_message_unmarshal_uint32(msg, size);
	break;
     case ECORE_DBUS_DATA_TYPE_STRING:
	f->value = _ecore_dbus_message_unmarshal_string(msg, size);
	break;
     case ECORE_DBUS_DATA_TYPE_OBJECT_PATH:
	f->value = _ecore_dbus_message_unmarshal_object_path(msg, size);
	break;
     case ECORE_DBUS_DATA_TYPE_SIGNATURE:
	f->value = _ecore_dbus_message_unmarshal_signature(msg, size);
	break;
     case ECORE_DBUS_DATA_TYPE_INVALID:
     case ECORE_DBUS_DATA_TYPE_BYTE:
     case ECORE_DBUS_DATA_TYPE_BOOLEAN:
     case ECORE_DBUS_DATA_TYPE_INT16:
     case ECORE_DBUS_DATA_TYPE_UINT16:
     case ECORE_DBUS_DATA_TYPE_INT32:
     case ECORE_DBUS_DATA_TYPE_INT64:
     case ECORE_DBUS_DATA_TYPE_UINT64:
     case ECORE_DBUS_DATA_TYPE_DOUBLE:
     case ECORE_DBUS_DATA_TYPE_ARRAY:
     case ECORE_DBUS_DATA_TYPE_VARIANT:
     case ECORE_DBUS_DATA_TYPE_STRUCT:
     case ECORE_DBUS_DATA_TYPE_STRUCT_BEGIN:
     case ECORE_DBUS_DATA_TYPE_STRUCT_END:
     case ECORE_DBUS_DATA_TYPE_DICT_ENTRY:
     case ECORE_DBUS_DATA_TYPE_DICT_ENTRY_BEGIN:
     case ECORE_DBUS_DATA_TYPE_DICT_ENTRY_END:
#if 0
     default:
#endif
	printf("[ecore_dbus] unknown/unhandled data type %c\n", type);
	break;
     }
   ecore_list_first_remove(msg->recurse);
   return f;
}

static Ecore_DBus_Unmarshal_Func
_ecore_dbus_message_unmarshal_func(Ecore_DBus_Data_Type type)
{
   switch (type)
     {
     case ECORE_DBUS_DATA_TYPE_BYTE:
	return _ecore_dbus_message_unmarshal_byte;
     case ECORE_DBUS_DATA_TYPE_INT32:
	return _ecore_dbus_message_unmarshal_int32;
     case ECORE_DBUS_DATA_TYPE_UINT32:
	return _ecore_dbus_message_unmarshal_uint32;
     case ECORE_DBUS_DATA_TYPE_STRING:
	return _ecore_dbus_message_unmarshal_string;
     case ECORE_DBUS_DATA_TYPE_OBJECT_PATH:
	return _ecore_dbus_message_unmarshal_object_path;
     case ECORE_DBUS_DATA_TYPE_SIGNATURE:
	return _ecore_dbus_message_unmarshal_signature;
     case ECORE_DBUS_DATA_TYPE_INVALID:
     case ECORE_DBUS_DATA_TYPE_BOOLEAN:
     case ECORE_DBUS_DATA_TYPE_INT16:
     case ECORE_DBUS_DATA_TYPE_UINT16:
     case ECORE_DBUS_DATA_TYPE_INT64:
     case ECORE_DBUS_DATA_TYPE_UINT64:
     case ECORE_DBUS_DATA_TYPE_DOUBLE:
     case ECORE_DBUS_DATA_TYPE_ARRAY:
     case ECORE_DBUS_DATA_TYPE_VARIANT:
     case ECORE_DBUS_DATA_TYPE_STRUCT:
     case ECORE_DBUS_DATA_TYPE_STRUCT_BEGIN:
     case ECORE_DBUS_DATA_TYPE_STRUCT_END:
     case ECORE_DBUS_DATA_TYPE_DICT_ENTRY:
     case ECORE_DBUS_DATA_TYPE_DICT_ENTRY_BEGIN:
     case ECORE_DBUS_DATA_TYPE_DICT_ENTRY_END:
#if 0
     default:
#endif
	printf("[ecore_dbus] unknown/unhandled data type %c\n", type);
	break;
     }
   return NULL;
}

Ecore_DBus_Message *
_ecore_dbus_message_unmarshal(Ecore_DBus_Server *svr, unsigned char *message, int size)
{
   Ecore_DBus_Message             *msg;
   Ecore_DBus_Message_Field_Array *arr;
   char                           *sig;
   unsigned int                    old_length;

   /* init */
   if (size < 16)
     {
	printf("Ecore_DBus: To few bytes for minimal header: %d\n", size);
	return NULL;
     }
   printf("[ecore_dbus] unmarshaling\n");
   msg = _ecore_dbus_message_new(svr);

   /* message header */
   msg->byte_order = *(message + 0);
   msg->type = *(message + 1);
   msg->flags = *(message + 2);
   msg->protocol = *(message + 3);
   if (msg->protocol != ECORE_DBUS_MAJOR_PROTOCOL_VERSION)
     {
	printf("Ecore_DBus: Only supports protocol 0x%x, message has protocol 0x%x.\n",
	       ECORE_DBUS_MAJOR_PROTOCOL_VERSION, msg->protocol);
	goto error;
     }
   msg->body_length = *(unsigned int *)(message + 4);
   msg->serial = *(unsigned int *)(message + 8);
   if (msg->type == ECORE_DBUS_MESSAGE_TYPE_INVALID)
     {
	printf("Ecore_DBus: Invalid message type.\n");
	goto error;
     }
   /* copy message to buffer */
   _ecore_dbus_message_append_bytes(msg, message, size);
   msg->length = 12;
   size -= 12;
   /* Parse header fields */
   if (!(arr = _ecore_dbus_message_unmarshal_array_begin(msg, ECORE_DBUS_DATA_TYPE_STRUCT, &size)))
     {
	printf("Could not parse header fields.\n");
	goto error;
     }
   while (msg->length < arr->end)
     {
	Ecore_DBus_Message_Field_Struct *s;

	s = _ecore_dbus_message_unmarshal_struct_begin(msg, &size);
	_ecore_dbus_message_unmarshal_byte(msg, &size);
	_ecore_dbus_message_unmarshal_variant(msg, &size);
	_ecore_dbus_message_unmarshal_struct_end(msg, s);
     }
   _ecore_dbus_message_unmarshal_array_end(msg, arr);
   msg->header = ecore_list_first_remove(msg->fields);
   sig = ecore_dbus_message_header_field_get(msg, ECORE_DBUS_HEADER_FIELD_SIGNATURE);
   old_length = msg->length;
   _ecore_dbus_message_padding_skip(msg, 8);
   
   size -= (msg->length - old_length);
   /* message body */
   if (sig)
     {
	while (*sig)
	  {
	     Ecore_DBus_Data_Type  type;

	     type = *sig;
	     switch (type)
	       {
		case ECORE_DBUS_DATA_TYPE_BOOLEAN:
		   _ecore_dbus_message_unmarshal_boolean(msg, &size);
		   break;
		case ECORE_DBUS_DATA_TYPE_INT32:
		   _ecore_dbus_message_unmarshal_int32(msg, &size);
		   break;
		case ECORE_DBUS_DATA_TYPE_UINT32:
		   _ecore_dbus_message_unmarshal_uint32(msg, &size);
		   break;
		case ECORE_DBUS_DATA_TYPE_STRING:
		   _ecore_dbus_message_unmarshal_string(msg, &size);
		   break;
		case ECORE_DBUS_DATA_TYPE_OBJECT_PATH:
		   _ecore_dbus_message_unmarshal_object_path(msg, &size);
		   break;
		case ECORE_DBUS_DATA_TYPE_SIGNATURE:
		    _ecore_dbus_message_unmarshal_signature(msg, &size);
		   break;
		case ECORE_DBUS_DATA_TYPE_ARRAY:
		     {
			Ecore_DBus_Message_Field_Array *ar2;
			Ecore_DBus_Unmarshal_Func func;
			sig++;
			type = *sig;
			ar2 = _ecore_dbus_message_unmarshal_array_begin(msg, type, &size);
			func = _ecore_dbus_message_unmarshal_func(type);
			if (!func) break;
			while (msg->length < ar2->end)
			  (*func)(msg, &size);
			_ecore_dbus_message_unmarshal_array_end(msg, ar2);
		     }
		   break;
		case ECORE_DBUS_DATA_TYPE_INVALID:
		case ECORE_DBUS_DATA_TYPE_BYTE:
		case ECORE_DBUS_DATA_TYPE_INT16:
		case ECORE_DBUS_DATA_TYPE_UINT16:
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
		   printf("[ecore_dbus] unhandled data type %c\n", type);
		   break;
		default:
		   printf("[ecore_dbus] unknown data type %c\n", type);
		   break;
	       }
	     sig++;
	  }
     }
   return msg;
error:
   _ecore_dbus_message_free(msg);
   return NULL;
}
