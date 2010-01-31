/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
/* Leave the OpenBSD version below so we can track upstream fixes */
/*      $OpenBSD: strlcpy.c,v 1.11 2006/05/05 15:27:38 millert Exp $        */

/*
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "eina_private.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

/*
 * Internal helper function used by eina_str_has_suffix() and
 * eina_str_has_extension()
 */
static Eina_Bool
eina_str_has_suffix_helper(const char *str,
			   const char *suffix,
			   int (*cmp)(const char *, const char *))
{
   size_t str_len;
   size_t suffix_len;

   str_len = strlen(str);
   suffix_len = strlen(suffix);
   if (suffix_len > str_len)
     return EINA_FALSE;

   return cmp(str + str_len - suffix_len, suffix) == 0;
}

/**
 * @endcond
 */

/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/


/**
 * @brief Copy a c-string to another.
 *
 * @param dst The destination string.
 * @param src The source string.
 * @param siz The size of the destination string.
 * @return The length of the source string.
 *
 * This function copies up to @p siz - 1 characters from the
 * NUL-terminated string @p src to @p dst, NUL-terminating the result
 * (unless @p siz is equal to 0). The returned value is the length of
 * @p src. If the returned value is greater than @p siz, truncation
 * occured.
 */
EAPI size_t
eina_strlcpy(char *dst, const char *src, size_t siz)
{
#ifdef HAVE_STRLCPY
   return strlcpy(dst, src, siz);
#else
   char *d = dst;
   const char *s = src;
   size_t n = siz;

   /* Copy as many bytes as will fit */
   if (n != 0)
     {
	while (--n != 0)
	  {
	     if ((*d++ = *s++) == '\0')
	       break;
	  }
     }

   /* Not enough room in dst, add NUL and traverse rest of src */
   if (n == 0)
     {
	if (siz != 0)
	  *d = '\0';                /* NUL-terminate dst */
	while (*s++)
	  ;
     }

   return(s - src - 1);        /* count does not include NUL */
#endif
}

/**
 * @brief Append a c-string.
 *
 * @param dst The destination string.
 * @param src The source string.
 * @param siz The size of the destination string.
 * @return The length of the source string plus MIN(siz, strlen(initial dst))
 *
 * This function appends @p src to @p dst of size @p siz (unlike
 * strncat, @p siz is the full size of @p dst, not space left).  At
 * most @p siz - 1 characters will be copied.  Always NUL terminates
 * (unless @p siz <= strlen(dst)). This function returns strlen(src) +
 * MIN(siz, strlen(initial dst)). If the returned value is greater or
 * equal than @p siz, truncation occurred.
 */
EAPI size_t
eina_strlcat(char *dst, const char *src, size_t siz)
{
   char *d = dst;
   const char *s = src;
   size_t n = siz;
   size_t dlen;

   /* Find the end of dst and adjust bytes left but don't go past end */
   while (n-- != 0 && *d != '\0')
     d++;
   dlen = d - dst;
   n = siz - dlen;

   if (n == 0)
     return(dlen + strlen(s));
   while (*s != '\0') {
	if (n != 1) {
	     *d++ = *s;
	     n--;
	}
	s++;
   }
   *d = '\0';

   return(dlen + (s - src));        /* count does not include NUL */
}

/**
 * @brief Check if the given string has the given prefix.
 *
 * @param str The string to work with.
 * @param prefix The prefix to check for.
 * @return #EINA_TRUE if the string has the given prefix, #EINA_FALSE otherwise.
 *
 * This function returns #EINA_TRUE if @p str has the prefix
 * @p prefix, #EINA_FALSE otherwise. If the length of @p prefix is
 * greater than @p str, #EINA_FALSE is returned.
 */
EAPI Eina_Bool
eina_str_has_prefix(const char *str, const char *prefix)
{
   size_t str_len;
   size_t prefix_len;

   str_len = strlen(str);
   prefix_len = strlen(prefix);
   if (prefix_len > str_len)
     return EINA_FALSE;

   return (strncmp(str, prefix, prefix_len) == 0);
}

/**
 * @brief Check if the given string has the given suffix.
 *
 * @param str The string to work with.
 * @param suffix The suffix to check for.
 * @return #EINA_TRUE if the string has the given suffix, #EINA_FALSE otherwise.
 *
 * This function returns #EINA_TRUE if @p str has the suffix
 * @p suffix, #EINA_FALSE otherwise. If the length of @p suffix is
 * greater than @p str, #EINA_FALSE is returned.
 */
/**
 * @param str the string to work with
 * @param suffix the suffix to check for
 * @return true if str has the given suffix
 * @brief checks if the string has the given suffix
 */
EAPI Eina_Bool
eina_str_has_suffix(const char *str, const char *suffix)
{
   return eina_str_has_suffix_helper(str, suffix, strcmp);
}

/**
 * @brief Check if the given string has the given suffix.
 *
 * @param str The string to work with.
 * @param ext The  extension to check for.
 * @return #EINA_TRUE if the string has the given extension, #EINA_FALSE otherwise.
 *
 * This function does the same like eina_str_has_suffix(), but with a
 * case insensitive compare.
 */
EAPI Eina_Bool
eina_str_has_extension(const char *str, const char *ext)
{
   return eina_str_has_suffix_helper(str, ext, strcasecmp);
}

/**
 * @brief Split a string using a delimiter.
 *
 * @param str The string to split.
 * @param delim The string which specifies the places at which to split the string.
 * @param max_tokens The maximum number of strings to split string into.
 * @return A newly-allocated NULL-terminated array of strings.
 *
 * This functin splits @p str into a maximum of @p max_tokens pieces,
 * using the given delimiter @p delim. @p delim is not included in any
 * of the resulting strings, unless @p max_tokens is reached. If
 * @p max_tokens is less than @c 1, the string is splitted completely. If
 * @p max_tokens is reached, the last string in the returned string
 * array contains the remainder of string. The returned value is a
 * newly allocated NUL-terminated array of string. To free it, free
 * the first element of the array and the array itself.
 */
EAPI char **
eina_str_split(const char *str, const char *delim, int max_tokens)
{
   char *s, *sep, **str_array;
   size_t len, dlen;
   int i;

   if (*delim == '\0')
     return NULL;

   max_tokens = ((max_tokens <= 0) ? (INT_MAX) : (max_tokens - 1));
   len = strlen(str);
   dlen = strlen(delim);
   s = strdup(str);
   str_array = malloc(sizeof(char *) * (len + 1));
   for (i = 0; (i < max_tokens) && (sep = strstr(s, delim)); i++)
     {
	str_array[i] = s;
	s = sep + dlen;
	*sep = 0;
     }

   str_array[i++] = s;
   str_array = realloc(str_array, sizeof(char *) * (i + 1));
   str_array[i] = NULL;

   return str_array;
}

/**
 * @brief Join two strings of known length.
 *
 * @param dst The buffer to store the result.
 * @param size Size (in byte) of the buffer.
 * @param sep The separator character to use.
 * @param a First string to use, before @p sep.
 * @param a_len length of @p a.
 * @param b Second string to use, after @p sep.
 * @param b_len length of @p b.
 * @return The number of characters printed.
 *
 * This function joins the strings @p a and @p b (in that order) and
 * separate them with @p sep. The result is stored in the buffer
 * @p dst and at most @p size - 1 characters will be written and the
 * string is NULL-terminated. @p a_len is the length of @p a (not
 * including '\0') and @p b_len is the length of @p b (not including
 * '\0'). This function returns the number of characters printed (not
 * including the trailing '\0' used to end output to strings). Just
 * like snprintf(), it will not write more than @p size bytes, thus a
 * returned value of @p size or more means that the output was
 * truncated.
 *
 * @see eina_str_join()
 * @see eina_str_join_static()
 */
EAPI size_t
eina_str_join_len(char *dst, size_t size, char sep, const char *a, size_t a_len, const char *b, size_t b_len)
{
   size_t ret = a_len + b_len + 1;
   size_t off;

   if (size < 1) return ret;

   if (size <= a_len)
     {
	memcpy(dst, a, size - 1);
	dst[size - 1] = '\0';
	return ret;
     }

   memcpy(dst, a, a_len);
   off = a_len;

   if (size <= off + 1)
     {
	dst[size - 1] = '\0';
	return ret;
     }

   dst[off] = sep;
   off++;

   if (size <= off + b_len + 1)
     {
	memcpy(dst + off, b, size - off - 1);
	dst[size - 1] = '\0';
	return ret;
     }

   memcpy(dst + off, b, b_len);
   dst[off + b_len] = '\0';
   return ret;
}
