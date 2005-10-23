/** @file etk_marshallers.c */
#include "etk_marshallers.h"

/* etk_marshaller_VOID__VOID */
void etk_marshaller_VOID__VOID(Etk_Signal_Callback_Function callback, Etk_Object *object, void *data, void *return_value, va_list arguments)
{
   typedef void (*Etk_Signal_Callback_Function_VOID__VOID)(Etk_Object *object, void *data);

   Etk_Signal_Callback_Function_VOID__VOID callback_VOID__VOID;

   if (!callback || !object)
      return;

   callback_VOID__VOID = (Etk_Signal_Callback_Function_VOID__VOID)callback;
   callback_VOID__VOID(object, data);
}

/* etk_marshaller_VOID__DOUBLE */
void etk_marshaller_VOID__DOUBLE(Etk_Signal_Callback_Function callback, Etk_Object *object, void *data, void *return_value, va_list arguments)
{
   typedef void (*Etk_Signal_Callback_Function_VOID__DOUBLE)(Etk_Object *object, double arg1, void *data);

   double arg1;

   Etk_Signal_Callback_Function_VOID__DOUBLE callback_VOID__DOUBLE;

   if (!callback || !object)
      return;

   arg1 = va_arg(arguments, double);

   callback_VOID__DOUBLE = (Etk_Signal_Callback_Function_VOID__DOUBLE)callback;
   callback_VOID__DOUBLE(object, arg1, data);
}

/* etk_marshaller_VOID__POINTER */
void etk_marshaller_VOID__POINTER(Etk_Signal_Callback_Function callback, Etk_Object *object, void *data, void *return_value, va_list arguments)
{
   typedef void (*Etk_Signal_Callback_Function_VOID__POINTER)(Etk_Object *object, void *arg1, void *data);

   void *arg1;

   Etk_Signal_Callback_Function_VOID__POINTER callback_VOID__POINTER;

   if (!callback || !object)
      return;

   arg1 = va_arg(arguments, void *);

   callback_VOID__POINTER = (Etk_Signal_Callback_Function_VOID__POINTER)callback;
   callback_VOID__POINTER(object, arg1, data);
}

/* etk_marshaller_BOOL__VOID */
void etk_marshaller_BOOL__VOID(Etk_Signal_Callback_Function callback, Etk_Object *object, void *data, void *return_value, va_list arguments)
{
   typedef Etk_Bool (*Etk_Signal_Callback_Function_BOOL__VOID)(Etk_Object *object, void *data);

   Etk_Bool result;

   Etk_Signal_Callback_Function_BOOL__VOID callback_BOOL__VOID;

   if (!callback || !object)
      return;

   callback_BOOL__VOID = (Etk_Signal_Callback_Function_BOOL__VOID)callback;
   result = callback_BOOL__VOID(object, data);

   if (return_value)
      *((Etk_Bool *)return_value) = result;
}

/* etk_marshaller_BOOL__DOUBLE */
void etk_marshaller_BOOL__DOUBLE(Etk_Signal_Callback_Function callback, Etk_Object *object, void *data, void *return_value, va_list arguments)
{
   typedef Etk_Bool (*Etk_Signal_Callback_Function_BOOL__DOUBLE)(Etk_Object *object, double arg1, void *data);

   double arg1;
   Etk_Bool result;

   Etk_Signal_Callback_Function_BOOL__DOUBLE callback_BOOL__DOUBLE;

   if (!callback || !object)
      return;

   arg1 = va_arg(arguments, double);

   callback_BOOL__DOUBLE = (Etk_Signal_Callback_Function_BOOL__DOUBLE)callback;
   result = callback_BOOL__DOUBLE(object, arg1, data);

   if (return_value)
      *((Etk_Bool *)return_value) = result;
}

/* etk_marshaller_BOOL__POINTER_POINTER */
void etk_marshaller_BOOL__POINTER_POINTER(Etk_Signal_Callback_Function callback, Etk_Object *object, void *data, void *return_value, va_list arguments)
{
   typedef Etk_Bool (*Etk_Signal_Callback_Function_BOOL__POINTER_POINTER)(Etk_Object *object, void *arg1, void *arg2, void *data);

   void *arg1;
   void *arg2;
   Etk_Bool result;

   Etk_Signal_Callback_Function_BOOL__POINTER_POINTER callback_BOOL__POINTER_POINTER;

   if (!callback || !object)
      return;

   arg1 = va_arg(arguments, void *);
   arg2 = va_arg(arguments, void *);

   callback_BOOL__POINTER_POINTER = (Etk_Signal_Callback_Function_BOOL__POINTER_POINTER)callback;
   result = callback_BOOL__POINTER_POINTER(object, arg1, arg2, data);

   if (return_value)
      *((Etk_Bool *)return_value) = result;
}
