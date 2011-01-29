#include <stdlib.h>

#include <Eina.h>

/*
 * FIXME:
 * - in ecore_xcb_cookie_cache, should provide better error management
 *   when memory allocation fails
 * - Use an array instead of a list
 * - Is ecore_xcb_reply_free really needed ?
 *   _ecore_xcb_reply_cache frees the current reply and
 *   _ecore_x_reply_shutdown frees the last reply to free.
 *   I keep it in case it is need for memory improvements,
 *   but its code is commented.
 */

static Eina_List *_ecore_xcb_cookies = NULL;
static void *_ecore_xcb_reply = NULL;

typedef struct _Ecore_Xcb_Data   Ecore_Xcb_Data;

struct _Ecore_Xcb_Data
{
   unsigned int cookie;
};

int
_ecore_x_reply_init ()
{
   return 1;
} /* _ecore_x_reply_init */

void
_ecore_x_reply_shutdown ()
{
   Ecore_Xcb_Data *data;

   if (_ecore_xcb_reply)
      free(_ecore_xcb_reply);

   if (!_ecore_xcb_cookies)
      return;

   EINA_LIST_FREE(_ecore_xcb_cookies, data)
   free(data);
} /* _ecore_x_reply_shutdown */

void
_ecore_xcb_cookie_cache (unsigned int cookie)
{
   Ecore_Xcb_Data *data;

   if (!_ecore_xcb_cookies)
      return;

   data = (Ecore_Xcb_Data *)malloc(sizeof(Ecore_Xcb_Data));
   if (!data)
      return;

   data->cookie = cookie;

   _ecore_xcb_cookies = eina_list_append(_ecore_xcb_cookies, data);
   if (!eina_list_data_find(_ecore_xcb_cookies, data))
     {
        free(data);
        return;
     }
} /* _ecore_xcb_cookie_cache */

unsigned int
_ecore_xcb_cookie_get (void)
{
   Ecore_Xcb_Data *data;
   unsigned int cookie;

   if (!_ecore_xcb_cookies)
      return 0;

   data = eina_list_data_get(_ecore_xcb_cookies);
   if (!data)
      return 0;

   _ecore_xcb_cookies = eina_list_remove_list(_ecore_xcb_cookies, _ecore_xcb_cookies);
   cookie = data->cookie;
   free(data);

   return cookie;
} /* _ecore_xcb_cookie_get */

void
_ecore_xcb_reply_cache (void *reply)
{
   if (_ecore_xcb_reply)
      free(_ecore_xcb_reply);

   _ecore_xcb_reply = reply;
} /* _ecore_xcb_reply_cache */

void *
_ecore_xcb_reply_get (void)
{
   return _ecore_xcb_reply;
} /* _ecore_xcb_reply_get */

EAPI void
ecore_xcb_reply_free()
{
/*   if (_ecore_xcb_reply) */
/*     { */
/*        free(_ecore_xcb_reply); */
/*        _ecore_xcb_reply = NULL; */
/*     } */
} /* ecore_xcb_reply_free */

