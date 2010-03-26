#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include "Evil.h"


#ifdef _WIN32_WCE

/*
 * Error related functions
 *
 */

char *strerror (int errnum __UNUSED__)
{
   return "[Windows CE] error\n";
}

#endif /* _WIN32_WCE */


/*
 * bit related functions
 *
 */

int ffs(int i)
{
   int size;
   int x;

   if (!i) return 1;

   /* remove the sign bit */
   x = i & -i;
   size = sizeof(int) << 3;
   for (i = size; i > 0; --i, x <<= 1)
     if (x & (1 << (size - 1))) return i;

   return x;
}


#ifdef _WIN32_WCE

/*
 * String manipulation related functions
 *
 */

int
strcoll (const char *s1, const char *s2)
{
#ifdef UNICODE
   wchar_t *ws1;
   wchar_t *ws2;
   int      res;

   ws1 = evil_char_to_wchar(s1);
   ws2 = evil_char_to_wchar(s2);
   res = wcscmp(ws1, ws2);
   if (ws1) free(ws1);
   if (ws2) free(ws2);

   return res;
#else
   return strcmp(s1, s2);
#endif /* ! UNICODE */
}


#endif /* _WIN32_WCE */

char *
strrstr (const char *str, const char *substr)
{
  char *it;
  char *ret = NULL;

  while ((it = strstr(str, substr)))
    ret = it;

  return ret;
}

#ifdef _MSC_VER

int strcasecmp(const char *s1, const char *s2)
{
   return lstrcmp(s1, s2);
}

#endif /* _MSC_VER */
