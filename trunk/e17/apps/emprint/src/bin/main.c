#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <Imlib2.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Ecore_File.h>
#include <Ecore_X.h>
#include <Ecore_X_Cursor.h>
#include <Edje.h>
#include "config.h"
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#include "emprint.h"

/* Function Prototypes */
static void _em_print_help(void);
static void _em_print_version(void);
static void _em_free_options(void);
static void _em_get_filename(void);
static void _em_init_imlib(void);
static void _em_do_shot(void);
static void _em_do_screen(void);
static void _em_do_window(void);
static void _em_do_region(void);
static void _em_do_thumb(void *data);
static void _em_take_shot(int x, int y, int w, int h);
static int _em_cb_mouse_move(void *data, int type, void *event);
static int _em_cb_mouse_up(void *data, int type, void *event);
static int _em_cb_mouse_down(void *data, int type, void *event);
static int _em_cb_timer(void *data);
static void _em_band_show(void);
static void _em_band_move(int x, int y);
static void _em_band_hide(void);
static void _em_band_resize(int w, int h);
static void _em_grab_region_end(void);
static void _em_do_app(void);

/* Global Variables */
static Options *opts = NULL;
static Ecore_X_Window input_window = 0;
static Ecore_Event_Handler *mouse_move_hdl = 0;
static Ecore_Event_Handler *mouse_up_hdl = 0;
static Ecore_Event_Handler *mouse_down_hdl = 0;
static Ecore_Timer *timer = NULL;
static Band *band = NULL;
static int gx = -1, gy = -1;

/* Main Program Function */
int 
main(int argc, char **argv) 
{
   int i, qual, delay;
   int tw, th, ts;
   char *app, *file;

   /* allocate a structure to hold our options */
   opts = calloc(1, sizeof(Options));

   /* parse the options provided by user */
   for (i = 1; i < argc; i++) 
     {
	if (!strcmp(argv[i], "--beep")) 
	  opts->beep = 1;
	else if ((!strcmp(argv[i], "--delay")) && (i < (argc - 1)))
	  {
	     i++;
	     if (sscanf(argv[i], "%i", &delay) == 1) 
	       opts->delay = delay;
	  }
	else if ((!strcmp(argv[i], "--app")) && (i < (argc - 1)))
	  {
	     i++;
	     if (sscanf(argv[i], "%s", &app) == 1) 
	       opts->app = evas_stringshare_add(argv[i]);
	  }
	else if ((!strcmp(argv[i], "--thumb")) && (i < (argc - 1)))
	  {
	     i++;
	     if (sscanf(argv[i], "%s", &file) == 1) 
	       {
		  opts->use_thumb = 1;
		  opts->thumb.filename = evas_stringshare_add(argv[i]);
	       }
	  }
	else if ((!strcmp(argv[i], "--thumb-geom")) && (i < (argc - 1)))
	  {
	     i++;
	     if (sscanf(argv[i], "%ix%i", &tw, &th) == 2) 
	       {
		  opts->use_thumb = 1;
		  opts->thumb.width = tw;
		  opts->thumb.height = th;
	       }
	     else if (sscanf(argv[i], "%i", &ts) == 1) 
	       {
		  if (ts < 1) ts = 1;
		  else if (ts > 100) ts = 100;
		  opts->use_thumb = 1;
		  opts->thumb.size = ts;
	       }
	  }
	else if ((!strcmp(argv[i], "--quality")) && (i < (argc - 1)))
	  {
	     i++;
	     if (sscanf(argv[i], "%i", &qual) == 1) 
	       opts->quality = qual;
	  }
	else if (!strcmp(argv[i], "--region")) 
	  opts->region = 1;
	else if (!strcmp(argv[i], "--window")) 
	  opts->window = 1;
	else if ((!strcmp(argv[i], "--help")) || (!strcmp(argv[i], "-h")))
	  {
	     _em_print_help();
	     _em_free_options();
	     exit(0);
	  }
	else if (!strcmp(argv[i], "--version")) 
	  {
	     _em_print_version();
	     _em_free_options();
	     exit(0);
	  }
	else if (i < argc) 
	  {
	     while (i < argc) 
	       {
		  if (!opts->filename) 
		    opts->filename = evas_stringshare_add(argv[i]);
		  else
		    {
		       printf("Unknown Option: %s\n", argv[i]);
		       _em_print_help();
		       _em_free_options();
		       exit(0);
		    }
		  i++;
	       }
	  }
     }

   /* initialize ecore */
   if (!ecore_init()) return -1;

   /* initialize ecore_evas */
   if (!ecore_evas_init()) return -1;

   /* initialize ecore_x */
   if (!ecore_x_init(NULL)) return -1;

   /* initialize edje */
   if (!edje_init()) return -1;

   /* set app arguments */
   ecore_app_args_set(argc, (const char **)argv);

   /* parse the filename for '%', or create one if no filename is given */
   _em_get_filename();

   /* start screenshot process */
   _em_do_shot();

   /* begin the ecore main loop which will keep our app running */
   ecore_main_loop_begin();

   /* launch application if user wanted one */
   if (opts->app) _em_do_app();

   /* free our option structure */
   _em_free_options();

   /* shutdown edje */
   edje_shutdown();

   /* shutdown ecore_x */
   ecore_x_shutdown();

   /* shutdown ecore_evas */
   ecore_evas_shutdown();

   /* shutdown ecore */
   ecore_shutdown();

   return 0;
}

static void 
_em_print_help(void) 
{
   printf("Usage: emprint [OPTIONS]... FILE\n"
	  "Where FILE is the target for the screenshot.\n"
	  "If no FILE is specified, a date-stamped file will be saved in "
	  "the current directory.\n"
	  "\t--help\t\t\tDisplay this help\n"
	  "\t--beep\t\t\tBeep before taking screenshot\n"
	  "\t--delay NUM\t\tWait NUM seconds before taking screenshot\n"
	  "\t--prompt\t\tPrompt for FILE\n"
	  "\t--app APP\t\tLaunch APP after taking screenshot\n"
	  "\t--thumb THUMB\t\tGenerate a thumbnail as THUMB\n"
	  "\t--thumb-geom NUM\tGeometry to use for thumbnail\n"
	  "\t\t\t\tNUM can be a percentage of the original size OR\n"
	  "\t\t\t\tthe actual geometry to use for the thumbnail,\n"
	  "\t\t\t\tie: 100x100\n"
	  "\t--quality NUM\t\tImage quality of screenshot\n"
	  "\t--region\t\tSelect a specific screen region\n"
	  "\t--window\t\tSelect a specifc window to grab\n"
	  );
}

static void 
_em_print_version(void) 
{
   printf("emprint version "VERSION"\n");
}

static void 
_em_free_options(void) 
{
   /* cleanup our options structure */
   if (!opts) return;
   if (opts->app) evas_stringshare_del(opts->app);
   if (opts->filename) evas_stringshare_del(opts->filename);
   if (opts->thumb.filename) evas_stringshare_del(opts->thumb.filename);
   free(opts);
   opts = NULL;
}

static void 
_em_get_filename(void) 
{
   Ecore_List *fl = NULL;
   char *dir = NULL, *ext = NULL, *file = NULL;
   const char *f;
   char buf[256];
   struct tm *loctime;
   time_t t;
   int c = 0;

   /* get the current time in local format */
   t = time(NULL);
   loctime = localtime(&t);

   /* if no filename given, create one based on time */
   if ((!opts->filename) || (opts->filename == NULL))
     strftime(buf, sizeof(buf), "%Y-%m-%d-%H%M%S.png", loctime);
   else 
     {
	/* filename was given, check for '%' signs to 
	 * format based on strftime */
	if (strstr(opts->filename, "%")) 
	  strftime(buf, sizeof(buf), opts->filename, loctime);
	else 
	  {
	     /* no '%' signs, check for files existing in the directory */
	     if (ecore_file_exists(opts->filename)) 
	       {
		  /* get the directory */
		  dir = ecore_file_dir_get(opts->filename);

		  /* get the filename */
		  f = ecore_file_file_get(opts->filename);

		  /* if these two match, then no dir was passed in,
		   * use current dir */
		  if (!strcmp(dir, f)) dir = getenv("PWD");

		  /* strip the extension for searches */
		  ext = ecore_file_strip_ext(opts->filename);

		  /* list files in this directory & count them */
		  fl = ecore_file_ls(dir);
		  ecore_list_first_goto(fl);
		  while ((file = ecore_list_next(fl)) != NULL) 
		    {
		       /* skip "thumb" files in the count */
		       if (strstr(file, "thumb")) continue;
		       if (strstr(file, ext)) c++;
		    }

		  /* destroy the file list */
		  if (fl) ecore_list_destroy(fl);

		  /* assemble new filename based on count */
		  if (c > 0) 
		    {
		       c++;

		       /* strip the extension from filename */
		       file = ecore_file_strip_ext(opts->filename);

		       /* get the actual extension */
		       ext = strrchr(opts->filename, '.');
		       if (!ext) ext = strdup(".png");

		       /* assemble new filename */
		       snprintf(buf, sizeof(buf), "%s%i%s", file, c, ext);
		    }
		  else return;
	       }
	     else return;
	  }
     }
   /* set the new filename */
   if (opts->filename) evas_stringshare_del(opts->filename);
   opts->filename = evas_stringshare_add(buf);
}

static void 
_em_init_imlib(void) 
{
   Ecore_X_Display *disp;
   Ecore_X_Window root;
//   Ecore_X_Screen *screen;

   /* get the display */
   disp = ecore_x_display_get();

   /* get the root window */
   root = RootWindow(disp, 0);

   /* get default screen */
//   screen = ecore_x_default_screen_get();

   /* setup imlib in preparation for shot */
   imlib_context_set_display(disp);
   imlib_context_set_drawable(root);
   imlib_context_set_visual(DefaultVisual(disp, 
					  DefaultScreen(ecore_x_display_get())));
}

static void 
_em_do_shot(void) 
{
   /* if user specified a delay, do it */
   if (opts->delay > 0) 
     {
	/* create a timer for the delay */
	if (timer) ecore_timer_del(timer);
	timer = ecore_timer_add(1.0, _em_cb_timer, NULL);
     }
   else 
     {
	/* are we taking a region, window or whole screen shot? */
	if (opts->region)
	  _em_do_region();
	else if (opts->window)
	  _em_do_window();
	else
	  _em_do_screen();
     }
}

static void 
_em_do_screen(void) 
{
   int w, h;

   /* initialize imlib */
   _em_init_imlib();

   /* get the size of the root window */
   ecore_x_window_size_get(RootWindow(ecore_x_display_get(), 0), &w, &h);

   /* actually take the shot */
   _em_take_shot(0, 0, w, h);

   /* quit the main ecore loop */
   ecore_main_loop_quit();
}

static void 
_em_do_window(void) 
{
   Ecore_X_Window root;
   Ecore_X_Cursor cursor = 0;
   int x, y, w, h;

   /* initialize imlib */
   _em_init_imlib();

   /* get the root window */
   root = RootWindow(ecore_x_display_get(), 0);

   /* get the size of the current root window */
   ecore_x_window_geometry_get(root, &x, &y, &w, &h);

   /* check if our input window already exists & delete it */
   if (input_window) ecore_x_window_del(input_window);

   /* create a new input window to recieve click event */
   input_window = ecore_x_window_input_new(root, x, y, w, h);

   /* show the input window */
   ecore_x_window_show(input_window);

   /* set the mouse pointer */
   if (cursor = ecore_x_cursor_shape_get(ECORE_X_CURSOR_CROSS))
     ecore_x_window_cursor_set(input_window, cursor);

   /* setup handler to recieve click event */
   mouse_up_hdl = ecore_event_handler_add(ECORE_X_EVENT_MOUSE_BUTTON_UP, 
					  _em_cb_mouse_up, NULL);
}

static void 
_em_do_region(void) 
{
   Ecore_X_Window root;
   Ecore_X_Cursor cursor = 0;
   int x, y, w, h;

   /* initialize imlib */
   _em_init_imlib();

   /* get the root window */
   root = RootWindow(ecore_x_display_get(), 0);

   /* get the size of the current root window */
   ecore_x_window_geometry_get(root, &x, &y, &w, &h);

   /* check if our input window already exists & delete it */
   if (input_window) ecore_x_window_del(input_window);

   /* create a new input window to recieve click event */
   input_window = ecore_x_window_input_new(root, x, y, w, h);
   ecore_x_window_show(input_window);

   /* setup handlers to recieve mouse events */
   mouse_move_hdl = ecore_event_handler_add(ECORE_X_EVENT_MOUSE_MOVE, 
					    _em_cb_mouse_move, NULL);
   mouse_up_hdl = ecore_event_handler_add(ECORE_X_EVENT_MOUSE_BUTTON_UP,
					  _em_cb_mouse_up, NULL);
   mouse_down_hdl = ecore_event_handler_add(ECORE_X_EVENT_MOUSE_BUTTON_DOWN,
					    _em_cb_mouse_down, NULL);

   /* create the rubber band */
   _em_band_show();

   /* set the mouse pointer */
   if (cursor = ecore_x_cursor_shape_get(ECORE_X_CURSOR_CROSS))
     ecore_x_window_cursor_set(input_window, cursor);
}

static void 
_em_do_thumb(void *data) 
{
   Imlib_Image *im = NULL, *thumb = NULL;
   Imlib_Load_Error err;
   char *ext = NULL;
   char buf[4096];
   int w, h, tw, th;

   if (!(im = data)) return;

   /* set imlib context to our current shot */
   imlib_context_set_image(im);

   /* get the image width & height */
   w = imlib_image_get_width();
   h = imlib_image_get_height();

   /* calculate thumbnail size */
   if ((opts->thumb.width > 0) && (opts->thumb.height > 0)) 
     {
	tw = w * opts->thumb.width / 100;
	th = h * opts->thumb.height / 100;
     }
   else if (opts->thumb.size > 0) 
     {
	tw = w * opts->thumb.size / 100;
	th = h * opts->thumb.size / 100;
     }
   else 
     {
	/* by default, create a 50% thumbnail */
	tw = w * 50 / 100;
	th = h * 50 / 100;
     }

   /* actually create the thumbnail of our image */
   thumb = imlib_create_cropped_scaled_image(0, 0, w, h, tw, th);
   if (!thumb) 
     printf("Error creating thumbnail: %s\n", opts->filename);
   else 
     {
	/* set imlib context to the thumbnail */
	imlib_context_set_image(thumb);

	/* check for user-supplied thumbnail filename */
	if (!opts->thumb.filename) 
	  {
	     /* no thumbname filename supplied, create one */
	     ext = ecore_file_strip_ext(opts->filename);
	     snprintf(buf, sizeof(buf), "%s-thumb", ext);
	     
	     /* check for extension */
	     ext = strrchr(opts->filename, '.');
	     if (ext) 
	       snprintf(buf, sizeof(buf), "%s%s", strdup(buf), ext);
	     else
	       snprintf(buf, sizeof(buf), "%s.png", strdup(buf));
	     opts->thumb.filename = evas_stringshare_add(buf);
	  }

	/* actually save the thumbnail */
	imlib_save_image_with_error_return(opts->thumb.filename, &err);
	if (err) 
	  printf("Error saving thumbnail: %s\n", opts->thumb.filename);

	/* cleanup imlib */
	imlib_free_image_and_decache();
     }
}

static void 
_em_take_shot(int x, int y, int w, int h) 
{
   Imlib_Image *im = NULL;
   Imlib_Load_Error err;

   /* if user wanted a beep, then beep there shall be */
   if (opts->beep) XBell(ecore_x_display_get(), 0);

   /* actually create the shot */
   im = imlib_create_image_from_drawable(0, x, y, w, h, 1);
   if (!im) return;

   /* set imlib context to our current shot: 
    * this tells imlib what image we are working with */
   imlib_context_set_image(im);

   /* if user passed in any quality settings, attach them to the image */
   if (opts->quality)
     imlib_image_attach_data_value("quality", NULL, opts->quality, NULL);

   /* save the image to disk */
   imlib_save_image_with_error_return(opts->filename, &err);
   if (err) 
     {
	printf("Error taking screenshot: %s\n", opts->filename);
	imlib_free_image_and_decache();
	return;
     }

   /* if user wanted a thumbnail, do it */
   if (opts->use_thumb) _em_do_thumb(im);

   /* cleanup imlib */
   imlib_context_set_image(im); 
   imlib_free_image_and_decache();
}

static int 
_em_cb_mouse_move(void *data, int type, void *event) 
{
   Ecore_X_Event_Mouse_Move *ev;
   int x, y, w, h;

   ev = event;
   if ((gx < 0) && (gy < 0)) return 1;

   w = ev->root.x - gx;
   h = ev->root.y - gy;

   /* figure out how to move the band */
   if ((w < 0) && (h < 0))
     {
	ecore_x_pointer_xy_get(input_window, &x, &y);
	_em_band_move(x, y);
	_em_band_resize(-w, -h);
     }
   else if ((w < 0) &&  (h > 0))
     {
	ecore_x_pointer_xy_get(input_window, &x, &y);
	_em_band_move(x, gy);
	_em_band_resize(-w, h);
     }
   else if ((w > 0) && (h < 0))
     {
	ecore_x_pointer_xy_get(input_window, &x, &y);
	_em_band_move(gx, y);
	_em_band_resize(w, -h);
     }
   else
     _em_band_resize(w, h);
   return 1;
}

static int 
_em_cb_mouse_up(void *data, int type, void *event) 
{
   Ecore_X_Event_Mouse_Button_Up *ev;
   Ecore_X_Display *disp;
   Ecore_X_Window win, root;
   int x, y, w, h;

   if (opts->region) 
     {
	_em_grab_region_end();
	return;
     }
   ev = event;

   /* check for correct mouse button */
   if (ev->button != 1) return 1;

   /* check for correct window */
   if (ev->win != input_window) return 1;

   /* get last known pointer position */
   ecore_x_pointer_last_xy_get(&x, &y);

   /* delete the event handlers */
   ecore_event_handler_del(mouse_up_hdl);
   mouse_up_hdl = 0;

   /* delete the input window */
   ecore_x_window_del(input_window);
   input_window = 0;

   /* get the window which was clicked */
   win = ecore_x_window_at_xy_get(x, y);

   /* get the display */
   disp = ecore_x_display_get();

   /* E uses many parent windows...grab the correct one */
   root = RootWindow(disp, 0);
   while (win != root) 
     {
	if (ecore_x_window_parent_get(win) == root) break;
	win = ecore_x_window_parent_get(win);
     }
   
   /* get this window's dimensions */
   ecore_x_window_geometry_get(win, &x, &y, &w, &h);

   /* actually take the shot */
   _em_take_shot(x, y, w, h);

   /* quit the main ecore loop */
   ecore_main_loop_quit();

   return 0;
}

static int 
_em_cb_mouse_down(void *data, int type, void *event) 
{
   Ecore_X_Event_Mouse_Button_Down *ev;

   ev = event;
   if (ev->win != input_window) return 1;
   if (ev->button != 1) return 1;

   /* get current mouse coordinates */
   ecore_x_pointer_xy_get(input_window, &gx, &gy);

   /* move the band to current coordinates */
   _em_band_move(gx, gy);
   return 1;
}

static int 
_em_cb_timer(void *data) 
{
   static int count = 0;

   if (count == opts->delay) 
     {
	/* are we taking a region, window or whole screen shot? */
	if (opts->region)
	  _em_do_region();
	else if (opts->window)
	  _em_do_window();
	else
	  _em_do_screen();
	return 0;
     }

   /* tell the user we are counting down */
   printf("Taking shot in %d\n", (opts->delay - count));
   count++;

   return 1;
}

static void 
_em_band_show(void) 
{
   /* if the band already exists return */
   if (band) return;

   /* allocate memory for the band */
   band = calloc(1, sizeof(Band));

   /* create the ecore_evas for the band */
   band->ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 0, 0);

   /* get the band's canvas */
   band->evas = ecore_evas_get(band->ee);

   /* check if we have composite, if not then used shaped */
   if ((ecore_x_composite_query()) && (ecore_x_screen_is_composited(0)))
     ecore_evas_alpha_set(band->ee, 1);
   else
     ecore_evas_shaped_set(band->ee, 1);

   /* create the edje object which will display the band */
   band->edj = evas_object_image_add(band->evas);
   evas_object_image_file_set(band->edj, PACKAGE_DATA_DIR"/images/rect.png", NULL);
   evas_object_image_border_set(band->edj, 3, 3, 3, 3);
   evas_object_move(band->edj, 0, 0);
   evas_object_resize(band->edj, 50, 50);
   evas_object_image_fill_set(band->edj, 0, 0, 50, 50);

   /* tell the window manager to ignore this window */
   ecore_evas_override_set(band->ee, 1);

   /* make this window borderless */
   ecore_evas_borderless_set(band->ee, 1);

   /* show the band */
   ecore_evas_show(band->ee);
   evas_object_show(band->edj);
}

static void 
_em_band_move(int x, int y) 
{
   if (!band) return;
   ecore_evas_move(band->ee, x, y);
}

static void 
_em_band_hide(void) 
{
   if (!band) return;
   ecore_evas_hide(band->ee);
}

static void 
_em_band_resize(int w, int h) 
{
   if (!band) return;
   ecore_evas_resize(band->ee, w, h);
   if ((w >= 6) && (h >= 6)) 
     {
	evas_object_resize(band->edj, w, h);
	evas_object_image_fill_set(band->edj, 0, 0, w, h);
     }
   _em_band_show();
}

static void 
_em_grab_region_end(void) 
{
   int x, y, w, h;

   /* delete the event handlers */
   ecore_event_handler_del(mouse_move_hdl);
   ecore_event_handler_del(mouse_up_hdl);
   ecore_event_handler_del(mouse_down_hdl);

   /* delete the input window */
   ecore_x_window_del(input_window);

   /* get the size of the band */
   ecore_evas_geometry_get(band->ee, &x, &y, &w, &h);

   /* hide the band */
   _em_band_hide();

   /* grab shot */
   _em_take_shot(x, y, w, h);

   /* quit the app */
   ecore_main_loop_quit();
}

static void 
_em_do_app(void) 
{
   Ecore_Exe *exe;
   char buf[4096];

   /* assemble exe string including app & filename */
   snprintf(buf, sizeof(buf), "%s %s", opts->app, opts->filename);

   /* run the app */
   exe = ecore_exe_run(buf, NULL);
   if (exe) ecore_exe_free(exe);
}
