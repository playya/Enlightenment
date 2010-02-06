/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "eina_private.h"
#include "eina_str.h"
#include "eina_strbuf.h"
#include "eina_magic.h"

#include <stdlib.h>
#include <string.h>

static const char EINA_MAGIC_STRBUF_STR[] = "Eina Strbuf";

#define EINA_MAGIC_CHECK_STRBUF(d, ...)				\
   do {								\
	if (!EINA_MAGIC_CHECK((d), EINA_MAGIC_STRBUF))		\
	  {							\
	     EINA_MAGIC_FAIL((d), EINA_MAGIC_STRBUF);		\
	     return __VA_ARGS__;				\
	  }							\
   } while (0);

#define EINA_STRBUF_INIT_SIZE 32
#define EINA_STRBUF_INIT_STEP 32
#define EINA_STRBUF_MAX_STEP 4096

struct _Eina_Strbuf
{
   char *buf;
   size_t len;
   size_t size;
   size_t step;

   EINA_MAGIC
};

static void _eina_strbuf_init(Eina_Strbuf *buf);
static int _eina_strbuf_resize(Eina_Strbuf *buf, size_t size);

Eina_Bool
eina_strbuf_init(void)
{
   eina_magic_string_static_set(EINA_MAGIC_STRBUF, EINA_MAGIC_STRBUF_STR);
   return EINA_TRUE;
}

Eina_Bool
eina_strbuf_shutdown(void)
{
   return EINA_TRUE;
}

/**
 * Create a new string buffer
 */
EAPI Eina_Strbuf *
eina_strbuf_new(void)
{
   Eina_Strbuf *buf;

   buf = malloc(sizeof(Eina_Strbuf));
   if (!buf) return NULL;
   EINA_MAGIC_SET(buf, EINA_MAGIC_STRBUF);

   _eina_strbuf_init(buf);

   return buf;
}

/**
 * Free a string buffer
 * @param buf the buffer to free
 */
EAPI void
eina_strbuf_free(Eina_Strbuf *buf)
{
   EINA_MAGIC_CHECK_STRBUF(buf);
   free(buf->buf);
   free(buf);
}

/**
 * Append a string to a buffer, reallocating as necessary.
 * @param buf the Eina_Strbuf to append to
 * @param str the string to append
 */
EAPI void
eina_strbuf_append(Eina_Strbuf *buf, const char *str)
{
   size_t len;
   EINA_MAGIC_CHECK_STRBUF(buf);

   len = strlen(str);
   _eina_strbuf_resize(buf, buf->len + len);
   eina_strlcpy(buf->buf + buf->len, str, buf->size - buf->len);
   buf->len += len;
}

/**
 * Append a string to a buffer, reallocating as necessary. Limited by maxlen.
 * @param buf the Eina_Strbuf to append to
 * @param str the string to append
 * @param maxlen maximum number of chars to append
 */
EAPI void
eina_strbuf_append_n(Eina_Strbuf *buf, const char *str, unsigned int maxlen)
{
   size_t len;

   EINA_MAGIC_CHECK_STRBUF(buf);

   len = strlen(str);
   if (len > maxlen) len = maxlen;
   _eina_strbuf_resize(buf, buf->len + len);

   eina_strlcpy(buf->buf + buf->len, str, len + 1); // + 1 for '\0'
   buf->len += len;
}

/**
 * Insert a string to a buffer, reallocating as necessary.
 * @param buf the Eina_Strbuf to insert
 * @param str the string to insert
 * @param pos the position to insert the string
 */
EAPI void
eina_strbuf_insert(Eina_Strbuf *buf, const char *str, size_t pos)
{
   size_t len;

   EINA_MAGIC_CHECK_STRBUF(buf);

   if (pos >= buf->len)
     {
	eina_strbuf_append(buf, str);
	return;
     }

   /*
    * resize the buffer if necessary
    */
   len = strlen(str);
   if (!_eina_strbuf_resize(buf, buf->len + len))
     return;
   /* move the existing text */
   memmove(buf->buf + len + pos, buf->buf + pos, buf->len - pos);
   /* and now insert the given string */
   memcpy(buf->buf + pos, str, len);
   buf->len += len;
   buf->buf[buf->len] = 0;
}

/**
 * Append a character to a string buffer, reallocating as necessary.
 * @param buf the Eina_Strbuf to append to
 * @param c the char to append
 */
EAPI void
eina_strbuf_append_char(Eina_Strbuf *buf, char c)
{
   EINA_MAGIC_CHECK_STRBUF(buf);

   _eina_strbuf_resize(buf, buf->len + 1);
   buf->buf[(buf->len)++] = c;
   buf->buf[buf->len] = '\0';
}

EAPI void
eina_strbuf_remove(Eina_Strbuf *buf, unsigned int start, unsigned int end)
{
   unsigned int remove_len, tail_len;

   EINA_MAGIC_CHECK_STRBUF(buf);

   if (end >= buf->len)
     end = buf->len;

   if (end <= start) return;

   remove_len = end - start;
   if (remove_len == buf->len)
     {
	free(buf->buf);
	_eina_strbuf_init(buf);
	return;
     }

   tail_len = buf->len - end + 1; /* includes '\0' */
   memmove(buf->buf + start, buf->buf + end, tail_len);
   buf->len -= remove_len;
   _eina_strbuf_resize(buf, buf->len);
}

/**
 * Retrieve a pointer to the contents of a string buffer
 * @param buf the buffer
 *
 * This pointer must not be modified, and will no longer be valid if
 * the Eina_Strbuf is modified.
 */
EAPI const char *
eina_strbuf_string_get(Eina_Strbuf *buf)
{
   EINA_MAGIC_CHECK_STRBUF(buf, NULL);

   return buf->buf;
}

/**
 * Remove the contents of a string buffer
 * @param buf the buffer
 */
EAPI char *
eina_strbuf_string_remove(Eina_Strbuf *buf)
{
   char *ret;
   EINA_MAGIC_CHECK_STRBUF(buf, NULL);

   ret = buf->buf;
   _eina_strbuf_init(buf);
   return ret;
}

/**
 * Retrieve the length of the string buffer content
 * @param buf the buffer
 */
EAPI size_t
eina_strbuf_length_get(Eina_Strbuf *buf)
{
   EINA_MAGIC_CHECK_STRBUF(buf, 0);

   return buf->len;
}

/**
 * Replace the n-th string with an other string.
 * @param buf the Eina_Strbuf to work with
 * @param str the string to replace
 * @param with the replaceing string
 * @param n the number of the fitting string
 *
 * @return true on success
 */
EAPI int
eina_strbuf_replace(Eina_Strbuf *buf, const char *str, const char *with,
                     unsigned int n)
{
   EINA_MAGIC_CHECK_STRBUF(buf, 0);

   size_t len1, len2;
   char *spos;
   size_t pos;

   if (n == 0)
     return 0;

   spos = buf->buf;
   while (n--)
     {
	spos = strstr(spos, str);
	if (!spos || *spos == '\0')
	  return 0;
	if (n) spos++;
     }

   pos = spos - buf->buf;
   len1 = strlen(str);
   len2 = strlen(with);
   if (len1 != len2)
     {
	/* resize the buffer if necessary */
	if (!_eina_strbuf_resize(buf, buf->len - len1 + len2))
	  return 0;
	/* move the existing text */
	memmove(buf->buf + pos + len2, buf->buf + pos + len1,
	      buf->len - pos - len1);
     }
   /* and now insert the given string */
   memcpy(buf->buf + pos, with, len2);
   buf->len += len2 - len1;
   buf->buf[buf->len] = 0;

   return 1;
}

/**
 * Replace all strings with an other string.
 * @param buf the Eina_Strbuf to work with
 * @param str the string to replace
 * @param with the replaceing string
 *
 * @return how often the string was replaced
 */
EAPI int
eina_strbuf_replace_all(Eina_Strbuf *buf, const char *str, const char *with)
{
   EINA_MAGIC_CHECK_STRBUF(buf, 0);

   size_t len1, len2, len;
   char *tmp_buf = NULL;
   char *spos;
   size_t pos, start;
   size_t pos_tmp, start_tmp;
   int n = 0;

   spos = strstr(buf->buf, str);
   if (!spos || *spos == '\0')
     return 0;

   len1 = strlen(str);
   len2 = strlen(with);

   /* if the size of the two string is equal, it is fairly easy to replace them
    * we don't need to resize the buffer or doing other calculations */
   if (len1 == len2)
     {
	while (spos)
	  {
	     memcpy(spos, with, len2);
	     spos = strstr(spos + len2, str);
	     n++;
	  }
	return n;
     }

   pos = pos_tmp = spos - buf->buf;
   tmp_buf = buf->buf;
   buf->buf = malloc(buf->size);
   if (!buf->buf)
     {
	buf->buf = tmp_buf;
	return 0;
     }
   start = start_tmp = 0;
   len = buf->len;

   while (spos)
     {
	n++;
	len = (len + len2) - len1;
	/* resize the buffer if necessary */
	if (!_eina_strbuf_resize(buf, len))
	  {
	     /* we have to stop replacing here, because we haven't enough
	      * memory to go on */
	     len = (len + len1) - len2;
	     break;
	  }

	/* copy the untouched text */
	memcpy(buf->buf + start, tmp_buf + start_tmp, pos - start);
	/* copy the new string */
	memcpy(buf->buf + pos, with, len2);

	/* calculate the next positions */
	start_tmp = pos_tmp + len1;
	start = pos + len2;
	spos = strstr(tmp_buf + start_tmp, str);
	/* this calculations don't make sense if spos == NULL, but the
	 * calculated values won't be used, because the loop will stop
	 * then */
	pos_tmp = spos - tmp_buf;
	pos = start + pos_tmp - start_tmp;
     }
   /* and now copy the rest of the text */
   memcpy(buf->buf + start, tmp_buf + start_tmp, len - start);
   buf->len = len;
   buf->buf[buf->len] = 0;

   free(tmp_buf);

   return n;
}

/**
 * init the buffer
 * @param buf the buffer to init
 */
static void
_eina_strbuf_init(Eina_Strbuf *buf)
{
   buf->len = 0;
   buf->size = EINA_STRBUF_INIT_SIZE;
   buf->step = EINA_STRBUF_INIT_STEP;

   buf->buf = malloc(buf->size);
   buf->buf[0] = '\0';
}

/**
 * resize the buffer
 * @param buf the buffer to resize
 * @param size the minimum size of the buffer
 */
static int
_eina_strbuf_resize(Eina_Strbuf *buf, size_t size)
{
   char *buffer;
   size_t new_size;
   size_t new_step;

   size += 1; // Add extra space for '\0'

   new_size = buf->size;
   new_step = buf->step;

   /*
    * first we have to determine the new buffer size
    */
   if (size == buf->size)
     /* nothing to do */
     return 1;
   else if (size > buf->size)
     {
	/* enlarge the buffer */
	while (size > new_size)
	  {
	     new_size += new_step;
	     if (new_step < EINA_STRBUF_MAX_STEP)
	       new_step *= 2;
	     if (new_step > EINA_STRBUF_MAX_STEP)
	       new_step = EINA_STRBUF_MAX_STEP;
	  }
     }
   else
     {
	/* shrink the buffer */
	while (new_size - new_step > size)
	  {
	     new_size -= new_step;
	     if (new_step > EINA_STRBUF_INIT_STEP)
	       new_step /= 2;
	     if (new_step < EINA_STRBUF_INIT_STEP)
	       new_step = EINA_STRBUF_INIT_STEP;
	  }
     }

   /* reallocate the buffer to the new size */
   buffer = realloc(buf->buf, new_size);
   if (!buffer)
     return 0;

   buf->buf = buffer;
   buf->size = new_size;
   buf->step = new_step;
   return 1;
}
