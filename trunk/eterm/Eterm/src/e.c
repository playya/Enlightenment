/* e.c -- Eterm Enlightenment support

 * This file is original work by Michael Jennings <mej@eterm.org> and
 * Tuomo Venalainen <vendu@cc.hut.fi>.  This file, and any other file
 * bearing this same message or a similar one, is distributed under
 * the GNU Public License (GPL) as outlined in the COPYING file.
 *
 * Copyright (C) 1997-1999, Michael Jennings and Tuomo Venalainen
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * 
 */

static const char cvs_ident[] = "$Id$";

#include "config.h"
#include "feature.h"

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <X11/cursorfont.h>
#include <signal.h>

#include "../libmej/debug.h"
#include "../libmej/mem.h"
#include "../libmej/strings.h"
#include "debug.h"
#include "e.h"
#include "command.h"
#include "startup.h"
#include "options.h"
#include "pixmap.h"
#include "system.h"

Window ipc_win = None;
Atom ipc_atom = None;
static unsigned char timeout = 0;

/* Returns true if running under E, false otherwise */
unsigned char
check_for_enlightenment(void)
{
  static char have_e = -1;

  if (have_e == -1) {
    if (XInternAtom(Xdisplay, "ENLIGHTENMENT_COMMS", True) != None) {
      D_ENL(("Enlightenment detected.\n"));
      have_e = 1;
    } else {
      D_ENL(("Enlightenment not detected.\n"));
      have_e = 0;
    }
  }
  return (have_e);
}

Window
enl_ipc_get_win(void)
{

  unsigned char *str = NULL;
  Atom prop, prop2;
  unsigned long num, after;
  int format;
  Window dummy_win;
  int dummy_int;
  unsigned int dummy_uint;

  D_ENL(("enl_ipc_get_win():  Searching for IPC window.\n"));

  prop = XInternAtom(Xdisplay, "ENLIGHTENMENT_COMMS", True);
  if (prop == None) {
    D_ENL((" -> Enlightenment is not running.  You lose!\n"));
    return None;
  }
  XGetWindowProperty(Xdisplay, Xroot, prop, 0, 14, False, AnyPropertyType, &prop2, &format, &num, &after, &str);
  if (str) {
    sscanf((char *) str, "%*s %x", (unsigned int *) &ipc_win);
    XFree(str);
  }
  if (ipc_win != None) {
    if (!XGetGeometry(Xdisplay, ipc_win, &dummy_win, &dummy_int, &dummy_int, &dummy_uint, &dummy_uint, &dummy_uint, &dummy_uint)) {
      D_ENL((" -> IPC Window property is valid, but the window doesn't exist.  I give up!\n"));
      ipc_win = None;
    }
    str = NULL;
    if (ipc_win != None) {
      XGetWindowProperty(Xdisplay, ipc_win, prop, 0, 14, False, AnyPropertyType, &prop2, &format, &num, &after, &str);
      if (str) {
	XFree(str);
      } else {
        D_ENL((" -> IPC Window lacks the proper atom.  I can't talk to fake IPC windows....\n"));
	ipc_win = None;
      }
    }
  }
  if (ipc_win != None) {
    D_ENL((" -> IPC Window found and verified as 0x%08x.  Registering Eterm as an IPC client.\n", (int) ipc_win));
    XSelectInput(Xdisplay, ipc_win, StructureNotifyMask | SubstructureNotifyMask);
    enl_ipc_send("set clientname " APL_NAME);
    enl_ipc_send("set version " VERSION);
    enl_ipc_send("set email mej@eterm.org");
    enl_ipc_send("set web http://www.eterm.org/");
    enl_ipc_send("set info Eterm Enlightened Terminal Emulator");
  }
  return (ipc_win);
}

void
enl_ipc_send(char *str)
{

  static char *last_msg = NULL;
  char buff[21];
  register unsigned short i;
  register unsigned char j;
  unsigned short len;
  XEvent ev;

  if (str == NULL) {
    ASSERT(last_msg != NULL);
    str = last_msg;
    D_ENL(("enl_ipc_send():  Resending last message \"%s\" to Enlightenment.\n", str));
  } else {
    if (last_msg != NULL) {
      FREE(last_msg);
    }
    last_msg = StrDup(str);
    D_ENL(("enl_ipc_send():  Sending \"%s\" to Enlightenment.\n", str));
  }

  if (ipc_win == None) {
    if ((ipc_win = enl_ipc_get_win()) == None) {
      D_ENL(("enl_ipc_send():  ...or perhaps not, since Enlightenment doesn't seem to be running.  No IPC window, no IPC.  Sorry....\n"));
      return;
    }
  }
  len = strlen(str);
  ipc_atom = XInternAtom(Xdisplay, "ENL_MSG", False);
  if (ipc_atom == None) {
    D_ENL(("enl_ipc_send():  IPC error:  Unable to find/create ENL_MSG atom.\n"));
    return;
  }
  ev.xclient.type = ClientMessage;
  ev.xclient.serial = 0;
  ev.xclient.send_event = True;
  ev.xclient.window = ipc_win;
  ev.xclient.message_type = ipc_atom;
  ev.xclient.format = 8;

  for (i = 0; i < len + 1; i += 12) {
    sprintf(buff, "%8x", (int) TermWin.parent);
    for (j = 0; j < 12; j++) {
      buff[8 + j] = str[i + j];
      if (!str[i + j]) {
	break;
      }
    }
    buff[20] = 0;
    for (j = 0; j < 20; j++) {
      ev.xclient.data.b[j] = buff[j];
    }
    XSendEvent(Xdisplay, ipc_win, False, 0, (XEvent *) & ev);
  }
  D_ENL(("enl_ipc_send():  Message sent to IPC window 0x%08x.\n", ipc_win));
}

static RETSIGTYPE
enl_ipc_timeout(int sig)
{
  timeout = 1;
  return ((RETSIGTYPE) sig);
}

char *
enl_wait_for_reply(void)
{

  XEvent ev;
  static char msg_buffer[20];
  register unsigned char i;

  alarm(3);
  for (; !XCheckTypedWindowEvent(Xdisplay, TermWin.parent, ClientMessage, &ev) && !timeout;);
  alarm(0);
  if (ev.xany.type != ClientMessage) {
    return (IPC_TIMEOUT);
  }
  for (i = 0; i < 20; i++) {
    msg_buffer[i] = ev.xclient.data.b[i];
  }
  return (msg_buffer + 8);
}

char *
enl_ipc_get(const char *msg_data)
{

  static char *message = NULL;
  static unsigned short len = 0;
  char buff[13], *ret_msg = NULL;
  register unsigned char i;
  unsigned char blen;

  if (msg_data == IPC_TIMEOUT) {
    return (IPC_TIMEOUT);
  }
  for (i = 0; i < 12; i++) {
    buff[i] = msg_data[i];
  }
  buff[12] = 0;
  blen = strlen(buff);
  if (message != NULL) {
    len += blen;
    message = (char *) REALLOC(message, len + 1);
    strcat(message, buff);
  } else {
    len = blen;
    message = (char *) MALLOC(len + 1);
    strcpy(message, buff);
  }
  if (blen < 12) {
    ret_msg = message;
    message = NULL;
    D_ENL(("enl_ipc_get():  Received complete reply:  \"%s\"\n", ret_msg));
  }
  return (ret_msg);
}

char *
enl_send_and_wait(char *msg)
{

  char *reply = IPC_TIMEOUT;
  sighandler_t old_alrm;

  if (ipc_win == None) {
    /* The IPC window is missing.  Wait for it to return or Eterm to be killed. */
    for (; enl_ipc_get_win() == None;) {
      sleep(1);
    }
  }
  old_alrm = (sighandler_t) signal(SIGALRM, (sighandler_t) enl_ipc_timeout);
  for (; reply == IPC_TIMEOUT;) {
    timeout = 0;
    enl_ipc_send(msg);
    for (; !(reply = enl_ipc_get(enl_wait_for_reply())););
    if (reply == IPC_TIMEOUT) {
      /* We timed out.  The IPC window must be AWOL.  Reset and resend message. */
      D_ENL(("enl_wait_for_reply():  IPC timed out.  IPC window 0x%08x has gone AWOL.  Clearing ipc_win.\n", ipc_win));
      XSelectInput(Xdisplay, ipc_win, None);
      ipc_win = None;
      check_image_ipc(1);
    }
  }
  signal(SIGALRM, old_alrm);
  return (reply);
}

void
eterm_ipc_parse(char *str)
{

  char *params;

  ASSERT(str != NULL);

  params = strchr(str, ':');
  if (params) {
    *params++ = 0;		/* Nuke the colon */
  }
  if (!strcasecmp(str, "echo") || !strcasecmp(str, "tty_write")) {
    if (params) {
      tt_write(params, strlen(params));
    } else {
      print_error("IPC Error:  Invalid syntax in command \"%s\"", str);
    }
  } else if (!strcasecmp(str, "parse")) {
    if (params) {
      cmd_write(params, strlen(params));
    } else {
      print_error("IPC Error:  Invalid syntax in command \"%s\"", str);
    }
  } else if (!strcasecmp(str, "enl_send")) {
    if (params) {
      enl_ipc_send(params);
    } else {
      print_error("IPC Error:  Invalid syntax in command \"%s\"", str);
    }
  } else if (!strcasecmp(str, "enl_query")) {
    if (params) {
      char *reply, header[512];

      reply = enl_send_and_wait(params);
      snprintf(header, sizeof(header), "Enlightenment IPC Reply to \"%s\":\n\n", params);
      tt_write(header, strlen(header));
      tt_write(reply, strlen(reply));
      tt_write("\n", 1);
      FREE(reply);
    } else {
      print_error("IPC Error:  Invalid syntax in command \"%s\"", str);
    }
  } else if (!strcasecmp(str, "winop")) {
    if (params) {
      eterm_handle_winop(params);
    } else {
      print_error("IPC Error:  Invalid syntax in command \"%s\"", str);
    }
  } else if (!strcasecmp(str, "exit")) {
    exit(0);
  } else {
    print_error("IPC Error:  Unrecognized command \"%s\"", str);
  }
}

void
eterm_ipc_send(char *str)
{

	str = NULL;
}

char *
eterm_ipc_get(void)
{

  return (NULL);
}

void
eterm_handle_winop(char *action)
{

  char *winid;
  Window win = 0;

  ASSERT(action != NULL);

  winid = strchr(action, ' ');
  if (winid) {
    win = (Window) strtoul(winid + 1, (char **) NULL, 0);
  }
  if (win == 0) {		/* If no window ID was given, or if the strtoul() call failed */
    win = TermWin.parent;
  }
  if (!BEG_STRCASECMP(action, "raise")) {
    XRaiseWindow(Xdisplay, win);
  } else if (!BEG_STRCASECMP(action, "lower")) {
    XLowerWindow(Xdisplay, win);
  } else if (!BEG_STRCASECMP(action, "map")) {
    XMapWindow(Xdisplay, win);
  } else if (!BEG_STRCASECMP(action, "unmap")) {
    XUnmapWindow(Xdisplay, win);
  } else if (!BEG_STRCASECMP(action, "kill")) {
    XKillClient(Xdisplay, win);
  } else if (!BEG_STRCASECMP(action, "iconify")) {
    XIconifyWindow(Xdisplay, win, Xscreen);
  } else {
    print_error("IPC Error:  Unrecognized window operation \"%s\"", action);
  }
}
