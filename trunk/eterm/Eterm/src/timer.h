/*
 * Copyright (C) 1997-2002, Michael Jennings
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

#ifndef _TIMER_H_
#define _TIMER_H_

#include <sys/time.h>
#include <unistd.h>
#include <X11/Xfuncproto.h>
#include <X11/Intrinsic.h>	/* Xlib, Xutil, Xresource, Xfuncproto */

/************ Macros and Definitions ************/
#define find_timer_by_handle(handle)  (handle)
#define timer_change_data(handle, data)  ((handle)->data = (data))
#define timer_change_handler(handle, handler)  ((handle)->handler = (handler))

/************ Structures ************/
typedef unsigned char (*timer_handler_t)(void *);
typedef struct timer_struct etimer_t;
typedef etimer_t *timerhdl_t;  /* The timer handles are actually pointers to a etimer_t struct, but clients shouldn't use them as such. */
struct timer_struct {
  unsigned long msec;
  struct timeval time;
  timer_handler_t handler;
  void *data;
  struct timer_struct *next;
};

/************ Variables ************/

/************ Function Prototypes ************/
_XFUNCPROTOBEGIN

extern timerhdl_t timer_add(unsigned long msec, timer_handler_t handler, void *data);
extern unsigned char timer_del(timerhdl_t handle);
extern unsigned char timer_change_delay(timerhdl_t handle, unsigned long msec);
extern void timer_check(void);

_XFUNCPROTOEND

#endif	/* _TIMER_H_ */
