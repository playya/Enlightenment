#include "E.h"
#include "timestamp.h"

ActionClass        *
CreateAclass(char *name)
{
   ActionClass        *a;

   EDBUG(5, "CreateAclass");
   a = Emalloc(sizeof(ActionClass));
   a->name = duplicate(name);
   a->num = 0;
   a->list = NULL;
   a->tooltipstring = NULL;
   a->ref_count = 0;
   EDBUG_RETURN(a);
}

void
GrabButtonGrabs(EWin * ewin)
{
   ActionClass        *ac;
   int                 j;
   Action             *a;

   ac = (ActionClass *) FindItem("BUTTONBINDINGS", 0,
				 LIST_FINDBY_NAME,
				 LIST_TYPE_ACLASS);

   if (ac)
     {
	ac->ref_count++;
	for (j = 0; j < ac->num; j++)
	  {
	     a = ac->list[j];
	     if ((a) &&
		 ((a->event == EVENT_MOUSE_DOWN) ||
		  (a->event == EVENT_MOUSE_UP)))
	       {
		  unsigned int        mod, button, mask;
		  int                 i;

		  mod = 0;
		  button = 0;
		  if (a->anymodifier)
		    {
		       mod = AnyModifier;
		    }
		  else
		    {
		       mod = a->modifiers;
		    }
		  if (a->anybutton)
		    {
		       button = AnyButton;
		    }
		  else
		    {
		       button = a->button;
		    }
		  mask = ButtonPressMask | ButtonReleaseMask;
		  if (mod == AnyModifier)
		    {
		       if ((ewin->pager) && (ewin->pager->hi_win))
			  XGrabButton(disp, button, mod, ewin->pager->hi_win,
				      False, mask, GrabModeSync, GrabModeAsync,
				      None, None);
		       XGrabButton(disp, button, mod, ewin->win, False, mask,
				   GrabModeSync, GrabModeAsync, None, None);
		    }
		  else
		    {
		       for (i = 0; i < 8; i++)
			 {
			    if ((ewin->pager) && (ewin->pager->hi_win))
			       XGrabButton(disp, button,
					   mod | mask_mod_combos[i],
					   ewin->pager->hi_win, False, mask,
					   GrabModeSync, GrabModeAsync,
					   None, None);
			    XGrabButton(disp, button, mod | mask_mod_combos[i],
					ewin->win, False, mask,
				     GrabModeSync, GrabModeAsync, None, None);
			 }
		    }
	       }
	  }
     }
}

void
UnGrabButtonGrabs(EWin * ewin)
{
   ActionClass        *ac;
   int                 j;
   Action             *a;

   ac = (ActionClass *) FindItem("BUTTONBINDINGS", 0,
				 LIST_FINDBY_NAME,
				 LIST_TYPE_ACLASS);

   if (ac)
     {
	ac->ref_count--;
	for (j = 0; j < ac->num; j++)
	  {
	     a = ac->list[j];
	     if ((a) &&
		 ((a->event == EVENT_MOUSE_DOWN) ||
		  (a->event == EVENT_MOUSE_UP)))
	       {
		  unsigned int        mod, button;

		  mod = 0;
		  button = 0;
		  if (a->anymodifier)
		    {
		       mod = AnyModifier;
		    }
		  else
		    {
		       mod = a->modifiers;
		    }
		  if (a->anybutton)
		    {
		       button = AnyButton;
		    }
		  else
		    {
		       button = a->button;
		    }
		  if (mod == AnyModifier)
		    {
		       if ((ewin->pager) && (ewin->pager->hi_win))
			  XUngrabButton(disp, button, mod, ewin->pager->hi_win);
		       XUngrabButton(disp, button, mod, ewin->win);
		    }
		  else
		    {
		       int                 i;

		       for (i = 0; i < 8; i++)
			 {
			    if ((ewin->pager) && (ewin->pager->hi_win))
			       XUngrabButton(disp, button,
					     mod | mask_mod_combos[i],
					     ewin->pager->hi_win);
			    XUngrabButton(disp, button,
					  mod | mask_mod_combos[i],
					  ewin->win);
			 }
		    }
	       }
	  }
     }
}

Action             *
CreateAction(char event, char anymod, int mod, int anybut, int but,
	     char anykey, char *key, char *tooltipstring)
{
   Action             *act;

   EDBUG(5, "CreateAction");

   act = Emalloc(sizeof(Action));
   act->action = NULL;
   act->event = event;
   act->anymodifier = anymod;
   act->modifiers = mod;
   act->anybutton = anybut;
   act->button = but;
   act->anykey = anykey;
   if (!key)
     {
	act->key = 0;
     }
   else
     {
	act->key = XKeysymToKeycode(disp, XStringToKeysym(key));
     }
   act->key_str = duplicate(key);
   if (tooltipstring)
     {
	act->tooltipstring = duplicate(tooltipstring);
     }
   else
     {
	act->tooltipstring = NULL;
     }

   EDBUG_RETURN(act);
}

void
RemoveActionType(ActionType * ActionTypeToRemove)
{
   ActionType         *ptr, *pp;

   EDBUG(5, "RemoveActionType");

   ptr = ActionTypeToRemove;
   while (ptr)
     {
	if (ptr->params)
	   Efree(ptr->params);
	pp = ptr;
	ptr = ptr->Next;
	Efree(pp);
     }

   EDBUG_RETURN_;
}

void
RemoveAction(Action * ActionToRemove)
{
   EDBUG(5, "RemoveAction");

   if (!ActionToRemove)
      EDBUG_RETURN_;

   if ((ActionToRemove->event == EVENT_KEY_DOWN) ||
       (ActionToRemove->event == EVENT_KEY_UP))
      UnGrabActionKey(ActionToRemove);
   if (ActionToRemove->action)
      RemoveActionType(ActionToRemove->action);
   if (ActionToRemove->tooltipstring)
      Efree(ActionToRemove->tooltipstring);
   if (ActionToRemove->key_str)
      Efree(ActionToRemove->key_str);
   Efree(ActionToRemove);

   EDBUG_RETURN_;

}

void
RemoveActionClass(ActionClass * ActionClassToRemove)
{
   int                 i;

   EDBUG(5, "RemoveActionClass");

   if (!ActionClassToRemove)
      EDBUG_RETURN_;

   if (ActionClassToRemove->ref_count > 0)
     {
	char                stuff[255];

	Esnprintf(stuff, sizeof(stuff), "reference count is still: %u\n",
		  ActionClassToRemove->ref_count);
	DIALOG_OK("ActionClass Error!", stuff);
	EDBUG_RETURN_;
     }
   while (RemoveItemByPtr(ActionClassToRemove, LIST_TYPE_ACLASS));

   for (i = 0; i < ActionClassToRemove->num; i++)
      RemoveAction(ActionClassToRemove->list[i]);
   if (ActionClassToRemove->list)
      Efree(ActionClassToRemove->list);
   if (ActionClassToRemove->name)
      Efree(ActionClassToRemove->name);
   if (ActionClassToRemove->tooltipstring)
      Efree(ActionClassToRemove->tooltipstring);
   Efree(ActionClassToRemove);
   mode.adestroy = 1;

   EDBUG_RETURN_;
}

void
AddToAction(Action * act, int id, void *params)
{
   ActionType         *pptr, *ptr, *at;

   EDBUG(5, "AddToAction");
   pptr = NULL;
   at = Emalloc(sizeof(ActionType));
   at->Next = NULL;
   at->Type = id;
   at->params = params;
   if (!act->action)
     {
	act->action = at;
     }
   else
     {
	ptr = act->action;
	while (ptr)
	  {
	     pptr = ptr;
	     ptr = ptr->Next;
	  }
	pptr->Next = at;
     }
   EDBUG_RETURN_;
}

void
AddAction(ActionClass * a, Action * act)
{
   EDBUG(5, "AddAction");
   a->num++;
   if (!a->list)
      a->list = Emalloc(sizeof(Action *));
   else
      a->list = Erealloc(a->list, a->num * sizeof(Action *));
   a->list[a->num - 1] = act;
   EDBUG_RETURN_;
}

int
EventAclass(XEvent * ev, ActionClass * a)
{
   KeyCode             key;
   int                 i, type, button, modifiers, ok, mouse, mask, val = 0;
   Action             *act;
   char                reset_ewin;

   EDBUG(5, "EventAclass");
   reset_ewin = key = type = button = modifiers = mouse = 0;
   if (!mode.ewin)
     {
	mode.ewin = mode.focuswin;
	if (!mode.ewin)
	   mode.ewin = mode.mouse_over_win;
	reset_ewin = 1;
     }
   {
      EWin               *ewin;

      ewin = GetEwin();
      if ((mode.movemode == 0) && (ewin) && (mode.mode == MODE_MOVE))
	 DetermineEwinFloat(ewin, 0, 0);
   }

   mask = (ShiftMask | ControlMask | Mod1Mask | Mod2Mask
	   | Mod3Mask | Mod4Mask |
	   Mod5Mask) & (~(numlock_mask | scrollock_mask | LockMask));

   switch (ev->type)
     {
     case KeyPress:
	type = EVENT_KEY_DOWN;
	key = ev->xkey.keycode;
	modifiers = ev->xbutton.state & mask;
	mouse = 0;
	break;
     case KeyRelease:
	type = EVENT_KEY_UP;
	key = ev->xkey.keycode;
	modifiers = ev->xbutton.state & mask;
	mouse = 0;
	break;
     case ButtonPress:
	if (ev->xbutton.time == 0)
	   type = EVENT_DOUBLE_DOWN;
	else
	   type = EVENT_MOUSE_DOWN;
	button = ev->xbutton.button;
	modifiers = ev->xbutton.state & mask;
	mouse = 1;
	break;
     case ButtonRelease:
	type = EVENT_MOUSE_UP;
	button = ev->xbutton.button;
	modifiers = ev->xbutton.state & mask;
	mouse = 1;
	break;
     case EnterNotify:
	type = EVENT_MOUSE_ENTER;
	button = -1;
	modifiers = ev->xcrossing.state & mask;
	mouse = 1;
	break;
     case LeaveNotify:
	type = EVENT_MOUSE_LEAVE;
	button = -1;
	modifiers = ev->xcrossing.state & mask;
	mouse = 1;
	break;
     default:
	break;
     }

   mode.adestroy = 0;

   for (i = 0; i < a->num; i++)
     {
	if (!mode.adestroy)
	  {
	     act = a->list[i];
	     ok = 0;
	     if ((act->event == type) && (act->action))
	       {
		  if (mouse)
		    {
		       if (button < 0)
			 {
			    if (act->anymodifier)
			       ok = 1;
			    else if (act->modifiers == modifiers)
			       ok = 1;
			 }
		       else
			 {
			    if (act->anymodifier)
			      {
				 if (act->anybutton)
				    ok = 1;
				 else if (act->button == button)
				    ok = 1;
			      }
			    else if (act->modifiers == modifiers)
			      {
				 if (act->anybutton)
				    ok = 1;
				 else if (act->button == button)
				    ok = 1;
			      }
			 }
		    }
		  else
		    {
		       if (act->anymodifier)
			 {
			    if (act->anykey)
			       ok = 1;
			    else if (act->key == key)
			       ok = 1;
			 }
		       else if (act->modifiers == modifiers)
			 {
			    if (act->anykey)
			       ok = 1;
			    else if (act->key == key)
			       ok = 1;
			 }
		    }
		  if (ok)
		    {
		       handleAction(act->action);
		       val = 1;
		    }
	       }
	  }
	if (mode.adestroy)
	   break;
     }
   if (reset_ewin)
      mode.ewin = NULL;
   mode.adestroy = 0;
   EDBUG_RETURN(val);
}

int
handleAction(ActionType * Action)
{

   /* This function will handle any type of action that is passed into
    * it.  ALL internal events should be passed through this function.
    * No exceptions.  --Mandrake (02/26/98)
    */

   int                 error;

   EDBUG(5, "handleAction");
   error = (*(ActionFunctions[Action->Type])) (Action->params);

   /* Did we just hose ourselves?
    * if so, we'd best not stick around here 
    */

   if (mode.adestroy)
      EDBUG_RETURN(0);
   /* If there is another action in this series, (now that
    * we're sure we didn't already die) perform it
    */
   if (!error)
      if (Action->Next)
	 error = handleAction(Action->Next);
   EDBUG_RETURN(error);
}

int
spawnMenu(void *params)
{
   Menu               *m = NULL;
   char                s[1024];
   char                s2[1024];
   int                 x, y, di;
   Window              dw;
   unsigned int        w, h, d;
   EWin               *ewin;
   char                desk_click = 0;
   int                 i;

   EDBUG(6, "spawnMenu");
   if (!params)
      EDBUG_RETURN(0);
   sscanf((char *)params, "%1000s %1000s", s, s2);
   ewin = mode.ewin = GetFocusEwin();
   for (i = 0; i < mode.numdesktops; i++)
     {
	if (mode.context_win == desks.desk[i].win)
	  {
	     desk_click = 1;
	     break;
	  }
     }
   if (!desk_click)
     {
	if ((ewin) && (ewin->win != mode.context_win) && (mode.context_win))
	  {
	     EGetGeometry(disp, mode.context_win, &dw, &di, &di, &w, &h, &d, &d);
	     XTranslateCoordinates(disp, mode.context_win, root.win,
				   0, 0, &x, &y, &dw);

	     if (w >= h)
		mode.y = -(y + h);
	     else
		mode.x = -(x + w);
	     mode.context_w = w;
	     mode.context_h = h;
	  }
     }
   if (!strcmp(s, "deskmenu"))
     {
	AUDIO_PLAY("SOUND_MENU_SHOW");
	ShowDeskMenu();
     }
   else if (!strcmp(s, "taskmenu"))
     {
	AUDIO_PLAY("SOUND_MENU_SHOW");
	ShowAllTaskMenu();
     }
   else if (!strcmp(s, "named"))
     {
	m = FindItem(s2, 0, LIST_FINDBY_NAME, LIST_TYPE_MENU);
	if (m)
	  {
	     AUDIO_PLAY("SOUND_MENU_SHOW");
	     mode.cur_menu_mode = 1;
	     XUngrabPointer(disp, CurrentTime);
	     if (!FindEwinByMenu(m))
		ShowMenu(m, 0);
	     mode.cur_menu[0] = m;
	     mode.cur_menu_depth = 1;
	     ShowMenuMasker(m);
	     m->ref_count++;
	  }
	else
	  {
	     mode.cur_menu[0] = NULL;
	     mode.cur_menu_depth = 0;
	     HideMenuMasker();
	  }
     }
   if (((ewin) && (ewin->win == mode.context_win)) ||
       (ewin = FindEwinByChildren(mode.context_win)))
     {
	if ((ewin) && (mode.cur_menu_depth > 0) &&
	    (mode.cur_menu[0]))
	   ewin->shownmenu = mode.cur_menu[0]->win;
     }
   params = NULL;
   if (mode.cur_menu_depth == 0)
      EDBUG_RETURN(0);
   EDBUG_RETURN(1);
}

int
hideMenu(void *params)
{
   EDBUG(6, "hideMenu");
   params = NULL;
   EDBUG_RETURN(0);
}

int
doNothing(void *params)
{
   EDBUG(6, "doNothing");
   params = NULL;
   EDBUG_RETURN(0);
}

int
execApplication(void *params)
{
   char               *sh;
   char               *path;
   char                exe[FILEPATH_LEN_MAX];
   char               *real_exec;

   EDBUG(6, "execApplication");
   if (!params)
      EDBUG_RETURN(0);
   if (fork())
      EDBUG_RETURN(0);
   setsid();
   sh = usershell(getuid());
   exe[0] = 0;
   sscanf((char *)params, "%4000s", exe);
   if (exe[0])
     {
	path = pathtoexec(exe);
	if (path)
	  {
	     Efree(path);
	     real_exec = (char *)Emalloc(strlen((char *)params) + 6);
	     sprintf(real_exec, "exec %s", (char *)params);
	     execl(sh, sh, "-c", (char *)real_exec, NULL);
	     exit(0);
	  }
	path = pathtofile(exe);
	if (!path)
	  {
	     /* absolute path */
	     if (((char *)exe)[0] == '/')
		DialogAlertOK("There was an error running the program:\n"
			      "%s\n"
			      "This program could not be executed.\n"
			      "This is because the file does not exist.\n",
			      (char *)exe);
	     /* relative path */
	     else
		DialogAlertOK("There was an error running the program:\n"
			      "%s\n"
			      "This program could not be executed.\n"
			      "This is most probably because this program "
			      "is not in the\n"
			      "path for your shell which is %s. I suggest "
			      "you read "
			      "the manual\n"
			      "page for that shell and read up how to "
			      "change or add "
			      "to your\n"
			      "execution path.\n",
			      (char *)exe, sh);
	  }
	else
	   /* it is a node on the filing sys */
	  {
	     /* it's a file */
	     if (isfile((char *)path))
	       {
		  /* can execute it */
		  if (canexec((char *)path))
		     DialogAlertOK("There was an error running the program:\n"
				   "%s\n"
				   "This program could not be executed.\n"
				   "I am unsure as to why you could not "
				   "do this. "
				   "The file exists,\n"
				   "is a file, and you are allowed to "
				   "execute it. I "
				   "suggest you look\n"
				   "into this.\n",
				   (char *)path);
		  /* not executable file */
		  else
		     DialogAlertOK("There was an error running the program:\n"
				   "%s\n"
				   "This program could not be executed.\n"
				   "This is because the file exists, is a"
				   " file, but "
				   "you are unable\n"
				   "to execute it because you do not "
				   "have execute "
				   "access to this file.\n",
				   (char *)path);
	       }
	     /* it's not a file */
	     else
	       {
		  /* its a dir */
		  if (isdir((char *)path))
		     DialogAlertOK("There was an error running the program:\n"
				   "%s\n"
				   "This program could not be executed.\n"
				   "This is because the file is infact "
				   "a directory.\n",
				   (char *)path);
		  /* its not a file or a dir */
		  else
		     DialogAlertOK("There was an error running the program:\n"
				   "%s\n"
				   "This program could not be executed.\n"
				   "This is because the file is not a "
				   "regular file.\n",
				   (char *)path);
	       }
	     if (path)
		Efree(path);
	  }
	exit(100);
     }
   real_exec = (char *)Emalloc(strlen((char *)params) + 6);
   sprintf(real_exec, "exec %s", (char *)params);
   execl(sh, sh, "-c", (char *)real_exec, NULL);
   exit(0);
   EDBUG_RETURN(0);
}

int
alert(void *params)
{
   char               *pp;
   int                 i;

   EDBUG(6, "alert");
   if (InZoom())
      Zoom(NULL);
   if ((mode.mode == MODE_MOVE) || (mode.mode == MODE_RESIZE_H) ||
       (mode.mode == MODE_RESIZE_V) || (mode.mode == MODE_RESIZE))
      EDBUG_RETURN(0);
   pp = duplicate((char *)params);
   i = 1;
   if (!pp)
      EDBUG_RETURN(1);
   if (strlen(pp) <= 0)
      EDBUG_RETURN(1);
   while (pp[i])
     {
	if ((pp[i - 1] == '\\') && (((char *)params)[i] == 'n'))
	  {
	     pp[i - 1] = ' ';
	     pp[i] = '\n';
	  }
	i++;
     }
   DialogAlertOK(pp);
   Efree(pp);
   EDBUG_RETURN(0);
}

int
doExit(void *params)
{
   EDBUG(6, "doExit");
   if ((mode.mode == MODE_MOVE) || (mode.mode == MODE_RESIZE_H) ||
       (mode.mode == MODE_RESIZE_V) || (mode.mode == MODE_RESIZE))
      EDBUG_RETURN(0);
   /* This function is now handled in session.c */
   if (InZoom())
      Zoom(NULL);
   doSMExit(params);
   EDBUG_RETURN(0);
}

int
doResize(void *params)
{
   EWin               *ewin;
   int                 x, y, w, h;

   EDBUG(6, "doResize");
   if (InZoom())
      EDBUG_RETURN(0);
   if ((mode.mode == MODE_MOVE) || (mode.mode == MODE_RESIZE_H) ||
       (mode.mode == MODE_RESIZE_V) || (mode.mode == MODE_RESIZE))
      EDBUG_RETURN(0);
   if (mode.slideout)
      HideSlideout(mode.slideout, mode.context_win);
   ewin = mode.ewin = GetFocusEwin();
   if (!ewin)
      EDBUG_RETURN(0);
   if (ewin->shaded)
      EDBUG_RETURN(0);
   if (mode.resizemode > 0)
     {
	FX_Pause();
	GrabX();
     }
   queue_up = 0;
   AUDIO_PLAY("SOUND_RESIZE_START");
   mode.mode = MODE_RESIZE;
   x = mode.x - ewin->x;
   y = mode.y - ewin->y;
   w = ewin->w >> 1;
   h = ewin->h >> 1;
   if ((x < w) && (y < h))
     {
	mode.resize_detail = 0;
     }
   if ((x >= w) && (y < h))
     {
	mode.resize_detail = 1;
     }
   if ((x < w) && (y >= h))
     {
	mode.resize_detail = 2;
     }
   if ((x >= w) && (y >= h))
     {
	mode.resize_detail = 3;
     }
   mode.start_x = mode.x;
   mode.start_y = mode.y;
   mode.win_x = ewin->x;
   mode.win_y = ewin->y;
   mode.win_w = ewin->client.w;
   mode.win_h = ewin->client.h;
   mode.firstlast = 0;
   DrawEwinShape(ewin, mode.resizemode, ewin->x, ewin->y, ewin->client.w,
		 ewin->client.h, mode.firstlast);
   mode.firstlast = 1;
   params = NULL;
   EDBUG_RETURN(0);
}

int
doResizeH(void *params)
{
   EWin               *ewin;
   int                 x, w;

   EDBUG(6, "doResizeH");
   if (InZoom())
      EDBUG_RETURN(0);
   if ((mode.mode == MODE_MOVE) || (mode.mode == MODE_RESIZE_H) ||
       (mode.mode == MODE_RESIZE_V) || (mode.mode == MODE_RESIZE))
      EDBUG_RETURN(0);
   if (mode.slideout)
      HideSlideout(mode.slideout, mode.context_win);
   ewin = mode.ewin = GetFocusEwin();
   if (!ewin)
      EDBUG_RETURN(0);
   if (mode.resizemode > 0)
     {
	FX_Pause();
	GrabX();
     }
   queue_up = 0;
   AUDIO_PLAY("SOUND_RESIZE_START");
   mode.mode = MODE_RESIZE_H;
   x = mode.x - ewin->x;
   w = ewin->w >> 1;
   if (x < w)
     {
	mode.resize_detail = 0;
     }
   else
     {
	mode.resize_detail = 1;
     }
   mode.start_x = mode.x;
   mode.start_y = mode.y;
   mode.win_x = ewin->x;
   mode.win_y = ewin->y;
   mode.win_w = ewin->client.w;
   mode.win_h = ewin->client.h;
   mode.firstlast = 0;
   DrawEwinShape(ewin, mode.resizemode, ewin->x, ewin->y, ewin->client.w,
		 ewin->client.h, mode.firstlast);
   mode.firstlast = 1;
   params = NULL;
   EDBUG_RETURN(0);
}

int
doResizeV(void *params)
{
   EWin               *ewin;
   int                 y, h;

   EDBUG(6, "doResizeV");
   if (InZoom())
      EDBUG_RETURN(0);
   if ((mode.mode == MODE_MOVE) || (mode.mode == MODE_RESIZE_H) ||
       (mode.mode == MODE_RESIZE_V) || (mode.mode == MODE_RESIZE))
      EDBUG_RETURN(0);
   if (mode.slideout)
      HideSlideout(mode.slideout, mode.context_win);
   ewin = mode.ewin = GetFocusEwin();
   if (!ewin)
      EDBUG_RETURN(0);
   if (mode.resizemode > 0)
     {
	FX_Pause();
	GrabX();
     }
   queue_up = 0;
   AUDIO_PLAY("SOUND_RESIZE_START");
   mode.mode = MODE_RESIZE_V;
   y = mode.y - ewin->y;
   h = ewin->h >> 1;
   if (y < h)
     {
	mode.resize_detail = 0;
     }
   else
     {
	mode.resize_detail = 1;
     }
   mode.start_x = mode.x;
   mode.start_y = mode.y;
   mode.win_x = ewin->x;
   mode.win_y = ewin->y;
   mode.win_w = ewin->client.w;
   mode.win_h = ewin->client.h;
   mode.firstlast = 0;
   DrawEwinShape(ewin, mode.resizemode, ewin->x, ewin->y, ewin->client.w,
		 ewin->client.h, mode.firstlast);
   mode.firstlast = 1;
   params = NULL;
   EDBUG_RETURN(0);
}

int
doResizeEnd(void *params)
{
   EWin               *ewin;
   int                 i;

   EDBUG(0, "doResizeEnd");
   ewin = GetFocusEwin();
   AUDIO_PLAY("SOUND_RESIZE_STOP");
   if (!ewin)
     {
	if (mode.resizemode > 0)
	   UngrabX();
	ForceUpdatePagersForDesktop(desks.current);
	EDBUG_RETURN(0);
     }
   queue_up = 1;
   mode.mode = MODE_NONE;
   if (mode.noewin)
      mode.ewin = NULL;
   mode.noewin = 0;
   mode.firstlast = 2;
   DrawEwinShape(ewin, mode.resizemode, ewin->x, ewin->y, ewin->client.w,
		 ewin->client.h, mode.firstlast);
   for (i = 0; i < ewin->border->num_winparts; i++)
      ewin->bits[i].no_expose = 1;
   ICCCM_Configure(ewin);
   HideCoords();
   XSync(disp, False);
   if (mode.resizemode > 0)
     {
	FX_Pause();
	UngrabX();
     }
   mode.firstlast = 0;
   params = NULL;
   ForceUpdatePagersForDesktop(desks.current);
   RememberImportantInfoForEwin(ewin);
   EDBUG_RETURN(0);
}

static int          start_move_desk = 0;
static int          start_move_x = 0;
static int          start_move_y = 0;

static int
doMoveImpl(void *params, char constrained)
{
   EWin              **gwins;
   EWin               *ewin;
   int                 xo, yo, i, num;

   EDBUG(6, "doMove");
   if (InZoom())
      EDBUG_RETURN(0);
   if ((mode.mode == MODE_MOVE) || (mode.mode == MODE_RESIZE_H) ||
       (mode.mode == MODE_RESIZE_V) || (mode.mode == MODE_RESIZE))
      EDBUG_RETURN(0);
   if (mode.slideout)
      HideSlideout(mode.slideout, mode.context_win);
   ewin = mode.ewin = GetFocusEwin();
   if (!ewin)
      EDBUG_RETURN(0);
   mode.moveresize_pending_ewin = ewin;
   if (mode.movemode > 0)
     {
	FX_Pause();
	GrabX();
     }
/*  GrabThePointer(root.win); */
   AUDIO_PLAY("SOUND_MOVE_START");
   mode.mode = MODE_MOVE;
   mode.constrained = constrained;
   mode.start_x = mode.x;
   mode.start_y = mode.y;
   mode.win_x = ewin->x;
   mode.win_y = ewin->y;
   mode.win_w = ewin->client.w;
   mode.win_h = ewin->client.h;
   mode.firstlast = 0;
   start_move_desk = ewin->desktop;
   xo = desks.desk[ewin->desktop].x;
   yo = desks.desk[ewin->desktop].y;

   gwins = ListWinGroupMembersForEwin(ewin, ACTION_MOVE, &num);
   for (i = 0; i < num; i++)
     {
	FloatEwinAt(gwins[i], gwins[i]->x, gwins[i]->y);
	if (!mode.moveresize_pending_ewin)
	   DrawEwinShape(gwins[i], mode.movemode, gwins[i]->x, gwins[i]->y,
		      gwins[i]->client.w, gwins[i]->client.h, mode.firstlast);
     }
   Efree(gwins);
   mode.firstlast = 1;
   params = NULL;
   start_move_x = ewin->x;
   start_move_y = ewin->y;
   EDBUG_RETURN(0);
}

int
doMove(void *params)
{
   return doMoveImpl(params, 0);
}

int
doMoveConstrained(void *params)
{
   return doMoveImpl(params, 1);
}

int
doMoveEnd(void *params)
{
   EWin              **gwins;
   EWin               *ewin;
   int                 d, wasresize = 0, num, i;

   EDBUG(6, "doMoveEnd");
   ewin = GetFocusEwin();
   UnGrabTheButtons();
   AUDIO_PLAY("SOUND_MOVE_STOP");
   if (!ewin)
     {
	if (mode.movemode > 0)
	   UngrabX();
	if (!mode.moveresize_pending_ewin)
	   ForceUpdatePagersForDesktop(desks.current);
	EDBUG_RETURN(0);
     }
   mode.mode = MODE_NONE;
   if (mode.noewin)
      mode.ewin = NULL;
   mode.noewin = 0;
   mode.firstlast = 2;
   d = DesktopAt(mode.x, mode.y);
   gwins = ListWinGroupMembersForEwin(ewin, ACTION_MOVE, &num);

   if (!mode.moveresize_pending_ewin)
     {
	wasresize = 1;
	for (i = 0; i < num; i++)
	   DrawEwinShape(gwins[i], mode.movemode, gwins[i]->x, gwins[i]->y, gwins[i]->client.w,
			 gwins[i]->client.h, mode.firstlast);
	for (i = 0; i < num; i++)
	   MoveEwin(gwins[i], gwins[i]->x, gwins[i]->y);
     }
   mode.moveresize_pending_ewin = NULL;
   for (i = 0; i < num; i++)
     {
	if ((gwins[i]->floating) || (mode.movemode > 0))
	  {
	     if (gwins[i]->floating)
		MoveEwinToDesktopAt(gwins[i], d,
				    gwins[i]->x - (desks.desk[d].x -
					     desks.desk[gwins[i]->desktop].x),
				    gwins[i]->y - (desks.desk[d].y -
					    desks.desk[gwins[i]->desktop].y));
	     else
		MoveEwinToDesktopAt(gwins[i], d, gwins[i]->x, gwins[i]->y);
	     gwins[i]->floating = 0;
	  }
	if ((mode.movemode > 0) && (gwins[i]->has_transients))
	  {
	     EWin              **lst;
	     int                 j, num2;
	     int                 dx, dy;

	     dx = ewin->x - start_move_x;
	     dy = ewin->y - start_move_y;

	     lst = ListTransientsFor(gwins[i]->client.win, &num2);
	     if (lst)
	       {
		  for (j = 0; j < num2; j++)
		     MoveEwin(lst[j], lst[j]->x + dx, lst[j]->y + dy);
		  Efree(lst);
	       }
	  }
	RaiseEwin(gwins[i]);
	ICCCM_Configure(gwins[i]);
     }
   mode.firstlast = 0;
   HideCoords();
   XSync(disp, False);
   if (mode.movemode > 0)
     {
	FX_Pause();
	UngrabX();
     }
   RememberImportantInfoForEwins(ewin);
   if (wasresize)
      ForceUpdatePagersForDesktop(desks.current);
   Efree(gwins);
   params = NULL;
   EDBUG_RETURN(0);
}

int
doRaise(void *params)
{
   EWin               *ewin;
   EWin              **gwins = NULL;
   int                 i, num;

   EDBUG(6, "doRaise");

   if (InZoom())
      EDBUG_RETURN(0);
   if (params)
      ewin = FindItem(NULL, atoi((char *)params), LIST_FINDBY_ID,
		      LIST_TYPE_EWIN);
   else
      ewin = GetFocusEwin();
   if (!ewin)
      EDBUG_RETURN(0);

   AUDIO_PLAY("SOUND_RAISE");
   gwins = ListWinGroupMembersForEwin(ewin, ACTION_RAISE, &num);
   for (i = 0; i < num; i++)
      RaiseEwin(gwins[i]);
   Efree(gwins);
   EDBUG_RETURN(0);
}

int
doLower(void *params)
{
   EWin               *ewin;
   EWin              **gwins = NULL;
   int                 i, num;

   EDBUG(6, "doLower");

   if (InZoom())
      EDBUG_RETURN(0);
   if (params)
      ewin = FindItem(NULL, atoi((char *)params), LIST_FINDBY_ID,
		      LIST_TYPE_EWIN);
   else
      ewin = GetFocusEwin();

   if (!ewin)
      EDBUG_RETURN(0);

   AUDIO_PLAY("SOUND_LOWER");
   gwins = ListWinGroupMembersForEwin(ewin, ACTION_LOWER, &num);
   for (i = 0; i < num; i++)
      LowerEwin(gwins[i]);
   Efree(gwins);
   EDBUG_RETURN(0);
}

int
doCleanup(void *params)
{
   char               *type;
   int                 method;
   void              **lst;
   int                 i, j, k, num, speed;
   RectBox            *fixed, *ret, *floating;
   char                doslide;
   EWin               *ewin;
   Button            **blst;

   EDBUG(6, "doCleanup");

   if (InZoom())
      EDBUG_RETURN(0);
   if ((mode.mode == MODE_MOVE) || (mode.mode == MODE_RESIZE_H) ||
       (mode.mode == MODE_RESIZE_V) || (mode.mode == MODE_RESIZE))
      EDBUG_RETURN(0);

   type = (char *)params;
   method = ARRANGE_BY_SIZE;
   speed = mode.slidespeedcleanup;
   doslide = mode.cleanupslide;

   if (params)
     {
	if (!strcmp("order", type))
	  {
	     method = ARRANGE_VERBATIM;
	  }
	else if (!strcmp("place", type))
	  {
	     method = ARRANGE_BY_POSITION;
	  }
     }
   lst = ListItemType(&num, LIST_TYPE_EWIN);
   if (lst)
     {
	fixed = NULL;
	floating = Emalloc(sizeof(RectBox) * num);
	ret = Emalloc(sizeof(RectBox) * (num));
	j = 0;
	k = 0;
	for (i = 0; i < num; i++)
	  {
	     if ((((EWin *) lst[i])->desktop == desks.current) &&
	     (!((EWin *) lst[i])->sticky) && (!((EWin *) lst[i])->floating) &&
		 (!((EWin *) lst[i])->iconified) &&
		 (!((EWin *) lst[i])->ignorearrange) &&
		 (((EWin *) lst[i])->area_x ==
		  desks.desk[((EWin *) lst[i])->desktop].current_area_x) &&
		 (((EWin *) lst[i])->area_y ==
		  desks.desk[((EWin *) lst[i])->desktop].current_area_y)
		)
	       {
		  floating[j].data = lst[i];
		  floating[j].x = ((EWin *) lst[i])->x;
		  floating[j].y = ((EWin *) lst[i])->y;
		  floating[j].w = ((EWin *) lst[i])->w;
		  floating[j].p = ((EWin *) lst[i])->layer;
		  floating[j++].h = ((EWin *) lst[i])->h;
	       }
	     else if (
			(
			   (((EWin *) lst[i])->desktop == desks.current) ||
			   (((EWin *) lst[i])->sticky)
			) &&
			(((EWin *) lst[i])->layer != 4) &&
			(((EWin *) lst[i])->layer != 0)
		)
	       {
		  fixed = Erealloc(fixed, sizeof(RectBox) * (k + 1));
		  fixed[k].data = lst[i];
		  fixed[k].x = ((EWin *) lst[i])->x;
		  fixed[k].y = ((EWin *) lst[i])->y;
		  fixed[k].w = ((EWin *) lst[i])->w;
		  if (!((EWin *) lst[i])->never_use_area)
		    {
		       fixed[k].p = ((EWin *) lst[i])->layer;
		    }
		  else
		    {
		       fixed[k].p = 99;
		    }
		  fixed[k++].h = ((EWin *) lst[i])->h;
	       }
	  }

	blst = (Button **) ListItemType(&num, LIST_TYPE_BUTTON);
	if (blst)
	  {
	     fixed = Erealloc(fixed, sizeof(RectBox) * (num + k));
	     ret = Erealloc(ret, sizeof(RectBox) * ((num + j) + 1 + k));
	     for (i = 0; i < num; i++)
	       {
		  if (((blst[i]->desktop == desks.current) ||
		       ((blst[i]->desktop == 0) && (blst[i]->sticky))) &&
		      (blst[i]->visible))
		    {
		       fixed[k].data = NULL;
		       fixed[k].x = blst[i]->x;
		       fixed[k].y = blst[i]->y;
		       fixed[k].w = blst[i]->w;
		       if (blst[i]->sticky)
			 {
			    fixed[i].p = 50;
			 }
		       else
			 {
			    fixed[i].p = 0;
			 }
		       fixed[k++].h = blst[i]->h;
		    }
	       }
	     Efree(blst);
	  }
	ArrangeRects(fixed, k, floating, j, ret, root.w, root.h, method);
	for (i = 0; i < (j + k); i++)
	  {
	     if (ret[i].data)
	       {
		  if (doslide)
		    {
		       ewin = (EWin *) ret[i].data;
		       if (ewin)
			 {
			    if ((ewin->x != ret[i].x) || (ewin->y != ret[i].y))
			      {
				 SlideEwinTo(ewin, ewin->x, ewin->y,
					     ret[i].x, ret[i].y, speed);
				 ICCCM_Configure(ewin);
			      }
			 }
		    }
		  else
		    {
		       ewin = (EWin *) ret[i].data;
		       if (ewin)
			 {
			    if ((ewin->x != ret[i].x) || (ewin->y != ret[i].y))
			       MoveEwin((EWin *) ret[i].data, ret[i].x, ret[i].y);
			 }
		    }
	       }
	  }

	if (fixed)
	   Efree(fixed);
	if (ret)
	   Efree(ret);
	if (floating)
	   Efree(floating);
	if (lst)
	   Efree(lst);
     }
   EDBUG_RETURN(0);
}

int
doKill(void *params)
{
   EWin               *ewin;
   EWin              **gwins;
   int                 num, i;

   EDBUG(6, "doKill");

   if (params)
      ewin = FindItem(NULL, atoi((char *)params), LIST_FINDBY_ID,
		      LIST_TYPE_EWIN);
   else
      ewin = GetFocusEwin();
   if (!ewin)
      EDBUG_RETURN(0);

   AUDIO_PLAY("SOUND_WINDOW_CLOSE");

   gwins = ListWinGroupMembersForEwin(ewin, ACTION_KILL, &num);
   for (i = 0; i < num; i++)
     {
	RemoveEwinFromGroup(gwins[i]);
	ICCCM_Delete(gwins[i]);
	ApplySclass(FindItem("SOUND_WINDOW_CLOSE", 0,
			     LIST_FINDBY_NAME, LIST_TYPE_SCLASS));
     }
   Efree(gwins);
   EDBUG_RETURN(0);
}

int
doKillNasty(void *params)
{
   EWin               *ewin;

   EDBUG(6, "doKillNasty");

   if (params)
      ewin = FindItem(NULL, atoi((char *)params), LIST_FINDBY_ID,
		      LIST_TYPE_EWIN);
   else
      ewin = GetFocusEwin();
   if (!ewin)
      EDBUG_RETURN(0);

   AUDIO_PLAY("SOUND_WINDOW_CLOSE");
   EDestroyWindow(disp, ewin->client.win);
   EDBUG_RETURN(0);
}

int
doNextDesktop(void *params)
{
   EDBUG(6, "doNextDesktop");

   if (InZoom())
      EDBUG_RETURN(0);
   GotoDesktop(desks.current + 1);
   AUDIO_PLAY("SOUND_DESKTOP_SHUT");
   params = NULL;
   EDBUG_RETURN(0);
}

int
doPrevDesktop(void *params)
{
   EDBUG(6, "doPrevDesktop");

   if (InZoom())
      EDBUG_RETURN(0);
   GotoDesktop(desks.current - 1);
   AUDIO_PLAY("SOUND_DESKTOP_SHUT");
   params = NULL;
   EDBUG_RETURN(0);
}

int
doRaiseDesktop(void *params)
{
   int                 d = 0;

   EDBUG(6, "doRaiseDesktop");

   if (InZoom())
      EDBUG_RETURN(0);
   if ((mode.mode == MODE_MOVE) || (mode.mode == MODE_RESIZE_H) ||
       (mode.mode == MODE_RESIZE_V) || (mode.mode == MODE_RESIZE))
      EDBUG_RETURN(0);

   if (!params)
      d = desks.current;
   else
      d = atoi((char *)params);
   AUDIO_PLAY("SOUND_DESKTOP_RAISE");
   RaiseDesktop(d);
   EDBUG_RETURN(0);
}

int
doLowerDesktop(void *params)
{
   int                 d = 0;

   EDBUG(6, "doLowerDesktop");

   if (InZoom())
      EDBUG_RETURN(0);
   if ((mode.mode == MODE_MOVE) || (mode.mode == MODE_RESIZE_H) ||
       (mode.mode == MODE_RESIZE_V) || (mode.mode == MODE_RESIZE))
      EDBUG_RETURN(0);

   if (!params)
      d = desks.current;
   else
      d = atoi((char *)params);
   AUDIO_PLAY("SOUND_DESKTOP_LOWER");
   LowerDesktop(d);
   EDBUG_RETURN(0);
}

int
doDragDesktop(void *params)
{
   int                 d = 0;

   EDBUG(6, "doDragDesktop");

   if (InZoom())
      EDBUG_RETURN(0);
   if ((mode.mode == MODE_MOVE) || (mode.mode == MODE_RESIZE_H) ||
       (mode.mode == MODE_RESIZE_V) || (mode.mode == MODE_RESIZE))
      EDBUG_RETURN(0);

   if (!params)
      d = desks.current;
   else
      d = atoi((char *)params);
   mode.deskdrag = d;
   mode.mode = MODE_DESKDRAG;
   mode.start_x = mode.x;
   mode.start_y = mode.y;
   mode.win_x = desks.desk[d].x;
   mode.win_y = desks.desk[d].y;
   EDBUG_RETURN(0);
}

int
doStick(void *params)
{
   EWin               *ewin;
   EWin              **gwins = NULL;
   int                 i, num;
   char                sticky;

   EDBUG(6, "doStick");

   if (InZoom())
      EDBUG_RETURN(0);
   if (params)
      ewin = FindItem(NULL, atoi((char *)params), LIST_FINDBY_ID,
		      LIST_TYPE_EWIN);
   else
      ewin = GetFocusEwin();
   if (!ewin)
      EDBUG_RETURN(0);
   gwins = ListWinGroupMembersForEwin(ewin, ACTION_STICK, &num);
   sticky = ewin->sticky;
   for (i = 0; i < num; i++)
     {
	if (gwins[i]->sticky && sticky)
	   MakeWindowUnSticky(gwins[i]);
	else if (!gwins[i]->sticky && !sticky)
	   MakeWindowSticky(gwins[i]);
	params = NULL;
	GNOME_SetHint(gwins[i]);
	RememberImportantInfoForEwin(gwins[i]);
     }
   Efree(gwins);
   EDBUG_RETURN(0);
}

int
doInplaceDesktop(void *params)
{
   int                 d;

   EDBUG(6, "doInplaceDesktop");

   if (InZoom())
      EDBUG_RETURN(0);
   if (!params)
      d = desks.current;
   else
      d = atoi((char *)params);

   GotoDesktop(d);
   AUDIO_PLAY("SOUND_DESKTOP_SHUT");
   EDBUG_RETURN(0);
}

int
doDragButtonStart(void *params)
{
   Button             *button;
   int                 xo, yo;

   EDBUG(6, "doDragButtonStart");

   if (InZoom())
      EDBUG_RETURN(0);
   if ((mode.mode == MODE_MOVE) || (mode.mode == MODE_RESIZE_H) ||
       (mode.mode == MODE_RESIZE_V) || (mode.mode == MODE_RESIZE))
      EDBUG_RETURN(0);

   button = mode.button;
   if (button->flags & FLAG_FIXED)
     {
	mode.button = NULL;
	EDBUG_RETURN(0);
     }
   if (!button)
      EDBUG_RETURN(0);

   GrabThePointer(root.win);
   mode.mode = MODE_BUTTONDRAG;
   mode.button_move_pending = 1;
   mode.start_x = mode.x;
   mode.start_y = mode.y;
   mode.win_x = button->x;
   mode.win_y = button->y;
   mode.firstlast = 0;
   xo = desks.desk[button->desktop].x;
   yo = desks.desk[button->desktop].y;
   params = NULL;

   EDBUG_RETURN(0);
}

int
doDragButtonEnd(void *params)
{
   Button             *button;
   int                 d;

   EDBUG(6, "doDragButtonEnd");
   button = mode.button;
   if (!button)
      EDBUG_RETURN(0);
   mode.mode = MODE_NONE;
   UnGrabTheButtons();
   if (!mode.button_move_pending)
     {
	d = DesktopAt(mode.x, mode.y);
	MoveButtonToDesktop(button, d);
	MovebuttonToCoord(button, button->x - desks.desk[button->desktop].x,
			  button->y - desks.desk[button->desktop].y);
     }
   else
      mode.button_move_pending = 0;
   params = NULL;
   autosave();

   EDBUG_RETURN(0);
}

int
doFocusModeSet(void *params)
{
   EDBUG(6, "doFocusModeSet");
   if (params)
     {
	if (!strcmp("pointer", (char *)params))
	   mode.focusmode = FOCUS_POINTER;
	else if (!strcmp("sloppy", (char *)params))
	   mode.focusmode = FOCUS_SLOPPY;
	else if (!strcmp("click", (char *)params))
	   mode.focusmode = FOCUS_CLICK;
     }
   else
     {
	if (mode.focusmode == FOCUS_POINTER)
	   mode.focusmode = FOCUS_SLOPPY;
	else if (mode.focusmode == FOCUS_SLOPPY)
	   mode.focusmode = FOCUS_CLICK;
	else if (mode.focusmode == FOCUS_CLICK)
	   mode.focusmode = FOCUS_POINTER;
     }
   FixFocus();
   autosave();
   EDBUG_RETURN(0);
}

int
doMoveModeSet(void *params)
{
   EDBUG(6, "doMoveModeSet");
   if ((mode.mode == MODE_MOVE) || (mode.mode == MODE_RESIZE_H) ||
       (mode.mode == MODE_RESIZE_V) || (mode.mode == MODE_RESIZE))
      EDBUG_RETURN(0);
   if (params)
     {
	mode.movemode = atoi((char *)params);
     }
   else
     {
	mode.movemode++;
	if (mode.movemode > 5)
	   mode.movemode = 0;
     }
   if ((ird) && (mode.movemode == 5))
      mode.movemode = 3;
   autosave();
   EDBUG_RETURN(0);
}

int
doResizeModeSet(void *params)
{
   EDBUG(6, "doResizeModeSet");
   if ((mode.mode == MODE_MOVE) || (mode.mode == MODE_RESIZE_H) ||
       (mode.mode == MODE_RESIZE_V) || (mode.mode == MODE_RESIZE))
      EDBUG_RETURN(0);
   if (params)
     {
	mode.resizemode = atoi((char *)params);
     }
   else
     {
	mode.resizemode++;
	if (mode.resizemode > 4)
	   mode.resizemode = 0;
     }
   if (mode.resizemode == 5)
      mode.resizemode = 3;
   autosave();
   EDBUG_RETURN(0);
}

int
doSlideModeSet(void *params)
{
   EDBUG(6, "doSlideModeSet");
   if (params)
     {
	mode.slidemode = atoi((char *)params);
     }
   else
     {
	mode.slidemode++;
	if (mode.slidemode > 4)
	   mode.slidemode = 0;
     }
   autosave();
   EDBUG_RETURN(0);
}

int
doCleanupSlideSet(void *params)
{
   EDBUG(6, "doCleanupSlideSet");
   if (params)
     {
	mode.cleanupslide = atoi((char *)params);
     }
   else
     {
	if (mode.cleanupslide)
	   mode.cleanupslide = 0;
	else
	   mode.cleanupslide = 1;
     }
   autosave();
   EDBUG_RETURN(0);
}

int
doMapSlideSet(void *params)
{
   EDBUG(6, "doMapSlideSet");
   if (params)
      mode.mapslide = atoi((char *)params);
   else
     {
	if (mode.mapslide)
	   mode.mapslide = 0;
	else
	   mode.mapslide = 1;
     }
   autosave();
   EDBUG_RETURN(0);
}

int
doSoundSet(void *params)
{
   SoundClass        **lst;
   int                 num, i;
   char                snd;

   EDBUG(6, "doSoundSet");
   snd = mode.sound;
   if (params)
      mode.sound = atoi((char *)params);
   else
     {
	if (mode.sound)
	   mode.sound = 0;
	else
	   mode.sound = 1;
     }
   if (mode.sound != snd)
     {
	if (!mode.sound)
	  {
	     lst = (SoundClass **) ListItemType(&num, LIST_TYPE_SCLASS);
	     if (lst)
	       {
		  for (i = 0; i < num; i++)
		    {
		       if (lst[i]->sample)
			  DestroySample(lst[i]->sample);
		       lst[i]->sample = NULL;
		    }
		  Efree(lst);
	       }
	     close(sound_fd);
	     sound_fd = -1;
	  }
	else
	   SoundInit();
     }
   autosave();
   EDBUG_RETURN(0);
}

int
doButtonMoveResistSet(void *params)
{
   EDBUG(6, "doButtonMoveResistSet");
   if (params)
      mode.button_move_resistance = atoi((char *)params);
   autosave();
   EDBUG_RETURN(0);
}

int
doDesktopBgTimeoutSet(void *params)
{
   EDBUG(6, "doDesktopBgTimeoutSet");
   if (params)
      mode.desktop_bg_timeout = atoi((char *)params);
   autosave();
   EDBUG_RETURN(0);
}

int
doMapSlideSpeedSet(void *params)
{
   EDBUG(6, "doMapSlideSpeedSet");
   if (params)
      mode.slidespeedmap = atoi((char *)params);
   autosave();
   EDBUG_RETURN(0);
}

int
doCleanupSlideSpeedSet(void *params)
{
   EDBUG(6, "doCleanupSlideSpeedSet");
   if (params)
      mode.slidespeedcleanup = atoi((char *)params);
   autosave();
   EDBUG_RETURN(0);
}

int
doDragdirSet(void *params)
{
   char                pd;
   Button             *b;
   int                 i;

   EDBUG(6, "doDragdirSet");
   pd = desks.dragdir;
   if (params)
      desks.dragdir = atoi((char *)params);
   else
     {
	desks.dragdir++;
	if (desks.dragdir > 3)
	   desks.dragdir = 0;
     }
   if (pd != desks.dragdir)
     {
	GotoDesktop(desks.current);
	for (i = 0; i < ENLIGHTENMENT_CONF_NUM_DESKTOPS; i++)
	   MoveDesktop(i, 0, 0);
	while ((b = RemoveItem("_DESKTOP_DRAG_CONTROL", 0, LIST_FINDBY_NAME,
			       LIST_TYPE_BUTTON)))
	   DestroyButton(b);
	while ((b = RemoveItem("_DESKTOP_DESKRAY_DRAG_CONTROL",
			       0, LIST_FINDBY_NAME, LIST_TYPE_BUTTON)))
	   DestroyButton(b);
	InitDesktopControls();
	ShowDesktopControls();
     }
   autosave();
   EDBUG_RETURN(0);
}

int
doDragbarOrderSet(void *params)
{
   char                pd;
   Button             *b;

   EDBUG(6, "doDragbarOrderSet");
   pd = desks.dragbar_ordering;
   if (params)
      desks.dragbar_ordering = atoi((char *)params);
   else
     {
	desks.dragbar_ordering++;
	if (desks.dragbar_ordering > 5)
	   desks.dragbar_ordering = 0;
     }
   if (pd != desks.dragbar_ordering)
     {
	while ((b = RemoveItem("_DESKTOP_DRAG_CONTROL", 0, LIST_FINDBY_NAME,
			       LIST_TYPE_BUTTON)))
	   DestroyButton(b);
	InitDesktopControls();
	ShowDesktopControls();
     }
   autosave();
   EDBUG_RETURN(0);
}

int
doDragbarWidthSet(void *params)
{
   int                 pd;
   Button             *b;

   EDBUG(6, "doDragbarWidthSet");
   pd = desks.dragbar_width;
   if (params)
      desks.dragbar_width = atoi((char *)params);
   if (pd != desks.dragbar_width)
     {
	while ((b = RemoveItem("_DESKTOP_DRAG_CONTROL", 0, LIST_FINDBY_NAME,
			       LIST_TYPE_BUTTON)))
	   DestroyButton(b);
	InitDesktopControls();
	ShowDesktopControls();
     }
   autosave();
   EDBUG_RETURN(0);
}

int
doDragbarLengthSet(void *params)
{
   int                 pd;
   Button             *b;

   EDBUG(6, "doDragbarLengthSet");
   pd = desks.dragbar_length;
   if (params)
      desks.dragbar_length = atoi((char *)params);
   if (pd != desks.dragbar_length)
     {
	while ((b = RemoveItem("_DESKTOP_DRAG_CONTROL", 0, LIST_FINDBY_NAME,
			       LIST_TYPE_BUTTON)))
	   DestroyButton(b);
	InitDesktopControls();
	ShowDesktopControls();
     }
   autosave();
   EDBUG_RETURN(0);
}

int
doDeskSlideSet(void *params)
{
   EDBUG(6, "doDeskSlideSet");
   if (params)
      desks.slidein = atoi((char *)params);
   else
     {
	if (desks.slidein)
	   desks.slidein = 0;
	else
	   desks.slidein = 1;
     }
   autosave();
   EDBUG_RETURN(0);
}

int
doDeskSlideSpeedSet(void *params)
{
   EDBUG(6, "doDeskSlideSpeedSet");
   if (params)
      desks.slidespeed = atoi((char *)params);
   autosave();
   EDBUG_RETURN(0);
}

int
doHiQualityBgSet(void *params)
{
   EDBUG(6, "doHiQualityBgSet");
   if (params)
      desks.hiqualitybg = atoi((char *)params);
   else
     {
	if (desks.hiqualitybg)
	   desks.hiqualitybg = 0;
	else
	   desks.hiqualitybg = 1;
     }
   autosave();
   EDBUG_RETURN(0);
}

int
doPlaySoundClass(void *params)
{
   EDBUG(6, "doPlaySoundClass");

   if (!params)
      EDBUG_RETURN(0);

   ApplySclass(FindItem((char *)params, 0, LIST_FINDBY_NAME,
			LIST_TYPE_SCLASS));

   EDBUG_RETURN(0);
}

int
doGotoDesktop(void *params)
{
   int                 d = 0;

   EDBUG(6, "doGotoDesktop");

   if (InZoom())
      EDBUG_RETURN(0);
   if (!params)
      EDBUG_RETURN(0);

   sscanf((char *)params, "%i", &d);
   GotoDesktop(d);
   AUDIO_PLAY("SOUND_DESKTOP_SHUT");
   EDBUG_RETURN(0);
}

int
doDeskray(void *params)
{
   EDBUG(6, "doDeskray");
   if (InZoom())
      EDBUG_RETURN(0);
   if ((mode.mode == MODE_MOVE) || (mode.mode == MODE_RESIZE_H) ||
       (mode.mode == MODE_RESIZE_V) || (mode.mode == MODE_RESIZE))
      EDBUG_RETURN(0);
   if (params)
     {
	if (!atoi((char *)params))
	  {
	     HideDesktopTabs();
	     mode.deskmode = MODE_NONE;
	  }
	else
	  {
	     mode.deskmode = MODE_DESKRAY;
	     ShowDesktopTabs();
	  }
     }
   else
     {
	if (mode.deskmode == MODE_DESKRAY)
	  {
	     HideDesktopTabs();
	     mode.deskmode = MODE_NONE;
	  }
	else
	  {
	     mode.deskmode = MODE_DESKRAY;
	     ShowDesktopTabs();
	  }
     }
   EDBUG_RETURN(0);
}

int
doAutosaveSet(void *params)
{
   EDBUG(6, "doAutosaveSet");
   if (params)
      mode.autosave = atoi((char *)params);
   else
     {
	if (mode.autosave)
	   mode.autosave = 0;
	else
	   mode.autosave = 1;
     }
   EDBUG_RETURN(0);
}

int
doHideShowButton(void *params)
{
   Button            **lst, *b;
   char                s[1024], *ss;
   int                 num, i;

   /* This is unused - where did this come from? -Mandrake */
   /* static char         lasthide = 0; */

   EDBUG(6, "doHideButtons");
   if (InZoom())
      EDBUG_RETURN(0);
   if ((mode.mode == MODE_MOVE) || (mode.mode == MODE_RESIZE_H) ||
       (mode.mode == MODE_RESIZE_V) || (mode.mode == MODE_RESIZE))
      EDBUG_RETURN(0);

   if (params)
     {
	sscanf((char *)params, "%1000s", s);
	if (!strcmp(s, "button"))
	  {
	     sscanf((char *)params, "%*s %1000s", s);
	     b = (Button *) FindItem(s, 0, LIST_FINDBY_NAME, LIST_TYPE_BUTTON);
	     if ((b) && (!b->used))
	       {
		  if (b->visible)
		    {
		       HideButton(b);
		    }
		  else
		    {
		       ShowButton(b);
		    }
	       }
	  }
	else if (!strcmp(s, "buttons"))
	  {
	     ss = atword((char *)params, 2);
	     if (ss)
	       {
		  lst = (Button **) ListItemType(&num, LIST_TYPE_BUTTON);
		  if (lst)
		    {
		       for (i = 0; i < num; i++)
			 {
			    if (matchregexp(ss, lst[i]->name))
			      {
				 if ((strcmp(lst[i]->name,
					   "_DESKTOP_DESKRAY_DRAG_CONTROL") &&
				      (!lst[i]->used)))
				   {
				      if (!(lst[i]->visible))
					{
					   ShowButton(lst[i]);
					}
				      else
					{
					   HideButton(lst[i]);
					}
				   }
			      }
			 }
		    }
	       }
	  }
	else if (!strcmp(s, "all_buttons_except"))
	  {
	     ss = atword((char *)params, 2);
	     if (ss)
	       {
		  lst = (Button **) ListItemTypeID(&num, LIST_TYPE_BUTTON, 0);
		  if (lst)
		    {
		       for (i = 0; i < num; i++)
			 {
			    if (!matchregexp(ss, lst[i]->name))
			      {
				 if ((strcmp(lst[i]->name,
					   "_DESKTOP_DESKRAY_DRAG_CONTROL") &&
				      (!lst[i]->used)))
				   {
				      if (!(lst[i]->visible))
					{
					   ShowButton(lst[i]);
					}
				      else
					{
					   HideButton(lst[i]);
					}
				   }
			      }
			 }
		    }
	       }
	  }
	else if (!strcmp(s, "all"))
	  {
	     lst = (Button **) ListItemType(&num, LIST_TYPE_BUTTON);
	     if (lst)
	       {
		  for (i = 0; i < num; i++)
		    {
		       if ((strcmp(lst[i]->name,
				   "_DESKTOP_DESKRAY_DRAG_CONTROL") &&
			    (!lst[i]->used)))
			 {
			    if (!(lst[i]->visible))
			      {
				 ShowButton(lst[i]);
			      }
			    else
			      {
				 HideButton(lst[i]);
			      }
			 }
		    }
	       }
	  }
     }
   else
     {
	lst = (Button **) ListItemTypeID(&num, LIST_TYPE_BUTTON, 0);
	if (lst)
	  {
	     for (i = 0; i < num; i++)
	       {
		  if (!lst[i]->used)
		    {
		       if (!(lst[i]->visible))
			 {
			    ShowButton(lst[i]);
			 }
		       else
			 {
			    HideButton(lst[i]);
			 }
		    }
	       }
	  }
     }
   autosave();

   EDBUG_RETURN(0);
}

int
doIconifyWindow(void *params)
{
   EWin               *ewin;
   char               *windowid = 0;
   char                iconified;
   EWin              **gwins = NULL;
   int                 i, num;

   EDBUG(6, "doIconifyWindow");

   if (params)
     {
	windowid = (char *)params;
	ewin = FindItem("ICON", atoi(windowid), LIST_FINDBY_BOTH,
			LIST_TYPE_ICONIFIEDS);
	if (!ewin)
	   ewin = FindItem(NULL, atoi((char *)params), LIST_FINDBY_ID,
			   LIST_TYPE_EWIN);
     }
   else
      ewin = GetFocusEwin();

   if (!ewin)
      EDBUG_RETURN(1);

   gwins = ListWinGroupMembersForEwin(ewin, ACTION_ICONIFY, &num);
   iconified = ewin->iconified;

   for (i = 0; i < num; i++)
     {
	if (gwins[i]->iconified && iconified)
	  {
	     DeIconifyEwin(gwins[i]);
	  }
	else if (!gwins[i]->iconified && !iconified)
	  {
	     IconifyEwin(gwins[i]);
	  }
     }
   Efree(gwins);
   EDBUG_RETURN(0);
}

int
doSlideout(void *params)
{
   Slideout           *s;

   EDBUG(6, "doSlideout");
   if (InZoom())
      EDBUG_RETURN(0);
   if (!params)
      EDBUG_RETURN(0);

   s = FindItem((char *)params, 0, LIST_FINDBY_NAME, LIST_TYPE_SLIDEOUT);
   if (s)
     {
	AUDIO_PLAY("SOUND_SLIDEOUT_SHOW");
	ShowSlideout(s, mode.context_win);
	s->ref_count++;
     }
   EDBUG_RETURN(0);
}

int
doScrollWindows(void *params)
{

   int                 x, y, num, i;
   EWin              **lst;

   EDBUG(6, "doScrollWindows");
   if (InZoom())
      EDBUG_RETURN(0);
   if (!params)
      EDBUG_RETURN(0);

   if (mode.slideout)
      HideSlideout(mode.slideout, mode.context_win);

   x = 0;
   y = 0;
   sscanf((char *)params, "%i %i", &x, &y);

   lst = (EWin **) ListItemType(&num, LIST_TYPE_EWIN);

   if ((lst) && (num > 0))
     {
	for (i = 0; i < num; i++)
	  {
	     if ((lst[i]->desktop == desks.current) &&
		 (!lst[i]->sticky) && (!lst[i]->floating) &&
		 (!lst[i]->fixedpos))
		MoveEwin(lst[i], lst[i]->x + x, lst[i]->y + y);
	  }
	Efree(lst);
     }
   EDBUG_RETURN(0);
}

int
doShade(void *params)
{
   EWin               *ewin;
   EWin              **gwins = NULL;
   int                 i, num;
   char                shaded;

   EDBUG(6, "doShade");
   if (InZoom())
      EDBUG_RETURN(0);
   if (params)
      ewin = FindItem(NULL, atoi((char *)params),
		      LIST_FINDBY_ID, LIST_TYPE_EWIN);
   else
      ewin = GetFocusEwin();

   if (!ewin)
      EDBUG_RETURN(0);

   gwins = ListWinGroupMembersForEwin(ewin, ACTION_SHADE, &num);
   shaded = ewin->shaded;
   for (i = 0; i < num; i++)
     {
	if (gwins[i]->shaded && shaded)
	  {
	     AUDIO_PLAY("SOUND_UNSHADE");
	     UnShadeEwin(gwins[i]);
	  }
	else if (!gwins[i]->shaded && !shaded)
	  {
	     AUDIO_PLAY("SOUND_SHADE");
	     ShadeEwin(gwins[i]);
	  }
	params = NULL;
	RememberImportantInfoForEwin(gwins[i]);
     }
   Efree(gwins);
   EDBUG_RETURN(0);
}

int
doMaxH(void *params)
{
   EWin               *ewin;

   EDBUG(6, "doMaxH");
   if (InZoom())
      EDBUG_RETURN(0);
   ewin = GetFocusEwin();
   if (ewin)
     {
	MaxHeight(ewin, (char *)params);
	RememberImportantInfoForEwin(ewin);
     }
   EDBUG_RETURN(0);
}

int
doMaxW(void *params)
{
   EWin               *ewin;

   EDBUG(6, "doMaxW");
   if (InZoom())
      EDBUG_RETURN(0);
   ewin = GetFocusEwin();
   if (ewin)
     {
	MaxWidth(ewin, (char *)params);
	RememberImportantInfoForEwin(ewin);
     }
   EDBUG_RETURN(0);
}

int
doMax(void *params)
{
   EWin               *ewin;

   EDBUG(6, "doMax");
   if (InZoom())
      EDBUG_RETURN(0);
   ewin = GetFocusEwin();
   if (ewin)
     {
	MaxSize(ewin, (char *)params);
	RememberImportantInfoForEwin(ewin);
     }
   EDBUG_RETURN(0);
}

int
doSendToNextDesk(void *params)
{
   EWin               *ewin;

   EDBUG(6, "doSendToNextDesk");
   if (InZoom())
      EDBUG_RETURN(0);
   ewin = GetFocusEwin();
   if (!ewin)
      EDBUG_RETURN(0);

   DesktopRemoveEwin(ewin);
   MoveEwinToDesktop(ewin, ewin->desktop + 1);
   RaiseEwin(ewin);
   ICCCM_Configure(ewin);
   ewin->sticky = 0;
   params = NULL;
   RememberImportantInfoForEwin(ewin);
   EDBUG_RETURN(0);
}

int
doSendToPrevDesk(void *params)
{
   EWin               *ewin;

   EDBUG(6, "doSendToPrevDesk");
   if (InZoom())
      EDBUG_RETURN(0);
   ewin = GetFocusEwin();
   if (!ewin)
      EDBUG_RETURN(0);

   DesktopRemoveEwin(ewin);
   MoveEwinToDesktop(ewin, ewin->desktop - 1);
   RaiseEwin(ewin);
   ICCCM_Configure(ewin);
   ewin->sticky = 0;
   params = NULL;
   RememberImportantInfoForEwin(ewin);
   EDBUG_RETURN(0);
}

int
doSnapshot(void *params)
{
   EWin               *ewin;

   EDBUG(6, "doSnapshot");

   ewin = GetFocusEwin();
   if (!ewin)
      EDBUG_RETURN(0);

   if (!params)
      SnapshotEwinAll(ewin);
   else if (!strcmp((char *)params, "none"))
      UnsnapshotEwin(ewin);
   else if (!strcmp((char *)params, "border"))
      SnapshotEwinBorder(ewin);
   else if (!strcmp((char *)params, "desktop"))
      SnapshotEwinDesktop(ewin);
   else if (!strcmp((char *)params, "size"))
      SnapshotEwinSize(ewin);
   else if (!strcmp((char *)params, "location"))
      SnapshotEwinLocation(ewin);
   else if (!strcmp((char *)params, "layer"))
      SnapshotEwinLayer(ewin);
   else if (!strcmp((char *)params, "sticky"))
      SnapshotEwinSticky(ewin);
   else if (!strcmp((char *)params, "icon"))
      SnapshotEwinIcon(ewin);
   else if (!strcmp((char *)params, "shade"))
      SnapshotEwinShade(ewin);
   else if (!strcmp((char *)params, "group"))
      SnapshotEwinGroup(ewin, 1);
   else if (!strcmp((char *)params, "dialog"))
      SnapshotEwinDialog(ewin);
   EDBUG_RETURN(0);
}

int
doScrollContainer(void *params)
{
   EDBUG(6, "doScrollContainer");

   if (!params)
      EDBUG_RETURN(0);
   if (mode.slideout)
      HideSlideout(mode.slideout, mode.context_win);
   EDBUG_RETURN(0);

}

int
doToolTipSet(void *params)
{
   EDBUG(6, "doToolTipSet");

   if (params)
      mode.tooltips = atoi((char *)params);
   else
     {
	mode.tooltips++;
	if (mode.tooltips > 1)
	   mode.tooltips = 0;
     }
   autosave();
   EDBUG_RETURN(0);
}

int
doFocusNext(void *params)
{
   EDBUG(6, "doFocusNext");

   GetNextFocusEwin();
   params = NULL;
   EDBUG_RETURN(0);
}

int
doFocusPrev(void *params)
{
   EDBUG(6, "doFocusPrev");

   GetPrevFocusEwin();
   params = NULL;
   EDBUG_RETURN(0);
}

int
doFocusSet(void *params)
{
   Window              win;
   EWin               *ewin;

   EDBUG(6, "doFocusSet");

   if (!params)
      EDBUG_RETURN(0);
   sscanf((char *)params, "%i", (int *)&win);
   ewin = (EWin *) FindItem(NULL, win, LIST_FINDBY_ID, LIST_TYPE_EWIN);
   if (ewin)
     {
	GotoDesktop(ewin->desktop);
	SetCurrentArea(ewin->area_x, ewin->area_y);
	if (ewin->iconified)
	   DeIconifyEwin(ewin);
	FocusToEWin(ewin);
     }
   EDBUG_RETURN(0);
}

int
doBackgroundSet(void *params)
{
   int                 desk;
   Background         *bg;
   char                view, s[1024];

   EDBUG(6, "doBackgroundSet");

   if (!params)
      EDBUG_RETURN(0);

   desk = desks.current;
   if (sscanf((char *)params, "%1000s %i", s, &desk) < 2)
      desk = desks.current;
   bg = (Background *) FindItem(s, 0, LIST_FINDBY_NAME, LIST_TYPE_BACKGROUND);
   if (!bg)
      EDBUG_RETURN(0);

   if (desks.desk[desk].bg != bg)
     {
	char                pq;

	if (desks.desk[desk].bg)
	   desks.desk[desk].bg->last_viewed = 0;
	view = desks.desk[desk].viewable;
	desks.desk[desk].viewable = 0;
	DesktopAccounting();
	desks.desk[desk].viewable = view;
	BGSettingsGoTo(bg);
	pq = queue_up;
	queue_up = 0;
	SetDesktopBg(desk, bg);
	RefreshDesktop(desk);
	RedrawPagersForDesktop(desk, 2);
	ForceUpdatePagersForDesktop(desk);
	queue_up = pq;
     }
   autosave();

   EDBUG_RETURN(0);
}

int
doAreaSet(void *params)
{
   int                 a, b;

   EDBUG(6, "doAreaSet");
   if (InZoom())
      EDBUG_RETURN(0);
   if (!params)
      EDBUG_RETURN(0);
   sscanf((char *)params, "%i %i", &a, &b);
   SetCurrentArea(a, b);

   EDBUG_RETURN(0);
}

int
doAreaMoveBy(void *params)
{
   int                 a, b;

   EDBUG(6, "doAreaMoveBy");
   if (InZoom())
      EDBUG_RETURN(0);
   if (!params)
      EDBUG_RETURN(0);

   sscanf((char *)params, "%i %i", &a, &b);
   MoveCurrentAreaBy(a, b);

   EDBUG_RETURN(0);
}

int
doToggleFixedPos(void *params)
{
   EWin               *ewin;

   EDBUG(6, "doToggleFixedPos");

   ewin = GetFocusEwin();
   if (!ewin)
      EDBUG_RETURN(0);

   if (ewin->fixedpos)
      ewin->fixedpos = 0;
   else
      ewin->fixedpos = 1;

   params = NULL;
   EDBUG_RETURN(0);
}

int
doSetLayer(void *params)
{
   EWin               *ewin;
   int                 l;

   EDBUG(6, "doSetLayer");
   if (InZoom())
      EDBUG_RETURN(0);
   ewin = GetFocusEwin();
   if (!ewin)
      EDBUG_RETURN(0);
   if (!params)
      EDBUG_RETURN(0);

   l = atoi((char *)params);
   if (ewin->layer > l)
     {
	AUDIO_PLAY("SOUND_WINDOW_CHANGE_LAYER_DOWN");
     }
   else if (ewin->layer < l)
     {
	AUDIO_PLAY("SOUND_WINDOW_CHANGE_LAYER_UP");
     }
   ewin->layer = l;
   RaiseEwin(ewin);
   RememberImportantInfoForEwin(ewin);
   EDBUG_RETURN(0);
}

int
doWarpPointer(void *params)
{
   int                 dx, dy;

   EDBUG(6, "doWarpPointer");

   if (params)
     {
	sscanf((char *)params, "%i %i", &dx, &dy);
	XWarpPointer(disp, None, None, 0, 0, 0, 0, dx, dy);
     }
   EDBUG_RETURN(0);
}

int
doMoveWinToArea(void *params)
{
   EWin               *ewin;
   int                 dx, dy;

   EDBUG(6, "doMoveWinToArea");
   if (InZoom())
      EDBUG_RETURN(0);
   ewin = GetFocusEwin();
   if (!ewin)
      EDBUG_RETURN(0);

   if (params)
     {
	sscanf((char *)params, "%i %i", &dx, &dy);
	MoveEwinToArea(ewin, dx, dy);
     }
   RememberImportantInfoForEwin(ewin);
   EDBUG_RETURN(0);
}

int
doMoveWinByArea(void *params)
{
   EWin               *ewin;
   int                 dx, dy;

   EDBUG(6, "doMoveWinByArea");
   if (InZoom())
      EDBUG_RETURN(0);
   ewin = GetFocusEwin();
   if (!ewin)
      EDBUG_RETURN(0);

   if (params)
     {
	sscanf((char *)params, "%i %i", &dx, &dy);
	dx = ewin->area_x + dx;
	dy = ewin->area_y + dy;
	MoveEwinToArea(ewin, dx, dy);
     }
   RememberImportantInfoForEwin(ewin);
   EDBUG_RETURN(0);
}

int
doSetWinBorder(void *params)
{
   EWin               *ewin;
   EWin              **gwins = NULL;
   int                 i, num;
   char                buf[1024], has_shaded;
   Border             *b;
   char                shadechange = 0;

   EDBUG(6, "doSetWinBorder");

   ewin = GetFocusEwin();
   if (!ewin)
      EDBUG_RETURN(0);

   if (!params)
      EDBUG_RETURN(0);

   gwins = ListWinGroupMembersForEwin(ewin, ACTION_SET_WINDOW_BORDER, &num);

   sscanf((char *)params, "%1000s", buf);
   b = (Border *) FindItem(buf, 0, LIST_FINDBY_NAME, LIST_TYPE_BORDER);
   if (!b)
      EDBUG_RETURN(0);
   has_shaded = 0;
   for (i = 0; i < num; i++)
     {
	if (gwins[i]->shaded)
	   has_shaded = 1;
     }
   if (has_shaded)
     {
	if ((b->border.left == 0) &&
	    (b->border.right == 0) &&
	    (b->border.top == 0) &&
	    (b->border.bottom == 0))
	   EDBUG_RETURN(0);
     }
   for (i = 0; i < num; i++)
     {
	if (b != gwins[i]->border)
	  {
	     gwins[i]->border_new = 1;
	     AUDIO_PLAY("SOUND_WINDOW_BORDER_CHANGE");
	     if (ewin->shaded)
	       {
		  shadechange = 1;
		  InstantUnShadeEwin(ewin);
	       }
	     SetEwinToBorder(gwins[i], b);
	     if (shadechange)
		InstantShadeEwin(ewin);
	     shadechange = 0;
	     ICCCM_MatchSize(gwins[i]);
	     MoveResizeEwin(gwins[i], gwins[i]->x, gwins[i]->y, gwins[i]->client.w,
			    gwins[i]->client.h);
	  }
	RememberImportantInfoForEwin(gwins[i]);
     }
   Efree(gwins);
   EDBUG_RETURN(0);
}

int
doLinearAreaSet(void *params)
{
   int                 da;

   EDBUG(6, "doLinearAreaSet");
   if (InZoom())
      EDBUG_RETURN(0);
   if (params)
     {
	sscanf((char *)params, "%i", &da);
	SetCurrentLinearArea(da);
     }
   EDBUG_RETURN(0);
}

int
doLinearAreaMoveBy(void *params)
{
   int                 da;

   EDBUG(6, "doLinearAreaMoveBy");
   if (InZoom())
      EDBUG_RETURN(0);
   if (params)
     {
	sscanf((char *)params, "%i", &da);
	MoveCurrentLinearAreaBy(da);
     }
   EDBUG_RETURN(0);
}

int
doAbout(void *params)
{
   Dialog             *d;
   DItem              *table, *di;

   EDBUG(6, "doAbout");
   if (InZoom())
      EDBUG_RETURN(0);
   if ((d = FindItem("ABOUT_ENLIGHTENMENT", 0,
		     LIST_FINDBY_NAME, LIST_TYPE_DIALOG)))
     {
	ShowDialog(d);
	EDBUG_RETURN(0);
     }
   d = CreateDialog("ABOUT_ENLIGHTENMENT");
   {
      char                stuff[255];

      Esnprintf(stuff, sizeof(stuff), "About Enlightenment %s",
		ENLIGHTENMENT_VERSION);
      DialogSetTitle(d, stuff);
   }

   table = DialogInitItem(d);
   DialogItemTableSetOptions(table, 1, 0, 0, 0);

   di = DialogAddItem(table, DITEM_IMAGE);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemImageSetFile(di, "pix/about.png");

   di = DialogAddItem(table, DITEM_TEXT);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemTextSetText(di,
			 "THIS IS NOT A RELEASE VERSION\n"
			 "You are using this version at your own risk!!\n"
			 "\n"
			 "Welcome to the "
			 ENLIGHTENMENT_VERSION
			 " development series "
			 "of the Enlightenment\n"
			 "window manager. Enlightenment is still under "
			 "development, but\n"
			 "we have tried to iron out all the bugs "
			 "that we can find. If\n"
			 "you find a bug in the software, please do "
			 "not hesitate to send\n"
			 "in a bug report.  See \"Help\" for information "
			 "on joining the\n"
			 "mailing list.\n"
			 "\n"
			 "This code last updated on:\n"
			 E_CHECKOUT_DATE "\n"
			 "\n"
			 "Good luck. I hope you enjoy the software.\n"
			 "\n"
			 "The Rasterman - raster@rasterman.com\n"
			 "Mandrake - mandrake@mandrake.net\n");

   DialogAddButton(d, "OK", NULL, 1);
   ShowDialog(d);

   params = NULL;
   EDBUG_RETURN(0);
}

int
doFX(void *params)
{
   EDBUG(6, "doFX");
   if (InZoom())
      EDBUG_RETURN(0);
   if (params)
      FX_Start((char *)params);
   autosave();
   EDBUG_RETURN(0);
}

int
doMoveWinToLinearArea(void *params)
{
   EWin               *ewin;
   int                 da;

   EDBUG(6, "doMoveWinToLinearArea");
   if (InZoom())
      EDBUG_RETURN(0);
   ewin = GetFocusEwin();
   if (!ewin)
      EDBUG_RETURN(0);
   if (params)
     {
	sscanf((char *)params, "%i", &da);
	MoveEwinToLinearArea(ewin, da);
     }
   RememberImportantInfoForEwin(ewin);
   EDBUG_RETURN(0);
}

int
doMoveWinByLinearArea(void *params)
{
   EWin               *ewin;
   int                 da;

   EDBUG(6, "doMoveWinByLinearArea");
   if (InZoom())
      EDBUG_RETURN(0);
   ewin = GetFocusEwin();
   if (!ewin)
      EDBUG_RETURN(0);
   if (params)
     {
	sscanf((char *)params, "%i", &da);
	MoveEwinLinearAreaBy(ewin, da);
     }
   RememberImportantInfoForEwin(ewin);
   EDBUG_RETURN(0);
}

int
doSetPagerHiq(void *params)
{
   EDBUG(6, "doSetPagerHiq");
   if (params)
     {
	char                num;

	num = atoi(params);
	PagerSetHiQ(num);
     }
   autosave();
   EDBUG_RETURN(0);
}

int
doSetPagerSnap(void *params)
{
   EDBUG(6, "doSetPagerSnap");
   if (params)
     {
	char                num;

	num = atoi((char *)params);
	PagerSetSnap(num);
     }
   autosave();
   EDBUG_RETURN(0);
}

int
doConfigure(void *params)
{
   char                s[1024];

   EDBUG(6, "doConfigure");
   if (InZoom())
      EDBUG_RETURN(0);
   sscanf((char *)params, "%1000s", s);
   if (params)
     {
	if (!strcmp(s, "pager"))
	   SettingsPager();
	else if (!strcmp(s, "focus"))
	   SettingsFocus();
	else if (!strcmp(s, "moveresize"))
	   SettingsMoveResize();
	else if (!strcmp(s, "desktops"))
	   SettingsDesktops();
	else if (!strcmp(s, "area"))
	   SettingsArea();
	else if (!strcmp(s, "placement"))
	   SettingsPlacement();
	else if (!strcmp(s, "icons"))
	   SettingsIcons();
	else if (!strcmp(s, "autoraise"))
	   SettingsAutoRaise();
	else if (!strcmp(s, "tooltips"))
	   SettingsTooltips();
	else if (!strcmp(s, "audio"))
	   SettingsAudio();
	else if (!strcmp(s, "fx"))
	   SettingsSpecialFX();
	else if (!strcmp(s, "bg"))
	   SettingsBackground(desks.desk[desks.current].bg);
	else if (!strcmp(s, "iconbox"))
	  {
	     sscanf((char *)params, "%*s %1000s", s);
	     SettingsIconbox(s);
	  }
	else if (!strcmp(s, "group"))
	  {
	     EWin               *ewin = GetFocusEwin();

	     if (ewin)
	       {
		  if (ewin->group)
		     SettingsGroup(ewin->group);
		  else
		     DIALOG_OK("Window Group Error", "\n  This window does not currently  \n  belong to a group.  \n");
	       }
	  }
	else if (!strcmp(s, "group_membership"))
	  {
	     EWin               *ewin = GetFocusEwin();

	     if (ewin)
	       {
		  ChooseGroupForEwinDialog(ewin);
	       }
	  }
     }
   EDBUG_RETURN(0);
}

struct _keyset
{
   char               *sym;
   int                 state;
   char               *ch;
};

int
doInsertKeys(void *params)
{
   const struct _keyset ks[] =
   {
      {"a", 0, "a"},
      {"b", 0, "b"},
      {"c", 0, "c"},
      {"d", 0, "d"},
      {"e", 0, "e"},
      {"f", 0, "f"},
      {"g", 0, "g"},
      {"h", 0, "h"},
      {"i", 0, "i"},
      {"j", 0, "j"},
      {"k", 0, "k"},
      {"l", 0, "l"},
      {"m", 0, "m"},
      {"n", 0, "n"},
      {"o", 0, "o"},
      {"p", 0, "p"},
      {"q", 0, "q"},
      {"r", 0, "r"},
      {"s", 0, "s"},
      {"t", 0, "t"},
      {"u", 0, "u"},
      {"v", 0, "v"},
      {"w", 0, "w"},
      {"x", 0, "x"},
      {"y", 0, "y"},
      {"z", 0, "z"},
      {"a", ShiftMask, "A"},
      {"b", ShiftMask, "B"},
      {"c", ShiftMask, "C"},
      {"d", ShiftMask, "D"},
      {"e", ShiftMask, "E"},
      {"f", ShiftMask, "F"},
      {"g", ShiftMask, "G"},
      {"h", ShiftMask, "H"},
      {"i", ShiftMask, "I"},
      {"j", ShiftMask, "J"},
      {"k", ShiftMask, "K"},
      {"l", ShiftMask, "L"},
      {"m", ShiftMask, "M"},
      {"n", ShiftMask, "N"},
      {"o", ShiftMask, "O"},
      {"p", ShiftMask, "P"},
      {"q", ShiftMask, "Q"},
      {"r", ShiftMask, "R"},
      {"s", ShiftMask, "S"},
      {"t", ShiftMask, "T"},
      {"u", ShiftMask, "U"},
      {"v", ShiftMask, "V"},
      {"w", ShiftMask, "W"},
      {"x", ShiftMask, "X"},
      {"y", ShiftMask, "Y"},
      {"z", ShiftMask, "Z"},
      {"grave", 0, "`"},
      {"1", 0, "1"},
      {"2", 0, "2"},
      {"3", 0, "3"},
      {"4", 0, "4"},
      {"5", 0, "5"},
      {"6", 0, "6"},
      {"7", 0, "7"},
      {"8", 0, "8"},
      {"9", 0, "9"},
      {"0", 0, "0"},
      {"minus", 0, "-"},
      {"equal", 0, "="},
      {"bracketleft", 0, "["},
      {"bracketright", 0, "]"},
      {"backslash", 0, "\\\\"},
      {"semicolon", 0, "\\s"},
      {"apostrophe", 0, "\\a"},
      {"comma", 0, ","},
      {"period", 0, "."},
      {"slash", 0, "/"},
      {"grave", ShiftMask, "~"},
      {"1", ShiftMask, "!"},
      {"2", ShiftMask, "@"},
      {"3", ShiftMask, "#"},
      {"4", ShiftMask, "$"},
      {"5", ShiftMask, "%"},
      {"6", ShiftMask, "^"},
      {"7", ShiftMask, "&"},
      {"8", ShiftMask, "*"},
      {"9", ShiftMask, "("},
      {"0", ShiftMask, ")"},
      {"minus", ShiftMask, "_"},
      {"equal", ShiftMask, "+"},
      {"bracketleft", ShiftMask, "{"},
      {"bracketright", ShiftMask, "}"},
      {"backslash", ShiftMask, "|"},
      {"semicolon", ShiftMask, ":"},
      {"apostrophe", ShiftMask, "\\q"},
      {"comma", ShiftMask, "<"},
      {"period", ShiftMask, ">"},
      {"slash", ShiftMask, "?"},
      {"space", ShiftMask, " "},
      {"Return", ShiftMask, "\\n"},
      {"Tab", ShiftMask, "\\t"}
   };

   EDBUG(6, "doInsertKeys");
   if (params)
     {
	Window              win = 0;
	int                 i, rev;
	char               *s;
	XKeyEvent           ev;

	s = (char *)params;
	XGetInputFocus(disp, &win, &rev);
	if (win)
	  {
	     AUDIO_PLAY("SOUND_INSERT_KEYS");
	     ev.window = win;
	     for (i = 0; i < (int)strlen(s); i++)
	       {
		  int                 j;

		  ev.x = mode.x;
		  ev.y = mode.y;
		  ev.x_root = mode.x;
		  ev.y_root = mode.y;
		  for (j = 0; j < (int)(sizeof(ks) / sizeof(struct _keyset)); j++)

		    {
		       if (!strncmp(ks[j].ch, &(s[i]), strlen(ks[j].ch)))
			 {
			    i += (strlen(ks[j].ch) - 1);
			    ev.keycode =
			       XKeysymToKeycode(disp, XStringToKeysym(ks[j].sym));
			    ev.state = ks[j].state;
			    ev.type = KeyPress;
			    XSendEvent(disp, win, False, 0, (XEvent *) & ev);
			    ev.type = KeyRelease;
			    XSendEvent(disp, win, False, 0, (XEvent *) & ev);
			    j = 0x7fffffff;
			 }
		    }
	       }
	  }
     }
   EDBUG_RETURN(0);
}

int
doCreateIconbox(void *params)
{
   EDBUG(6, "doSetPagerSnap");
   if (InZoom())
      EDBUG_RETURN(0);
   if (params)
     {
	Iconbox            *ib;

	AUDIO_PLAY("SOUND_NEW_ICONBOX");
	ib = CreateIconbox(params);
	ShowIconbox(ib);
     }
   else
     {
	Iconbox            *ib, **ibl;
	int                 num = 0;
	char                s[64];

	ibl = ListAllIconboxes(&num);
	if (ibl)
	   Efree(ibl);
	Esnprintf(s, sizeof(s), "_IB_%i", num);
	AUDIO_PLAY("SOUND_NEW_ICONBOX");
	ib = CreateIconbox(s);
	ShowIconbox(ib);
     }
   autosave();
   EDBUG_RETURN(0);
}

int
doRaiseLower(void *params)
{
   EWin               *ewin;

   EDBUG(6, "doRaiseLower");
   if (InZoom())
      EDBUG_RETURN(0);

   if (params)
      ewin = FindItem(NULL, atoi((char *)params), LIST_FINDBY_ID,
		      LIST_TYPE_EWIN);
   else
      ewin = GetFocusEwin();
   if (!ewin)
      EDBUG_RETURN(0);
   if (desks.desk[ewin->desktop].list)
     {
	if (desks.desk[0].list[0] == ewin)
	  {
	     AUDIO_PLAY("SOUND_LOWER");
	     LowerEwin(ewin);
	  }
	else
	  {
	     AUDIO_PLAY("SOUND_RAISE");
	     RaiseEwin(ewin);
	  }
     }
   EDBUG_RETURN(0);
}

int
doShowHideGroup(void *params)
{
   EWin               *ewin;

   EWin              **gwins;
   int                 i, num;
   Border             *b = NULL;

   EDBUG(6, "doShowGroup");
   if (InZoom())
      EDBUG_RETURN(0);
   if (params)
      ewin = FindItem(NULL, atoi((char *)params), LIST_FINDBY_ID,
		      LIST_TYPE_EWIN);
   else
      ewin = GetFocusEwin();

   if (!ewin)
      EDBUG_RETURN(0);

   gwins = ListWinGroupMembersForEwin(ewin, ACTION_NONE, &num);
   current_group = ewin->group;

   for (i = 0; i < num; i++)
     {
	if (!gwins[i]->previous_border)
	  {
	     if (!gwins[i]->border->group_border_name)
		continue;

	     b = (Border *) FindItem(gwins[i]->border->group_border_name, 0, LIST_FINDBY_NAME,
				     LIST_TYPE_BORDER);
	     if (!b)
		b = (Border *) FindItem("__FALLBACK_BORDER", 0, LIST_FINDBY_NAME,
					LIST_TYPE_BORDER);
	     gwins[i]->previous_border = gwins[i]->border;
	     b->ref_count++;
	  }
	else
	  {
	     b = gwins[i]->previous_border;
	     b->ref_count--;
	     gwins[i]->previous_border = NULL;
	  }

	gwins[i]->border_new = 1;
	SetEwinToBorder(gwins[i], b);
	ICCCM_MatchSize(gwins[i]);
	MoveResizeEwin(gwins[i], gwins[i]->x, gwins[i]->y, gwins[i]->client.w,
		       gwins[i]->client.h);
	RememberImportantInfoForEwin(gwins[i]);
     }
   Efree(gwins);

   EDBUG_RETURN(0);
}

int
doStartGroup(void *params)
{
   EWin               *ewin;

   EDBUG(6, "doStartGroup");
   if (InZoom())
      EDBUG_RETURN(0);
   if (params)
      ewin = FindItem(NULL, atoi((char *)params), LIST_FINDBY_ID,
		      LIST_TYPE_EWIN);
   else
      ewin = GetFocusEwin();

   if (!ewin)
      EDBUG_RETURN(0);

   BuildWindowGroup(&ewin, 1);

   EDBUG_RETURN(0);
}

int
doAddToGroup(void *params)
{
   EWin               *ewin;

   EDBUG(6, "doAddToGroup");
   if (InZoom())
      EDBUG_RETURN(0);
   if (params)
      ewin = FindItem(NULL, atoi((char *)params), LIST_FINDBY_ID,
		      LIST_TYPE_EWIN);
   else
      ewin = GetFocusEwin();

   if (!ewin)
      EDBUG_RETURN(0);

   AddEwinToGroup(ewin, current_group);

   EDBUG_RETURN(0);
}

int
doRemoveFromGroup(void *params)
{
   EWin               *ewin;

   EDBUG(6, "doRemoveFromGroup");
   if (InZoom())
      EDBUG_RETURN(0);
   if (params)
      ewin = FindItem(NULL, atoi((char *)params), LIST_FINDBY_ID,
		      LIST_TYPE_EWIN);
   else
      ewin = GetFocusEwin();

   if (!ewin)
      EDBUG_RETURN(0);

   RemoveEwinFromGroup(ewin);

   EDBUG_RETURN(0);
}

int
doBreakGroup(void *params)
{
   EWin               *ewin;

   EDBUG(6, "doBreakGroup");
   if (InZoom())
      EDBUG_RETURN(0);
   if (params)
      ewin = FindItem(NULL, atoi((char *)params), LIST_FINDBY_ID,
		      LIST_TYPE_EWIN);
   else
      ewin = GetFocusEwin();

   if (!ewin)
      EDBUG_RETURN(0);

   BreakWindowGroup(ewin);

   EDBUG_RETURN(0);
}

int
doZoom(void *params)
{
   EWin               *ewin;

   EDBUG(6, "doZoom");

   if (!(CanZoom()))
      EDBUG_RETURN(0);

   ewin = GetFocusEwin();

   if (!ewin)
      EDBUG_RETURN(0);
   if (InZoom())
      Zoom(NULL);
   else
      Zoom(ewin);
   EDBUG_RETURN(0);
   params = NULL;
}

int
initFunctionArray(void)
{
   EDBUG(5, "initFunctionArray");
   ActionFunctions[ACTION_NONE] = (int (*)(void *))(doNothing);
   ActionFunctions[ACTION_EXIT] = (int (*)(void *))(doExit);
   ActionFunctions[ACTION_EXEC] = (int (*)(void *))(execApplication);
   ActionFunctions[ACTION_ALERT] = (int (*)(void *))(alert);
   ActionFunctions[ACTION_SHOW_MENU] = (int (*)(void *))(spawnMenu);
   ActionFunctions[ACTION_HIDE_MENU] = (int (*)(void *))(hideMenu);
   ActionFunctions[ACTION_MOVE] = (int (*)(void *))(doMove);
   ActionFunctions[ACTION_RESIZE] = (int (*)(void *))(doResize);
   ActionFunctions[ACTION_RAISE] = (int (*)(void *))(doRaise);
   ActionFunctions[ACTION_LOWER] = (int (*)(void *))(doLower);
   ActionFunctions[ACTION_CLEANUP] = (int (*)(void *))(doCleanup);
   ActionFunctions[ACTION_RESIZE_H] = (int (*)(void *))(doResizeH);
   ActionFunctions[ACTION_RESIZE_V] = (int (*)(void *))(doResizeV);
   ActionFunctions[ACTION_KILL] = (int (*)(void *))(doKill);
   ActionFunctions[ACTION_KILL_NASTY] = (int (*)(void *))(doKillNasty);
   ActionFunctions[ACTION_DESKTOP_NEXT] = (int (*)(void *))(doNextDesktop);
   ActionFunctions[ACTION_DESKTOP_PREV] = (int (*)(void *))(doPrevDesktop);
   ActionFunctions[ACTION_DESKTOP_RAISE] = (int (*)(void *))(doRaiseDesktop);
   ActionFunctions[ACTION_DESKTOP_LOWER] = (int (*)(void *))(doLowerDesktop);
   ActionFunctions[ACTION_DESKTOP_DRAG] = (int (*)(void *))(doDragDesktop);
   ActionFunctions[ACTION_STICK] = (int (*)(void *))(doStick);
   ActionFunctions[ACTION_DESKTOP_INPLACE] = (int (*)(void *))(doInplaceDesktop);
   ActionFunctions[ACTION_DRAG_BUTTON] = (int (*)(void *))(doDragButtonStart);
   ActionFunctions[ACTION_FOCUSMODE_SET] = (int (*)(void *))(doFocusModeSet);
   ActionFunctions[ACTION_MOVEMODE_SET] = (int (*)(void *))(doMoveModeSet);
   ActionFunctions[ACTION_RESIZEMODE_SET] = (int (*)(void *))(doResizeModeSet);
   ActionFunctions[ACTION_SLIDEMODE_SET] = (int (*)(void *))(doSlideModeSet);
   ActionFunctions[ACTION_CLEANUPSILDE_SET] = (int (*)(void *))(doCleanupSlideSet);
   ActionFunctions[ACTION_MAPSLIDE_SET] = (int (*)(void *))(doMapSlideSet);
   ActionFunctions[ACTION_SOUND_SET] = (int (*)(void *))(doSoundSet);
   ActionFunctions[ACTION_BUTTONMOVE_RESIST_SET] = (int (*)(void *))(doButtonMoveResistSet);
   ActionFunctions[ACTION_DESKTOPBG_TIMEOUT_SET] = (int (*)(void *))(doDesktopBgTimeoutSet);
   ActionFunctions[ACTION_MAPSLIDE_SPEED_SET] = (int (*)(void *))(doMapSlideSpeedSet);
   ActionFunctions[ACTION_CLEANUPSLIDE_SPEED_SET] = (int (*)(void *))(doCleanupSlideSpeedSet);
   ActionFunctions[ACTION_DRAGDIR_SET] = (int (*)(void *))(doDragdirSet);
   ActionFunctions[ACTION_DRAGBAR_ORDER_SET] = (int (*)(void *))(doDragbarOrderSet);
   ActionFunctions[ACTION_DRAGBAR_WIDTH_SET] = (int (*)(void *))(doDragbarWidthSet);
   ActionFunctions[ACTION_DRAGBAR_LENGTH_SET] = (int (*)(void *))(doDragbarLengthSet);
   ActionFunctions[ACTION_DESKSLIDE_SET] = (int (*)(void *))(doDeskSlideSet);
   ActionFunctions[ACTION_DESKSLIDE_SPEED_SET] = (int (*)(void *))(doDeskSlideSpeedSet);
   ActionFunctions[ACTION_HIQUALITYBG_SET] = (int (*)(void *))(doHiQualityBgSet);
   ActionFunctions[ACTION_PLAYSOUNDCLASS] = (int (*)(void *))(doPlaySoundClass);
   ActionFunctions[ACTION_GOTO_DESK] = (int (*)(void *))(doGotoDesktop);
   ActionFunctions[ACTION_DESKRAY] = (int (*)(void *))(doDeskray);
   ActionFunctions[ACTION_AUTOSAVE_SET] = (int (*)(void *))(doAutosaveSet);
   ActionFunctions[ACTION_HIDESHOW_BUTTON] = (int (*)(void *))(doHideShowButton);
   ActionFunctions[ACTION_ICONIFY] = (int (*)(void *))(doIconifyWindow);
   ActionFunctions[ACTION_SLIDEOUT] = (int (*)(void *))(doSlideout);
   ActionFunctions[ACTION_SCROLL_WINDOWS] = (int (*)(void *))(doScrollWindows);
   ActionFunctions[ACTION_SHADE] = (int (*)(void *))(doShade);
   ActionFunctions[ACTION_MAX_HEIGHT] = (int (*)(void *))(doMaxH);
   ActionFunctions[ACTION_MAX_WIDTH] = (int (*)(void *))(doMaxW);
   ActionFunctions[ACTION_MAX_SIZE] = (int (*)(void *))(doMax);
   ActionFunctions[ACTION_SEND_TO_NEXT_DESK] = (int (*)(void *))(doSendToNextDesk);
   ActionFunctions[ACTION_SEND_TO_PREV_DESK] = (int (*)(void *))(doSendToPrevDesk);
   ActionFunctions[ACTION_SNAPSHOT] = (int (*)(void *))(doSnapshot);
   ActionFunctions[ACTION_SCROLL_CONTAINER] = (int (*)(void *))(doScrollContainer);
   ActionFunctions[ACTION_TOOLTIP_SET] = (int (*)(void *))(doToolTipSet);
   ActionFunctions[ACTION_FOCUS_NEXT] = (int (*)(void *))(doFocusNext);
   ActionFunctions[ACTION_FOCUS_PREV] = (int (*)(void *))(doFocusPrev);
   ActionFunctions[ACTION_FOCUS_SET] = (int (*)(void *))(doFocusSet);
   ActionFunctions[ACTION_BACKGROUND_SET] = (int (*)(void *))(doBackgroundSet);
   ActionFunctions[ACTION_AREA_SET] = (int (*)(void *))(doAreaSet);
   ActionFunctions[ACTION_MOVE_BY] = (int (*)(void *))(doAreaMoveBy);
   ActionFunctions[ACTION_TOGGLE_FIXED] = (int (*)(void *))(doToggleFixedPos);
   ActionFunctions[ACTION_SET_LAYER] = (int (*)(void *))(doSetLayer);
   ActionFunctions[ACTION_WARP_POINTER] = (int (*)(void *))(doWarpPointer);
   ActionFunctions[ACTION_MOVE_WINDOW_TO_AREA] = (int (*)(void *))(doMoveWinToArea);
   ActionFunctions[ACTION_MOVE_WINDOW_BY_AREA] = (int (*)(void *))(doMoveWinByArea);
   ActionFunctions[ACTION_SET_WINDOW_BORDER] = (int (*)(void *))(doSetWinBorder);
   ActionFunctions[ACTION_LINEAR_AREA_SET] = (int (*)(void *))(doLinearAreaSet);
   ActionFunctions[ACTION_LINEAR_MOVE_BY] = (int (*)(void *))(doLinearAreaMoveBy);
   ActionFunctions[ACTION_ABOUT] = (int (*)(void *))(doAbout);
   ActionFunctions[ACTION_FX] = (int (*)(void *))(doFX);
   ActionFunctions[ACTION_MOVE_WINDOW_TO_LINEAR_AREA] = (int (*)(void *))(doMoveWinToLinearArea);
   ActionFunctions[ACTION_MOVE_WINDOW_BY_LINEAR_AREA] = (int (*)(void *))(doMoveWinByArea);
   ActionFunctions[ACTION_SET_PAGER_HIQ] = (int (*)(void *))(doSetPagerHiq);
   ActionFunctions[ACTION_SET_PAGER_SNAP] = (int (*)(void *))(doSetPagerSnap);
   ActionFunctions[ACTION_CONFIG] = (int (*)(void *))(doConfigure);
   ActionFunctions[ACTION_MOVE_CONSTRAINED] = (int (*)(void *))(doMoveConstrained);
   ActionFunctions[ACTION_INSERT_KEYS] = (int (*)(void *))(doInsertKeys);
   ActionFunctions[ACTION_START_GROUP] = (int (*)(void *))(doStartGroup);
   ActionFunctions[ACTION_ADD_TO_GROUP] = (int (*)(void *))(doAddToGroup);
   ActionFunctions[ACTION_REMOVE_FROM_GROUP] = (int (*)(void *))(doRemoveFromGroup);
   ActionFunctions[ACTION_BREAK_GROUP] = (int (*)(void *))(doBreakGroup);
   ActionFunctions[ACTION_SHOW_HIDE_GROUP] = (int (*)(void *))(doShowHideGroup);
   ActionFunctions[ACTION_CREATE_ICONBOX] = (int (*)(void *))(doCreateIconbox);
   ActionFunctions[ACTION_RAISE_LOWER] = (int (*)(void *))(doRaiseLower);
   ActionFunctions[ACTION_ZOOM] = (int (*)(void *))(doZoom);
   EDBUG_RETURN(0);
}
