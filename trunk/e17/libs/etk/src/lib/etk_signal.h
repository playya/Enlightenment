/** @file etk_signal.h */
#ifndef _ETK_SIGNAL_H_
#define _ETK_SIGNAL_H_

#include <stdarg.h>
#include "etk_marshallers.h"
#include "etk_types.h"

/**
 * @defgroup Etk_Signal Etk_Signal
 * @{
 */

/** @brief Gets the offset of a member of a struct (mainly used for etk_signal_new) */
#define ETK_MEMBER_OFFSET(type, member)   ((long)(&((type *)0)->member))

/**
 * @struct Etk_Signal
 * @brief The signal system is a flexible system to create, connect and emit callbacks
 */
struct _Etk_Signal
{
   /* private: */
   char *name;
   Etk_Type *object_type;
   int default_handler_offset;
   Etk_Marshaller marshaller;
   Etk_Accumulator accumulator;
   void *accum_data;
};

Etk_Bool etk_signal_init();
void etk_signal_shutdown();

Etk_Signal *etk_signal_new(const char *signal_name, Etk_Type *object_type, int default_handler_offset, Etk_Marshaller marshaller, Etk_Accumulator accumulator, void *accum_adata);
void etk_signal_delete(Etk_Signal *signal);

Etk_Signal *etk_signal_lookup(const char *signal_name, Etk_Type *type);
const char *etk_signal_name_get(Etk_Signal *signal);
Etk_Marshaller etk_signal_marshaller_get(Etk_Signal *signal);

void etk_signal_connect(const char *signal_name, Etk_Object *object, Etk_Signal_Callback_Function callback, void *data);
void etk_signal_connect_after(const char *signal_name, Etk_Object *object, Etk_Signal_Callback_Function callback, void *data);
void etk_signal_connect_swapped(const char *signal_name, Etk_Object *object, Etk_Signal_Callback_Function callback, void *data);
void etk_signal_connect_full(Etk_Signal *signal, Etk_Object *object, Etk_Signal_Callback_Function callback, void *data, Etk_Bool swapped, Etk_Bool after);

void etk_signal_emit(Etk_Signal *signal, Etk_Object *object, void *return_value, ...);
void etk_signal_emit_by_name(const char *signal_name, Etk_Object *object, void *return_value, ...);
void etk_signal_emit_valist(Etk_Signal *signal, Etk_Object *object, void *return_value, va_list args);

void etk_signal_stop();

/** @} */

#endif
