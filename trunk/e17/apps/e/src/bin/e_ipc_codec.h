/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifdef E_TYPEDEFS

typedef struct _E_Ipc_Int              E_Ipc_Int;
typedef struct _E_Ipc_Double           E_Ipc_Double;
typedef struct _E_Ipc_2Int             E_Ipc_2Int;
typedef struct _E_Ipc_List             E_Ipc_List;
typedef struct _E_Ipc_Str              E_Ipc_Str;
typedef struct _E_Ipc_2Str             E_Ipc_2Str;
typedef struct _E_Ipc_Str_Int          E_Ipc_Str_Int;
typedef struct _E_Ipc_Str_Int_List     E_Ipc_Str_Int_List;
typedef struct _E_Ipc_2Str_Int         E_Ipc_2Str_Int;
typedef struct _E_Ipc_2Str_Int_List    E_Ipc_2Str_Int_List;
typedef struct _E_Ipc_4Int_2Str	       E_Ipc_4Int_2Str;
typedef struct _E_Ipc_3Int_3Str	       E_Ipc_3Int_3Str;

#else
#ifndef E_IPC_CODEC_H
#define E_IPC_CODEC_H

struct _E_Ipc_Int
{
   int val;
};

struct _E_Ipc_Double
{
   double val;
};

struct _E_Ipc_2Int
{
   int val1, val2;
};

struct _E_Ipc_List
{
   Evas_List *list;
};

struct _E_Ipc_Str
{
   char *str;
};

struct _E_Ipc_2Str
{
   char *str1, *str2;
};

struct _E_Ipc_Str_Int
{
   char *str;
   int   val;
};

struct _E_Ipc_2Str_Int
{
   char *str1, *str2;
   int   val;
};

struct _E_Ipc_4Int_2Str
{
   int	 val1, val2, val3, val4;
   char *str1, *str2;
};

struct _E_Ipc_3Int_3Str
{
   int   val1, val2, val3;
   char *str1, *str2, *str3;
};

EAPI int      e_ipc_codec_init(void);
EAPI void     e_ipc_codec_shutdown(void);

EAPI int      e_ipc_codec_int_dec(char *data, int bytes, int *dest);
EAPI void    *e_ipc_codec_int_enc(int val, int *size_ret);
EAPI int      e_ipc_codec_double_dec(char *data, int bytes, double *dest);
EAPI void    *e_ipc_codec_double_enc(double val, int *size_ret);
EAPI int      e_ipc_codec_2int_dec(char *data, int bytes, int *dest, int *dest2x);
EAPI void    *e_ipc_codec_2int_enc(int val1, int val2, int *size_ret);

EAPI int      e_ipc_codec_str_dec(char *data, int bytes, char **dest);
EAPI void    *e_ipc_codec_str_enc(char *str, int *size_ret);
EAPI int      e_ipc_codec_str_list_dec(char *data, int bytes, Evas_List **dest);
EAPI void    *e_ipc_codec_str_list_enc(Evas_List *list, int *size_ret);

EAPI int      e_ipc_codec_2str_dec(char *data, int bytes, E_Ipc_2Str **dest);
EAPI void    *e_ipc_codec_2str_enc(char *str1, char *str2, int *size_ret);
EAPI int      e_ipc_codec_2str_list_dec(char *data, int bytes, Evas_List **dest);
EAPI void    *e_ipc_codec_2str_list_enc(Evas_List *list, int *size_ret);

EAPI int      e_ipc_codec_str_int_dec(char *data, int bytes, E_Ipc_Str_Int **dest);
EAPI void    *e_ipc_codec_str_int_enc(char *str, int val, int *size_ret);
EAPI int      e_ipc_codec_str_int_list_dec(char *data, int bytes, Evas_List **dest);
EAPI void    *e_ipc_codec_str_int_list_enc(Evas_List *list, int *size_ret);

EAPI int      e_ipc_codec_2str_int_dec(char *data, int bytes, E_Ipc_2Str_Int **dest);
EAPI void    *e_ipc_codec_2str_int_enc(char *str1, char *str2, int val, int *size_ret);
EAPI int      e_ipc_codec_2str_int_list_dec(char *data, int bytes, Evas_List **dest);
EAPI void    *e_ipc_codec_2str_int_list_enc(Evas_List *list, int *size_ret);

EAPI int      e_ipc_codec_4int_2str_dec(char *data, int bytes, E_Ipc_4Int_2Str **dest);
EAPI void    *e_ipc_codec_4int_2str_enc(int val1, int val2, int val3, int val4, char *str1, char *str2, int *size_ret);
EAPI int      e_ipc_codec_4int_2str_list_dec(char *data, int bytes, Evas_List **dest);
EAPI void    *e_ipc_codec_4int_2str_list_enc(Evas_List *list, int *size_ret);   

EAPI int      e_ipc_codec_3int_3str_dec(char *data, int bytes, E_Ipc_3Int_3Str **dest);
EAPI void    *e_ipc_codec_3int_3str_enc(int val1, int val2, int val3, char *str1, char *str2, char *str3, int *size_ret);
EAPI int      e_ipc_codec_3int_3str_list_dec(char *data, int bytes, Evas_List **dest);
EAPI void    *e_ipc_codec_3int_3str_list_enc(Evas_List *list, int *size_ret);   
 
#endif
#endif
