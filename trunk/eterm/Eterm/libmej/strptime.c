
/***************************************************************
 * STRPTIME.C -- strptime() for IRIX                           *
 *            -- Michael Jennings                              *
 *            -- 2 April 1997                                  *
 ***************************************************************/
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

#ifdef IRIX

#include "config.h"
#include "../src/feature.h"

#include "global.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "strptime.h"

char *
strptime(char *buf, const char *format, struct tm *tm)
{

  register char c;
  register const char *tmp;
  register int i, len;

  for (tmp = format; *tmp;) {
    if (!(*buf))
      break;

    if ((c = *tmp++) != '%') {
      if (!isspace(*buf) && c != *buf++)
	return ((char *) NULL);
      for (; *buf != 0 && isspace(*buf); buf++);
      continue;
    }
    switch ((c = *tmp++)) {
      case 0:
      case '%':
	if (*buf++ != '%')
	  return ((char *) NULL);
	break;

      case 'C':
	buf = strptime(buf, USMap.LocaleDateFormat, tm);
	if (!buf)
	  return ((char *) NULL);
	break;

      case 'c':
	buf = strptime(buf, "%x %X", tm);
	if (!buf)
	  return ((char *) NULL);
	break;

      case 'D':
	buf = strptime(buf, "%m/%d/%y", tm);
	if (!buf)
	  return ((char *) NULL);
	break;

      case 'R':
	buf = strptime(buf, "%H:%M", tm);
	if (!buf)
	  return ((char *) NULL);
	break;

      case 'r':
	buf = strptime(buf, "%I:%M:%S %p", tm);
	if (!buf)
	  return ((char *) NULL);
	break;

      case 'T':
	buf = strptime(buf, "%H:%M:%S", tm);
	if (!buf)
	  return ((char *) NULL);
	break;

      case 'X':
	buf = strptime(buf, USMap.TimeFormat, tm);
	if (!buf)
	  return ((char *) NULL);
	break;

      case 'x':
	buf = strptime(buf, USMap.DateFormat, tm);
	if (!buf)
	  return ((char *) NULL);
	break;

      case 'j':
	if (!isdigit(*buf))
	  return ((char *) NULL);
	for (i = 0; *buf != 0 && isdigit(*buf); buf++) {
	  i *= 10;
	  i += *buf - '0';
	}
	if (i > 365)
	  return ((char *) NULL);
	tm->tm_yday = i;
	break;

      case 'M':
      case 'S':
	if (!(*buf) || isspace(*buf))
	  break;
	if (!isdigit(*buf))
	  return ((char *) NULL);
	for (i = 0; *buf != 0 && isdigit(*buf); buf++) {
	  i *= 10;
	  i += *buf - '0';
	}
	if (i > 59)
	  return ((char *) NULL);
	if (c == 'M')
	  tm->tm_min = i;
	else
	  tm->tm_sec = i;

	if (*buf && isspace(*buf))
	  for (; *tmp && !isspace(*tmp); tmp++);
	break;

      case 'H':
      case 'I':
      case 'k':
      case 'l':
	if (!isdigit(*buf))
	  return ((char *) NULL);
	for (i = 0; *buf && isdigit(*buf); buf++) {
	  i *= 10;
	  i += *buf - '0';
	}
	if (c == 'H' || c == 'k') {
	  if (i > 23)
	    return ((char *) NULL);
	} else if (i > 11)
	  return ((char *) NULL);
	tm->tm_hour = i;
	if (*buf && isspace(*buf))
	  for (; *tmp && !isspace(*tmp); tmp++);
	break;

      case 'p':
	len = strlen(USMap.AM);
	if (!strncasecmp(buf, USMap.AM, len)) {
	  if (tm->tm_hour > 12)
	    return ((char *) NULL);
	  if (tm->tm_hour == 12)
	    tm->tm_hour = 0;
	  buf += len;
	  break;
	}
	len = strlen(USMap.PM);
	if (!strncasecmp(buf, USMap.PM, len)) {
	  if (tm->tm_hour > 12)
	    return ((char *) NULL);
	  if (tm->tm_hour != 12)
	    tm->tm_hour += 12;
	  buf += len;
	  break;
	}
	return ((char *) NULL);

      case 'A':
      case 'a':
	for (i = 0; i < NUM_DAYS; i++) {
	  len = strlen(USMap.Days[i]);
	  if (!strncasecmp(buf, USMap.Days[i], len))
	    break;
	  len = strlen(USMap.DaysAbbrev[i]);
	  if (!strncasecmp(buf, USMap.DaysAbbrev[i], len))
	    break;
	}
	if (i == NUM_DAYS)
	  return ((char *) NULL);
	tm->tm_wday = i;
	buf += len;
	break;

      case 'd':
      case 'e':
	if (!isdigit(*buf))
	  return ((char *) NULL);
	for (i = 0; *buf && isdigit(*buf); buf++) {
	  i *= 10;
	  i += *buf - '0';
	}
	if (i > 31)
	  return ((char *) NULL);
	tm->tm_mday = i;
	if (*buf && isspace(*buf))
	  for (; *tmp && !isspace(*tmp); tmp++);
	break;

      case 'B':
      case 'b':
      case 'h':
	for (i = 0; i < NUM_MONTHS; i++) {
	  len = strlen(USMap.Months[i]);
	  if (!strncasecmp(buf, USMap.Months[i], len))
	    break;
	  len = strlen(USMap.MonthsAbbrev[i]);
	  if (!strncasecmp(buf, USMap.MonthsAbbrev[i], len))
	    break;
	}
	if (i == NUM_MONTHS)
	  return ((char *) NULL);
	tm->tm_mon = i;
	buf += len;
	break;

      case 'm':
	if (!isdigit(*buf))
	  return ((char *) NULL);
	for (i = 0; *buf && isdigit(*buf); buf++) {
	  i *= 10;
	  i += *buf - '0';
	}
	if (i < 1 || i > 12)
	  return ((char *) NULL);
	tm->tm_mon = i - 1;
	if (*buf && isspace(*buf))
	  for (; *tmp && !isspace(*tmp); tmp++);
	break;

      case 'Y':
      case 'y':
	if (!(*buf) || isspace(*buf))
	  break;
	if (!isdigit(*buf))
	  return ((char *) NULL);
	for (i = 0; *buf && isdigit(*buf); buf++) {
	  i *= 10;
	  i += *buf - '0';
	}
	if (c == 'Y')
	  i -= 1900;
	if (i < 0)
	  return ((char *) NULL);
	tm->tm_year = i;
	if (*buf && isspace(*buf))
	  for (; *tmp && !isspace(*tmp); tmp++);
	break;
    }
  }
  return (buf);
}

#endif
