/*********************************************************
 * DEBUG.C -- Standardized debugging output              *
 *         -- Michael Jennings                           *
 *         -- 20 December 1996                           *
 *********************************************************/
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
#include "../src/feature.h"

#include "global.h"
#include <stdio.h>
#include <stdlib.h>
#ifndef WITH_DMALLOC
# include <malloc.h>
#endif
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include "debug.h"

int
real_dprintf(const char *format,...)
{

  va_list args;
  int n;

  va_start(args, format);
  n = vfprintf(stderr, format, args);
  va_end(args);
  fflush(stderr);
  return (n);
}
