/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"
#include "config.h"

/* local subsystem functions */
static void _e_main_shutdown_push(int (*func)(void));
static void _e_main_shutdown(int errorcode);

static int  _e_main_x_shutdown(void);
static int  _e_main_dirs_init(void);
static int  _e_main_dirs_shutdown(void);
static int  _e_main_screens_init(void);
static int  _e_main_screens_shutdown(void);
static int  _e_main_path_init(void);
static int  _e_main_path_shutdown(void);
static int  _e_main_ipc_init(void);
static int  _e_main_ipc_shutdown(void);

static void _e_main_cb_x_fatal(void *data);
static int  _e_main_cb_signal_exit(void *data, int ev_type, void *ev);
static int  _e_main_cb_signal_hup(void *data, int ev_type, void *ev);
static int  _e_main_cb_x_flusher(void *data);
static int  _e_main_cb_idler_before(void *data);
static int  _e_main_cb_idler_after(void *data);

static int  _e_main_cb_startup_fake_status(void *data);
static int  _e_main_cb_startup_fake_end(void *data);

E_Path *path_data    = NULL;
E_Path *path_images  = NULL;
E_Path *path_fonts   = NULL;
E_Path *path_themes  = NULL;
E_Path *path_init    = NULL;
E_Path *path_icons   = NULL;
int     restart      = 0;

/* local subsystem globals */
#define MAX_LEVEL 32
static int (*_e_main_shutdown_func[MAX_LEVEL]) (void);
static int _e_main_level = 0;

static Evas_List *_e_main_idler_before_list = NULL;

static Ecore_Idle_Enterer *_e_main_idle_enterer_before  = NULL;
static Ecore_Idle_Enterer *_e_main_idle_enterer_after   = NULL;
static Ecore_Idle_Enterer *_e_main_idle_enterer_flusher = NULL;

/* externally accessible functions */
int
main(int argc, char **argv)
{
   int ipc_failed = 0;
   int i;
   int nosplash = 0;
   int nostartup = 0;
   int nowelcome = 0;
   int after_restart = 0; 
   char buf[1024];
   char *s;
   /* install the signal handlers. */ 
   struct sigaction sigsegv_action;
   sigsegv_action.sa_sigaction=&e_sigseg_act;
   sigsegv_action.sa_flags=0;
   sigaction(SIGSEGV, &sigsegv_action, NULL);


   /* for debugging by redirecting stdout of e to a log file to tail */
   setvbuf(stdout, NULL, _IONBF, 0);
      
   if (getenv("NOSPLASH")) nosplash = 1;
   if (getenv("NOSTARTUP")) nostartup = 1;
   if (getenv("NOWELCOME")) nowelcome = 1;
   
   if (getenv("RESTART"))
     {
	printf("after restart!!!\n");
	after_restart = 1;
     }
   putenv("RESTART=1");
   
   /* handle some command-line parameters */
   for (i = 1; i < argc; i++)
     {
	if ((!strcmp(argv[i], "-display")) && (i < (argc - 1)))
	  {
	     i++;
	     
	     snprintf(buf, sizeof(buf), "DISPLAY=%s", argv[i]);
	     putenv(buf);
	  }
     }

   /* fix up DISPLAY to be :N.0 if no .screen is in it */
   s = getenv("DISPLAY");
   if (s)
     {
	char *p;
	
	p = strrchr(s, ':');
	if (!p)
	  {
	     snprintf(buf, sizeof(buf), "DISPLAY=%s:0.0", s);
	     putenv(buf);
	  }
	else
	  {
	     p = strrchr(p, '.');
	     if (!p)
	       {
		  snprintf(buf, sizeof(buf), "DISPLAY=%s.0", s);
		  putenv(buf);
	       }
	  }
     }
   
   /* init edje and set it up in frozen mode */
   edje_init();
   edje_freeze();
   
   /* basic ecore init */
   if (!ecore_init())
     {
	e_error_message_show("Enlightenment cannot Initialize Ecore!\n"
			     "Perhaps you are out of memory?");
	exit(-1);
     }
   _e_main_shutdown_push(ecore_shutdown);
   /* setup my args */
   ecore_app_args_set((int)argc, (const char **)argv);
   /* setup a handler for when e is asked to exit via a system signal */
   if (!ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, _e_main_cb_signal_exit, NULL))
     {
	e_error_message_show("Enlightenment cannot set up an exit signal handler.\n"
			     "Perhaps you are out of memory?");
	_e_main_shutdown(-1);
     }
   if(!ecore_event_handler_add(ECORE_EVENT_SIGNAL_HUP, _e_main_cb_signal_hup, NULL))
     {
	e_error_message_show("Enlightenment cannot set up a HUP signal handler.\n"
                             "Perhaps you are out of memory?");
	_e_main_shutdown(-1);
     }

   /* an idle enterer to be called before all others */
   _e_main_idle_enterer_before = ecore_idle_enterer_add(_e_main_cb_idler_before, NULL);
   
   /* init x */
   if (!ecore_x_init(NULL))
     {
	e_error_message_show("Enlightenment cannot initialize its X connection.\n"
			     "Have you set your DISPLAY variable?");
	_e_main_shutdown(-1);
     }
   _e_main_shutdown_push(_e_main_x_shutdown);
   
   ecore_x_grab();
   
   ecore_x_io_error_handler_set(_e_main_cb_x_fatal, NULL);

   /* Init window manager hints */
   e_hints_init();
   
   /* init generic communications */
   if (!ecore_con_init())
     {
	e_error_message_show("Enlightenment cannot initialize the connections system.\n"
			     "Perhaps you are out of memory?");
	_e_main_shutdown(-1);
     }
   _e_main_shutdown_push(ecore_con_shutdown);
   /* init ipc */
   if (!ecore_ipc_init())
     {
	e_error_message_show("Enlightenment cannot initialize the ipc system.\n"
			     "Perhaps you are out of memory?");
	_e_main_shutdown(-1);
     }
   _e_main_shutdown_push(ecore_ipc_shutdown);
   /* init the evas wrapper */
   if (!ecore_evas_init())
     {
	e_error_message_show("Enlightenment cannot initialize the evas system.\n"
			     "Perhaps you are out of memory?");
	_e_main_shutdown(-1);
     }
   _e_main_shutdown_push(ecore_evas_shutdown);
   
   /* setup directories we will be using for configurations sotrage etc. */
   if (!_e_main_dirs_init())
     {
	e_error_message_show("Enlightenment cannot create directories in your home directory.\n"
			     "Perhaps you have no home directory or the disk is full?");
	_e_main_shutdown(-1);
     }
   _e_main_shutdown_push(_e_main_dirs_shutdown);
   /* setup paths for finding things */
   if (!_e_main_path_init())
     {
	e_error_message_show("Enlightenment cannot set up paths for finding files.\n"
			     "Perhaps you are out of memory?");
	_e_main_shutdown(-1);
     }
   _e_main_shutdown_push(_e_main_path_shutdown);
   /* init config system */
   if (!e_config_init())
     {
	e_error_message_show("Enlightenment cannot set up its config system.");
	_e_main_shutdown(-1);
     }
   _e_main_shutdown_push(e_config_shutdown);
   
   /* setup edje to animate @ e_config->framerate frames per sec. */
   edje_frametime_set(1.0 / e_config->framerate);
   e_canvas_recache();
   
   /* setup init status window/screen */
   if (!e_init_init())
     {
	e_error_message_show("Enlightenment cannot set up init screen.\n"
			     "Perhaps you are out of memory?");
	_e_main_shutdown(-1);
     }
   _e_main_shutdown_push(e_init_shutdown);
   /* manage the root window */
   if (!_e_main_screens_init())
     {
	e_error_message_show("Enlightenment set up window management for all the screens on your system\n"
			     "failed. Perhaps another window manager is running?\n");
	_e_main_shutdown(-1);
     }
   _e_main_shutdown_push(_e_main_screens_shutdown);
   /* init app system */
   if (!e_app_init())
     {
	e_error_message_show("Enlightenment cannot set up its app system.");
	_e_main_shutdown(-1);
     }
   _e_main_shutdown_push(e_app_shutdown);

   /* tell the error system that it can use gui dialogs now */
   e_error_gui_set(1);
   
   /* setup e ipc service */
   if (!_e_main_ipc_init())
     {
	e_error_message_show(
			     "Enlightenment cannot set up the IPC socket.\n"
			     "It likely is already in use by an exisiting copy of Enlightenment.\n"
			     "Double check to see if Enlightenment is not already on this display,\n"
			     "but if that fails try deleting all files in ~/.ecore/enlightenment-*\n"
			     "and try running again.");
	ipc_failed = 1;
     }
   else
     _e_main_shutdown_push(_e_main_ipc_shutdown);

   /* setup module loading etc. FIXME: check return value */
   e_module_init();

   if (!e_iconify_init())
     {
	e_error_message_show("Enlightenment cannot setup its iconify system.");
	_e_main_shutdown(-1);	
     }
   else
     _e_main_shutdown_push(e_iconify_shutdown);

   if (!nowelcome)
     {
	/* explicitly show a gui dialog */
	e_error_dialog_show("Welcome to Enlightenment 0.17",
			    "This is program has barely been started on, so it is not complete by a long\n"
			    "shot. Please do NOT expect anything to work properly at this stage. It's\n"
			    "being worked on.\n"
			    "\n"
			    "Hit \"OK\" to dismiss this dialog and continue using Enlightenment 0.17.");
     }
   
   if (ipc_failed)
     e_error_dialog_show("Enlightenment IPC setup error!",
			 "Enlightenment cannot set up the IPC socket.\n"
			 "It likely is already in use by an exisiting copy of Enlightenment.\n"
			 "Double check to see if Enlightenment is not already on this display,\n"
			 "but if that fails try deleting all files in ~/.ecore/enlightenment-*\n"
			 "and try running again.");
   
   /* add in a handler that just before we go idle we flush x */
   _e_main_idle_enterer_flusher = ecore_idle_enterer_add(_e_main_cb_x_flusher, NULL);
      
   /* an intle enterer to be called after all others */
   _e_main_idle_enterer_after = ecore_idle_enterer_add(_e_main_cb_idler_after, NULL);

   ecore_x_ungrab();
   
   e_init_title_set("Enlightenment");
   e_init_version_set(VERSION);
   e_init_status_set("Enlightenment Starting. Please wait.");
   
   if (!nostartup)
     {
	if (after_restart) e_startup(E_STARTUP_RESTART);
	else e_startup(E_STARTUP_START);
     }
   
   if ((nosplash) || (after_restart))
     {
	ecore_timer_add(0.0, _e_main_cb_startup_fake_end, NULL);
     }
   else
     {
	ecore_timer_add( 3.0, _e_main_cb_startup_fake_status, "Artificially slowing startup so you can see it all.");
	ecore_timer_add( 7.5, _e_main_cb_startup_fake_status, "This is development code, so be warned.");
	ecore_timer_add(12.0, _e_main_cb_startup_fake_status, "Most features do not work yet, and those that do are buggy.");
	ecore_timer_add(16.0, _e_main_cb_startup_fake_end, NULL);
     }
   
   /* start our main loop */
   ecore_main_loop_begin();
   
   /* ask all modules to save their config and then shutdown */
   e_module_save_all();
   e_config_save();
   e_module_shutdown();

   /* unroll our stack of shutdown functions with exit code of 0 */
   _e_main_shutdown(0);
   
   /* if we were flagged to restart, then  restart. */
   if (restart)
     {
	printf("Restart...\n");
	ecore_app_restart();
	printf("eh? restart failed!\n");
     }
   
   /* just return 0 to keep the compiler quiet */
   return 0;
}

/* FIXME: make save to deleet within a callback */
E_Before_Idler *
e_main_idler_before_add(int (*func) (void *data), void *data, int once)
{
   E_Before_Idler *eb;
   
   eb = calloc(1, sizeof(E_Before_Idler));
   eb->func = func;
   eb->data = data;
   eb->once = once;
   _e_main_idler_before_list = evas_list_append(_e_main_idler_before_list, eb);
   return eb;
}

void
e_main_idler_before_del(E_Before_Idler *eb)
{
   eb->delete_me = 1;
}

/* local subsystem functions */
static void
_e_main_shutdown_push(int (*func) (void))
{
   _e_main_level++;
   if (_e_main_level > MAX_LEVEL)
     {
	_e_main_level--;
	e_error_message_show("WARNING: too many init levels. MAX = %i", MAX_LEVEL);
	return;
     }
   _e_main_shutdown_func[_e_main_level - 1] = func;
}

static void
_e_main_shutdown(int errorcode)
{
   int i;
   
   printf("E17: Begin shutdown procedure!\n");
   if (_e_main_idle_enterer_before)
     {
	ecore_idle_enterer_del(_e_main_idle_enterer_before);
	_e_main_idle_enterer_before = NULL;
     }
   if (_e_main_idle_enterer_after)
     {
	ecore_idle_enterer_del(_e_main_idle_enterer_after);
	_e_main_idle_enterer_after = NULL;
     }
   if (_e_main_idle_enterer_flusher)
     {
	ecore_idle_enterer_del(_e_main_idle_enterer_flusher);
	_e_main_idle_enterer_flusher = NULL;
     }
   for (i = _e_main_level - 1; i >= 0; i--)
     (*_e_main_shutdown_func[i])();
   if (errorcode < 0) exit(errorcode);
}

static int
_e_main_x_shutdown(void)
{
   ecore_x_ungrab();
   ecore_x_focus_reset();
   ecore_x_events_allow_all();

   ecore_x_shutdown();

   return 1;
}

static int
_e_main_dirs_init(void)
{
   char *homedir;
   char buf[PATH_MAX];
   const char *dirs[] =
     {
	"%s/.e",
	"%s/.e/e",
	"%s/.e/e/images",
	"%s/.e/e/fonts",
	"%s/.e/e/themes",
	"%s/.e/e/init",
	"%s/.e/e/icons",
	"%s/.e/e/backgrounds",
	"%s/.e/e/applications",
	"%s/.e/e/applications/all",
	"%s/.e/e/applications/favorite",
	"%s/.e/e/applications/bar",
	"%s/.e/e/applications/startup",
	"%s/.e/e/applications/restart",
	"%s/.e/e/modules",
	"%s/.e/e/config"
     };
   int i;
   
   homedir = e_user_homedir_get();
   if (!homedir) return 0;
   for (i = 0; i < (int)(sizeof(dirs) / sizeof(char *)); i++)
     {
	snprintf(buf, sizeof(buf), dirs[i], homedir);
	if (!e_file_mkpath(buf))
	  {
	     e_error_message_show("Error creating directory:\n"
				  "%s",
				  buf);
	     free(homedir);
	     return 0;
	  }
     }
   
   /* FIXME: THIS is a hack to get people started!!! */
   /*
   snprintf(buf, sizeof(buf), "%s/.e/e/applications/all/eterm.eapp", homedir);
   if (!e_file_exists(buf))
     {
	printf("GETTING YOU STARTED!\n");
	snprintf(buf, sizeof(buf), 
		 "gzip -d -c < %s/data/other/applications.tar.gz | "
		 "(cd %s/.e/e/ ; tar -xf -)", 
		 PACKAGE_DATA_DIR,
		 homedir);
	system(buf);
     }
   */
   free(homedir);
   
   return 1;
}

static int
_e_main_dirs_shutdown(void)
{
   return 1;
}

static int
_e_main_screens_init(void)
{
   Ecore_X_Window *roots;
   int num, i;

   if (!e_atoms_init()) return 0;
   if (!e_manager_init()) return 0;
   if (!e_container_init()) return 0;
   if (!e_zone_init()) return 0;
   if (!e_desk_init()) return 0;
   if (!e_gadman_init()) return 0;
   if (!e_menu_init()) return 0;
   
   num = 0;
   roots = ecore_x_window_root_list(&num);
   if ((!roots) || (num <= 0))
     {
	e_error_message_show("X reports there are no root windows and %i screens!\n", 
			     num);
	return 0;
     }
   if (!e_focus_init()) return 0;
   if (!e_border_init()) return 0;
   for (i = 0; i < num; i++)
     {
	E_Manager *man;
	E_Container *con;
	
	man = e_manager_new(roots[i]);
	e_init_show();
	if (man) e_manager_show(man);
	else
	  {
	     e_error_message_show("Cannot create manager object for screen %i\n", 
				  i);
	     return 0;
	  }
	con = e_container_new(man);
	if (con)
	  {
	     /* FIXME: move this to an actual function to start managing */
	     Ecore_X_Window *windows;
	     int wnum;
	     
	     windows = ecore_x_window_children_get(con->manager->root, &wnum);
	     if (windows)
	       {
		  int i;
		  
		  for (i = 0; i < wnum; i++)
		    {
		       Ecore_X_Window_Attributes att;
		       unsigned int ret_val, deskxy[2];
		       int ret;
		       
		       ecore_x_window_attributes_get(windows[i], &att);
		       ret = ecore_x_window_prop_card32_get(windows[i],
							    E_ATOM_MANAGED,
							    &ret_val, 1);
		       
		       /* we have seen this window before */
		       if ((ret > -1) && (ret_val == 1))
			 {
			    E_Border *bd;
			    
			    /* get all information from window before it is 
			     * reset by e_border_new */
			    ret = ecore_x_window_prop_card32_get(windows[i],
								 E_ATOM_DESK,
								 deskxy, 2);
			    bd = e_border_new(con, windows[i], 1);
			    if (bd)
			      {
				 if (ret == 2)
				   {
				      E_Desk *target;
				      target = e_desk_at_xy_get(bd->zone,
								deskxy[0],
								deskxy[1]);
				      e_border_desk_set(bd, target);
				      if (target == e_desk_current_get(bd->zone))
					{
					   ret = ecore_x_window_prop_card32_get(windows[i],
										E_ATOM_MAPPED,
										&ret_val, 1);
					   if ((ret > -1) && ret_val)
					     e_border_show(bd);
					}
				   }
			      }
			    ret = ecore_x_window_prop_card32_get(windows[i],
								 E_ATOM_ICONIC,
								 &ret_val, 1);
			    if ((ret > -1) && ret_val)
			      e_border_iconify(bd);
			 }
		       else if ((att.visible) && (!att.override) &&
				(!att.input_only))
			 {
			    /* We have not seen this window, and X tells us it
			     * should be seen */
			    E_Border *bd;
			    bd = e_border_new(con, windows[i], 1);
			    if (bd)
			      e_border_show(bd);
			 }
		    }
	       }
	     ecore_x_netwm_desk_roots_set(man->root, 1, &(con->win));
	     e_container_show(con);
	  }
	else
	  {
	     e_error_message_show("Cannot create desktop object for manager on screen %i\n", 
				  i);
	     return 0;
	  }
	/*
	  {
	     E_Gadman_Client *gmc;
	     
	     gmc = e_gadman_client_new(con->gadman);
	     gmc->x = 0;
	     gmc->y = 300;
	     gmc->w = 100;
	     gmc->h = 100;
	     e_gadman_client_policy_set(gmc, 
//					E_GADMAN_POLICY_EDGES | 
					E_GADMAN_POLICY_ANYWHERE |
					E_GADMAN_POLICY_HMOVE | 
					E_GADMAN_POLICY_VMOVE |
					E_GADMAN_POLICY_HSIZE | 
					E_GADMAN_POLICY_VSIZE);
	     e_gadman_client_min_size_set(gmc, 20, 20);
	     e_gadman_client_auto_size_set(gmc, 50, 200);
	     e_gadman_client_align_set(gmc, 0.0, 0.5);
	     e_client_gadman_edge_set(gmc, E_GADMAN_EDGE_LEFT);
	     e_gadman_client_domain_set(gmc, "test", 0);
	     e_gadman_client_load(gmc);

	     gmc = e_gadman_client_new(con->gadman);
	     gmc->x = 0;
	     gmc->y = 400;
	     gmc->w = 100;
	     gmc->h = 100;
	     e_gadman_client_policy_set(gmc, 
//					E_GADMAN_POLICY_EDGES | 
					E_GADMAN_POLICY_ANYWHERE |
					E_GADMAN_POLICY_HMOVE | 
					E_GADMAN_POLICY_VMOVE |
					E_GADMAN_POLICY_HSIZE | 
					E_GADMAN_POLICY_VSIZE);
	     e_gadman_client_min_size_set(gmc, 20, 20);
	     e_gadman_client_auto_size_set(gmc, 50, 200);
	     e_gadman_client_align_set(gmc, 0.0, 0.5);
	     e_client_gadman_edge_set(gmc, E_GADMAN_EDGE_LEFT);
	     e_gadman_client_domain_set(gmc, "test", 1);
	     e_gadman_client_load(gmc);

	     gmc = e_gadman_client_new(con->gadman);
	     gmc->x = 0;
	     gmc->y = 500;
	     gmc->w = 100;
	     gmc->h = 100;
	     e_gadman_client_policy_set(gmc, 
//					E_GADMAN_POLICY_EDGES | 
					E_GADMAN_POLICY_ANYWHERE |
					E_GADMAN_POLICY_HMOVE | 
					E_GADMAN_POLICY_VMOVE |
					E_GADMAN_POLICY_HSIZE | 
					E_GADMAN_POLICY_VSIZE);
	     e_gadman_client_min_size_set(gmc, 20, 20);
	     e_gadman_client_auto_size_set(gmc, 50, 200);
	     e_gadman_client_align_set(gmc, 0.0, 0.5);
	     e_client_gadman_edge_set(gmc, E_GADMAN_EDGE_LEFT);
	     e_gadman_client_domain_set(gmc, "test", 2);
	     e_gadman_client_load(gmc);
	  }
	 */
     }
   free(roots);
   ecore_x_sync();
   return 1;
}

static int
_e_main_screens_shutdown(void)
{
   e_border_shutdown();
   e_focus_shutdown();
   e_menu_shutdown();
   e_gadman_shutdown();
   e_desk_shutdown();
   e_zone_shutdown();
   e_container_shutdown();
   e_manager_shutdown();
   e_atoms_shutdown();
   return 1;
}

static int
_e_main_path_init(void)
{
   path_data = e_path_new();
   if (!path_data)
     {
	e_error_message_show("Cannot allocate path for path_data\n");
	return 0;
     }
   e_path_path_append(path_data, PACKAGE_DATA_DIR"/data");
   path_images = e_path_new();
   if (!path_images)
     {
	e_error_message_show("Cannot allocate path for path_images\n");
	return 0;
     }
   e_path_path_append(path_images, "~/.e/e/images");
   e_path_path_append(path_images, PACKAGE_DATA_DIR"/data/images");
   path_fonts = e_path_new();
   if (!path_fonts)
     {
	e_error_message_show("Cannot allocate path for path_fonts\n");
	return 0;
     }
   e_path_path_append(path_fonts, "~/.e/e/fonts");
   e_path_path_append(path_fonts, PACKAGE_DATA_DIR"/data/fonts");
   path_themes = e_path_new();
   if (!path_themes)
     {
	e_error_message_show("Cannot allocate path for path_themes\n");
	return 0;
     }
   e_path_path_append(path_themes, "~/.e/e/themes");
   e_path_path_append(path_themes, PACKAGE_DATA_DIR"/data/themes");
   path_icons = e_path_new();
   if (!path_icons)
     {
	e_error_message_show("Cannot allocate path for path_icons\n");
	return 0;
     }
   e_path_path_append(path_icons, "~/.e/e/icons");
   e_path_path_append(path_icons, PACKAGE_DATA_DIR"/data/icons");
   path_init = e_path_new();
   if (!path_init)
     {
	e_error_message_show("Cannot allocate path for path_init\n");
	return 0;
     }
   e_path_path_append(path_init, "~/.e/e/init");
   e_path_path_append(path_init, PACKAGE_DATA_DIR"/data/init");
   return 1;
}

static int
_e_main_path_shutdown(void)
{
   if (path_data)
     {
	e_object_unref(E_OBJECT(path_data));
	path_data = NULL;
     }
   if (path_images)
     {
	e_object_unref(E_OBJECT(path_images));
	path_images = NULL;
     }
   if (path_fonts)
     {
	e_object_unref(E_OBJECT(path_fonts));
	path_fonts = NULL;
     }
   if (path_themes)
     {
	e_object_unref(E_OBJECT(path_themes));
	path_themes = NULL;
     }
   if (path_icons)
     {
	e_object_unref(E_OBJECT(path_icons));
	path_icons = NULL;
     }
   if (path_init)
     {
	e_object_unref(E_OBJECT(path_init));
	path_init = NULL;
     }
   return 1;
}

static int
_e_main_ipc_init(void)
{
   if (!e_ipc_init())
     {
	e_error_message_show("Cannot init IPC subsystem!\n");
	return 0;
     }
   return 1;
}

static int
_e_main_ipc_shutdown(void)
{
   e_ipc_shutdown();
   return 1;
}

static void
_e_main_cb_x_fatal(void *data)
{
   e_error_gui_set(0);
   e_error_message_show("Lost X connection.");
   ecore_main_loop_quit();
   _e_main_shutdown(-1);
}

static int
_e_main_cb_signal_exit(void *data, int ev_type, void *ev)
{
   /* called on ctrl-c, kill (pid) (also SIGINT, SIGTERM and SIGQIT) */
   ecore_main_loop_quit();
   return 1;
}

static int
_e_main_cb_signal_hup(void *data, int ev_type, void *ev)
{
   /* called on SIGHUP to restart Enlightenment */
   printf("RESTART ON!\n");
   restart = 1;
   ecore_main_loop_quit();
   return 1;
}

static int
_e_main_cb_x_flusher(void *data)
{
   ecore_x_flush();
   return 1;
}

static int
_e_main_cb_idler_before(void *data)
{
   Evas_List *l, *pl;
   
   e_menu_idler_before();
   e_focus_idler_before();
   e_border_idler_before();
   for (l = _e_main_idler_before_list; l; l = l->next)
     {
	E_Before_Idler *eb;
	
	eb = l->data;
	if (!eb->delete_me)
	  {
	     if (!eb->func(eb->data)) eb->delete_me = 1;
	  }
     }
   for (l = _e_main_idler_before_list; l;)
     {
	E_Before_Idler *eb;
	
	eb = l->data;
	pl = l;
	l = l->next;
	if ((eb->once) || (eb->delete_me))
	  {
	     _e_main_idler_before_list =
	       evas_list_remove_list(_e_main_idler_before_list, pl);
	     free(eb);
	  }
     }
   edje_thaw();
//   printf("IN to idle... %3.3f\n", ecore_time_get());
   return 1;
}

static int
_e_main_cb_idler_after(void *data)
{
//   printf("OUT of idle... %3.3f\n", ecore_time_get());
   edje_freeze();
   return 1;
}

static int
_e_main_cb_startup_fake_status(void *data)
{
   e_init_status_set((const char *)data);
   return 0;
}

static int
_e_main_cb_startup_fake_end(void *data)
{
   e_init_hide();
   return 0;
}

