/*
 * Copyright 2010, 2011 Mike Blumenkrantz <mike@zentific.com>
 */

#ifndef ESQL_PRIV_H
#define ESQL_PRIV_H

#include <Esskyuehl.h>
#include <Ecore.h>
#include <time.h>

#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#elif defined __GNUC__
#define alloca __builtin_alloca
#elif defined _AIX
#define alloca __alloca
#else
#include <stddef.h>
void *alloca (size_t);
#endif

extern int esql_log_dom;

#define DBG(...)  EINA_LOG_DOM_DBG(esql_log_dom, __VA_ARGS__)
#define INFO(...) EINA_LOG_DOM_INFO(esql_log_dom, __VA_ARGS__)
#define WARN(...) EINA_LOG_DOM_WARN(esql_log_dom, __VA_ARGS__)
#define ERR(...)  EINA_LOG_DOM_ERR(esql_log_dom, __VA_ARGS__)
#define CRI(...)  EINA_LOG_DOM_CRIT(esql_log_dom, __VA_ARGS__)

typedef enum
{
   ESQL_CONNECT_TYPE_NONE,
   ESQL_CONNECT_TYPE_INIT,
   ESQL_CONNECT_TYPE_DATABASE_SET,
   ESQL_CONNECT_TYPE_QUERY
} Esql_Connect_Type;

typedef const char *(*Esql_Error_Cb)(Esql *);
typedef void (*Esql_Cb)(Esql *);
typedef Ecore_Fd_Handler_Flags (*Esql_Connect_Cb)(Esql *);
typedef void (*Esql_Setup_Cb)(Esql *, const char *, const char *, const char *);
typedef void (*Esql_Set_Cb)(Esql *, const char *);
typedef int (*Esql_Fd_Cb)(Esql *);
typedef char *(*Esql_Escape_Cb)(Esql *, const char *, va_list);
typedef void (*Esql_Res_Cb)(Esql_Res *);
typedef Esql_Row *(*Esql_Row_Cb)(Esql_Res *);

typedef const char *(*Esql_Row_Col_Name_Cb)(Esql_Row *);

struct Esql
{
   struct
   {
      void *db; /* db object pointer */
      Esql_Error_Cb error_get;
      Esql_Connect_Cb connect;
      Esql_Cb disconnect;
      Esql_Cb free;
      Esql_Setup_Cb setup;
      Esql_Set_Cb database_set;
      Esql_Set_Cb query;
      Esql_Connect_Cb database_send;
      Esql_Connect_Cb io;
      Esql_Fd_Cb fd_get;
      Esql_Escape_Cb escape;
      Esql_Res_Cb res;
   } backend;

   const char *database;
   Ecore_Fd_Handler *fdh;
   Eina_Bool connected : 1;
   Esql_Type type;
   Esql_Connect_Type current;
   Eina_List *backend_set_funcs; /* Esql_Set_Cb */
   Eina_List *backend_set_params; /* char * */
   void *data;
};

struct Esql_Res
{
   Esql *e; /* parent object */
   const char *error;

   Eina_Inlist *rows;
   int row_count;
   int num_cols;
   long long int affected;
   long long int id;

   struct
   {
      void *res; /* backend res type */
   } backend;
};

struct Esql_Row
{
   EINA_INLIST;

   Esql_Res *res;

   Eina_Inlist *cells;
   int num_cells;
   struct
   {
      void *row;
   } backend;
};

typedef struct Esql_Row_Iterator
{
   Eina_Iterator iterator;

   const Esql_Row *head;
   const Esql_Row *current;
} Esql_Row_Iterator;

void esql_mysac_init(Esql *e);

void esql_res_free(Esql_Res *res);
void esql_row_free(Esql_Row *r);

Eina_Bool esql_connect_handler(Esql *e, Ecore_Fd_Handler *fdh);

char *esql_query_escape(Eina_Bool backslashes, size_t *len, const char *fmt, va_list args);
char *esql_string_escape(Eina_Bool backslashes, const char *s);
#endif
