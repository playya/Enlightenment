/*
 * Copyright (C) 2000-2007 Carsten Haitzler, Geoff Harrison and various contributors
 * Copyright (C) 2004-2009 Kim Woelders
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
#ifndef _TIMERS_H_
#define _TIMERS_H_

#include "etypes.h"

/* timers.c */
double              GetTime(void);

Timer              *TimerAdd(double in_time,
			     int (*func) (void *data), void *data);
void                TimerDel(Timer * timer);
void                TimerSetInterval(Timer * timer, double dt);
double              TimersRun(double t);

#define TIMER_ADD(timer, in, func, prm) \
   timer = TimerAdd(in, func, prm)
#define TIMER_DEL(timer) \
   if (timer) { TimerDel(timer); timer = NULL; }

Idler              *IdlerAdd(void (*func) (void *data), void *data);
void                IdlerDel(Idler * id);
void                IdlersRun(void);

Animator           *AnimatorAdd(int (*func) (void *data), void *data);
void                AnimatorDel(Animator * an);

#endif /* _TIMERS_H_ */
