/*
 * Copyright (C) 2000-2004 Carsten Haitzler, Geoff Harrison and various contributors
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

static Window       comms_win = 0;

static Atom         XA_ENLIGHTENMENT_COMMS = 0;
static Atom         XA_ENL_MSG = 0;

void
CommsSetup(void)
{
   char                s[1024];

   EDBUG(5, "CommsSetup");

   comms_win = XCreateSimpleWindow(disp, root.win, -100, -100, 5, 5, 0, 0, 0);
   XSelectInput(disp, comms_win, StructureNotifyMask | SubstructureNotifyMask);
   Esnprintf(s, sizeof(s), "WINID %8x", (int)comms_win);
   XA_ENLIGHTENMENT_COMMS = XInternAtom(disp, "ENLIGHTENMENT_COMMS", False);
   XChangeProperty(disp, comms_win, XA_ENLIGHTENMENT_COMMS, XA_STRING, 8,
		   PropModeReplace, (unsigned char *)s, strlen(s));
   XChangeProperty(disp, root.win, XA_ENLIGHTENMENT_COMMS, XA_STRING, 8,
		   PropModeReplace, (unsigned char *)s, strlen(s));

   XA_ENL_MSG = XInternAtom(disp, "ENL_MSG", False);

   EDBUG_RETURN_;
}

void
CommsFindCommsWindow(void)
{
   unsigned char      *s;
   Atom                a, ar;
   unsigned long       num, after;
   int                 format;
   Window              rt;
   int                 dint;
   unsigned int        duint;

   EDBUG(6, "CommsFindCommsWindow");
   a = XA_ENLIGHTENMENT_COMMS;
   if (a != None)
     {
	s = NULL;
	XGetWindowProperty(disp, root.win, a, 0, 14, False, AnyPropertyType,
			   &ar, &format, &num, &after, &s);
	if (s)
	  {
	     comms_win = 0;
	     sscanf((char *)s, "%*s %lx", &comms_win);
	     XFree(s);
	  }
	else
	  {
	     (comms_win = 0);
	  }
	if (comms_win)
	  {
	     if (!EGetGeometry
		 (disp, comms_win, &rt, &dint, &dint, &duint, &duint, &duint,
		  &duint))
		comms_win = 0;
	     s = NULL;
	     if (comms_win)
	       {
		  XGetWindowProperty(disp, comms_win, a, 0, 14, False,
				     AnyPropertyType, &ar, &format, &num,
				     &after, &s);
		  if (s)
		     XFree(s);
		  else
		     comms_win = 0;
	       }
	  }
     }
   EDBUG_RETURN_;
}

static void
CommsDoSend(Window win, const char *s)
{
   char                ss[21];
   int                 i, j, k, len;
   XEvent              ev;

   EDBUG(5, "CommsDoSend");

   if ((!win) || (!s))
      EDBUG_RETURN_;

   len = strlen(s);
   ev.xclient.type = ClientMessage;
   ev.xclient.serial = 0;
   ev.xclient.send_event = True;
   ev.xclient.window = win;
   ev.xclient.message_type = XA_ENL_MSG;
   ev.xclient.format = 8;
   for (i = 0; i < len + 1; i += 12)
     {
	Esnprintf(ss, sizeof(ss), "%8x", (int)comms_win);
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
   EDBUG_RETURN_;
}

void
CommsSend(Client * c, const char *s)
{
   EDBUG(5, "CommsSend");

   if (!c)
      EDBUG_RETURN_;

   CommsDoSend(c->win, s);

   EDBUG_RETURN_;
}

/*
 * When we are running in multi-head, connect to the master wm process
 * and send the message
 */
void
CommsSendToMasterWM(const char *s)
{
   EDBUG(5, "CommsSendToMasterWM");

   if (Mode.wm.master)
      EDBUG_RETURN_;

   CommsDoSend(RootWindow(disp, Mode.wm.master_screen), s);

   EDBUG_RETURN_;
}

/*
 * When we are running in multi-head, connect to the slave wm processes
 * and broadcast the message
 */
void
CommsBroadcastToSlaveWMs(const char *s)
{
   int                 screen;

   EDBUG(5, "CommsBroadcastToSlaveWMs");

   if (!Mode.wm.master || Mode.wm.single)
      EDBUG_RETURN_;

   for (screen = 0; screen < Mode.display.screens; screen++)
     {
	if (screen != Mode.wm.master_screen)
	   CommsDoSend(RootWindow(disp, screen), s);
     }

   EDBUG_RETURN_;
}

static char        *
CommsGet(Client ** c, XClientMessageEvent * ev)
{
   char                s[13], s2[9], *msg, st[32];
   int                 i;
   Window              win = 0;
   Client             *cl;

   EDBUG(5, "CommsGet");
   if ((!ev) || (!c))
      EDBUG_RETURN(NULL);
   if (ev->message_type != XA_ENL_MSG)
      EDBUG_RETURN(NULL);

   s[12] = 0;
   s2[8] = 0;
   msg = NULL;
   for (i = 0; i < 8; i++)
      s2[i] = ev->data.b[i];
   for (i = 0; i < 12; i++)
      s[i] = ev->data.b[i + 8];
   sscanf(s2, "%lx", &win);
   cl = (Client *) FindItem(NULL, win, LIST_FINDBY_ID, LIST_TYPE_CLIENT);
   if (!cl)
     {
	cl = MakeClient(win);
	if (!cl)
	   EDBUG_RETURN(NULL);
	Esnprintf(st, sizeof(st), "%8x", (int)win);
	cl->name = Estrdup(st);
	AddItem((void *)cl, st, cl->win, LIST_TYPE_CLIENT);
	XSelectInput(disp, win, StructureNotifyMask | SubstructureNotifyMask);
     }
   if (cl->msg)
     {
	/* append text to end of msg */
	cl->msg = Erealloc(cl->msg, strlen(cl->msg) + strlen(s) + 1);
	if (!cl->msg)
	   EDBUG_RETURN(NULL);
	strcat(cl->msg, s);
     }
   else
     {
	/* new msg */
	cl->msg = Emalloc(strlen(s) + 1);
	if (!cl->msg)
	   EDBUG_RETURN(NULL);
	strcpy(cl->msg, s);
     }
   if (strlen(s) < 12)
     {
	msg = cl->msg;
	cl->msg = NULL;
	*c = cl;
     }
   EDBUG_RETURN(msg);
}

void
CommsBroadcast(const char *s)
{
   char              **l;
   int                 num, i;
   Client             *c;

   EDBUG(5, "CommsBroadcast");
   l = ListItems(&num, LIST_TYPE_CLIENT);
   if (!s)
      EDBUG_RETURN_;
   for (i = 0; i < num; i++)
     {
	c = (Client *) FindItem(l[i], 0, LIST_FINDBY_NAME, LIST_TYPE_CLIENT);
	if (c)
	   CommsSend(c, s);
     }
   freestrlist(l, num);
   EDBUG_RETURN_;
}

Client             *
MakeClient(Window win)
{
   Client             *c;

   EDBUG(6, "MakeClient");
   c = Emalloc(sizeof(Client));
   if (!c)
      EDBUG_RETURN(NULL);
   c->name = NULL;
   c->win = win;
   c->msg = NULL;
   c->clientname = NULL;
   c->version = NULL;
   c->author = NULL;
   c->email = NULL;
   c->web = NULL;
   c->address = NULL;
   c->info = NULL;
   c->pmap = 0;
   EDBUG_RETURN(c);
}

void
ListFreeClient(void *ptr)
{
   Client             *c;

   EDBUG(6, "ListFreeClient");
   c = (Client *) ptr;
   if (!c)
      EDBUG_RETURN_;
   if (c->name)
      Efree(c->name);
   if (c->msg)
      Efree(c->msg);
   if (c->clientname)
      Efree(c->clientname);
   if (c->version)
      Efree(c->version);
   if (c->author)
      Efree(c->author);
   if (c->email)
      Efree(c->email);
   if (c->web)
      Efree(c->web);
   if (c->address)
      Efree(c->address);
   if (c->info)
      Efree(c->info);
   Efree(c);
   EDBUG_RETURN_;
}

void
DeleteClient(Client * c)
{
   Client             *cc;

   EDBUG(6, "DeleteClient");
   cc = RemoveItem(NULL, c->win, LIST_FINDBY_ID, LIST_TYPE_CLIENT);
   ListFreeClient(cc);
   EDBUG_RETURN_;
}

void
HandleComms(XClientMessageEvent * ev)
{
   Client             *c;
   char               *s, *s1, *s2;
   char                w[FILEPATH_LEN_MAX], w2[FILEPATH_LEN_MAX];
   char                sunknown[] = "UNKNOWN";
   int                 unknown;

   EDBUG(4, "HandleComms");
   s = CommsGet(&c, ev);
   if (!s)
      EDBUG_RETURN_;

   if (HandleIPC(s, c))
     {
	Efree(s);
	EDBUG_RETURN_;
     }
   unknown = 0;
   s1 = NULL, s2 = NULL;
   word(s, 1, w);
   if (!strcmp(w, "set"))
     {
	word(s, 2, w);
	if (!strcmp(w, "clientname"))
	  {
	     if (c->clientname)
		Efree(c->clientname);
	     c->clientname = Estrdup(atword(s, 3));
	  }
	else if (!strcmp(w, "version"))
	  {
	     if (c->version)
		Efree(c->version);
	     c->version = Estrdup(atword(s, 3));
	  }
	else if (!strcmp(w, "author"))
	  {
	     if (c->author)
		Efree(c->author);
	     c->author = Estrdup(atword(s, 3));
	  }
	else if (!strcmp(w, "email"))
	  {
	     if (c->email)
		Efree(c->email);
	     c->email = Estrdup(atword(s, 3));
	  }
	else if (!strcmp(w, "web"))
	  {
	     if (c->web)
		Efree(c->web);
	     c->web = Estrdup(atword(s, 3));
	  }
	else if (!strcmp(w, "address"))
	  {
	     if (c->address)
		Efree(c->address);
	     c->address = Estrdup(atword(s, 3));
	  }
	else if (!strcmp(w, "info"))
	  {
	     if (c->info)
		Efree(c->info);
	     c->info = Estrdup(atword(s, 3));
	  }
	else if (!strcmp(w, "pixmap"))
	  {
	     word(s, 3, w);
	     c->pmap = 0;
	     sscanf(w, "%x", (int *)&c->pmap);
	  }
	else
	   unknown = 1;
     }
   else if (!strcmp(w, "reply"))
     {
	word(s, 2, w);
	if (!strcmp(w, "imageclass"))
	  {
	     /* Reply format "reply imageclass NAME 24243" */
	     word(s, 3, w);
	     word(s, 4, w2);
	     HonorIclass(w, atoi(w2));
	  }
	else
	   unknown = 1;
     }
   else if (!strcmp(w, "get_default_theme"))
     {
	char               *buf;

	buf = ThemeGetDefault();
	if (buf)
	  {
	     CommsSend(c, buf);
	     Efree(buf);
	  }
	else
	   CommsSend(c, "");
     }
   else if (!strcmp(w, "set_default_theme"))
     {
	char                sss[FILEPATH_LEN_MAX], buf[FILEPATH_LEN_MAX];

	word(s, 2, buf);
	if (exists(buf))
	  {
	     ThemeSetDefault(buf);
	     Esnprintf(sss, sizeof(sss), "restart_theme %s", buf);
	     SessionExit(sss);
	  }
     }
   else if (!strcmp(w, "list_bg"))
     {
	Background        **bg;
	int                 i, num, len = 0;
	char               *buf = NULL;

	bg = (Background **) ListItemType(&num, LIST_TYPE_BACKGROUND);
	if (bg)
	  {
	     for (i = 0; i < num; i++)
	       {
		  len += strlen(bg[i]->name) + 1;
		  if (buf)
		     buf = Erealloc(buf, len + 1);
		  else
		    {
		       buf = Erealloc(buf, len + 1);
		       buf[0] = 0;
		    }
		  strcat(buf, bg[i]->name);
		  strcat(buf, "\n");
	       }
	     Efree(bg);
	  }
	if (buf)
	  {
	     CommsSend(c, buf);
	     Efree(buf);
	  }
	else
	   CommsSend(c, "");
     }
   else if (!strcmp(w, "del_bg"))
     {
	Background         *bg;
	int                 i;
	char                dodel = 1;

	sscanf(s, "%*s %1000s", w);
	bg = (Background *) FindItem(w, 0, LIST_FINDBY_NAME,
				     LIST_TYPE_BACKGROUND);
	if (bg)
	  {
	     /* check for desktops referencing this bg - if there are - don't */
	     /* delete it */
	     for (i = 0; i < ENLIGHTENMENT_CONF_NUM_DESKTOPS; i++)
	       {
		  if (desks.desk[i].bg == bg)
		     dodel = 0;
	       }
	     if (dodel)
	       {
		  bg = (Background *) RemoveItem(w, 0, LIST_FINDBY_NAME,
						 LIST_TYPE_BACKGROUND);
		  if (bg->name)
		     Efree(bg->name);
		  if (bg->bg.file)
		     Efree(bg->bg.file);
		  if (bg->top.file)
		     Efree(bg->top.file);
		  if (bg->pmap)
		     imlib_free_pixmap_and_mask(bg->pmap);
		  Efree(bg);
	       }
	  }
     }
   else if (!strcmp(w, "use_bg"))
     {
	Background         *bg;
	int                 i, wd;
	char                view;

	sscanf(s, "%*s %1000s", w);
	bg = (Background *) FindItem(w, 0, LIST_FINDBY_NAME,
				     LIST_TYPE_BACKGROUND);
	if (bg)
	  {
	     wd = 3;
	     w[0] = ' ';
	     while (w[0])
	       {
		  w[0] = 0;
		  word(s, wd++, w);
		  if (w[0])
		    {
		       i = atoi(w);
		       if ((i >= 0) && (i < ENLIGHTENMENT_CONF_NUM_DESKTOPS))
			 {
			    if (desks.desk[i].bg)
			       desks.desk[i].bg->last_viewed = 0;
			    view = desks.desk[i].viewable;
			    desks.desk[i].viewable = 0;
			    BackgroundsAccounting();
			    desks.desk[i].viewable = view;
			    desks.desk[i].bg = bg;
			    if (i < Conf.desks.num)
			      {
				 if (desks.desk[i].viewable)
				    RefreshDesktop(i);
				 if (i == desks.current)
				   {
				      RedrawPagersForDesktop(i, 2);
				      ForceUpdatePagersForDesktop(i);
				   }
				 else
				    RedrawPagersForDesktop(i, 1);
			      }
			 }
		    }
	       }
	  }
     }
   else if (!strcmp(w, "use_no_bg"))
     {
	int                 i, wd;
	char                view;

	wd = 2;
	w[0] = ' ';
	while (w[0])
	  {
	     w[0] = 0;
	     word(s, wd++, w);
	     if (w[0])
	       {
		  i = atoi(w);
		  if ((i >= 0) && (i < ENLIGHTENMENT_CONF_NUM_DESKTOPS))
		    {
		       if (desks.desk[i].bg)
			  desks.desk[i].bg->last_viewed = 0;
		       view = desks.desk[i].viewable;
		       desks.desk[i].viewable = 0;
		       BackgroundsAccounting();
		       desks.desk[i].viewable = view;
		       desks.desk[i].bg = NULL;
		       if (i < Conf.desks.num)
			 {
			    if (desks.desk[i].viewable)
			       RefreshDesktop(i);
			    if (i == desks.current)
			      {
				 RedrawPagersForDesktop(i, 2);
				 ForceUpdatePagersForDesktop(i);
			      }
			    else
			       RedrawPagersForDesktop(i, 1);
			 }
		    }
	       }
	  }
     }
   else if (!strcmp(w, "uses_bg"))
     {
	Background         *bg;
	int                 i;
	char                buf[FILEPATH_LEN_MAX], buf2[FILEPATH_LEN_MAX];

	sscanf(s, "%*s %1000s", w);
	bg = (Background *) FindItem(w, 0, LIST_FINDBY_NAME,
				     LIST_TYPE_BACKGROUND);
	buf[0] = 0;
	if (bg)
	  {
	     for (i = 0; i < ENLIGHTENMENT_CONF_NUM_DESKTOPS; i++)
	       {
		  if (desks.desk[i].bg == bg)
		    {
		       Esnprintf(buf2, sizeof(buf2), "%i\n", i);
		       strcat(buf, buf2);
		    }
	       }
	  }
	CommsSend(c, buf);
     }
   else if (!strcmp(w, "get_keybindings"))
     {
	ActionClass        *ac;
	Action             *a;
	int                 i, mod;
	char               *buf = NULL, buf2[FILEPATH_LEN_MAX];

	ac = (ActionClass *) FindItem("KEYBINDINGS", 0, LIST_FINDBY_NAME,
				      LIST_TYPE_ACLASS_GLOBAL);
	if (ac)
	  {
	     for (i = 0; i < ac->num; i++)
	       {
		  a = ac->list[i];
		  if ((a) && (a->action) && (a->event == EVENT_KEY_DOWN))
		    {
		       char               *key;

		       key = XKeysymToString(XKeycodeToKeysym(disp, a->key, 0));
		       if (key)
			 {
			    mod = 0;
			    if (a->modifiers == (ControlMask))
			       mod = 1;
			    else if (a->modifiers == (Mod1Mask))
			       mod = 2;
			    else if (a->modifiers == (ShiftMask))
			       mod = 3;
			    else if (a->modifiers == (ControlMask | Mod1Mask))
			       mod = 4;
			    else if (a->modifiers == (ShiftMask | ControlMask))
			       mod = 5;
			    else if (a->modifiers == (ShiftMask | Mod1Mask))
			       mod = 6;
			    else if (a->modifiers ==
				     (ShiftMask | ControlMask | Mod1Mask))
			       mod = 7;
			    else if (a->modifiers == (Mod2Mask))
			       mod = 8;
			    else if (a->modifiers == (Mod3Mask))
			       mod = 9;
			    else if (a->modifiers == (Mod4Mask))
			       mod = 10;
			    else if (a->modifiers == (Mod5Mask))
			       mod = 11;
			    else if (a->modifiers == (Mod2Mask | ShiftMask))
			       mod = 12;
			    else if (a->modifiers == (Mod2Mask | ControlMask))
			       mod = 13;
			    else if (a->modifiers == (Mod2Mask | Mod1Mask))
			       mod = 14;
			    else if (a->modifiers == (Mod4Mask | ShiftMask))
			       mod = 15;
			    else if (a->modifiers == (Mod4Mask | ControlMask))
			       mod = 16;
			    else if (a->modifiers ==
				     (Mod4Mask | ControlMask | ShiftMask))
			       mod = 17;
			    else if (a->modifiers == (Mod5Mask | ShiftMask))
			       mod = 18;
			    else if (a->modifiers == (Mod5Mask | ControlMask))
			       mod = 19;
			    else if (a->modifiers ==
				     (Mod5Mask | ControlMask | ShiftMask))
			       mod = 20;
			    if (a->action->params)
			       Esnprintf(buf2, sizeof(buf2), "%s %i %i %s\n",
					 key, mod, a->action->Type,
					 (char *)a->action->params);
			    else
			       Esnprintf(buf2, sizeof(buf2), "%s %i %i\n", key,
					 mod, a->action->Type);
			    if (buf)
			      {
				 buf =
				    Erealloc(buf,
					     strlen(buf) + strlen(buf2) + 1);
				 strcat(buf, buf2);
			      }
			    else
			       buf = Estrdup(buf2);
			 }
		    }
	       }
	     if (buf)
	       {
		  CommsSend(c, buf);
		  Efree(buf);
	       }
	     else
		CommsSend(c, "\n");
	  }
	else
	   CommsSend(c, "\n");
     }
   else if (!strcmp(w, "set_keybindings"))
     {
	ActionClass        *ac;
	Action             *a;
	int                 i, l;
	char                buf[FILEPATH_LEN_MAX];
	const char         *sp, *ss;

	Mode.keybinds_changed = 1;
	ac = (ActionClass *) RemoveItem("KEYBINDINGS", 0, LIST_FINDBY_NAME,
					LIST_TYPE_ACLASS_GLOBAL);
	if (ac)
	   RemoveActionClass(ac);
	ac = CreateAclass("KEYBINDINGS");
	AddItem(ac, ac->name, 0, LIST_TYPE_ACLASS_GLOBAL);
	i = 0;
	ss = atword(s, 2);
	if (ss)
	  {
	     l = strlen(ss);
	     while (i < l)
	       {
		  char                key[256];
		  int                 mod = 0;
		  int                 act_id = 0;
		  int                 j = 0;

		  /* put line in buf */
		  sp = &(ss[i]);
		  while ((sp[j]) && (sp[j] != '\n'))
		    {
		       buf[j] = sp[j];
		       j++;
		    }
		  buf[j] = 0;
		  if (sp[j] == '\n')
		     j++;
		  i += j;
		  /* parse the line */
		  sscanf(buf, "%250s %i %i", key, &mod, &act_id);
		  if (mod == 0)
		     mod = 0;
		  else if (mod == 1)
		     mod = ControlMask;
		  else if (mod == 2)
		     mod = Mod1Mask;
		  else if (mod == 3)
		     mod = ShiftMask;
		  else if (mod == 4)
		     mod = ControlMask | Mod1Mask;
		  else if (mod == 5)
		     mod = ShiftMask | ControlMask;
		  else if (mod == 6)
		     mod = ShiftMask | Mod1Mask;
		  else if (mod == 7)
		     mod = ShiftMask | ControlMask | Mod1Mask;
		  else if (mod == 8)
		     mod = Mod2Mask;
		  else if (mod == 9)
		     mod = Mod3Mask;
		  else if (mod == 10)
		     mod = Mod4Mask;
		  else if (mod == 11)
		     mod = Mod5Mask;
		  else if (mod == 12)
		     mod = Mod2Mask | ShiftMask;
		  else if (mod == 13)
		     mod = Mod2Mask | ControlMask;
		  else if (mod == 14)
		     mod = Mod2Mask | Mod1Mask;
		  else if (mod == 15)
		     mod = Mod4Mask | ShiftMask;
		  else if (mod == 16)
		     mod = Mod4Mask | ControlMask;
		  else if (mod == 17)
		     mod = Mod4Mask | ControlMask | ShiftMask;
		  else if (mod == 18)
		     mod = Mod5Mask | ShiftMask;
		  else if (mod == 19)
		     mod = Mod5Mask | ControlMask;
		  else if (mod == 20)
		     mod = Mod5Mask | ControlMask | ShiftMask;
		  a = CreateAction(4, 0, mod, 0, 0, 0, key, NULL);
		  GrabActionKey(a);
		  AddAction(ac, a);
		  if (atword(buf, 4))
		     AddToAction(a, act_id, Estrdup(atword(buf, 4)));
		  else
		     AddToAction(a, act_id, NULL);
	       }
	  }
     }
   else if (!strcmp(w, "set_bg_colmod"))
     {
	Background         *bg;
	ColorModifierClass *cm;
	int                 i;
	char                buf[FILEPATH_LEN_MAX], buf2[FILEPATH_LEN_MAX];

	sscanf(s, "%*s %1000s %1000s", buf, buf2);
	bg = (Background *) FindItem(buf, 0, LIST_FINDBY_NAME,
				     LIST_TYPE_BACKGROUND);
	cm = (ColorModifierClass *) FindItem(buf2, 0, LIST_FINDBY_NAME,
					     LIST_TYPE_COLORMODIFIER);
	if ((bg) && (bg->cmclass != cm))
	  {
	     if (!strcmp(buf, "(null)"))
	       {
		  bg->cmclass->ref_count--;
		  bg->cmclass = NULL;
	       }
	     else if (cm)
	       {
		  bg->cmclass->ref_count--;
		  bg->cmclass = cm;
	       }
	     if (bg->pmap)
		imlib_free_pixmap_and_mask(bg->pmap);
	     bg->pmap = 0;
	     for (i = 0; i < ENLIGHTENMENT_CONF_NUM_DESKTOPS; i++)
	       {
		  if ((desks.desk[i].bg == bg) && (desks.desk[i].viewable))
		     RefreshDesktop(i);
	       }
	  }
     }
   else if (!strcmp(w, "get_bg_colmod"))
     {
	Background         *bg;
	char                buf[FILEPATH_LEN_MAX];

	sscanf(s, "%*s %1000s", w);
	bg = (Background *) FindItem(w, 0, LIST_FINDBY_NAME,
				     LIST_TYPE_BACKGROUND);
	Esnprintf(buf, sizeof(buf), "(null)");
	if ((bg) && (bg->cmclass))
	   Esnprintf(buf, sizeof(buf), "%s", bg->cmclass->name);
	CommsSend(c, buf);
     }
   else if (!strcmp(w, "del_colmod"))
     {
	ColorModifierClass *cm;
	char                buf[FILEPATH_LEN_MAX];

	sscanf(s, "%*s %1000s", w);
	cm = (ColorModifierClass *) FindItem(w, 0, LIST_FINDBY_NAME,
					     LIST_TYPE_COLORMODIFIER);
	Esnprintf(buf, sizeof(buf), "(null)");
	if (cm)
	   FreeCMClass(cm);
     }
   else if (!strcmp(w, "get_colmod"))
     {
	ColorModifierClass *cm;
	int                 i;
	char                buf[FILEPATH_LEN_MAX], buf2[FILEPATH_LEN_MAX];

	sscanf(s, "%*s %1000s", w);
	cm = (ColorModifierClass *) FindItem(w, 0, LIST_FINDBY_NAME,
					     LIST_TYPE_COLORMODIFIER);
	Esnprintf(buf, sizeof(buf), "(null)");
	if (cm)
	  {
	     Esnprintf(buf, sizeof(buf), "%i", (int)(cm->red.num));
	     for (i = 0; i < cm->red.num; i++)
	       {
		  Esnprintf(buf2, sizeof(buf2), " %i", (int)(cm->red.px[i]));
		  strcat(buf, buf2);
		  Esnprintf(buf2, sizeof(buf2), " %i", (int)(cm->red.py[i]));
		  strcat(buf, buf2);
	       }
	     Esnprintf(buf2, sizeof(buf2), "\n%i", (int)(cm->green.num));
	     strcat(buf, buf2);
	     for (i = 0; i < cm->green.num; i++)
	       {
		  Esnprintf(buf2, sizeof(buf2), " %i", (int)(cm->green.px[i]));
		  strcat(buf, buf2);
		  Esnprintf(buf2, sizeof(buf2), " %i", (int)(cm->green.py[i]));
		  strcat(buf, buf2);
	       }
	     Esnprintf(buf2, sizeof(buf2), "\n%i", (int)(cm->red.num));
	     strcat(buf, buf2);
	     for (i = 0; i < cm->blue.num; i++)
	       {
		  Esnprintf(buf2, sizeof(buf2), " %i", (int)(cm->blue.px[i]));
		  strcat(buf, buf2);
		  Esnprintf(buf2, sizeof(buf2), " %i", (int)(cm->blue.py[i]));
		  strcat(buf, buf2);
	       }
	  }
	CommsSend(c, buf);
     }
   else if (!strcmp(w, "set_colmod"))
     {
	ColorModifierClass *cm;
	int                 i, j, k;
	char               *name;
	int                 rnum = 0, gnum = 0, bnum = 0;
	unsigned char      *rpx = NULL, *rpy = NULL;
	unsigned char      *gpx = NULL, *gpy = NULL;
	unsigned char      *bpx = NULL, *bpy = NULL;

	sscanf(s, "%*s %1000s", w);
	cm = (ColorModifierClass *) FindItem(w, 0, LIST_FINDBY_NAME,
					     LIST_TYPE_COLORMODIFIER);
	name = Estrdup(w);
	i = 3;
	word(s, i++, w);
	rnum = atoi(w);
	j = 0;
	rpx = Emalloc(rnum);
	rpy = Emalloc(rnum);
	while (j < rnum)
	  {
	     word(s, i++, w);
	     k = atoi(w);
	     rpx[j] = k;
	     word(s, i++, w);
	     k = atoi(w);
	     rpy[j++] = k;
	  }
	word(s, i++, w);
	gnum = atoi(w);
	j = 0;
	gpx = Emalloc(gnum);
	gpy = Emalloc(gnum);
	while (j < gnum)
	  {
	     word(s, i++, w);
	     k = atoi(w);
	     gpx[j] = k;
	     word(s, i++, w);
	     k = atoi(w);
	     gpy[j++] = k;
	  }
	word(s, i++, w);
	bnum = atoi(w);
	j = 0;
	bpx = Emalloc(bnum);
	bpy = Emalloc(bnum);
	while (j < bnum)
	  {
	     word(s, i++, w);
	     k = atoi(w);
	     bpx[j] = k;
	     word(s, i++, w);
	     k = atoi(w);
	     bpy[j++] = k;
	  }
	if (cm)
	   ModifyCMClass(name, rnum, rpx, rpy, gnum, gpx, gpy, bnum, bpx, bpy);
	else
	  {
	     cm = CreateCMClass(name, rnum, rpx, rpy, gnum, gpx, gpy, bnum, bpx,
				bpy);
	     AddItem(cm, cm->name, 0, LIST_TYPE_COLORMODIFIER);
	  }
	Efree(name);
	if (rpx)
	   Efree(rpx);
	if (rpy)
	   Efree(rpy);
	if (gpx)
	   Efree(gpx);
	if (gpy)
	   Efree(gpy);
	if (bpx)
	   Efree(bpx);
	if (bpy)
	   Efree(bpy);
     }
   else if (!strcmp(w, "get_bg"))
     {
	Background         *bg;
	char                buf[FILEPATH_LEN_MAX];
	int                 r, g, b;

	sscanf(s, "%*s %1000s", w);
	bg = (Background *) FindItem(w, 0, LIST_FINDBY_NAME,
				     LIST_TYPE_BACKGROUND);
	Esnprintf(buf, sizeof(buf), "(null)");
	if (bg)
	  {
	     EGetColor(&(bg->bg_solid), &r, &g, &b);
	     if ((bg->bg.file) && (bg->top.file))
		Esnprintf(buf, sizeof(buf),
			  "%s %i %i %i %s %i %i %i %i %i %i %s %i %i %i %i %i",
			  bg->name, r, g, b, bg->bg.file, bg->bg_tile,
			  bg->bg.keep_aspect, bg->bg.xjust, bg->bg.yjust,
			  bg->bg.xperc, bg->bg.yperc, bg->top.file,
			  bg->top.keep_aspect, bg->top.xjust, bg->top.yjust,
			  bg->top.xperc, bg->top.yperc);
	     else if ((!(bg->bg.file)) && (bg->top.file))
		Esnprintf(buf, sizeof(buf),
			  "%s %i %i %i %s %i %i %i %i %i %i %s %i %i %i %i %i",
			  bg->name, r, g, b, "(null)", bg->bg_tile,
			  bg->bg.keep_aspect, bg->bg.xjust, bg->bg.yjust,
			  bg->bg.xperc, bg->bg.yperc, bg->top.file,
			  bg->top.keep_aspect, bg->top.xjust, bg->top.yjust,
			  bg->top.xperc, bg->top.yperc);
	     else if ((bg->bg.file) && (!(bg->top.file)))
		Esnprintf(buf, sizeof(buf),
			  "%s %i %i %i %s %i %i %i %i %i %i %s %i %i %i %i %i",
			  bg->name, r, g, b, bg->bg.file, bg->bg_tile,
			  bg->bg.keep_aspect, bg->bg.xjust, bg->bg.yjust,
			  bg->bg.xperc, bg->bg.yperc, "(null)",
			  bg->top.keep_aspect, bg->top.xjust, bg->top.yjust,
			  bg->top.xperc, bg->top.yperc);
	     else if ((!(bg->bg.file)) && (!(bg->top.file)))
		Esnprintf(buf, sizeof(buf),
			  "%s %i %i %i %s %i %i %i %i %i %i %s %i %i %i %i %i",
			  bg->name, r, g, b, "(null)", bg->bg_tile,
			  bg->bg.keep_aspect, bg->bg.xjust, bg->bg.yjust,
			  bg->bg.xperc, bg->bg.yperc, "(null)",
			  bg->top.keep_aspect, bg->top.xjust, bg->top.yjust,
			  bg->top.xperc, bg->top.yperc);
	  }
	CommsSend(c, buf);
     }
   else if (!strcmp(w, "set_bg"))
     {
	Background         *bg;
	XColor              xclr;
	int                 i, r, g, b;
	char                tmp[1024];
	char               *name = NULL, *bgf = NULL, *topf = NULL;
	int                 updated = 0, tile, keep_aspect, tkeep_aspect;
	int                 xjust, yjust, xperc, yperc;
	int                 txjust, tyjust, txperc, typerc;

	sscanf(s, "%1000s %1000s", tmp, w);
	bg = (Background *) FindItem(w, 0, LIST_FINDBY_NAME,
				     LIST_TYPE_BACKGROUND);
	r = 99;
	i = sscanf(s,
		   "%1000s %1000s %i %i %i %1000s %i %i %i %i %i %i %1000s %i %i %i %i %i",
		   tmp, tmp, &r, &g, &b, tmp, &tile,
		   (int *)&keep_aspect, &xjust, &yjust, &xperc, &yperc, tmp,
		   &tkeep_aspect, &txjust, &tyjust, &txperc, &typerc);
	ESetColor(&xclr, r, g, b);

	if (bg)
	  {
	     name = Estrdup(w);
	     word(s, 6, w);
	     if (strcmp("(null)", w))
		bgf = Estrdup(w);
	     word(s, 13, w);
	     if (strcmp("(null)", w))
		topf = Estrdup(w);

	     if (xclr.red != bg->bg_solid.red)
		updated = 1;
	     if (xclr.green != bg->bg_solid.green)
		updated = 1;
	     if (xclr.blue != bg->bg_solid.blue)
		updated = 1;
	     bg->bg_solid = xclr;
	     if ((bg->bg.file) && (bgf))
	       {
		  if (strcmp(bg->bg.file, bgf))
		     updated = 1;
	       }
	     else
		updated = 1;
	     if (bg->bg.file)
		Efree(bg->bg.file);
	     bg->bg.file = bgf;
	     if ((int)tile != bg->bg_tile)
		updated = 1;
	     if ((int)keep_aspect != bg->bg.keep_aspect)
		updated = 1;
	     if (xjust != bg->bg.xjust)
		updated = 1;
	     if (yjust != bg->bg.yjust)
		updated = 1;
	     if (xperc != bg->bg.xperc)
		updated = 1;
	     if (yperc != bg->bg.yperc)
		updated = 1;
	     bg->bg_tile = (char)tile;
	     bg->bg.keep_aspect = (char)keep_aspect;
	     bg->bg.xjust = xjust;
	     bg->bg.yjust = yjust;
	     bg->bg.xperc = xperc;
	     bg->bg.yperc = yperc;
	     if ((bg->top.file) && (topf))
	       {
		  if (strcmp(bg->top.file, topf))
		     updated = 1;
	       }
	     else
		updated = 1;
	     if (bg->top.file)
		Efree(bg->top.file);
	     bg->top.file = topf;
	     if ((int)tkeep_aspect != bg->top.keep_aspect)
		updated = 1;
	     if (txjust != bg->top.xjust)
		updated = 1;
	     if (tyjust != bg->top.yjust)
		updated = 1;
	     if (txperc != bg->top.xperc)
		updated = 1;
	     if (typerc != bg->top.yperc)
		updated = 1;
	     bg->top.keep_aspect = (char)tkeep_aspect;
	     bg->top.xjust = txjust;
	     bg->top.yjust = tyjust;
	     bg->top.xperc = txperc;
	     bg->top.yperc = typerc;
	     if (updated)
	       {
		  if (bg->pmap)
		     imlib_free_pixmap_and_mask(bg->pmap);
		  bg->pmap = 0;

		  for (i = 0; i < ENLIGHTENMENT_CONF_NUM_DESKTOPS; i++)
		    {
		       if (desks.desk[i].bg == bg)
			 {
			    if (desks.desk[i].viewable)
			       RefreshDesktop(i);
			    if (i == desks.current)
			      {
				 RedrawPagersForDesktop(i, 2);
				 ForceUpdatePagersForDesktop(i);
			      }
			    else
			       RedrawPagersForDesktop(i, 1);
			 }
		    }
	       }
	  }
	else
	  {
	     name = Estrdup(w);
	     word(s, 6, w);
	     if (strcmp("(null)", w))
		bgf = Estrdup(w);
	     word(s, 13, w);
	     if (strcmp("(null)", w))
		topf = Estrdup(w);
	     bg = BackgroundCreate(name, &xclr, bgf, tile, keep_aspect, xjust,
				   yjust, xperc, yperc, topf, tkeep_aspect,
				   txjust, tyjust, txperc, typerc);
	     if (name)
		Efree(name);
	     if (bgf)
		Efree(bgf);
	     if (topf)
		Efree(topf);
	     AddItem(bg, bg->name, 0, LIST_TYPE_BACKGROUND);
	  }
     }
   else if (!strcmp(w, "draw_bg_to"))
     {
	Window              win = 0;
	Background         *bg;

	sscanf(s, "%*s %lx %1000s", &win, w);
	bg = (Background *) FindItem(w, 0, LIST_FINDBY_NAME,
				     LIST_TYPE_BACKGROUND);
	if (bg)
	   BackgroundApply(bg, win, 0);
	CommsSend(c, "done");
     }
   else if (!strcmp(w, "set_controls"))
     {
	int                 wd;
	int                 a, b;
	int                 ax, ay;
	char                dragbar_change = 0;

	wd = 2;
	w[0] = 0;
	while (atword(s, wd))
	  {
	     word(s, wd, w);
	     wd++;
	     if (!strcmp(w, "FOCUSMODE:"))
	       {
		  word(s, wd, w);
		  Conf.focus.mode = atoi(w);
	       }
	     else if (!strcmp(w, "DOCKAPP_SUPPORT:"))
	       {
		  word(s, wd, w);
		  Conf.dock.dirmode = atoi(w);
	       }
	     else if (!strcmp(w, "DOCKDIRMODE:"))
	       {
		  word(s, wd, w);
		  Conf.dockapp_support = atoi(w);
	       }
	     else if (!strcmp(w, "ICONDIRMODE:"))
	       {
		  word(s, wd, w);
		  Conf.primaryicondir = atoi(w);
	       }
	     else if (!strcmp(w, "MOVEMODE:"))
	       {
		  word(s, wd, w);
		  Conf.movemode = atoi(w);
#if !USE_IMLIB2
		  if ((prImlib_Context) && (Conf.movemode == 5))
		     Conf.movemode = 3;
#endif
	       }
	     else if (!strcmp(w, "RESIZEMODE:"))
	       {
		  word(s, wd, w);
		  Conf.resizemode = atoi(w);
		  if (Conf.resizemode == 5)
		     Conf.resizemode = 3;
	       }
	     else if (!strcmp(w, "SLIDEMODE:"))
	       {
		  word(s, wd, w);
		  Conf.slidemode = atoi(w);
	       }
	     else if (!strcmp(w, "CLEANUPSLIDE:"))
	       {
		  word(s, wd, w);
		  Conf.cleanupslide = atoi(w);
	       }
	     else if (!strcmp(w, "MAPSLIDE:"))
	       {
		  word(s, wd, w);
		  Conf.mapslide = atoi(w);
	       }
	     else if (!strcmp(w, "SLIDESPEEDMAP:"))
	       {
		  word(s, wd, w);
		  Conf.slidespeedmap = atoi(w);
	       }
	     else if (!strcmp(w, "SLIDESPEEDCLEANUP:"))
	       {
		  word(s, wd, w);
		  Conf.slidespeedcleanup = atoi(w);
	       }
	     else if (!strcmp(w, "SHADESPEED:"))
	       {
		  word(s, wd, w);
		  Conf.shadespeed = atoi(w);
	       }
	     else if (!strcmp(w, "DESKTOPBGTIMEOUT:"))
	       {
		  word(s, wd, w);
		  Conf.backgrounds.timeout = atoi(w);
	       }
	     else if (!strcmp(w, "SOUND:"))
	       {
		  word(s, wd, w);
		  Conf.sound = atoi(w);
		  if (Conf.sound)
		     SoundInit();
		  else
		     SoundExit();
	       }
	     else if (!strcmp(w, "BUTTONMOVERESISTANCE:"))
	       {
		  word(s, wd, w);
		  Conf.button_move_resistance = atoi(w);
	       }
	     else if (!strcmp(w, "AUTOSAVE:"))
	       {
		  word(s, wd, w);
		  Conf.autosave = atoi(w);
	       }
	     else if (!strcmp(w, "MEMORYPARANOIA:"))
	       {
		  word(s, wd, w);
		  Conf.memory_paranoia = atoi(w);
	       }
	     else if (!strcmp(w, "MENUSLIDE:"))
	       {
		  word(s, wd, w);
		  Conf.menuslide = atoi(w);
	       }
	     else if (!strcmp(w, "NUMDESKTOPS:"))
	       {
		  word(s, wd, w);
		  ChangeNumberOfDesktops(atoi(w));
	       }
	     else if (!strcmp(w, "TOOLTIPS:"))
	       {
		  word(s, wd, w);
		  Conf.tooltips.enable = atoi(w);
	       }
	     else if (!strcmp(w, "TIPTIME:"))
	       {
		  word(s, wd, w);
		  Conf.tooltips.delay = atof(w);
	       }
	     else if (!strcmp(w, "AUTORAISE:"))
	       {
		  word(s, wd, w);
		  Conf.autoraise.enable = atoi(w);
	       }
	     else if (!strcmp(w, "AUTORAISETIME:"))
	       {
		  word(s, wd, w);
		  Conf.autoraise.delay = atof(w);
	       }
	     else if (!strcmp(w, "DOCKSTARTX:"))
	       {
		  word(s, wd, w);
		  Conf.dock.startx = atoi(w);
	       }
	     else if (!strcmp(w, "DOCKSTARTY:"))
	       {
		  word(s, wd, w);
		  Conf.dock.starty = atoi(w);
	       }
	     else if (!strcmp(w, "SAVEUNDER:"))
	       {
		  word(s, wd, w);
		  Conf.save_under = atoi(w);
	       }
	     else if (!strcmp(w, "DRAGDIR:"))
	       {
		  word(s, wd, w);
		  if (Conf.desks.dragdir != atoi(w))
		     dragbar_change = 1;
		  Conf.desks.dragdir = atoi(w);
	       }
	     else if (!strcmp(w, "DRAGBARWIDTH:"))
	       {
		  word(s, wd, w);
		  if (Conf.desks.dragbar_width != atoi(w))
		     dragbar_change = 1;
		  Conf.desks.dragbar_width = atoi(w);
	       }
	     else if (!strcmp(w, "DRAGBARORDERING:"))
	       {
		  word(s, wd, w);
		  if (Conf.desks.dragbar_ordering != atoi(w))
		     dragbar_change = 1;
		  Conf.desks.dragbar_ordering = atoi(w);
	       }
	     else if (!strcmp(w, "DRAGBARLENGTH:"))
	       {
		  word(s, wd, w);
		  if (Conf.desks.dragbar_length != atoi(w))
		     dragbar_change = 1;
		  Conf.desks.dragbar_length = atoi(w);
	       }
	     else if (!strcmp(w, "DESKSLIDEIN:"))
	       {
		  word(s, wd, w);
		  Conf.desks.slidein = atoi(w);
	       }
	     else if (!strcmp(w, "DESKSLIDESPEED:"))
	       {
		  word(s, wd, w);
		  Conf.desks.slidespeed = atoi(w);
	       }
	     else if (!strcmp(w, "HIQUALITYBG:"))
	       {
		  word(s, wd, w);
		  Conf.backgrounds.hiquality = atoi(w);
	       }
	     else if (!strcmp(w, "TRANSIENTSFOLLOWLEADER:"))
	       {
		  word(s, wd, w);
		  Conf.focus.transientsfollowleader = atoi(w);
	       }
	     else if (!strcmp(w, "SWITCHFORTRANSIENTMAP:"))
	       {
		  word(s, wd, w);
		  Conf.focus.switchfortransientmap = atoi(w);
	       }
	     else if (!strcmp(w, "SHOWICONS:"))
	       {
		  /* Obsolete */
	       }
	     else if (!strcmp(w, "ALL_NEW_WINDOWS_GET_FOCUS:"))
	       {
		  word(s, wd, w);
		  Conf.focus.all_new_windows_get_focus = atoi(w);
	       }
	     else if (!strcmp(w, "NEW_TRANSIENTS_GET_FOCUS:"))
	       {
		  word(s, wd, w);
		  Conf.focus.new_transients_get_focus = atoi(w);
	       }
	     else if (!strcmp(w, "NEW_TRANSIENTS_GET_FOCUS_IF_GROUP_FOCUSED:"))
	       {
		  word(s, wd, w);
		  Conf.focus.new_transients_get_focus_if_group_focused =
		     atoi(w);
	       }
	     else if (!strcmp(w, "MANUAL_PLACEMENT:"))
	       {
		  word(s, wd, w);
		  Conf.manual_placement = atoi(w);
	       }
	     else if (!strcmp(w, "MANUAL_PLACEMENT_MOUSE_POINTER:"))
	       {
		  word(s, wd, w);
		  Conf.manual_placement_mouse_pointer = atoi(w);
	       }
	     else if (!strcmp(w, "RAISE_ON_NEXT_FOCUS:"))
	       {
		  word(s, wd, w);
		  Conf.focus.raise_on_next_focus = atoi(w);
	       }
	     else if (!strcmp(w, "RAISE_AFTER_NEXT_FOCUS:"))
	       {
		  word(s, wd, w);
		  Conf.focus.raise_after_next_focus = atoi(w);
	       }
	     else if (!strcmp(w, "DISPLAY_WARP:"))
	       {
		  word(s, wd, w);
		  Conf.warplist.enable = atoi(w);
	       }
	     else if (!strcmp(w, "WARP_ON_NEXT_FOCUS:"))
	       {
		  word(s, wd, w);
		  Conf.focus.warp_on_next_focus = atoi(w);
	       }
	     else if (!strcmp(w, "WARP_AFTER_NEXT_FOCUS:"))
	       {
		  word(s, wd, w);
		  Conf.focus.warp_after_next_focus = atoi(w);
	       }
	     else if (!strcmp(w, "EDGE_FLIP_RESISTANCE:"))
	       {
		  word(s, wd, w);
		  Conf.edge_flip_resistance = atoi(w);
		  ShowEdgeWindows();
	       }
	     else if (!strcmp(w, "AREA_SIZE:"))
	       {
		  w[0] = 0;
		  word(s, wd, w);
		  if (w[0])
		     a = atoi(w);
		  else
		     a = 0;
		  wd++;
		  w[0] = 0;
		  word(s, wd, w);
		  if (w[0])
		     b = atoi(w);
		  else
		     b = 0;
		  if ((a > 0) && (b > 0))
		     SetAreaSize(a, b);
	       }
	     wd++;
	  }
	if (dragbar_change)
	  {
	     Button             *btn;

	     while ((btn = RemoveItem("_DESKTOP_DRAG_CONTROL", 0,
				      LIST_FINDBY_NAME, LIST_TYPE_BUTTON)))
		ButtonDestroy(btn);
	     InitDesktopControls();
	     ShowDesktopControls();
	  }
	FocusFix();

	GetAreaSize(&ax, &ay);
	GetCurrentArea(&a, &b);
	if (a >= ax)
	  {
	     SetCurrentArea(ax - 1, b);
	     GetCurrentArea(&a, &b);
	  }
	if (b >= ay)
	   SetCurrentArea(a, ay - 1);
     }
   else if (!strcmp(w, "get_controls"))
     {
	char                buf[FILEPATH_LEN_MAX];
	int                 a, b;

	GetAreaSize(&a, &b);
	Esnprintf(buf, sizeof(buf),
		  "FOCUSMODE: %i\n" "DOCKAPP_SUPPORT: %i\n" "DOCKDIRMODE: %i\n"
		  "ICONDIRMODE: %i\n" "MOVEMODE: %i\n" "RESIZEMODE: %i\n"
		  "SLIDEMODE: %i\n" "CLEANUPSLIDE: %i\n" "MAPSLIDE: %i\n"
		  "SLIDESPEEDMAP: %i\n" "SLIDESPEEDCLEANUP: %i\n"
		  "SHADESPEED: %i\n" "DESKTOPBGTIMEOUT: %i\n" "SOUND: %i\n"
		  "BUTTONMOVERESISTANCE: %i\n" "AUTOSAVE: %i\n"
		  "MEMORYPARANOIA: %i\n" "TOOLTIPS: %i\n" "TIPTIME: %f\n"
		  "AUTORAISE: %i\n" "AUTORAISETIME: %f\n" "DOCKSTARTX: %i\n"
		  "DOCKSTARTY: %i\n" "SAVEUNDER: %i\n" "MENUSLIDE: %i\n"
		  "NUMDESKTOPS: %i\n" "DRAGDIR: %i\n" "DRAGBARWIDTH: %i\n"
		  "DRAGBARORDERING: %i\n" "DRAGBARLENGTH: %i\n"
		  "DESKSLIDEIN: %i\n" "DESKSLIDESPEED: %i\n" "HIQUALITYBG: %i\n"
		  "TRANSIENTSFOLLOWLEADER: %i\n" "SWITCHFORTRANSIENTMAP: %i\n"
		  "AREA_SIZE: %i %i\n"
		  "ALL_NEW_WINDOWS_GET_FOCUS: %i\n"
		  "NEW_TRANSIENTS_GET_FOCUS: %i\n"
		  "NEW_TRANSIENTS_GET_FOCUS_IF_GROUP_FOCUSED: %i\n"
		  "MANUAL_PLACEMENT: %i\n"
		  "MANUAL_PLACEMENT_MOUSE_POINTER: %i\n"
		  "RAISE_ON_NEXT_FOCUS: %i\n" "RAISE_AFTER_NEXT_FOCUS: %i\n"
		  "DISPLAY_WARP: %i\n" "WARP_ON_NEXT_FOCUS: %i\n"
		  "WARP_AFTER_NEXT_FOCUS: %i\n" "EDGE_FLIP_RESISTANCE: %i\n",
		  Conf.focus.mode, Conf.dockapp_support, Conf.dock.dirmode,
		  Conf.primaryicondir, Conf.movemode, Conf.resizemode,
		  Conf.slidemode, Conf.cleanupslide, Conf.mapslide,
		  Conf.slidespeedmap, Conf.slidespeedcleanup, Conf.shadespeed,
		  Conf.backgrounds.timeout, Conf.sound,
		  Conf.button_move_resistance, Conf.autosave,
		  Conf.memory_paranoia, Conf.tooltips.enable,
		  Conf.tooltips.delay, Conf.autoraise.enable,
		  Conf.autoraise.delay, Conf.dock.startx, Conf.dock.starty,
		  Conf.save_under, Conf.menuslide, Conf.desks.num,
		  Conf.desks.dragdir, Conf.desks.dragbar_width,
		  Conf.desks.dragbar_ordering, Conf.desks.dragbar_length,
		  Conf.desks.slidein, Conf.desks.slidespeed,
		  Conf.backgrounds.hiquality, Conf.focus.transientsfollowleader,
		  Conf.focus.switchfortransientmap, a, b,
		  Conf.focus.all_new_windows_get_focus,
		  Conf.focus.new_transients_get_focus,
		  Conf.focus.new_transients_get_focus_if_group_focused,
		  Conf.manual_placement, Conf.manual_placement_mouse_pointer,
		  Conf.focus.raise_on_next_focus,
		  Conf.focus.raise_after_next_focus, Conf.warplist.enable,
		  Conf.focus.warp_on_next_focus,
		  Conf.focus.warp_after_next_focus, Conf.edge_flip_resistance);
	CommsSend(c, buf);
     }
   else if (!strcmp(w, "call_raw"))
     {
	const char         *par;
	int                 aid;

	word(s, 2, w);
	aid = atoi(w);
	par = atword(s, 3);
	ActionsCall(aid, NULL, par);
     }
   else if (!strcmp(w, "num_desks"))
     {
	word(s, 2, w);
	if (!strcmp(w, "?"))
	  {
	     char                buf[FILEPATH_LEN_MAX];

	     buf[0] = 0;
	     Esnprintf(buf, sizeof(buf), "Number of desks is %d\n",
		       Conf.desks.num);
	     CommsSend(c, buf);
	  }
	else
	  {
	     int                 i, num;
	     EWin               *const *lst;

	     Conf.desks.num = atoi(w);
	     if (Conf.desks.num <= 0)
		Conf.desks.num = 1;
	     else if (Conf.desks.num > ENLIGHTENMENT_CONF_NUM_DESKTOPS)
		Conf.desks.num = ENLIGHTENMENT_CONF_NUM_DESKTOPS;
	     lst = EwinListGetAll(&num);
	     for (i = 0; i < num; i++)
	       {
		  if (lst[i]->desktop >= Conf.desks.num)
		     MoveEwinToDesktop(lst[i], Conf.desks.num - 1);
	       }
	     if (desks.current >= Conf.desks.num)
		GotoDesktop(Conf.desks.num - 1);
	  }
     }
   else if (!strcmp(w, "get_client_info"))
     {
	char                buf[FILEPATH_LEN_MAX], none[] = "-NONE-";
	EWin               *ewin;
	unsigned int        win;

	sscanf(s, "%*s %8x", &win);
	ewin = (EWin *) FindItem(NULL, win, LIST_FINDBY_ID, LIST_TYPE_EWIN);
	if (ewin)
	  {
	     if (!ewin->icccm.wm_name)
		ewin->icccm.wm_name = none;
	     if (!ewin->icccm.wm_res_class)
		ewin->icccm.wm_res_class = none;
	     if (!ewin->icccm.wm_res_name)
		ewin->icccm.wm_res_name = none;
	     if (!ewin->icccm.wm_command)
		ewin->icccm.wm_command = none;
	     if (!ewin->icccm.wm_machine)
		ewin->icccm.wm_machine = none;
	     if (!ewin->icccm.wm_icon_name)
		ewin->icccm.wm_icon_name = none;
	     if (ewin->border)
	       {
		  Esnprintf(buf, sizeof(buf),
			    "***CLIENT***\n" "CLIENT_WIN_ID:          %8x\n"
			    "FRAME_WIN_ID:           %8x\n"
			    "FRAME_X,Y:              %5i , %5i\n"
			    "FRAME_WIDTH,HEIGHT:     %5i , %5i\n"
			    "BORDER_NAME:            %s\n"
			    "BORDER_BORDER:          %5i , %5i , %5i , %5i\n"
			    "DESKTOP_NUMBER:         %5i\n"
			    "MEMBER_OF_GROUPS:       %5i\n"
			    "DOCKED:                 %5i\n"
			    "STICKY:                 %5i\n"
			    "VISIBLE:                %5i\n"
			    "ICONIFIED:              %5i\n"
			    "SHADED:                 %5i\n"
			    "ACTIVE:                 %5i\n"
			    "LAYER:                  %5i\n"
			    "NEVER_USE_AREA:         %5i\n"
			    "FLOATING:               %5i\n"
			    "CLIENT_WIDTH,HEIGHT:    %5i , %5i\n"
			    "ICON_WIN_ID:            %8x\n"
			    "ICON_PIXMAP,MASK_ID:    %8x , %8x\n"
			    "CLIENT_GROUP_LEADER_ID: %8x\n"
			    "CLIENT_NEEDS_INPUT:     %5i\n"
			    "TRANSIENT:              %5i\n"
			    "TITLE:                  %s\n"
			    "CLASS:                  %s\n"
			    "NAME:                   %s\n"
			    "COMMAND:                %s\n"
			    "MACHINE:                %s\n"
			    "ICON_NAME:              %s\n"
			    "IS_GROUP_LEADER:        %5i\n"
			    "NO_RESIZE_HORIZONTAL:   %5i\n"
			    "NO_RESIZE_VERTICAL:     %5i\n"
			    "SHAPED:                 %5i\n"
			    "MIN_WIDTH,HEIGHT:       %5i , %5i\n"
			    "MAX_WIDTH,HEIGHT:       %5i , %5i\n"
			    "BASE_WIDTH,HEIGHT:      %5i , %5i\n"
			    "WIDTH,HEIGHT_INC:       %5i , %5i\n"
			    "ASPECT_MIN,MAX:         %5.5f , %5.5f\n"
			    "MWM_BORDER:             %5i\n"
			    "MWM_RESIZEH:            %5i\n"
			    "MWM_TITLE:              %5i\n"
			    "MWM_MENU:               %5i\n"
			    "MWM_MINIMIZE:           %5i\n"
			    "MWM_MAXIMIZE:           %5i\n"
			    "APP_STATE:              %5i\n",
			    (unsigned)ewin->client.win, (unsigned)ewin->win,
			    ewin->x, ewin->y, ewin->w, ewin->h,
			    ewin->border->name, ewin->border->border.left,
			    ewin->border->border.right,
			    ewin->border->border.top,
			    ewin->border->border.bottom, ewin->desktop,
			    ewin->num_groups, ewin->docked, ewin->sticky,
			    ewin->visible, ewin->iconified, ewin->shaded,
			    ewin->active, ewin->layer, ewin->never_use_area,
			    ewin->floating, ewin->client.w, ewin->client.h,
			    (unsigned)ewin->client.icon_win,
			    (unsigned)ewin->client.icon_pmap,
			    (unsigned)ewin->client.icon_mask,
			    (unsigned)ewin->client.group,
			    ewin->client.need_input, ewin->client.transient,
			    ewin->icccm.wm_name, ewin->icccm.wm_res_class,
			    ewin->icccm.wm_res_name, ewin->icccm.wm_command,
			    ewin->icccm.wm_machine, ewin->icccm.wm_icon_name,
			    ewin->client.is_group_leader,
			    ewin->client.no_resize_h, ewin->client.no_resize_v,
			    ewin->client.shaped, ewin->client.width.min,
			    ewin->client.height.min, ewin->client.width.max,
			    ewin->client.height.max, ewin->client.base_w,
			    ewin->client.base_h, ewin->client.w_inc,
			    ewin->client.h_inc, ewin->client.aspect_min,
			    ewin->client.aspect_max,
			    ewin->client.mwm_decor_border,
			    ewin->client.mwm_decor_resizeh,
			    ewin->client.mwm_decor_title,
			    ewin->client.mwm_decor_menu,
			    ewin->client.mwm_decor_minimize,
			    ewin->client.mwm_decor_maximize,
			    ewin->client.app_state);
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf),
			    "***CLIENT***\n" "CLIENT_WIN_ID:          %8x\n"
			    "FRAME_WIN_ID:           %8x\n"
			    "FRAME_X,Y:              %5i , %5i\n"
			    "FRAME_WIDTH,HEIGHT:     %5i , %5i\n"
			    "BORDER_NAME:            %s\n"
			    "BORDER_BORDER:          %5i , %5i , %5i , %5i\n"
			    "DESKTOP_NUMBER:         %5i\n"
			    "MEMBER_OF_GROUPS:       %5i\n"
			    "DOCKED:                 %5i\n"
			    "STICKY:                 %5i\n"
			    "VISIBLE:                %5i\n"
			    "ICONIFIED:              %5i\n"
			    "SHADED:                 %5i\n"
			    "ACTIVE:                 %5i\n"
			    "LAYER:                  %5i\n"
			    "NEVER_USE_AREA:         %5i\n"
			    "FLOATING:               %5i\n"
			    "CLIENT_WIDTH,HEIGHT:    %5i , %5i\n"
			    "ICON_WIN_ID:            %8x\n"
			    "ICON_PIXMAP,MASK_ID:    %8x , %8x\n"
			    "CLIENT_GROUP_LEADER_ID: %8x\n"
			    "CLIENT_NEEDS_INPUT:     %5i\n"
			    "TRANSIENT:              %5i\n"
			    "TITLE:                  %s\n"
			    "CLASS:                  %s\n"
			    "NAME:                   %s\n"
			    "COMMAND:                %s\n"
			    "MACHINE:                %s\n"
			    "ICON_NAME:              %s\n"
			    "IS_GROUP_LEADER:        %5i\n"
			    "NO_RESIZE_HORIZONTAL:   %5i\n"
			    "NO_RESIZE_VERTICAL:     %5i\n"
			    "SHAPED:                 %5i\n"
			    "MIN_WIDTH,HEIGHT:       %5i , %5i\n"
			    "MAX_WIDTH,HEIGHT:       %5i , %5i\n"
			    "BASE_WIDTH,HEIGHT:      %5i , %5i\n"
			    "WIDTH,HEIGHT_INC:       %5i , %5i\n"
			    "ASPECT_MIN,MAX:         %5.5f , %5.5f\n"
			    "MWM_BORDER:             %5i\n"
			    "MWM_RESIZEH:            %5i\n"
			    "MWM_TITLE:              %5i\n"
			    "MWM_MENU:               %5i\n"
			    "MWM_MINIMIZE:           %5i\n"
			    "MWM_MAXIMIZE:           %5i\n"
			    "APP_STATE:              %5i\n",
			    (unsigned)ewin->client.win, (unsigned)ewin->win,
			    ewin->x, ewin->y, ewin->w, ewin->h, none, 0, 0, 0,
			    0, ewin->desktop, ewin->num_groups, ewin->docked,
			    ewin->sticky, ewin->visible, ewin->iconified,
			    ewin->shaded, ewin->active, ewin->layer,
			    ewin->never_use_area, ewin->floating,
			    ewin->client.w, ewin->client.h,
			    (unsigned)ewin->client.icon_win,
			    (unsigned)ewin->client.icon_pmap,
			    (unsigned)ewin->client.icon_mask,
			    (unsigned)ewin->client.group,
			    ewin->client.need_input, ewin->client.transient,
			    ewin->icccm.wm_name, ewin->icccm.wm_res_class,
			    ewin->icccm.wm_res_name, ewin->icccm.wm_command,
			    ewin->icccm.wm_machine, ewin->icccm.wm_icon_name,
			    ewin->client.is_group_leader,
			    ewin->client.no_resize_h, ewin->client.no_resize_v,
			    ewin->client.shaped, ewin->client.width.min,
			    ewin->client.height.min, ewin->client.width.max,
			    ewin->client.height.max, ewin->client.base_w,
			    ewin->client.base_h, ewin->client.w_inc,
			    ewin->client.h_inc, ewin->client.aspect_min,
			    ewin->client.aspect_max,
			    ewin->client.mwm_decor_border,
			    ewin->client.mwm_decor_resizeh,
			    ewin->client.mwm_decor_title,
			    ewin->client.mwm_decor_menu,
			    ewin->client.mwm_decor_minimize,
			    ewin->client.mwm_decor_maximize,
			    ewin->client.app_state);
	       }
	     if (ewin->icccm.wm_name == none)
		ewin->icccm.wm_name = NULL;
	     if (ewin->icccm.wm_res_class == none)
		ewin->icccm.wm_res_class = NULL;
	     if (ewin->icccm.wm_res_name == none)
		ewin->icccm.wm_res_name = NULL;
	     if (ewin->icccm.wm_command == none)
		ewin->icccm.wm_command = NULL;
	     if (ewin->icccm.wm_machine == none)
		ewin->icccm.wm_machine = NULL;
	     if (ewin->icccm.wm_icon_name == none)
		ewin->icccm.wm_icon_name = NULL;
	  }
	else
	  {
	     Esnprintf(buf, sizeof(buf), "No matching EWin found\n");
	  }
	if (buf)
	  {
	     CommsSend(c, buf);
	  }
     }
   else if (!strcmp(w, "dump_info"))
     {

	char                buf[FILEPATH_LEN_MAX];
	char                buf3[FILEPATH_LEN_MAX];

	Esnprintf(buf, sizeof(buf), "stuff:\n");
	if (Mode.focuswin)
	  {
	     Esnprintf(buf3, sizeof(buf3), "mode.focuswin - %8x\n",
		       (unsigned)Mode.focuswin->client.win);
	     strcat(buf, buf3);
	  }
	if (Mode.cur_menu_mode)
	  {
	     strcat(buf, "cur_menu_mode is set\n");
	  }
	CommsSend(c, buf);
     }
   else if (!strcmp(w, "list_clients"))
     {
	char                buf[FILEPATH_LEN_MAX], *ret = NULL, none[] =
	   "-NONE-";
	EWin               *const *lst;
	int                 i, num;

	lst = EwinListGetAll(&num);
	for (i = 0; i < num; i++)
	  {
	     if (!lst[i]->icccm.wm_name)
		lst[i]->icccm.wm_name = none;
	     if (!lst[i]->icccm.wm_res_class)
		lst[i]->icccm.wm_res_class = none;
	     if (!lst[i]->icccm.wm_res_name)
		lst[i]->icccm.wm_res_name = none;
	     if (!lst[i]->icccm.wm_command)
		lst[i]->icccm.wm_command = none;
	     if (!lst[i]->icccm.wm_machine)
		lst[i]->icccm.wm_machine = none;
	     if (!lst[i]->icccm.wm_icon_name)
		lst[i]->icccm.wm_icon_name = none;
	     if (lst[i]->border)
	       {
		  Esnprintf(buf, sizeof(buf),
			    "***CLIENT***\n" "CLIENT_WIN_ID:          %8x\n"
			    "FRAME_WIN_ID:           %8x\n"
			    "FRAME_X,Y:              %5i , %5i\n"
			    "FRAME_WIDTH,HEIGHT:     %5i , %5i\n"
			    "BORDER_NAME:            %s\n"
			    "BORDER_BORDER:          %5i , %5i , %5i , %5i\n"
			    "DESKTOP_NUMBER:         %5i\n"
			    "MEMBER_OF_GROUPS:       %5i\n"
			    "DOCKED:                 %5i\n"
			    "STICKY:                 %5i\n"
			    "VISIBLE:                %5i\n"
			    "ICONIFIED:              %5i\n"
			    "SHADED:                 %5i\n"
			    "ACTIVE:                 %5i\n"
			    "LAYER:                  %5i\n"
			    "NEVER_USE_AREA:         %5i\n"
			    "FLOATING:               %5i\n"
			    "CLIENT_WIDTH,HEIGHT:    %5i , %5i\n"
			    "ICON_WIN_ID:            %8x\n"
			    "ICON_PIXMAP,MASK_ID:    %8x , %8x\n"
			    "CLIENT_GROUP_LEADER_ID: %8x\n"
			    "CLIENT_NEEDS_INPUT:     %5i\n"
			    "TRANSIENT:              %5i\n"
			    "TITLE:                  %s\n"
			    "CLASS:                  %s\n"
			    "NAME:                   %s\n"
			    "COMMAND:                %s\n"
			    "MACHINE:                %s\n"
			    "ICON_NAME:              %s\n"
			    "IS_GROUP_LEADER:        %5i\n"
			    "NO_RESIZE_HORIZONTAL:   %5i\n"
			    "NO_RESIZE_VERTICAL:     %5i\n"
			    "SHAPED:                 %5i\n"
			    "MIN_WIDTH,HEIGHT:       %5i , %5i\n"
			    "MAX_WIDTH,HEIGHT:       %5i , %5i\n"
			    "BASE_WIDTH,HEIGHT:      %5i , %5i\n"
			    "WIDTH,HEIGHT_INC:       %5i , %5i\n"
			    "ASPECT_MIN,MAX:         %5.5f , %5.5f\n"
			    "MWM_BORDER:             %5i\n"
			    "MWM_RESIZEH:            %5i\n"
			    "MWM_TITLE:              %5i\n"
			    "MWM_MENU:               %5i\n"
			    "MWM_MINIMIZE:           %5i\n"
			    "MWM_MAXIMIZE:           %5i\n"
			    "APP_STATE:              %5i\n",
			    (unsigned)lst[i]->client.win, (unsigned)lst[i]->win,
			    lst[i]->x, lst[i]->y, lst[i]->w, lst[i]->h,
			    lst[i]->border->name, lst[i]->border->border.left,
			    lst[i]->border->border.right,
			    lst[i]->border->border.top,
			    lst[i]->border->border.bottom, lst[i]->desktop,
			    lst[i]->num_groups, lst[i]->docked, lst[i]->sticky,
			    lst[i]->visible, lst[i]->iconified, lst[i]->shaded,
			    lst[i]->active, lst[i]->layer,
			    lst[i]->never_use_area, lst[i]->floating,
			    lst[i]->client.w, lst[i]->client.h,
			    (unsigned)lst[i]->client.icon_win,
			    (unsigned)lst[i]->client.icon_pmap,
			    (unsigned)lst[i]->client.icon_mask,
			    (unsigned)lst[i]->client.group,
			    lst[i]->client.need_input, lst[i]->client.transient,
			    lst[i]->icccm.wm_name, lst[i]->icccm.wm_res_class,
			    lst[i]->icccm.wm_res_name, lst[i]->icccm.wm_command,
			    lst[i]->icccm.wm_machine,
			    lst[i]->icccm.wm_icon_name,
			    lst[i]->client.is_group_leader,
			    lst[i]->client.no_resize_h,
			    lst[i]->client.no_resize_v, lst[i]->client.shaped,
			    lst[i]->client.width.min, lst[i]->client.height.min,
			    lst[i]->client.width.max, lst[i]->client.height.max,
			    lst[i]->client.base_w, lst[i]->client.base_h,
			    lst[i]->client.w_inc, lst[i]->client.h_inc,
			    lst[i]->client.aspect_min,
			    lst[i]->client.aspect_max,
			    lst[i]->client.mwm_decor_border,
			    lst[i]->client.mwm_decor_resizeh,
			    lst[i]->client.mwm_decor_title,
			    lst[i]->client.mwm_decor_menu,
			    lst[i]->client.mwm_decor_minimize,
			    lst[i]->client.mwm_decor_maximize,
			    lst[i]->client.app_state);
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf),
			    "***CLIENT***\n" "CLIENT_WIN_ID:          %8x\n"
			    "FRAME_WIN_ID:           %8x\n"
			    "FRAME_X,Y:              %5i , %5i\n"
			    "FRAME_WIDTH,HEIGHT:     %5i , %5i\n"
			    "BORDER_NAME:            %s\n"
			    "BORDER_BORDER:          %5i , %5i , %5i , %5i\n"
			    "DESKTOP_NUMBER:         %5i\n"
			    "MEMBER_OF_GROUPS:       %5i\n"
			    "DOCKED:                 %5i\n"
			    "STICKY:                 %5i\n"
			    "VISIBLE:                %5i\n"
			    "ICONIFIED:              %5i\n"
			    "SHADED:                 %5i\n"
			    "ACTIVE:                 %5i\n"
			    "LAYER:                  %5i\n"
			    "NEVER_USE_AREA:         %5i\n"
			    "FLOATING:               %5i\n"
			    "CLIENT_WIDTH,HEIGHT:    %5i , %5i\n"
			    "ICON_WIN_ID:            %8x\n"
			    "ICON_PIXMAP,MASK_ID:    %8x , %8x\n"
			    "CLIENT_GROUP_LEADER_ID: %8x\n"
			    "CLIENT_NEEDS_INPUT:     %5i\n"
			    "TRANSIENT:              %5i\n"
			    "TITLE:                  %s\n"
			    "CLASS:                  %s\n"
			    "NAME:                   %s\n"
			    "COMMAND:                %s\n"
			    "MACHINE:                %s\n"
			    "ICON_NAME:              %s\n"
			    "IS_GROUP_LEADER:        %5i\n"
			    "NO_RESIZE_HORIZONTAL:   %5i\n"
			    "NO_RESIZE_VERTICAL:     %5i\n"
			    "SHAPED:                 %5i\n"
			    "MIN_WIDTH,HEIGHT:       %5i , %5i\n"
			    "MAX_WIDTH,HEIGHT:       %5i , %5i\n"
			    "BASE_WIDTH,HEIGHT:      %5i , %5i\n"
			    "WIDTH,HEIGHT_INC:       %5i , %5i\n"
			    "ASPECT_MIN,MAX:         %5.5f , %5.5f\n"
			    "MWM_BORDER:             %5i\n"
			    "MWM_RESIZEH:            %5i\n"
			    "MWM_TITLE:              %5i\n"
			    "MWM_MENU:               %5i\n"
			    "MWM_MINIMIZE:           %5i\n"
			    "MWM_MAXIMIZE:           %5i\n"
			    "APP_STATE:              %5i\n",
			    (unsigned)lst[i]->client.win, (unsigned)lst[i]->win,
			    lst[i]->x, lst[i]->y, lst[i]->w, lst[i]->h, none, 0,
			    0, 0, 0, lst[i]->desktop, lst[i]->num_groups,
			    lst[i]->docked, lst[i]->sticky, lst[i]->visible,
			    lst[i]->iconified, lst[i]->shaded, lst[i]->active,
			    lst[i]->layer, lst[i]->never_use_area,
			    lst[i]->floating, lst[i]->client.w,
			    lst[i]->client.h, (unsigned)lst[i]->client.icon_win,
			    (unsigned)lst[i]->client.icon_pmap,
			    (unsigned)lst[i]->client.icon_mask,
			    (unsigned)lst[i]->client.group,
			    lst[i]->client.need_input, lst[i]->client.transient,
			    lst[i]->icccm.wm_name, lst[i]->icccm.wm_res_class,
			    lst[i]->icccm.wm_res_name, lst[i]->icccm.wm_command,
			    lst[i]->icccm.wm_machine,
			    lst[i]->icccm.wm_icon_name,
			    lst[i]->client.is_group_leader,
			    lst[i]->client.no_resize_h,
			    lst[i]->client.no_resize_v, lst[i]->client.shaped,
			    lst[i]->client.width.min, lst[i]->client.height.min,
			    lst[i]->client.width.max, lst[i]->client.height.max,
			    lst[i]->client.base_w, lst[i]->client.base_h,
			    lst[i]->client.w_inc, lst[i]->client.h_inc,
			    lst[i]->client.aspect_min,
			    lst[i]->client.aspect_max,
			    lst[i]->client.mwm_decor_border,
			    lst[i]->client.mwm_decor_resizeh,
			    lst[i]->client.mwm_decor_title,
			    lst[i]->client.mwm_decor_menu,
			    lst[i]->client.mwm_decor_minimize,
			    lst[i]->client.mwm_decor_maximize,
			    lst[i]->client.app_state);
	       }
	     if (lst[i]->icccm.wm_name == none)
		lst[i]->icccm.wm_name = NULL;
	     if (lst[i]->icccm.wm_res_class == none)
		lst[i]->icccm.wm_res_class = NULL;
	     if (lst[i]->icccm.wm_res_name == none)
		lst[i]->icccm.wm_res_name = NULL;
	     if (lst[i]->icccm.wm_command == none)
		lst[i]->icccm.wm_command = NULL;
	     if (lst[i]->icccm.wm_machine == none)
		lst[i]->icccm.wm_machine = NULL;
	     if (lst[i]->icccm.wm_icon_name == none)
		lst[i]->icccm.wm_icon_name = NULL;
	     if (!ret)
	       {
		  ret = Emalloc(strlen(buf) + 1);
		  ret[0] = 0;
	       }
	     else
	       {
		  ret = Erealloc(ret, strlen(ret) + strlen(buf) + 1);
	       }
	     strcat(ret, buf);
	  }
	if (ret)
	  {
	     CommsSend(c, ret);
	     Efree(ret);
	  }
     }
   else
      unknown = 1;

   if (unknown)
     {
	s1 = c->clientname;
	s2 = c->version;
	if (!s1)
	   s1 = sunknown;
	if (!s2)
	   s2 = sunknown;
	{
	   DialogOK(_("E IPC Error"),
		    _("Received Unknown Client Message.\n"
		      "Client Name:    %s\n" "Client Version: %s\n"
		      "Message Contents:\n\n" "%s\n"), s1, s2, s);
	   SoundPlay("SOUND_ERROR_IPC");
	}
     }
   Efree(s);
   EDBUG_RETURN_;
}
