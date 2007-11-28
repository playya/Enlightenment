/*
 * Copyright (C) 2000-2007 Carsten Haitzler, Geoff Harrison and various contributors
 * Copyright (C) 2004-2007 Kim Woelders
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
#include "E.h"
#include "comms.h"
#include "ipc.h"
#include "e16-ecore_hints.h"
#include "e16-ecore_list.h"
#include "xwin.h"

struct _client
{
   char               *name;
   Window              xwin;
   char               *msg;
   char               *clientname;
   char               *version;
   char               *info;
};

static Ecore_List  *client_list = NULL;

static Win          comms_win = NoWin;

static Atom         XA_ENLIGHTENMENT_COMMS = 0;
static Atom         XA_ENL_MSG = 0;

static Client      *
ClientCreate(Window xwin)
{
   Client             *c;
   char                st[32];

   c = ECALLOC(Client, 1);
   if (!c)
      return NULL;

   Esnprintf(st, sizeof(st), "%8x", (int)xwin);
   c->name = Estrdup(st);
   c->xwin = xwin;

   if (!client_list)
      client_list = ecore_list_new();
   ecore_list_prepend(client_list, c);

   return c;
}

static void
ClientDestroy(Client * c)
{
   if (!c)
      return;

   ecore_list_node_remove(client_list, c);

   if (c->name)
      Efree(c->name);
   if (c->msg)
      Efree(c->msg);
   if (c->clientname)
      Efree(c->clientname);
   if (c->version)
      Efree(c->version);
   if (c->info)
      Efree(c->info);
   Efree(c);
}

static int
ClientConfigure(Client * c, const char *str)
{
   char                param[64];
   const char         *value;
   int                 len;

   len = 0;
   sscanf(str, "%*s %60s %n", param, &len);
   value = str + len;

   if (!strcmp(param, "clientname"))
     {
	if (c->clientname)
	   Efree(c->clientname);
	c->clientname = Estrdup(value);
     }
   else if (!strcmp(param, "version"))
     {
	if (c->version)
	   Efree(c->version);
	c->version = Estrdup(value);
     }
   else if (!strcmp(param, "author"))
     {
     }
   else if (!strcmp(param, "email"))
     {
     }
   else if (!strcmp(param, "web"))
     {
     }
   else if (!strcmp(param, "address"))
     {
     }
   else if (!strcmp(param, "info"))
     {
	if (c->info)
	   Efree(c->info);
	c->info = Estrdup(value);
     }
   else if (!strcmp(param, "pixmap"))
     {
     }
   else
     {
	return -1;
     }

   return 0;
}

static int
ClientMatchWindow(const void *data, const void *match)
{
   return ((const Client *)data)->xwin != (Window) match;
}

static Client      *
ClientFind(Window xwin)
{
   return (Client *) ecore_list_find(client_list, ClientMatchWindow,
				     (void *)xwin);
}

static char        *
ClientCommsGet(Client ** c, XClientMessageEvent * ev)
{
   char                s[13], s2[9], *msg;
   unsigned int        i;
   Window              xwin;
   Client             *cl;

   if ((!ev) || (!c))
      return NULL;
   if (ev->message_type != XA_ENL_MSG)
      return NULL;

   s[12] = 0;
   s2[8] = 0;
   for (i = 0; i < 8; i++)
      s2[i] = ev->data.b[i];
   for (i = 0; i < 12; i++)
      s[i] = ev->data.b[i + 8];
   xwin = None;
   sscanf(s2, "%lx", &xwin);
   if (xwin == None)
      return NULL;
   cl = ClientFind(xwin);
   if (!cl)
     {
	cl = ClientCreate(xwin);
	if (!cl)
	   return NULL;
     }

   /* append text to end of msg */
   i = (cl->msg) ? strlen(cl->msg) : 0;
   cl->msg = EREALLOC(char, cl->msg, i + strlen(s) + 1);
   if (!cl->msg)
      return NULL;
   strcpy(cl->msg + i, s);

   msg = NULL;
   if (strlen(s) < 12)
     {
	msg = cl->msg;
	cl->msg = NULL;
	*c = cl;
     }

   return msg;
}

static void
ClientIpcReply(void *data, const char *str)
{
   Client             *c = (Client *) data;

   if (!str)
      str = "";
   CommsSend(c, str);
}

static void
ClientHandleComms(XClientMessageEvent * ev)
{
   Client             *c;
   char               *s;

   s = ClientCommsGet(&c, ev);
   if (!s)
      return;

   if (EDebug(EDBUG_TYPE_IPC))
      Eprintf("ClientHandleComms: %s\n", s);

   if (!strncmp(s, "set ", 4))
     {
	/* The old Client set command (used by epplets) */
	if (ClientConfigure(c, s) == 0)
	   goto done;
     }

   if (!IpcExecReply(s, ClientIpcReply, c))
     {
	const char         *s1, *s2;

	s1 = (c->clientname) ? c->clientname : "UNKNOWN";
	s2 = (c->version) ? c->version : "UNKNOWN";
	DialogOK(_("E IPC Error"),
		 _("Received Unknown Client Message.\n"
		   "Client Name:    %s\n" "Client Version: %s\n"
		   "Message Contents:\n\n" "%s\n"), s1, s2, s);
	SoundPlay("SOUND_ERROR_IPC");
     }

 done:
   Efree(s);
}

static void
ClientHandleRootEvents(Win win __UNUSED__, XEvent * ev, void *prm __UNUSED__)
{
   Client             *c;

#if 0
   Eprintf("ClientHandleRootEvents: type=%d win=%#lx\n", ev->type,
	   ev->xany.window);
#endif
   switch (ev->type)
     {
     case DestroyNotify:
	c = ClientFind(ev->xdestroywindow.window);
	if (!c)
	   break;
	ClientDestroy(c);
	break;
     }
}

static void
ClientHandleCommsEvents(Win win __UNUSED__, XEvent * ev, void *prm __UNUSED__)
{
#if 0
   Eprintf("ClientHandleCommsEvents: type=%d win=%#lx\n", ev->type,
	   ev->xany.window);
#endif
   switch (ev->type)
     {
     case ClientMessage:
	ClientHandleComms(&(ev->xclient));
	break;
     }
}

void
CommsInit(void)
{
   char                s[1024];

   comms_win = ECreateEventWindow(VRoot.win, -100, -100, 5, 5);
   ESelectInput(comms_win, StructureNotifyMask | SubstructureNotifyMask);
   EventCallbackRegister(comms_win, 0, ClientHandleCommsEvents, NULL);
   EventCallbackRegister(VRoot.win, 0, ClientHandleRootEvents, NULL);

   Esnprintf(s, sizeof(s), "WINID %8lx", WinGetXwin(comms_win));
   XA_ENLIGHTENMENT_COMMS = XInternAtom(disp, "ENLIGHTENMENT_COMMS", False);
   ecore_x_window_prop_string_set(WinGetXwin(comms_win), XA_ENLIGHTENMENT_COMMS,
				  s);
   ecore_x_window_prop_string_set(VRoot.xwin, XA_ENLIGHTENMENT_COMMS, s);

   XA_ENL_MSG = XInternAtom(disp, "ENL_MSG", False);
}

static void
CommsDoSend(Window win, const char *s)
{
   char                ss[21];
   int                 i, j, k, len;
   XEvent              ev;

   if ((!win) || (!s))
      return;

   len = strlen(s);
   ev.xclient.type = ClientMessage;
   ev.xclient.serial = 0;
   ev.xclient.send_event = True;
   ev.xclient.window = win;
   ev.xclient.message_type = XA_ENL_MSG;
   ev.xclient.format = 8;
   for (i = 0; i < len + 1; i += 12)
     {
	Esnprintf(ss, sizeof(ss), "%8lx", WinGetXwin(comms_win));
	for (j = 0; j < 12; j++)
	  {
	     ss[8 + j] = s[i + j];
	     if (!s[i + j])
		j = 12;
	  }
	ss[20] = 0;
	for (k = 0; k < 20; k++)
	   ev.xclient.data.b[k] = ss[k];
	XSendEvent(disp, win, False, 0, (XEvent *) & ev);
     }
}

void
CommsSend(Client * c, const char *s)
{
   if (!c)
      return;

   CommsDoSend(c->xwin, s);
}

/*
 * When we are running in multi-head, connect to the master wm process
 * and send the message
 */
void
CommsSendToMasterWM(const char *s)
{
   if (Mode.wm.master)
      return;

   CommsDoSend(RootWindow(disp, Mode.wm.master_screen), s);
}

#if 0				/* Unused */
/*
 * When we are running in multi-head, connect to the slave wm processes
 * and broadcast the message
 */
void
CommsBroadcastToSlaveWMs(const char *s)
{
   int                 screen;

   if (!Mode.wm.master || Mode.wm.single)
      return;

   for (screen = 0; screen < Mode.display.screens; screen++)
     {
	if (screen != Mode.wm.master_screen)
	   CommsDoSend(RootWindow(disp, screen), s);
     }
}

void
CommsBroadcast(const char *s)
{
   char              **l;
   int                 num, i;
   Client             *c;

   l = ListItems(&num, LIST_TYPE_CLIENT);
   if (!s)
      return;
   for (i = 0; i < num; i++)
     {
	c = FindItem(l[i], 0, LIST_FINDBY_NAME, LIST_TYPE_CLIENT);
	if (c)
	   CommsSend(c, s);
     }
   StrlistFree(l, num);
}
#endif
