/*
 * Copyright (C) 2004-2005 Kim Woelders
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
#ifndef _ECOMPMGR_H
#define _ECOMPMGR_H

#if USE_COMPOSITE

typedef struct
{
   char                enable;
   int                 shadow;
} cfg_composite;

int                 EVisualIsARGB(Visual * vis);

void                ECompMgrParseArgs(const char *args);

void                ECompMgrWinNew(EObj * eo);
void                ECompMgrWinDel(EObj * eo, Bool gone, Bool do_fade);
void                ECompMgrWinMap(EObj * eo);
void                ECompMgrWinUnmap(EObj * eo);
void                ECompMgrWinMoveResize(EObj * eo, int change_xy,
					  int change_wh, int change_bw);
void                ECompMgrWinReparent(EObj * eo, int desk, int change_xy);
void                ECompMgrWinChangeShape(EObj * eo);

void                ECompMgrWinChangeOpacity(EObj * eo, unsigned int opacity);
Pixmap              ECompMgrWinGetPixmap(const EObj * eo);
void                ECompMgrConfigGet(cfg_composite * cfg);
void                ECompMgrConfigSet(const cfg_composite * cfg);

void                ECompMgrMoveResizeFix(EObj * eo, int x, int y, int w,
					  int h);

#else

#define EVisualIsARGB(vis)      0

#endif

#endif /* _ECOMPMGR_H */
