#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "includes.h"
#include "error.h"
#include "list.h"
#include "hash.h"
#include "event.h"

#define EWL_OBJECT(a) ((EwlObject*)a)
#define EWL_HANDLER(a) ((EwlHandler)a)

typedef struct _EwlObject       EwlObject;
typedef void (*EwlHandler)(void *object, char *type, EwlHash *params);

struct _EwlObject	{
	int            ref_count;

	EwlHash       *event_callbacks;
	EwlHash       *handlers;
	EwlHash       *data;
};


/* OBJECT LIST FUNCTIONS */
EwlList   *ewl_get_object_list(); /* returns INTERNAL list */
void       ewl_add(void *object);


/* OBJECT NEW/FREE FUNCTIONS */
EwlObject *ewl_object_new();
void       ewl_object_init(EwlObject *object);
void       ewl_object_free(EwlObject *object);


/* OBJECT CALLBACK/EVENT FUNCTIONS */
void       ewl_callback_add(void        *object,
                            char        *type,
                            EwlCallback  callback,
                            void        *data);
void       ewl_callback_push(void        *object,
                             char        *type,
                             EwlCallback  callback,
                             void        *data);
void       ewl_object_handle_event(void *object, EwlEvent *event);


/* OBJECT HANDLER FUNCTIONS */
void       ewl_handler_set(void *object, char *type, EwlHandler handler);
EwlHandler ewl_handler_get(void *object, char *type);
void       ewl_handler_remove(void *object, char *type);


/* OBJECT GET/SET DATA FUNCTIONS */
void       ewl_set(void *object, char *key, void *data);
void      *ewl_get(void *object, char *key);
void       ewl_remove(void *object, char *key);

void       ewl_set_flag(void *object, char *key, char flag);
char       ewl_get_flag(void *object, char *key);

void       ewl_set_int(void *object, char *key, int val);
int        ewl_get_int(void *object, char *key);

void       ewl_set_double(void *object, char *key, double val);
double     ewl_get_double(void *object, char *key);

void       ewl_set_string(void *object, char *key, char *val);
char      *ewl_get_string(void *object, char *key);


/* OBJeCT_REFERENCE FUNCTIONS */
void       ewl_ref(void *object);
void       ewl_unref(void *object);


/*******************/
/* OBJECT HANDLERS */
/*******************/
void       ewl_callback_add_handler(void        *object,
                                    char        *handler_type,
                                    EwlHash     *params);
void       ewl_callback_push_handler(void        *object,
                                     char        *handler_type,
                                     EwlHash     *params);
/*void       ewl_callback_remove_handler(void        *object,
                                       char        *handler_type,
                                       EwlHash     *params);*/
/* FIXME -- add these later 
EwlHandler ewl_object_set_data_handler;
EWlHandler ewl_object_get_data_handler;
*/

/*********************/
/* PRIVATE FUNCTIONS */
/*********************/
void       ewl_object_list_add(EwlObject *object);
void       ewl_object_list_remove(EwlObject *object);

void       ewl_object_set_data(EwlObject *object, char *key, void *data);
void      *ewl_object_get_data(EwlObject *object, char *key);
void       ewl_object_remove_data(EwlObject *object, char *key);

void       ewl_object_ref(EwlObject *object);
void       ewl_object_unref(EwlObject *object);


#endif /* _OBJECT_H_ */
