#include "E.h"
#include <errno.h>

void                DeskAccountTimeout(int val, void *data);

HandleStruct        HArray[] =
{
   {DefaultFunc},
   {DefaultFunc},
   {HKeyPress},
   {HKeyRelease},
   {HButtonPress},
   {HButtonRelease},
   {HMotionNotify},
   {HEnterNotify},
   {HLeaveNotify},
   {HFocusIn},
   {HFocusOut},
   {HKeymapNotify},
   {HExpose},
   {HGraphicsExpose},
   {HNoExpose},
   {HVisibilityNotify},
   {HCreateNotify},
   {HDestroyNotify},
   {HUnmapNotify},
   {HMapNotify},
   {HMapRequest},
   {HReparentNotify},
   {HConfigureNotify},
   {HConfigureRequest},
   {HGravityNotify},
   {HResizeRequest},
   {HCirculateNotify},
   {HCirculateRequest},
   {HPropertyNotify},
   {HSelectionClear},
   {HSelectionRequest},
   {HSelectionNotify},
   {HColormapNotify},
   {HClientMessage},
   {HMappingNotify},
   {DefaultFunc}
};

static char         diddeskaccount = 1;

void
DeskAccountTimeout(int val, void *data)
{
   EDBUG(5, "DeskAccountTimeout");

   DesktopAccounting();
   diddeskaccount = 1;
   EDBUG_RETURN_;
   data = NULL;
   val = 0;

   EDBUG_RETURN_;
}

char               *
NukeBoringevents(XEvent * ev, int num)
{
   char               *ok;
   int                 i, j;
   int                 first, last;

   if (!num)
      return NULL;

   ok = Emalloc(num * sizeof(char));

   for (i = 0; i < num; i++)
     {
/*      DebugEvent(&(ev[i])); */
	ok[i] = 1;
     }
   /* get rid of all but the last motion event */
   last = -1;
   for (i = 0; i < num; i++)
     {
	if (ev[i].type == MotionNotify)
	  {
	     ok[i] = 0;
	     last = i;
	  }
     }
   if (last >= 0)
      ok[last] = 1;
   /* look for paired enter / leave events for windows that contain no click */
   /* events for those windows whilst mouse is in them */
   for (i = 0; i < num; i++)
     {
	if (ev[i].type == EnterNotify)
	  {
	     Window              win;
	     char                is_ok;

	     first = -1;
	     last = -1;
	     win = ev[i].xany.window;
	     /* if its a normal window we created - not an event window */
	     if (FindXID(win))
	       {
		  first = i;
		  for (j = i + 1; j < num; j++)
		    {
		       if ((ev[j].xany.window == win) &&
			   ((ev[j].type == LeaveNotify) ||
			    (ev[j].type == UnmapNotify) ||
			    (ev[j].type == DestroyNotify)))
			 {
			    last = j;
			    break;
			 }
		    }
		  if ((first >= 0) && (last > first))
		    {
		       is_ok = 0;
		       for (j = first + 1; j <= last; j++)
			 {
			    if (ev[j].xany.window == win)
			      {
				 if ((ev[j].type == ButtonPress) ||
				     (ev[j].type == ButtonRelease))
				   {
				      is_ok = 1;
				      break;
				   }
			      }
			 }
		       if (!is_ok)
			 {
			    for (j = first; j <= last; j++)
			      {
				 if (ev[j].xany.window == win)
				   {
				      if ((ev[j].type == EnterNotify) ||
					  (ev[j].type == LeaveNotify) ||
					  (ev[j].type == MotionNotify))
					 ok[j] = 0;
				   }
			      }
			 }
		    }
	       }
	  }
     }
   /* compress all shapenotify events for a window and onyl take the last one */
   /* as beign valid */
   for (i = 0; i < num; i++)
     {
	if (ev[i].type == event_base_shape + ShapeNotify)
	  {
	     Window              win;

	     last = i;
	     win = ev[i].xany.window;
	     for (j = i; j < num; j++)
	       {
		  if ((ev[j].type == event_base_shape + ShapeNotify) &&
		      (ev[j].xany.window == win))
		    {
		       ok[j] = 0;
		       last = j;
		    }
	       }
	     ok[last] = 1;
	  }
     }
   /* FIXME: add maprequest compression */
   /* FIXME: add configurerequest compression */
   /* FIXME: add resizerequest compression */
   return ok;
}

void
DebugEvent(XEvent * ev)
{
   EDBUG(8, "DebugEvent");
   if (ev->type == event_base_shape + ShapeNotify)
      fprintf(stderr, "EV: ShapeNotify:\n");
   else
     {
	switch (ev->type)
	  {
	  case KeyPress:
	     fprintf(stderr, "EV: KeyPress:\n");
	     break;
	  case KeyRelease:
	     fprintf(stderr, "EV: KeyRelease:\n");
	     break;
	  case ButtonPress:
	     fprintf(stderr, "EV: ButtonPress:\n");
	     break;
	  case ButtonRelease:
	     fprintf(stderr, "EV: ButtonRelease:\n");
	     break;
	  case MotionNotify:
	     fprintf(stderr, "EV: MotionNotify:\n");
	     break;
	  case EnterNotify:
	     fprintf(stderr, "EV: EnterNotify:\n");
	     break;
	  case LeaveNotify:
	     fprintf(stderr, "EV: LeaveNotify:\n");
	     break;
	  case FocusIn:
	     fprintf(stderr, "EV: FocusIn:\n");
	     break;
	  case FocusOut:
	     fprintf(stderr, "EV: FocusOut:\n");
	     break;
	  case KeymapNotify:
	     fprintf(stderr, "EV: KeymapNotify:\n");
	     break;
	  case Expose:
	     fprintf(stderr, "EV: Expose:\n");
	     break;
	  case GraphicsExpose:
	     fprintf(stderr, "EV: GraphicsExpose:\n");
	     break;
	  case NoExpose:
	     fprintf(stderr, "EV: NoExpose:\n");
	     break;
	  case VisibilityNotify:
	     fprintf(stderr, "EV: VisibilityNotify:\n");
	     break;
	  case CreateNotify:
	     fprintf(stderr, "EV: CreateNotify:\n");
	     break;
	  case DestroyNotify:
	     fprintf(stderr, "EV: DestroyNotify:\n");
	     break;
	  case UnmapNotify:
	     fprintf(stderr, "EV: UnmapNotify:\n");
	     break;
	  case MapNotify:
	     fprintf(stderr, "EV: MapNotify:\n");
	     break;
	  case MapRequest:
	     fprintf(stderr, "EV: MapRequest:\n");
	     break;
	  case ReparentNotify:
	     fprintf(stderr, "EV: ReparentNotify:\n");
	     break;
	  case ConfigureNotify:
	     fprintf(stderr, "EV: ConfigureNotify:\n");
	     break;
	  case ConfigureRequest:
	     fprintf(stderr, "EV: ConfigureRequest:\n");
	     break;
	  case GravityNotify:
	     fprintf(stderr, "EV: GravityNotify:\n");
	     break;
	  case ResizeRequest:
	     fprintf(stderr, "EV: ResizeRequest:\n");
	     break;
	  case CirculateNotify:
	     fprintf(stderr, "EV: CirculateNotify:\n");
	     break;
	  case CirculateRequest:
	     fprintf(stderr, "EV: CirculateRequest:\n");
	     break;
	  case PropertyNotify:
	     fprintf(stderr, "EV: PropertyNotify:\n");
	     break;
	  case SelectionClear:
	     fprintf(stderr, "EV: SelectionClear:\n");
	     break;
	  case SelectionRequest:
	     fprintf(stderr, "EV: SelectionRequest:\n");
	     break;
	  case SelectionNotify:
	     fprintf(stderr, "EV: SelectionNotify:\n");
	     break;
	  case ColormapNotify:
	     fprintf(stderr, "EV: ColormapNotify:\n");
	     break;
	  case ClientMessage:
	     fprintf(stderr, "EV: ClientMessage:\n");
	     break;
	  case MappingNotify:
	     fprintf(stderr, "EV: MappingNotify:\n");
	     break;
	  default:
	     fprintf(stderr, "EV: ???\n");
	     break;
	  }
     }
   EDBUG_RETURN_;
}

void
HandleEvent(XEvent * ev)
{
   void              **lst;
   int                 i, num;

   EDBUG(7, "HandleEvent");
   WarpFocusHandleEvent(ev);
   if (ev->type == event_base_shape + ShapeNotify)
      HandleChildShapeChange(ev);
   if ((ev->type == KeyPress) || (ev->type == KeyRelease) ||
       (ev->type == ButtonPress) || (ev->type == ButtonRelease) ||
       (ev->type == EnterNotify) || (ev->type == LeaveNotify))
     {
	if (((ev->type == KeyPress) || (ev->type == KeyRelease)) &&
	    (ev->xkey.root != root.win))
	  {
	     XSetInputFocus(disp, ev->xkey.root, RevertToPointerRoot, CurrentTime);
	     XSync(disp, False);
	     ev->xkey.time = CurrentTime;
	     XSendEvent(disp, ev->xkey.root, False, 0, ev);
	  }
	else
	  {
	     lst = ListItemType(&num, LIST_TYPE_ACLASS_GLOBAL);
	     if (lst)
	       {
		  for (i = 0; i < num; i++)
		     EventAclass(ev, (ActionClass *) lst[i]);
		  Efree(lst);
	       }
	  }
     }
   if (ev->type <= 35)
      HArray[ev->type].func(ev);
   IconboxHandleEvent(ev);

   if (diddeskaccount)
     {
	DoIn("DESKTOP_ACCOUNTING_TIMEOUT", 30.0, DeskAccountTimeout, 0, NULL);
	diddeskaccount = 0;
     }
   EDBUG_RETURN_;
}

void
CheckEvent()
{
   XEvent              ev;

   EDBUG(7, "CheckEvent");
   while (XPending(disp))
     {
	XNextEvent(disp, &ev);
	HandleEvent(&ev);
     }
   EDBUG_RETURN_;
}

#ifdef DEBUG
#define DBUG_STACKSTART \
  int save = call_level + 1;
#define DBUG_STACKCHECK \
  if (save != call_level) { \
    fprintf (stderr, "Unstack error: ["); \
    for (save = 0; save < 4; ++ save) \
      fprintf (stderr, "%s%s", save ? ", " : "", call_stack[save]); \
    fprintf (stderr, "]\n"); \
    save = call_level; \
  }
#else
#define DBUG_STACKSTART
#define DBUG_STACKCHECK
#endif

  /* This is the primary event loop.  Everything that is going to happen in the
   * window manager has to start here at some point.  This is where all the
   * events from the X server are interpreted, timer events are inserted, etc
   */

void
WaitEvent()
{
/*  XEvent              ev; */
   fd_set              fdset;
   struct timeval      tval;
   static struct timeval tval_last =
   {0, 0};
   double              time1, time2;
   Qentry             *qe;
   int                 count, pcount;
   int                 fdsize;
   int                 xfd, smfd;
   int                 i;
   static int          evq_num = 0;
   static XEvent      *evq = NULL;
   char               *ok;

   DBUG_STACKSTART;

   EDBUG(7, "WaitEvent");
   smfd = GetSMfd();
   xfd = ConnectionNumber(disp);
   fdsize = MAX(xfd, smfd) + 1;

   /* if we've never set the time we were last here before */
   if ((tval_last.tv_sec == 0) && (tval_last.tv_usec == 0))
      gettimeofday(&tval_last, NULL);
   /* time1 = time we last entered this routine */
   time1 = ((double)tval_last.tv_sec) + (((double)tval_last.tv_usec) / 1000000);
   gettimeofday(&tval, NULL);
   tval_last.tv_sec = tval.tv_sec;
   tval_last.tv_usec = tval.tv_usec;
   /* time2 = current time */
   time2 = ((double)tval.tv_sec) + (((double)tval.tv_usec) / 1000000);
   time2 -= time1;
   if (time2 < 0.0)
      time2 = 0.0;
   /* time2 = time spent since we last were here */

   count = 0;
   while (XPending(disp))
     {
	count++;
	if (count > evq_num)
	  {
	     evq_num += 16;
	     if (!evq)
		evq = Emalloc(sizeof(XEvent) * evq_num);
	     else
		evq = Erealloc(evq, sizeof(XEvent) * evq_num);
	  }
	XNextEvent(disp, &(evq[count - 1]));
     }
   /* remove multiple extraneous events here */
   ok = NukeBoringevents(evq, count);
   if (ok)
     {
	for (i = 0; i < count; i++)
	  {
	     if (ok[i])
		HandleEvent(&(evq[i]));
	  }
	Efree(ok);
     }

   DBUG_STACKCHECK;

   HandleDrawQueue();
   XFlush(disp);
   pcount = count;

   DBUG_STACKCHECK;

   count = 0;
   while (XPending(disp))
     {
	count++;
	if (count > evq_num)
	  {
	     evq_num += 16;
	     if (!evq)
		evq = Emalloc(sizeof(XEvent) * evq_num);
	     else
		evq = Erealloc(evq, sizeof(XEvent) * evq_num);
	  }
	XNextEvent(disp, &(evq[count - 1]));
     }
   /* remove multiple extraneous events here */
   ok = NukeBoringevents(evq, count);
   if (ok)
     {
	for (i = 0; i < count; i++)
	  {
	     if (ok[i])
		HandleEvent(&(evq[i]));
	  }
	Efree(ok);
     }
   if (count > 0)
      XFlush(disp);

   if (pcount > count)
      count = pcount;
   if ((evq) && ((evq_num - count) > 64))
     {
	evq_num = 0;
	Efree(evq);
	evq = NULL;
     }

   DBUG_STACKCHECK;

   FD_ZERO(&fdset);
   FD_SET(xfd, &fdset);
   if (smfd >= 0)
      FD_SET(smfd, &fdset);

   qe = GetHeadTimerQueue();
   if (qe)
     {
	if (qe->just_added)
	  {
	     qe->just_added = 0;
	     time1 = qe->in_time;
	  }
	else
	  {
	     time1 = qe->in_time - time2;
	     if (time1 < 0.0)
		time1 = 0.0;
	     qe->in_time = time1;
	  }
	tval.tv_sec = (long)time1;
	tval.tv_usec = (long)((time1 - ((double)tval.tv_sec)) * 1000000);
	count = select(fdsize, &fdset, NULL, NULL, &tval);
     }
   else
      count = select(fdsize, &fdset, NULL, NULL, NULL);
   if (count < 0)
     {
	EDBUG_RETURN_;
     }
   if ((smfd >= 0) && (count > 0) && (FD_ISSET(smfd, &fdset)))
      ProcessICEMSGS();

   DBUG_STACKCHECK;

   if ((!(FD_ISSET(xfd, &fdset))) && (qe) && (count == 0) &&
       (((smfd >= 0) && (!(FD_ISSET(smfd, &fdset)))) || (smfd < 0)))
      HandleTimerEvent();

   DBUG_STACKCHECK;

   EDBUG_RETURN_;
}

void
HKeyPress(XEvent * ev)
{
   Dialog             *d;

   EDBUG(7, "HKeyPress");
   d = FindDialog(ev->xkey.window);
   if (d)
     {
	int                 i;

	for (i = 0; i < d->num_bindings; i++)
	  {
	     if (ev->xkey.keycode == d->keybindings[i].key)
		(d->keybindings[i].func) (d->keybindings[i].val,
					  d->keybindings[i].data);
	  }
     }
   EDBUG_RETURN_;
}

void
HKeyRelease(XEvent * ev)
{
   EDBUG(7, "HKeyRelease");
   ev = NULL;
   EDBUG_RETURN_;
}

void
HButtonPress(XEvent * ev)
{
   EDBUG(7, "HButtonPress");
   ApplySclass(FindItem("SOUND_BUTTON_CLICK", 0,
			LIST_FINDBY_NAME, LIST_TYPE_SCLASS));
   HandleMouseDown(ev);
   EDisplayMemUse();
   EDBUG_RETURN_;
}

void
HButtonRelease(XEvent * ev)
{
   EDBUG(7, "HButtonRelease");
   ApplySclass(FindItem("SOUND_BUTTON_RAISE", 0,
			LIST_FINDBY_NAME, LIST_TYPE_SCLASS));
   HandleMouseUp(ev);
   EDBUG_RETURN_;
}

void
HMotionNotify(XEvent * ev)
{
   EDBUG(7, "HMotionNotify");
   HandleMotion(ev);
   EDBUG_RETURN_;
}

void
HEnterNotify(XEvent * ev)
{
   EDBUG(7, "HEnterNotify");
   if (mode.mode == MODE_NONE)
     {
	/*
	 * multi screen handling -- root windows receive
	 * enter / leave notify
	 */
	if (ev->xany.window == root.win)
	  {
	     PagerHideAllHi();
	     if (!mode.focuswin || FOCUS_POINTER == mode.focusmode)
		HandleFocusWindow(root.focuswin);
	  }
	else
	  {
	     HandleMouseIn(ev);
	     HandleFocusWindow(ev->xcrossing.window);
	  }
     }
   EDBUG_RETURN_;
}

void
HLeaveNotify(XEvent * ev)
{
   EDBUG(7, "HLeaveNotify");
   if (mode.mode == MODE_NONE)
     {
	HandleMouseOut(ev);

	/*
	 * If we are leaving the root window, we are switching
	 * screens on a multi screen system - need to unfocus
	 * to allow other desk to grab focus...
	 */
	if (ev->xcrossing.window == root.win)
	  {
	     if (ev->xcrossing.mode == NotifyNormal &&
		 ev->xcrossing.detail != NotifyInferior &&
		 mode.focuswin)
		HandleFocusWindow(root.focuswin);
	     else
		HandleFocusWindow(ev->xcrossing.window);
	  }
/* THIS caused the "emacs focus bug" ? */
/*      else */
/*      HandleFocusWindow(ev->xcrossing.window); */
     }
   EDBUG_RETURN_;
}

void
HFocusIn(XEvent * ev)
{
   EDBUG(7, "HFocusIn");
   if (ev->xfocus.detail != NotifyPointer)
      HandleFocusWindowIn(ev->xfocus.window);
   EDBUG_RETURN_;
}

void
HFocusOut(XEvent * ev)
{
   EDBUG(7, "HFocusOut");
   if (ev->xfocus.detail == NotifyNonlinear)
     {
	Window              rt, ch;
	int                 d;
	unsigned int        ud;

	XQueryPointer(disp, root.win, &rt, &ch, &d, &d, &d, &d, &ud);
	if (rt != root.win)
	   HandleFocusWindowIn(0);
     }
   EDBUG_RETURN_;
}

void
HKeymapNotify(XEvent * ev)
{
   EDBUG(7, "HKeymapNotify");
   ev = NULL;
   EDBUG_RETURN_;
}

void
HExpose(XEvent * ev)
{
   EDBUG(7, "HExpose");
   HandleExpose(ev);
   EDBUG_RETURN_;
}

void
HGraphicsExpose(XEvent * ev)
{
   EDBUG(7, "HGraphicsExpose");
   ev = NULL;
   EDBUG_RETURN_;
}

void
HNoExpose(XEvent * ev)
{
   EDBUG(7, "HNoExpose");
   ev = NULL;
   EDBUG_RETURN_;
}

void
HVisibilityNotify(XEvent * ev)
{
   EDBUG(7, "HVisibilityNotify");
   ev = NULL;
   EDBUG_RETURN_;
}

void
HCreateNotify(XEvent * ev)
{
   EDBUG(7, "HCreateNotify");
   ev = NULL;
   EDBUG_RETURN_;
}

void
HDestroyNotify(XEvent * ev)
{
   EDBUG(7, "HDestroyNotify");
   HandleDestroy(ev);
   EDBUG_RETURN_;
}

void
HUnmapNotify(XEvent * ev)
{
   EDBUG(7, "HUnmapNotify");
   HandleUnmap(ev);
   EDBUG_RETURN_;
}

void
HMapNotify(XEvent * ev)
{
   EDBUG(7, "HMapNotify");
   ev = NULL;
   EDBUG_RETURN_;
}

void
HMapRequest(XEvent * ev)
{
   EDBUG(7, "HMapRequest");
   HandleMapRequest(ev);
   EDBUG_RETURN_;
}

void
HReparentNotify(XEvent * ev)
{
   EDBUG(7, "HReparentNotify");
   HandleReparent(ev);
   EDBUG_RETURN_;
}

void
HConfigureNotify(XEvent * ev)
{
   EDBUG(7, "HConfigureNotify");
   ev = NULL;
   EDBUG_RETURN_;
}

void
HConfigureRequest(XEvent * ev)
{
   EDBUG(7, "HConfigureRequest");
   HandleConfigureRequest(ev);
   EDBUG_RETURN_;
}

void
HGravityNotify(XEvent * ev)
{
   EDBUG(7, "HGravityNotify");
   ev = NULL;
   EDBUG_RETURN_;
}

void
HResizeRequest(XEvent * ev)
{
   EDBUG(7, "HResizeRequest");
   HandleResizeRequest(ev);
   EDBUG_RETURN_;
}

void
HCirculateNotify(XEvent * ev)
{
   EDBUG(7, "HCirculateNotify");
   ev = NULL;
   EDBUG_RETURN_;
}

void
HCirculateRequest(XEvent * ev)
{
   EDBUG(7, "HCirculateRequest");
   HandleCirculate(ev);
   EDBUG_RETURN_;
}

void
HPropertyNotify(XEvent * ev)
{
   EDBUG(7, "HPropertyNotify");
   HandleProperty(ev);
   EDBUG_RETURN_;
}

void
HSelectionClear(XEvent * ev)
{
   EDBUG(7, "HSelectionClear");
   ev = NULL;
   EDBUG_RETURN_;
}

void
HSelectionRequest(XEvent * ev)
{
   EDBUG(7, "HSelectionRequest");
   ev = NULL;
   EDBUG_RETURN_;
}

void
HSelectionNotify(XEvent * ev)
{
   EDBUG(7, "HSelectionNotify");
   ev = NULL;
   EDBUG_RETURN_;
}

void
HColormapNotify(XEvent * ev)
{
   EDBUG(7, "HColormapNotify");
   ev = NULL;
   EDBUG_RETURN_;
}

void
HClientMessage(XEvent * ev)
{
   EDBUG(7, "HClientMessage");
   HandleClientMessage(ev);
   EDBUG_RETURN_;
}

void
HMappingNotify(XEvent * ev)
{
   EDBUG(7, "HMappingNotify");
   ev = NULL;
   EDBUG_RETURN_;
}

void
DefaultFunc(XEvent * ev)
{
   EDBUG(7, "DefaultFunc");
   ev = NULL;
   EDBUG_RETURN_;
}
