/*
 * Copyright (C) 1997-2000, Michael Jennings
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies of the Software, its documentation and marketing & publicity
 * materials, and acknowledgment shall be given in the documentation, materials
 * and software packages that this Software was used.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

static const char cvs_ident[] = "$Id$";

#include "config.h"
#include "feature.h"

#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include "../libmej/debug.h"
#include "debug.h"
#include "../libmej/mem.h"
#include "../libmej/strings.h"
#include "command.h"
#include "startup.h"
#include "misc.h"
#include "options.h"

const char *
my_basename(const char *str)
{

  const char *base = strrchr(str, '/');

  return (base ? base + 1 : str);

}

/* Print a non-terminal error message */
void
print_error(const char *fmt,...)
{

  va_list arg_ptr;

  va_start(arg_ptr, fmt);
  fprintf(stderr, APL_NAME ":  ");
  vfprintf(stderr, fmt, arg_ptr);
  fprintf(stderr, "\n");
  va_end(arg_ptr);
}

/* Print a simple warning. */
void
print_warning(const char *fmt,...)
{

  va_list arg_ptr;

  va_start(arg_ptr, fmt);
  fprintf(stderr, APL_NAME ":  warning:  ");
  vfprintf(stderr, fmt, arg_ptr);
  fprintf(stderr, "\n");
  va_end(arg_ptr);
}

/* Print a fatal error message and terminate */
void
fatal_error(const char *fmt,...)
{

  va_list arg_ptr;

  va_start(arg_ptr, fmt);
  fprintf(stderr, APL_NAME ":  FATAL:  ");
  vfprintf(stderr, fmt, arg_ptr);
  fprintf(stderr, "\n");
  va_end(arg_ptr);
  exit(-1);
}

/*
 * Compares the first n characters of s1 and s2, where n is strlen(s2)
 * Returns strlen(s2) if they match, 0 if not.
 */
unsigned long
str_leading_match(register const char *s1, register const char *s2)
{

  register unsigned long n;

  if (!s1 || !s2) {
    return (0);
  }
  for (n = 0; *s2; n++, s1++, s2++) {
    if (*s1 != *s2) {
      return 0;
    }
  }

  return n;
}

/* Strip leading and trailing whitespace and quotes from a string */
char *
str_trim(char *str)
{

  register char *tmp = str;
  size_t n;

  if (str && *str) {

    chomp(str);
    n = strlen(str);

    if (!n) {
      *str = 0;
      return str;
    }
    /* strip leading/trailing quotes */
    if (*tmp == '"') {
      tmp++;
      n--;
      if (!n) {
	*str = 0;
	return str;
      } else if (tmp[n - 1] == '"') {
	tmp[--n] = '\0';
      }
    }
    if (tmp != str) {
      memmove(str, tmp, (strlen(tmp)) + 1);
    }
  }
  return str;
}

/*
 * in-place interpretation of string:
 *
 *      backslash-escaped:      "\a\b\E\e\n\r\t", "\octal"
 *      Ctrl chars:     ^@ .. ^_, ^?
 *
 *      Emacs-style:    "M-" prefix
 *
 * Also,
 *      "M-x" prefixed strings, append "\r" if needed
 *      "\E]" prefixed strings (XTerm escape sequence) append "\a" if needed
 *
 * returns the converted string length
 */

int
parse_escaped_string(char *str)
{

  register char *pold = str, *pnew;
  unsigned char i;

  D_STRINGS(("parse_escaped_string(\"%s\")\n", str));

  if (!BEG_STRCASECMP(pold, "m-")) {
    *pold = '\\';
    *(pold + 1) = 'e';
  }
  for (pold = pnew = str; *pold; pold++, pnew++) {
    D_STRINGS(("Looking at \"%s\"\n", pold));
    if (!BEG_STRCASECMP(pold, "m-") && (isspace(*(pold - 1)) || !isprint(*(pold -1)))) {
      *pold = '\\';
      *(pold + 1) = 'e';
    } else if (!BEG_STRCASECMP(pold, "c-")) {
      *(++pold) = '^';
    }
    D_STRINGS(("Operating on \'%c\'\n", *pold));
    switch (*pold) {
      case '\\':
	D_STRINGS(("Backslash + %c\n", *(pold + 1)));
	switch (tolower(*(++pold))) {
	  case '0':
	  case '1':
	  case '2':
	  case '3':
	  case '4':
	  case '5':
	  case '6':
	  case '7':
	    for (i = 0; *pold >= '0' && *pold <= '7'; pold++) {
	      i = (i * 8) + (*pold - '0');
	    }
	    pold--;
	    D_STRINGS(("Octal number evaluates to %d\n", i));
	    *pnew = i;
	    break;
	  case 'n':
	    *pnew = '\n';
	    break;
	  case 'r':
	    *pnew = '\r';
	    break;
	  case 't':
	    *pnew = '\t';
	    break;
	  case 'b':
	    *pnew = '\b';
	    break;
	  case 'f':
	    *pnew = '\f';
	    break;
	  case 'a':
	    *pnew = '\a';
	    break;
	  case 'v':
	    *pnew = '\v';
	    break;
	  case 'e':
	    *pnew = '\033';
	    break;
	  case 'c':
	    pold++;
	    *pnew = MAKE_CTRL_CHAR(*pold);
	    break;
	  default:
	    *pnew = *pold;
	    break;
	}
	break;
      case '^':
	D_STRINGS(("Caret + %c\n", *(pold + 1)));
	pold++;
	*pnew = MAKE_CTRL_CHAR(*pold);
	break;
      default:
	*pnew = *pold;
    }
  }

  if (!BEG_STRCASECMP(str, "\033x") && *(pnew - 1) != '\r') {
    D_STRINGS(("Adding carriage return\n"));
    *(pnew++) = '\r';
  } else if (!BEG_STRCASECMP(str, "\033]") && *(pnew - 1) != '\a') {
    D_STRINGS(("Adding bell character\n"));
    *(pnew++) = '\a';
  }
  *pnew = 0;

#if DEBUG >= DEBUG_STRINGS
  if (debug_level >= DEBUG_STRINGS) {
    D_STRINGS(("New value is:\n\n"));
    HexDump(str, (size_t) (pnew - str));
  }
#endif

  return (pnew - str);
}

const char *
find_file(const char *file, const char *ext)
{

  const char *f;

#if defined(PIXMAP_SUPPORT)
  if ((f = search_path(rs_path, file, ext)) != NULL) {
    return (f);
  } else if ((f = search_path(getenv(PATH_ENV), file, ext)) != NULL) {
    return (f);
  } else {
    return (search_path(initial_dir, file, ext));
  }
#else
  return ((const char *) NULL);
#endif

}

char *
safe_print_string(char *str, unsigned long len)
{
  static char *ret_buff = NULL;
  static unsigned long rb_size = 0;
  char *p;
  unsigned long n = 0, i;

  if (ret_buff == NULL) {
    ret_buff = (char *) MALLOC(len + 1);
    rb_size = len;
  }
  for (i = 0, p = ret_buff; i < len; i++, str++, n++) {
    if (n + 2 >= rb_size) {
      rb_size *= 2;
      ret_buff = (char *) REALLOC(ret_buff, rb_size + 1);
    }
    if (*str < ' ') {
      *p++ = '^';
      *p++ = *str + '@';
      n++;
    } else {
      *p++ = *str;
    }
  }
  *p = 0;
  return ret_buff;
}

unsigned long
add_carriage_returns(unsigned char *buff, unsigned long cnt)
{
  register unsigned char *out, *outp, *in;
  register unsigned long i;

  D_CMD(("buff == %8p \"%s\", cnt == %lu\n", buff, safe_print_string(buff, cnt), cnt));
  outp = out = (unsigned char *) MALLOC(cnt * 2);
  for (i = 0, in = buff; i < cnt; i++) {
    if (*in == '\n') {
      *out++ = '\r';
    }
    *out++ = *in++;
  }
  i = (unsigned long) (out - outp);
  memcpy(buff, outp, i);
  FREE(outp);
  D_CMD(("buff == %8p \"%s\", i == %lu\n", buff, safe_print_string(buff, i), i));
  return i;
}
