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
 *  File: file.h
 *  Created by: Andreas Volz <linux@brachttal.net>
 *
 */

#ifndef _FILE_H
#define _FILE_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <pwd.h>
#include <sys/types.h>
#include <glib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <locale.h>
#include "e16menuedit2.h"

char *field (char *s, int field);
void fword (char *s, int num, char *wd);
char *homedir (int uid);
int mkdir_with_parent (const char *pathname, mode_t mode);
char *strtok_left (char *s, const char *delim, unsigned int number);
char *strsplit (char *s, char **right, int count);
int version_cmp (char *ver1, char *ver2);
char *pkg_config_version (char *package);
char *get_fallback_locale (char *locale);
int run_help (char *help_app, char* help_dir, char *help_file);

#endif /* _FILE_H */
