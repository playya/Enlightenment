/*
 * Copyright (C) 2000 Carsten Haitzler, Geoff Harrison and various contributors
 * *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * *
 * The above copyright notice and this permission notice shall be included in
 * all copies of the Software, its documentation and marketing & publicity
 * materials, and acknowledgment shall be given in the documentation, materials
 * and software packages that this Software was used.
 * *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "E.h"
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

#ifdef _LIBC
#include <stdint.h>
#define gettimeofday __gettimeofday
#define set_errno(e) __set_errno(e)
typedef uint64_t    big_type;

#else
#define set_errno(e) errno = (e)
typedef unsigned long big_type;

#endif

/* Generate a unique temporary file name from TEMPLATE.
 * The last six characters of TEMPLATE must be "XXXXXX";
 * they are replaced with a string that makes the filename unique.
 * Returns a file descriptor open on the file for reading and writing.  */
int
Emkstemp(char *template)
{
   static const char   letters[]
#ifndef __EMX__
      = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

#else
      = "abcdefghijklmnopqrstuvwxyz��������������������������0123456789";

#endif

   static big_type     value;
   struct timeval      tv;
   char               *XXXXXX;
   size_t              len;
   int                 count;

   len = strlen(template);
   if ((len < 6) || (strcmp(&template[len - 6], "XXXXXX")))
     {
	set_errno(EINVAL);
	return -1;
     }
   /* This is where the Xs start.  */
   XXXXXX = &template[len - 6];

   /* Get some more or less random data.  */
   gettimeofday(&tv, NULL);
   value += ((big_type) tv.tv_usec << 16) ^ tv.tv_sec ^ getpid();

   for (count = 0; count < TMP_MAX; ++count)
     {
	big_type            v = value;
	int                 fd;

	/* Fill in the random bits.  */
	XXXXXX[0] = letters[v % 62];
	v /= 62;
	XXXXXX[1] = letters[v % 62];
	v /= 62;
	XXXXXX[2] = letters[v % 62];
	v /= 62;
	XXXXXX[3] = letters[v % 62];
	v /= 62;
	XXXXXX[4] = letters[v % 62];
	v /= 62;
	XXXXXX[5] = letters[v % 62];

	fd = open(template, O_RDWR | O_CREAT | O_EXCL, 0600);
	if (fd >= 0)
	   /* The file does not exist.  */
	   return fd;

	/* This is a random value.  It is only necessary that the next
	 * TMP_MAX values generated by adding 7777 to VALUE are different
	 * with (module 2^32).  */
	value += 7777;
     }

   /* We return the null string if we can't find a unique file name.  */
   template[0] = '\0';
   return -1;
}

/*
 * This code renames the E session save files so that they all bunch together.
 * This makes it easier to identify them and clean them up. 
 * The user control config is called "~/.enlightenment/...e_session-XXXXXX"
 * The client data appends ".clients.SCREEN" onto this filename and the
 * snapshot data appends ".snapshots.SCREEN".
 * Each SM requested save replaces the XXXXXXs with random characters to 
 * create a unique name (this is necessary because, for all that we know, the
 * SM might want to keep the earlier save files). The SM discards old copies.
 */

static char        *
default_save_prefix(void)
{
   static char        *def_prefix = NULL;

   if (!def_prefix)
     {
	char                s[1024];

	Esnprintf(s, sizeof(s), "%s/...e_session-XXXXXX", UserEDir());
	def_prefix = duplicate(s);
     }
   return def_prefix;
}

/* holders for command line options */
static char        *sm_file = NULL;
static char        *userthemepath;

/* The saved window details */
static int          num_match = 0;
typedef struct _match
{
   char               *session_id;
   char               *name;
   char               *class;
   char               *role;
   char               *command;
   char                used;
   int                 x, y, w, h, desktop, iconified, shaded, sticky, layer;
}
Match;

Match              *matches = NULL;

void
SetSMUserThemePath(char *path)
{
   userthemepath = duplicate(path);
}

/* Used by multiheaded child processes to identify when they have
 * recieved the new sm_file value from the master_pid process */
static int          stale_sm_file = 0;

void
SetSMFile(char *path)
{
   if (sm_file)
      Efree(sm_file);
   if (!path)
      sm_file = duplicate(default_save_prefix());
   else
      sm_file = duplicate(path);
   stale_sm_file = 0;
}

char               *
GetSMFile(void)
{
   return sm_file;
}

char               *
GetGenericSMFile(void)
{
   return default_save_prefix();
}

/* This code covers X11R6 and X11R5 clients (xterm, KDE, ...). */
static void
SaveWindowStates(void)
{
   EWin              **lst, *ewin;
   int                 i, num, x, y;
   FILE               *f;
   char                s[4096], ss[4096];

   lst = (EWin **) ListItemType(&num, LIST_TYPE_EWIN);
   if (lst)
     {
	Etmp(s);
	f = fopen(s, "w");
	if (f)
	  {
	     for (i = 0; i < num; i++)
	       {
		  ewin = lst[i];
		  if ((!(ewin->internal)) &&
		      ((ewin->client.command) || (ewin->session_id)))
		    {
		       x = 0;
		       y = 0;
		       if (!ewin->sticky)
			 {
			    x =
			       desks.desk[ewin->desktop].current_area_x *
			       root.w;
			    y =
			       desks.desk[ewin->desktop].current_area_y *
			       root.h;
			 }
		       fprintf(f, "[CLIENT] %i %i %i %i %i %i %i %i %i\n",
			       ewin->x + x, ewin->y + y,
			       ewin->client.w, ewin->client.h,
			       ewin->desktop, ewin->iconified, ewin->shaded,
			       ewin->sticky, ewin->layer);
		       if (ewin->session_id)
			  fprintf(f, "  [SESSION_ID] %s\n", ewin->session_id);
		       if (ewin->client.name)
			  fprintf(f, "  [NAME] %s\n", ewin->client.name);
		       if (ewin->client.class)
			  fprintf(f, "  [CLASS] %s\n", ewin->client.class);
		       if (ewin->client.role)
			  fprintf(f, "  [ROLE] %s\n", ewin->client.role);
		       if (ewin->client.command)
			  fprintf(f, "  [COMMAND] %s\n", ewin->client.command);
		    }
	       }
	     fclose(f);
	     Esnprintf(ss, sizeof(ss), "%s.clients.%i", GetSMFile(), root.scr);
	     rm(ss);
	     mv(s, ss);
	     if (!isfile(ss))
		Alert(_("There was an error writing the clients "
			"session save file.\n"
			"You may have run out of disk "
			"space, not have permission\n"
			"to write to your filing system "
			"or other similar problems.\n"));
	  }
	Efree(lst);
     }
}

static void
LoadWindowStates(void)
{
   FILE               *f;
   char                s[4096], s1[4096];

   Esnprintf(s, sizeof(s), "%s.clients.%i", GetSMFile(), root.scr);
#ifndef __EMX__
   f = fopen(s, "r");
#else
   f = fopen(s, "rt");
#endif
   if (f)
     {
	while (fgets(s, sizeof(s), f))
	  {
	     s[strlen(s) - 1] = 0;
	     sscanf(s, "%4000s", s1);
	     if (!strcmp(s1, "[CLIENT]"))
	       {
		  num_match++;
		  matches = Erealloc(matches, sizeof(Match) * num_match);
		  matches[num_match - 1].session_id = NULL;
		  matches[num_match - 1].name = NULL;
		  matches[num_match - 1].class = NULL;
		  matches[num_match - 1].role = NULL;
		  matches[num_match - 1].command = NULL;
		  matches[num_match - 1].used = 0;
		  sscanf(s, "%*s %i %i %i %i %i %i %i %i %i",
			 &(matches[num_match - 1].x),
			 &(matches[num_match - 1].y),
			 &(matches[num_match - 1].w),
			 &(matches[num_match - 1].h),
			 &(matches[num_match - 1].desktop),
			 &(matches[num_match - 1].iconified),
			 &(matches[num_match - 1].shaded),
			 &(matches[num_match - 1].sticky),
			 &(matches[num_match - 1].layer));
	       }
	     else if (!strcmp(s1, "[SESSION_ID]"))
	       {
		  matches[num_match - 1].session_id = duplicate(atword(s, 2));
	       }
	     else if (!strcmp(s1, "[NAME]"))
	       {
		  matches[num_match - 1].name = duplicate(atword(s, 2));
	       }
	     else if (!strcmp(s1, "[CLASS]"))
	       {
		  matches[num_match - 1].class = duplicate(atword(s, 2));
	       }
	     else if (!strcmp(s1, "[ROLE]"))
	       {
		  matches[num_match - 1].role = duplicate(atword(s, 2));
		  /* Needed for matching X11R5 clients */
	       }
	     else if (!strcmp(s1, "[COMMAND]"))
	       {
		  matches[num_match - 1].command = duplicate(atword(s, 2));
	       }
	  }
	fclose(f);
     }
}

/* These matching rules try to cover everyone with minimal work done
 * for clients that actually comply with the X11R6 ICCCM. */
void
MatchEwinToSM(EWin * ewin)
{
   int                 i;

   if (GetSMfd() < 0)
      return;
   for (i = 0; i < num_match; i++)
     {
	if ((!matches[i].used))
	  {
	     /* No match unless both have or both lack a session_id */
	     if (!ewin->session_id)
	       {
		  if (matches[i].session_id)
		     continue;
	       }
	     if (ewin->session_id)
	       {
		  if (!matches[i].session_id)
		     continue;
	       }
	     if ((ewin->session_id))
	       {
		  /* The X11R6 protocol guarantees matching session_ids */
		  if (strcmp(ewin->session_id, matches[i].session_id))
		     continue;
	       }
	     else
	       {
		  /* The X11R5 protocol was based around the WM_COMMAND
		   * property which should be preserved over sessions
		   * by compliant apps.
		   * 
		   * FIXME: Mozilla DELETES the WM_COMMAND property on 
		   * a regular basis so is is wise NOT to update 
		   * this property when it is set to NULL. */
		  if ((ewin->client.command) && (matches[i].command) &&
		      strcmp(ewin->client.command, matches[i].command))
		     continue;
	       }

	     if ((ewin->client.role) && (matches[i].role))
	       {
		  /* The X11R6 protocol guarantees that any WM_WINDOW_ROLE
		   * is unique among the windows sharing a SM_CLIENT_ID.
		   * 
		   * Clients which use the same WM_WINDOW_ROLE on two 
		   * windows are breaking the ICCCM even if they have 
		   * different WM_CLASS properties on those windows. */
		  if (strcmp(ewin->client.role, matches[i].role))
		     continue;
	       }
	     else
	       {
		  /* The WM_CLASS is a stable basis for a test. */
		  if ((ewin->client.class) && (matches[i].class) &&
		      (strcmp(ewin->client.class, matches[i].class)))
		     continue;
		  if ((ewin->client.name) && (matches[i].name) &&
		      (strcmp(ewin->client.name, matches[i].name)))
		     continue;

		  /* Twm also matches on the WM_NAME but only when this value
		   * has not changed since the window was first mapped.
		   * This seems a bit kludgy to me. (: */
	       }

	     if (!ICCCM_GetEInfo(ewin))
	       {
		  matches[i].used = 1;
		  ewin->client.already_placed = 1;
		  ewin->iconified = matches[i].iconified;
		  ewin->sticky = matches[i].sticky;
		  ewin->shaded = matches[i].shaded;
		  ewin->layer = matches[i].layer;
		  if (!ewin->sticky)
		     ewin->desktop = matches[i].desktop;
		  /* if it's NOT (X11R6 and already placed by the client) */
		  if (!((ewin->client.already_placed) && (ewin->session_id)))
		    {
		       ewin->client.x = matches[i].x -
			  (desks.desk[ewin->desktop].current_area_x * root.w);
		       ewin->client.y = matches[i].y -
			  (desks.desk[ewin->desktop].current_area_y * root.h);
		       ewin->client.w = matches[i].w;
		       ewin->client.h = matches[i].h;
		       EMoveResizeWindow(disp, ewin->client.win,
					 ewin->client.x, ewin->client.y,
					 ewin->client.w, ewin->client.h);
		    }
	       }
	     break;
	  }
     }
}

/* Used when X11R6 session management is not available. */
void
autosave(void)
{
   if (mode.startup)
      return;
   if (mode.autosave)
     {
	char                s[4096];

	SaveSnapInfo();
	Etmp(s);
	SaveUserControlConfig(fopen(s, "w"));
	rm(GetGenericSMFile());
	mv(s, GetGenericSMFile());
	if (!isfile(GetGenericSMFile()))
	   Alert(_
		 ("There was an error saving your autosave data - filing\n"
		  "system problems.\n"));
/*      
 * if (strcmp(GetSMFile(), GetGenericSMFile()))
 * {
 * if (exists(GetGenericSMFile()))
 * rm(GetGenericSMFile());
 * symlink(GetSMFile(), GetGenericSMFile());
 * }
 */
     }
   else
     {
/*      char                buf[1024];
 * 
 * * * Esnprintf(buf, sizeof(buf) / sizeof(char), "rm %s*", GetSMFile());
 * * * 
 * * * system(buf); */
	rm(GetGenericSMFile());
     }
}

#ifndef HAVE_SM
static void         LogoutCB(int val, void *data);

void
SessionInit(void)
{
   int                 i;

   LoadWindowStates();
}

void
ProcessICEMSGS(void)
{
}

int
GetSMfd(void)
{
   return -1;
}

void
SessionGetInfo(EWin * ewin, Atom atom_change)
{
}

void
SetSMID(char *smid)
{
}

void
SaveSession(int shutdown)
{
   /* don't need */
   /* autosave(); */
}

static void
LogoutCB(int val, void *data)
{
   doSMExit(NULL);
   val = 0;
   data = NULL;
}

static void         CB_SettingsEscape(int val, void *data);
static void
CB_SettingsEscape(int val, void *data)
{
   DialogClose((Dialog *) data);
   val = 0;
}

/* This is the original code from actions.c(doExit). */
void
doSMExit(void *params)
{
   char                s[1024];
   char               *real_exec;
   char                sss[FILEPATH_LEN_MAX];
   Window              w;
   char                master_flag, do_master_kill;

   master_flag = (master_pid == getpid())? 1 : 0;

   do_master_kill = 1;

   if (!params)
      SaveSession(1);
   if ((disp) && ((!params) || ((params) && strcmp((char *)params, "logout"))))
      SetEInfoOnAll();

   if (params)
     {
	SoundExit();
#ifndef __EMX__
	setsid();
#endif
	sscanf(params, "%1000s", s);
	if (mustdel)
	  {
#ifndef __EMX__
	     Esnprintf(sss, sizeof(sss), "/bin/rm -rf %s", themepath);
#else
	     Esnprintf(sss, sizeof(sss), "rm.exe -rf %s", themepath);
#endif
	     system(sss);
	  }
	if (!strcmp(s, "restart"))
	  {
	     AUDIO_PLAY("SOUND_WAIT");
	     if (sound_fd >= 0)
		close(sound_fd);
	     w = MakeExtInitWin();
	     XCloseDisplay(disp);
	     disp = NULL;

	     if (themepath[0] != 0)
	       {
		  Esnprintf(sss, sizeof(sss),
			    "exec %s -single -ext_init_win %i -theme %s "
			    "-econfdir %s -ecachedir %s -display %s",
			    command, w, themename,
			    UserEDir(), UserCacheDir(), dstr);
		  execl(DEFAULT_SH_PATH, DEFAULT_SH_PATH, "-c", sss, NULL);
	       }
	     else
	       {
		  Esnprintf(sss, sizeof(sss),
			    "exec %s -single -ext_init_win %i "
			    "-econfdir %s -ecachedir %s -display %s",
			    command, w, UserEDir(), UserCacheDir(), dstr);
		  execl(DEFAULT_SH_PATH, DEFAULT_SH_PATH, "-c", sss, NULL);
	       }
	  }
	else if (!strcmp(s, "restart_theme"))
	  {
	     AUDIO_PLAY("SOUND_WAIT");
	     if (sound_fd >= 0)
		close(sound_fd);
	     w = MakeExtInitWin();
	     XCloseDisplay(disp);
	     disp = NULL;
	     sscanf(params, "%*s %1000s", s);
	     Esnprintf(sss, sizeof(sss),
		       "exec %s -single -ext_init_win %i -theme %s "
		       "-econfdir %s -ecachedir %s -display %s",
		       command, w, s, UserEDir(), UserCacheDir(), dstr);
	     execl(DEFAULT_SH_PATH, DEFAULT_SH_PATH, "-c", sss, NULL);
	  }
	else if (!strcmp(s, "restart_wm"))
	  {
	     /* kill off kde */
	     if (mode.kde_support)
		KDE_Shutdown();
	     AUDIO_PLAY("SOUND_EXIT");
	     if (sound_fd >= 0)
		close(sound_fd);
	     XCloseDisplay(disp);
	     disp = NULL;
	     if (atword(params, 2))
		strncpy(s, atword(params, 2), 1000);
	     real_exec = (char *)Emalloc(strlen(s) + 6);
	     sprintf(real_exec, "exec %s "
		     "-econfdir %s -ecachedir %s -display %s",
		     s, UserEDir(), UserCacheDir(), dstr);
	     execl(DEFAULT_SH_PATH, DEFAULT_SH_PATH, "-c", "exec",
		   real_exec, NULL);
	  }
	else if (!strcmp(s, "logout"))
	  {
	     Dialog             *d;
	     EWin               *ewin;

	     if (!(d = FindItem("LOGOUT_DIALOG", 0, LIST_FINDBY_NAME,
				LIST_TYPE_DIALOG)))
	       {
		  AUDIO_PLAY("SOUND_LOGOUT");
		  d = CreateDialog("LOGOUT_DIALOG");
		  DialogSetTitle(d, "Are you sure?");
		  DialogSetText(d,
				"\n"
				"\n"
				"    Are you sure you wish to log out ?    \n"
				"\n" "\n");
		  DialogAddButton(d, "  Yes, Log Out  ", LogoutCB, 1);
		  DialogAddButton(d, "  No  ", NULL, 1);
		  DialogBindKey(d, "Escape", CB_SettingsEscape, 0, d);
		  DialogBindKey(d, "Return", LogoutCB, 0, d);
	       }
	     ShowDialog(d);
	     ewin = FindEwinByDialog(d);
	     if (ewin)
	       {
#ifdef HAS_XINERAMA
		  if (xinerama_active)
		    {
		       Window              rt, ch;
		       XineramaScreenInfo *screens;
		       int                 pointer_x, pointer_y;
		       int                 num;
		       int                 d;
		       unsigned int        ud;

		       XQueryPointer(disp, root.win, &rt, &ch, &pointer_x,
				     &pointer_y, &d, &d, &ud);
		       screens = XineramaQueryScreens(disp, &num);
		       for (i = 0; i < num; i++)
			 {
			    if (pointer_x >= screens[i].x_org)
			      {
				 if (pointer_x <=
				     (screens[i].width + screens[i].x_org))
				   {
				      if (pointer_y >= screens[i].y_org)
					{
					   if (pointer_y <= (screens[i].height +
							     screens[i].y_org))
					     {
						ewin->x =
						   ((screens[i].width - ewin->w)
						    / 2) + screens[i].x_org;
						ewin->y =
						   ((screens
						     [i].height -
						     ewin->h) / 2) +
						   screens[i].y_org;
					     }
					}
				   }
			      }
			 }
		       XFree(screens);
		       MoveEwin(ewin, ewin->x, ewin->y);
		       FocusToEWin(ewin);
		    }
		  else
		    {
#endif
		       MoveEwin(ewin, ((root.w - (ewin->w)) / 2),
				((root.h - (ewin->h)) / 2));
		       FocusToEWin(ewin);
#ifdef HAS_XINERAMA
		    }
#endif
	       }
	     return;
	  }
     }
   /* kill off kde */
   if (mode.kde_support)
      KDE_Shutdown();
   AUDIO_PLAY("SOUND_EXIT");
   EExit(0);
}
#else /* HAVE_SM */
#include <X11/SM/SMlib.h>

static char        *sm_client_id = NULL;
static SmcConn      sm_conn = NULL;
static int          sm_fd = -1;

/* True if we are saving state for a doExit("restart") */
static int          restarting = False;

static void         LogoutCB(int val, void *data);
static void         set_save_props(SmcConn smc_conn, int master_flag);
static void         callback_save_yourself2(SmcConn smc_conn,

					    SmPointer client_data);
static void         callback_save_yourself(SmcConn smc_conn,
					   SmPointer client_data,
					   int save_style, Bool shutdown,
					   int interact_style, Bool fast);
static void         callback_die(SmcConn smc_conn, SmPointer client_data);
static void         callback_save_complete(SmcConn smc_conn,

					   SmPointer client_data);
static void         callback_shutdown_cancelled(SmcConn smc_conn,

						SmPointer client_data);

static void
set_save_props(SmcConn smc_conn, int master_flag)
{
   char               *user = NULL;
   char               *program = NULL;
   char               *pristr = "_GSM_Priority";
   char               *smid = "-smid";
   char               *single = "-single";
   char               *smfile = "-smfile";
   char               *econfdir = "-econfdir";
   char               *e_conf_dir;
   char               *ecachedir = "-ecachedir";
   char               *e_cache_dir;
   char               *extinitwin = "-ext_init_win";
   char               *sh = "sh";
   char               *c = "-c";
   char                buf[512];
   char                priority = 10;
   char                style;
   int                 n = 0;
   SmPropValue         programVal = { 0, NULL };
   SmPropValue         userIDVal = { 0, NULL };
   SmPropValue         discardVal[] = {
      {0, NULL},
      {0, NULL},
      {0, NULL}
   };
   SmPropValue         restartVal[] = {
      {0, NULL},
      {0, NULL},
      {0, NULL},
      {0, NULL},
      {0, NULL},
      {0, NULL},
      {0, NULL},
      {0, NULL},
      {0, NULL},
      {0, NULL}
   };
   SmPropValue         styleVal = { 0, NULL };
   SmPropValue         priorityVal = { 0, NULL };
   SmProp              programProp;
   SmProp              userIDProp;
   SmProp              discardProp;
   SmProp              restartProp;
   SmProp              cloneProp;
   SmProp              styleProp;
   SmProp              priorityProp;
   SmProp             *props[7];

   programProp.name = (char *)SmProgram;
   programProp.type = (char *)SmLISTofARRAY8;
   programProp.num_vals = 1;
   programProp.vals = &programVal;

   userIDProp.name = (char *)SmUserID;
   userIDProp.type = (char *)SmLISTofARRAY8;
   userIDProp.num_vals = 1;
   userIDProp.vals = &userIDVal;

   discardProp.name = (char *)SmDiscardCommand;
   discardProp.type = (char *)SmLISTofARRAY8;
   discardProp.num_vals = 3;
   discardProp.vals = (SmPropValue *) & discardVal;

   restartProp.name = (char *)SmRestartCommand;
   restartProp.type = (char *)SmLISTofARRAY8;
   restartProp.vals = (SmPropValue *) & restartVal;

   cloneProp.name = (char *)SmCloneCommand;
   cloneProp.type = (char *)SmLISTofARRAY8;
   cloneProp.vals = (SmPropValue *) & restartVal;

   styleProp.name = (char *)SmRestartStyleHint;
   styleProp.type = (char *)SmCARD8;
   styleProp.num_vals = 1;
   styleProp.vals = (SmPropValue *) & styleVal;

   priorityProp.name = (char *)pristr;
   priorityProp.type = (char *)SmCARD8;
   priorityProp.num_vals = 1;
   priorityProp.vals = (SmPropValue *) & priorityVal;

   props[0] = &programProp;
   props[1] = &userIDProp;
   props[2] = &discardProp;
   props[3] = &restartProp;
   props[4] = &cloneProp;
   props[5] = &styleProp;
   props[6] = &priorityProp;

   if (master_flag)
      /* Master WM restarts immediately for a doExit("restart") */
      style = restarting ? SmRestartImmediately : SmRestartIfRunning;
   else
      /* Slave WMs never restart */
      style = SmRestartNever;

   user = username(getuid());
   e_conf_dir = UserEDir();
   e_cache_dir = UserCacheDir();
   /* The SM specs state that the SmProgram should be the argument passed
    * to execve. Passing argv[0] is close enough. */
   program = command;

   userIDVal.length = strlen(user);
   userIDVal.value = user;
   programVal.length = strlen(program);
   programVal.value = program;
   styleVal.length = 1;
   styleVal.value = &style;
   priorityVal.length = 1;
   priorityVal.value = &priority;

   /* Tell session manager how to clean up our old data */
   Esnprintf(buf, sizeof(buf) / sizeof(char), "rm %s*.clients.*", sm_file);

   discardVal[0].length = strlen(sh);
   discardVal[0].value = sh;
   discardVal[1].length = strlen(c);
   discardVal[1].value = c;
   discardVal[2].length = strlen(buf);
   discardVal[2].value = buf;

   restartVal[n].length = strlen(command);
   restartVal[n++].value = command;
   if (single_screen_mode)
     {
	restartVal[n].length = strlen(single);
	restartVal[n++].value = single;
     }
   if (restarting)
     {
	Esnprintf(buf, sizeof(buf) / sizeof(char), "%i", init_win_ext);

	restartVal[n].length = strlen(extinitwin);
	restartVal[n++].value = extinitwin;
	restartVal[n].length = strlen(buf);
	restartVal[n++].value = buf;
     }
   restartVal[n].length = strlen(smfile);
   restartVal[n++].value = smfile;
   restartVal[n].length = strlen(sm_file);
   restartVal[n++].value = sm_file;
   restartVal[n].length = strlen(smid);
   restartVal[n++].value = smid;
   restartVal[n].length = strlen(sm_client_id);
   restartVal[n++].value = sm_client_id;
   restartVal[n].length = strlen(econfdir);
   restartVal[n++].value = econfdir;
   restartVal[n].length = strlen(e_conf_dir);
   restartVal[n++].value = e_conf_dir;
   restartVal[n].length = strlen(ecachedir);
   restartVal[n++].value = ecachedir;
   restartVal[n].length = strlen(e_cache_dir);
   restartVal[n++].value = e_cache_dir;

   restartProp.num_vals = n;

   /* SM specs require SmCloneCommand excludes "-smid" option */
   cloneProp.num_vals = restartProp.num_vals - 2;

   SmcSetProperties(smc_conn, sizeof(props) / sizeof(props[0]), props);
   if (user)
      Efree(user);
}

/* This function is usually exclusively devoted to saving data.
 * However, E sometimes wants to save state and exit immediately afterwards
 * so that the SM will restart it in a different theme. Therefore, we include
 * a suicide clause at the end.
 */
static void
callback_save_yourself2(SmcConn smc_conn, SmPointer client_data)
{
   char                master_flag = (master_pid == getpid());

   /* dont need anymore */
   /* autosave(); */
   if (!master_flag)
     {
	struct timeval      tv1, tv2;

	gettimeofday(&tv1, NULL);

	/* This loop should rarely be needed */
	while (stale_sm_file)
	  {
	     WaitEvent();
	     gettimeofday(&tv2, NULL);
	     if (tv2.tv_sec - tv1.tv_sec > 10)
	       {
		  SmcSaveYourselfDone(smc_conn, False);
		  return;
	       }
	  }
     }
   stale_sm_file = 1;
   set_save_props(smc_conn, master_flag);
   SmcSaveYourselfDone(smc_conn, True);
   if (restarting)
      EExit(0);
   client_data = NULL;
}

static void
callback_save_yourself(SmcConn smc_conn, SmPointer client_data,
		       int save_style, Bool shutdown, int interact_style,
		       Bool fast)
{
   if (master_pid == getpid())
     {
	char                s[4096];

/*      int                 fd; */

	Esnprintf(s, sizeof(s), "sm_file %s", default_save_prefix());
/*      fd = Emkstemp(s + 8);
 * if (fd < 0)
 * {
 * SmcSaveYourselfDone(smc_conn, False);
 * return;
 * }
 * SetSMFile(s + 8); */
	CommsBroadcastToSlaveWMs(GetGenericSMFile());
	/* dont need */
	/* autosave(); */
/*      
 * if (strcmp(GetSMFile(), GetGenericSMFile()))
 * {
 * if (exists(GetGenericSMFile()))
 * rm(GetGenericSMFile());
 * symlink(GetSMFile(), GetGenericSMFile());
 * }
 */
     }
   SaveWindowStates();
   SmcRequestSaveYourselfPhase2(smc_conn, callback_save_yourself2, NULL);
   client_data = NULL;
   save_style = 0;
   shutdown = 0;
   interact_style = 0;
   fast = 0;
}

static void
callback_die(SmcConn smc_conn, SmPointer client_data)
{
   if (master_pid == getpid())
      ApplySclass(FindItem("SOUND_EXIT", 0,
			   LIST_FINDBY_NAME, LIST_TYPE_SCLASS));
   EExit(0);
   smc_conn = 0;
   client_data = NULL;
}

static void
callback_save_complete(SmcConn smc_conn, SmPointer client_data)
{
   smc_conn = 0;
   client_data = NULL;
}

static void
callback_shutdown_cancelled(SmcConn smc_conn, SmPointer client_data)
{
   SmcSaveYourselfDone(smc_conn, False);
   client_data = NULL;
}

static Atom         atom_sm_client_id;
static Atom         atom_wm_client_leader;

static IceConn      ice_conn;

static void
ice_io_error_handler(IceConn connection)
{
   /* The less we do here the better - the default handler does an
    * exit(1) instead of closing the losing connection. */
   connection = 0;
}

void
SessionInit(void)
{
   static SmPointer    context;
   SmcCallbacks        callbacks;

   atom_sm_client_id = XInternAtom(disp, "SM_CLIENT_ID", False);
   atom_wm_client_leader = XInternAtom(disp, "WM_CLIENT_LEADER", False);

   IceSetIOErrorHandler(ice_io_error_handler);

   callbacks.save_yourself.callback = callback_save_yourself;
   callbacks.die.callback = callback_die;
   callbacks.save_complete.callback = callback_save_complete;
   callbacks.shutdown_cancelled.callback = callback_shutdown_cancelled;

   callbacks.save_yourself.client_data =
      callbacks.die.client_data =
      callbacks.save_complete.client_data =
      callbacks.shutdown_cancelled.client_data = (SmPointer) NULL;
   if (getenv("SESSION_MANAGER"))
     {
	char                error_string_ret[4096] = "";
	char               *client_id = NULL;

	if (sm_client_id)
	   client_id = duplicate(sm_client_id);
	sm_conn = SmcOpenConnection(NULL, &context, SmProtoMajor, SmProtoMinor,
				    SmcSaveYourselfProcMask | SmcDieProcMask |
				    SmcSaveCompleteProcMask |
				    SmcShutdownCancelledProcMask,
				    &callbacks, client_id, &sm_client_id,
				    4096, error_string_ret);
	if (client_id)
	   Efree(client_id);

	if (error_string_ret[0])
	   fprintf(stderr, "While connecting to session manager:\n%s.",
		   error_string_ret);
     }
   if (sm_conn)
     {
	char                style[2];
	SmPropValue         styleVal;
	SmProp              styleProp;
	SmProp             *props[1];

	style[0] = SmRestartIfRunning;
	style[1] = 0;

	styleVal.length = 1;
	styleVal.value = style;

	styleProp.name = SmRestartStyleHint;
	styleProp.type = SmCARD8;
	styleProp.num_vals = 1;
	styleProp.vals = &styleVal;

	props[0] = &styleProp;

	ice_conn = SmcGetIceConnection(sm_conn);
	sm_fd = IceConnectionNumber(ice_conn);
	/* Just in case we are a copy of E created by a doExit("restart") */
	SmcSetProperties(sm_conn, 1, props);
	fcntl(sm_fd, F_SETFD, fcntl(sm_fd, F_GETFD, 0) | FD_CLOEXEC);
     }
   stale_sm_file = 1;
   LoadWindowStates();
}

void
ProcessICEMSGS(void)
{
   IceProcessMessagesStatus status;

   if (sm_fd < 0)
      return;
   status = IceProcessMessages(ice_conn, NULL, NULL);
   if (status == IceProcessMessagesIOError)
     {
	/* Less of the hope.... E survives */
	DialogAlert(_("ERROR!\n"
		      "\n"
		      "Lost the Session Manager that was there?\n"
		      "Here here session manager... come here... want a bone?\n"
		      "Oh come now! Stop sulking! Bugger. Oh well. "
		      "Will continue without\n"
		      "a session manager.\n"
		      "\n"
		      "I'll survive somehow.\n" "\n" "\n" "... I hope.\n"));
	SmcCloseConnection(sm_conn, 0, NULL);
	sm_conn = NULL;
	sm_fd = -1;
     }
}

int
GetSMfd(void)
{
   return sm_fd;
}

void
SessionGetInfo(EWin * ewin, Atom atom_change)
{
   char               *s;
   Window             *w;
   int                 size;

   if (ewin->session_id)
     {
	Efree(ewin->session_id);
	ewin->session_id = NULL;
     }
   /* We can comply with the ICCCM because gtk is working correctly */
   if ((atom_change) && (!((atom_change == atom_sm_client_id) ||
			   (atom_change == atom_wm_client_leader))))
      return;
   w = AtomGet(ewin->client.win, atom_wm_client_leader, XA_WINDOW, &size);
   if (w)
     {
	s = AtomGet(*w, atom_sm_client_id, XA_STRING, &size);
	if (s)
	  {
	     ewin->session_id = Emalloc(size + 1);
	     memcpy(ewin->session_id, s, size);
	     ewin->session_id[size] = 0;
	     Efree(w);
	     Efree(s);
	     return;
	  }
	Efree(w);
     }
}

void
SetSMID(char *smid)
{
   sm_client_id = smid;
}

static void
LogoutCB(int val, void *data)
{
   if (sm_conn)
     {
	SmcRequestSaveYourself(sm_conn, SmSaveBoth, True,
			       SmInteractStyleAny, False, True);
     }
   else
     {
	doSMExit(NULL);
     }
   val = 0;
   data = NULL;
}

void
SaveSession(int shutdown)
{
   /* dont' need anymore */
   /* ayosave(); */
   if (shutdown && sm_conn)
     {
	SmcCloseConnection(sm_conn, 0, NULL);
	sm_conn = NULL;
	sm_fd = -1;
     }
}

static void         CB_SettingsEscape(int val, void *data);
static void
CB_SettingsEscape(int val, void *data)
{
   DialogClose((Dialog *) data);
   val = 0;
}

/*
 * Normally, the SM will throw away all the session data for a client
 * that breaks its connection unexpectedly. In order to avoid this we 
 * have to let the SM handle the restart (by setting a SmRestartStyleHint
 * of SmRestartImmediately). Rather than forcing all SM clients to do a
 * checkpoint (which would be a bit cleaner) we just save our own state
 * and then restore it on restart. We grab X input via the ext_init_win
 * so the our clients remain frozen while we are down.
 */
void
doSMExit(void *params)
{
   char                s[1024];
   char                master_flag, do_master_kill;

   master_flag = (master_pid == getpid())? 1 : 0;
   do_master_kill = 1;

   restarting = True;

   s[0] = 0;
   if (params)
      sscanf(params, "%1000s", s);

   if (!params)
      SaveSession(1);
   if ((disp) && ((!params) || ((params) && strcmp((char *)params, "logout"))))
      SetEInfoOnAll();
   if ((!params) || (!strcmp((char *)s, "exit")))
     {
	callback_die(sm_conn, NULL);
     }
   else if (!strcmp(s, "logout"))
     {
	Dialog             *d;
	EWin               *ewin;

	if (!(d = FindItem("LOGOUT_DIALOG", 0, LIST_FINDBY_NAME,
			   LIST_TYPE_DIALOG)))
	  {
	     AUDIO_PLAY("SOUND_LOGOUT");
	     d = CreateDialog("LOGOUT_DIALOG");
	     DialogSetTitle(d, _("Are you sure?"));
	     DialogSetText(d,
			   _("\n"
			     "\n"
			     "    Are you sure you wish to log out ?    \n"
			     "\n" "\n"));
	     DialogAddButton(d, _("  Yes, Log Out  "), LogoutCB, 1);
	     DialogAddButton(d, _("  No  "), NULL, 1);
	     DialogBindKey(d, "Escape", CB_SettingsEscape, 1, d);
	     DialogBindKey(d, "Return", LogoutCB, 0, d);
	  }
	ShowDialog(d);
	ewin = FindEwinByDialog(d);
	if (ewin)
	  {
#ifdef HAS_XINERAMA
	     if (xinerama_active)
	       {
		  Window              rt, ch;
		  XineramaScreenInfo *screens;
		  int                 pointer_x, pointer_y;
		  int                 num;
		  int                 d;
		  int                 i;
		  unsigned int        ud;

		  XQueryPointer(disp, root.win, &rt, &ch, &pointer_x,
				&pointer_y, &d, &d, &ud);
		  screens = XineramaQueryScreens(disp, &num);
		  for (i = 0; i < num; i++)
		    {
		       if (pointer_x >= screens[i].x_org)
			 {
			    if (pointer_x <=
				(screens[i].width + screens[i].x_org))
			      {
				 if (pointer_y >= screens[i].y_org)
				   {
				      if (pointer_y <= (screens[i].height +
							screens[i].y_org))
					{
					   ewin->x =
					      ((screens[i].width - ewin->w) /
					       2) + screens[i].x_org;
					   ewin->y =
					      ((screens[i].height - ewin->h) /
					       2) + screens[i].y_org;
					}
				   }
			      }
			 }
		    }
		  XFree(screens);
		  MoveEwin(ewin, ewin->x, ewin->y);
		  FocusToEWin(ewin);
	       }
	     else
	       {
#endif

		  MoveEwin(ewin, ((root.w - (ewin->w)) / 2),
			   ((root.h - (ewin->h)) / 2));
		  FocusToEWin(ewin);
#ifdef HAS_XINERAMA
	       }
#endif

	  }
	return;
     }
   else if (!strcmp(s, "restart_wm"))
     {
	AUDIO_PLAY("SOUND_WAIT");
	/* kill off kde */
	if (mode.kde_support)
	   KDE_Shutdown();
	XCloseDisplay(disp);
	disp = NULL;
	Esnprintf(s, sizeof(s), "exec %s -display %s", atword(params, 2), dstr);
	execl(DEFAULT_SH_PATH, DEFAULT_SH_PATH, "-c", s, NULL);
     }
   else if (!strcmp(s, "restart"))
     {
	AUDIO_PLAY("SOUND_WAIT");
	if (disp)
	   init_win_ext = MakeExtInitWin();

	if (disp)
	  {
	     XCloseDisplay(disp);
	     disp = NULL;
	  }
	if (themepath[0] != 0)
	  {
	     if (sm_client_id)
		Esnprintf(s, sizeof(s),
			  "exec %s -single -ext_init_win %i -theme %s "
			  "-econfdir %s -ecachedir %s "
			  "-smfile %s -smid %s -display %s",
			  command, init_win_ext, themename,
			  UserEDir(), UserCacheDir(),
			  sm_file, sm_client_id, dstr);
	     else
		Esnprintf(s, sizeof(s),
			  "exec %s -single -ext_init_win %i -theme %s "
			  "-econfdir %s -ecachedir %s "
			  "-smfile %s -display %s",
			  command, init_win_ext, themename,
			  UserEDir(), UserCacheDir(), sm_file, dstr);
	     execl(DEFAULT_SH_PATH, DEFAULT_SH_PATH, "-c", s, NULL);
	  }
	else
	  {
	     if (sm_client_id)
		Esnprintf(s, sizeof(s),
			  "exec %s -single -ext_init_win %i "
			  "-econfdir %s -ecachedir %s "
			  "-smfile %s -smid %s -display %s",
			  command, init_win_ext,
			  UserEDir(), UserCacheDir(),
			  sm_file, sm_client_id, dstr);
	     else
		Esnprintf(s, sizeof(s),
			  "exec %s -single -ext_init_win %i"
			  "-econfdir %s -ecachedir %s "
			  "-smfile %s -display %s",
			  command, init_win_ext,
			  UserEDir(), UserCacheDir(), sm_file, dstr);
	     execl(DEFAULT_SH_PATH, DEFAULT_SH_PATH, "-c", s, NULL);
	  }
     }
   else if (!strcmp((char *)s, "restart_theme"))
     {
	AUDIO_PLAY("SOUND_WAIT");
	init_win_ext = MakeExtInitWin();
	if (atword(params, 1) && strlen((char *)params) < 1024)
	  {
	     sscanf(params, "%*s %1000s", s);
	     SetSMUserThemePath(s);
	  }
	if (disp)
	  {
	     XCloseDisplay(disp);
	     disp = NULL;
	  }
	if (sm_client_id)
	   Esnprintf(s, sizeof(s),
		     "exec %s -single -ext_init_win %i -theme %s "
		     "-econfdir %s -ecachedir %s "
		     "-smfile %s -smid %s -display %s",
		     command, init_win_ext, userthemepath,
		     UserEDir(), UserCacheDir(), sm_file, sm_client_id, dstr);
	else
	   Esnprintf(s, sizeof(s),
		     "exec %s -ext_init_win %i -theme %s "
		     "-econfdir %s -ecachedir %s "
		     "-smfile %s -single -display %s",
		     command, init_win_ext, userthemepath,
		     UserEDir(), UserCacheDir(), sm_file, dstr);
	execl(DEFAULT_SH_PATH, DEFAULT_SH_PATH, "-c", s, NULL);
     }
   else if (!strcmp((char *)s, "error"))
      EExit(0);

   /* kill off kde */
   if (mode.kde_support)
      KDE_Shutdown();
   restarting = False;
   SaveSession(1);
   AUDIO_PLAY("SOUND_EXIT");
   EExit(0);
}

#endif /* HAVE_SM */
