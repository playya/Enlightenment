/* Copyright (C) 2004 Andreas Volz and various contributors
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
 *
 *  File: e16menuedit2.h
 *  Created by: Andreas Volz <linux@brachttal.net>
 *
 */

#ifndef _E16MENUEDIT_H
#define _E16MENUEDIT_H

#include <gtk/gtk.h>
#include <glade/glade.h>
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "nls.h"
#include "libglade_support.h"

#define to_utf8(String) g_locale_to_utf8(String,-1,0,0,0)
#define from_utf8(String) g_locale_from_utf8(String,-1,0,0,0)

#define APP_HOME ".e16menuedit2"
#define ICON_DIR "icons"

#endif /* _E16MENUEDIT_H */
