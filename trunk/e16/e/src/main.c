#include "E.h"
#include "timestamp.h"

int
main(int argc, char **argv)
{
   int                 i, num;
   Button            **lst;
   Background         *bg;
   ECursor            *ec = NULL;

   /* This function runs all the setup for startup, and then 
    * proceeds into the primary event loop at the end.
    */

   single_screen_mode = 0;
/*  unsetenv("LD_PRELOAD"); */

#ifdef DEBUG
   call_level = 0;
   debug_level = 0;
   {
      char               *debug_str;

      debug_str = getenv("EDBUG");
      if (debug_str)
	 debug_level = atoi(debug_str);
   }
#endif

   EDBUG(1, "main");

   /* go head and set up the internal data lists that enlightenment
    * uses for finding everything
    */
   lists = Emalloc(sizeof(List) * LIST_TYPE_COUNT);
   lists = memset(lists, 0, (sizeof(List) * LIST_TYPE_COUNT));

   /* Set up all the text bits that belong on the GSOD */
   AssignTitleText("Enlightenment Message Dialog");
   AssignIgnoreText("Ignore this");
   AssignRestartText("Restart Enlightenment");
   AssignExitText("Quit Enlightenment");

   /* We'll set up what the buttons do now, too */
   AssignRestartFunction(doExit, "restart");
   AssignExitFunction(doExit, NULL);
   srand(time(NULL));

   command = duplicate(argv[0]);
   themepath[0] = 0;
   {
      int                 j = 0;

      /* Set a default location for the "previous session" data when
       * we do not actually have a previous session. */
      SetSMFile(NULL);

      /* Now we're going to interpret any of the commandline parameters
       * that are passed to it -- Well, at least the ones that we
       * understand.
       */

      for (j = 1; j < argc; j++)
	{
	   if ((!strcmp("-theme", argv[j])) && (argc - j > 1))
	     {
		Esnprintf(themepath, sizeof(themepath), "%s", argv[++j]);
	     }
	   else if ((!strcmp("-econfdir", argv[j])) && (argc - j > 1))
	     {
		SetEDir(argv[++j]);
	     }
	   else if (!strcmp("-single", argv[j]))
	     {
		single_screen_mode = 1;
	     }
	   else if ((!strcmp("-smid", argv[j])) && (argc - j > 1))
	     {
		SetSMID(argv[++j]);
	     }
	   else if ((!strcmp("-clientId", argv[j])) && (argc - j > 1))
	     {
		SetSMID(argv[++j]);
	     }
	   else if ((!strcmp("--sm-client-id", argv[j])) && (argc - j > 1))
	     {
		SetSMID(argv[++j]);
	     }
	   else if ((!strcmp("-smfile", argv[j])) && (argc - j > 1))
	     {
		SetSMFile(argv[++j]);
	     }
	   else if ((!strcmp("-ext_init_win", argv[j])) && (argc - j > 1))
	     {
		init_win_ext = atoi(argv[++j]);
	     }
	   else if (!strcmp("-no_overwrite", argv[j]))
	     {
		no_overwrite = 1;
	     }
	   else if (!strcmp("-help", argv[j]))
	     {
		printf("enlightenment options:                      \n"
		       "\t-theme /path/to/theme                     \n"
		       "\t-econfdir /path/to/.enlightenment/conf/dir\n"
		       "\t[-smid | -clientId | --sm-client-id] id   \n"
		       "\t-smfile file                              \n"
		       "\t-ext_init_win window_id                   \n"
		       "\t-no_overwrite                             \n"
		       "\t[-v | -version | --version]               \n");
		exit(0);
	     }
	   else if ((!strcmp("-v", argv[j])) ||
		    (!strcmp("-version", argv[j])) ||
		    (!strcmp("--version", argv[j])) ||
		    (!strcmp("-v", argv[j])))
	     {
		printf("Enlightenment Version: %s\nLast updated on: %s\n",
		       ENLIGHTENMENT_VERSION, E_CHECKOUT_DATE);
		exit(0);
	     }
	}
   }

   if (themepath[0] == 0)
     {
	FILE               *f;
	char                s[FILEPATH_LEN_MAX];
	char               *file;

	file = FindFile("user_theme.cfg");
	if (file)
	  {
	     s[0] = 0;
	     f = fopen(file, "r");
	     if (f)
	       {
		  if (fscanf(f, "%4000s", s) < 1)
		     s[0] = 0;
		  fclose(f);
		  if (s[0])
		     Esnprintf(themepath, sizeof(themepath), "%s", s);
	       }
	     Efree(file);
	  }

	if (themepath[0] == 0)
	  {
	     char               *def;

	     def = GetDefaultTheme();
	     if (def)
	       {
		  Esnprintf(themepath, sizeof(themepath), "%s", def);
		  Efree(def);
	       }
	  }
     }
   SetSMUserThemePath(themepath);

   /* run most of the setup */
   SetupSignals();
   SetupX();
   ZoomInit();
   SetupDirs();
/*  SC_Init();
 * SC_SetHotspot(30, 30);
 * SC_SetWait(); */
   InitDesktopBgs();
   GotoDesktop(0);
   CommsSetup();
   CommsFindCommsWindow();
   GrabX();
   LoadGroups();
   LoadSnapInfo();
   MapUnmap(0);

   /* make all of our fallback classes */
   SetupFallbackClasses();
   UngrabX();
   /* We'll set up what the buttons do now, too */
   /* again?  why are we doing this twice? */
   AssignRestartFunction((*doExit), duplicate("restart"));
   AssignExitFunction((*EExit), (void *)1);
   desks.desk[0].viewable = 0;
   /* now we're going to load the configuration/theme */
   LoadEConfig(themepath);

   desks.desk[0].viewable = 1;
   RefreshDesktop(0);
   if (mode.sound)
     {
	ApplySclass(FindItem("SOUND_STARTUP", 0,
			     LIST_FINDBY_NAME, LIST_TYPE_SCLASS));
	DestroySclass(RemoveItem("SOUND_STARTUP", 0,
				 LIST_FINDBY_NAME, LIST_TYPE_SCLASS));
     }
   /* toss down the dragbar and related */
   InitDesktopControls();
   /* then draw all the buttons that belong on the desktop */
   lst = (Button **) ListItemTypeID(&num, LIST_TYPE_BUTTON, 0);
   if (lst)
     {
	for (i = 0; i < num; i++)
	  {
	     printf("%i / %i: %s\n", i, num, lst[i]->name);
	     if ((!lst[i]->internal) && (lst[i]->default_show))
		SimpleShowButton(lst[i]);
	  }
	Efree(lst);
     }
   /* gnome hints stuff & session initialization here */
   GNOME_SetHints();
   SessionInit();
   ShowDesktopControls();
   CheckEvent();
   /* retreive stuff from last time we were loaded if we're restarting */
   ICCCM_GetMainEInfo();
   SetupEnv();
   if (mode.mapslide)
      CreateStartupDisplay(0);
   MapUnmap(1);
   /* set some more stuff for gnome */
   GNOME_SetCurrentArea();
   /* if we didn't have an external window piped to us, we'll do some stuff */
   if (!init_win_ext)
      SpawnSnappedCmds();
   desks.current = 0;
   /* Set up the internal pagers */
   IB_Setup();
   if (mode.show_pagers)
     {
	mode.show_pagers = 0;
	queue_up = 0;
	EnableAllPagers();
	queue_up = 1;
     }
   if (getpid() == master_pid && init_win_ext)
     {
	XKillClient(disp, init_win_ext);
	init_win_ext = 0;
     }
   GNOME_SetClientList();

   /* start up any kde crap we might need to start up */
   if (mode.kde_support)
      KDE_Init();

   /* sync just to make sure */
   XSync(disp, False);
   queue_up = 1;

   /* hello!  we don't have a resizemode of 5! */
   if (mode.resizemode == 5)
      mode.resizemode = 0;
   /* of course, we have to set the cursors */
   ec = FindItem("DEFAULT", 0, LIST_FINDBY_NAME, LIST_TYPE_ECURSOR);
   if (ec)
     {
	ApplyECursor(root.win, ec);
	ec->ref_count++;
	ec->inroot = 1;
     }

   if (mode.display_warp < 0)
      mode.display_warp = 0;
   mode.startup = 0;
   /*  SC_Kill(); */
   /* ok - paranoia - save current settings to disk */
   if (root.scr == 0)
      autosave();
   /* let's make sure we set this up and go to our desk anyways */
   ICCCM_GetMainEInfo();
   GotoDesktop(desks.current);
   if (desks.current < (ENLIGHTENMENT_CONF_NUM_DESKTOPS - 1))
     {
	char                ps = 0;

	if (!mode.mapslide)
	  {
	     ps = desks.slidein;
	     desks.slidein = 0;
	  }
	GotoDesktop(desks.current + 1);
	GotoDesktop(desks.current - 1);
	if (!mode.mapslide)
	   desks.slidein = ps;
     }
   else if (desks.current > 0)
     {
	char                ps = 0;

	if (!mode.mapslide)
	  {
	     ps = desks.slidein;
	     desks.slidein = 0;
	  }
	GotoDesktop(desks.current - 1);
	GotoDesktop(desks.current + 1);
	if (!mode.mapslide)
	   desks.slidein = ps;
     }
   XSync(disp, False);
   if (!mode.mapslide)
      CreateStartupDisplay(0);
   if ((bg = RemoveItem("STARTUP_BACKGROUND_SIDEWAYS", 0, LIST_FINDBY_NAME,
			LIST_TYPE_BACKGROUND)))
      FreeDesktopBG(bg);
   if ((bg = RemoveItem("STARTUP_BACKGROUND", 0, LIST_FINDBY_NAME,
			LIST_TYPE_BACKGROUND)))
      FreeDesktopBG(bg);
   for (i = 0; i < child_count; i++)
      kill(e_children[i], SIGCONT);

   /* The primary event loop */
   for (;;)
      WaitEvent();
   /* Of course, we should NEVER get to this point */
   EDBUG_RETURN(0);
}
