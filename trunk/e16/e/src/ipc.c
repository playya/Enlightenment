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
#define DECLARE_STRUCT_BACKGROUND
#include "E.h"
#include "timestamp.h"
#include <ctype.h>

#if HAVE___ATTRIBUTE__
static void         IpcPrintf(const char *fmt, ...)
   __attribute__ ((__format__(__printf__, 1, 2)));
#else
static void         IpcPrintf(const char *fmt, ...);
#endif

typedef struct _IPCstruct
{
   void                (*func) (const char *params, Client * c);
   const char         *commandname;
   const char         *nick;
   const char         *help_text;
   const char         *extended_help_text;
}
IPCStruct;

#define SS(s) ((s) ? (s) : NoText)
static const char   NoText[] = "-NONE-";

static size_t       bufsiz;
static char        *bufptr;

static void
IpcPrintInit(void)
{
   bufsiz = 0;
   bufptr = NULL;
}

static void
IpcPrintFlush(Client * c)
{
   if (bufptr == NULL)
      return;

   CommsSend(c, bufptr);
   Efree(bufptr);
   bufsiz = 0;
   bufptr = NULL;
}

static void
IpcPrintf(const char *fmt, ...)
{
   char                tmp[FILEPATH_LEN_MAX];
   int                 len;
   va_list             args;

   va_start(args, fmt);
   len = Evsnprintf(tmp, sizeof(tmp), fmt, args);
   va_end(args);

   bufptr = Erealloc(bufptr, bufsiz + len + 1);
   strcpy(bufptr + bufsiz, tmp);
   bufsiz += len;
}

static EWin        *
IpcFindEwin(const char *windowid)
{
   unsigned int        win;

   if (!strcmp(windowid, "current"))
      return GetFocusEwin();

   if (isdigit(windowid[0]))
     {
	sscanf(windowid, "%x", &win);
	return FindEwinByChildren(win);
     }

   if (windowid[0] == '+')
      return FindEwinByPartial(windowid + 1, '+');

   if (windowid[0] == '=')
      return FindEwinByPartial(windowid + 1, '=');

   return FindEwinByPartial(windowid, '=');
}

static int
SetEwinBoolean(char *buf, int len, const char *txt, char *item,
	       const char *value, int set)
{
   int                 old, new;

   new = old = *item;		/* Remember old value */

   if (value == NULL || value[0] == '\0')
      new = !old;
   else if (!strcmp(value, "on"))
      new = 1;
   else if (!strcmp(value, "off"))
      new = 0;
   else if (!strcmp(value, "?"))
      Esnprintf(buf, len, "%s: %s", txt, (old) ? "on" : "off");
   else
      Esnprintf(buf, len, "Error: %s", value);

   if (new != old)
     {
	if (set)
	   *item = new;
	return 1;
     }

   return 0;
}

/* The IPC functions */

static void
IPC_ConfigPanel(const char *params, Client * c)
{
   int                 i = 0;
   char                param[256], buf[FILEPATH_LEN_MAX],
      buf2[FILEPATH_LEN_MAX];
   static const char  *cfg_panels[] = {
/* I just hardcoded this list form actions.c:doConfigure() -- perhaps
 * this should be tad more dynamic?? - pabs */
      "pager", "pager settings dialog",
      "focus", "focus settings dialog",
      "moveresize", "move and resize settings dialog",
      "desktops", "multiple desktop settings dialog",
      "area", "virtual desktop settings dialog",
      "placement", "window placement settings dialog",
      "icons", "icons settings dialog",
      "autoraise", "autoraise settings dialog",
      "tooltips", "tooltips settings dialog",
      "audio", "audio settings dialog",
      "fx", "special effects settings dialog",
      "bg", "background settings dialog",
      "iconbox", "iconbox settings dialog",
      "group_defaults", "default group settings dialog",
      "group_membership", "group settings for focused window",
      "remember", "list of open remembered windows",
      "miscellaneous", "miscellaneous settings dialog",
      0
   };

   buf[0] = 0;
   buf2[0] = 0;
   param[0] = 0;
   if (params)
     {
	word(params, 1, param);
	if (!strcmp(param, "?"))
	  {
	     for (i = 0; cfg_panels[i]; i += 2)
	       {
		  Esnprintf(buf2, sizeof(buf2), "%s : %s\n", cfg_panels[i],
			    cfg_panels[i + 1]);
		  strcat(buf, buf2);
	       }
	     if (strlen(buf))
		CommsSend(c, buf);
	  }
	else
	  {
	     ActionsCall(ACTION_CONFIG, NULL, params);
	  }
     }
   else
     {
	CommsSend(c, "Error: no panel specified");
     }
}

static void
IPC_Xinerama(const char *params, Client * c)
{
   params = NULL;
#ifdef HAS_XINERAMA
   if (xinerama_active)
     {
	XineramaScreenInfo *screens;
	int                 num, i;
	char                stufftosend[4096];

	screens = XineramaQueryScreens(disp, &num);

	strcpy(stufftosend, "");
	for (i = 0; i < num; i++)
	  {
	     char                s[1024];

	     sprintf(s,
		     "Head %d\nscreen # %d\nx origin: %d\ny origin: %d\n"
		     "width: %d\nheight: %d\n\n", i, screens[i].screen_number,
		     screens[i].x_org, screens[i].y_org, screens[i].width,
		     screens[i].height);
	     strcat(stufftosend, s);
	  }
	CommsSend(c, stufftosend);
	XFree(screens);
     }
   else
     {
	CommsSend(c, "Xinerama is not active on your system");
     }
#else
   CommsSend(c, "Xinerama is disabled on your system");
#endif
}

static void
IPC_Nop(const char *params, Client * c)
{
   CommsSend(c, "nop");
   params = NULL;
}

static void
IPC_Remember(const char *params, Client * c)
{
   char                buf[FILEPATH_LEN_MAX];
   char                param[FILEPATH_LEN_MAX];

   buf[0] = 0;
   param[0] = 0;

   if (params)
     {
	Window              win = 0;
	EWin               *ewin;

	sscanf(params, "%lx", &win);
	ewin = FindItem(NULL, (int)win, LIST_FINDBY_ID, LIST_TYPE_EWIN);
	if (ewin)
	  {
	     params = atword(params, 2);
	     word(params, 1, param);
	     while (params)
	       {
		  if (!strcmp((char *)param, "all"))
		    {
		       SnapshotEwinAll(ewin);
		       break;
		    }
		  else if (!strcmp((char *)param, "none"))
		     UnsnapshotEwin(ewin);
		  else if (!strcmp((char *)param, "border"))
		     SnapshotEwinBorder(ewin);
		  else if (!strcmp((char *)param, "desktop"))
		     SnapshotEwinDesktop(ewin);
		  else if (!strcmp((char *)param, "size"))
		     SnapshotEwinSize(ewin);
		  else if (!strcmp((char *)param, "location"))
		     SnapshotEwinLocation(ewin);
		  else if (!strcmp((char *)param, "layer"))
		     SnapshotEwinLayer(ewin);
		  else if (!strcmp((char *)param, "sticky"))
		     SnapshotEwinSticky(ewin);
		  else if (!strcmp((char *)param, "icon"))
		     SnapshotEwinIcon(ewin);
		  else if (!strcmp((char *)param, "shade"))
		     SnapshotEwinShade(ewin);
		  else if (!strcmp((char *)param, "group"))
		     SnapshotEwinGroups(ewin, 1);
		  else if (!strcmp((char *)param, "command"))
		     SnapshotEwinCmd(ewin);
		  else if (!strcmp((char *)param, "dialog"))
		     SnapshotEwinDialog(ewin);
		  params = atword(params, 2);
		  word(params, 1, param);
	       }
	     SaveSnapInfo();
	  }
	else
	   Esnprintf(buf, sizeof(buf), "Error: no window found");
     }
   else
      Esnprintf(buf, sizeof(buf), "Error: no parameters");

   if (buf[0])
      CommsSend(c, buf);
}

static void
IPC_DockConfig(const char *params, Client * c)
{
   char                buf[FILEPATH_LEN_MAX];

   buf[0] = 0;

   if (params)
     {
	char                param1[FILEPATH_LEN_MAX];
	char                param2[FILEPATH_LEN_MAX];
	char                param3[FILEPATH_LEN_MAX];

	param1[0] = 0;
	param2[0] = 0;
	param3[0] = 0;

	word(params, 1, param1);
	word(params, 2, param2);

	if (!strcmp(param1, "start_pos"))
	  {
	     if (param2[0])
	       {
		  if (!strcmp(param2, "?"))
		    {
		       Esnprintf(buf, sizeof(buf), "dock_startposition: %d %d",
				 Conf.dock.startx, Conf.dock.starty);
		    }
		  else
		    {
		       word(params, 3, param3);
		       if (param3[0])
			 {
			    Conf.dock.startx = atoi(param2);
			    Conf.dock.starty = atoi(param3);
			 }
		       else
			 {
			    Esnprintf(buf, sizeof(buf),
				      "Error: no y coordinate");

			 }
		    }
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf), "Error: no operation specified");
	       }
	  }
	else if (!strcmp(param1, "direction"))
	  {
	     if (param2[0])
	       {
		  if (!strcmp(param2, "?"))
		    {
		       if (Conf.dock.dirmode == DOCK_LEFT)
			 {
			    Esnprintf(buf, sizeof(buf), "dock_dir: left");
			 }
		       else if (Conf.dock.dirmode == DOCK_RIGHT)
			 {
			    Esnprintf(buf, sizeof(buf), "dock_dir: right");
			 }
		       else if (Conf.dock.dirmode == DOCK_UP)
			 {
			    Esnprintf(buf, sizeof(buf), "dock_dir: up");
			 }
		       else if (Conf.dock.dirmode == DOCK_DOWN)
			 {
			    Esnprintf(buf, sizeof(buf), "dock_dir: down");
			 }
		       else
			 {
			    Esnprintf(buf, sizeof(buf),
				      "Error: I have NO " "idea what direction "
				      "this thing is going");
			 }
		    }
		  else if (!strcmp(param2, "left"))
		    {
		       Conf.dock.dirmode = DOCK_LEFT;
		    }
		  else if (!strcmp(param2, "right"))
		    {
		       Conf.dock.dirmode = DOCK_RIGHT;
		    }
		  else if (!strcmp(param2, "up"))
		    {
		       Conf.dock.dirmode = DOCK_UP;
		    }
		  else if (!strcmp(param2, "down"))
		    {
		       Conf.dock.dirmode = DOCK_DOWN;
		    }
		  else
		    {
		       Esnprintf(buf, sizeof(buf),
				 "Error: unknown direction " "specified");
		    }
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf), "Error: no operation specified");
	       }
	  }
	else if (!strcmp(param1, "support"))
	  {
	     if (param2[0])
	       {
		  if (!strcmp(param2, "?"))
		    {
		       Esnprintf(buf, sizeof(buf), "dockapp support: %s",
				 (Conf.dockapp_support) ? "enabled" :
				 "disabled");
		    }
		  else if ((!strcmp(param2, "on"))
			   || (!strcmp(param2, "enable")))
		    {
		       Conf.dockapp_support = 1;
		       Esnprintf(buf, sizeof(buf), "dockapp support: enabled");
		    }
		  else if ((!strcmp(param2, "off"))
			   || (!strcmp(param2, "disable")))
		    {
		       Conf.dockapp_support = 0;
		       Esnprintf(buf, sizeof(buf), "dockapp support: disabled");
		    }
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf), "Error: no operation specified");
	       }
	  }
	else
	  {
	     Esnprintf(buf, sizeof(buf), "Error: unknown mode");
	  }
     }
   else
     {
	Esnprintf(buf, sizeof(buf), "Error: no mode given");
     }

   if (buf[0])
      CommsSend(c, buf);
}

static void
IPC_GeneralInfo(const char *params, Client * c)
{
   char                buf[FILEPATH_LEN_MAX];

   buf[0] = 0;

   if (params)
     {
	if (!strcmp(params, "screen_size"))
	  {
	     Esnprintf(buf, sizeof(buf), "screen_size: %d %d", VRoot.w,
		       VRoot.h);
	  }
	else
	  {
	     Esnprintf(buf, sizeof(buf), "Error: unknown info requested");
	  }
     }
   else
     {
	Esnprintf(buf, sizeof(buf), "Error: no info requested");
     }

   if (buf[0])
      CommsSend(c, buf);
}

static void
IPC_Button(const char *params, Client * c)
{

   char                buf[FILEPATH_LEN_MAX];

   buf[0] = 0;

   if (params)
     {
	char                param1[FILEPATH_LEN_MAX];
	char                param2[FILEPATH_LEN_MAX];
	char                param3[FILEPATH_LEN_MAX];

	param1[0] = 0;
	param2[0] = 0;
	param3[0] = 0;

	word(params, 1, param1);
	word(params, 2, param2);
	if (param2[0])
	  {
	     if (!strcmp(param2, "create"))
	       {
	       }
	     else if (!strcmp(param2, "delete"))
	       {
		  Button             *b;

		  b = (Button *) FindItem(param1, 0, LIST_FINDBY_NAME,
					  LIST_TYPE_BUTTON);
		  if (b)
		     ButtonDestroy(b);
	       }
	     else if (!strcmp(param2, "modify"))
	       {
	       }
	     else if (!strcmp(param2, "ref_count"))
	       {
		  Button             *b;

		  b = (Button *) FindItem(param1, 0, LIST_FINDBY_NAME,
					  LIST_TYPE_BUTTON);
		  if (b)
		     Esnprintf(buf, sizeof(buf), "%u references remain",
			       ButtonGetRefcount(b));
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf), "Error: no cursor specified");
	       }
	  }
	else
	  {
	     Esnprintf(buf, sizeof(buf), "Error: unknown operation specified");
	  }
     }
   else
     {
	Esnprintf(buf, sizeof(buf), "Error: no operation specified");
     }

   if (buf[0])
      CommsSend(c, buf);
}

static void
IPC_Background(const char *params, Client * c)
{
   char                buf[FILEPATH_LEN_MAX];
   char                name[FILEPATH_LEN_MAX];
   char                type[FILEPATH_LEN_MAX];
   char                valu[FILEPATH_LEN_MAX];
   Background         *bg = NULL;
   int                 r, g, b;

   buf[0] = 0;

   name[0] = 0;
   type[0] = 0;
   valu[0] = 0;

   word(params, 1, name);
   word(params, 2, type);

   if (params)
     {
	if (type[0])
	  {
	     if (!strcmp(type, "?"))
	       {
		  /* query background values */

		  bg = (Background *) FindItem(name, 0, LIST_FINDBY_NAME,
					       LIST_TYPE_BACKGROUND);

		  if (bg)
		    {
		       EGetColor(&(bg->bg_solid), &r, &g, &b);
		       Esnprintf(buf, sizeof(buf),
				 "%s ref_count %u keepim %u\n"
				 " bg.solid\t %i %i %i \n"
				 " bg.file\t %s \ttop.file\t %s \n"
				 " bg.tile\t %i \n"
				 " bg.keep_aspect\t %i \ttop.keep_aspect\t %i \n"
				 " bg.xjust\t %i \ttop.xjust\t %i \n"
				 " bg.yjust\t %i \ttop.yjust\t %i \n"
				 " bg.xperc\t %i \ttop.xperc\t %i \n"
				 " bg.yperc\t %i \ttop.yperc\t %i \n", bg->name,
				 bg->ref_count, bg->keepim, r, g, b,
				 bg->bg.file, bg->top.file, bg->bg_tile,
				 bg->bg.keep_aspect, bg->top.keep_aspect,
				 bg->bg.xjust, bg->top.xjust, bg->bg.yjust,
				 bg->top.yjust, bg->bg.xperc, bg->top.xperc,
				 bg->bg.yperc, bg->top.yperc);
		    }
		  else
		     Esnprintf(buf, sizeof(buf),
			       "Error: background '%s' does not exist.", name);
	       }
	     else
	       {
		  /* create / modify background */

		  bg = (Background *) FindItem(name, 0, LIST_FINDBY_NAME,
					       LIST_TYPE_BACKGROUND);

		  if (!bg)
		    {
		       bg = BackgroundCreate(Estrdup(name), NULL, NULL, 0, 0, 0,
					     0, 0, 0, NULL, 0, 0, 0, 0, 0);
		    }
		  if (!bg)
		     Esnprintf(buf, sizeof(buf),
			       "Error: could not create background '%s'.",
			       name);
		  else
		    {
		       word(params, 3, valu);
		       if (!strcmp(type, "bg.solid"))
			 {
			    char                R[3], G[3], B[3];	/* Crash ??? */

			    R[0] = 0;
			    G[0] = 0;
			    B[0] = 0;

			    word(params, 3, R);
			    word(params, 4, G);
			    word(params, 5, B);

			    ESetColor(&(bg->bg_solid), atoi(R), atoi(G),
				      atoi(B));
			 }
		       else if (!strcmp(type, "bg.file"))
			 {
			    if (bg->bg.file)
			       Efree(bg->bg.file);
			    bg->bg.file = Estrdup(valu);
			 }
		       else if (!strcmp(type, "bg.tile"))
			 {
			    bg->bg_tile = atoi(valu);
			 }
		       else if (!strcmp(type, "bg.keep_aspect"))
			 {
			    bg->bg.keep_aspect = atoi(valu);
			 }
		       else if (!strcmp(type, "bg.xjust"))
			 {
			    bg->bg.xjust = atoi(valu);
			 }
		       else if (!strcmp(type, "bg.yjust"))
			 {
			    bg->bg.yjust = atoi(valu);
			 }
		       else if (!strcmp(type, "bg.xperc"))
			 {
			    bg->bg.xperc = atoi(valu);
			 }
		       else if (!strcmp(type, "bg.yperc"))
			 {
			    bg->bg.yperc = atoi(valu);
			 }
		       else if (!strcmp(type, "top.file"))
			 {
			    if (bg->top.file)
			       Efree(bg->top.file);
			    bg->top.file = Estrdup(valu);
			 }
		       else if (!strcmp(type, "top.keep_aspect"))
			 {
			    bg->top.keep_aspect = atoi(valu);
			 }
		       else if (!strcmp(type, "top.xjust"))
			 {
			    bg->top.xjust = atoi(valu);
			 }
		       else if (!strcmp(type, "top.yjust"))
			 {
			    bg->top.yjust = atoi(valu);
			 }
		       else if (!strcmp(type, "top.xperc"))
			 {
			    bg->top.xperc = atoi(valu);
			 }
		       else if (!strcmp(type, "top.yperc"))
			 {
			    bg->top.yperc = atoi(valu);
			 }
		       else
			 {
			    Esnprintf(buf, sizeof(buf),
				      "Error: unknown background value type '%s'.",
				      type);
			 }
		    }
	       }
	  }
	else
	  {
	     /* delete background */
	     bg = (Background *) FindItem(name, 0, LIST_FINDBY_NAME,
					  LIST_TYPE_BACKGROUND);

	     if (bg)
	       {
		  if (bg->ref_count == 0)
		     BackgroundDestroy(bg);
		  else
		     Esnprintf(buf, sizeof(buf),
			       "Error: ref_count for background '%s' is %u.",
			       name, bg->ref_count);
	       }
	     else
		Esnprintf(buf, sizeof(buf),
			  "Error: background '%s' does not exist.", name);
	  }
     }
   else
     {
	/* show all backgrounds */
	Background        **lst;
	char                buf2[FILEPATH_LEN_MAX];
	char               *buf3 = NULL;
	int                 num, i;

	buf2[0] = 0;
	lst = (Background **) ListItemType(&num, LIST_TYPE_BACKGROUND);
	if (lst)
	  {
	     for (i = 0; i < num; i++)
	       {
		  buf2[0] = 0;
		  Esnprintf(buf2, sizeof(buf2), "%s\n", lst[i]->name);
		  if (buf3)
		     buf3 = realloc(buf3, strlen(buf3) + strlen(buf2) + 1);
		  else
		    {
		       buf3 = malloc(strlen(buf2) + 1);
		       buf3[0] = 0;
		    }
		  strcat(buf3, buf2);
	       }
	     if (buf3)
	       {
		  CommsSend(c, buf3);
		  Efree(buf3);
	       }
	     Efree(lst);
	  }
     }

   if (buf[0])
      CommsSend(c, buf);
}

static void
IPC_Border(const char *params, Client * c)
{
   char                buf[FILEPATH_LEN_MAX];

   buf[0] = 0;

   if (params)
     {
	char                param1[FILEPATH_LEN_MAX];
	char                param2[FILEPATH_LEN_MAX];
	char                param3[FILEPATH_LEN_MAX];

	param1[0] = 0;
	param2[0] = 0;
	param3[0] = 0;

	word(params, 1, param1);
	word(params, 2, param2);
	if (param2[0])
	  {
	     if (!strcmp(param2, "create"))
	       {
	       }
	     else if (!strcmp(param2, "delete"))
	       {
		  Border             *b;

		  b = (Border *) FindItem(param1, 0, LIST_FINDBY_NAME,
					  LIST_TYPE_BORDER);
		  if (b)
		     FreeBorder(b);
	       }
	     else if (!strcmp(param2, "modify"))
	       {
	       }
	     else if (!strcmp(param2, "ref_count"))
	       {
		  Border             *b;

		  b = (Border *) FindItem(param1, 0, LIST_FINDBY_NAME,
					  LIST_TYPE_BORDER);
		  if (b)
		     Esnprintf(buf, sizeof(buf), "%u references remain",
			       b->ref_count);
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf), "Error: no cursor specified");
	       }
	  }
	else
	  {
	     Esnprintf(buf, sizeof(buf), "Error: unknown operation specified");
	  }
     }
   else
     {
	Esnprintf(buf, sizeof(buf), "Error: no operation specified");
     }

   if (buf[0])
      CommsSend(c, buf);
}

static void
IPC_Cursor(const char *params, Client * c)
{
   char                buf[FILEPATH_LEN_MAX];

   buf[0] = 0;

   if (params)
     {
	char                param1[FILEPATH_LEN_MAX];
	char                param2[FILEPATH_LEN_MAX];
	char                param3[FILEPATH_LEN_MAX];

	param1[0] = 0;
	param2[0] = 0;
	param3[0] = 0;

	word(params, 1, param1);
	word(params, 2, param2);
	if (param2[0])
	  {
	     if (!strcmp(param2, "create"))
	       {
	       }
	     else if (!strcmp(param2, "delete"))
	       {
		  ECursor            *ec;

		  ec = (ECursor *) FindItem(param1, 0, LIST_FINDBY_NAME,
					    LIST_TYPE_ECURSOR);
		  if (ec)
		     ECursorDestroy(ec);
	       }
	     else if (!strcmp(param2, "modify"))
	       {
	       }
	     else if (!strcmp(param2, "ref_count"))
	       {
		  ECursor            *ec;

		  ec = (ECursor *) FindItem(param1, 0, LIST_FINDBY_NAME,
					    LIST_TYPE_ECURSOR);
		  if (ec)
		     Esnprintf(buf, sizeof(buf), "%u references remain",
			       ECursorGetRefcount(ec));
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf), "Error: no cursor specified");
	       }
	  }
	else
	  {
	     Esnprintf(buf, sizeof(buf), "Error: unknown operation specified");
	  }
     }
   else
     {
	Esnprintf(buf, sizeof(buf), "Error: no operation specified");
     }

   if (buf[0])
      CommsSend(c, buf);
}

static void
IPC_TextClass(const char *params, Client * c)
{
   char                pq;
   char                buf[FILEPATH_LEN_MAX];

   buf[0] = 0;

   if (params)
     {
	char                param1[FILEPATH_LEN_MAX];
	char                param2[FILEPATH_LEN_MAX];
	char                param3[FILEPATH_LEN_MAX];

	param1[0] = 0;
	param2[0] = 0;
	param3[0] = 0;

	word(params, 1, param1);
	word(params, 2, param2);
	if (param2[0])
	  {
	     if (!strcmp(param2, "create"))
	       {
	       }
	     else if (!strcmp(param2, "delete"))
	       {
		  TextClass          *t;

		  t = (TextClass *) FindItem(param1, 0, LIST_FINDBY_NAME,
					     LIST_TYPE_TCLASS);
		  if (t)
		     DeleteTclass(t);
	       }
	     else if (!strcmp(param2, "modify"))
	       {
	       }
	     else if (!strcmp(param2, "apply"))
	       {
		  TextClass          *t;

		  t = (TextClass *) FindItem(param1, 0, LIST_FINDBY_NAME,
					     LIST_TYPE_TCLASS);
		  if (t)
		    {
		       int                 state;
		       int                 x, y;
		       const char         *txt;
		       Window              win;

		       word(params, 3, param3);
		       win = (Window) strtol(param3, (char **)NULL, 0);
		       word(params, 4, param3);
		       x = atoi(param3);
		       word(params, 5, param3);
		       y = atoi(param3);
		       word(params, 6, param3);
		       state = STATE_NORMAL;
		       if (!strcmp(param3, "normal"))
			  state = STATE_NORMAL;
		       else if (!strcmp(param3, "hilited"))
			  state = STATE_HILITED;
		       else if (!strcmp(param3, "clicked"))
			  state = STATE_CLICKED;
		       else if (!strcmp(param3, "disabled"))
			  state = STATE_DISABLED;
		       txt = atword(params, 7);
		       pq = Mode.queue_up;
		       Mode.queue_up = 0;
		       if (txt)
			  TextDraw(t, win, 0, 0, state, txt, x, y, 99999, 99999,
				   17, 0);
		       Mode.queue_up = pq;
		    }
	       }
	     else if (!strcmp(param2, "query_size"))
	       {
		  TextClass          *t;

		  t = (TextClass *) FindItem(param1, 0, LIST_FINDBY_NAME,
					     LIST_TYPE_TCLASS);
		  if (t)
		    {
		       int                 w, h;
		       const char         *txt;

		       txt = atword(params, 3);
		       if (txt)
			 {
			    TextSize(t, 0, 0, STATE_NORMAL, txt, &w, &h, 17);
			    Esnprintf(buf, sizeof(buf), "%i %i", w, h);
			 }
		       else
			  Esnprintf(buf, sizeof(buf), "0 0");
		    }
		  else
		     Esnprintf(buf, sizeof(buf), "TextClass %s not found",
			       param1);
	       }
	     else if (!strcmp(param2, "query"))
	       {
		  TextClass          *t;

		  t = (TextClass *) FindItem(param1, 0, LIST_FINDBY_NAME,
					     LIST_TYPE_TCLASS);
		  if (t)
		     Esnprintf(buf, sizeof(buf), "TextClass %s found", t->name);
		  else
		     Esnprintf(buf, sizeof(buf), "TextClass %s not found",
			       param1);
	       }
	     else if (!strcmp(param2, "ref_count"))
	       {
		  TextClass          *t;

		  t = (TextClass *) FindItem(param1, 0, LIST_FINDBY_NAME,
					     LIST_TYPE_TCLASS);
		  if (t)
		     Esnprintf(buf, sizeof(buf), "%u references remain.",
			       t->ref_count);
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf),
			    "Error: unknown operation specified");
	       }
	  }
	else
	  {
	     Esnprintf(buf, sizeof(buf), "Error: no class specified");
	  }
     }
   else
     {
	Esnprintf(buf, sizeof(buf), "Error: no operation specified");
     }

   if (buf[0])
      CommsSend(c, buf);
}

static void
IPC_ColorModifierClass(const char *params, Client * c)
{
   char                buf[FILEPATH_LEN_MAX];

   buf[0] = 0;

   if (params)
     {
	char                param1[FILEPATH_LEN_MAX];
	char                param2[FILEPATH_LEN_MAX];
	char                param3[FILEPATH_LEN_MAX];

	param1[0] = 0;
	param2[0] = 0;
	param3[0] = 0;

	word(params, 1, param1);
	word(params, 2, param2);
	if (param2[0])
	  {
	     if (!strcmp(param1, "create"))
	       {
	       }
	     else if (!strcmp(param2, "delete"))
	       {
		  ColorModifierClass *cm;

		  cm = (ColorModifierClass *) FindItem(param1, 0,
						       LIST_FINDBY_NAME,
						       LIST_TYPE_COLORMODIFIER);
		  if (cm)
		     FreeCMClass(cm);
	       }
	     else if (!strcmp(param2, "modify"))
	       {
	       }
	     else if (!strcmp(param2, "ref_count"))
	       {
		  ColorModifierClass *cm;

		  cm = (ColorModifierClass *) FindItem(param1, 0,
						       LIST_FINDBY_NAME,
						       LIST_TYPE_COLORMODIFIER);
		  if (cm)
		     Esnprintf(buf, sizeof(buf), "%u references remain.",
			       cm->ref_count);
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf),
			    "Error: unknown operation specified");
	       }
	  }
	else
	  {
	     Esnprintf(buf, sizeof(buf), "Error: no class specified");
	  }
     }
   else
     {
	Esnprintf(buf, sizeof(buf), "Error: no operation specified");
     }

   if (buf[0])
      CommsSend(c, buf);
}

static void
IPC_ActionClass(const char *params, Client * c)
{
   char                buf[FILEPATH_LEN_MAX];

   buf[0] = 0;

   if (params)
     {
	char                param1[FILEPATH_LEN_MAX];
	char                param2[FILEPATH_LEN_MAX];
	char                param3[FILEPATH_LEN_MAX];

	param1[0] = 0;
	param2[0] = 0;
	param3[0] = 0;

	word(params, 1, param1);
	word(params, 2, param2);
	if (param2[0])
	  {
	     if (!strcmp(param1, "create"))
	       {
	       }
	     else if (!strcmp(param2, "delete"))
	       {
		  ActionClass        *a;

		  a = (ActionClass *) FindItem(param1, 0, LIST_FINDBY_NAME,
					       LIST_TYPE_ACLASS);
		  if (a)
		     RemoveActionClass(a);
	       }
	     else if (!strcmp(param2, "modify"))
	       {
	       }
	     else if (!strcmp(param2, "ref_count"))
	       {
		  ActionClass        *a;

		  a = (ActionClass *) FindItem(param1, 0, LIST_FINDBY_NAME,
					       LIST_TYPE_ACLASS);
		  if (a)
		     Esnprintf(buf, sizeof(buf), "%u references remain.",
			       a->ref_count);
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf),
			    "Error: unknown operation specified");
	       }
	  }
	else
	  {
	     Esnprintf(buf, sizeof(buf), "Error: no class specified");
	  }
     }
   else
     {
	Esnprintf(buf, sizeof(buf), "Error: no operation specified");
     }

   if (buf[0])
      CommsSend(c, buf);
}

static void
IPC_ImageClass(const char *params, Client * c)
{
   char                pq;
   char                buf[FILEPATH_LEN_MAX];

   buf[0] = 0;

   if (params)
     {
	char                param1[FILEPATH_LEN_MAX];
	char                param2[FILEPATH_LEN_MAX];
	char                param3[FILEPATH_LEN_MAX];

	param1[0] = 0;
	param2[0] = 0;
	param3[0] = 0;

	word(params, 1, param1);
	word(params, 2, param2);
	if (param2[0])
	  {
	     if (!strcmp(param1, "create"))
	       {
	       }
	     else if (!strcmp(param2, "delete"))
	       {
		  ImageClass         *i;

		  i = (ImageClass *) FindItem(param1, 0, LIST_FINDBY_NAME,
					      LIST_TYPE_ICLASS);
		  if (i)
		     FreeImageClass(i);
	       }
	     else if (!strcmp(param2, "modify"))
	       {
	       }
	     else if (!strcmp(param2, "free_pixmap"))
	       {
		  Pixmap              p;

		  word(params, 3, param3);
		  p = (Pixmap) strtol(param3, (char **)NULL, 0);
		  imlib_free_pixmap_and_mask(p);
	       }
	     else if (!strcmp(param2, "get_padding"))
	       {
		  ImageClass         *iclass;

		  iclass =
		     (ImageClass *) FindItem(param1, 0, LIST_FINDBY_NAME,
					     LIST_TYPE_ICLASS);
		  if (iclass)
		     Esnprintf(buf, sizeof(buf), "%i %i %i %i",
			       iclass->padding.left, iclass->padding.right,
			       iclass->padding.top, iclass->padding.bottom);
		  else
		     Esnprintf(buf, sizeof(buf),
			       "Error: Imageclass does not exist");
	       }
	     else if (!strcmp(param2, "get_image_size"))
	       {
		  ImageClass         *iclass;

		  iclass =
		     (ImageClass *) FindItem(param1, 0, LIST_FINDBY_NAME,
					     LIST_TYPE_ICLASS);
		  if (iclass)
		    {
		       Imlib_Image        *im = NULL;

		       if (iclass->norm.normal->im_file)
			 {
			    if (!iclass->norm.normal->real_file)
			       iclass->norm.normal->real_file =
				  FindFile(iclass->norm.normal->im_file);
			    if (iclass->norm.normal->real_file)
			       im =
				  imlib_load_image(iclass->norm.normal->
						   real_file);
			    if (im)
			      {
				 imlib_context_set_image(im);
				 Esnprintf(buf, sizeof(buf), "%i %i",
					   imlib_image_get_width(),
					   imlib_image_get_height());
				 imlib_free_image();
			      }
			    else
			       Esnprintf(buf, sizeof(buf),
					 "Error: Image does not exist");
			 }
		       else
			  Esnprintf(buf, sizeof(buf),
				    "Error: Image does not exist");
		    }
		  else
		     Esnprintf(buf, sizeof(buf),
			       "Error: Imageclass does not exist");
	       }
	     else if (!strcmp(param2, "apply"))
	       {
		  ImageClass         *iclass;

		  iclass =
		     (ImageClass *) FindItem(param1, 0, LIST_FINDBY_NAME,
					     LIST_TYPE_ICLASS);
		  if (iclass)
		    {
		       Window              win;
		       char                state[20];
		       const char         *winptr, *hptr;
		       int                 st, w = -1, h = -1;

		       winptr = atword(params, 3);
		       word(params, 4, state);
		       win = (Window) strtol(winptr, (char **)NULL, 0);
		       if (!strcmp(state, "hilited"))
			  st = STATE_HILITED;
		       else if (!strcmp(state, "clicked"))
			  st = STATE_CLICKED;
		       else if (!strcmp(state, "disabled"))
			  st = STATE_DISABLED;
		       else
			  st = STATE_NORMAL;
		       if ((hptr = atword(params, 6)))
			 {
			    w = (int)strtol(atword(params, 5), (char **)NULL,
					    0);
			    h = (int)strtol(hptr, (char **)NULL, 0);
			 }
		       pq = Mode.queue_up;
		       Mode.queue_up = 0;
		       IclassApply(iclass, win, w, h, 0, 0, st, 0, ST_UNKNWN);
		       Mode.queue_up = pq;
		    }
	       }
	     else if (!strcmp(param2, "apply_copy"))
	       {
		  ImageClass         *iclass;

		  iclass =
		     (ImageClass *) FindItem(param1, 0, LIST_FINDBY_NAME,
					     LIST_TYPE_ICLASS);
		  if (iclass)
		    {
		       Window              win;
		       char                state[20];
		       const char         *winptr, *hptr;
		       int                 st, w = -1, h = -1;

		       winptr = atword(params, 3);
		       word(params, 4, state);
		       win = (Window) strtol(winptr, (char **)NULL, 0);
		       if (!strcmp(state, "hilited"))
			  st = STATE_HILITED;
		       else if (!strcmp(state, "clicked"))
			  st = STATE_CLICKED;
		       else if (!strcmp(state, "disabled"))
			  st = STATE_DISABLED;
		       else
			  st = STATE_NORMAL;
		       if (!(hptr = atword(params, 6)))
			  Esnprintf(buf, sizeof(buf),
				    "Error:  missing width and/or height");
		       else
			 {
			    PmapMask            pmm;

			    w = (int)strtol(atword(params, 5), (char **)NULL,
					    0);
			    h = (int)strtol(hptr, (char **)NULL, 0);
			    pq = Mode.queue_up;
			    Mode.queue_up = 0;
			    IclassApplyCopy(iclass, win, w, h, 0, 0, st, &pmm,
					    1, ST_UNKNWN);
			    Mode.queue_up = pq;
			    Esnprintf(buf, sizeof(buf), "0x%08x 0x%08x",
				      (unsigned)pmm.pmap, (unsigned)pmm.mask);
/*			    FreePmapMask(&pmm);		??? */
			 }
		    }
	       }
	     else if (!strcmp(param2, "ref_count"))
	       {
		  ImageClass         *i;

		  i = (ImageClass *) FindItem(param1, 0, LIST_FINDBY_NAME,
					      LIST_TYPE_ICLASS);
		  if (i)
		     Esnprintf(buf, sizeof(buf), "%u references remain",
			       i->ref_count);
	       }
	     else if (!strcmp(param2, "query"))
	       {
		  ImageClass         *i;

		  i = (ImageClass *) FindItem(param1, 0, LIST_FINDBY_NAME,
					      LIST_TYPE_ICLASS);
		  if (i)
		     Esnprintf(buf, sizeof(buf), "ImageClass %s found",
			       i->name);
		  else
		     Esnprintf(buf, sizeof(buf), "ImageClass %s not found",
			       param1);
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf),
			    "Error: unknown operation specified");
	       }
	  }
	else
	  {
	     Esnprintf(buf, sizeof(buf), "Error: no class specified");
	  }
     }
   else
     {
	Esnprintf(buf, sizeof(buf), "Error: no operation specified");
     }

   if (buf[0])
      CommsSend(c, buf);
}

static void
IPC_SoundClass(const char *params, Client * c)
{
   char                buf[FILEPATH_LEN_MAX];

   buf[0] = 0;

   if (params)
     {
	char                param1[FILEPATH_LEN_MAX];
	char                param2[FILEPATH_LEN_MAX];
	char                param3[FILEPATH_LEN_MAX];
	SoundClass         *sc;

	param1[0] = 0;
	param2[0] = 0;
	param3[0] = 0;

	word(params, 1, param1);
	word(params, 2, param2);
	if (param2[0])
	  {
	     if (!strcmp(param1, "create"))
	       {
		  word(params, 3, param3);
		  if (param3[0])
		    {
		       sc = SclassCreate(param1, param2);
		    }
		  else
		    {
		       Esnprintf(buf, sizeof(buf), "Error: no file specified");
		    }
	       }
	     else if (!strcmp(param1, "delete"))
	       {
		  SoundFree((char *)param2);
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf),
			    "Error: unknown operation specified");
	       }
	  }
	else
	  {
	     Esnprintf(buf, sizeof(buf), "Error: no class specified");
	  }
     }
   else
     {
	Esnprintf(buf, sizeof(buf), "Error: no operation specified");
     }

   if (buf[0])
      CommsSend(c, buf);
}

static void
IPC_PlaySoundClass(const char *params, Client * c)
{
   char                buf[FILEPATH_LEN_MAX];

   buf[0] = 0;

   if (params)
     {
	if (SoundPlay((char *)params))
	   Esnprintf(buf, sizeof(buf), "Error: unknown soundclass selected");
     }
   else
     {
	Esnprintf(buf, sizeof(buf), "Error: no soundclass selected");
     }

   if (buf[0])
      CommsSend(c, buf);
}

static void
IPC_ListClassMembers(const char *params, Client * c)
{
   char               *buf = NULL;
   char                buf2[FILEPATH_LEN_MAX];
   int                 num, i;

   if (params)
     {
	if (!strcmp(params, "backgrounds"))
	  {

	     Background        **lst;

	     lst = (Background **) ListItemType(&num, LIST_TYPE_BACKGROUND);
	     for (i = 0; i < num; i++)
	       {
		  buf2[0] = 0;
		  Esnprintf(buf2, sizeof(buf2), "%s\n", lst[i]->name);
		  if (buf)
		     buf = realloc(buf, strlen(buf) + strlen(buf2) + 1);
		  else
		    {
		       buf = malloc(strlen(buf2) + 1);
		       buf[0] = 0;
		    }
		  strcat(buf, buf2);
	       }
	     if (lst)
		Efree(lst);
	  }
	else if (!strcmp(params, "actions"))
	  {
	     ActionClass       **lst;

	     lst = (ActionClass **) ListItemType(&num, LIST_TYPE_ACLASS);
	     for (i = 0; i < num; i++)
	       {
		  buf2[0] = 0;
		  Esnprintf(buf2, sizeof(buf2), "%s\n", lst[i]->name);
		  if (buf)
		     buf = realloc(buf, strlen(buf) + strlen(buf2) + 1);
		  else
		    {
		       buf = malloc(strlen(buf2) + 1);
		       buf[0] = 0;
		    }
		  strcat(buf, buf2);
	       }
	     if (lst)
		Efree(lst);
	  }
	else if (!strcmp(params, "sounds"))
	  {
	     SoundClass        **lst;

	     lst = (SoundClass **) ListItemType(&num, LIST_TYPE_SCLASS);
	     for (i = 0; i < num; i++)
	       {
		  buf2[0] = 0;
		  Esnprintf(buf2, sizeof(buf2), "%s\n", SclassGetName(lst[i]));
		  if (buf)
		     buf = realloc(buf, strlen(buf) + strlen(buf2) + 1);
		  else
		    {
		       buf = malloc(strlen(buf2) + 1);
		       buf[0] = 0;
		    }
		  strcat(buf, buf2);
	       }
	     if (lst)
		Efree(lst);
	  }
	else if (!strcmp(params, "cursors"))
	  {
	     ECursor           **lst;

	     lst = (ECursor **) ListItemType(&num, LIST_TYPE_ECURSOR);
	     for (i = 0; i < num; i++)
	       {
		  buf2[0] = 0;
		  Esnprintf(buf2, sizeof(buf2), "%s\n", ECursorGetName(lst[i]));
		  if (buf)
		     buf = realloc(buf, strlen(buf) + strlen(buf2) + 1);
		  else
		    {
		       buf = malloc(strlen(buf2) + 1);
		       buf[0] = 0;
		    }
		  strcat(buf, buf2);
	       }
	     if (lst)
		Efree(lst);
	  }
	else if (!strcmp(params, "borders"))
	  {
	     Border            **lst;

	     lst = (Border **) ListItemType(&num, LIST_TYPE_BORDER);
	     for (i = 0; i < num; i++)
	       {
		  buf2[0] = 0;
		  Esnprintf(buf2, sizeof(buf2), "%s\n", lst[i]->name);
		  if (buf)
		     buf = realloc(buf, strlen(buf) + strlen(buf2) + 1);
		  else
		    {
		       buf = malloc(strlen(buf2) + 1);
		       buf[0] = 0;
		    }
		  strcat(buf, buf2);
	       }
	     if (lst)
		Efree(lst);
	  }
	else if (!strcmp(params, "text"))
	  {
	     TextClass         **lst;

	     lst = (TextClass **) ListItemType(&num, LIST_TYPE_TCLASS);
	     for (i = 0; i < num; i++)
	       {
		  buf2[0] = 0;
		  Esnprintf(buf2, sizeof(buf2), "%s\n", lst[i]->name);
		  if (buf)
		     buf = realloc(buf, strlen(buf) + strlen(buf2) + 1);
		  else
		    {
		       buf = malloc(strlen(buf2) + 1);
		       buf[0] = 0;
		    }
		  strcat(buf, buf2);
	       }
	     if (lst)
		Efree(lst);
	  }
	else if (!strcmp(params, "images"))
	  {
	     ImageClass        **lst;

	     lst = (ImageClass **) ListItemType(&num, LIST_TYPE_ICLASS);
	     for (i = 0; i < num; i++)
	       {
		  buf2[0] = 0;
		  Esnprintf(buf2, sizeof(buf2), "%s\n", lst[i]->name);
		  if (buf)
		     buf = realloc(buf, strlen(buf) + strlen(buf2) + 1);
		  else
		    {
		       buf = malloc(strlen(buf2) + 1);
		       buf[0] = 0;
		    }
		  strcat(buf, buf2);
	       }
	     if (lst)
		Efree(lst);
	  }
	else if (!strcmp(params, "buttons"))
	  {
	     Button            **lst;

	     lst = (Button **) ListItemType(&num, LIST_TYPE_BUTTON);
	     for (i = 0; i < num; i++)
	       {
		  buf2[0] = 0;
		  Esnprintf(buf2, sizeof(buf2), "%s\n", ButtonGetName(lst[i]));
		  if (buf)
		     buf = realloc(buf, strlen(buf) + strlen(buf2) + 1);
		  else
		    {
		       buf = malloc(strlen(buf2) + 1);
		       buf[0] = 0;
		    }
		  strcat(buf, buf2);
	       }
	     if (lst)
		Efree(lst);
	  }
	else
	   CommsSend(c, "Error: unknown class selected");
     }
   else
      CommsSend(c, "Error: no class selected");

   if (buf)
     {
	CommsSend(c, buf);
	Efree(buf);
     }
}

static void
IPC_DialogOK(const char *params, Client * c)
{
   char                buf[FILEPATH_LEN_MAX];

   buf[0] = 0;

   if (params)
      DialogOKstr(_("Message"), params);
   else
      Esnprintf(buf, sizeof(buf), "Error: No text for dialog specified");

   if (buf[0])
      CommsSend(c, buf);
}

static void
IPC_SetFocus(const char *params, Client * c __UNUSED__)
{
   char                param1[FILEPATH_LEN_MAX];
   EWin               *ewin;

   if (params == NULL)
      return;

   sscanf(params, "%1000s", param1);

   if (!strcmp(param1, "?"))
     {
	ewin = GetFocusEwin();
	if (ewin)
	   IpcPrintf("focused: %#lx\n", ewin->client.win);
	else
	   IpcPrintf("focused: none\n");
     }
   else
     {
	ewin = IpcFindEwin(param1);
	if (ewin)
	   FocusToEWin(ewin, FOCUS_SET);
	else
	   IpcPrintf("No matching EWin found\n");
     }
}

static void
IPC_AdvancedFocus(const char *params, Client * c)
{
   char                buf[FILEPATH_LEN_MAX];

   buf[0] = 0;

   if (params)
     {
	char                param1[FILEPATH_LEN_MAX];
	char                param2[FILEPATH_LEN_MAX];

	param1[0] = 0;
	param2[0] = 0;
	word(params, 1, param1);
	word(params, 2, param2);
	if (!strcmp(param1, "new_window_focus"))
	  {
	     if (!strcmp(param2, "on"))
	       {
		  Conf.focus.all_new_windows_get_focus = 1;
	       }
	     else if (!strcmp(param2, "off"))
	       {
		  Conf.focus.all_new_windows_get_focus = 0;
	       }
	     else if (!strcmp(param2, "?"))
	       {
		  if (Conf.focus.all_new_windows_get_focus)
		    {
		       Esnprintf(buf, sizeof(buf), "new_window_focus: on");
		    }
		  else
		    {
		       Esnprintf(buf, sizeof(buf), "new_window_focus: off");
		    }
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf), "Error: unknown mode selected");
	       }
	  }
	else if (!strcmp(param1, "focus_list"))
	  {
	     if (!strcmp(param2, "on"))
	       {
		  Conf.warplist.enable = 1;
	       }
	     else if (!strcmp(param2, "off"))
	       {
		  Conf.warplist.enable = 0;
	       }
	     else if (!strcmp(param2, "?"))
	       {
		  if (Conf.warplist.enable)
		    {
		       Esnprintf(buf, sizeof(buf), "focus_list: on");
		    }
		  else
		    {
		       Esnprintf(buf, sizeof(buf), "focus_list: off");
		    }
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf), "Error: unknown mode selected");
	       }
	  }
	else if (!strcmp(param1, "new_popup_window_focus"))
	  {
	     if (!strcmp(param2, "on"))
	       {
		  Conf.focus.new_transients_get_focus = 1;
	       }
	     else if (!strcmp(param2, "off"))
	       {
		  Conf.focus.new_transients_get_focus = 0;
	       }
	     else if (!strcmp(param2, "?"))
	       {
		  if (Conf.focus.new_transients_get_focus)
		    {
		       Esnprintf(buf, sizeof(buf),
				 "new_popup_window_focus: on");
		    }
		  else
		    {
		       Esnprintf(buf, sizeof(buf),
				 "new_popup_window_focus: off");
		    }
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf), "Error: unknown mode selected");
	       }
	  }
	else if (!strcmp(param1, "new_popup_of_owner_focus"))
	  {
	     if (!strcmp(param2, "on"))
	       {
		  Conf.focus.new_transients_get_focus_if_group_focused = 1;
	       }
	     else if (!strcmp(param2, "off"))
	       {
		  Conf.focus.new_transients_get_focus_if_group_focused = 0;
	       }
	     else if (!strcmp(param2, "?"))
	       {
		  if (Conf.focus.new_transients_get_focus_if_group_focused)
		    {
		       Esnprintf(buf, sizeof(buf),
				 "new_popup_of_owner_focus: on");
		    }
		  else
		    {
		       Esnprintf(buf, sizeof(buf),
				 "new_popup_of_owner_focus: off");
		    }
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf), "Error: unknown mode selected");
	       }
	  }
	else if (!strcmp(param1, "raise_on_keyboard_focus_switch"))
	  {
	     if (!strcmp(param2, "on"))
	       {
		  Conf.focus.raise_on_next = 1;
	       }
	     else if (!strcmp(param2, "off"))
	       {
		  Conf.focus.raise_on_next = 0;
	       }
	     else if (!strcmp(param2, "?"))
	       {
		  if (Conf.focus.raise_on_next)
		    {
		       Esnprintf(buf, sizeof(buf),
				 "raise_on_keyboard_focus_switch: on");
		    }
		  else
		    {
		       Esnprintf(buf, sizeof(buf),
				 "raise_on_keyboard_focus_switch: off");
		    }
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf), "Error: unknown mode selected");
	       }
	  }
	else if (!strcmp(param1, "raise_after_keyboard_focus_switch"))
	  {
	     if (!strcmp(param2, "on"))
	       {
		  Conf.warplist.raise_on_select = 1;
	       }
	     else if (!strcmp(param2, "off"))
	       {
		  Conf.warplist.raise_on_select = 0;
	       }
	     else if (!strcmp(param2, "?"))
	       {
		  if (Conf.warplist.raise_on_select)
		    {
		       Esnprintf(buf, sizeof(buf),
				 "raise_after_keyboard_focus_switch: on");
		    }
		  else
		    {
		       Esnprintf(buf, sizeof(buf),
				 "raise_after_keyboard_focus_switch: off");
		    }
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf), "Error: unknown mode selected");
	       }
	  }
	else if (!strcmp(param1, "display_warp"))
	  {
	     if (!strcmp(param2, "on"))
	       {
		  Conf.warplist.enable = 1;
	       }
	     else if (!strcmp(param2, "off"))
	       {
		  Conf.warplist.enable = 0;
	       }
	     else if (!strcmp(param2, "?"))
	       {
		  if (Conf.warplist.enable)
		    {
		       Esnprintf(buf, sizeof(buf), "display_warp: on");
		    }
		  else
		    {
		       Esnprintf(buf, sizeof(buf), "display_warp: off");
		    }
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf), "Error: unknown mode selected");
	       }
	  }
	else if (!strcmp(param1, "pointer_to_keyboard_focus_window"))
	  {
	     if (!strcmp(param2, "on"))
	       {
		  Conf.focus.warp_on_next = 1;
	       }
	     else if (!strcmp(param2, "off"))
	       {
		  Conf.focus.warp_on_next = 0;
	       }
	     else if (!strcmp(param2, "?"))
	       {
		  if (Conf.focus.warp_on_next)
		    {
		       Esnprintf(buf, sizeof(buf),
				 "pointer_to_keyboard_focus_window: on");
		    }
		  else
		    {
		       Esnprintf(buf, sizeof(buf),
				 "pointer_to_keyboard_focus_window: off");
		    }
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf), "Error: unknown mode selected");
	       }
	  }
	else if (!strcmp(param1, "pointer_after_keyboard_focus_window"))
	  {
	     if (!strcmp(param2, "on"))
	       {
		  Conf.warplist.warp_on_select = 1;
	       }
	     else if (!strcmp(param2, "off"))
	       {
		  Conf.warplist.warp_on_select = 0;
	       }
	     else if (!strcmp(param2, "?"))
	       {
		  if (Conf.warplist.warp_on_select)
		    {
		       Esnprintf(buf, sizeof(buf),
				 "pointer_after_keyboard_focus_window: on");
		    }
		  else
		    {
		       Esnprintf(buf, sizeof(buf),
				 "pointer_after_keyboard_focus_window: off");
		    }
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf), "Error: unknown mode selected");
	       }
	  }
	else if (!strcmp(param1, "transients_follow_leader"))
	  {
	     if (!strcmp(param2, "on"))
	       {
		  Conf.focus.transientsfollowleader = 1;
	       }
	     else if (!strcmp(param2, "off"))
	       {
		  Conf.focus.transientsfollowleader = 0;
	       }
	     else if (!strcmp(param2, "?"))
	       {
		  if (Conf.focus.transientsfollowleader)
		    {
		       Esnprintf(buf, sizeof(buf),
				 "transients_follow_leader: on");
		    }
		  else
		    {
		       Esnprintf(buf, sizeof(buf),
				 "transients_follow_leader: off");
		    }
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf), "Error: unknown mode selected");
	       }
	  }
	else if (!strcmp(param1, "switch_to_popup_location"))
	  {
	     if (!strcmp(param2, "on"))
	       {
		  Conf.focus.switchfortransientmap = 1;
	       }
	     else if (!strcmp(param2, "off"))
	       {
		  Conf.focus.switchfortransientmap = 0;
	       }
	     else if (!strcmp(param2, "?"))
	       {
		  if (Conf.focus.switchfortransientmap)
		    {
		       Esnprintf(buf, sizeof(buf),
				 "switch_to_popup_location: on");
		    }
		  else
		    {
		       Esnprintf(buf, sizeof(buf),
				 "switch_to_popup_location: off");
		    }
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf), "Error: unknown mode selected");
	       }
	  }
	else if (!strcmp(param1, "manual_placement"))
	  {
	     if (!strcmp(param2, "on"))
	       {
		  Conf.place.manual = 1;
	       }
	     else if (!strcmp(param2, "off"))
	       {
		  Conf.place.manual = 0;
	       }
	     else if (!strcmp(param2, "?"))
	       {
		  if (Conf.place.manual)
		    {
		       Esnprintf(buf, sizeof(buf), "manual_placement: on");
		    }
		  else
		    {
		       Esnprintf(buf, sizeof(buf), "manual_placement: off");
		    }
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf), "Error: unknown mode selected");
	       }
	  }
	else if (!strcmp(param1, "manual_placement_mouse_pointer"))
	  {
	     if (!strcmp(param2, "on"))
	       {
		  Conf.place.manual_mouse_pointer = 1;
	       }
	     else if (!strcmp(param2, "off"))
	       {
		  Conf.place.manual_mouse_pointer = 0;
	       }
	     else if (!strcmp(param2, "?"))
	       {
		  if (Conf.place.manual_mouse_pointer)
		    {
		       Esnprintf(buf, sizeof(buf),
				 "manual_placement_mouse_pointer: on");
		    }
		  else
		    {
		       Esnprintf(buf, sizeof(buf),
				 "manual_placement_mouse_pointer: off");
		    }
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf), "Error: unknown mode selected");
	       }
	  }
	else
	  {
	     Esnprintf(buf, sizeof(buf), "Error: unknown mode selected");
	  }
     }
   else
     {
	Esnprintf(buf, sizeof(buf), "Error: no mode selected");
     }

   if (buf[0])
      CommsSend(c, buf);
}

static void
IPC_InternalList(const char *params, Client * c)
{
   char                buf[FILEPATH_LEN_MAX];
   char                buf2[FILEPATH_LEN_MAX];

   buf[0] = 0;
   buf2[0] = 0;

   if (params)
     {
	EWin               *const *lst;
	int                 num, i;

	lst = EwinListGetAll(&num);
	if (!strcmp(params, "pagers"))
	  {
	     for (i = 0; i < num; i++)
	       {
		  if (lst[i]->pager)
		    {
		       Esnprintf(buf2, sizeof(buf2), "%8x\n",
				 (unsigned)lst[i]->client.win);
		       strcat(buf, buf2);
		    }
	       }
	  }
	else if (!strcmp(params, "menus"))
	  {
	     for (i = 0; i < num; i++)
	       {
		  if (lst[i]->menu)
		    {
		       Esnprintf(buf2, sizeof(buf2), "%8x\n",
				 (unsigned)lst[i]->client.win);
		       strcat(buf, buf2);
		    }
	       }
	  }
	else if (!strcmp(params, "dialogs"))
	  {
	     for (i = 0; i < num; i++)
	       {
		  if (lst[i]->dialog)
		    {
		       Esnprintf(buf2, sizeof(buf2), "%8x\n",
				 (unsigned)lst[i]->client.win);
		       strcat(buf, buf2);
		    }
	       }
	  }
	else if (!strcmp(params, "internal_ewin"))
	  {
	     for (i = 0; i < num; i++)
	       {
		  if (lst[i]->internal)
		    {
		       Esnprintf(buf2, sizeof(buf2), "%8x\n",
				 (unsigned)lst[i]->client.win);
		       strcat(buf, buf2);
		    }
	       }
	  }
	else
	  {
	     Esnprintf(buf, sizeof(buf),
		       "Error: unknown internal list specified");
	  }
     }
   if (buf[0])
      CommsSend(c, buf);
   else
      CommsSend(c, "");
}

static void
IPC_Pager(const char *params, Client * c)
{
   char                buf[FILEPATH_LEN_MAX];
   char                param1[FILEPATH_LEN_MAX];
   char                param2[FILEPATH_LEN_MAX];

   buf[0] = 0;
   param1[0] = 0;
   param2[0] = 0;

   if (params)
     {
	word(params, 1, param1);
	if (!strcmp(param1, "on"))
	  {
	     EnableAllPagers();
	  }
	else if (!strcmp(param1, "off"))
	  {
	     DisableAllPagers();
	  }
	else if (!strcmp(param1, "?"))
	  {
	     if (Conf.pagers.enable)
	       {
		  Esnprintf(buf, sizeof(buf), "pager: on");
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf), "pager: off");
	       }
	  }
	else if (!strcmp(param1, "hiq"))
	  {
	     word(params, 2, param2);
	     if (!strcmp(param2, "?"))
	       {
		  if (Conf.pagers.hiq)
		    {
		       Esnprintf(buf, sizeof(buf), "pager_hiq: on");
		    }
		  else
		    {
		       Esnprintf(buf, sizeof(buf), "pager_hiq: off");
		    }
	       }
	     else if (!strcmp(param2, "on"))
	       {
		  PagerSetHiQ(1);
	       }
	     else if (!strcmp(param2, "off"))
	       {
		  PagerSetHiQ(0);
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf), "Error: unknown mode selected");

	       }
	  }
	else if (!strcmp(param1, "zoom"))
	  {
	     word(params, 2, param2);
	     if (param2[0])
	       {
		  if (!strcmp(param2, "on"))
		    {
		       Conf.pagers.zoom = 1;
		    }
		  else if (!strcmp(param2, "off"))
		    {
		       Conf.pagers.zoom = 0;
		    }
		  else if (!strcmp(param2, "?"))
		    {
		       if (Conf.pagers.zoom)
			 {
			    CommsSend(c, "pager_zoom: on");
			 }
		       else
			 {
			    CommsSend(c, "pager_zoom: off");
			 }
		    }
		  else
		    {
		       Esnprintf(buf, sizeof(buf),
				 "Error: unknown mode selected");
		    }
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf), "Error: no mode selected");
	       }
	  }
	else if (!strcmp(param1, "title"))
	  {
	     word(params, 2, param2);
	     if (param2[0])
	       {
		  if (!strcmp(param2, "on"))
		    {
		       Conf.pagers.title = 1;
		    }
		  else if (!strcmp(param2, "off"))
		    {
		       Conf.pagers.title = 0;
		    }
		  else if (!strcmp(param2, "?"))
		    {
		       if (Conf.pagers.title)
			 {
			    CommsSend(c, "pager_title: on");
			 }
		       else
			 {
			    CommsSend(c, "pager_title: off");
			 }
		    }
		  else
		    {
		       Esnprintf(buf, sizeof(buf), "Error: no mode selected");
		    }
	       }
	  }
	else if (!strcmp(param1, "scanrate"))
	  {
	     word(params, 2, param2);
	     if (param2[0])
	       {
		  if (!strcmp(param2, "?"))
		    {
		       Esnprintf(buf, sizeof(buf), "pager_scanrate: %d",
				 Conf.pagers.scanspeed);
		    }
		  else
		    {
		       Conf.pagers.scanspeed = atoi(param2);
		    }
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf), "Error: no scanrate specified.");
	       }
	  }
	else if (!strcmp(param1, "snap"))
	  {
	     word(params, 2, param2);
	     if (!strcmp(param2, "?"))
	       {
		  if (Conf.pagers.hiq)
		    {
		       Esnprintf(buf, sizeof(buf), "pager_snap: on");
		    }
		  else
		    {
		       Esnprintf(buf, sizeof(buf), "pager_snap: off");
		    }
	       }
	     else if (!strcmp(param2, "on"))
	       {
		  PagerSetSnap(1);
	       }
	     else if (!strcmp(param2, "off"))
	       {
		  PagerSetSnap(0);
	       }
	  }
	else if (!strcmp(param1, "desk"))
	  {
	     char                param3[FILEPATH_LEN_MAX];

	     param3[0] = 0;

	     word(params, 2, param2);
	     word(params, 3, param3);
	     if (param3[0])
	       {
		  if (!strcmp(param3, "on"))
		    {
		       EnableSinglePagerForDesktop(atoi(param2));
		    }
		  else if (!strcmp(param3, "new"))
		    {
		       NewPagerForDesktop(atoi(param2));
		    }
		  else if (!strcmp(param3, "off"))
		    {
		       DisablePagersForDesktop(atoi(param2));
		    }
		  else if (!strcmp(param3, "?"))
		    {
		       Esnprintf(buf, sizeof(buf), "Desk %s: %i pagers", param2,
				 PagerForDesktop(atoi(param2)));
		    }
		  else
		    {
		       Esnprintf(buf, sizeof(buf),
				 "Error: unknown mode specified");
		    }
	       }
	     else
	       {
		  if (param2[0])
		    {
		       Esnprintf(buf, sizeof(buf), "Error: no mode specified");
		    }
		  else
		    {
		       Esnprintf(buf, sizeof(buf), "Error: no desk specified");
		    }
	       }
	  }
	else
	  {
	     Esnprintf(buf, sizeof(buf), "Error: unknown mode specified");
	  }
     }
   else
     {
	Esnprintf(buf, sizeof(buf), "Error: no mode specified");
     }

   if (buf[0])
      CommsSend(c, buf);
}

static void
IPC_MoveMode(const char *params, Client * c)
{
   char                buf[FILEPATH_LEN_MAX];

   buf[0] = 0;

   if (params)
     {
	if (!strcmp(params, "opaque"))
	  {
	     Conf.movemode = 0;
	  }
	else if (!strcmp(params, "lined"))
	  {
	     Conf.movemode = 1;
	  }
	else if (!strcmp(params, "box"))
	  {
	     Conf.movemode = 2;
	  }
	else if (!strcmp(params, "shaded"))
	  {
	     Conf.movemode = 3;
	  }
	else if (!strcmp(params, "semi-solid"))
	  {
	     Conf.movemode = 4;
	  }
	else if (!strcmp(params, "translucent"))
	  {
	     Conf.movemode = 5;
	  }
	else if (!strcmp(params, "?"))
	  {
	     if (Conf.movemode)
	       {
		  if (Conf.movemode == 1)
		     Esnprintf(buf, sizeof(buf), "movemode: lined");
		  else if (Conf.movemode == 2)
		     Esnprintf(buf, sizeof(buf), "movemode: box");
		  else if (Conf.movemode == 3)
		     Esnprintf(buf, sizeof(buf), "movemode: shaded");
		  else if (Conf.movemode == 4)
		     Esnprintf(buf, sizeof(buf), "movemode: semi-solid");
		  else if (Conf.movemode == 5)
		     Esnprintf(buf, sizeof(buf), "movemode: translucent");
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf), "movemode: opaque");
	       }
	  }
	else
	  {
	     Esnprintf(buf, sizeof(buf), "Error: unknown mode specified");
	  }
     }
   else
     {
	Esnprintf(buf, sizeof(buf), "Error: no mode specified");
     }

   if (buf[0])
      CommsSend(c, buf);
}

static void
IPC_ResizeMode(const char *params, Client * c)
{
   char                buf[FILEPATH_LEN_MAX];

   buf[0] = 0;

   if (params)
     {
	if (!strcmp(params, "opaque"))
	  {
	     Conf.resizemode = 0;
	  }
	else if (!strcmp(params, "lined"))
	  {
	     Conf.resizemode = 1;
	  }
	else if (!strcmp(params, "box"))
	  {
	     Conf.resizemode = 2;
	  }
	else if (!strcmp(params, "shaded"))
	  {
	     Conf.resizemode = 3;
	  }
	else if (!strcmp(params, "semi-solid"))
	  {
	     Conf.resizemode = 4;
	  }
	else if (!strcmp(params, "?"))
	  {
	     if (Conf.resizemode)
	       {
		  if (Conf.resizemode == 1)
		     Esnprintf(buf, sizeof(buf), "resizemode: lined");
		  else if (Conf.resizemode == 2)
		     Esnprintf(buf, sizeof(buf), "resizemode: box");
		  else if (Conf.resizemode == 3)
		     Esnprintf(buf, sizeof(buf), "resizemode: shaded");
		  else if (Conf.resizemode == 4)
		     Esnprintf(buf, sizeof(buf), "resizemode: semi-solid");
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf), "resizemode: opaque");
	       }
	  }
	else
	  {
	     Esnprintf(buf, sizeof(buf), "Error: unknown mode specified");
	  }
     }
   else
     {
	Esnprintf(buf, sizeof(buf), "Error: no mode specified");
     }

   if (buf[0])
      CommsSend(c, buf);
}

static void
IPC_GeomInfoMode(const char *params, Client * c)
{
   char                buf[FILEPATH_LEN_MAX];

   buf[0] = 0;

   if (params)
     {
	if (!strcmp(params, "never"))
	  {
	     Conf.geominfomode = 0;
	  }
	else if (!strcmp(params, "center"))
	  {
	     Conf.geominfomode = 1;
	  }
	else if (!strcmp(params, "corner"))
	  {
	     Conf.geominfomode = 2;
	  }
	else if (!strcmp(params, "?"))
	  {
	     if (Conf.geominfomode)
	       {
		  if (Conf.geominfomode == 1)
		     Esnprintf(buf, sizeof(buf), "geominfomode: center");
		  else if (Conf.geominfomode == 2)
		     Esnprintf(buf, sizeof(buf), "geominfomode: corner");
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf), "geominfomode: never");
	       }
	  }
	else
	  {
	     Esnprintf(buf, sizeof(buf), "Error: unknown mode specified");
	  }
     }
   else
     {
	Esnprintf(buf, sizeof(buf), "Error: no mode specified");
     }

   if (buf[0])
      CommsSend(c, buf);
}

static void
IPC_FX(const char *params, Client * c)
{
   char                buf[FILEPATH_LEN_MAX];

   buf[0] = 0;

   if (params)
     {

	char                word1[FILEPATH_LEN_MAX];
	char                word2[FILEPATH_LEN_MAX];

	word1[0] = '\0';
	word2[0] = '\0';

	word(params, 1, word1);

	if (!strcmp(word1, "raindrops") || !strcmp(word1, "ripples") ||
	    !strcmp(word1, "waves"))
	  {
	     word(params, 2, word2);
	     if (!strcmp(word2, ""))
		FX_Op(word1, FX_OP_TOGGLE);
	     else if (!strcmp(word2, "on"))
		FX_Op(word1, FX_OP_START);
	     else if (!strcmp(word2, "off"))
		FX_Op(word1, FX_OP_STOP);
	     else if (!strcmp(word2, "?"))
		Esnprintf(buf, sizeof(buf), "%s: %s", word1,
			  FX_IsOn(word1) ? "on" : "off");
	     else
		Esnprintf(buf, sizeof(buf), "Error: unknown mode specified");
	  }
	else if (!strcmp(word1, "deskslide"))
	  {
	     word(params, 2, word2);
	     if (!strcmp(word2, "on"))
	       {
		  Conf.desks.slidein = 1;
	       }
	     else if (!strcmp(word2, "off"))
	       {
		  Conf.desks.slidein = 0;
	       }
	     else if (!strcmp(word2, "?"))
	       {
		  if (Conf.desks.slidein)
		     Esnprintf(buf, sizeof(buf), "deskslide: on");
		  else
		     Esnprintf(buf, sizeof(buf), "deskslide: off");
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf), "Error: unknown mode specified");
	       }
	  }
	else if (!strcmp(word1, "mapslide"))
	  {
	     word(params, 2, word2);
	     if (!strcmp(word2, "on"))
	       {
		  Conf.mapslide = 1;
	       }
	     else if (!strcmp(word2, "off"))
	       {
		  Conf.mapslide = 0;
	       }
	     else if (!strcmp(word2, "?"))
	       {
		  if (Conf.mapslide)
		     Esnprintf(buf, sizeof(buf), "mapslide: on");
		  else
		     Esnprintf(buf, sizeof(buf), "mapslide: off");
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf), "Error: unknown mode specified");
	       }
	  }
	else if (!strcmp(word1, "menu_animate"))
	  {
	     word(params, 2, word2);
	     if (!strcmp(word2, "on"))
	       {
		  Conf.menuslide = 1;
	       }
	     else if (!strcmp(word2, "off"))
	       {
		  Conf.menuslide = 0;
	       }
	     else if (!strcmp(word2, "?"))
	       {
		  if (Conf.menuslide)
		     Esnprintf(buf, sizeof(buf), "menu_animate: on");
		  else
		     Esnprintf(buf, sizeof(buf), "menu_animate: off");
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf), "Error: unknown mode specified");
	       }
	  }
	else if (!strcmp(word1, "animate_win_shading"))
	  {
	     word(params, 2, word2);
	     if (!strcmp(word2, "on"))
	       {
		  Conf.animate_shading = 1;
	       }
	     else if (!strcmp(word2, "off"))
	       {
		  Conf.animate_shading = 0;
	       }
	     else if (!strcmp(word2, "?"))
	       {
		  if (Conf.animate_shading)
		     Esnprintf(buf, sizeof(buf), "animate_win_shading: on");
		  else
		     Esnprintf(buf, sizeof(buf), "animate_win_shading: off");
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf), "Error: unknown mode specified");
	       }
	  }
	else if (!strcmp(word1, "window_shade_speed"))
	  {
	     word(params, 2, word2);
	     if (!strcmp(word2, "?"))
	       {
		  if (Conf.animate_shading)
		    {
		       Esnprintf(buf, sizeof(buf), "shadespeed: %d seconds",
				 Conf.shadespeed);
		    }
		  else
		    {
		       Esnprintf(buf, sizeof(buf), "shadespeed: off");
		    }
	       }
	     else
	       {
		  Conf.shadespeed = atoi(word2);
	       }
	  }
	else if (!strcmp(word1, "dragbar"))
	  {

	     char                move;

	     word(params, 2, word2);
	     move = 0;
	     if (!strcmp(word2, "off"))
	       {
		  Conf.desks.dragbar_width = 0;
		  move = 1;
	       }
	     else if (!strcmp(word2, "on"))
	       {
		  Conf.desks.dragbar_width = 16;
		  move = 1;
	       }
	     else if (!strcmp(word2, "bottom"))
	       {
		  Conf.desks.dragbar_width = 16;
		  Conf.desks.dragdir = 3;
		  move = 1;
	       }
	     else if (!strcmp(word2, "right"))
	       {
		  Conf.desks.dragbar_width = 16;
		  Conf.desks.dragdir = 1;
		  move = 1;
	       }
	     else if (!strcmp(word2, "left"))
	       {
		  Conf.desks.dragbar_width = 16;
		  Conf.desks.dragdir = 0;
		  move = 1;
	       }
	     else if (!strcmp(word2, "top"))
	       {
		  Conf.desks.dragbar_width = 16;
		  Conf.desks.dragdir = 2;
		  move = 1;
	       }
	     else if (!strcmp(word2, "?"))
	       {
		  if (Conf.desks.dragbar_width)
		    {
		       if (Conf.desks.dragdir == 1)
			 {
			    Esnprintf(buf, sizeof(buf), "Dragbar: right");
			 }
		       else if (Conf.desks.dragdir == 2)
			 {
			    Esnprintf(buf, sizeof(buf), "Dragbar: top");
			 }
		       else if (Conf.desks.dragdir == 3)
			 {
			    Esnprintf(buf, sizeof(buf), "Dragbar: bottom");
			 }
		       else
			 {
			    Esnprintf(buf, sizeof(buf), "Dragbar: left");
			 }
		    }
		  else
		    {
		       Esnprintf(buf, sizeof(buf), "Dragbar: off");
		    }
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf), "Error: unknown mode specified");
	       }

	     if (move)
	       {

		  Button             *b;
		  int                 i;

		  GotoDesktop(desks.current);
		  for (i = 0; i < ENLIGHTENMENT_CONF_NUM_DESKTOPS; i++)
		     MoveDesktop(i, 0, 0);
		  while ((b =
			  RemoveItem("_DESKTOP_DRAG_CONTROL", 0,
				     LIST_FINDBY_NAME, LIST_TYPE_BUTTON)))
		     ButtonDestroy(b);
		  while ((b =
			  RemoveItem("_DESKTOP_DESKRAY_DRAG_CONTROL", 0,
				     LIST_FINDBY_NAME, LIST_TYPE_BUTTON)))
		     ButtonDestroy(b);
		  InitDesktopControls();
		  ShowDesktopControls();
	       }
	  }
	else if (!strcmp(word1, "tooltips"))
	  {
	     word(params, 2, word2);
	     if (!strcmp(word2, "off"))
	       {
		  Conf.tooltips.enable = 0;
	       }
	     else if (!strcmp(word2, "on"))
	       {
		  Conf.tooltips.enable = 1;
	       }
	     else if (!strcmp(word2, "?"))
	       {
		  if (Conf.tooltips.enable)
		    {
		       Esnprintf(buf, sizeof(buf), "tooltips: %f seconds",
				 Conf.tooltips.delay);
		    }
		  else
		    {
		       Esnprintf(buf, sizeof(buf), "tooltips: off");
		    }
	       }
	     else
	       {
		  Conf.tooltips.delay = atof(word2);
		  if (!Conf.tooltips.delay)
		     Conf.tooltips.enable = 0;
		  else
		     Conf.tooltips.enable = 1;
	       }
	  }
	else if (!strcmp(word1, "edge_resistance"))
	  {
	     word(params, 2, word2);
	     if (word2[0])
	       {
		  if (!strcmp(word2, "off"))
		    {
		       Conf.edge_flip_resistance = -1;
		    }
		  else if (!strcmp(word2, "?"))
		    {
		       if (Conf.edge_flip_resistance >= 0)
			 {
			    Esnprintf(buf, sizeof(buf),
				      "edge_resistance: %d / 100 seconds",
				      Conf.edge_flip_resistance);
			 }
		       else
			 {
			    Esnprintf(buf, sizeof(buf), "edge_resistance: off");
			 }
		    }
		  else
		    {
		       Conf.edge_flip_resistance = atoi(word2);
		    }
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf), "Error: no time given");
	       }
	  }
	else if (!strcmp(word1, "edge_snap_distance"))
	  {
	     word(params, 2, word2);
	     if (word2[0])
	       {
		  if (!strcmp(word2, "?"))
		    {
		       Esnprintf(buf, sizeof(buf), "edge_snap_distance: %d",
				 Conf.snap.edge_snap_dist);
		    }
		  else
		    {
		       Conf.snap.edge_snap_dist = atoi(word2);
		    }
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf), "Error: no pixel distance given");
	       }
	  }
	else if (!strcmp(word1, "autoraise"))
	  {
	     word(params, 2, word2);
	     if (!strcmp(word2, "off"))
	       {
		  Conf.autoraise.enable = 0;
	       }
	     else if (!strcmp(word2, "on"))
	       {
		  Conf.autoraise.enable = 1;
	       }
	     else if (!strcmp(word2, "?"))
	       {
		  if (Conf.autoraise.enable)
		    {
		       Esnprintf(buf, sizeof(buf), "autoraise: %f seconds",
				 Conf.autoraise.delay);
		    }
		  else
		    {
		       Esnprintf(buf, sizeof(buf), "autoraise: off");
		    }
	       }
	     else
	       {
		  Conf.autoraise.delay = atof(word2);
		  if (!Conf.autoraise.delay)
		     Conf.autoraise.enable = 0;
		  else
		     Conf.autoraise.enable = 1;
	       }
	  }
	else if (!strcmp(word1, "audio"))
	  {
	     word(params, 2, word2);
	     if (!strcmp(word2, "on"))
	       {
		  if (!Conf.sound)
		    {
		       Conf.sound = 1;
		       SoundInit();
		    }
	       }
	     else if (!strcmp(word2, "off"))
	       {
		  if (Conf.sound)
		    {
		       Conf.sound = 0;
		       SoundExit();
		    }
	       }
	     else if (!strcmp(word2, "?"))
	       {
		  if (Conf.sound)
		     Esnprintf(buf, sizeof(buf), "audio: on");
		  else
		     Esnprintf(buf, sizeof(buf), "audio: off");
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf), "Error: unknown mode specified");
	       }
	  }
	else
	  {
	     Esnprintf(buf, sizeof(buf), "Error: unknown effect specified");
	  }
     }
   else
     {
	Esnprintf(buf, sizeof(buf), "Error: no effect specified");
     }

   if (buf[0])
      CommsSend(c, buf);
}

static void
IPC_ButtonShow(const char *params, Client * c)
{
   ActionsCall(ACTION_HIDESHOW_BUTTON, NULL, params);
   return;
   c = NULL;
}

static void
IPC_WinList(const char *params, Client * c __UNUSED__)
{
   char                param1[FILEPATH_LEN_MAX];
   EWin               *const *lst, *e;
   int                 num, i;

   word(params, 1, param1);

   lst = EwinListGetAll(&num);
   for (i = 0; i < num; i++)
     {
	e = lst[i];
	switch (param1[0])
	  {
	  case '\0':
	     IpcPrintf("%#lx : %s\n", e->client.win, SS(e->icccm.wm_name));
	     break;
	  default:
	     IpcPrintf("%#lx : %s :: %d : %d %d : %d %d %dx%d\n",
		       e->client.win, SS(e->icccm.wm_name),
		       (e->sticky) ? -1 : e->desktop, e->area_x, e->area_y,
		       e->x, e->y, e->w, e->h);
	     break;
	  case 'a':
	     IpcPrintf("%#10lx : %4d %4d %4dx%4d :: %2d : %d %d : %s\n",
		       e->client.win, e->x, e->y, e->w, e->h,
		       (e->sticky) ? -1 : e->desktop, e->area_x, e->area_y,
		       SS(e->icccm.wm_name));
	     break;
	  }
     }
   if (num <= 0)
      IpcPrintf("No windows\n");
}

static void
IPC_GotoArea(const char *params, Client * c)
{
   char                buf[FILEPATH_LEN_MAX];
   char                param1[FILEPATH_LEN_MAX];
   char                param2[FILEPATH_LEN_MAX];

   buf[0] = 0;
   param1[0] = 0;
   param2[0] = 0;

   if (!params)
     {
	Esnprintf(buf, sizeof(buf), "Error: no area specified");
     }
   else
     {
	int                 a, b;

	word(params, 1, param1);
	if (!strcmp(param1, "next"))
	  {
	     GetCurrentArea(&a, &b);
	     word(params, 2, param2);
	     if ((param2[0]) && (!strcmp(param2, "horiz")))
	       {
		  a++;
	       }
	     else if ((param2[0]) && (!strcmp(param2, "vert")))
	       {
		  b++;
	       }
	     else
	       {
		  a++;
		  b++;
	       }
	     SetCurrentArea(a, b);
	  }
	else if (!strcmp(param1, "prev"))
	  {
	     GetCurrentArea(&a, &b);
	     word(params, 2, param2);
	     if ((param2[0]) && (!strcmp(param2, "horiz")))
	       {
		  a--;
	       }
	     else if ((param2[0]) && (!strcmp(param2, "vert")))
	       {
		  b--;
	       }
	     else
	       {
		  a--;
		  b--;
	       }
	     SetCurrentArea(a, b);
	  }
	else if (!strcmp(param1, "?"))
	  {
	     GetCurrentArea(&a, &b);
	     Esnprintf(buf, sizeof(buf), "Current Area: %d %d", a, b);
	  }
	else
	  {
	     sscanf(params, "%i %i", &a, &b);
	     SetCurrentArea(a, b);
	  }
     }

   if (buf[0])
      CommsSend(c, buf);
}

static void
IPC_WinOps(const char *params, Client * c)
{
   char                buf[FILEPATH_LEN_MAX];
   EWin               *ewin;

   char                windowid[FILEPATH_LEN_MAX];
   char                operation[FILEPATH_LEN_MAX];
   char                param1[FILEPATH_LEN_MAX];
   unsigned int        win;

   if (params == NULL)
     {
	Esnprintf(buf, sizeof(buf), "Error: no window specified");
	goto done;
     }

   win = 0;
   buf[0] = 0;
   windowid[0] = 0;
   operation[0] = 0;
   param1[0] = 0;

   word(params, 1, windowid);
   ewin = IpcFindEwin(windowid);
   if (!ewin)
     {
	Esnprintf(buf, sizeof(buf), "Error: no such window: %8x", win);
	goto done;
     }

   word(params, 2, operation);
   word(params, 3, param1);

   if (!operation[0])
     {
	Esnprintf(buf, sizeof(buf), "Error: no operation specified");
	goto done;
     }

   if (!strncmp(operation, "close", 2))
     {
	ICCCM_Delete(ewin);
	SoundPlay("SOUND_WINDOW_CLOSE");
     }
   else if (!strncmp(operation, "annihiliate", 2))
     {
	EDestroyWindow(disp, ewin->client.win);
	SoundPlay("SOUND_WINDOW_CLOSE");
     }
   else if (!strncmp(operation, "iconify", 2))
     {
	if (SetEwinBoolean(buf, sizeof(buf), "window iconified",
			   &ewin->iconified, param1, 0))
	  {
	     if (ewin->iconified)
		DeIconifyEwin(ewin);
	     else
		IconifyEwin(ewin);
	  }
     }
   else if (!strncmp(operation, "shade", 2))
     {
	if (SetEwinBoolean(buf, sizeof(buf), "window shaded",
			   &ewin->shaded, param1, 0))
	  {
	     if (ewin->shaded)
		EwinUnShade(ewin);
	     else
		EwinShade(ewin);
	  }
     }
   else if (!strncmp(operation, "stick", 2))
     {
	if (SetEwinBoolean(buf, sizeof(buf), "window sticky",
			   &ewin->sticky, param1, 0))
	  {
	     if (ewin->sticky)
		EwinUnStick(ewin);
	     else
		EwinStick(ewin);
	  }
     }
   else if (!strcmp(operation, "fixedpos"))
     {
	SetEwinBoolean(buf, sizeof(buf), "window fixedpos",
		       &ewin->fixedpos, param1, 1);
     }
   else if (!strcmp(operation, "never_use_area"))
     {
	SetEwinBoolean(buf, sizeof(buf), "window never_use_area",
		       &ewin->never_use_area, param1, 1);
     }
   else if (!strcmp(operation, "focusclick"))
     {
	SetEwinBoolean(buf, sizeof(buf), "window focusclick",
		       &ewin->focusclick, param1, 1);
     }
   else if (!strcmp(operation, "neverfocus"))
     {
	SetEwinBoolean(buf, sizeof(buf), "window neverfocus",
		       &ewin->neverfocus, param1, 1);
     }
   else if (!strncmp(operation, "title", 2))
     {
	char               *ptr = strstr(params, "title");

	if (ptr)
	  {
	     ptr += strlen("title");
	     while (*ptr == ' ')
		ptr++;
	     if (strlen(ptr))
	       {
		  if (!strncmp(ptr, "?", 1))
		    {
		       /* return the window title */
		       Esnprintf(buf, sizeof(buf),
				 "window title: %s", ewin->icccm.wm_name);
		    }
		  else
		    {
		       /* set the new title */
		       if (ewin->icccm.wm_name)
			  Efree(ewin->icccm.wm_name);
		       ewin->icccm.wm_name =
			  Emalloc((strlen(ptr) + 1) * sizeof(char));

		       strcpy(ewin->icccm.wm_name, ptr);
		       XStoreName(disp, ewin->client.win, ewin->icccm.wm_name);
		       EwinBorderUpdateInfo(ewin);
		    }
	       }
	     else
	       {
		  /* error */
		  Esnprintf(buf, sizeof(buf), "Error: no title specified");
	       }
	  }
     }
   else if (!strcmp(operation, "toggle_width") || !strcmp(operation, "tw"))
     {
	MaxWidth(ewin, param1);
     }
   else if (!strcmp(operation, "toggle_height") || !strcmp(operation, "th"))
     {
	MaxHeight(ewin, param1);
     }
   else if (!strcmp(operation, "toggle_size") || !strcmp(operation, "ts"))
     {
	MaxSize(ewin, param1);
     }
   else if (!strncmp(operation, "raise", 2))
     {
	RaiseEwin(ewin);
     }
   else if (!strncmp(operation, "lower", 2))
     {
	LowerEwin(ewin);
     }
   else if (!strncmp(operation, "layer", 2))
     {
	if (!strcmp(param1, "?"))
	  {
	     Esnprintf(buf, sizeof(buf), "window layer: %d", ewin->layer);
	  }
	else
	  {
	     ewin->layer = atoi(param1);
	     RaiseEwin(ewin);
	     RememberImportantInfoForEwin(ewin);
	  }
     }
   else if (!strncmp(operation, "border", 2))
     {
	Border             *b;

	if (param1[0])
	  {
	     if (!strcmp(param1, "?"))
	       {
		  if (ewin->border)
		    {
		       if (ewin->border->name)
			 {
			    Esnprintf(buf, sizeof(buf),
				      "window border: %s", ewin->border->name);
			 }
		    }
	       }
	     else
	       {
		  b = (Border *) FindItem(param1, 0,
					  LIST_FINDBY_NAME, LIST_TYPE_BORDER);
		  if (b)
		     EwinSetBorder(ewin, b, 1);
	       }
	  }
	else
	  {
	     Esnprintf(buf, sizeof(buf), "Error: no border specified");
	  }
     }
   else if (!strncmp(operation, "desk", 2))
     {
	if (param1[0])
	  {
	     if (!strncmp(param1, "next", 1))
	       {
		  MoveEwinToDesktop(ewin, ewin->desktop + 1);
		  RaiseEwin(ewin);
		  ICCCM_Configure(ewin);
		  ewin->sticky = 0;
	       }
	     else if (!strncmp(param1, "prev", 1))
	       {
		  MoveEwinToDesktop(ewin, ewin->desktop - 1);
		  RaiseEwin(ewin);
		  ICCCM_Configure(ewin);
		  ewin->sticky = 0;
	       }
	     else if (!strcmp(param1, "?"))
	       {
		  Esnprintf(buf, sizeof(buf), "window desk: %d", ewin->desktop);
	       }
	     else
	       {
		  MoveEwinToDesktop(ewin, atoi(param1));
		  RaiseEwin(ewin);
		  ICCCM_Configure(ewin);
		  ewin->sticky = 0;
	       }
	  }
	else
	  {
	     Esnprintf(buf, sizeof(buf), "Error: no desktop supplied");
	  }
     }
   else if (!strncmp(operation, "area", 2))
     {
	int                 a, b;

	if (param1[0])
	  {
	     if (!strcmp(param1, "?"))
	       {
		  Esnprintf(buf, sizeof(buf),
			    "window area: %d %d", ewin->area_x, ewin->area_y);
	       }
	     else
	       {
		  sscanf(params, "%*s %*s %i %i", &a, &b);
		  MoveEwinToArea(ewin, a, b);
	       }
	  }
	else
	  {
	     Esnprintf(buf, sizeof(buf), "Error: no area supplied");
	  }
     }
   else if (!strncmp(operation, "move", 2))
     {
	int                 a, b;

	if (param1[0])
	  {
	     if (!strcmp(param1, "?"))
	       {
		  Esnprintf(buf, sizeof(buf),
			    "window location: %d %d", ewin->x, ewin->y);
	       }
	     else if (!strcmp(param1, "??"))
	       {
		  Esnprintf(buf, sizeof(buf),
			    "client location: %d %d",
			    ewin->x + ewin->border->border.left,
			    ewin->y + ewin->border->border.top);
	       }
	     else
	       {
		  sscanf(params, "%*s %*s %i %i", &a, &b);
		  MoveResizeEwin(ewin, a, b, ewin->client.w, ewin->client.h);
	       }
	  }
	else
	  {
	     Esnprintf(buf, sizeof(buf), "Error: no coords supplied");
	  }
     }
   else if (!strcmp(operation, "resize") || !strcmp(operation, "sz"))
     {
	int                 a, b;

	if (param1[0])
	  {
	     if (!strcmp(param1, "?"))
	       {
		  Esnprintf(buf, sizeof(buf),
			    "window size: %d %d", ewin->client.w,
			    ewin->client.h);
	       }
	     else if (!strcmp(param1, "??"))
	       {
		  Esnprintf(buf, sizeof(buf),
			    "frame size: %d %d", ewin->w, ewin->h);
	       }
	     else
	       {
		  sscanf(params, "%*s %*s %i %i", &a, &b);
		  MoveResizeEwin(ewin, ewin->x, ewin->y, a, b);
	       }
	  }
     }
   else if (!strcmp(operation, "move_relative") || !strcmp(operation, "mr"))
     {
	int                 a, b;

	if (param1[0])
	  {
	     sscanf(params, "%*s %*s %i %i", &a, &b);
	     a += ewin->x;
	     b += ewin->y;
	     MoveResizeEwin(ewin, a, b, ewin->client.w, ewin->client.h);
	  }
     }
   else if (!strcmp(operation, "resize_relative") || !strcmp(operation, "sr"))
     {
	int                 a, b;

	if (param1[0])
	  {
	     sscanf(params, "%*s %*s %i %i", &a, &b);
	     a += ewin->client.w;
	     b += ewin->client.h;
	     MoveResizeEwin(ewin, ewin->x, ewin->y, a, b);
	  }
     }
   else if (!strncmp(operation, "focus", 2))
     {
	if (!strcmp(param1, "?"))
	  {
	     if (ewin == GetFocusEwin())
	       {
		  Esnprintf(buf, sizeof(buf), "focused: yes");
	       }
	     else
	       {
		  Esnprintf(buf, sizeof(buf), "focused: no");
	       }
	  }
	else
	  {
	     FocusToEWin(ewin, FOCUS_SET);
	  }
     }
   else
     {
	Esnprintf(buf, sizeof(buf), "Error: unknown operation");
     }

 done:
   if (buf[0])
      CommsSend(c, buf);
}

static void
IPC_NumAreas(const char *params, Client * c)
{
   char                buf[FILEPATH_LEN_MAX];

   buf[0] = 0;
   if (params)
     {
	if (!strcmp(params, "?"))
	  {
	     int                 ax, ay;

	     GetAreaSize(&ax, &ay);
	     Esnprintf(buf, sizeof(buf), "Number of Areas: %d %d", ax, ay);
	  }
	else
	  {
	     char                ax[128], ay[128];

	     word(params, 1, ax);
	     word(params, 2, ay);
	     SetNewAreaSize(atoi(ax), atoi(ay));
	  }
     }
   else
     {
	Esnprintf(buf, sizeof(buf), "Error: number of areas not given");
     }

   if (buf[0])
      CommsSend(c, buf);
}

static void
IPC_NumDesks(const char *params, Client * c)
{
   char                buf[FILEPATH_LEN_MAX];

   buf[0] = 0;
   if (params)
     {
	if (!strcmp(params, "?"))
	  {
	     Esnprintf(buf, sizeof(buf), "Number of Desks: %d", Conf.desks.num);
	  }
	else
	  {
	     ChangeNumberOfDesktops(atoi(params));
	  }
     }
   else
     {
	Esnprintf(buf, sizeof(buf), "Error: number of desks not given");
     }

   if (buf[0])
      CommsSend(c, buf);
}

static void
IPC_FocusMode(const char *params, Client * c)
{
   char                buf[FILEPATH_LEN_MAX];

   buf[0] = 0;
   if (params)
     {
	if (!strcmp(params, "click"))
	  {
	     Conf.focus.mode = MODE_FOCUS_CLICK;
	     Mode.click_focus_grabbed = 1;
	  }
	else if (!strcmp(params, "pointer"))
	  {
	     Conf.focus.mode = MODE_FOCUS_POINTER;
	  }
	else if (!strcmp(params, "sloppy"))
	  {
	     Conf.focus.mode = MODE_FOCUS_SLOPPY;
	  }
	else if (!strcmp(params, "clicknograb"))
	  {
	     Conf.focus.mode = MODE_FOCUS_CLICK;
	     Mode.click_focus_grabbed = 0;
	  }
	else if (!strcmp(params, "?"))
	  {
	     Esnprintf(buf, sizeof(buf), "Focus Mode: ");
	     if (Conf.focus.mode == MODE_FOCUS_CLICK)
	       {
		  if (Mode.click_focus_grabbed)
		    {
		       strcat(buf, "click");
		    }
		  else
		    {
		       strcat(buf, "clicknograb");
		    }
	       }
	     else if (Conf.focus.mode == MODE_FOCUS_SLOPPY)
	       {
		  strcat(buf, "sloppy");
	       }
	     else if (Conf.focus.mode == MODE_FOCUS_POINTER)
	       {
		  strcat(buf, "pointer");
	       }
	     else
	       {
		  strcat(buf, "unknown");
	       }
	  }
	else
	  {
	     Esnprintf(buf, sizeof(buf), "Error: unknown focus type");
	  }
     }
   else
     {
	Esnprintf(buf, sizeof(buf), "Error: no focus type given");
     }

   if (buf[0])
      CommsSend(c, buf);
}

static void
IPC_ShowIcons(const char *params, Client * c)
{
   /* Doesn't look like this function is doing anything, but it used to 
    * if I recall correctly --Mandrake
    * If it did anything useful, it was before 0.16.5 /Kim */
   return;
   params = NULL;
   c = NULL;
}

static void
IPC_GotoDesktop(const char *params, Client * c)
{
   char                buf[FILEPATH_LEN_MAX];

   buf[0] = 0;
   if (!params)
     {
	Esnprintf(buf, sizeof(buf), "Error: no desktop selected");
     }
   else
     {
	if (!strcmp(params, "next"))
	  {
	     ActionsCall(ACTION_DESKTOP_NEXT, NULL, NULL);
	  }
	else if (!strcmp(params, "prev"))
	  {
	     ActionsCall(ACTION_DESKTOP_PREV, NULL, NULL);
	  }
	else if (!strcmp(params, "?"))
	  {
	     Esnprintf(buf, sizeof(buf), "Current Desktop: %d", desks.current);
	  }
	else
	  {
	     ActionsCall(ACTION_GOTO_DESK, NULL, params);
	  }
     }

   if (buf[0])
      CommsSend(c, buf);
}

static void
IPC_ListThemes(const char *params, Client * c)
{
   char              **list, *buf = NULL;
   int                 i, num;

   params = NULL;
   list = ListThemes(&num);
   for (i = 0; i < num; i++)
     {
	if (buf)
	  {
	     buf = Erealloc(buf, strlen(buf) + strlen(list[i]) + 2);
	  }
	else
	  {
	     buf = Emalloc(strlen(list[i]) + 2);
	     buf[0] = 0;
	  }
	strcat(buf, list[i]);
	strcat(buf, "\n");
     }

   if (list)
      freestrlist(list, num);

   if (buf)
     {
	CommsSend(c, buf);
	Efree(buf);
     }
   else
     {
	CommsSend(c, "");
     }
}

static void
IPC_SMFile(const char *params, Client * c)
{
   char                buf[FILEPATH_LEN_MAX];

   buf[0] = 0;

   if (params)
     {
	if (!strcmp(params, "?"))
	  {
	     Esnprintf(buf, sizeof(buf), GetSMFile());
	  }
	else
	  {
	     SetSMFile(params);
	  }
     }
   else
     {
	Esnprintf(buf, sizeof(buf), "Error: no file prefix specified");
     }

   if (buf[0])
      CommsSend(c, buf);
}

static void
IPC_ForceSave(const char *params, Client * c)
{
   c = NULL;
   params = NULL;

   if (!Mode.wm.master)
      return;

   if (Conf.autosave)
      SaveUserControlConfig(fopen(GetGenericSMFile(), "w"));
   else
      E_rm(GetGenericSMFile());
}

static void
IPC_Restart(const char *params, Client * c)
{
   c = NULL;
   params = NULL;

   SessionExit("restart");

}

static void
IPC_RestartWM(const char *params, Client * c)
{
   char                buf[FILEPATH_LEN_MAX];

   if (params)
     {
	Esnprintf(buf, sizeof(buf), "restart_wm %s", params);
	params = NULL;
	SessionExit(buf);
     }
   else
     {
	Esnprintf(buf, sizeof(buf), "Error: no window manager specified");
	CommsSend(c, buf);
     }
}

static void
IPC_RestartTheme(const char *params, Client * c)
{
   char                buf[FILEPATH_LEN_MAX];

   if (params)
     {
	Esnprintf(buf, sizeof(buf), "restart_theme %s", params);
	params = NULL;
	SessionExit(buf);
     }
   else
     {
	Esnprintf(buf, sizeof(buf), "Error: no theme specified");
	CommsSend(c, buf);
     }
}

static void
IPC_Exit(const char *params, Client * c)
{
   c = NULL;

   if (params)
      SessionExit("quit");
   else
      SessionExit("logout");
}

static void
IPC_DefaultTheme(const char *params, Client * c)
{
   char                buf[FILEPATH_LEN_MAX];

   buf[0] = 0;

   if (!params)
      return;

   if (!strcmp(params, "?"))
     {
	Esnprintf(buf, sizeof(buf), "%s", ThemeGetDefault());
     }
   else
     {
	if (exists(params))
	  {
	     char                restartcommand[FILEPATH_LEN_MAX];

	     ThemeSetDefault(params);
	     Esnprintf(restartcommand, sizeof(restartcommand),
		       "restart_theme %s", params);
	     SessionExit(restartcommand);
	  }
	else
	  {
	     Esnprintf(buf, sizeof(buf), "Could not find theme: %s",
		       ThemeGetDefault());
	  }
     }

   if (buf[0])
      CommsSend(c, buf);
}

static void
IPC_CurrentTheme(const char *params, Client * c)
{
   char                buf[FILEPATH_LEN_MAX];

   buf[0] = 0;

   Esnprintf(buf, sizeof(buf), themepath);

   if (buf[0])
      CommsSend(c, buf);

   params = NULL;
}

static void
IPC_AutoSave(const char *params, Client * c)
{
   char                buf[FILEPATH_LEN_MAX];

   buf[0] = 0;

   if (!params)
      return;

   if (!strcmp(params, "?"))
     {
	if (Conf.autosave)
	   Esnprintf(buf, sizeof(buf), "Autosave : on");
	else
	   Esnprintf(buf, sizeof(buf), "Autosave : off");
     }
   else if (!strcmp(params, "on"))
     {
	Conf.autosave = 1;
     }
   else if (!strcmp(params, "off"))
     {
	Conf.autosave = 0;
     }
   else
     {
	Esnprintf(buf, sizeof(buf), "Unknown autosave state: %s", params);
     }

   if (buf[0])
      CommsSend(c, buf);
}

static void
IPC_Copyright(const char *params, Client * c)
{
   char                buf[FILEPATH_LEN_MAX];

   params = NULL;
   Esnprintf(buf, sizeof(buf),
	     "Copyright (C) 2000-2004 Carsten Haitzler and Geoff Harrison,\n"
	     "with various contributors (Isaac Richards, Sung-Hyun Nam, "
	     "Kimball Thurston,\n"
	     "Michael Kellen, Frederic Devernay, Felix Bellaby, "
	     "Michael Jennings,\n"
	     "Christian Kreibich, Peter Kjellerstedt, Troy Pesola, Owen Taylor, "
	     "Stalyn,\n" "Knut Neumann, Nathan Heagy, Simon Forman, "
	     "Brent Nelson,\n"
	     "Martin Tyler, Graham MacDonald, Jessse Michael, "
	     "Paul Duncan, Daniel Erat,\n"
	     "Tom Gilbert, Peter Alm, Ben Frantzdale, "
	     "Hallvar Helleseth, Kameran Kashani,\n"
	     "Carl Strasen, David Mason, Tom Christiansen, and others\n"
	     "-- please see the AUTHORS file for a complete listing)\n\n"
	     "Permission is hereby granted, free of charge, to "
	     "any person obtaining a copy\n"
	     "of this software and associated documentation files "
	     "(the \"Software\"), to\n"
	     "deal in the Software without restriction, including "
	     "without limitation the\n"
	     "rights to use, copy, modify, merge, publish, distribute, "
	     "sub-license, and/or\n"
	     "sell copies of the Software, and to permit persons to "
	     "whom the Software is\n"
	     "furnished to do so, subject to the following conditions:\n\n"
	     "The above copyright notice and this permission notice "
	     "shall be included in\n"
	     "all copies or substantial portions of the Software.\n\n"
	     "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF "
	     "ANY KIND, EXPRESS OR\n"
	     "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF "
	     "MERCHANTABILITY,\n"
	     "FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. "
	     "IN NO EVENT SHALL\n"
	     "THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER "
	     "LIABILITY, WHETHER\n"
	     "IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, "
	     "OUT OF OR IN\n"
	     "CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS "
	     "IN THE SOFTWARE.\n");

   if (buf)
      CommsSend(c, buf);
}

static void
IPC_Version(const char *params, Client * c)
{
   char                buf[FILEPATH_LEN_MAX];

   params = NULL;

   buf[0] = 0;

   Esnprintf(buf, sizeof(buf),
	     _("Enlightenment Version : %s\n" "code is current to    : %s\n"),
	     e_wm_version, E_CHECKOUT_DATE);

   if (buf)
      CommsSend(c, buf);
}

#if 0				/* Not implemented */
/* The External function designed for attaching to a dialog box
 * to return a message back to an external app telling you what
 * button was depressed
 */

static void
ButtonIPC(int val, void *data)
{
   val = 0;
   data = NULL;
}
#endif

/*
 * Reloads the menus.cfg file from cache, 
 *
 */

static void
IPC_ReloadMenus(const char *params, Client * c)
{
   /*
    * Do nothing here but call doExit, following the pattern
    * that raster/mandrake have setup 08/16/99
    *
    * Ok that wasn't nice, I forgot to deallocate menus
    * Now the way I'm doing this if any menu req's come in
    * while this is happening we're probably in la-la land
    * but i'll try this 08/17/99
    */

   MenusDestroyLoaded();

   LoadConfigFile("menus.cfg");
   return;
   params = NULL;
   c = NULL;
}

static void
IPC_GroupInfo(const char *params, Client * c __UNUSED__)
{
   char                buf[FILEPATH_LEN_MAX];
   Group             **groups = NULL;
   int                 num_groups, i, j;

   buf[0] = 0;

   if (params)
     {
	Group              *group;
	char                groupid[FILEPATH_LEN_MAX];
	int                 gix;

	groupid[0] = 0;
	word(params, 1, groupid);
	sscanf(groupid, "%d", &gix);

	group = FindItem(NULL, gix, LIST_FINDBY_ID, LIST_TYPE_GROUP);

	if (!group)
	  {
	     IpcPrintf("Error: no such group: %d", gix);
	     return;
	  }

	groups = (Group **) Emalloc(sizeof(Group **));
	if (!groups)
	   return;

	groups[0] = group;
	num_groups = 1;
     }
   else
     {
	groups = (Group **) ListItemType(&num_groups, LIST_TYPE_GROUP);

	IpcPrintf("Number of groups: %d\n", num_groups);
     }

   for (i = 0; i < num_groups; i++)
     {
	for (j = 0; j < groups[i]->num_members; j++)
	   IpcPrintf("%d: %s\n", groups[i]->index,
		     groups[i]->members[j]->icccm.wm_name);

	IpcPrintf("        index: %d\n" "  num_members: %d\n"
		  "      iconify: %d\n" "         kill: %d\n"
		  "         move: %d\n" "        raise: %d\n"
		  "   set_border: %d\n" "        stick: %d\n"
		  "        shade: %d\n" "       mirror: %d\n",
		  groups[i]->index, groups[i]->num_members,
		  groups[i]->cfg.iconify, groups[i]->cfg.kill,
		  groups[i]->cfg.move, groups[i]->cfg.raise,
		  groups[i]->cfg.set_border, groups[i]->cfg.stick,
		  groups[i]->cfg.shade, groups[i]->cfg.mirror);
     }

   if (groups)
      Efree(groups);
}

static void
IPC_GroupOps(const char *params, Client * c)
{
   char                buf[FILEPATH_LEN_MAX];
   Group              *group = current_group;
   char                groupid[FILEPATH_LEN_MAX];
   int                 gix;

   buf[0] = 0;
   if (params)
     {
	char                windowid[FILEPATH_LEN_MAX];
	char                operation[FILEPATH_LEN_MAX];
	char                param1[FILEPATH_LEN_MAX];
	unsigned int        win;

	windowid[0] = 0;
	operation[0] = 0;
	param1[0] = 0;
	word(params, 1, windowid);
	sscanf(windowid, "%x", &win);
	word(params, 2, operation);

	if (!operation[0])
	  {
	     Esnprintf(buf, sizeof(buf), "Error: no operation specified");
	  }
	else
	  {
	     EWin               *ewin;

	     ewin = FindEwinByChildren(win);
	     if (!ewin)
	       {
		  Esnprintf(buf, sizeof(buf), "Error: no such window: %8x",
			    win);
	       }
	     else
	       {
		  if (!strcmp(operation, "start"))
		    {
		       BuildWindowGroup(&ewin, 1);
		       Esnprintf(buf, sizeof(buf), "start %8x", win);
		    }
		  else if (!strcmp(operation, "add"))
		    {
		       groupid[0] = 0;
		       word(params, 3, groupid);

		       if (groupid[0])
			 {
			    sscanf(groupid, "%d", &gix);
			    group = FindItem(NULL, gix, LIST_FINDBY_ID,
					     LIST_TYPE_GROUP);
			 }
		       AddEwinToGroup(ewin, group);
		       Esnprintf(buf, sizeof(buf), "add %8x", win);
		    }
		  else if (!strcmp(operation, "remove"))
		    {
		       groupid[0] = 0;
		       word(params, 3, groupid);

		       if (groupid[0])
			 {
			    sscanf(groupid, "%d", &gix);
			    group = FindItem(NULL, gix, LIST_FINDBY_ID,
					     LIST_TYPE_GROUP);
			 }
		       RemoveEwinFromGroup(ewin, group);
		       Esnprintf(buf, sizeof(buf), "remove %8x", win);
		    }
		  else if (!strcmp(operation, "break"))
		    {
		       groupid[0] = 0;
		       word(params, 3, groupid);

		       if (groupid[0])
			 {
			    sscanf(groupid, "%d", &gix);
			    group = FindItem(NULL, gix, LIST_FINDBY_ID,
					     LIST_TYPE_GROUP);
			 }
		       BreakWindowGroup(ewin, group);
		       Esnprintf(buf, sizeof(buf), "break %8x", win);
		    }
		  else if (!strcmp(operation, "showhide"))
		    {
		       ActionsCall(ACTION_SHOW_HIDE_GROUP, NULL, windowid);
		       Esnprintf(buf, sizeof(buf), "showhide %8x", win);
		    }
		  else
		    {
		       Esnprintf(buf, sizeof(buf),
				 "Error: no such operation: %s", operation);

		    }
	       }
	  }
     }
   else
     {
	Esnprintf(buf, sizeof(buf), "Error: no window specified");
     }

   if (buf)
      CommsSend(c, buf);
}

static void
IPC_Group(const char *params, Client * c)
{
   char                buf[FILEPATH_LEN_MAX];

   buf[0] = 0;
   if (params)
     {

	char                groupid[FILEPATH_LEN_MAX];
	char                operation[FILEPATH_LEN_MAX];
	char                param1[FILEPATH_LEN_MAX];
	int                 gix;

	groupid[0] = 0;
	operation[0] = 0;
	param1[0] = 0;
	word(params, 1, groupid);
	sscanf(groupid, "%d", &gix);
	word(params, 2, operation);

	if (!operation[0])
	  {
	     Esnprintf(buf, sizeof(buf), "Error: no operation specified");
	  }
	else
	  {
	     Group              *group;
	     int                 onoff = -1;

	     group = FindItem(NULL, gix, LIST_FINDBY_ID, LIST_TYPE_GROUP);

	     if (!group)
	       {
		  Esnprintf(buf, sizeof(buf), "Error: no such group: %d", gix);
	       }
	     else
	       {
		  word(params, 3, param1);
		  if (param1[0])
		    {
		       if (!strcmp(param1, "on"))
			  onoff = 1;
		       else if (!strcmp(param1, "off"))
			  onoff = 0;

		       if (onoff == -1 && strcmp(param1, "?"))
			 {
			    Esnprintf(buf, sizeof(buf),
				      "Error: unknown mode specified");
			 }
		       else if (!strcmp(operation, "num_members"))
			 {
			    Esnprintf(buf, sizeof(buf), "num_members: %d",
				      group->num_members);
			    onoff = -1;
			 }
		       else if (!strcmp(operation, "iconify"))
			 {
			    if (onoff >= 0)
			       group->cfg.iconify = onoff;
			    else
			       onoff = group->cfg.iconify;
			 }
		       else if (!strcmp(operation, "kill"))
			 {
			    if (onoff >= 0)
			       group->cfg.kill = onoff;
			    else
			       onoff = group->cfg.kill;
			 }
		       else if (!strcmp(operation, "move"))
			 {
			    if (onoff >= 0)
			       group->cfg.move = onoff;
			    else
			       onoff = group->cfg.move;
			 }
		       else if (!strcmp(operation, "raise"))
			 {
			    if (onoff >= 0)
			       group->cfg.raise = onoff;
			    else
			       onoff = group->cfg.raise;
			 }
		       else if (!strcmp(operation, "set_border"))
			 {
			    if (onoff >= 0)
			       group->cfg.set_border = onoff;
			    else
			       onoff = group->cfg.set_border;
			 }
		       else if (!strcmp(operation, "stick"))
			 {
			    if (onoff >= 0)
			       group->cfg.stick = onoff;
			    else
			       onoff = group->cfg.stick;
			 }
		       else if (!strcmp(operation, "shade"))
			 {
			    if (onoff >= 0)
			       group->cfg.shade = onoff;
			    else
			       onoff = group->cfg.shade;
			 }
		       else if (!strcmp(operation, "mirror"))
			 {
			    if (onoff >= 0)
			       group->cfg.mirror = onoff;
			    else
			       onoff = group->cfg.mirror;
			 }
		       else
			 {
			    Esnprintf(buf, sizeof(buf),
				      "Error: no such operation: %s",
				      operation);
			    onoff = -1;
			 }
		    }
		  else
		    {
		       Esnprintf(buf, sizeof(buf), "Error: no mode specified");
		    }
	       }

	     if (onoff == 1)
		Esnprintf(buf, sizeof(buf), "%s: on", operation);
	     else if (onoff == 0)
		Esnprintf(buf, sizeof(buf), "%s: off", operation);
	  }
     }
   else
     {
	Esnprintf(buf, sizeof(buf), "Error: no group specified");
     }

   if (buf)
      CommsSend(c, buf);
}

static void
IPC_MemDebug(const char *params, Client * c)
{
#if !USE_LIBC_MALLOC
   EDisplayMemUse();
#endif

   params = NULL;
   c = NULL;
}

static void
IPC_RememberList(const char *params, Client * c)
{
   Snapshot          **lst;
   int                 i, j, num, f;
   char                buf[FILEPATH_LEN_MAX * 2];	/* hope 2x doesn't break anything */
   char                buf2[FILEPATH_LEN_MAX], fullstr[FILEPATH_LEN_MAX],
      nstr[] = "null";

   buf[0] = 0;
   buf2[0] = 0;
   fullstr[0] = 0;
   f = 0;
   j = 0;

   if (params)
     {
	word(params, 1, fullstr);
	if (fullstr && !strncmp(fullstr, "full", 5))
	  {
	     f++;
	  }
     }

   lst = (Snapshot **) ListItemType(&num, LIST_TYPE_SNAPSHOT);
   if (!num)
     {
	Esnprintf(buf, sizeof(buf), "Error: no remembered windows\n");
     }
   else
     {
	if (f)
	  {
	     for (i = 0; i < num; i++)
	       {
		  if (!lst[i] || (lst[i] && !lst[i]->used))
		     j++;
	       }
	     Esnprintf(buf, sizeof(buf), "Number of remembered windows: %d\n",
		       num - j);
	  }
	/* strncat(buf, buf2, sizeof(buf)); */
	for (i = 0; i < num; i++)
	  {
	     if (lst[i] && lst[i]->used)
	       {
		  if (!f)
		    {
		       Esnprintf(buf2, sizeof(buf2), "%s\n",
				 lst[i]->name ? lst[i]->name : nstr);
		    }
		  else
		    {
		       Esnprintf(buf2, sizeof(buf2),
				 "             Name: %s\n"
				 "     Window Title: %s\n"
				 "      Window Name: %s\n"
				 "     Window Class: %s\n"
				 "      Border Name: %s\n"
				 /*"             Used: %s\n" */
				 "      use_desktop: %d     desktop: %d      area (x, y): %d, %d\n"
				 "           use_wh: %d      (w, h): %d, %d\n"
				 "           use_xy: %d      (x, y): %d, %d\n"
				 "        use_layer: %d       layer: %d\n"
				 "       use_sticky: %d      sticky: %d\n"
				 "        use_shade: %d       shade: %d\n"
				 "      use_command: %d     command: %s\n"
				 "  use_skipwinlist: %d skipwinlist: %d\n"
				 "    use_skiplists: %d    skiptask: %d        skipfocus: %d\n"
				 "   use_neverfocus: %d  neverfocus: %d\n\n",
				 lst[i]->name ? lst[i]->name : nstr,
				 lst[i]->win_title ? lst[i]->win_title : nstr,
				 lst[i]->win_name ? lst[i]->win_name : nstr,
				 lst[i]->win_class ? lst[i]->win_class : nstr,
				 lst[i]->
				 border_name ? lst[i]->border_name : nstr,
				 /*lst[i]->used?"yes":"no", */
				 lst[i]->use_desktop, lst[i]->desktop,
				 lst[i]->area_x, lst[i]->area_y, lst[i]->use_wh,
				 lst[i]->w, lst[i]->h, lst[i]->use_xy,
				 lst[i]->x, lst[i]->y, lst[i]->use_layer,
				 lst[i]->layer, lst[i]->use_sticky,
				 lst[i]->sticky, lst[i]->use_shade,
				 lst[i]->shade, lst[i]->use_cmd,
				 lst[i]->cmd ? lst[i]->cmd : nstr,
				 lst[i]->use_skipwinlist, lst[i]->skipwinlist,
				 lst[i]->use_skiplists, lst[i]->skiptask,
				 lst[i]->skipfocus, lst[i]->use_neverfocus,
				 lst[i]->neverfocus);
		    }
	       }
	     else
	       {
		  /* null snapshot or unused: argh hot grits, hot grits!!! :) */
		  buf2[0] = 0;
	       }

	     if (strlen(buf) + strlen(buf2) > sizeof(buf))
	       {
		  CommsSend(c, buf);
		  buf[0] = 0;
	       }
	     strncat(buf, buf2, sizeof(buf));
	  }
     }

   if (buf)
      CommsSend(c, buf);
}

static void
IPC_Hints(const char *params, Client * c)
{
   char                buf[FILEPATH_LEN_MAX];
   char                param1[FILEPATH_LEN_MAX];
   char                param2[FILEPATH_LEN_MAX];

   buf[0] = 0;
   param1[0] = 0;
   param2[0] = 0;

   word(params, 1, param1);
   word(params, 2, param2);

   if (!strcmp(param1, "xroot"))
     {
	if (!strncmp(param2, "norm", 4))
	   Conf.hints.set_xroot_info_on_root_window = 0;
	else if (!strncmp(param2, "root", 4))
	   Conf.hints.set_xroot_info_on_root_window = 1;
     }

   Esnprintf(buf, sizeof(buf), "Set _XROOT* hints: %s",
	     (Conf.hints.set_xroot_info_on_root_window) ? "root" : "normal");

   CommsSend(c, buf);
}

static void
IPC_Debug(const char *params, Client * c)
{
   char                buf[FILEPATH_LEN_MAX];
   char                param1[FILEPATH_LEN_MAX];
   char                param2[FILEPATH_LEN_MAX];

   buf[0] = 0;
   param1[0] = 0;
   param2[0] = 0;

   word(params, 1, param1);
   word(params, 2, param2);

   if (!strncmp(param1, "event", 2))
     {
	EventDebugInit(param2);
     }

   CommsSend(c, buf);
}

static void
IPC_ClientSet(const char *params, Client * c)
{
   char                param1[FILEPATH_LEN_MAX], param2[FILEPATH_LEN_MAX];

   word(params, 1, param1);
   word(params, 2, param2);

   if (!strcmp(param1, "clientname"))
     {
	if (c->clientname)
	   Efree(c->clientname);
	c->clientname = Estrdup(param2);
     }
   else if (!strcmp(param1, "version"))
     {
	if (c->version)
	   Efree(c->version);
	c->version = Estrdup(param2);
     }
   else if (!strcmp(param1, "author"))
     {
	if (c->author)
	   Efree(c->author);
	c->author = Estrdup(param2);
     }
   else if (!strcmp(param1, "email"))
     {
	if (c->email)
	   Efree(c->email);
	c->email = Estrdup(param2);
     }
   else if (!strcmp(param1, "web"))
     {
	if (c->web)
	   Efree(c->web);
	c->web = Estrdup(param2);
     }
   else if (!strcmp(param1, "address"))
     {
	if (c->address)
	   Efree(c->address);
	c->address = Estrdup(param2);
     }
   else if (!strcmp(param1, "info"))
     {
	if (c->info)
	   Efree(c->info);
	c->info = Estrdup(param2);
     }
   else if (!strcmp(param1, "pixmap"))
     {
	c->pmap = 0;
	sscanf(param2, "%x", (int *)&c->pmap);
     }
}

static void
IPC_Reply(const char *params, Client * c __UNUSED__)
{
   char                param1[FILEPATH_LEN_MAX], param2[FILEPATH_LEN_MAX];

   word(params, 1, param1);

   if (!strcmp(param1, "imageclass"))
     {
	/* Reply format "reply imageclass NAME 24243" */
	word(params, 2, param1);
	word(params, 3, param2);
	HonorIclass(param1, atoi(param2));
     }
}

static void
IPC_ThemeGet(const char *params __UNUSED__, Client * c)
{
   char               *s1;

   s1 = ThemeGetDefault();
   if (s1)
     {
	CommsSend(c, s1);
	Efree(s1);
     }
   else
      CommsSend(c, "");
}

static void
IPC_ThemeSet(const char *params, Client * c __UNUSED__)
{
   char                param1[FILEPATH_LEN_MAX], sss[FILEPATH_LEN_MAX];

   word(params, 1, param1);

   if (exists(param1))
     {
	ThemeSetDefault(param1);
	Esnprintf(sss, sizeof(sss), "restart_theme %s", param1);
	SessionExit(sss);
     }
}

static void
IPC_BackgroundsList(const char *params, Client * c)
{
   char                param1[FILEPATH_LEN_MAX];
   Background        **bg;
   int                 i, num, len = 0;
   char               *buf = NULL;

   word(params, 1, param1);

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

static void
IPC_BackgroundDestroy(const char *params, Client * c __UNUSED__)
{
   char                param1[FILEPATH_LEN_MAX];

   word(params, 1, param1);

   BackgroundDestroyByName(param1);
}

static void
IPC_BackgroundUse(const char *params, Client * c __UNUSED__)
{
   char                param1[FILEPATH_LEN_MAX], w[FILEPATH_LEN_MAX];
   Background         *bg;
   int                 i, wd;

   word(params, 1, param1);

   bg = (Background *) FindItem(param1, 0, LIST_FINDBY_NAME,
				LIST_TYPE_BACKGROUND);
   if (bg)
     {
	wd = 2;
	w[0] = ' ';
	while (w[0])
	  {
	     w[0] = 0;
	     word(params, wd++, w);
	     if (w[0])
	       {
		  i = atoi(w);
		  DesktopSetBg(i, bg, 1);
	       }
	  }
     }
}

static void
IPC_BackgroundUseNone(const char *params, Client * c __UNUSED__)
{
   char                w[FILEPATH_LEN_MAX];
   int                 i, wd;

   wd = 1;
   w[0] = ' ';
   while (w[0])
     {
	w[0] = 0;
	word(params, wd++, w);
	if (w[0])
	  {
	     i = atoi(w);
	     DesktopSetBg(i, NULL, 1);
	  }
     }
}

static void
IPC_BackgroundUsed(const char *params, Client * c)
{
   char                param1[FILEPATH_LEN_MAX], buf[FILEPATH_LEN_MAX];
   Background         *bg;
   int                 i;

   word(params, 1, param1);

   bg = (Background *) FindItem(param1, 0, LIST_FINDBY_NAME,
				LIST_TYPE_BACKGROUND);
   buf[0] = 0;
   if (bg)
     {
	for (i = 0; i < ENLIGHTENMENT_CONF_NUM_DESKTOPS; i++)
	  {
	     if (desks.desk[i].bg == bg)
	       {
		  Esnprintf(param1, sizeof(param1), "%i\n", i);
		  strcat(buf, param1);
	       }
	  }
     }
   CommsSend(c, buf);
}

static void
IPC_KeybindingsGet(const char *params __UNUSED__, Client * c)
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
			    buf = Erealloc(buf, strlen(buf) + strlen(buf2) + 1);
			    strcat(buf, buf2);
			 }
		       else
			  buf = Estrdup(buf2);
		    }
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

static void
IPC_KeybindingsSet(const char *params, Client * c __UNUSED__)
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

   ss = atword(params, 1);
   if (ss)
     {
	i = 0;
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

static void
IPC_BackgroundColormodifierSet(const char *params, Client * c __UNUSED__)
{
   Background         *bg;
   ColorModifierClass *cm;
   int                 i;
   char                buf[FILEPATH_LEN_MAX], buf2[FILEPATH_LEN_MAX];

   if (params == NULL)
      return;

   sscanf(params, "%1000s %1000s", buf, buf2);
   bg = (Background *) FindItem(buf, 0, LIST_FINDBY_NAME, LIST_TYPE_BACKGROUND);
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

static void
IPC_BackgroundColormodifierGet(const char *params, Client * c)
{
   char                param1[FILEPATH_LEN_MAX], buf[FILEPATH_LEN_MAX];
   Background         *bg;

   if (params == NULL)
      return;

   sscanf(params, "%1000s", param1);
   bg = (Background *) FindItem(param1, 0, LIST_FINDBY_NAME,
				LIST_TYPE_BACKGROUND);
   Esnprintf(buf, sizeof(buf), "(null)");
   if ((bg) && (bg->cmclass))
      Esnprintf(buf, sizeof(buf), "%s", bg->cmclass->name);
   CommsSend(c, buf);
}

static void
IPC_ColormodifierDelete(const char *params, Client * c __UNUSED__)
{
   char                param1[FILEPATH_LEN_MAX], buf[FILEPATH_LEN_MAX];
   ColorModifierClass *cm;

   if (params == NULL)
      return;

   sscanf(params, "%1000s", param1);
   cm = (ColorModifierClass *) FindItem(param1, 0, LIST_FINDBY_NAME,
					LIST_TYPE_COLORMODIFIER);
   Esnprintf(buf, sizeof(buf), "(null)");
   if (cm)
      FreeCMClass(cm);
}

static void
IPC_ColormodifierGet(const char *params, Client * c)
{
   char                param1[FILEPATH_LEN_MAX];
   char                buf[FILEPATH_LEN_MAX], buf2[FILEPATH_LEN_MAX];
   ColorModifierClass *cm;
   int                 i;

   if (params == NULL)
      return;

   sscanf(params, "%1000s", param1);
   cm = (ColorModifierClass *) FindItem(param1, 0, LIST_FINDBY_NAME,
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

static void
IPC_ColormodifierSet(const char *params, Client * c __UNUSED__)
{
   char                w[FILEPATH_LEN_MAX];
   ColorModifierClass *cm;
   int                 i, j, k;
   char               *name;
   int                 rnum = 0, gnum = 0, bnum = 0;
   unsigned char      *rpx = NULL, *rpy = NULL;
   unsigned char      *gpx = NULL, *gpy = NULL;
   unsigned char      *bpx = NULL, *bpy = NULL;

   if (params == NULL)
      return;

   sscanf(params, "%1000s", w);
   cm = (ColorModifierClass *) FindItem(w, 0, LIST_FINDBY_NAME,
					LIST_TYPE_COLORMODIFIER);
   name = Estrdup(w);
   i = 2;
   word(params, i++, w);
   rnum = atoi(w);
   j = 0;
   rpx = Emalloc(rnum);
   rpy = Emalloc(rnum);
   while (j < rnum)
     {
	word(params, i++, w);
	k = atoi(w);
	rpx[j] = k;
	word(params, i++, w);
	k = atoi(w);
	rpy[j++] = k;
     }
   word(params, i++, w);
   gnum = atoi(w);
   j = 0;
   gpx = Emalloc(gnum);
   gpy = Emalloc(gnum);
   while (j < gnum)
     {
	word(params, i++, w);
	k = atoi(w);
	gpx[j] = k;
	word(params, i++, w);
	k = atoi(w);
	gpy[j++] = k;
     }
   word(params, i++, w);
   bnum = atoi(w);
   j = 0;
   bpx = Emalloc(bnum);
   bpy = Emalloc(bnum);
   while (j < bnum)
     {
	word(params, i++, w);
	k = atoi(w);
	bpx[j] = k;
	word(params, i++, w);
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

static void
IPC_BackgroundGet(const char *params, Client * c)
{
   char                param1[FILEPATH_LEN_MAX], buf[FILEPATH_LEN_MAX];
   Background         *bg;
   int                 r, g, b;

   if (params == NULL)
      return;

   sscanf(params, "%1000s", param1);

   Esnprintf(buf, sizeof(buf), "(null)");

   bg = (Background *) FindItem(param1, 0, LIST_FINDBY_NAME,
				LIST_TYPE_BACKGROUND);
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

static void
IPC_BackgroundSet(const char *params, Client * c __UNUSED__)
{
   char                name[FILEPATH_LEN_MAX];
   Background         *bg;
   XColor              xclr;
   int                 i, r, g, b;
   char                bgf[FILEPATH_LEN_MAX], topf[FILEPATH_LEN_MAX];
   int                 updated = 0, tile, keep_aspect, tkeep_aspect;
   int                 xjust, yjust, xperc, yperc;
   int                 txjust, tyjust, txperc, typerc;

   if (params == NULL)
      return;

   name[0] = bgf[0] = topf[0] = '\0';
   r = 99;
   i = sscanf(params,
	      "%4000s %i %i %i %4000s %i %i %i %i %i %i %4000s %i %i %i %i %i",
	      name, &r, &g, &b,
	      bgf, &tile, &keep_aspect, &xjust, &yjust, &xperc, &yperc,
	      topf, &tkeep_aspect, &txjust, &tyjust, &txperc, &typerc);
   ESetColor(&xclr, r, g, b);
   bg =
      (Background *) FindItem(name, 0, LIST_FINDBY_NAME, LIST_TYPE_BACKGROUND);
   if (bg)
     {
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
	bg->bg.file = (bgf[0]) ? Estrdup(bgf) : NULL;
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
	bg->top.file = (topf[0]) ? Estrdup(topf) : NULL;
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
		     DesktopSetBg(i, bg, 0);
	       }
	  }
     }
   else
     {
	bg = BackgroundCreate(name, &xclr, bgf, tile, keep_aspect, xjust,
			      yjust, xperc, yperc, topf, tkeep_aspect,
			      txjust, tyjust, txperc, typerc);
     }
}

static void
IPC_BackgroundApply(const char *params, Client * c)
{
   char                name[FILEPATH_LEN_MAX];
   Window              win = 0;
   Background         *bg;

   if (params == NULL)
      return;

   sscanf(params, "%lx %1000s", &win, name);
   bg = (Background *) FindItem(name, 0, LIST_FINDBY_NAME,
				LIST_TYPE_BACKGROUND);
   if (bg)
      BackgroundApply(bg, win, 0);
   CommsSend(c, "done");
}

static void
IPC_ControlsSet(const char *s, Client * c __UNUSED__)
{
   char                w[FILEPATH_LEN_MAX];
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
	     Conf.focus.new_transients_get_focus_if_group_focused = atoi(w);
	  }
	else if (!strcmp(w, "MANUAL_PLACEMENT:"))
	  {
	     word(s, wd, w);
	     Conf.place.manual = atoi(w);
	  }
	else if (!strcmp(w, "MANUAL_PLACEMENT_MOUSE_POINTER:"))
	  {
	     word(s, wd, w);
	     Conf.place.manual_mouse_pointer = atoi(w);
	  }
	else if (!strcmp(w, "RAISE_ON_NEXT_FOCUS:"))
	  {
	     word(s, wd, w);
	     Conf.focus.raise_on_next = atoi(w);
	  }
	else if (!strcmp(w, "RAISE_AFTER_NEXT_FOCUS:"))
	  {
	     word(s, wd, w);
	     Conf.warplist.raise_on_select = atoi(w);
	  }
	else if (!strcmp(w, "DISPLAY_WARP:"))
	  {
	     word(s, wd, w);
	     Conf.warplist.enable = atoi(w);
	  }
	else if (!strcmp(w, "WARP_ON_NEXT_FOCUS:"))
	  {
	     word(s, wd, w);
	     Conf.focus.warp_on_next = atoi(w);
	  }
	else if (!strcmp(w, "WARP_AFTER_NEXT_FOCUS:"))
	  {
	     word(s, wd, w);
	     Conf.warplist.warp_on_select = atoi(w);
	  }
	else if (!strcmp(w, "EDGE_FLIP_RESISTANCE:"))
	  {
	     word(s, wd, w);
	     Conf.edge_flip_resistance = atoi(w);
	     EdgeWindowsShow();
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

static void
IPC_ControlsGet(const char *s __UNUSED__, Client * c)
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
	     Conf.place.manual, Conf.place.manual_mouse_pointer,
	     Conf.focus.raise_on_next,
	     Conf.warplist.raise_on_select, Conf.warplist.enable,
	     Conf.focus.warp_on_next,
	     Conf.warplist.warp_on_select, Conf.edge_flip_resistance);
   CommsSend(c, buf);
}

static void
IPC_CallRaw(const char *params, Client * c __UNUSED__)
{
   const char         *par;
   int                 aid;

   if (params == NULL)
      return;

   aid = -1;
   sscanf(params, "%i", &aid);
   par = atword(params, 2);
   ActionsCall(aid, NULL, par);
}

static void
EwinShowInfo1(const EWin * ewin)
{
   Border              NoBorder, *border;

   border = ewin->border;
   if (border == NULL)
     {
	border = &NoBorder;
	memset(border, 0, sizeof(Border));
     }

   IpcPrintf("***CLIENT***\n"
	     "CLIENT_WIN_ID:          %#10lx\n"
	     "FRAME_WIN_ID:           %#10lx\n"
	     "CONTAINER_WIN_ID:       %#10lx\n"
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
	     "ICON_WIN_ID:            %#10lx\n"
	     "ICON_PIXMAP,MASK_ID:    %#10lx , %#10lx\n"
	     "CLIENT_GROUP_LEADER_ID: %#10lx\n"
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
	     "MWM_MAXIMIZE:           %5i\n",
	     ewin->client.win, ewin->win, ewin->win_container,
	     ewin->x, ewin->y, ewin->w, ewin->h,
	     border->name,
	     border->border.left, border->border.right,
	     border->border.top, border->border.bottom,
	     ewin->desktop,
	     ewin->num_groups, ewin->docked, ewin->sticky,
	     ewin->shown, ewin->iconified, ewin->shaded,
	     ewin->active, ewin->layer, ewin->never_use_area,
	     ewin->floating, ewin->client.w, ewin->client.h,
	     ewin->client.icon_win,
	     ewin->client.icon_pmap, ewin->client.icon_mask,
	     ewin->client.group,
	     ewin->client.need_input, ewin->client.transient,
	     SS(ewin->icccm.wm_name), SS(ewin->icccm.wm_res_class),
	     SS(ewin->icccm.wm_res_name), SS(ewin->icccm.wm_command),
	     SS(ewin->icccm.wm_machine), SS(ewin->icccm.wm_icon_name),
	     ewin->client.is_group_leader,
	     ewin->client.no_resize_h, ewin->client.no_resize_v,
	     ewin->client.shaped,
	     ewin->client.width.min, ewin->client.height.min,
	     ewin->client.width.max, ewin->client.height.max,
	     ewin->client.base_w, ewin->client.base_h,
	     ewin->client.w_inc, ewin->client.h_inc,
	     ewin->client.aspect_min, ewin->client.aspect_max,
	     ewin->client.mwm_decor_border, ewin->client.mwm_decor_resizeh,
	     ewin->client.mwm_decor_title, ewin->client.mwm_decor_menu,
	     ewin->client.mwm_decor_minimize, ewin->client.mwm_decor_maximize);
}

static void
EwinShowInfo2(const EWin * ewin)
{
   Border              NoBorder, *border;

   border = ewin->border;
   if (border == NULL)
     {
	border = &NoBorder;
	memset(border, 0, sizeof(Border));
     }

   IpcPrintf("WM_NAME                 %s\n"
	     "WM_ICON_NAME            %s\n"
	     "WM_CLASS name.class     %s.%s\n"
	     "WM_COMMAND              %s\n"
	     "WM_CLIENT_MACHINE       %s\n"
	     "Client window           %#10lx   x,y %4i,%4i   wxh %4ix%4i\n"
	     "Frame window            %#10lx   x,y %4i,%4i   wxh %4ix%4i\n"
	     "Container window        %#10lx\n"
	     "Border                  %s   lrtb %i,%i,%i,%i\n"
	     "Icon window, pixmap, mask %#10lx, %#10lx, %#10lx\n"
	     "Is group leader  %i  Window group leader %#lx   Client leader %#10lx\n"
	     "Has transients   %i  Transient type  %i  Transient for %#10lx\n"
	     "No resize H/V    %i/%i       Shaped      %i\n"
	     "Base, min, max, inc w/h %ix%i, %ix%i, %ix%i %ix%i\n"
	     "Aspect min, max         %5.5f, %5.5f\n"
	     "MWM border %i resizeh %i title %i menu %i minimize %i maximize %i\n"
	     "NeedsInput   %i   TakeFocus    %i   FocusNever   %i   FocusClick   %i\n"
	     "NeverUseArea %i   FixedPos     %i\n"
	     "Desktop      %i   Layer        %i\n"
	     "Iconified    %i   Sticky       %i   Shaded       %i   Docked       %i\n"
	     "State        %i   Shown        %i   Active       %i   Floating     %i\n"
	     "Member of groups        %i\n",
	     SS(ewin->icccm.wm_name),
	     SS(ewin->icccm.wm_icon_name),
	     SS(ewin->icccm.wm_res_name), SS(ewin->icccm.wm_res_class),
	     SS(ewin->icccm.wm_command),
	     SS(ewin->icccm.wm_machine),
	     ewin->client.win,
	     ewin->client.x, ewin->client.y, ewin->client.w, ewin->client.h,
	     ewin->win, ewin->x, ewin->y, ewin->w, ewin->h,
	     ewin->win_container,
	     border->name,
	     border->border.left, border->border.right,
	     border->border.top, border->border.bottom,
	     ewin->client.icon_win,
	     ewin->client.icon_pmap, ewin->client.icon_mask,
	     ewin->client.is_group_leader,
	     ewin->client.group, ewin->client.client_leader,
	     ewin->has_transients,
	     ewin->client.transient, ewin->client.transient_for,
	     ewin->client.no_resize_h, ewin->client.no_resize_v,
	     ewin->client.shaped,
	     ewin->client.base_w, ewin->client.base_h,
	     ewin->client.width.min, ewin->client.height.min,
	     ewin->client.width.max, ewin->client.height.max,
	     ewin->client.w_inc, ewin->client.h_inc,
	     ewin->client.aspect_min, ewin->client.aspect_max,
	     ewin->client.mwm_decor_border, ewin->client.mwm_decor_resizeh,
	     ewin->client.mwm_decor_title, ewin->client.mwm_decor_menu,
	     ewin->client.mwm_decor_minimize, ewin->client.mwm_decor_maximize,
	     ewin->client.need_input, ewin->client.take_focus,
	     ewin->neverfocus, ewin->focusclick,
	     ewin->never_use_area, ewin->fixedpos,
	     ewin->desktop, ewin->layer,
	     ewin->iconified, ewin->sticky, ewin->shaded, ewin->docked,
	     ewin->state, ewin->shown, ewin->active, ewin->floating,
	     ewin->num_groups);
}

static void
IPC_EwinInfo(const char *params, Client * c __UNUSED__)
{
   char                param1[FILEPATH_LEN_MAX];
   EWin               *ewin;

   if (params == NULL)
      return;

   sscanf(params, "%1000s", param1);

   if (!strcmp(param1, "all"))
     {
	EWin               *const *lst;
	int                 i, num;

	lst = EwinListGetAll(&num);
	for (i = 0; i < num; i++)
	   EwinShowInfo1(lst[i]);
     }
   else
     {
	ewin = IpcFindEwin(param1);
	if (ewin)
	   EwinShowInfo1(ewin);
	else
	   IpcPrintf("No matching EWin found\n");
     }
}

static void
IPC_EwinInfo2(const char *params, Client * c __UNUSED__)
{
   char                param1[FILEPATH_LEN_MAX];
   EWin               *ewin;

   if (params == NULL)
      return;

   sscanf(params, "%1000s", param1);

   if (!strcmp(param1, "all"))
     {
	EWin               *const *lst;
	int                 i, num;

	lst = EwinListGetAll(&num);
	for (i = 0; i < num; i++)
	   EwinShowInfo2(lst[i]);
     }
   else
     {
	ewin = IpcFindEwin(param1);
	if (ewin)
	   EwinShowInfo2(ewin);
	else
	   IpcPrintf("No matching EWin found\n");
     }
}

static void
IPC_MiscInfo(const char *params __UNUSED__, Client * c)
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

static void
IPC_Reparent(const char *params, Client * c __UNUSED__)
{
   char                param1[FILEPATH_LEN_MAX];
   char                param2[FILEPATH_LEN_MAX];
   EWin               *ewin, *enew;

   if (params == NULL)
      return;

   sscanf(params, "%100s %100s", param1, param2);

   ewin = IpcFindEwin(param1);
   enew = IpcFindEwin(param2);
   if (!ewin || !enew)
      IpcPrintf("No matching client or target EWin found\n");
   else
      EwinReparent(ewin, enew->client.win);
}

/* the IPC Array */

/* the format of an IPC member of the IPC array is as follows:
 * {
 *    NameOfMyFunction,
 *    "command_name",
 *    "quick-help explanation",
 *    "extended help data"
 *    "may go on for several lines, be sure\n"
 *    "to add line feeds when you need them and to \"quote\"\n"
 *    "properly"
 * }
 *
 * when you add a function into this array, make sure you also add it into
 * the declarations above and also put the function in this file.  PLEASE
 * if you add a new function in, add help to it also.  since my end goal
 * is going to be to have this whole IPC usable by an end-user or to your
 * scripter, it should be easy to learn to use without having to crack
 * open the source code.
 * --Mandrake
 */
static void         IPC_Help(const char *params, Client * c);

IPCStruct           IPCArray[] = {
   {
    IPC_Help,
    "help", "?",
    "gives you this help screen",
    "Additional parameters will retrieve help on many topics - "
    "\"help <command>\".\nuse \"help all\" for a list of commands."},
   {
    IPC_Version,
    "version", "ver",
    "displays the current version of Enlightenment running",
    NULL},
   {
    IPC_Nop,
    "nop", NULL,
    "IPC No-operation - returns nop",
    NULL},
   {
    IPC_Copyright,
    "copyright", NULL,
    "displays copyright information for Enlightenment",
    NULL},
   {
    IPC_AutoSave,
    "autosave", NULL,
    "toggle the Automatic Saving Feature",
    "Use \"autosave ?\" to list the current status\n"
    "use \"autosave on\" or \"autosave off\" to toggle the status"},
   {
    IPC_DefaultTheme,
    "default_theme", NULL,
    "toggle the default theme",
    "Use \"default_theme ?\" to get the current default theme\n"
    "use \"default_theme /path/to/theme\"\n"
    "you can retrieve a list of available themes from the "
    "\"list_themes\" command"},
   {
    IPC_Restart,
    "restart", NULL,
    "Restart Enlightenment",
    NULL},
   {
    IPC_RestartWM,
    "restart_wm", NULL,
    "Restart another window manager",
    "Use \"restart_wm <wmname>\" to start another window manager.\n"
    "Example: \"restart_wm fvwm\""},
   {
    IPC_RestartTheme,
    "restart_theme", NULL,
    "Restart with another theme",
    "Use \"restart_theme <themename>\" to restart enlightenment "
    "with another theme\nExample: \"restart_theme icE\""},
   {
    IPC_Exit,
    "exit", "q",
    "Exit Enlightenment",
    NULL},
   {
    IPC_ForceSave,
    "save_config", "s",
    "Force Enlightenment to save settings now",
    NULL},
   {
    IPC_SMFile,
    "sm_file", NULL,
    "Change the default prefix used for session saves",
    "Average users are encouraged not to touch this setting.\n"
    "Use \"sm_file ?\" to retrieve the current session management "
    "file prefix\nUse \"sm_file /path/to/prefix/filenameprefix\" "
    "to change."},
   {
    IPC_ListThemes,
    "list_themes", "tl",
    "List currently available themes",
    NULL},
   {
    IPC_GotoDesktop,
    "goto_desktop", "sd",
    "Change currently active destkop",
    "Use \"goto_desktop num\" to go to a specific desktop.\n"
    "Use \"goto_desktop next\" and \"goto_desktop prev\" to go to "
    "the next and\n     previous desktop\n"
    "Use \"goto_desktop ?\" to find out what desktop you are " "currently on"},
   {
    IPC_GotoArea,
    "goto_area", "sa",
    "Change currently active area",
    "Use \"goto_area <horiz> <vert>\" to go to a specific desktop.\n"
    "Use \"goto_desktop next <vert/horiz>\" and \"goto_desktop "
    "prev <vert/horiz>\" to go to the next and\n     "
    "previous areas\nUse \"goto_area ?\" to find out what area "
    "you are currently on"},
   {
    IPC_ShowIcons,
    "show_icons", NULL,
    "Obsolete - Toggle the display of icons on the desktop",
    "Use \"show_icons on\" and \"show_icons off\" to change this setting\n"
    "Use \"show_icons ?\" to retrieve the current setting"},
   {
    IPC_FocusMode,
    "focus_mode", "sf",
    "Change the current focus mode setting",
    "Use \"focus_mode <mode>\" to change the focus mode.\n"
    "Use \"focus_mode ?\" to retrieve the current setting\n" "Focus Types:\n"
    "click: This is the traditional click-to-focus mode.\n"
    "clicknograb: This is a similar focus mode, but without the "
    "grabbing of the click\n    "
    "(you cannot click anywhere in a window to focus it)\n"
    "pointer: The focus will follow the mouse pointer\n"
    "sloppy: in sloppy-focus, the focus follows the mouse, "
    "but when over\n    "
    "the desktop background the last window does not lose the focus"},
   {
    IPC_AdvancedFocus,
    "advanced_focus", "sfa",
    "Toggle Advanced Focus Settings",
    "use \"advanced_focus <option> <on/off/?>\" to change.\n"
    "the options you may set are:\n"
    "new_window_focus : all new windows get the keyboard focus\n"
    "new_popup_window_focus : all new transient windows get focus\n"
    "new_popup_of_owner_focus : transient windows from apps that have\n"
    "   focus already may receive focus\n"
    "raise_on_keyboard_focus_switch: Raise windows when switching focus\n"
    "   with the keyboard\n"
    "raise_after_keyboard_focus_switch: Raise windows after switching "
    "focus\n" "   with the keyboard\n"
    "pointer_to_keyboard_focus_window: Send the pointer to the focused\n"
    "   window when changing focus with the keyboard\n"
    "pointer_after_keyboard_focus_window: Send the pointer to the " "focused\n"
    "   window after changing focus with the keyboard\n"
    "transients_follow_leader: popup windows appear together with the\n"
    "   window that created them.\n"
    "switch_to_popup_location: switch to where a popup window appears\n"
    "focus_list: display and use focus list (requires XKB)\n"
    "manual_placement: place all new windows by hand\n"
    "manual_placement_mouse_pointer: place all new windows under mouse pointer"},
   {
    IPC_NumDesks,
    "num_desks", "snd",
    "Change the number of available desktops",
    "Use \"num_desks <num>\" to change the available number of desktops.\n"
    "Use \"num_desks ?\" to retrieve the current setting"},
   {
    IPC_NumAreas,
    "num_areas", "sna",
    "Change the size of the virtual desktop",
    "Use \"num_areas <width> <height>\" to change the size of the "
    "virtual desktop.\nExample: \"num_areas 2 2\" makes 2x2 "
    "virtual destkops\nUse \"num_areas ?\" to retrieve the " "current setting"},
   {
    IPC_WinOps,
    "win_op", "wop",
    "Change a property of a specific window",
    "Use \"win_op <windowid> <property> <value>\" to change the "
    "property of a window\nYou can use the \"window_list\" "
    "command to retrieve a list of available windows\n"
    "You can use ? after most of these commands to receive the current\n"
    "status of that flag\n"
    "available win_op commands are:\n"
    "  win_op <windowid> <close/annihilate>\n"
    "  win_op <windowid> <iconify/shade/stick>\n"
    "  win_op <windowid> toggle_<width/height/size> <conservative/available/xinerama>\n"
    "          (or none for absolute)\n"
    "  win_op <windowid> border <BORDERNAME>\n"
    "  win_op <windowid> desk <desktochangeto/next/prev>\n"
    "  win_op <windowid> area <x> <y>\n"
    "  win_op <windowid> <raise/lower>\n"
    "  win_op <windowid> <move/resize> <x> <y>\n"
    "          (you can use ? and ?? to retreive client and frame locations)\n"
    "  win_op <windowid> focus\n"
    "  win_op <windowid> title <title>\n"
    "  win_op <windowid> layer <0-100,4=normal>\n"
    "  win_op <windowid> <fixedpos/never_use_area/focusclick/neverfocus>\n"
    "<windowid> may be substituted with \"current\" to use the current window"},
   {
    IPC_WinList,
    "window_list", "wl",
    "Get a list of currently open windows",
    "the list will be returned in the following "
    "format - \"window_id : title\"\n"
    "you can get an extended list using \"window_list extended\"\n"
    "returns the following format:\n\"window_id : title :: "
    "desktop : area_x area_y : x_coordinate y_coordinate\""},
   {
    IPC_ButtonShow,
    "button_show", NULL,
    "Show or Hide buttons on desktop",
    "use \"button_show <button/buttons/all_buttons_except/all> "
    "<BUTTON_STRING>\"\nexamples: \"button_show buttons all\" "
    "(removes all buttons and the dragbar)\n\"button_show\" "
    "(removes all buttons)\n \"button_show buttons CONFIG*\" "
    "(removes all buttons with CONFIG in the start)"},
   {
    IPC_FX,
    "fx", NULL,
    "Toggle various effects on/off",
    "Use \"fx <effect> <mode>\" to set the mode of a particular effect\n"
    "Use \"fx <effect> ?\" to get the current mode\n"
    "the following effects are available\n"
    "ripples <on/off> (ripples that act as a water effect on the screen)\n"
    "deskslide <on/off> (slide in desktops on desktop change)\n"
    "mapslide <on/off> (slide in new windows)\n"
    "raindrops <on/off> (raindrops will appear across your desktop)\n"
    "menu_animate <on/off> (toggles the animation of menus "
    "as they appear)\n"
    "animate_win_shading <on/off> (toggles the animation of "
    "window shading)\n"
    "window_shade_speed <#> (number of pixels/sec to shade a window)\n"
    "dragbar <on/off/left/right/top/bottom> (changes " "location of dragbar)\n"
    "tooltips <on/off/#> (changes state of tooltips and "
    "seconds till popup)\n"
    "autoraise <on/off/#> (changes state of autoraise and "
    "seconds till raise)\n"
    "edge_resistance <#/?/off> (changes the amount (in 1/100 seconds)\n"
    "   of time to push for resistance to give)\n"
    "edge_snap_resistance <#/?> (changes the number of pixels that "
    "a window will\n   resist moving against another window\n"
    "audio <on/off> (changes state of audio)\n"
    "-  seconds for tooltips and autoraise can have less than one second\n"
    "   (i.e. 0.5) or greater (1.3, 3.5, etc)"},
   {
    IPC_DockConfig,
    "dock", NULL,
    "Enable/Disable dock, or change dock position and direction",
    "use \"dock support <on/off/?>\" to test, enable, or disable the dock\n"
    "use \"dock direction <up/down/left/right/?>\" to set or "
    "test direction\n"
    "use \"dock start_pos ?\" to test the starting x y coords\n"
    "use \"dock start_pos x y\" to set the starting x y coords"},
   {
    IPC_MoveMode,
    "move_mode", "smm",
    "Toggle the Window move mode",
    "use \"move_mode <opaque/lined/box/shaded/semi-solid/translucent>\" "
    "to set\nuse \"move_mode ?\" to get the current mode"},
   {
    IPC_ResizeMode,
    "resize_mode", "srm",
    "Toggle the Window resize mode",
    "use \"resize_mode <opaque/lined/box/shaded/semi-solid>\" "
    "to set\nuse \"resize_mode ?\" to get the current mode"},
   {
    IPC_GeomInfoMode,
    "geominfo_mode", "sgm",
    "Change position of geometry info display during Window move or resize",
    "use \"geominfo_mode <center/corner/never>\" "
    "to set\nuse \"geominfo_mode ?\" to get the current mode"},
   {
    IPC_Pager,
    "pager", NULL,
    "Toggle the status of the Pager and various pager settings",
    "use \"pager <on/off>\" to set the current mode\nuse \"pager ?\" "
    "to get the current mode\n"
    "use \"pager <#> <on/off/?>\" to toggle or test any desktop's pager\n"
    "use \"pager hiq <on/off>\" to toggle high quality pager\n"
    "use \"pager snap <on/off>\" to toggle snapshotting in the pager\n"
    "use \"pager zoom <on/off>\" to toggle zooming in the pager\n"
    "use \"pager title <on/off>\" to toggle title display in the pager\n"
    "use \"pager scanrate <#>\" to toggle number of line update " "per second"},
   {
    IPC_InternalList,
    "internal_list", "il",
    "Retrieve a list of internal items",
    "use \"internal_list <pagers/menus/dialogs/internal_ewin>\"\n"
    "to retrieve a list of various internal window types.\n"
    "(note that listing internal_ewin  doesn't retrieve "
    "dialogs currently)\n"},
   {
    IPC_SetFocus,
    "set_focus", "wf",
    "Set/Retrieve focused window",
    "use \"set_focus <win_id>\" to focus a new window\n"
    "use \"set_focus ?\" to retrieve the currently focused window"},
   {
    IPC_DialogOK,
    "dialog_ok", "dok",
    "Pop up a dialog box with an OK button",
    "use \"dialog_ok <message>\" to pop up a dialog box."},
   {
    IPC_ListClassMembers,
    "list_class", "cl",
    "List all members of a class",
    "use \"list_class <classname>\" to get back a list of class members\n"
    "available classes are:\n" "sounds\n" "actions\n" "backgrounds\n"
    "borders\n" "text\n" "images\n" "cursors\n" "buttons"},
   {
    IPC_PlaySoundClass,
    "play_sound", "ps",
    "Plays a soundclass via E",
    "use \"play_sound <soundclass>\" to play a sound.\n"
    "use \"list_class sounds\" to get a list of available sounds"},
   {
    IPC_SoundClass,
    "soundclass", NULL,
    "Create/Delete soundclasses",
    "use \"soundclass create <classname> <filename>\" to create\n"
    "use \"soundclass delete <classname>\" to delete"},
   {
    IPC_ImageClass,
    "imageclass", NULL,
    "Create/delete/modify/apply an ImageClass",
    "This doesn't do anything yet."},
   {
    IPC_ActionClass,
    "actionclass", NULL,
    "Create/Delete/Modify an ActionClass",
    "This doesn't do anything yet."},
   {
    IPC_ColorModifierClass,
    "colormod", NULL,
    "Create/Delete/Modify a ColorModifierClass",
    "This doesn't do anything yet."},
   {
    IPC_TextClass,
    "textclass", NULL,
    "Create/Delete/Modify/apply a TextClass",
    "This doesn't do anything yet."},
   {
    IPC_Background,
    "background", NULL,
    "Create/Delete/Modify a Background",
    "use \"background\" to list all defined backgrounds.\n"
    "use \"background <name>\" to delete a background.\n"
    "use \"background <name> ?\" to show current values.\n"
    "use \"background <name> <type> <value> to create / modify.\n"
    "(get available types from \"background <name> ?\"."},
   {
    IPC_Border,
    "border", NULL,
    "Create/Delete/Modify a Border",
    "This doesn't do anything yet."},
   {
    IPC_Cursor,
    "cursor", NULL,
    "Create/Delete/Modify a Cursor",
    "This doesn't do anything yet."},
   {
    IPC_Button,
    "button", NULL,
    "Create/Delete/Modify a Button",
    "This doesn't do anything yet."},
   {
    IPC_GeneralInfo,
    "general_info", NULL,
    "Retrieve some general information",
    "use \"general_info <info>\" to retrieve information\n"
    "available info is: screen_size"},
   {
    IPC_ReloadMenus,
    "reload_menus", NULL,
    "Reload menus.cfg without restarting (Asmodean_)",
    NULL},
   {
    IPC_GroupInfo,
    "group_info", "gl",
    "Retrieve some info on groups",
    "use \"group_info [group_index]\""},
   {
    IPC_GroupOps,
    "group_op", "gop",
    "Group operations",
    "use \"group_op <windowid> <property> [<value>]\" to perform "
    "group operations on a window.\n" "Available group_op commands are:\n"
    "  group_op <windowid> start\n"
    "  group_op <windowid> add [<group_index>]\n"
    "  group_op <windowid> remove [<group_index>]\n"
    "  group_op <windowid> break [<group_index>]\n"
    "  group_op <windowid> showhide\n"},
   {
    IPC_Group,
    "group", "gc",
    "Group commands",
    "use \"group <groupid> <property> <value>\" to set group properties.\n"
    "Available group commands are:\n"
    "  group <groupid> num_members <on/off/?>\n"
    "  group <groupid> iconify <on/off/?>\n"
    "  group <groupid> kill <on/off/?>\n" "  group <groupid> move <on/off/?>\n"
    "  group <groupid> raise <on/off/?>\n"
    "  group <groupid> set_border <on/off/?>\n"
    "  group <groupid> stick <on/off/?>\n"
    "  group <groupid> shade <on/off/?>\n"
    "  group <groupid> mirror <on/off/?>\n"},
   {
    IPC_MemDebug,
    "dump_mem_debug", NULL,
    "Dumps memory debugging information out to e.mem.out",
    "Use this command to have E dump its current memory debugging table\n"
    "to the e.mem.out file. NOTE: please read comments at the top of\n"
    "memory.c to see how to enable this. This will let you hunt memory\n"
    "leaks, over-allocations of memory, and other " "memory-related problems\n"
    "very easily with all pointers allocated stamped with a time, call\n"
    "tree that led to that allocation, file and line, "
    "and the chunk size.\n"},
   {
    IPC_Remember,
    "remember", NULL,
    "Remembers parameters for client window ID x",
    "usage:\n" "  remember <windowid> <parameter>...\n"
    "  where parameter is one of: all, none, border, desktop, size,\n"
    "  location, layer, sticky, icon, shade, group, dialog, command\n"
    "  Multiple parameters may be given."},
   {
    IPC_CurrentTheme,
    "current_theme", "tc",
    "Returns the name of the currently used theme",
    NULL},
   {
    IPC_Xinerama,
    "xinerama", NULL,
    "return xinerama information about your current system",
    NULL},
   {
    IPC_ConfigPanel,
    "configpanel", NULL,
    "open up a config window",
    "usage:\n" "  configpanel <panelname>\n"
    "  where panelname is one of the following: focus, moveresize,\n"
    "  desktops, area, placement, icons, autoraise, tooltips,\n"
    "  audio, fx, bg, group_defaults, remember"},
   {
    IPC_RememberList,
    "list_remember", "rl",
    "Retrieve a list of remembered windows and their attributes",
    "usage:\n" "  list_remember [full]\n"
    "  Retrieve a list of remembered windows.  with full, the list\n"
    "  includes the window's remembered attributes."},
   {
    IPC_Hints,
    "hints", NULL,
    "Set hint options",
    "usage:\n" "  hints xroot <normal/root>"},
   {
    IPC_Debug,
    "debug", NULL,
    "Set debug options",
    "usage:\n" "  debug events <EvNo>:<EvNo>..."},
   {
    IPC_ClientSet, "set", NULL, "Set client parameters", NULL},
   {
    IPC_Reply, "reply", NULL, "TBD", NULL},
   {
    IPC_ThemeGet, "get_default_theme", NULL, "TBD", NULL},
   {
    IPC_ThemeSet, "set_default_theme", NULL, "TBD", NULL},
   {
    IPC_BackgroundsList, "list_bg", NULL, "TBD", NULL},
   {
    IPC_BackgroundDestroy, "del_bg", NULL, "TBD", NULL},
   {
    IPC_BackgroundUse, "use_bg", NULL, "TBD", NULL},
   {
    IPC_BackgroundUseNone, "use_no_bg", NULL, "TBD", NULL},
   {
    IPC_BackgroundUsed, "uses_bg", NULL, "TBD", NULL},
   {
    IPC_KeybindingsGet, "get_keybindings", NULL, "TBD", NULL},
   {
    IPC_KeybindingsSet, "set_keybindings", NULL, "TBD", NULL},
   {
    IPC_BackgroundColormodifierSet, "set_bg_colmod", NULL, "TBD", NULL},
   {
    IPC_BackgroundColormodifierGet, "get_bg_colmod", NULL, "TBD", NULL},
   {
    IPC_ColormodifierDelete, "del_colmod", NULL, "TBD", NULL},
   {
    IPC_ColormodifierGet, "get_colmod", NULL, "TBD", NULL},
   {
    IPC_ColormodifierSet, "set_colmod", NULL, "TBD", NULL},
   {
    IPC_BackgroundGet, "get_bg", NULL, "TBD", NULL},
   {
    IPC_BackgroundSet, "set_bg", NULL, "TBD", NULL},
   {
    IPC_BackgroundApply, "draw_bg_to", NULL, "TBD", NULL},
   {
    IPC_ControlsSet, "set_controls", NULL, "TBD", NULL},
   {
    IPC_ControlsGet, "get_controls", NULL, "TBD", NULL},
   {
    IPC_CallRaw, "call_raw", NULL, "TBD", NULL},
   {
    IPC_EwinInfo, "get_client_info", NULL, "Show client window info", NULL},
   {
    IPC_EwinInfo2, "win_info", "wi", "Show client window info", NULL},
   {
    IPC_MiscInfo, "dump_info", NULL, "TBD", NULL},
   {
    IPC_Reparent,
    "reparent", "rep",
    "Reparent window",
    "usage:\n" "  reparent <windowid> <new parent>"},
};

/* The IPC Handler */
/* this is the function that actually loops through the IPC array
 * and finds the command that you were trying to run, and then executes it.
 * you shouldn't have to touch this function
 * - Mandrake
 */
int
HandleIPC(const char *params, Client * c)
{
   int                 i;
   int                 numIPC;
   char                w[FILEPATH_LEN_MAX];
   IPCStruct          *ipc;

   IpcPrintInit();

   word(params, 1, w);

   numIPC = sizeof(IPCArray) / sizeof(IPCStruct);
   for (i = 0; i < numIPC; i++)
     {
	ipc = &IPCArray[i];
	if (!strcmp(w, ipc->commandname) ||
	    (ipc->nick && !strcmp(w, ipc->nick)))
	  {
	     word(params, 2, w);
	     if (w)
		ipc->func(atword(params, 2), c);
	     else
		ipc->func(NULL, c);

	     IpcPrintFlush(c);
	     return 1;
	  }
     }

   return 0;
}

static int
ipccmp(void *p1, void *p2)
{
   return strcmp(((IPCStruct *) p1)->commandname,
		 ((IPCStruct *) p2)->commandname);
}

static void
IPC_Help(const char *params, Client * c __UNUSED__)
{
   char                buf[FILEPATH_LEN_MAX];
   int                 i, numIPC;
   IPCStruct         **lst, *ipc;
   const char         *nick;

   numIPC = sizeof(IPCArray) / sizeof(IPCStruct);

   IpcPrintf(_("Enlightenment IPC Commands Help"));

   if (!params)
     {
	IpcPrintf(_("\ncommands currently available:\n"));
	IpcPrintf(_("use \"help all\" for descriptions of each command\n"
		    "use \"help <command>\" for an individual description\n\n"));

	lst = (IPCStruct **) Emalloc(numIPC * sizeof(IPCStruct *));

	for (i = 0; i < numIPC; i++)
	   lst[i] = &IPCArray[i];

	Quicksort((void **)lst, 0, numIPC - 1, ipccmp);

	for (i = 0; i < numIPC; i++)
	  {
	     ipc = lst[i];
	     nick = (ipc->nick) ? ipc->nick : "";
	     IpcPrintf("  %-18s %-3s  ", ipc->commandname, nick);
	     if ((i % 3) == 2)
		IpcPrintf("\n");
	  }
	if (i % 3)
	   IpcPrintf("\n");

	Efree(lst);
     }
   else
     {
	if (!strcmp(params, "all"))
	  {
	     IpcPrintf(_("\ncommands currently available:\n"));
	     IpcPrintf(_("use \"help <command>\" "
			 "for an individual description\n"));
	     IpcPrintf(_("      <command>   : <description>\n"));

	     for (i = 0; i < numIPC; i++)
	       {
		  ipc = &IPCArray[i];
		  nick = (ipc->nick) ? ipc->nick : "";
		  IpcPrintf("%18s %3s: %s\n",
			    ipc->commandname, nick, ipc->help_text);
	       }
	  }
	else
	  {
	     for (i = 0; i < numIPC; i++)
	       {
		  ipc = &IPCArray[i];
		  if (strcmp(params, ipc->commandname) &&
		      (ipc->nick == NULL || strcmp(params, ipc->nick)))
		     continue;

		  if (ipc->nick)
		     sprintf(buf, " (%s)", ipc->nick);
		  else
		     buf[0] = '\0';

		  IpcPrintf(" : %s%s\n--------------------------------\n%s\n",
			    ipc->commandname, buf, ipc->help_text);
		  if (ipc->extended_help_text)
		     IpcPrintf("%s\n", ipc->extended_help_text);
	       }
	  }
     }
}
