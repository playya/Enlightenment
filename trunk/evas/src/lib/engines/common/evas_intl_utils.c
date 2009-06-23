/* Authors:
 *	Tom Hacohen (tom@stsob.com)
 */

#include <string.h>
#include <stdlib.h>

#include "evas_common.h"
#include "evas_intl_utils.h"

#ifdef USE_FRIBIDI
#include <fribidi/fribidi.h>

#define UTF8_BYTES_PER_CHAR 4

/* FIXME: fribidi_utf8_to_unicode should use char len and not byte len!*/
char *
evas_intl_utf8_to_visual(const char *text, int *ret_len, FriBidiCharType *direction, FriBidiLevel **embedding_level_list)
{
   FriBidiChar *unicode_in, *unicode_out;
   char *text_out;
   size_t len;
   size_t byte_len;

   if (!text)
     return NULL;

   len = evas_string_char_len_get(text);

   /* if there's nothing to do, return text
    * one char draws are quite common */
   if (len <= 1)
     {
	*ret_len = len;
	*embedding_level_list = NULL;
	return strdup(text);
     }

   byte_len = strlen(text); /* we need the actual number of bytes, not number of chars */

   unicode_in = (FriBidiChar *)alloca(sizeof(FriBidiChar) * (len + 1));
   if (!unicode_in)
     {
	len = -1;
	goto error1;
     }

   len = fribidi_utf8_to_unicode(text, byte_len, unicode_in);

   unicode_out = (FriBidiChar *)alloca(sizeof(FriBidiChar) * (len + 1));
   if (!unicode_out)
     {
	len = -2;
	goto error1;
     }

   *embedding_level_list = (FriBidiLevel *)malloc(sizeof(FriBidiLevel) * len);
   if (!*embedding_level_list)
     {
	len = -3;
	goto error2;
     }

#ifdef ARABIC_SUPPORT
   /* fix arabic context */
   evas_intl_arabic_to_context(unicode_in);
#endif
   if (!fribidi_log2vis(unicode_in, len, direction,
	    unicode_out, NULL, NULL, *embedding_level_list))
     {
	len = -4;
	goto error2;
     }

   text_out = malloc(UTF8_BYTES_PER_CHAR * len + 1);
   if (!text_out)
     {
	len = -5;
	goto error2;
     }

   fribidi_unicode_to_utf8(unicode_out, len, text_out);

   *ret_len = len;
   return text_out;

   /* ERROR HANDLING */
error2:
   free(*embedding_level_list);
error1:

   *ret_len = len;
   return NULL;
}

int
evas_intl_is_rtl_char(FriBidiLevel *embedded_level_list, FriBidiStrIndex i)
{
   if(embedded_level_list || i < 0)
     return 0;
   return FRIBIDI_IS_RTL(embedded_level_list[i]);
}
#endif
