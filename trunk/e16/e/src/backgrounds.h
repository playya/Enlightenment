/*
 * Copyright (C) 2000-2006 Carsten Haitzler, Geoff Harrison and various contributors
 * Copyright (C) 2004-2006 Kim Woelders
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
#ifndef _BACKGROUNDS_H_
#define _BACKGROUNDS_H_

typedef struct _background Background;

/* backgrounds.c */
Background         *BackgroundFind(const char *name);
Background         *BackgroundCheck(Background * bg);
Background         *BackgroundGetRandom(void);
char               *BackgroundGetUniqueString(const Background * bg);
void                BackgroundPixmapSet(Background * bg, Pixmap pmap);
void                BackgroundPixmapUnset(Background * bg, Pixmap pmap);
void                BackgroundDestroyByName(const char *name);
void                BackgroundRealize(Background * bg, Drawable draw,
				      unsigned int rw, unsigned int rh,
				      int is_win, Pixmap * ppmap,
				      unsigned long *ppixel);
void                BackgroundApplyPmap(Background * bg, Drawable draw,
					unsigned int rw, unsigned int rh);
void                BackgroundSet(Background * bg, Window win, unsigned int rw,
				  unsigned int rh);
void                BackgroundIncRefcount(Background * bg);
void                BackgroundDecRefcount(Background * bg);

void                BackgroundTouch(Background * bg);
const char         *BackgroundGetName(const Background * bg);
int                 BackgroundGetColor(const Background * bg);
Pixmap              BackgroundGetPixmap(const Background * bg);
int                 BackgroundIsNone(const Background * bg);
Background         *BrackgroundCreateFromImage(const char *bgid,
					       const char *file, char *thumb,
					       int thlen);

void                BackgroundsInvalidate(int refresh);
int                 BackgroundsConfigLoad(FILE * fs);

#endif /* _BACKGROUNDS_H_ */
