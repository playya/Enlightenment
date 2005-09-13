/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
/* NOTE:
 * 
 * This is a very SPECIAL file. This servers as a kind of "auto code generator"
 * to handle the encoder, decoder etc. aspects of ipc requests. the aim here
 * is to make writing support for a new opcode simple and compact. It intends
 * to centralize the handling into 1 spot, instead of having ti spread around
 * the code in N different places, as well as providing common construct macros
 * to help make the code more compact and easier to create.
 */

/* This is a bunch of convenience stuff for this to work */
#ifndef E_IPC_HANDLERS_H
# define E_IPC_HANDLERS_H

/* 
 * add a new ooption for enlightenment_remote
 * OP(opt, num_params, description, num_expected_replies, HDL) 
 */
# define OP(__a, __b, __c, __d, __e) \
     {__a, __b, __c, __d, __e},


# define STRING(__str, HDL) \
case HDL: \
if (e->data) { \
   char *__str = NULL; \
   if (e_ipc_codec_str_dec(e->data, e->size, &__str)) {
# define END_STRING(__str) \
      free(__str); \
   } \
} \
break;

/**
 * STRING2:
 * decode event data of type E_Ipc_2Str
 */
# define STRING2(__str1, __str2, __2str, HDL) \
case HDL: \
if (e->data) { \
   char *__str1 = NULL, *__str2 = NULL; \
   E_Ipc_2Str *__2str = NULL; \
   __2str = calloc(1, sizeof(E_Ipc_2Str)); \
   if (e_ipc_codec_2str_dec(e->data, e->size, &(__2str))) { \
      __str1 = __2str->str1; \
      __str2 = __2str->str2;
# define END_STRING2(__2str) \
      free(__2str->str1); \
      free(__2str->str2); \
      free(__2str); \
   } \
} \
break;

/**
 * INT3_STRING3:
 * Decode event data of type E_Ipc_3Int_3Str
 */
# define INT3_STRING3(__3int_3str, HDL) \
case HDL: \
if (e->data) { \
   E_Ipc_3Int_3Str *__3int_3str = NULL; \
   __3int_3str = calloc(1, sizeof(E_Ipc_3Int_3Str)); \
   if (e_ipc_codec_3int_3str_dec(e->data, e->size, &(__3int_3str))) {
# define END_INT3_STRING3(__3int_3str) \
      free(__3int_3str->str1); \
      free(__3int_3str->str2); \
      free(__3int_3str->str3); \
      free(__3int_3str); \
   } \
} \
break;

/**
 * INT4_STRING2:
 * Decode event data of type E_Ipc_4Int_2Str
 */
# define INT4_STRING2(__4int_2str, HDL) \
case HDL: \
if (e->data) { \
   E_Ipc_4Int_2Str *__4int_2str = NULL; \
   __4int_2str = calloc(1, sizeof(E_Ipc_4Int_2Str)); \
   if (e_ipc_codec_4int_2str_dec(e->data, e->size, &(__4int_2str))) {
# define END_INT4_STRING2(__4int_2str) \
      free(__4int_2str->str1); \
      free(__4int_2str->str2); \
      free(__4int_2str); \
   } \
} \
break;

# define STRING2_INT(__str1, __str2, __int, __2str_int, HDL) \
case HDL: \
if (e->data) { \
   char *__str1 = NULL, *__str2 = NULL; \
   int __int; \
   E_Ipc_2Str_Int *__2str_int = NULL; \
   __2str_int = calloc(1, sizeof(E_Ipc_2Str_Int)); \
   if (e_ipc_codec_2str_int_dec(e->data, e->size, &(__2str_int))) { \
      __str1 = __2str_int->str1; \
      __str2 = __2str_int->str2; \
      __int  = __2str_int->val;
# define END_STRING2_INT(__2str_int) \
      free(__2str_int->str1); \
      free(__2str_int->str2); \
      free(__2str_int); \
   } \
} \
break;

# define START_DOUBLE(__dbl, HDL) \
case HDL: \
if (e->data) { \
   double __dbl = 0.0; \
   if (e_ipc_codec_double_dec(e->data, e->size, &(__dbl))) {
# define END_DOUBLE \
   } \
} \
break;

# define START_INT(__int, HDL) \
case HDL: \
if (e->data) { \
   int __int = 0; \
   if (e_ipc_codec_int_dec(e->data, e->size, &(__int))) {
# define END_INT \
   } \
} \
break;

# define START_2INT(__int1, __int2, HDL) \
case HDL: \
if (e->data) { \
   int __int1 = 0; \
   int __int2 = 0; \
   if (e_ipc_codec_2int_dec(e->data, e->size, &(__int1), &(__int2))) {
# define END_2INT \
   } \
} \
break;

/**
 * Get event data for libe processing
 */
# define RESPONSE(__res, __store) \
   __store *__res = calloc(1, sizeof(__store)); \
   if (e->data) {
#define END_RESPONSE(__res, __type) \
   } \
   ecore_event_add(__type, __res, NULL, NULL);

# define SAVE e_config_save_queue()

# define REQ_STRING(__str, HDL) \
case HDL: { void *data; int bytes; \
   data = e_ipc_codec_str_enc(__str, &bytes); \
   if (data) { \
      ecore_ipc_server_send(e->server, E_IPC_DOMAIN_REQUEST, HDL, 0, 0, 0, data, bytes); \
      free(data); \
   } \
} \
break;

# define REQ_2STRING(__str1, __str2, HDL) \
case HDL: { void *data; int bytes; \
   data = e_ipc_codec_2str_enc(__str1, __str2, &bytes); \
   if (data) { \
      ecore_ipc_server_send(e->server, E_IPC_DOMAIN_REQUEST, HDL, 0, 0, 0, data, bytes); \
      free(data); \
   } \
} \
break;

#define REQ_3INT_3STRING_START(HDL) \
case HDL: { void *data; int bytes; \

#define REQ_3INT_3STRING_END(__val1, __val2, __val3, __str1, __str2, __str3, HDL) \
   data = e_ipc_codec_3int_3str_enc(__val1, __val2, __val3, __str1, __str2, __str3, &bytes); \
   if (data) { \
      ecore_ipc_server_send(e->server, E_IPC_DOMAIN_REQUEST, HDL, 0, 0, 0, data, bytes); \
      free(data); \
   } \
} \
break;


#define REQ_4INT_2STRING_START(HDL) \
case HDL: { void *data; int bytes; \

#define REQ_4INT_2STRING_END(__val1, __val2, __val3, __val4, __str1, __str2, HDL) \
   data = e_ipc_codec_4int_2str_enc(__val1, __val2, __val3, __val4, __str1, __str2, &bytes); \
   if (data) { \
      ecore_ipc_server_send(e->server, E_IPC_DOMAIN_REQUEST, HDL, 0, 0, 0, data, bytes); \
      free(data); \
   } \
} \
break;

# define REQ_2STRING_INT(__str1, __str2, __int, HDL) \
case HDL: { void *data; int bytes; \
   data = e_ipc_codec_2str_int_enc(__str1, __str2, __int, &bytes); \
   if (data) { \
      ecore_ipc_server_send(e->server, E_IPC_DOMAIN_REQUEST, HDL, 0, 0, 0, data, bytes); \
      free(data); \
   } \
} \
break;

# define REQ_DOUBLE(__dbl, HDL) \
case HDL: { void *data; int bytes; \
   data = e_ipc_codec_double_enc(__dbl, &bytes); \
   if (data) { \
      ecore_ipc_server_send(e->server, E_IPC_DOMAIN_REQUEST, HDL, 0, 0, 0, data, bytes); \
      free(data); \
   } \
} \
break;

# define REQ_INT_START(HDL) \
case HDL: { void *data; int bytes;

# define REQ_INT_END(__int, HDL) \
   data = e_ipc_codec_int_enc(__int, &bytes); \
   if (data) { \
      ecore_ipc_server_send(e->server, E_IPC_DOMAIN_REQUEST, HDL, 0, 0, 0, data, bytes); \
      free(data); \
   } \
} \
break;

# define REQ_INT(__int, HDL) \
   REQ_INT_START(HDL) \
   REQ_INT_END(__int, HDL)

# define REQ_2INT(__int1, __int2, HDL) \
case HDL: { void *data; int bytes; \
   data = e_ipc_codec_2int_enc(__int1, __int2, &bytes); \
   if (data) { \
      ecore_ipc_server_send(e->server, E_IPC_DOMAIN_REQUEST, HDL, 0, 0, 0, data, bytes); \
      free(data); \
   } \
} \
break;

# define REQ_NULL(HDL) \
case HDL: \
   ecore_ipc_server_send(e->server, E_IPC_DOMAIN_REQUEST, HDL, 0, 0, 0, NULL, 0); \
break;

# define FREE_LIST(__list) \
while (__list) { \
   free(__list->data); \
   __list = evas_list_remove_list(__list, __list); \
}

# define SEND_DATA(__opcode) \
ecore_ipc_client_send(e->client, E_IPC_DOMAIN_REPLY, __opcode, 0, 0, 0, data, bytes); \
free(data);

# define STRING_INT_LIST(__v, HDL) \
 case HDL: { \
    Evas_List *dat = NULL, *l; \
    if (e_ipc_codec_str_int_list_dec(e->data, e->size, &dat)) { \
       for (l = dat; l; l = l->next) { \
	  E_Ipc_Str_Int *__v; \
	  __v = l->data;
#define END_STRING_INT_LIST(__v) \
	  free(__v->str); \
	  free(__v); \
       } \
       evas_list_free(dat); \
    } \
 } \
   break;

#define SEND_STRING_INT_LIST(__list, __typ1, __v1, __v2, HDL) \
 case HDL: { \
    Evas_List *dat = NULL, *l; \
    void *data; int bytes; \
    for (l = __list; l; l = l->next) { \
       __typ1 *__v1; \
       E_Ipc_Str_Int *__v2; \
       __v1 = l->data; \
       __v2 = calloc(1, sizeof(E_Ipc_Str_Int));
#define END_SEND_STRING_INT_LIST(__v1, __op) \
       dat = evas_list_append(dat, __v1); \
    } \
    data = e_ipc_codec_str_int_list_enc(dat, &bytes); \
    SEND_DATA(__op); \
    FREE_LIST(dat); \
 } \
   break;

/**
 * INT3_STRING3:
 * Decode event data is a list of E_Ipc_3Int_3Str objects and iterate
 * the list. For each iteration the object __v will contain a decoded list
 * element.
 *
 * Use END_INT3_STRING3_LIST to terminate the loop and free all data. 
 */
#define INT3_STRING3_LIST(__v, HDL) \
   INT3_STRING3_LIST_START(__v, HDL) \
   INT3_STRING3_LIST_ITERATE(__v)

#define INT3_STRING3_LIST_START(__v, HDL) \
 case HDL: { \
    Evas_List *dat = NULL, *l; \
    if (e_ipc_codec_3int_3str_list_dec(e->data, e->size, &dat)) {
#define INT3_STRING3_LIST_ITERATE(__v) \
       for (l = dat; l; l = l->next) { \
	  E_Ipc_3Int_3Str *__v; \
	  __v = l->data;
#define END_INT3_STRING3_LIST(__v) \
   END_INT3_STRING3_LIST_ITERATE(__v) \
   END_INT3_STRING3_LIST_START()

#define END_INT3_STRING3_LIST_ITERATE(__v) \
          free(__v->str1); \
          free(__v->str2); \
          free(__v->str3); \
          free(__v); \
       } 
#define END_INT3_STRING3_LIST_START() \
       evas_list_free(dat); \
    } \
 } \
  break;

/** 
 * SEND_INT3_STRING3_LIST:
 * Start to encode a list of objects to prepare them for sending via
 * ipc. The object __v1 will be of type __typ1 and __v2 will be of type
 * E_Ipc_3Int_3Str. 
 *
 * Use END_SEND_INT3_STRING3_LIST to terminate the encode iteration and 
 * send that data. The list will be freed.
 */
#define SEND_INT3_STRING3_LIST(__list, __typ1, __v1, __v2, HDL) \
 case HDL: { \
    Evas_List *dat = NULL, *l; \
    void *data; int bytes; \
    for (l = __list; l; l = l->next) { \
       __typ1 *__v1; \
       E_Ipc_3Int_3Str *__v2; \
       __v1 = l->data; \
       __v2 = calloc(1, sizeof(E_Ipc_3Int_3Str));
#define END_SEND_INT3_STRING3_LIST(__v1, __op) \
       dat = evas_list_append(dat, __v1); \
    } \
    data = e_ipc_codec_3int_3str_list_enc(dat, &bytes); \
    SEND_DATA(__op); \
    FREE_LIST(dat); \
 } \
   break;

/**
 * INT4_STRING2:
 * Decode event data is a list of E_Ipc_4Int_2Str objects and iterate
 * the list. For each iteration the object __v will contain a decoded list
 * element.
 *
 * Use END_INT4_STRING2_LIST to terminate the loop and free all data. 
 */
#define INT4_STRING2_LIST(__v, HDL) \
   INT4_STRING2_LIST_START(__v, HDL) \
   INT4_STRING2_LIST_ITERATE(__v)

#define INT4_STRING2_LIST_START(__v, HDL) \
 case HDL: { \
    Evas_List *dat = NULL, *l; \
    if (e_ipc_codec_4int_2str_list_dec(e->data, e->size, &dat)) { 
#define INT4_STRING2_LIST_ITERATE(__v) \
       for (l = dat; l; l = l->next) { \
	  E_Ipc_4Int_2Str *__v; \
	  __v = l->data;
#define END_INT4_STRING2_LIST(__v) \
   END_INT4_STRING2_LIST_ITERATE(__v) \
   END_INT4_STRING2_LIST_START()

#define END_INT4_STRING2_LIST_ITERATE(__v) \
          free(__v->str1); \
          free(__v->str2); \
          free(__v); \
       } \
       evas_list_free(dat);
#define END_INT4_STRING2_LIST_START() \
    } \
 } \
  break;

/** 
 * SEND_INT4_STRING2_LIST:
 * Start to encode a list of objects to prepare them for sending via
 * ipc. The object __v1 will be of type __typ1 and __v2 will be of type
 * E_Ipc_4Int_2Str. 
 *
 * Use END_SEND_INT4_STRING2_LIST to terminate the encode iteration and 
 * send that data. The list will be freed.
 */
#define SEND_INT4_STRING2_LIST(__list, __typ1, __v1, __v2, HDL) \
 case HDL: { \
    Evas_List *dat = NULL, *l; \
    void *data; int bytes; \
    for (l = __list; l; l = l->next) { \
       __typ1 *__v1; \
       E_Ipc_4Int_2Str *__v2; \
       __v1 = l->data; \
       __v2 = calloc(1, sizeof(E_Ipc_4Int_2Str));
#define END_SEND_INT4_STRING2_LIST(__v1, __op) \
       dat = evas_list_append(dat, __v1); \
    } \
    data = e_ipc_codec_4int_2str_list_enc(dat, &bytes); \
    SEND_DATA(__op); \
    FREE_LIST(dat); \
 } \
   break;

/**
 * STRING2_INT_LIST:
 * Decode event data which is a list of E_Ipc_2Str_Int objects and iterate 
 * the list. For each iteration the object __v will contain a decoded list
 * element. 
 *
 * Use END_STRING2_INT_LIST to terminate the loop and free all data.
 */
#define STRING2_INT_LIST(__v, HDL) \
 case HDL: { \
    Evas_List *dat = NULL, *l; \
    if (e_ipc_codec_2str_int_list_dec(e->data, e->size, &dat)) { \
       for (l = dat; l; l = l->next) { \
	  E_Ipc_2Str_Int *__v; \
	  __v = l->data;
#define END_STRING2_INT_LIST(__v) \
	  free(__v->str1); \
	  free(__v->str2); \
	  free(__v); \
       } \
       evas_list_free(dat); \
    } \
 } \
   break;

/** 
 * SEND_STRING2_INT_LIST:
 * Start to encode a list of objects to prepare them for sending via
 * ipc. The object __v1 will be of type __typ1 and __v2 will be of type
 * E_Ipc_2Str_Int. 
 *
 * Use END_SEND_STRING2_INT_LIST to terminate the encode iteration and 
 * send that data. The list will be freed.
 */
#define SEND_STRING2_INT_LIST(__list, __typ1, __v1, __v2, HDL) \
 case HDL: { \
    Evas_List *dat = NULL, *l; \
    void *data; int bytes; \
    for (l = __list; l; l = l->next) { \
       __typ1 *__v1; \
       E_Ipc_2Str_Int *__v2; \
       __v1 = l->data; \
       __v2 = calloc(1, sizeof(E_Ipc_2Str_Int));
#define END_SEND_STRING2_INT_LIST(__v1, __op) \
       dat = evas_list_append(dat, __v1); \
    } \
    data = e_ipc_codec_2str_int_list_enc(dat, &bytes); \
    SEND_DATA(__op); \
    FREE_LIST(dat); \
 } \
   break;

/**
 * STRING2_LIST:
 * Decode event data which is a list of E_Ipc_2Str objects and iterate 
 * the list. For each iteration the object __v will contain a decoded list
 * element. 
 *
 * Use END_STRING2_LIST to terminate the loop and free all data.
 */
#define STRING2_LIST(__v, HDL) \
 case HDL: { \
    Evas_List *dat = NULL, *l; \
    if (e_ipc_codec_2str_list_dec(e->data, e->size, &dat)) { \
       for (l = dat; l; l = l->next) { \
	  E_Ipc_2Str *__v; \
	  __v = l->data;
#define END_STRING2_LIST(__v) \
	  free(__v->str1); \
	  free(__v->str2); \
	  free(__v); \
       } \
       evas_list_free(dat); \
    } \
 } \
   break;

/** 
 * SEND_STRING2_LIST:
 * Start to encode a list of objects to prepare them for sending via
 * ipc. The object __v1 will be of type __typ1 and __v2 will be of type
 * E_Ipc_2Str. 
 *
 * Use END_SEND_STRING2_LIST to terminate the encode iteration and 
 * send that data. The list will be freed.
 */
#define SEND_STRING2_LIST(__list, __typ1, __v1, __v2, HDL) \
 case HDL: { \
    Evas_List *dat = NULL, *l; \
    void *data; int bytes; \
    for (l = __list; l; l = l->next) { \
       __typ1 *__v1; \
       E_Ipc_2Str *__v2; \
       __v1 = l->data; \
       __v2 = calloc(1, sizeof(E_Ipc_2Str));
#define END_SEND_STRING2_LIST(__v1, __op) \
       dat = evas_list_append(dat, __v1); \
    } \
    data = e_ipc_codec_2str_list_enc(dat, &bytes); \
    SEND_DATA(__op); \
    FREE_LIST(dat); \
 } \
   break;


#define SEND_STRING(__str, __op, HDL) \
case HDL: { void *data; int bytes; \
   data = e_ipc_codec_str_enc(__str, &bytes); \
   if (data) { \
      ecore_ipc_client_send(e->client, E_IPC_DOMAIN_REPLY, __op, 0, 0, 0, data, bytes); \
      free(data); \
   } \
} \
break;

#define SEND_DOUBLE(__dbl, __op, HDL) \
case HDL: { void *data; int bytes; \
   data = e_ipc_codec_double_enc(__dbl, &bytes); \
   if (data) { \
      ecore_ipc_client_send(e->client, E_IPC_DOMAIN_REPLY, __op, 0, 0, 0, data, bytes); \
      free(data); \
   } \
} \
break;

# define SEND_INT(__int, __op, HDL) \
case HDL: { void *data; int bytes; \
   data = e_ipc_codec_int_enc(__int, &bytes); \
   if (data) { \
      ecore_ipc_client_send(e->client, E_IPC_DOMAIN_REPLY, __op, 0, 0, 0, data, bytes); \
      free(data); \
   } \
} \
break;

# define SEND_2INT(__int1, __int2,__op, HDL) \
case HDL: { void *data; int bytes; \
   data = e_ipc_codec_2int_enc(__int1, __int2, &bytes); \
   if (data) { \
      ecore_ipc_client_send(e->client, E_IPC_DOMAIN_REPLY, __op, 0, 0, 0, data, bytes); \
      free(data); \
   } \
} \
break;

#define LIST_DATA() \
   Evas_List *dat = NULL, *l; \
   void *data; int bytes;

#define ENCODE(__dat, __enc) \
   data = __enc(__dat, &bytes);

#define FOR(__start) \
   for (l = __start; l; l = l->next)
#define GENERIC(HDL) \
 case HDL: {

#define END_GENERIC() \
   } \
break;

#define LIST() \
   Evas_List *dat = NULL, *l;

#define DECODE(__dec) \
   if (__dec(e->data, e->size, &dat))

# define E_PATH_GET(__path, __str) \
   E_Path *__path = NULL; \
   if (!strcmp(__str, "data")) \
     __path = path_data; \
   else if (!strcmp(__str, "images")) \
     __path = path_images; \
   else if (!strcmp(__str, "fonts")) \
     __path = path_fonts; \
   else if (!strcmp(__str, "themes")) \
     __path = path_themes; \
   else if (!strcmp(__str, "init")) \
     __path = path_init; \
   else if (!strcmp(__str, "icons")) \
     __path = path_icons; \
   else if (!strcmp(__str, "modules")) \
     __path = path_modules; \
   else if (!strcmp(__str, "backgrounds")) \
     __path = path_backgrounds; 


#endif










/*
 * ****************
 * IPC handlers
 * ****************
 */

/* what a handler looks like
 * 
 * E_REMOTE_OPTIONS
 *   OP(opt, num_params, description, num_expected_replies, HDL)
 * E_REMOTE_OUT
 *   ...
 * E_WM_IN
 *   ...
 * E_REMOTE_IN
 *   ...
 * E_LIB_IN
 *   ...
 */
   
/****************************************************************************/
#define HDL E_IPC_OP_MODULE_LOAD
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-module-load", 1, "Loads the module named 'OPT1' into memory", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_STRING(params[0], HDL);
#elif (TYPE == E_WM_IN)
   STRING(s, HDL);
   if (!e_module_find(s)) {
      e_module_new(s);
      SAVE;
   }
   END_STRING(s);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL
     
/****************************************************************************/
#define HDL E_IPC_OP_MODULE_UNLOAD
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-module-unload", 1, "Unloads the module named 'OPT1' from memory", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_STRING(params[0], HDL);
#elif (TYPE == E_WM_IN)
   STRING(s, HDL);
   E_Module *m;
   if ((m = e_module_find(s))) {
      e_module_disable(m);
      e_object_del(E_OBJECT(m));
      SAVE;
   }
   END_STRING(s);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL
   
/****************************************************************************/
#define HDL E_IPC_OP_MODULE_ENABLE
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-module-enable", 1, "Enable the module named 'OPT1'", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_STRING(params[0], HDL);
#elif (TYPE == E_WM_IN)
   STRING(s, HDL);
   E_Module *m;
   if ((m = e_module_find(s))) {
      e_module_enable(m);
      SAVE;
   }
   END_STRING(s);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL
     
/****************************************************************************/
#define HDL E_IPC_OP_MODULE_DISABLE
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-module-disable", 1, "Disable the module named 'OPT1'", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_STRING(params[0], HDL);
#elif (TYPE == E_WM_IN)
   STRING(s, HDL);
   E_Module *m;
   if ((m = e_module_find(s))) {
      e_module_disable(m);
      SAVE;
   }
   END_STRING(s);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL
     
/****************************************************************************/
#define HDL E_IPC_OP_MODULE_LIST
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-module-list", 0, "List all loaded modules", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_STRING_INT_LIST(e_module_list(), E_Module, mod, v, HDL);
   v->str = mod->name;
   v->val = mod->enabled;
   END_SEND_STRING_INT_LIST(v, E_IPC_OP_MODULE_LIST_REPLY);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL
     
/****************************************************************************/
#define HDL E_IPC_OP_MODULE_LIST_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   STRING_INT_LIST(v, HDL);
   printf("REPLY: \"%s\" ENABLED %i\n", v->str, v->val);
   END_STRING_INT_LIST(v);
#elif (TYPE == E_LIB_IN)
   GENERIC(HDL);
   Evas_List *dat = NULL;
   DECODE(e_ipc_codec_str_int_list_dec) {
      LIST();
      int count;
      RESPONSE(r, E_Response_Module_List);

      /* FIXME - this is a mess, needs to be merged into macros... */
      count = evas_list_count(dat);
      r->modules = malloc(sizeof(E_Response_Module_Data *) * count);
      r->count = count;

      count = 0;
      FOR(dat) {
	 E_Response_Module_Data *md;
	 E_Ipc_Str_Int *v;
	 
	 v = l->data;
	 md = malloc(sizeof(E_Response_Module_Data));
	 
	 md->name = v->str;
	 md->enabled = v->val;
	 r->modules[count] = md;
	 count++;
      }
      END_RESPONSE(r, E_RESPONSE_MODULE_LIST); /* FIXME - need a custom free */
   }
   END_GENERIC();
#endif
#undef HDL
     
/****************************************************************************/
#define HDL E_IPC_OP_BG_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-default-bg-set", 1, "Set the default background edje to the desktop background in the file 'OPT1' (must be a full path)", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_STRING(params[0], HDL);
#elif (TYPE == E_WM_IN)
   STRING(s, HDL);
   E_FREE(e_config->desktop_default_background);
   e_config->desktop_default_background = strdup(s);
   e_bg_update();
   SAVE;
   END_STRING(s);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL
     
/****************************************************************************/
#define HDL E_IPC_OP_BG_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-default-bg-get", 0, "Get the default background edje file path", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_STRING(e_config->desktop_default_background, E_IPC_OP_BG_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL
     
/****************************************************************************/
#define HDL E_IPC_OP_BG_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   STRING(s, HDL);
   printf("REPLY: \"%s\"\n", s);
   END_STRING(s);
#elif (TYPE == E_LIB_IN)
   STRING(s, HDL);
   RESPONSE(r, E_Response_Background_Get);
   r->file = strdup(s);
   END_RESPONSE(r, E_RESPONSE_BACKGROUND_GET);
   END_STRING(s);
#endif
#undef HDL
     
/****************************************************************************/
#define HDL E_IPC_OP_FONT_AVAILABLE_LIST
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-font-available-list", 0, "List all available fonts", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   GENERIC(HDL);
   LIST_DATA();
   E_Font_Available *fa;
   Evas_List *fa_list;
   fa_list = e_font_available_list();
   FOR(fa_list) { fa = l->data;
      dat = evas_list_append(dat, fa->name);
   }
   ENCODE(dat, e_ipc_codec_str_list_enc);
   SEND_DATA(E_IPC_OP_FONT_AVAILABLE_LIST_REPLY);
   evas_list_free(dat);
   e_font_available_list_free(fa_list);
   END_GENERIC();
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL
     
/****************************************************************************/
#define HDL E_IPC_OP_FONT_AVAILABLE_LIST_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   GENERIC(HDL);
   LIST();
   DECODE(e_ipc_codec_str_list_dec) {
      FOR(dat) {
	 printf("REPLY: \"%s\"\n", (char *)(l->data));
      }
      FREE_LIST(dat);
   }
   END_GENERIC();
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_FONT_APPLY
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-font-apply", 0, "Apply font settings changes", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   GENERIC(HDL);
   e_font_apply();
   SAVE;
   END_GENERIC();
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL
 
/****************************************************************************/
#define HDL E_IPC_OP_FONT_FALLBACK_APPEND
#if (TYPE == E_REMOTE_OPTIONS)
   OP(	"-font-fallback-append", 
	1 /*num_params*/, 
	"Append OPT1 to the fontset", 
	0 /*num_expected_replies*/, 
	HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_STRING(params[0], HDL);
#elif (TYPE == E_WM_IN)
   STRING(s, HDL);
   e_font_fallback_append(s);
   SAVE;   
   END_STRING(s);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_FONT_FALLBACK_PREPEND
#if (TYPE == E_REMOTE_OPTIONS)
   OP(	"-font-fallback-prepend", 
	1 /*num_params*/, 
	"Prepend OPT1 to the fontset", 
	0 /*num_expected_replies*/, 
	HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_STRING(params[0], HDL);
#elif (TYPE == E_WM_IN)
   STRING(s, HDL);
   e_font_fallback_prepend(s);
   SAVE;   
   END_STRING(s);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_FONT_FALLBACK_LIST
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-font-fallback-list", 0, "List the fallback fonts in order", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   GENERIC(HDL);

   LIST_DATA();
   E_Font_Fallback *ff;
   FOR(e_config->font_fallbacks) { ff = l->data;
      dat = evas_list_append(dat, ff->name);
   }
   ENCODE(dat, e_ipc_codec_str_list_enc);
   SEND_DATA(E_IPC_OP_FONT_FALLBACK_LIST_REPLY);
   evas_list_free(dat);

   END_GENERIC();
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_FONT_FALLBACK_LIST_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   GENERIC(HDL);
   LIST();
   DECODE(e_ipc_codec_str_list_dec) {
      FOR(dat) {
	 printf("REPLY: \"%s\"\n", (char *)(l->data));
      }
      FREE_LIST(dat);
   }
   END_GENERIC();
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_FONT_FALLBACK_REMOVE
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-font-fallback-remove", 1, "Remove OPT1 from the fontset", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_STRING(params[0], HDL);
#elif (TYPE == E_WM_IN)
   STRING(s, HDL);
   e_font_fallback_remove(s);
   SAVE;   
   END_STRING(s);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_FONT_DEFAULT_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-font-default-set", 3, "Set textclass (OPT1) font (OPT2) and size (OPT3)", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_2STRING_INT(params[0], params[1], atoi(params[2]), HDL) 
#elif (TYPE == E_WM_IN)
   STRING2_INT(text_class, font_name, font_size, e_2str_int, HDL);
   e_font_default_set(text_class, font_name, font_size);
   SAVE;   
   END_STRING2_INT(e_2str_int);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_FONT_DEFAULT_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-font-default-get", 1, "List the default font associated with OPT1", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_STRING(params[0], HDL);
#elif (TYPE == E_WM_IN)
   STRING(text_class, HDL);
   E_Font_Default *efd;
   void *data;
   int bytes;
   
   efd = e_font_default_get(text_class);
   if (efd == NULL)
     data = NULL;
   else
     data = e_ipc_codec_2str_int_enc(efd->text_class, efd->font, efd->size, &bytes);
	
   SEND_DATA(E_IPC_OP_FONT_DEFAULT_GET_REPLY);

   END_STRING(text_class);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_FONT_DEFAULT_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   STRING2_INT(text_class, font_name, font_size, e_2str_int, HDL);
   printf("REPLY: DEFAULT TEXT_CLASS=\"%s\" NAME=\"%s\" SIZE=%d\n",
                    text_class, font_name, font_size); 
   END_STRING2_INT(e_2str_int);
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_FONT_DEFAULT_REMOVE
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-font-default-remove", 1, "Remove the default text class OPT1", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_STRING(params[0], HDL);
#elif (TYPE == E_WM_IN)
   STRING(text_class, HDL);
   e_font_default_remove(text_class);
   SAVE;
   END_STRING(text_class);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_FONT_DEFAULT_LIST
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-font-default-list", 0, "List all configured text classes", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_STRING2_INT_LIST(e_font_default_list(), E_Font_Default, efd, v, HDL);
   v->str1 = efd->text_class;
   v->str2 = efd->font;
   v->val  = efd->size;
   END_SEND_STRING2_INT_LIST(v, E_IPC_OP_FONT_DEFAULT_LIST_REPLY);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_FONT_DEFAULT_LIST_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   STRING2_INT_LIST(v, HDL);
   printf("REPLY: DEFAULT TEXT_CLASS=\"%s\" NAME=\"%s\" SIZE=%d\n", v->str1, v->str2, v->val);
   END_STRING2_INT_LIST(v);
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_FONT_FALLBACK_CLEAR
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-font-fallback-clear", 0, "Clear list of fallback fonts", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
   GENERIC(HDL);
   e_font_fallback_clear();
   SAVE;
   END_GENERIC();
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL
 





/****************************************************************************/
#define HDL E_IPC_OP_RESTART
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-restart", 0, "Restart Enlightenment", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   GENERIC(HDL);
   restart = 1;
   ecore_main_loop_quit();
   END_GENERIC();
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL
/****************************************************************************/
#define HDL E_IPC_OP_SHUTDOWN
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-shutdown", 0, "Shutdown (exit) Enlightenment", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   GENERIC(HDL);
   if (!e_util_immortal_check()) ecore_main_loop_quit();
   END_GENERIC();
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_LANG_LIST
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-lang-list", 0, "List all available languages", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   GENERIC(HDL);
   LIST_DATA();
   ENCODE((Evas_List *)e_intl_language_list(), e_ipc_codec_str_list_enc);
   SEND_DATA(E_IPC_OP_LANG_LIST_REPLY);
   END_GENERIC();
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL
     
/****************************************************************************/
#define HDL E_IPC_OP_LANG_LIST_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   GENERIC(HDL);
   LIST();
   DECODE(e_ipc_codec_str_list_dec) {
      FOR(dat) {
	 printf("REPLY: \"%s\"\n", (char *)(l->data));
      }
      FREE_LIST(dat);
   }
   END_GENERIC();
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_LANG_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-lang-set", 1, "Set the current language to 'OPT1'", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_STRING(params[0], HDL);
#elif (TYPE == E_WM_IN)
   STRING(s, HDL);
   E_FREE(e_config->language);
   e_config->language = strdup(s);
   if ((e_config->language) && (strlen(e_config->language) > 0))
     e_intl_language_set(e_config->language);
   else
     e_intl_language_set(NULL);
   SAVE;
   END_STRING(s);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL
     
/****************************************************************************/
#define HDL E_IPC_OP_LANG_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-lang-get", 0, "Get the current language", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_STRING(e_config->language, E_IPC_OP_LANG_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL
     
/****************************************************************************/
#define HDL E_IPC_OP_LANG_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   STRING(s, HDL);
   printf("REPLY: \"%s\"\n", s);
   END_STRING(s);
#elif (TYPE == E_LIB_IN)
   STRING(s, HDL);
   RESPONSE(r, E_Response_Language_Get);
   r->lang = strdup(s);
   END_RESPONSE(r, E_RESPONSE_LANGUAGE_GET);
   END_STRING(s);
#endif
#undef HDL




/****************************************************************************/
#define HDL E_IPC_OP_DIRS_LIST
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-dirs-list", 1, "List the directory of type specified by 'OPT1', try 'themes'", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_STRING(params[0], HDL);
#elif (TYPE == E_WM_IN)
   STRING(s, HDL);
   LIST_DATA()
   Evas_List *dir_list = NULL;
   if (!strcmp(s, "data"))
     dir_list = e_path_dir_list_get(path_data);
   else if (!strcmp(s, "images"))
     dir_list = e_path_dir_list_get(path_images);
   else if (!strcmp(s, "fonts"))
     dir_list = e_path_dir_list_get(path_fonts);
   else if (!strcmp(s, "themes"))
     dir_list = e_path_dir_list_get(path_themes);
   else if (!strcmp(s, "init"))
     dir_list = e_path_dir_list_get(path_init);
   else if (!strcmp(s, "icons"))
     dir_list = e_path_dir_list_get(path_icons);
   else if (!strcmp(s, "modules"))
     dir_list = e_path_dir_list_get(path_modules);
   else if (!strcmp(s, "backgrounds"))
     dir_list = e_path_dir_list_get(path_backgrounds);
   E_Path_Dir *p;
   dat = evas_list_append(dat, strdup(s));
   FOR(dir_list) { p = l->data;
     dat = evas_list_append(dat, p->dir);
   }

   ENCODE(dat, e_ipc_codec_str_list_enc);
   SEND_DATA(E_IPC_OP_DIRS_LIST_REPLY);
   evas_list_free(dat);
   e_path_dir_list_free(dir_list);
   END_STRING(s)
#elif (TYPE == E_REMOTE_IN)
#elif (TYPE == E_LIB_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_DIRS_LIST_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   GENERIC(HDL);
   LIST();
   DECODE(e_ipc_codec_str_list_dec) {
     FOR(dat) {
       if (dat == l)
	 printf("REPLY: Listing for \"%s\"\n", (char *)(l->data));
       else
	 printf("REPLY: \"%s\"\n", (char *)(l->data));
     }
     FREE_LIST(dat);
   }
   END_GENERIC();
#elif (TYPE == E_LIB_IN)
   GENERIC(HDL);
   LIST();
   DECODE(e_ipc_codec_str_list_dec) {
      int count;
      char *type;
      int res;
      RESPONSE(r, E_Response_Dirs_List);

      /* FIXME - this is a mess, needs to be merged into macros... */
      count = evas_list_count(dat);
      r->dirs = malloc(sizeof(char *) * count);
      r->count = count - 1; /* leave off the "type" */

      count = 0;
      FOR(dat) {
	 if (dat == l)
	   type = l->data;
	 else {
	   r->dirs[count] = l->data;
	   count++;
	 }
      }

      if (!strcmp(type, "data"))
	res = E_RESPONSE_DATA_DIRS_LIST;
      else if (!strcmp(type, "images"))
	res = E_RESPONSE_IMAGE_DIRS_LIST;
      else if (!strcmp(type, "fonts"))
	res = E_RESPONSE_FONT_DIRS_LIST;
      else if (!strcmp(type, "themes"))
	res = E_RESPONSE_THEME_DIRS_LIST;
      else if (!strcmp(type, "init"))
	res = E_RESPONSE_INIT_DIRS_LIST;
      else if (!strcmp(type, "icons"))
	res = E_RESPONSE_ICON_DIRS_LIST;
      else if (!strcmp(type, "modules"))
	res = E_RESPONSE_MODULE_DIRS_LIST;
      else if (!strcmp(type, "backgrounds"))
	res = E_RESPONSE_BACKGROUND_DIRS_LIST;
      END_RESPONSE(r, res); /* FIXME - need a custom free */
   }
   END_GENERIC();
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_DIRS_APPEND
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-dirs-list-append", 1, "Append the directory of type specified by 'OPT2 to the list in 'OPT1'", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_2STRING(params[0], params[1], HDL);
#elif (TYPE == E_WM_IN)
   STRING2(s1, s2, e_2str, HDL);
   { E_PATH_GET(path, s1)
     e_path_user_path_append(path, s2);
   }
   SAVE;
   END_STRING2(e_2str)
#elif (TYPE == E_REMOTE_IN)
#elif (TYPE == E_LIB_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_DIRS_PREPEND
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-dirs-list-prepend", 1, "Prepend the directory of type specified by 'OPT2 to the list in 'OPT1'", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_2STRING(params[0], params[1], HDL);
#elif (TYPE == E_WM_IN)
   STRING2(s1, s2, e_2str, HDL);
   { E_PATH_GET(path, s1)
     e_path_user_path_prepend(path, s2);
   }
   SAVE;
   END_STRING2(e_2str)
#elif (TYPE == E_REMOTE_IN)
#elif (TYPE == E_LIB_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_DIRS_REMOVE
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-dirs-list-remove", 1, "Remove the directory of type specified by 'OPT2 to the list in 'OPT1'", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_2STRING(params[0], params[1], HDL);
#elif (TYPE == E_WM_IN)
   STRING2(s1, s2, e_2str, HDL);
   { E_PATH_GET(path, s1)
     e_path_user_path_remove(path, s2);
   }
   SAVE;
   END_STRING2(e_2str)
#elif (TYPE == E_REMOTE_IN)
#elif (TYPE == E_LIB_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_FRAMERATE_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-framerate-set", 1, "Set the animation framerate (fps)", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_DOUBLE(atof(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_DOUBLE(dbl, HDL);
   e_config->framerate = dbl;
   E_CONFIG_LIMIT(e_config->framerate, 1.0, 200.0);
   edje_frametime_set(1.0 / e_config->framerate);
   SAVE;
   END_DOUBLE;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_FRAMERATE_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-framerate-get", 0, "Get the animation framerate (fps)", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_DOUBLE(e_config->framerate, E_IPC_OP_FRAMERATE_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_FRAMERATE_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_DOUBLE(fps, HDL);
   printf("REPLY: %3.3f\n", fps);
   END_DOUBLE;
#endif
#undef HDL


/****************************************************************************/
#define HDL E_IPC_OP_MENUS_SCROLL_SPEED_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-menus-scroll-speed-set", 1, "Set the scroll speed of menus (pixels/sec)", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_DOUBLE(atof(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_DOUBLE(dbl, HDL);
   e_config->menus_scroll_speed = dbl;
   E_CONFIG_LIMIT(e_config->menus_scroll_speed, 1.0, 20000.0);
   SAVE;
   END_DOUBLE;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_MENUS_SCROLL_SPEED_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-menus-scroll-speed-get", 0, "Get the scroll speed of menus (pixels/sec)", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_DOUBLE(e_config->menus_scroll_speed, E_IPC_OP_MENUS_SCROLL_SPEED_GET_REPLY, HDL)
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_MENUS_SCROLL_SPEED_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_DOUBLE(speed, HDL);
   printf("REPLY: %3.3f\n", speed);
   END_DOUBLE;
#endif
#undef HDL

   
/****************************************************************************/
#define HDL E_IPC_OP_FOCUS_POLICY_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-focus-policy-set", 1, "Set the focus policy. OPT1 = CLICK, MOUSE or SLOPPY", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT_START(HDL)
   int value = 0;
   if (!strcmp(params[0], "MOUSE")) value = E_FOCUS_MOUSE;
   else if (!strcmp(params[0], "CLICK")) value = E_FOCUS_CLICK;
   else if (!strcmp(params[0], "SLOPPY")) value = E_FOCUS_SLOPPY;
   else
     {
	 printf("focus must be MOUSE, CLICK or SLOPPY\n");
	 exit(-1);
     }
   REQ_INT_END(value, HDL);
#elif (TYPE == E_WM_IN)
   START_INT(value, HDL);
   e_border_button_bindings_ungrab_all();
   e_config->focus_policy = value;
   E_CONFIG_LIMIT(e_config->focus_policy, 0, 2);
   e_border_button_bindings_grab_all();
   SAVE;
   END_INT
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_FOCUS_POLICY_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-focus-policy-get", 0, "Get focus policy", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->focus_policy, E_IPC_OP_FOCUS_POLICY_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_FOCUS_POLICY_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(policy, HDL);
   if (policy == E_FOCUS_MOUSE)
     printf("REPLY: MOUSE\n");
   else if (policy == E_FOCUS_CLICK)
     printf("REPLY: CLICK\n");
   else if (policy == E_FOCUS_SLOPPY)
     printf("REPLY: SLOPPY\n");
   END_INT
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_USE_EDGE_FLIP_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-edge-flip-set", 1, "Set the edge flip flag (0/1)", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(value, HDL);
   e_config->use_edge_flip = value;
   E_CONFIG_LIMIT(e_config->use_edge_flip, 0, 1);
   e_zone_update_flip_all();
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_USE_EDGE_FLIP_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-edge-flip-get", 0, "Get the edge flip flag", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL)
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->use_edge_flip, E_IPC_OP_USE_EDGE_FLIP_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_USE_EDGE_FLIP_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(val, HDL)
   printf("REPLY: %i\n", val);
   END_INT;
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_EDGE_FLIP_TIMEOUT_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-edge-flip-timeout-set", 1, "Set the edge flip timeout (sec)", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_DOUBLE(atof(params[0]), HDL)
#elif (TYPE == E_WM_IN)
   START_DOUBLE(dbl, HDL);
   e_config->edge_flip_timeout = dbl;
   E_CONFIG_LIMIT(e_config->edge_flip_timeout, 0.0, 2.0);
   SAVE;
   END_DOUBLE;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_EDGE_FLIP_TIMEOUT_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-edge-flip-timeout-get", 0, "Get the edge flip timeout", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL)
#elif (TYPE == E_WM_IN)
   SEND_DOUBLE(e_config->edge_flip_timeout, E_IPC_OP_EDGE_FLIP_TIMEOUT_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_EDGE_FLIP_TIMEOUT_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_DOUBLE(val, HDL)
   printf("REPLY: %3.3f\n", val);
   END_DOUBLE;
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_FONT_CACHE_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-font-cache-set", 1, "Set the font cache size (Kb)", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL)
#elif (TYPE == E_WM_IN)
   START_INT(val, HDL);
   e_config->font_cache = val;
   E_CONFIG_LIMIT(e_config->font_cache, 0, 32 * 1024);
   e_canvas_recache();
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_FONT_CACHE_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-font-cache-get", 0, "Get the speculative font cache size (Kb)", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL)
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->font_cache, E_IPC_OP_FONT_CACHE_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_FONT_CACHE_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(val, HDL)
   printf("REPLY: %i\n", val);
   END_INT;
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_IMAGE_CACHE_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-image-cache-set", 1, "Set the image cache size (Kb)", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL)
#elif (TYPE == E_WM_IN)
   START_INT(val, HDL);
   e_config->image_cache = val;
   E_CONFIG_LIMIT(e_config->image_cache, 0, 256 * 1024);
   e_canvas_recache();
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_IMAGE_CACHE_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-image-cache-get", 0, "Get the speculative image cache size (Kb)", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL)
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->image_cache, E_IPC_OP_IMAGE_CACHE_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_IMAGE_CACHE_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(val, HDL)
   printf("REPLY: %i\n", val);
   END_INT;
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_MENUS_FAST_MOVE_THRESHOLD_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-menus-fast-move-threshold-set", 1, "Set the mouse speed (pixels/second) that is considered a 'fast move'", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_DOUBLE(atof(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_DOUBLE(val, HDL);
   e_config->menus_fast_mouse_move_threshhold = val;
   E_CONFIG_LIMIT(e_config->menus_fast_mouse_move_threshhold, 1.0, 2000.0);
   SAVE;
   END_DOUBLE;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_MENUS_FAST_MOVE_THRESHOLD_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-menus-fast-move-threshold-get", 0, "Get the mouse speed (pixels/second) that is considered a 'fast move'", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_DOUBLE(e_config->menus_fast_mouse_move_threshhold, E_IPC_OP_MENUS_FAST_MOVE_THRESHOLD_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_MENUS_FAST_MOVE_THRESHOLD_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_DOUBLE(val, HDL)
   printf("REPLY: %3.3f\n", val);
   END_DOUBLE;
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_MENUS_CLICK_DRAG_TIMEOUT_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-menus-click-drag-timeout-set", 1, "Set the time (in sec) between a mouse press and release that will keep the menu up anyway", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_DOUBLE(atof(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_DOUBLE(val, HDL);
   e_config->menus_click_drag_timeout = val;
   E_CONFIG_LIMIT(e_config->menus_click_drag_timeout, 0.0, 10.0);
   SAVE;
   END_DOUBLE;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_MENUS_CLICK_DRAG_TIMEOUT_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-menus-click-drag-timeout-get", 0, "Get the time (in sec) between a mouse press and release that will keep the menu up anyway", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL)
#elif (TYPE == E_WM_IN)
   SEND_DOUBLE(e_config->menus_click_drag_timeout, E_IPC_OP_MENUS_CLICK_DRAG_TIMEOUT_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_MENUS_CLICK_DRAG_TIMEOUT_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_DOUBLE(val, HDL)
   printf("REPLY: %3.3f\n", val);
   END_DOUBLE;
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_BORDER_SHADE_ANIMATE_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-border-shade-animate-set", 1, "Set the shading animation flag (0/1)", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(val, HDL);
   e_config->border_shade_animate = val;
   E_CONFIG_LIMIT(e_config->border_shade_animate, 0, 1);
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_BORDER_SHADE_ANIMATE_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-border-shade-animate-get", 0, "Get the shading animation flag (0/1)", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL)
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->border_shade_animate, E_IPC_OP_BORDER_SHADE_ANIMATE_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_BORDER_SHADE_ANIMATE_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(val, HDL)
   printf("REPLY: %i\n", val);
   END_INT;
#endif
#undef HDL

   /****************************************************************************/
#define HDL E_IPC_OP_BORDER_SHADE_TRANSITION_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-border-shade-transition-set", 1, "Set the shading animation algorithm (0, 1, 2 or 3)", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
      REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(val, HDL);
   e_config->border_shade_transition = val;
   E_CONFIG_LIMIT(e_config->border_shade_transition, 0, 3);
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_BORDER_SHADE_TRANSITION_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-border-shade-transition-get", 0, "Get the shading animation algorithm (0, 1, 2 or 3)", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL)
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->border_shade_transition, E_IPC_OP_BORDER_SHADE_TRANSITION_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_BORDER_SHADE_TRANSITION_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(val, HDL)
   printf("REPLY: %i\n", val);
   END_INT;
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_BORDER_SHADE_SPEED_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-border-shade-speed-set", 1, "Set the shading speed (pixels/sec)", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
      REQ_DOUBLE(atof(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_DOUBLE(val, HDL);
   e_config->border_shade_speed = val;
   E_CONFIG_LIMIT(e_config->border_shade_speed, 1.0, 20000.0);
   SAVE;
   END_DOUBLE;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_BORDER_SHADE_SPEED_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-border-shade-speed-get", 0, "Get the shading speed (pixels/sec)", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL)
#elif (TYPE == E_WM_IN)
   SEND_DOUBLE(e_config->border_shade_speed, E_IPC_OP_BORDER_SHADE_SPEED_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_BORDER_SHADE_SPEED_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_DOUBLE(val, HDL)
   printf("REPLY: %3.3f\n", val);
   END_DOUBLE;
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_DESKS_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-desks-set", 1, "Set the number of virtual desktops (X x Y. OPT1 = X, OPT2 = Y)", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_2INT(atoi(params[0]), atoi(params[1]), HDL);
#elif (TYPE == E_WM_IN)
   START_2INT(val1, val2, HDL);
   e_config->zone_desks_x_count = val1;
   e_config->zone_desks_y_count = val2;
   E_CONFIG_LIMIT(e_config->zone_desks_x_count, 1, 64)
   E_CONFIG_LIMIT(e_config->zone_desks_y_count, 1, 64)
   {
      Evas_List *l;
      for (l = e_manager_list(); l; l = l->next)
	{
	   E_Manager *man;
	   Evas_List *l2;
	   man = l->data;
	   for (l2 = man->containers; l2; l2 = l2->next)
	     {
		E_Container *con;
		Evas_List *l3;
		con = l2->data;
		for (l3 = con->zones; l3; l3 = l3->next)
		  {
		     E_Zone *zone;
		     zone = l3->data;
		     e_zone_desk_count_set(zone, 
					   e_config->zone_desks_x_count, 
					   e_config->zone_desks_y_count);
		  }
	     }
	}
   }
   SAVE;
   END_2INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_DESKS_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-desks-get", 0, "Get the number of virtual desktops", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL)
#elif (TYPE == E_WM_IN)
   SEND_2INT(e_config->zone_desks_x_count, e_config->zone_desks_y_count, E_IPC_OP_DESKS_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_DESKS_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_2INT(val1, val2, HDL)
   printf("REPLY: %i %i\n", val1, val2);
   END_2INT;
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_MAXIMIZE_POLICY_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-maximize-policy-set", 1, "Set the maximize policy. OPT1 = FULLSCREEN, SMART, EXPAND or FILL", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT_START(HDL)
   int value = 0;
   if (!strcmp(params[0], "FULLSCREEN")) value = E_MAXIMIZE_FULLSCREEN;
   else if (!strcmp(params[0], "SMART")) value = E_MAXIMIZE_SMART;
   else if (!strcmp(params[0], "EXPAND")) value = E_MAXIMIZE_EXPAND;
   else if (!strcmp(params[0], "FILL")) value = E_MAXIMIZE_FILL;
   else
     {
	 printf("maximize must be FULLSCREEN, SMART, EXPAND or FILL\n");
	 exit(-1);
     }
   REQ_INT_END(value, HDL);
#elif (TYPE == E_WM_IN)
   START_INT(value, HDL);
   e_config->maximize_policy = value;
   E_CONFIG_LIMIT(e_config->maximize_policy, E_MAXIMIZE_FULLSCREEN, E_MAXIMIZE_FILL);
   SAVE;
   END_INT
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_MAXIMIZE_POLICY_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-maximize-policy-get", 0, "Get maximize policy", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->maximize_policy, E_IPC_OP_MAXIMIZE_POLICY_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_MAXIMIZE_POLICY_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(policy, HDL);
   if (policy == E_MAXIMIZE_FULLSCREEN)
     printf("REPLY: FULLSCREEN\n");
   else if (policy == E_MAXIMIZE_SMART)
     printf("REPLY: SMART\n");
   else if (policy == E_MAXIMIZE_EXPAND)
     printf("REPLY: EXPAND\n");
   else if (policy == E_MAXIMIZE_FILL)
     printf("REPLY: FILL\n");
   END_INT
#endif
#undef HDL

/****************************************************************************/

#define HDL E_IPC_OP_BINDING_MOUSE_LIST 
#if (TYPE == E_REMOTE_OPTIONS)
   /* e_remote define command line args */
   OP("-binding-mouse-list", 0, "List all mouse bindings", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   /* e_remote parse command line args encode and send request */
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   /* e_ipc decode request and do action or send reply */
   SEND_INT4_STRING2_LIST(e_config->mouse_bindings, E_Config_Binding_Mouse, emb, v, HDL);
   v->val1 = emb->context;
   v->val2 = emb->modifiers;
   v->str1 = emb->action;
   v->str2 = emb->params;
   v->val3 = emb->button;
   v->val4 = emb->any_mod;
   END_SEND_INT4_STRING2_LIST(v, E_IPC_OP_BINDING_MOUSE_LIST_REPLY);
#endif
#undef HDL

/****************************************************************************/

#define HDL E_IPC_OP_BINDING_MOUSE_LIST_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   /* e_remote decode the response from e_ipc and print it to the console */
   INT4_STRING2_LIST(v, HDL);
   {
      char *context;
      char modifier[256];
      
      if (v->val1 == E_BINDING_CONTEXT_NONE) context = "NONE";
      else if (v->val1 == E_BINDING_CONTEXT_UNKNOWN) context = "UNKNOWN";
      else if (v->val1 == E_BINDING_CONTEXT_BORDER) context = "BORDER";
      else if (v->val1 == E_BINDING_CONTEXT_ZONE) context = "ZONE";
      else if (v->val1 == E_BINDING_CONTEXT_CONTAINER) context = "CONTAINER";
      else if (v->val1 == E_BINDING_CONTEXT_MANAGER) context = "MANAGER";
      else if (v->val1 == E_BINDING_CONTEXT_MENU) context = "MENU";
      else if (v->val1 == E_BINDING_CONTEXT_WINLIST) context = "WINLIST";
      else if (v->val1 == E_BINDING_CONTEXT_ANY) context = "ANY";
      else context = "";
 
      modifier[0] = 0;
      if (v->val2 & E_BINDING_MODIFIER_SHIFT)
      {
        if (modifier[0] != 0) strcat(modifier, "|");
        strcat(modifier, "SHIFT");
      }
      if (v->val2 & E_BINDING_MODIFIER_CTRL)
      {
        if (modifier[0] != 0) strcat(modifier, "|");
        strcat(modifier, "CTRL");
      }
      if (v->val2 & E_BINDING_MODIFIER_ALT)
      {
        if (modifier[0] != 0) strcat(modifier, "|");
        strcat(modifier, "ALT");
      }
      if (v->val2 & E_BINDING_MODIFIER_WIN)
      {
        if (modifier[0] != 0) strcat(modifier, "|");
        strcat(modifier, "WIN");
      }
      if (v->val2 == E_BINDING_MODIFIER_NONE)
         strcpy(modifier, "NONE");

      printf("REPLY: BINDING CONTEXT=%s BUTTON=%i MODIFIERS=%s ANY_MOD=%i ACTION=\"%s\" PARAMS=\"%s\"\n",
		context,
                v->val3,
		modifier,
                v->val4,
                v->str1,
                v->str2
        );
   }
   END_INT4_STRING2_LIST(v);
#elif E_LIB_IN
   INT4_STRING2_LIST_START(v, HDL);
   {
      int count;
      RESPONSE(r, E_Response_Binding_Mouse_List);
      count = evas_list_count(dat);
      r->bindings = malloc(sizeof(E_Response_Binding_Mouse_Data *) * count);
      r->count = count;

      count = 0;
      INT4_STRING2_LIST_ITERATE(v);
      {
	 E_Response_Binding_Mouse_Data *d;

	 d = malloc(sizeof(E_Response_Binding_Mouse_Data));
	 d->ctx = v->val1;
	 d->button = v->val3;
	 d->mod = v->val2;
	 d->any_mod = v->val4;
	 d->action = ((v->str1) ? strdup(v->str1) : NULL);
	 d->params = ((v->str2) ? strdup(v->str2) : NULL);

	 r->bindings[count] = d;
	 count++;
      }
      END_INT4_STRING2_LIST_ITERATE(v);
      /* this will leak, need to free the event data */
      END_RESPONSE(r, E_RESPONSE_BINDING_MOUSE_LIST);
   }
   END_INT4_STRING2_LIST_START();
#endif
#undef HDL

/****************************************************************************/

#define HDL E_IPC_OP_BINDING_MOUSE_ADD
#if (TYPE == E_REMOTE_OPTIONS)
   /* e_remote define command line args */
   OP("-binding-mouse-add", 6, "Add an existing mouse binding. OPT1 = Context, OPT2 = button, OPT3 = modifiers, OPT4 = any modifier ok, OPT5 = action, OPT6 = action parameters", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   /* e_remote parse command line args encode and send request */
   REQ_4INT_2STRING_START(HDL);
   E_Config_Binding_Mouse eb;

   if (!strcmp(params[0], "NONE")) eb.context = E_BINDING_CONTEXT_NONE;
   else if (!strcmp(params[0], "UNKNOWN")) eb.context = E_BINDING_CONTEXT_UNKNOWN;
   else if (!strcmp(params[0], "BORDER")) eb.context = E_BINDING_CONTEXT_BORDER;
   else if (!strcmp(params[0], "ZONE")) eb.context = E_BINDING_CONTEXT_ZONE;
   else if (!strcmp(params[0], "CONTAINER")) eb.context = E_BINDING_CONTEXT_CONTAINER;
   else if (!strcmp(params[0], "MANAGER")) eb.context = E_BINDING_CONTEXT_MANAGER;
   else if (!strcmp(params[0], "MENU")) eb.context = E_BINDING_CONTEXT_MENU;
   else if (!strcmp(params[0], "WINLIST")) eb.context = E_BINDING_CONTEXT_WINLIST;
   else if (!strcmp(params[0], "ANY")) eb.context = E_BINDING_CONTEXT_ANY;
   else
     {
        printf("OPT1 (CONTEXT) is not a valid context. Must be:\n"
               "  NONE UNKNOWN BORDER ZONE CONTAINER MANAGER MENU WINLIST ANY\n");
        exit(-1);
     }
   eb.button = atoi(params[1]);
   /* M1[|M2...] */
     {
        char *p, *pp;

        eb.modifiers = 0;
        pp = params[2];
        for (;;)
          {
             p = strchr(pp, '|');
             if (p)
               {
                  if (!strncmp(pp, "SHIFT|", 6)) eb.modifiers |= E_BINDING_MODIFIER_SHIFT;
                  else if (!strncmp(pp, "CTRL|", 5)) eb.modifiers |= E_BINDING_MODIFIER_CTRL;
                  else if (!strncmp(pp, "ALT|", 4)) eb.modifiers |= E_BINDING_MODIFIER_ALT;
                  else if (!strncmp(pp, "WIN|", 4)) eb.modifiers |= E_BINDING_MODIFIER_WIN;
                  else if (strlen(pp) > 0)
                    {
                       printf("OPT3 moidifier unknown. Must be or mask of:\n"
                              "  SHIFT CTRL ALT WIN (eg SHIFT|CTRL or ALT|SHIFT|CTRL or ALT or just NONE)\n");
                       exit(-1);
                    }
                  pp = p + 1;
               }
             else
               {
                  if (!strcmp(pp, "SHIFT")) eb.modifiers |= E_BINDING_MODIFIER_SHIFT;
                  else if (!strcmp(pp, "CTRL")) eb.modifiers |= E_BINDING_MODIFIER_CTRL;
                  else if (!strcmp(pp, "ALT")) eb.modifiers |= E_BINDING_MODIFIER_ALT;
                  else if (!strcmp(pp, "WIN")) eb.modifiers |= E_BINDING_MODIFIER_WIN;
                  else if (!strcmp(pp, "NONE")) eb.modifiers = E_BINDING_MODIFIER_NONE;
                  else if (strlen(pp) > 0)
                    {
                       printf("OPT3 moidifier unknown. Must be or mask of:\n"
                              "  SHIFT CTRL ALT WIN (eg SHIFT|CTRL or ALT|SHIFT|CTRL or ALT or just NONE)\n");
                       exit(-1);
                    }
                  break;
               }
          }
     }
   eb.any_mod = atoi(params[3]);
   eb.action = params[4];
   eb.params = params[5];
   REQ_4INT_2STRING_END(eb.context, eb.modifiers, eb.button, eb.any_mod, eb.action, eb.params, HDL);
#elif (TYPE == E_WM_IN)
   /* e_ipc decode request and do action */
   INT4_STRING2(v, HDL)
   E_Config_Binding_Mouse bind, *eb;

   bind.context = v->val1; 
   bind.modifiers = v->val2;
   bind.button = v->val3;
   bind.any_mod = v->val4;
   bind.action = v->str1; 
   bind.params = v->str2;

   eb = e_config_binding_mouse_match(&bind);
   if (!eb)
     {
        eb = E_NEW(E_Config_Binding_Mouse, 1);
        e_config->mouse_bindings = evas_list_append(e_config->mouse_bindings, eb);
        eb->context = bind.context;
        eb->button = bind.button;
        eb->modifiers = bind.modifiers;
        eb->any_mod = bind.any_mod;
        eb->action = strdup(bind.action);
        eb->params = strdup(bind.params);
        e_border_button_bindings_ungrab_all();
        e_bindings_mouse_add(bind.context, bind.button, bind.modifiers,
                        bind.any_mod, bind.action, bind.params);
	e_border_button_bindings_grab_all();
        e_config_save_queue();  
     }
   END_INT4_STRING2(v);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/

#define HDL E_IPC_OP_BINDING_MOUSE_DEL
#if (TYPE == E_REMOTE_OPTIONS)
   /* e_remote define command line args */
   OP("-binding-mouse-del", 6, "Delete an existing mouse binding. OPT1 = Context, OPT2 = button, OPT3 = modifiers, OPT4 = any modifier ok, OPT5 = action, OPT6 = action parameters", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   /* e_remote parse command line args encode and send request */
   REQ_4INT_2STRING_START(HDL);
   E_Config_Binding_Mouse eb;

   if (!strcmp(params[0], "NONE")) eb.context = E_BINDING_CONTEXT_NONE;
   else if (!strcmp(params[0], "UNKNOWN")) eb.context = E_BINDING_CONTEXT_UNKNOWN;
   else if (!strcmp(params[0], "BORDER")) eb.context = E_BINDING_CONTEXT_BORDER;
   else if (!strcmp(params[0], "ZONE")) eb.context = E_BINDING_CONTEXT_ZONE;
   else if (!strcmp(params[0], "CONTAINER")) eb.context = E_BINDING_CONTEXT_CONTAINER;
   else if (!strcmp(params[0], "MANAGER")) eb.context = E_BINDING_CONTEXT_MANAGER;
   else if (!strcmp(params[0], "MENU")) eb.context = E_BINDING_CONTEXT_MENU;
   else if (!strcmp(params[0], "WINLIST")) eb.context = E_BINDING_CONTEXT_WINLIST;
   else if (!strcmp(params[0], "ANY")) eb.context = E_BINDING_CONTEXT_ANY;
   else
     {
        printf("OPT1 (CONTEXT) is not a valid context. Must be:\n"
               "  NONE UNKNOWN BORDER ZONE CONTAINER MANAGER MENU WINLIST ANY\n");
        exit(-1);
     }
   eb.button = atoi(params[1]);
   /* M1[|M2...] */
     {
        char *p, *pp;

        eb.modifiers = 0;
        pp = params[2];
        for (;;)
          {
             p = strchr(pp, '|');
             if (p)
               {
                  if (!strncmp(pp, "SHIFT|", 6)) eb.modifiers |= E_BINDING_MODIFIER_SHIFT;
                  else if (!strncmp(pp, "CTRL|", 5)) eb.modifiers |= E_BINDING_MODIFIER_CTRL;
                  else if (!strncmp(pp, "ALT|", 4)) eb.modifiers |= E_BINDING_MODIFIER_ALT;
                  else if (!strncmp(pp, "WIN|", 4)) eb.modifiers |= E_BINDING_MODIFIER_WIN;
                  else if (strlen(pp) > 0)
                    {
                       printf("OPT3 moidifier unknown. Must be or mask of:\n"
                              "  SHIFT CTRL ALT WIN (eg SHIFT|CTRL or ALT|SHIFT|CTRL or ALT or just NONE)\n");
                       exit(-1);
                    }
                  pp = p + 1;
               }
             else
               {
                  if (!strcmp(pp, "SHIFT")) eb.modifiers |= E_BINDING_MODIFIER_SHIFT;
                  else if (!strcmp(pp, "CTRL")) eb.modifiers |= E_BINDING_MODIFIER_CTRL;
                  else if (!strcmp(pp, "ALT")) eb.modifiers |= E_BINDING_MODIFIER_ALT;
                  else if (!strcmp(pp, "WIN")) eb.modifiers |= E_BINDING_MODIFIER_WIN;
                  else if (!strcmp(pp, "NONE")) eb.modifiers = E_BINDING_MODIFIER_NONE;
                  else if (strlen(pp) > 0)
                    {
                       printf("OPT3 moidifier unknown. Must be or mask of:\n"
                              "  SHIFT CTRL ALT WIN (eg SHIFT|CTRL or ALT|SHIFT|CTRL or ALT or just NONE)\n");
                       exit(-1);
                    }
                  break;
               }
          }
     }
   eb.any_mod = atoi(params[3]);
   eb.action = params[4];
   eb.params = params[5];
 
   REQ_4INT_2STRING_END(eb.context, eb.modifiers, eb.button, eb.any_mod, eb.action, eb.params, HDL);
#elif (TYPE == E_WM_IN)
   /* e_ipc decode request and do action */
   INT4_STRING2(v, HDL)
   E_Config_Binding_Mouse bind, *eb;

   bind.context = v->val1;
   bind.modifiers = v->val2;
   bind.button = v->val3;
   bind.any_mod = v->val4;
   bind.action = v->str1;
   bind.params = v->str2;
   
   eb = e_config_binding_mouse_match(&bind);
   if (eb)
     {
	e_config->mouse_bindings = evas_list_remove(e_config->mouse_bindings, eb);
        E_FREE(eb->action);
        E_FREE(eb->params);
        E_FREE(eb);
        e_border_button_bindings_ungrab_all();
        e_bindings_mouse_del(bind.context, bind.button, bind.modifiers,
                bind.any_mod, bind.action, bind.params);
        e_border_button_bindings_grab_all();
        e_config_save_queue();
     }
   END_INT4_STRING2(v);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/

#define HDL E_IPC_OP_BINDING_KEY_LIST 
#if (TYPE == E_REMOTE_OPTIONS)
   /* e_remote define command line args */
   OP("-binding-key-list", 0, "List all key bindings", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   /* e_remote parse command line args encode and send request */
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   /* e_ipc decode request and do action or send reply */
   SEND_INT3_STRING3_LIST(e_config->key_bindings, E_Config_Binding_Key, ekb, v, HDL);
   v->val1 = ekb->context;
   v->val2 = ekb->modifiers;
   v->val3 = ekb->any_mod;
   v->str1 = ekb->key;
   v->str2 = ekb->action;
   v->str3 = ekb->params;
   END_SEND_INT3_STRING3_LIST(v, E_IPC_OP_BINDING_KEY_LIST_REPLY);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/

#define HDL E_IPC_OP_BINDING_KEY_LIST_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   /* e_remote decode the response from e_ipc and print it to the console */
   INT3_STRING3_LIST(v, HDL);
   {
      char *context;
      char modifier[256];
      
      if (v->val1 == E_BINDING_CONTEXT_NONE) context = "NONE";
      else if (v->val1 == E_BINDING_CONTEXT_UNKNOWN) context = "UNKNOWN";
      else if (v->val1 == E_BINDING_CONTEXT_BORDER) context = "BORDER";
      else if (v->val1 == E_BINDING_CONTEXT_ZONE) context = "ZONE";
      else if (v->val1 == E_BINDING_CONTEXT_CONTAINER) context = "CONTAINER";
      else if (v->val1 == E_BINDING_CONTEXT_MANAGER) context = "MANAGER";
      else if (v->val1 == E_BINDING_CONTEXT_MENU) context = "MENU";
      else if (v->val1 == E_BINDING_CONTEXT_WINLIST) context = "WINLIST";
      else if (v->val1 == E_BINDING_CONTEXT_ANY) context = "ANY";
      else context = "";
 
      modifier[0] = 0;
      if (v->val2 & E_BINDING_MODIFIER_SHIFT)
      {
        if (modifier[0] != 0) strcat(modifier, "|");
        strcat(modifier, "SHIFT");
      }
      if (v->val2 & E_BINDING_MODIFIER_CTRL)
      {
        if (modifier[0] != 0) strcat(modifier, "|");
        strcat(modifier, "CTRL");
      }
      if (v->val2 & E_BINDING_MODIFIER_ALT)
      {
        if (modifier[0] != 0) strcat(modifier, "|");
        strcat(modifier, "ALT");
      }
      if (v->val2 & E_BINDING_MODIFIER_WIN)
      {
        if (modifier[0] != 0) strcat(modifier, "|");
        strcat(modifier, "WIN");
      }
      if (v->val2 == E_BINDING_MODIFIER_NONE)
         strcpy(modifier, "NONE");

      printf("REPLY: BINDING CONTEXT=%s KEY=\"%s\" MODIFIERS=%s ANY_MOD=%i ACTION=\"%s\" PARAMS=\"%s\"\n",
		context,
                v->str1,
		modifier,
                v->val3,
                v->str2,
                v->str3
        );
   }
   END_INT3_STRING3_LIST(v);
#elif E_LIB_IN
   INT3_STRING3_LIST_START(v, HDL);
   {
      int count = 0;
      RESPONSE(r, E_Response_Binding_Key_List);
      count = evas_list_count(dat);
      r->bindings = malloc(sizeof(E_Response_Binding_Key_Data *) * count);
      r->count = count;

      count = 0;
      INT3_STRING3_LIST_ITERATE(v);
      {
	 E_Response_Binding_Key_Data *d;

	 d = malloc(sizeof(E_Response_Binding_Key_Data));
	 d->ctx = v->val1;
	 d->key = ((v->str1) ? strdup(v->str1) : NULL);
	 d->mod = v->val2;
	 d->any_mod = v->val3;
	 d->action = ((v->str2) ? strdup(v->str2) : NULL);
	 d->params = ((v->str3) ? strdup(v->str3) : NULL);

	 r->bindings[count] = d;
	 count++;
      }
      END_INT3_STRING3_LIST_ITERATE(v);
      /* this will leak, need to free the event data */
      END_RESPONSE(r, E_RESPONSE_BINDING_KEY_LIST);
   }
   END_INT3_STRING3_LIST_START();
#endif
#undef HDL

/****************************************************************************/

#define HDL E_IPC_OP_BINDING_KEY_ADD
#if (TYPE == E_REMOTE_OPTIONS)
   /* e_remote define command line args */
   OP("-binding-key-add", 6, "Add an existing key binding. OPT1 = Context, OPT2 = key, OPT3 = modifiers, OPT4 = any modifier ok, OPT5 = action, OPT6 = action parameters", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   /* e_remote parse command line args encode and send request */
   REQ_3INT_3STRING_START(HDL);
   E_Config_Binding_Key eb;
   if (!strcmp(params[0], "NONE")) eb.context = E_BINDING_CONTEXT_NONE;
   else if (!strcmp(params[0], "UNKNOWN")) eb.context = E_BINDING_CONTEXT_UNKNOWN;
   else if (!strcmp(params[0], "BORDER")) eb.context = E_BINDING_CONTEXT_BORDER;
   else if (!strcmp(params[0], "ZONE")) eb.context = E_BINDING_CONTEXT_ZONE;
   else if (!strcmp(params[0], "CONTAINER")) eb.context = E_BINDING_CONTEXT_CONTAINER;
   else if (!strcmp(params[0], "MANAGER")) eb.context = E_BINDING_CONTEXT_MANAGER;
   else if (!strcmp(params[0], "MENU")) eb.context = E_BINDING_CONTEXT_MENU;
   else if (!strcmp(params[0], "WINLIST")) eb.context = E_BINDING_CONTEXT_WINLIST;
   else if (!strcmp(params[0], "ANY")) eb.context = E_BINDING_CONTEXT_ANY;
   else
     {
        printf("OPT1 (CONTEXT) is not a valid context. Must be:\n"
               "  NONE UNKNOWN BORDER ZONE CONTAINER MANAGER MENU WINLIST ANY\n");
        exit(-1);
     }
   eb.key = params[1];
   /* M1[|M2...] */
     {
	char *p, *pp;
	
	eb.modifiers = 0;
	pp = params[2];
	for (;;)
	  {
	     p = strchr(pp, '|');
	     if (p)
	       {
		  if (!strncmp(pp, "SHIFT|", 6)) eb.modifiers |= E_BINDING_MODIFIER_SHIFT;
		  else if (!strncmp(pp, "CTRL|", 5)) eb.modifiers |= E_BINDING_MODIFIER_CTRL;
		  else if (!strncmp(pp, "ALT|", 4)) eb.modifiers |= E_BINDING_MODIFIER_ALT;
		  else if (!strncmp(pp, "WIN|", 4)) eb.modifiers |= E_BINDING_MODIFIER_WIN;
		  else if (strlen(pp) > 0)
		    {
		       printf("OPT3 moidifier unknown. Must be or mask of:\n"
			      "  SHIFT CTRL ALT WIN (eg SHIFT|CTRL or ALT|SHIFT|CTRL or ALT or just NONE)\n");
		       exit(-1);
		    }
		  pp = p + 1;
	       }
	     else
	       {
		  if (!strcmp(pp, "SHIFT")) eb.modifiers |= E_BINDING_MODIFIER_SHIFT;
		  else if (!strcmp(pp, "CTRL")) eb.modifiers |= E_BINDING_MODIFIER_CTRL;
		  else if (!strcmp(pp, "ALT")) eb.modifiers |= E_BINDING_MODIFIER_ALT;
		  else if (!strcmp(pp, "WIN")) eb.modifiers |= E_BINDING_MODIFIER_WIN;
		  else if (!strcmp(pp, "NONE")) eb.modifiers = E_BINDING_MODIFIER_NONE;
		  else if (strlen(pp) > 0)
		    {
		       printf("OPT3 moidifier unknown. Must be or mask of:\n"
			      "  SHIFT CTRL ALT WIN (eg SHIFT|CTRL or ALT|SHIFT|CTRL or ALT or just NONE)\n");
		       exit(-1);
		    }
		  break;
	       }
	  }
     }
   eb.any_mod = atoi(params[3]);
   eb.action = params[4];
   eb.params = params[5];
   REQ_3INT_3STRING_END(eb.context, eb.modifiers, eb.any_mod, eb.key, eb.action, eb.params, HDL);
#elif (TYPE == E_WM_IN)
   /* e_ipc decode request and do action */
   INT3_STRING3(v, HDL)
   E_Config_Binding_Key bind, *eb;

   bind.context = v->val1; 
   bind.modifiers = v->val2;
   bind.any_mod = v->val3;
   bind.key = v->str1; 
   bind.action = v->str2;
   bind.params = v->str3;

   eb = e_config_binding_key_match(&bind);
   if (!eb)
     {
        eb = E_NEW(E_Config_Binding_Key, 1);
        e_config->key_bindings = evas_list_append(e_config->key_bindings, eb);
        eb->context = bind.context;
        eb->modifiers = bind.modifiers;
        eb->any_mod = bind.any_mod;
        eb->key = strdup(bind.key);
        eb->action = strdup(bind.action);
        eb->params = strdup(bind.params);
        e_managers_keys_ungrab();
        e_bindings_key_add(bind.context, bind.key, bind.modifiers,
                bind.any_mod, bind.action, bind.params);
        e_managers_keys_grab();
        e_config_save_queue();
     }
   END_INT3_STRING3(v);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/

#define HDL E_IPC_OP_BINDING_KEY_DEL
#if (TYPE == E_REMOTE_OPTIONS)
   /* e_remote define command line args */
   OP("-binding-key-del", 6, "Delete an existing key binding. OPT1 = Context, OPT2 = key, OPT3 = modifiers, OPT4 = any modifier ok, OPT5 = action, OPT6 = action parameters", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   /* e_remote parse command line args encode and send request */
   REQ_3INT_3STRING_START(HDL);
   E_Config_Binding_Key eb;
   if (!strcmp(params[0], "NONE")) eb.context = E_BINDING_CONTEXT_NONE;
   else if (!strcmp(params[0], "UNKNOWN")) eb.context = E_BINDING_CONTEXT_UNKNOWN;
   else if (!strcmp(params[0], "BORDER")) eb.context = E_BINDING_CONTEXT_BORDER;
   else if (!strcmp(params[0], "ZONE")) eb.context = E_BINDING_CONTEXT_ZONE;
   else if (!strcmp(params[0], "CONTAINER")) eb.context = E_BINDING_CONTEXT_CONTAINER;
   else if (!strcmp(params[0], "MANAGER")) eb.context = E_BINDING_CONTEXT_MANAGER;
   else if (!strcmp(params[0], "MENU")) eb.context = E_BINDING_CONTEXT_MENU;
   else if (!strcmp(params[0], "WINLIST")) eb.context = E_BINDING_CONTEXT_WINLIST;
   else if (!strcmp(params[0], "ANY")) eb.context = E_BINDING_CONTEXT_ANY;
   else
     {
        printf("OPT1 (CONTEXT) is not a valid context. Must be:\n"
               "  NONE UNKNOWN BORDER ZONE CONTAINER MANAGER MENU WINLIST ANY\n");
        exit(-1);
     }
   eb.key = params[1];
   /* M1[|M2...] */
     {
	char *p, *pp;
	
	eb.modifiers = 0;
	pp = params[2];
	for (;;)
	  {
	     p = strchr(pp, '|');
	     if (p)
	       {
		  if (!strncmp(pp, "SHIFT|", 6)) eb.modifiers |= E_BINDING_MODIFIER_SHIFT;
		  else if (!strncmp(pp, "CTRL|", 5)) eb.modifiers |= E_BINDING_MODIFIER_CTRL;
		  else if (!strncmp(pp, "ALT|", 4)) eb.modifiers |= E_BINDING_MODIFIER_ALT;
		  else if (!strncmp(pp, "WIN|", 4)) eb.modifiers |= E_BINDING_MODIFIER_WIN;
		  else if (strlen(pp) > 0)
		    {
		       printf("OPT3 moidifier unknown. Must be or mask of:\n"
			      "  SHIFT CTRL ALT WIN (eg SHIFT|CTRL or ALT|SHIFT|CTRL or ALT or just NONE)\n");
		       exit(-1);
		    }
		  pp = p + 1;
	       }
	     else
	       {
		  if (!strcmp(pp, "SHIFT")) eb.modifiers |= E_BINDING_MODIFIER_SHIFT;
		  else if (!strcmp(pp, "CTRL")) eb.modifiers |= E_BINDING_MODIFIER_CTRL;
		  else if (!strcmp(pp, "ALT")) eb.modifiers |= E_BINDING_MODIFIER_ALT;
		  else if (!strcmp(pp, "WIN")) eb.modifiers |= E_BINDING_MODIFIER_WIN;
		  else if (!strcmp(pp, "NONE")) eb.modifiers = E_BINDING_MODIFIER_NONE;
		  else if (strlen(pp) > 0)
		    {
		       printf("OPT3 moidifier unknown. Must be or mask of:\n"
			      "  SHIFT CTRL ALT WIN (eg SHIFT|CTRL or ALT|SHIFT|CTRL or ALT or just NONE)\n");
		       exit(-1);
		    }
		  break;
	       }
	  }
     }
   eb.any_mod = atoi(params[3]);
   eb.action = params[4];
   eb.params = params[5];
   REQ_3INT_3STRING_END(eb.context, eb.modifiers, eb.any_mod, eb.key, eb.action, eb.params, HDL);
#elif (TYPE == E_WM_IN)
   /* e_ipc decode request and do action */
   INT3_STRING3(v, HDL)
   E_Config_Binding_Key bind, *eb;

   bind.context = v->val1; 
   bind.modifiers = v->val2;
   bind.any_mod = v->val3;
   bind.key = v->str1; 
   bind.action = v->str2;
   bind.params = v->str3;

   eb = e_config_binding_key_match(&bind);
   if (eb)
     {
       e_config->key_bindings = evas_list_remove(e_config->key_bindings, eb);
       E_FREE(eb->key);
       E_FREE(eb->action);
       E_FREE(eb->params);
       E_FREE(eb);
       e_managers_keys_ungrab();
       e_bindings_key_del(bind.context, bind.key, bind.modifiers,
			  bind.any_mod, bind.action, bind.params);
       e_managers_keys_grab();
       e_config_save_queue();
    }

   END_INT3_STRING3(v);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_ALWAYS_CLICK_TO_RAISE_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-always-click-to-raise-set", 1, "Set the always click to raise policy, 1 for enabled 0 for disabled", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(policy, HDL);
   e_config->always_click_to_raise = policy;
   E_CONFIG_LIMIT(e_config->always_click_to_raise, 0, 1);
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_ALWAYS_CLICK_TO_RAISE_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-always-click-to-raise-get", 0, "Get the always click to raise policy, 1 for enabled 0 for disabled", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->always_click_to_raise, E_IPC_OP_ALWAYS_CLICK_TO_RAISE_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_ALWAYS_CLICK_TO_RAISE_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(policy, HDL);
   printf("REPLY: POLICY=%d\n", policy);
   END_INT;
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_USE_AUTO_RAISE_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-use-auto-raise-set", 1, "Set use auto raise policy, 1 for enabled 0 for disabled", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(policy, HDL);
   e_config->use_auto_raise = policy;
   E_CONFIG_LIMIT(e_config->use_auto_raise, 0, 1);
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_USE_AUTO_RAISE_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-use-auto-raise-get", 0, "Get use auto raise policy, 1 for enabled 0 for disabled", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->use_auto_raise, E_IPC_OP_USE_AUTO_RAISE_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_USE_AUTO_RAISE_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(policy, HDL);
   printf("REPLY: POLICY=%d\n", policy);
   END_INT;
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_PASS_CLICK_ON_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-pass-click-on-set", 1, "Set pass click on policy, 1 for enabled 0 for disabled", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(policy, HDL);
   e_config->pass_click_on = policy;
   E_CONFIG_LIMIT(e_config->pass_click_on, 0, 1);
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_PASS_CLICK_ON_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-pass-click-on-get", 0, "Get pass click on policy, 1 for enabled 0 for disabled", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->pass_click_on, E_IPC_OP_PASS_CLICK_ON_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_PASS_CLICK_ON_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(policy, HDL);
   printf("REPLY: POLICY=%d\n", policy);
   END_INT;
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_AUTO_RAISE_DELAY_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-auto-raise-delay-set", 1, "Set the auto raise delay (Seconds)", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_DOUBLE(atof(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_DOUBLE(dbl, HDL);
   e_config->auto_raise_delay = dbl;
   E_CONFIG_LIMIT(e_config->auto_raise_delay, 0.0, 5.0);
   SAVE;
   END_DOUBLE;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_AUTO_RAISE_DELAY_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-auto-raise-delay-get", 0, "Get the auto raise delay  (Seconds)", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_DOUBLE(e_config->auto_raise_delay, E_IPC_OP_AUTO_RAISE_DELAY_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_AUTO_RAISE_DELAY_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_DOUBLE(dbl, HDL);
   printf("REPLY: DELAY=%3.3f\n", dbl);
   END_DOUBLE;
#endif
#undef HDL

/****************************************************************************/

#define HDL E_IPC_OP_USE_RESIST_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-use-resist-set", 1, "Set resist policy, 1 for enabled 0 for disabled", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(policy, HDL);
   e_config->use_resist = policy;
   E_CONFIG_LIMIT(e_config->use_resist, 0, 1);
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_USE_RESIST_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-use-resist-get", 0, "Get use resist policy, 1 for enabled 0 for disabled", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->use_resist, E_IPC_OP_USE_RESIST_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_USE_RESIST_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(policy, HDL);
   printf("REPLY: POLICY=%d\n", policy);
   END_INT;
#endif
#undef HDL

/****************************************************************************/

#define HDL E_IPC_OP_DRAG_RESIST_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-drag-resist-set", 1, "Set drag resist threshold (0-100)", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(val, HDL);
   e_config->drag_resist = val;
   E_CONFIG_LIMIT(e_config->drag_resist, 0, 100);
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_DRAG_RESIST_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-drag-resist-get", 0, "Get drag resist threshold", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->drag_resist, E_IPC_OP_DRAG_RESIST_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_DRAG_RESIST_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(val, HDL);
   printf("REPLY: THRESHOLD=%d\n", val);
   END_INT;
#endif
#undef HDL

/****************************************************************************/

#define HDL E_IPC_OP_DESK_RESIST_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-desk-resist-set", 1, "Set desktop resist threshold (0-100)", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(val, HDL);
   e_config->desk_resist = val;
   E_CONFIG_LIMIT(e_config->desk_resist, 0, 100);
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_DESK_RESIST_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-desk-resist-get", 0, "Get desktop resist threshold", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->desk_resist, E_IPC_OP_DESK_RESIST_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_DESK_RESIST_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(val, HDL);
   printf("REPLY: THRESHOLD=%d\n", val);
   END_INT;
#endif
#undef HDL

/****************************************************************************/

#define HDL E_IPC_OP_WINDOW_RESIST_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-window-resist-set", 1, "Set window resist threshold (0-100)", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(val, HDL);
   e_config->window_resist = val;
   E_CONFIG_LIMIT(e_config->window_resist, 0, 100);
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINDOW_RESIST_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-window-resist-get", 0, "Get window resist threshold", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->window_resist, E_IPC_OP_WINDOW_RESIST_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINDOW_RESIST_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(val, HDL);
   printf("REPLY: THRESHOLD=%d\n", val);
   END_INT;
#endif
#undef HDL

/****************************************************************************/

#define HDL E_IPC_OP_GADGET_RESIST_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-gadget-resist-set", 1, "Set gadget resist threshold (0-100)", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(val, HDL);
   e_config->gadget_resist = val;
   E_CONFIG_LIMIT(e_config->gadget_resist, 0, 100);
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_GADGET_RESIST_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-gadget-resist-get", 0, "Get gadget resist threshold", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->gadget_resist, E_IPC_OP_GADGET_RESIST_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_GADGET_RESIST_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(val, HDL);
   printf("REPLY: THRESHOLD=%d\n", val);
   END_INT;
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_DESKTOP_BG_ADD
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-desktop-bg-add", 5, "Add a desktop bg definition. OPT1 = container no. OPT2 = zone no. OPT3 = desk_x. OPT4 = desk_y. OPT5 = bg file path", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_4INT_2STRING_START(HDL);
   REQ_4INT_2STRING_END(atoi(params[0]), atoi(params[1]), atoi(params[2]), atoi(params[3]), params[4], "", HDL);
#elif (TYPE == E_WM_IN)
   INT4_STRING2(v, HDL);
   e_bg_add(v->val1, v->val2, v->val3, v->val4, v->str1);
   e_bg_update();
   SAVE;
   END_INT4_STRING2(v);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_DESKTOP_BG_DEL
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-desktop-bg-del", 4, "Delete a desktop bg definition. OPT1 = container no. OPT2 = zone no. OPT3 = desk_x. OPT4 = desk_y.", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_4INT_2STRING_START(HDL);
   REQ_4INT_2STRING_END(atoi(params[0]), atoi(params[1]), atoi(params[2]), atoi(params[3]), "", "", HDL);
#elif (TYPE == E_WM_IN)
   INT4_STRING2(v, HDL);
   e_bg_del(v->val1, v->val2, v->val3, v->val4);
   e_bg_update();
   SAVE;
   END_INT4_STRING2(v);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_DESKTOP_BG_LIST
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-desktop-bg-list", 0, "List all current desktop bg definitions", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT4_STRING2_LIST(e_config->desktop_backgrounds, E_Config_Desktop_Background, cfbg, v, HDL);
   v->val1 = cfbg->container;
   v->val2 = cfbg->zone;
   v->val3 = cfbg->desk_x;
   v->val4 = cfbg->desk_y;
   v->str1 = cfbg->file;
   v->str2 = "";
   END_SEND_INT4_STRING2_LIST(v, E_IPC_OP_DESKTOP_BG_LIST_REPLY);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_DESKTOP_BG_LIST_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   INT4_STRING2_LIST(v, HDL);
   printf("REPLY: BG CONTAINER=%i ZONE=%i DESK_X=%i DESK_Y=%i FILE=\"%s\"\n",
	  v->val1, v->val2, v->val3, v->val4, v->str1);
   END_INT4_STRING2_LIST(v);
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_WARP_WHILE_SELECTING_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-winlist-warp-while-selecting-set", 1, "Set winlist (alt+tab) warp while selecting policy", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(policy, HDL);
   e_config->winlist_warp_while_selecting = policy;
   E_CONFIG_LIMIT(e_config->winlist_warp_while_selecting, 0, 1);
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_WARP_WHILE_SELECTING_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-winlist-warp-while-selecting-get", 0, "Get winlist (alt+tab) warp while selecting policy", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->winlist_warp_while_selecting, E_IPC_OP_WINLIST_WARP_WHILE_SELECTING_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_WARP_WHILE_SELECTING_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(val, HDL);
   printf("REPLY: POLICY=%d\n", val);
   END_INT;
#endif
#undef HDL

/****************************************************************************/

#define HDL E_IPC_OP_WINLIST_WARP_AT_END_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-winlist-warp-at-end-set", 1, "Set winlist (alt+tab) warp at end policy", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(policy, HDL);
   e_config->winlist_warp_at_end = policy;
   E_CONFIG_LIMIT(e_config->winlist_warp_at_end, 0, 1);
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_WARP_AT_END_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-winlist-warp-at-end-get", 0, "Get winlist (alt+tab) warp at end policy", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->winlist_warp_at_end, E_IPC_OP_WINLIST_WARP_AT_END_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_WARP_AT_END_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(val, HDL);
   printf("REPLY: POLICY=%d\n", val);
   END_INT;
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_WARP_SPEED_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-winlist-warp-speed-set", 1, "Set winlist warp speed (0.0-1.0)", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_DOUBLE(atof(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_DOUBLE(val, HDL);
   e_config->winlist_warp_speed = val;
   E_CONFIG_LIMIT(e_config->winlist_warp_speed, 0.0, 1.0);
   SAVE;
   END_DOUBLE;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_WARP_SPEED_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-winlist-warp-speed-get", 0, "Get winlist warp speed", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_DOUBLE(e_config->winlist_warp_speed, E_IPC_OP_WINLIST_WARP_SPEED_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_WARP_SPEED_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_DOUBLE(val, HDL);
   printf("REPLY: SPEED=%3.3f\n", val);
   END_DOUBLE;
#endif
#undef HDL

/****************************************************************************/

#define HDL E_IPC_OP_WINLIST_SCROLL_ANIMATE_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-winlist-scroll-animate-set", 1, "Set winlist (alt+tab) scroll animate policy", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(policy, HDL);
   e_config->winlist_scroll_animate = policy;
   E_CONFIG_LIMIT(e_config->winlist_scroll_animate, 0, 1);
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_SCROLL_ANIMATE_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-winlist-scroll-animate-get", 0, "Get winlist (alt+tab) scroll animate policy", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->winlist_scroll_animate, E_IPC_OP_WINLIST_SCROLL_ANIMATE_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_SCROLL_ANIMATE_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(val, HDL);
   printf("REPLY: POLICY=%d\n", val);
   END_INT;
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_SCROLL_SPEED_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-winlist-scroll-speed-set", 1, "Set winlist scroll speed (0.0-1.0)", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_DOUBLE(atof(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_DOUBLE(val, HDL);
   e_config->winlist_scroll_speed = val;
   E_CONFIG_LIMIT(e_config->winlist_scroll_speed, 0.0, 1.0);
   SAVE;
   END_DOUBLE;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_SCROLL_SPEED_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-winlist-scroll-speed-get", 0, "Get winlist scroll speed", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_DOUBLE(e_config->winlist_scroll_speed, E_IPC_OP_WINLIST_SCROLL_SPEED_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_SCROLL_SPEED_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_DOUBLE(val, HDL);
   printf("REPLY: SPEED=%3.3f\n", val);
   END_DOUBLE;
#endif
#undef HDL

/****************************************************************************/

#define HDL E_IPC_OP_WINLIST_LIST_SHOW_ICONIFIED_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-winlist-list-show-iconified-set", 1, "Set whether winlist (alt+tab) will show iconfied windows", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(policy, HDL);
   e_config->winlist_list_show_iconified = policy;
   E_CONFIG_LIMIT(e_config->winlist_list_show_iconified, 0, 1);
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_LIST_SHOW_ICONIFIED_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-winlist-list-show-iconified-get", 0, "Get whether winlist (alt+tab) will show iconfied windows", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->winlist_list_show_iconified, E_IPC_OP_WINLIST_LIST_SHOW_ICONIFIED_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_LIST_SHOW_ICONIFIED_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(val, HDL);
   printf("REPLY: POLICY=%d\n", val);
   END_INT;
#endif
#undef HDL

/****************************************************************************/

#define HDL E_IPC_OP_WINLIST_LIST_SHOW_OTHER_DESK_WINDOWS_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-winlist-list-show-other-desk-windows-set", 1, "Set whether winlist (alt+tab) will show other desk windows", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(policy, HDL);
   e_config->winlist_list_show_other_desk_windows = policy;
   E_CONFIG_LIMIT(e_config->winlist_list_show_other_desk_windows, 0, 1);
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_LIST_SHOW_OTHER_DESK_WINDOWS_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-winlist-list-show-other-desk-windows-get", 0, "Get winlist (alt+tab) show other desk windows", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->winlist_list_show_other_desk_windows, E_IPC_OP_WINLIST_LIST_SHOW_OTHER_DESK_WINDOWS_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_LIST_SHOW_OTHER_DESK_WINDOWS_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(val, HDL);
   printf("REPLY: POLICY=%d\n", val);
   END_INT;
#endif
#undef HDL

/****************************************************************************/

#define HDL E_IPC_OP_WINLIST_LIST_SHOW_OTHER_SCREEN_WINDOWS_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-winlist-list-show-other-screen-windows-set", 1, "Set winlist (alt+tab) show other screen windows policy", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(policy, HDL);
   e_config->winlist_list_show_other_screen_windows = policy;
   E_CONFIG_LIMIT(e_config->winlist_list_show_other_screen_windows, 0, 1);
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_LIST_SHOW_OTHER_SCREEN_WINDOWS_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-winlist-list-show-other-screen-windows-get", 0, "Get winlist (alt+tab) show other screen windows policy", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->winlist_list_show_other_screen_windows, E_IPC_OP_WINLIST_LIST_SHOW_OTHER_SCREEN_WINDOWS_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_LIST_SHOW_OTHER_SCREEN_WINDOWS_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(val, HDL);
   printf("REPLY: POLICY=%d\n", val);
   END_INT;
#endif
#undef HDL

/****************************************************************************/

#define HDL E_IPC_OP_WINLIST_LIST_UNCOVER_WHILE_SELECTING_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-winlist-list-uncover-while-selecting-set", 1, "Set whether winlist (alt+tab) will show iconified windows while selecting", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(policy, HDL);
   e_config->winlist_list_uncover_while_selecting = policy;
   E_CONFIG_LIMIT(e_config->winlist_list_uncover_while_selecting, 0, 1);
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_LIST_UNCOVER_WHILE_SELECTING_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-winlist-list-uncover-while-selecting-get", 0, "Get whether winlist (alt+tab) will show iconified windows while selecting", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->winlist_list_uncover_while_selecting, E_IPC_OP_WINLIST_LIST_UNCOVER_WHILE_SELECTING_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_LIST_UNCOVER_WHILE_SELECTING_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(val, HDL);
   printf("REPLY: POLICY=%d\n", val);
   END_INT;
#endif
#undef HDL

/****************************************************************************/

#define HDL E_IPC_OP_WINLIST_LIST_JUMP_DESK_WHILE_SELECTING_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-winlist-list-jump-desk-while-selecting-set", 1, "Set winlist (alt+tab) jump desk while selecting policy", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(policy, HDL);
   e_config->winlist_list_jump_desk_while_selecting = policy;
   E_CONFIG_LIMIT(e_config->winlist_list_jump_desk_while_selecting, 0, 1);
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_LIST_JUMP_DESK_WHILE_SELECTING_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-winlist-list-jump-desk-while-selecting-get", 0, "Get winlist (alt+tab) jump desk while selecting policy", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->winlist_list_jump_desk_while_selecting, E_IPC_OP_WINLIST_LIST_JUMP_DESK_WHILE_SELECTING_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_LIST_JUMP_DESK_WHILE_SELECTING_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(policy, HDL);
   printf("REPLY: POLICY=%d\n", policy);
   END_INT;
#endif
#undef HDL


/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_POS_ALIGN_X_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-winlist-pos-align-x-set", 1, "Set winlist position align for x axis (0.0-1.0)", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_DOUBLE(atof(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_DOUBLE(val, HDL);
   e_config->winlist_pos_align_x = val;
   E_CONFIG_LIMIT(e_config->winlist_pos_align_x, 0.0, 1.0);
   SAVE;
   END_DOUBLE;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_POS_ALIGN_X_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-winlist-pos-align-x-get", 0, "Get winlist position align for x axis", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_DOUBLE(e_config->winlist_pos_align_x, E_IPC_OP_WINLIST_POS_ALIGN_X_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_POS_ALIGN_X_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_DOUBLE(val, HDL);
   printf("REPLY: %3.3f\n", val);
   END_DOUBLE;
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_POS_ALIGN_Y_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-winlist-pos-align-y-set", 1, "Set winlist position align for y axis (0.0-1.0)", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_DOUBLE(atof(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_DOUBLE(val, HDL);
   e_config->winlist_pos_align_y = val;
   E_CONFIG_LIMIT(e_config->winlist_pos_align_y, 0.0, 1.0);
   SAVE;
   END_DOUBLE;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_POS_ALIGN_Y_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-winlist-pos-align-y-get", 0, "Get winlist position align for y axis", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_DOUBLE(e_config->winlist_pos_align_y, E_IPC_OP_WINLIST_POS_ALIGN_Y_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_POS_ALIGN_Y_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_DOUBLE(val, HDL);
   printf("REPLY: %3.3f\n", val);
   END_DOUBLE;
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_POS_SIZE_W_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-winlist-pos-size-w-set", 1, "Set winlist position size width (0.0-1.0)", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_DOUBLE(atof(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_DOUBLE(val, HDL);
   e_config->winlist_pos_size_w = val;
   E_CONFIG_LIMIT(e_config->winlist_pos_size_w, 0.0, 1.0);
   SAVE;
   END_DOUBLE;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_POS_SIZE_W_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-winlist-pos-size-w-get", 0, "Get winlist position size width", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_DOUBLE(e_config->winlist_pos_size_w, E_IPC_OP_WINLIST_POS_SIZE_W_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_POS_SIZE_W_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_DOUBLE(val, HDL);
   printf("REPLY: %3.3f\n", val);
   END_DOUBLE;
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_POS_SIZE_H_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-winlist-pos-size-h-set", 1, "Set winlist position size height (0.0-1.0)", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_DOUBLE(atof(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_DOUBLE(val, HDL);
   e_config->winlist_pos_size_h = val;
   E_CONFIG_LIMIT(e_config->winlist_pos_size_h, 0.0, 1.0);
   SAVE;
   END_DOUBLE;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_POS_SIZE_H_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-winlist-pos-size-h-get", 0, "Get winlist position size height", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_DOUBLE(e_config->winlist_pos_size_h, E_IPC_OP_WINLIST_POS_SIZE_H_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_POS_SIZE_H_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_DOUBLE(val, HDL);
   printf("REPLY: %3.3f\n", val);
   END_DOUBLE;
#endif
#undef HDL

/****************************************************************************/

#define HDL E_IPC_OP_WINLIST_POS_MIN_W_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-winlist-pos-min-w-set", 1, "Set winlist (alt+tab) minimum width", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(val, HDL);
   e_config->winlist_pos_min_w = val;
   E_CONFIG_LIMIT(e_config->winlist_pos_min_w, 0, 4000);
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_POS_MIN_W_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-winlist-pos-min-w-get", 0, "Get winlist (alt+tab) minimum width", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->winlist_pos_min_w, E_IPC_OP_WINLIST_POS_MIN_W_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_POS_MIN_W_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(val, HDL);
   printf("REPLY: %d\n", val);
   END_INT;
#endif
#undef HDL

/****************************************************************************/

#define HDL E_IPC_OP_WINLIST_POS_MIN_H_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-winlist-pos-min-h-set", 1, "Set winlist (alt+tab) minimum height", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(val, HDL);
   e_config->winlist_pos_min_h = val;
   E_CONFIG_LIMIT(e_config->winlist_pos_min_h, 0, 4000);
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_POS_MIN_H_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-winlist-pos-min-h-get", 0, "Get winlist (alt+tab) minimum height", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->winlist_pos_min_h, E_IPC_OP_WINLIST_POS_MIN_H_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_POS_MIN_H_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(val, HDL);
   printf("REPLY: %d\n", val);
   END_INT;
#endif
#undef HDL

/****************************************************************************/

#define HDL E_IPC_OP_WINLIST_POS_MAX_W_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-winlist-pos-max-w-set", 1, "Set winlist (alt+tab) maximum width", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(val, HDL);
   e_config->winlist_pos_max_w = val;
   E_CONFIG_LIMIT(e_config->winlist_pos_max_w, 8, 4000);
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_POS_MAX_W_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-winlist-pos-max-w-get", 0, "Get winlist (alt+tab) maximum width", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->winlist_pos_max_w, E_IPC_OP_WINLIST_POS_MAX_W_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_POS_MAX_W_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(val, HDL);
   printf("REPLY: %d\n", val);
   END_INT;
#endif
#undef HDL

/****************************************************************************/

#define HDL E_IPC_OP_WINLIST_POS_MAX_H_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-winlist-pos-max-h-set", 1, "Set winlist (alt+tab) maximum height", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(val, HDL);
   e_config->winlist_pos_max_h = val;
   E_CONFIG_LIMIT(e_config->winlist_pos_max_h, 8, 4000);
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_POS_MAX_H_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-winlist-pos-max-h-get", 0, "Get winlist (alt+tab) maximum height", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->winlist_pos_max_h, E_IPC_OP_WINLIST_POS_MAX_H_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINLIST_POS_MAX_H_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(val, HDL);
   printf("REPLY: %d\n", val);
   END_INT;
#endif
#undef HDL

/****************************************************************************/

#define HDL E_IPC_OP_KILL_IF_CLOSE_NOT_POSSIBLE_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-kill-if-close-not-possible-set", 1, "Set whether E should kill an application if it can not close", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(val, HDL);
   e_config->kill_if_close_not_possible = val;
   E_CONFIG_LIMIT(e_config->kill_if_close_not_possible, 0, 1);
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_KILL_IF_CLOSE_NOT_POSSIBLE_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-kill-if-close-not-possible-get", 0, "Get whether E should kill an application if it can not close", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->kill_if_close_not_possible, E_IPC_OP_KILL_IF_CLOSE_NOT_POSSIBLE_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_KILL_IF_CLOSE_NOT_POSSIBLE_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(val, HDL);
   printf("REPLY: KILL=%d\n", val);
   END_INT;
#endif
#undef HDL

/****************************************************************************/

#define HDL E_IPC_OP_KILL_PROCESS_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-kill-process-set", 1, "Set whether E should kill the process directly or through x", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(val, HDL);
   e_config->kill_process = val;
   E_CONFIG_LIMIT(e_config->kill_process, 0, 1);
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_KILL_PROCESS_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-kill-process-get", 0, "Get whether E should kill the process directly or through x", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->kill_process, E_IPC_OP_KILL_PROCESS_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_KILL_PROCESS_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(val, HDL);
   printf("REPLY: KILL=%d\n", val);
   END_INT;
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_KILL_TIMER_WAIT_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-kill-timer-wait-set", 1, "Set interval to wait before killing client (0.0-120.0)", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_DOUBLE(atof(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_DOUBLE(val, HDL);
   e_config->kill_timer_wait = val;
   E_CONFIG_LIMIT(e_config->kill_timer_wait, 0.0, 120.0);
   SAVE;
   END_DOUBLE;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_KILL_TIMER_WAIT_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-kill-timer-wait-get", 0, "Get interval to wait before killing client", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_DOUBLE(e_config->kill_timer_wait, E_IPC_OP_KILL_TIMER_WAIT_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_KILL_TIMER_WAIT_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_DOUBLE(val, HDL);
   printf("REPLY: %3.3f\n", val);
   END_DOUBLE;
#endif
#undef HDL


/****************************************************************************/

#define HDL E_IPC_OP_PING_CLIENTS_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-ping-clients-set", 1, "Set whether E should ping clients", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(val, HDL);
   e_config->ping_clients = val;
   E_CONFIG_LIMIT(e_config->ping_clients, 0, 1);
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_PING_CLIENTS_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-ping-clients-get", 0, "Get whether E should ping clients", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->ping_clients, E_IPC_OP_PING_CLIENTS_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_PING_CLIENTS_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(val, HDL);
   printf("REPLY: %d\n", val);
   END_INT;
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_PING_CLIENTS_WAIT_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-ping-clients-wait-set", 1, "Set client ping interval (0.0-120.0)", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_DOUBLE(atof(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_DOUBLE(val, HDL);
   e_config->ping_clients_wait = val;
   E_CONFIG_LIMIT(e_config->ping_clients_wait, 0.0, 120.0);
   SAVE;
   END_DOUBLE;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_PING_CLIENTS_WAIT_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-ping-clients-wait-get", 0, "Get client ping interval", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_DOUBLE(e_config->ping_clients_wait, E_IPC_OP_PING_CLIENTS_WAIT_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_PING_CLIENTS_WAIT_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_DOUBLE(val, HDL);
   printf("REPLY: %3.3f\n", val);
   END_DOUBLE;
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_TRANSITION_START_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-transition-start-set", 1, "Get the background transition used when E starts", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_STRING(params[0], HDL);
#elif (TYPE == E_WM_IN)
   STRING(s, HDL);
   E_FREE(e_config->transition_start);
   e_config->transition_start = strdup(s);
   SAVE;
   END_STRING(s);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL
     
/****************************************************************************/
#define HDL E_IPC_OP_TRANSITION_START_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-transition-start-get", 0, "Get the background transition used when E starts", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_STRING(e_config->transition_start, E_IPC_OP_TRANSITION_START_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL
     
/****************************************************************************/
#define HDL E_IPC_OP_TRANSITION_START_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   STRING(s, HDL);
   printf("REPLY: \"%s\"\n", s);
   END_STRING(s);
#elif (TYPE == E_LIB_IN)
#endif
#undef HDL
 
/****************************************************************************/
#define HDL E_IPC_OP_TRANSITION_DESK_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-transition-desk-set", 1, "Set the transition used when switching desktops", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_STRING(params[0], HDL);
#elif (TYPE == E_WM_IN)
   STRING(s, HDL);
   E_FREE(e_config->transition_desk);
   e_config->transition_desk = strdup(s);
   SAVE;
   END_STRING(s);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL
     
/****************************************************************************/
#define HDL E_IPC_OP_TRANSITION_DESK_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-transition-desk-get", 0, "Get the transition used when switching desktops", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_STRING(e_config->transition_desk, E_IPC_OP_TRANSITION_DESK_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL
     
/****************************************************************************/
#define HDL E_IPC_OP_TRANSITION_DESK_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   STRING(s, HDL);
   printf("REPLY: \"%s\"\n", s);
   END_STRING(s);
#elif (TYPE == E_LIB_IN)
#endif
#undef HDL
 
/****************************************************************************/
#define HDL E_IPC_OP_TRANSITION_CHANGE_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-transition-change-set", 1, "Set the transition used when changing backgrounds", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_STRING(params[0], HDL);
#elif (TYPE == E_WM_IN)
   STRING(s, HDL);
   E_FREE(e_config->transition_change);
   e_config->transition_change = strdup(s);
   SAVE;
   END_STRING(s);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL
     
/****************************************************************************/
#define HDL E_IPC_OP_TRANSITION_CHANGE_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-transition-change-get", 0, "Get the transition used when changing backgrounds", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_STRING(e_config->transition_change, E_IPC_OP_TRANSITION_CHANGE_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL
     
/****************************************************************************/
#define HDL E_IPC_OP_TRANSITION_CHANGE_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   STRING(s, HDL);
   printf("REPLY: \"%s\"\n", s);
   END_STRING(s);
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_FOCUS_SETTING_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-focus-setting-set", 1, "Set the focus setting policy (\"NONE\", \"NEW_WINDOW\", \"NEW_DIALOG\", \"NEW_DIALOG_IF_OWNER_FOCUSED\")", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT_START(HDL)
   int value = 0;
   if (!strcmp(params[0], "NONE")) value = E_FOCUS_NONE;
   else if (!strcmp(params[0], "NEW_WINDOW")) value = E_FOCUS_NEW_WINDOW;
   else if (!strcmp(params[0], "NEW_DIALOG")) value = E_FOCUS_NEW_DIALOG;
   else if (!strcmp(params[0], "NEW_DIALOG_IF_OWNER_FOCUSED")) value = E_FOCUS_NEW_DIALOG_IF_OWNER_FOCUSED;
   else
     {
	 printf("focus setting must be \"NONE\", \"NEW_WINDOW\", \"NEW_DIALOG\", \"NEW_DIALOG_IF_OWNER_FOCUSED\"\n");
	 exit(-1);
     }
   REQ_INT_END(value, HDL);
#elif (TYPE == E_WM_IN)
   START_INT(value, HDL);
   e_border_button_bindings_ungrab_all();
   e_config->focus_setting = value;
   E_CONFIG_LIMIT(e_config->focus_setting, 0, 3);
   e_border_button_bindings_grab_all();
   SAVE;
   END_INT
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL
     
/****************************************************************************/
#define HDL E_IPC_OP_FOCUS_SETTING_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-focus-setting-get", 0, "Get the focus setting policy", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->focus_setting, E_IPC_OP_FOCUS_SETTING_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL
     
/****************************************************************************/
#define HDL E_IPC_OP_FOCUS_SETTING_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(setting, HDL);
   if (setting == E_FOCUS_NONE)
     printf("REPLY: NONE\n");
   else if (setting == E_FOCUS_NEW_WINDOW)
     printf("REPLY: NEW_WINDOW\n");
   else if (setting == E_FOCUS_NEW_DIALOG)
     printf("REPLY: NEW_DIALOG\n");
   else if (setting == E_FOCUS_NEW_DIALOG_IF_OWNER_FOCUSED)
     printf("REPLY: NEW_DIALOG_IF_OWNER_FOCUSED\n");
   END_INT
#endif
#undef HDL
 
/****************************************************************************/
#define HDL E_IPC_OP_EXEC_ACTION
#if (TYPE == E_REMOTE_OPTIONS)
	OP("-exec-action", 2, "Executes an action given the name (OPT1) and a string of parameters (OPT2).", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
	REQ_2STRING(params[0], params[1], HDL);
#elif (TYPE == E_WM_IN)
	STRING2(actionName, paramList, e_2str, HDL);
	{
		Evas_List *m;
		E_Manager *man;
		E_Action
			*act
		;

		man = NULL;

		m = e_manager_list();
		if (m) {
			man = m->data;

			if (man) {
				act = e_action_find(actionName);

				if (act && act->func.go) {
					act->func.go(E_OBJECT(man), paramList);
				}
			}
		}
	}
	END_STRING2(e_2str)
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_THEME_LIST
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-theme-list", 0, "List themes and associated categories", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_STRING2_LIST(e_config->themes, E_Config_Theme, theme, v, HDL);
   v->str1 = theme->category;
   v->str2 = theme->file;
   END_SEND_STRING2_LIST(v, E_IPC_OP_THEME_LIST_REPLY);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL
    
/****************************************************************************/
#define HDL E_IPC_OP_THEME_LIST_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   STRING2_LIST(v, HDL);
   printf("REPLY: CATEGORY=\"%s\" FILE=\"%s\"\n", v->str1, v->str2);
   END_STRING2_LIST(v);
#endif
#undef HDL
     
/****************************************************************************/

#define HDL E_IPC_OP_THEME_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-theme-set", 2, "Set theme category (OPT1) and edje file (OPT2)", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_2STRING(params[0], params[1], HDL) 
#elif (TYPE == E_WM_IN)
   STRING2(category, file, e_2str, HDL);
   e_theme_config_set(category, file);
   SAVE;   
   END_STRING2(e_2str);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_THEME_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-theme-get", 1, "List the theme associated with the category OPT1", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_STRING(params[0], HDL);
#elif (TYPE == E_WM_IN)
   STRING(category, HDL);
   E_Config_Theme *ect;
   void *data;
   int bytes;
   
   ect = e_theme_config_get(category);
   if (ect == NULL)
     data = NULL;
   else
     data = e_ipc_codec_2str_enc(ect->category, ect->file, &bytes);
  	
   SEND_DATA(E_IPC_OP_THEME_GET_REPLY);

   END_STRING(category);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_THEME_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   STRING2(category, file, e_2str, HDL);
   printf("REPLY: DEFAULT CATEGORY=\"%s\" FILE=\"%s\"\n",
                    category, file); 
   END_STRING2(e_2str);
#elif (TYPE == E_LIB_IN)
   STRING2(category, file, e_2str, HDL);
   RESPONSE(r, E_Response_Theme_Get);
   r->file = strdup(file);
   r->category = strdup(category);
   END_RESPONSE(r, E_RESPONSE_THEME_GET);
   END_STRING2(e_2str);
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_THEME_REMOVE
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-theme-remove", 1, "Remove the theme category OPT1", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_STRING(params[0], HDL);
#elif (TYPE == E_WM_IN)
   STRING(category, HDL);
   e_theme_config_remove(category);
   SAVE;
   END_STRING(category);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/

#define HDL E_IPC_OP_MOVE_INFO_FOLLOWS_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-move-info-follows-set", 1, "Set whether the move dialog should follow the client window", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(val, HDL);
   e_config->move_info_follows = val;
   E_CONFIG_LIMIT(e_config->move_info_follows, 0, 1);
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_MOVE_INFO_FOLLOWS_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-move-info-follows-get", 0, "Get whether the move dialog should follow the client window", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->move_info_follows, E_IPC_OP_MOVE_INFO_FOLLOWS_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_MOVE_INFO_FOLLOWS_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(val, HDL);
   printf("REPLY: %d\n", val);
   END_INT;
#endif
#undef HDL

/****************************************************************************/

#define HDL E_IPC_OP_RESIZE_INFO_FOLLOWS_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-resize-info-follows-set", 1, "Set whether the resize dialog should follow the client window", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(val, HDL);
   e_config->resize_info_follows = val;
   E_CONFIG_LIMIT(e_config->resize_info_follows, 0, 1);
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_RESIZE_INFO_FOLLOWS_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-resize-info-follows-get", 0, "Set whether the resize dialog should follow the client window", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->resize_info_follows, E_IPC_OP_RESIZE_INFO_FOLLOWS_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_RESIZE_INFO_FOLLOWS_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(val, HDL);
   printf("REPLY: %d\n", val);
   END_INT;
#endif
#undef HDL

/****************************************************************************/

#define HDL E_IPC_OP_FOCUS_LAST_FOCUSED_PER_DESKTOP_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-focus-last-focused-per-desktop-set", 1, "Set whether E should remember focused windows when switching desks", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(val, HDL);
   e_config->focus_last_focused_per_desktop = val;
   E_CONFIG_LIMIT(e_config->focus_last_focused_per_desktop, 0, 1);
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_FOCUS_LAST_FOCUSED_PER_DESKTOP_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-focus-last-focused-per-desktop-get", 0, "Get whether E should remember focused windows when switching desks", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->focus_last_focused_per_desktop, E_IPC_OP_FOCUS_LAST_FOCUSED_PER_DESKTOP_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_FOCUS_LAST_FOCUSED_PER_DESKTOP_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(val, HDL);
   printf("REPLY: %d\n", val);
   END_INT;
#endif
#undef HDL

/****************************************************************************/

#define HDL E_IPC_OP_FOCUS_REVERT_ON_HIDE_OR_CLOSE_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-focus-revert-on-hide-or-close-set", 1, "Set whether E will focus the last focused window when you hide or close a focused window", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(val, HDL);
   e_config->focus_revert_on_hide_or_close = val;
   E_CONFIG_LIMIT(e_config->focus_revert_on_hide_or_close, 0, 1);
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_FOCUS_REVERT_ON_HIDE_OR_CLOSE_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-focus-revert-on-hide-or-close-get", 0, "Get whether E will focus the last focused window when you hide or close a focused window", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->focus_revert_on_hide_or_close, E_IPC_OP_FOCUS_REVERT_ON_HIDE_OR_CLOSE_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_FOCUS_REVERT_ON_HIDE_OR_CLOSE_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(val, HDL);
   printf("REPLY: %d\n", val);
   END_INT;
#endif
#undef HDL

/****************************************************************************/

/****************************************************************************/
#define HDL E_IPC_OP_PROFILE_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-default-profile-set", 1, "Set the default configuration profile to OPT1", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_STRING(params[0], HDL);
#elif (TYPE == E_WM_IN)
   STRING(s, HDL);
   e_config_save_flush();
   e_config_profile_set(s);
   e_config_profile_save();
   e_config_save_block_set(1);
   restart = 1;
   ecore_main_loop_quit();
   END_STRING(s);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL
     
/****************************************************************************/
#define HDL E_IPC_OP_PROFILE_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-default-profile-get", 0, "Get the default configuration profile", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_STRING(e_config_profile_get(), E_IPC_OP_PROFILE_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL
     
/****************************************************************************/
#define HDL E_IPC_OP_PROFILE_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   STRING(s, HDL);
   printf("REPLY: \"%s\"\n", s);
   END_STRING(s);
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_DESKTOP_NAME_ADD
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-desktop-name-add", 5, "Add a desktop name definition. OPT1 = container no. OPT2 = zone no. OPT3 = desk_x. OPT4 = desk_y. OPT5 = desktop name", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_4INT_2STRING_START(HDL);
   REQ_4INT_2STRING_END(atoi(params[0]), atoi(params[1]), atoi(params[2]), atoi(params[3]), params[4], "", HDL);
#elif (TYPE == E_WM_IN)
   INT4_STRING2(v, HDL);
   e_desk_name_add(v->val1, v->val2, v->val3, v->val4, v->str1);
   e_desk_name_update();
   SAVE;
   END_INT4_STRING2(v);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_DESKTOP_NAME_DEL
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-desktop-name-del", 4, "Delete a desktop name definition. OPT1 = container no. OPT2 = zone no. OPT3 = desk_x. OPT4 = desk_y.", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_4INT_2STRING_START(HDL);
   REQ_4INT_2STRING_END(atoi(params[0]), atoi(params[1]), atoi(params[2]), atoi(params[3]), "", "", HDL);
#elif (TYPE == E_WM_IN)
   INT4_STRING2(v, HDL);
   e_desk_name_del(v->val1, v->val2, v->val3, v->val4);
   e_desk_name_update();
   SAVE;
   END_INT4_STRING2(v);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_DESKTOP_NAME_LIST
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-desktop-name-list", 0, "List all current desktop name definitions", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT4_STRING2_LIST(e_config->desktop_names, E_Config_Desktop_Name, cfname, v, HDL);
   v->val1 = cfname->container;
   v->val2 = cfname->zone;
   v->val3 = cfname->desk_x;
   v->val4 = cfname->desk_y;
   v->str1 = cfname->name;
   v->str2 = "";
   END_SEND_INT4_STRING2_LIST(v, E_IPC_OP_DESKTOP_NAME_LIST_REPLY);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_DESKTOP_NAME_LIST_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   INT4_STRING2_LIST(v, HDL);
   printf("REPLY: BG CONTAINER=%i ZONE=%i DESK_X=%i DESK_Y=%i NAME=\"%s\"\n",
	  v->val1, v->val2, v->val3, v->val4, v->str1);
   END_INT4_STRING2_LIST(v);
#endif
#undef HDL

/****************************************************************************/

#define HDL E_IPC_OP_CURSOR_SIZE_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-cursor-size-set", 1, "Set the E cursor size", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(val, HDL);
   e_config->cursor_size = val;
   E_CONFIG_LIMIT(e_config->cursor_size, 0, 1024);
   e_pointers_size_set(e_config->cursor_size);
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_CURSOR_SIZE_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-cursor-size-get", 0, "Get the E cursor size", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->cursor_size, E_IPC_OP_CURSOR_SIZE_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_CURSOR_SIZE_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(val, HDL);
   printf("REPLY: %d\n", val);
   END_INT;
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_MENU_AUTOSCROLL_MARGIN_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-menu-autoscroll-margin-set", 1, "Set the distance from the edge of the screen the menu will autoscroll to", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(value, HDL);
   e_config->menu_autoscroll_margin = value;
   E_CONFIG_LIMIT(e_config->menu_autoscroll_margin, 0, 50);
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_MENU_AUTOSCROLL_MARGIN_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-menu-autoscroll-margin-get", 0, "Get the distance from the edge of the screen the menu will autoscroll to", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL)
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->menu_autoscroll_margin, E_IPC_OP_MENU_AUTOSCROLL_MARGIN_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_MENU_AUTOSCROLL_MARGIN_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(val, HDL)
		 printf("REPLY: %i\n", val);
		 END_INT;
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_MENU_AUTOSCROLL_CURSOR_MARGIN_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-menu-autoscroll-cursor-margin-set", 1, "Set the distance from the edge of the screen the cursor needs to be to start menu autoscrolling", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(value, HDL);
   e_config->menu_autoscroll_cursor_margin = value;
   E_CONFIG_LIMIT(e_config->menu_autoscroll_cursor_margin, 0, 50);
	 //   e_zone_update_flip_all();
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_MENU_AUTOSCROLL_CURSOR_MARGIN_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-menu-autoscroll-cursor-margin-get", 0, "Get the distance from the edge of the screen the cursor needs to be to start menu autoscrolling", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL)
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->menu_autoscroll_cursor_margin, E_IPC_OP_MENU_AUTOSCROLL_CURSOR_MARGIN_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_MENU_AUTOSCROLL_CURSOR_MARGIN_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(val, HDL)
		 printf("REPLY: %i\n", val);
		 END_INT;
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_TRANSIENT_MOVE_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-transient-move-set", 1, "Set if transients should move with it's parent", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(val, HDL);
   e_config->transient.move = val;
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_TRANSIENT_MOVE_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-transient-move-get", 0, "Get if transients should move with it's parent", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->transient.move, E_IPC_OP_TRANSIENT_MOVE_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_TRANSIENT_MOVE_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(val, HDL);
   printf("REPLY: %d\n", val);
   END_INT;
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_TRANSIENT_RESIZE_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-transient-resize-set", 1, "Set if transients should move when it's parent resizes", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(val, HDL);
   e_config->transient.resize = val;
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_TRANSIENT_RESIZE_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-transient-resize-get", 0, "Get if transients should move when it's parent resizes", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->transient.resize, E_IPC_OP_TRANSIENT_RESIZE_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_TRANSIENT_RESIZE_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(val, HDL);
   printf("REPLY: %d\n", val);
   END_INT;
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_TRANSIENT_RAISE_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-transient-raise-set", 1, "Set if transients should raise with it's parent", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(val, HDL);
   e_config->transient.raise = val;
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_TRANSIENT_RAISE_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-transient-raise-get", 0, "Get if transients should raise with it's parent", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->transient.raise, E_IPC_OP_TRANSIENT_RAISE_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_TRANSIENT_RAISE_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(val, HDL);
   printf("REPLY: %d\n", val);
   END_INT;
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_TRANSIENT_LOWER_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-transient-lower-set", 1, "Set if transients should lower with it's parent", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(val, HDL);
   e_config->transient.lower = val;
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_TRANSIENT_LOWER_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-transient-lower-get", 0, "Get if transients should lower with it's parent", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->transient.lower, E_IPC_OP_TRANSIENT_LOWER_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_TRANSIENT_LOWER_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(val, HDL);
   printf("REPLY: %d\n", val);
   END_INT;
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_TRANSIENT_LAYER_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-transient-layer-set", 1, "Set if transients should change layer with it's parent", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(val, HDL);
   e_config->transient.layer = val;
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_TRANSIENT_LAYER_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-transient-layer-get", 0, "Get if transients should change layer with it's parent", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->transient.layer, E_IPC_OP_TRANSIENT_LAYER_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_TRANSIENT_LAYER_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(val, HDL);
   printf("REPLY: %d\n", val);
   END_INT;
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_TRANSIENT_DESKTOP_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-transient-desktop-set", 1, "Set if transients should change desktop with it's parent", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(val, HDL);
   e_config->transient.desktop = val;
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_TRANSIENT_DESKTOP_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-transient-desktop-get", 0, "Get if transients should change desktop with it's parent", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->transient.desktop, E_IPC_OP_TRANSIENT_DESKTOP_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_TRANSIENT_DESKTOP_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(val, HDL);
   printf("REPLY: %d\n", val);
   END_INT;
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_TRANSIENT_ICONIFY_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-transient-iconify-set", 1, "Set if transients should iconify with it's parent", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(val, HDL);
   e_config->transient.iconify = val;
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_TRANSIENT_ICONIFY_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-transient-iconify-get", 0, "Get if transients should iconify with it's parent", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->transient.iconify, E_IPC_OP_TRANSIENT_ICONIFY_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_TRANSIENT_ICONIFY_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(val, HDL);
   printf("REPLY: %d\n", val);
   END_INT;
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_MODAL_WINDOWS_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-modal-windows-set", 1, "Set if enlightenment should honour modal windows", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT(atoi(params[0]), HDL);
#elif (TYPE == E_WM_IN)
   START_INT(val, HDL);
   e_config->modal_windows = val;
   SAVE;
   END_INT;
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_MODAL_WINDOWS_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-modal-windows-get", 0, "Get if enlightenment should honour modal windows", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->modal_windows, E_IPC_OP_MODAL_WINDOWS_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_MODAL_WINDOWS_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(val, HDL);
   printf("REPLY: %d\n", val);
   END_INT;
#endif
#undef HDL


/****************************************************************************/
#define HDL E_IPC_OP_WINDOW_PLACEMENT_POLICY_SET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-window-placement-policy-set", 1, "Set the window placement policy. OPT1 = SMART or CURSOR", 0, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_INT_START(HDL)
   int value = 0;
   if (!strcmp(params[0], "SMART")) value = E_WINDOW_PLACEMENT_SMART;
   else if (!strcmp(params[0], "CURSOR")) value = E_WINDOW_PLACEMENT_CURSOR;
   else
     {
	 printf("window placement policy must be SMART or CURSOR\n");
	 exit(-1);
     }
   REQ_INT_END(value, HDL);
#elif (TYPE == E_WM_IN)
   START_INT(value, HDL);
   e_config->window_placement_policy = value;
   E_CONFIG_LIMIT(e_config->window_placement_policy, 0, 1);
   SAVE;
   END_INT
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINDOW_PLACEMENT_POLICY_GET
#if (TYPE == E_REMOTE_OPTIONS)
   OP("-window-placement-policy-get", 0, "Get window placement policy", 1, HDL)
#elif (TYPE == E_REMOTE_OUT)
   REQ_NULL(HDL);
#elif (TYPE == E_WM_IN)
   SEND_INT(e_config->window_placement_policy, E_IPC_OP_WINDOW_PLACEMENT_POLICY_GET_REPLY, HDL);
#elif (TYPE == E_REMOTE_IN)
#endif
#undef HDL

/****************************************************************************/
#define HDL E_IPC_OP_WINDOW_PLACEMENT_POLICY_GET_REPLY
#if (TYPE == E_REMOTE_OPTIONS)
#elif (TYPE == E_REMOTE_OUT)
#elif (TYPE == E_WM_IN)
#elif (TYPE == E_REMOTE_IN)
   START_INT(policy, HDL);
   if (policy == E_WINDOW_PLACEMENT_SMART)
     printf("REPLY: SMART\n");
   else if (policy == E_WINDOW_PLACEMENT_CURSOR)
     printf("REPLY: CURSOR\n");
   END_INT
#endif
#undef HDL

