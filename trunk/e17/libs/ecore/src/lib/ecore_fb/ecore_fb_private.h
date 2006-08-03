#ifndef _ECORE_FB_PRIVATE_H
#define _ECORE_FB_PRIVATE_H

#include "ecore_private.h"
#include "Ecore.h"
#include "Ecore_Data.h"

#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

/* ecore_fb_li.c */
struct _Ecore_Fb_Input_Device
{
	int fd;
	Ecore_Fd_Handler *handler;
	int listen;
	struct {
		Ecore_Fb_Input_Device_Cap cap;
		char *name;
		char *dev;
	} info;
	struct
	{
		/* common mouse */
		int x,y;
		int w,h;
		
		double last;
		double prev;
		double threshold;
		/* absolute axis */
		int min_w, min_h;
		double rel_w, rel_h;

	} mouse;
	struct
	{
		int shift;
		int ctrl;
		int alt;
		int lock;
	} keyboard;
};

/* ecore_fb_vt.c */
int  ecore_fb_vt_init(void);
void ecore_fb_vt_shutdown(void);

#if 0
/* hacks to stop people NEEDING #include <linux/h3600_ts.h> */
#ifndef TS_SET_CAL
#define TS_SET_CAL 0x4014660b
#endif
#ifndef TS_GET_CAL
#define TS_GET_CAL 0x8014660a
#endif
#ifndef TS_SET_BACKLIGHT
#define TS_SET_BACKLIGHT 0x40086614
#endif
#ifndef TS_GET_BACKLIGHT
#define TS_GET_BACKLIGHT 0x80086614
#endif
#ifndef LED_ON
#define LED_ON 0x40046605
#endif
#ifndef TS_SET_CONTRAST
#define TS_SET_CONTRAST 0x40046615
#endif
#ifndef TS_GET_CONTRAST
#define TS_GET_CONTRAST 0x80046615
#endif
#ifndef FLITE_ON
#define FLITE_ON 0x40046607
#endif
#endif

#endif
