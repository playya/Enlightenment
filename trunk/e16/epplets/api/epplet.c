#include "epplet.h"
#include <errno.h>
#include <sys/utsname.h>
#include <signal.h>
#include <sys/wait.h>

static Display   *disp        = NULL;
static Window     win         = 0;
static ImlibData *id          = NULL;
static Display   *dd          = NULL;
static Window     comms_win   = 0;
static Window     my_win      = 0;
static Window     root        = 0;
static ETimer    *q_first     = NULL;
static XContext   xid_context = 0;
static int        win_w       = 0;
static int        win_h       = 0;
static Pixmap     bg_pmap     = 0;
static Pixmap     bg_mask     = 0;
static Pixmap     bg_bg       = 0;
static char       win_vert    = 0;

static char      *conf_dir           = NULL;
static char      *epplet_name        = NULL;
static char      *epplet_cfg_file    = NULL;
static int        epplet_instance    = 0;

static int            gad_num = 0;
static Epplet_gadget *gads    = NULL;

static void *expose_data = NULL;
static void *moveresize_data = NULL;
static void *buttonpress_data = NULL;
static void *buttonrelease_data = NULL;
static void *mousemotion_data = NULL;
static void *keypress_data = NULL;
static void *keyrelease_data = NULL;
static void *enter_data = NULL;
static void *leave_data = NULL;
static void *focusin_data = NULL;
static void *focusout_data = NULL;
static void *event_data = NULL;
static void *comms_data = NULL;
static void *child_data = NULL;

static void (*expose_func) (void *data, Window win, int x, int y, int w, int h) = NULL;
static void (*moveresize_func) (void *data, Window win, int x, int y, int w, int h) = NULL;
static void (*buttonpress_func) (void *data, Window win, int x, int y, int b) = NULL;
static void (*buttonrelease_func) (void *data, Window win, int x, int y, int b) = NULL;
static void (*mousemotion_func) (void *data, Window win, int x, int y) = NULL;
static void (*keypress_func) (void *data, Window win, char *key) = NULL;
static void (*keyrelease_func) (void *data, Window win, char *key) = NULL;
static void (*enter_func) (void *data, Window win) = NULL;
static void (*leave_func) (void *data, Window win) = NULL;
static void (*focusin_func) (void *data, Window win) = NULL;
static void (*focusout_func) (void *data, Window win) = NULL;
static void (*event_func) (void *data, XEvent *ev) = NULL;
static void (*comms_func) (void *data, char *s) = NULL;
static void (*child_func) (void *data, int pid, int exit_code) = NULL;

#define MWM_HINTS_DECORATIONS         (1L << 1)
typedef struct _mwmhints
{
   unsigned long flags;
   unsigned long functions;
   unsigned long decorations;
   long          inputMode;
   unsigned long status;
}
MWMHints;
struct _etimer
{
      char *name;
      void (*func) (void *data);
      void *data;
      double in;
      char just_added;
      ETimer *next;
};

#define ESYNC ECommsSend("nop");free(ECommsWaitForMessage());

/* The structures for the config file management ... */
typedef struct _configdict
{
    ConfigItem *entries;
    int         num_entries;
}
ConfigDict;

static ConfigDict *config_dict = NULL;

static void    CommsFindCommsWindow(void);
static void    ECommsSetup(Display *d);
static void    CommsHandleDestroy(Window win);
static int     CommsHandlePropertyNotify(XEvent *ev);
static void    CommsFindCommsWindow(void);
static void    ECommsSend(char *s);
static char   *ECommsGet(XEvent * ev);
static char   *ECommsWaitForMessage(void);
static void    Epplet_handle_timer(void);
static ETimer *Epplet_get_first(void);
static void    Epplet_handle_event(XEvent *ev);
static void    ECommsSend(char *s);
static Bool    ev_check(Display *d, XEvent *ev, XPointer p);
static char   *win_name = NULL;
static char   *win_version = NULL;
static char   *win_info = NULL;

static void    Epplet_redraw(void);
static void    Epplet_event(Epplet_gadget gadget, XEvent *ev);
static void    Epplet_add_gad(Epplet_gadget gadget);
static void    Epplet_del_gad(Epplet_gadget gadget);
static void    Epplet_draw_button(Epplet_gadget eg);
static void    Epplet_draw_textbox(Epplet_gadget eg);
static void    Epplet_draw_togglebutton(Epplet_gadget eg);
static void    Epplet_draw_drawingarea(Epplet_gadget eg);
static void    Epplet_draw_hslider(Epplet_gadget eg);
static void    Epplet_draw_vslider(Epplet_gadget eg);
static void    Epplet_draw_hbar(Epplet_gadget eg);
static void    Epplet_draw_vbar(Epplet_gadget eg);
static void    Epplet_draw_image(Epplet_gadget eg, char un_only);
static void    Epplet_draw_label(Epplet_gadget eg, char un_only);
static void    Epplet_draw_popup(Epplet_gadget gadget);
static void    Epplet_draw_popupbutton(Epplet_gadget eg);
static void    Epplet_popup_arrange_contents(Epplet_gadget gadget);
static void    Epplet_prune_events(XEvent *ev, int num);
static void    Epplet_handle_child(int num);
static void    Epplet_textbox_handle_keyevent(XEvent *ev, Epplet_gadget g);
static void    Epplet_find_instance(char *name);

ImlibData *
Epplet_get_imlib_data(void)
{
   return (id);
}

void 
Epplet_send_ipc(char *s)
{
   ECommsSend(s);
}

char *
Epplet_wait_for_ipc(void)
{
   return ECommsWaitForMessage();
}

void
Epplet_Init(char *name,
            char *version,
            char *info, 
            int w, int h,
            int argc, char **argv, char vertical)
{
   struct sigaction     sa;
   char                 s[1024];
   XSetWindowAttributes attr;
   Atom                 a;
   XTextProperty        xtp;
   XClassHint          *xch;
   XSizeHints           sh;
   struct utsname       ubuf;
   MWMHints             mwm;
   unsigned long        val;

   w *= 16;
   h *= 16;
   disp = XOpenDisplay(NULL);
   id = Imlib_init(disp);
   if (!disp)
     {
	fprintf(stderr, "Epplet Error: Cannot open display\n");
	exit(1);
     }
   ECommsSetup(disp);
   XSelectInput(disp, DefaultRootWindow(disp), PropertyChangeMask);

   /* Find the instance number for this instance and compose the name from it*/
   Epplet_find_instance(name);

   /* create a window with everythign set */
   attr.backing_store     = NotUseful;
   attr.override_redirect = False;
   attr.colormap          = Imlib_get_colormap(id);
   attr.border_pixel      = 0;
   attr.background_pixel  = 0;
   attr.save_under        = False;
   attr.event_mask        = StructureNotifyMask | ButtonPressMask |
      ButtonReleaseMask | PointerMotionMask | EnterWindowMask |
      LeaveWindowMask | KeyPressMask | KeyReleaseMask | ButtonMotionMask |
      ExposureMask | FocusChangeMask | PropertyChangeMask |
      VisibilityChangeMask;
   win = XCreateWindow(disp, DefaultRootWindow(disp), 0, 0, w, h, 0, 
		       id->x.depth, InputOutput, Imlib_get_visual(id),
		       CWOverrideRedirect | CWSaveUnder | CWBackingStore |
		       CWColormap | CWBackPixel | CWBorderPixel |
		       CWEventMask, &attr);

   /* set hints to be borderless */
   mwm.flags = MWM_HINTS_DECORATIONS;
   mwm.functions = 0;
   mwm.decorations = 0;
   mwm.inputMode = 0;
   mwm.status = 0;
   a = XInternAtom(disp, "_MOTIF_WM_HINTS", False); 
   XChangeProperty(disp, win, a, a, 32, PropModeReplace,
		   (unsigned char *)&mwm, sizeof(MWMHints) / 4);

   /* set the window title , name , class */
   XStoreName(disp, win, epplet_name);
   xch = XAllocClassHint();
   xch->res_name = epplet_name;
   xch->res_class = "Epplet";
   XSetClassHint(disp, win, xch);
   XFree(xch);
   /* set the size hints */
   sh.flags = PSize | PMinSize | PMaxSize;
   sh.width = w;
   sh.height = h;
   sh.min_width = w;
   sh.min_height = h;
   sh.max_width = w;
   sh.max_height = h;
   XSetWMNormalHints(disp, win, &sh);
   /* set the command hint */
   XSetCommand(disp, win, argv, argc);
   /* set the client machine name */
   if (!uname(&ubuf))
     {
	Esnprintf(s, sizeof(s), "%s", ubuf.nodename);
	xtp.encoding = XA_STRING;
	xtp.format = 8;
	xtp.value = (unsigned char *)s;
	xtp.nitems = strlen((char *)(xtp.value));
	XSetWMClientMachine(disp, win, &xtp);
     }
   /* set the icons name property */
   XSetIconName(disp, win, epplet_name);

   /* set sticky & arrange ignore */
   val = (1 << 0)/* | (1 << 9)*/;
   a = XInternAtom(disp, "_WIN_STATE", False);
   XChangeProperty(disp, win, a, XA_CARDINAL, 32, PropModeReplace,
		   (unsigned char *)&val, 1);
   /* set the default layer to below */
   val = 2;
   a = XInternAtom(disp, "_WIN_LAYER", False);
   XChangeProperty(disp, win, a, XA_CARDINAL, 32, PropModeReplace,
		   (unsigned char *)&val, 1);
   /* set skip focus, skip winlist dont cover skip taskbar flags */
   val = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 5);
   a = XInternAtom(disp, "_WIN_HINTS", False);
   XChangeProperty(disp, win, a, XA_CARDINAL, 32, PropModeReplace, 
		   (unsigned char *)&val, 1);
   win_vert = vertical;
   win_w = w;
   win_h = h;
   win_name = epplet_name;
   win_version = version;
   win_info = info;
   xid_context = XUniqueContext();   
   while (!comms_win)
     {
	ECommsSetup(disp);
	sleep(1);
     }
   Esnprintf(s, sizeof(s), "set clientname %s", win_name);
   ECommsSend(s);
   Esnprintf(s, sizeof(s), "set version %s", win_version);
   ECommsSend(s);
   Esnprintf(s, sizeof(s), "set info %s", win_info);
   ECommsSend(s);
   ESYNC;
   Epplet_background_properties(win_vert);
   sa.sa_handler = Epplet_handle_child;
   sa.sa_flags = SA_RESTART;
   sigemptyset(&sa.sa_mask);
   sigaction(SIGCHLD, &sa, (struct sigaction *)0);   
}

void
Epplet_cleanup(void)
{
   char s[1024];

   /* remove lock file */
   Esnprintf(s, sizeof(s), "%s/.lock_%i", conf_dir, epplet_instance);
   if (unlink(s) < 0)
     {
       char err[255];
       
       Esnprintf(err, sizeof(err), "Unable to remove lock file %s -- %s.\n",
	       s, strerror(errno));
       Epplet_dialog_ok(err);
     }
   
   /* save config settings */
   Epplet_save_config();

   /* Freeing all this memory right before you exit is pretty pointless.... -- mej */
#if 0
   /* free config stuff */
   if (config_dict)
     {
       int i;

       for (i = 0; i < config_dict->num_entries; i++)
	 {
	   if (config_dict->entries[i].key)
	     free(config_dict->entries[i].key);
	   if (config_dict->entries[i].value)
	     free(config_dict->entries[i].value);
	 }
       free(config_dict->entries);
       free(config_dict);
       config_dict = NULL;
     }

   if (conf_dir)
     {
       free(conf_dir);
       conf_dir = NULL;
    }
   if (epplet_name)
     {
       free(epplet_name);
       epplet_name = NULL;
     }
   if (epplet_cfg_file)
     {
       free(epplet_cfg_file);
       epplet_cfg_file = NULL;
     }
#endif

}

void
Epplet_show(void)
{
   XEvent               ev;
   
   XMapWindow(disp, win);
   /* wait for the window to map */
   XMaskEvent(disp, StructureNotifyMask, &ev);
}

void
Epplet_remember(void)
{
   char   s[1024];

   Esnprintf(s, sizeof(s), "remember %x none", (unsigned int)win);
   ECommsSend(s);
   Esnprintf(s, sizeof(s), "remember %x layer", (unsigned int)win);
   ECommsSend(s);
   Esnprintf(s, sizeof(s), "remember %x border", (unsigned int)win);
   ECommsSend(s);
   Esnprintf(s, sizeof(s), "remember %x location", (unsigned int)win);
   ECommsSend(s);
   Esnprintf(s, sizeof(s), "remember %x sticky", (unsigned int)win);
   ECommsSend(s);
   Esnprintf(s, sizeof(s), "remember %x shade", (unsigned int)win);
   ECommsSend(s);
   Esnprintf(s, sizeof(s), "remember %x group", (unsigned int)win);
   ECommsSend(s);
   Esnprintf(s, sizeof(s), "remember %x command", (unsigned int)win);
   ECommsSend(s);
}

void
Epplet_unremember(void)
{
   char   s[1024];

   Esnprintf(s, sizeof(s), "remember %x none", (unsigned int)win);
   ECommsSend(s);
   ESYNC;
}

Window
Epplet_get_main_window(void)
{
   return win;
}

void
Epplet_imageclass_apply(char *iclass, char *state, Window ww)
{
   char s[1024];
   
   Esnprintf(s, sizeof(s), "imageclass %s apply 0x%x %s", iclass, (unsigned int)ww, state);
   ECommsSend(s);
}

void
Epplet_imageclass_paste(char *iclass, char *state, Window ww, int x, int y, 
			int w, int h)
{
   Pixmap    p, m;   
   char      s[1024], *msg;
   static GC gc = 0;
   XGCValues gcv;
   
   Esnprintf(s, sizeof(s), "imageclass %s apply_copy 0x%x %s %i %i", iclass, (unsigned int)ww, state, w, h);
   ECommsSend(s);
   msg = ECommsWaitForMessage();
   if (msg)
     {
	sscanf(msg, "%x %x", (unsigned int *)&p, (unsigned int *)&m);
	free(msg);
	if (!gc)
	   gc = XCreateGC(disp, win, 0, &gcv);
	XSetClipMask(disp, gc, m);
	XSetClipOrigin(disp, gc, x, y);
	XCopyArea(disp, p, win, gc, 0, 0, w, h, x, y);   
	Esnprintf(s, sizeof(s), "imageclass %s free_pixmap 0x%x", iclass, (unsigned int)p);
	ECommsSend(s);
     }
}

void
Epplet_imageclass_get_pixmaps(char *iclass, char *state, Pixmap *p, Pixmap *m,
			      int w, int h)
{
   Pixmap    pp, mm;   
   char      s[1024], *msg;
   static GC gc = 0, mgc = 0;
   XGCValues gcv;
   
   Esnprintf(s, sizeof(s), "imageclass %s apply_copy 0x%x %s %i %i", iclass, (unsigned int)win, state, w, h);
   ECommsSend(s);
   msg = ECommsWaitForMessage();
   if (msg)
     {
	sscanf(msg, "%x %x", (unsigned int *)&pp, (unsigned int *)&mm);
	free(msg);
	if (pp)
	   *p = XCreatePixmap(disp, win, w, h, id->x.depth);
	else
	   *p = 0;
	if (mm)
	   *m = XCreatePixmap(disp, win, w, h, 1);
	else
	   *m = 0;
	if ((*p) && (!gc))
	   gc = XCreateGC(disp, *p, 0, &gcv);
	if ((*m) && (!mgc))
	   mgc = XCreateGC(disp, *m, 0, &gcv);
	if (*p)
	   XCopyArea(disp, pp, *p, gc, 0, 0, w, h, 0, 0);   
	if (*m)
	   XCopyArea(disp, mm, *m, mgc, 0, 0, w, h, 0, 0);
	Esnprintf(s, sizeof(s), "imageclass %s free_pixmap 0x%x", iclass, (unsigned int)pp);
	ECommsSend(s);
     }
}

void
Epplet_textclass_draw(char *iclass, char *state, Window ww, int x, int y, char *txt)
{
   char s[1024];
   
   Esnprintf(s, sizeof(s), "textclass %s apply 0x%x %i %i %s %s", iclass, 
	   (unsigned int)ww, x, y, state, txt);
   ECommsSend(s);
}

void
Epplet_textclass_get_size(char *iclass, int *w, int *h, char *txt)
{
   char s[1024], *msg = NULL;
   
   Esnprintf(s, sizeof(s), "textclass %s query_size %s", iclass, txt);
   ECommsSend(s);
   msg = ECommsWaitForMessage();
   if (msg)
     {
	sscanf(msg, "%i %i", w, h);
	free(msg);
     }
   else
     {
	*w = 0;
	*h = 0;
     }
}

void
Epplet_register_expose_handler(void (*func) 
			       (void *data, Window win, int x, int y, int w, int h),
			       void *data)
{
   expose_data = data;
   expose_func = func;
}

void
Epplet_register_move_resize_handler(void (*func) 
				    (void *data, Window win, int x, int y, int w, int h),
				    void *data)
{
   moveresize_data = data;
   moveresize_func = func;
}

void
Epplet_register_button_press_handler(void (*func) 
				     (void *data, Window win, int x, int y, int b),
				     void *data)
{
   buttonpress_data = data;
   buttonpress_func = func;
}

void
Epplet_register_button_release_handler(void (*func) 
				       (void *data, Window win, int x, int y, int b),
				       void *data)
{
   buttonrelease_data = data;
   buttonrelease_func = func;
}

void
Epplet_register_key_press_handler(void (*func) 
				  (void *data, Window win, char *key),
				  void *data)
{
   keypress_data = data;
   keypress_func = func;
}

void
Epplet_register_key_release_handler(void (*func) 
				    (void *data, Window win, char *key),
				    void *data)
{
   keyrelease_data = data;
   keyrelease_func = func;
}

void
Epplet_register_mouse_motion_handler(void (*func) 
				     (void *data, Window win, int x, int y),
				     void *data)
{
   mousemotion_data = data;
   mousemotion_func = func;
}

void
Epplet_register_mouse_enter_handler(void (*func) 
				    (void *data, Window win),
				    void *data)
{
   enter_data = data;
   enter_func = func;
}

void
Epplet_register_mouse_leave_handler(void (*func) 
				    (void *data, Window win),
				    void *data)
{
   leave_data = data;
   leave_func = func;
}

void
Epplet_register_focus_in_handler(void (*func) 
				 (void *data, Window win),
				 void *data)
{
   focusin_data = data;
   focusin_func = func;
}

void
Epplet_register_focus_out_handler(void (*func) 
				  (void *data, Window win),
				  void *data)
{
   focusout_data = data;
   focusout_func = func;
}

void
Epplet_register_event_handler(void (*func) 
			      (void *data, XEvent *ev),
			      void *data)
{
   event_data = data;
   event_func = func;
}

void
Epplet_register_comms_handler(void (*func) 
			      (void *data, char *s),
			      void *data)
{
   comms_data = data;
   comms_func = func;
}

Display *
Epplet_get_display(void)
{
   return disp;
}

static void
Epplet_handle_event(XEvent *ev)
{
   Epplet_gadget g = NULL;
   if (event_func)
      (*event_func) (ev, event_data);
   switch (ev->type)
     {
     case ClientMessage:
	  {
	     char *msg;
	     
	     msg = ECommsGet(ev);
	     if (msg)
	       {
		  if (comms_func)
		     (*comms_func) (comms_data, msg);
		  free(msg);
	       }
	  }
	break;
     case KeyPress:
	  {
	if (XFindContext(disp, ev->xkey.window, xid_context,
			 (XPointer *) & g) == XCNOENT)
	  g = NULL;

	if (g)
	  Epplet_event(g, ev);
	else
	  {
	     char *key;
	     
	     key = 
		XKeysymToString(XKeycodeToKeysym(disp, 
						 (KeyCode)ev->xkey.keycode, 
						 0));
	     if (keypress_func)
		(*keypress_func) (keypress_data, ev->xkey.window
				  , key);
	  }
	  }
	break;
     case KeyRelease:
	  {
	     char *key;
	     
	     key = 
		XKeysymToString(XKeycodeToKeysym(disp, 
						 (KeyCode)ev->xkey.keycode, 
						 0));
	     if (keyrelease_func)
		(*keyrelease_func) (keyrelease_data, ev->xkey.window,
				    key);
	  }
	break;
     case ButtonPress:
	if (XFindContext(disp, ev->xkey.window, xid_context, 
			 (XPointer *) &g) == XCNOENT)
	   g = NULL;
	if (g)
	   Epplet_event(g, ev);
	else	
	  {
	     if (buttonpress_func)
		(*buttonpress_func) (buttonpress_data, ev->xbutton.window, 
				     ev->xbutton.x, ev->xbutton.y, 
				     ev->xbutton.button);	     
	  }
	break;
     case ButtonRelease:
	if (XFindContext(disp, ev->xkey.window, xid_context, 
			 (XPointer *) &g) == XCNOENT)
	   g = NULL;
	if (g)
	   Epplet_event(g, ev);
	else
	  {
	     if (buttonrelease_func)
		(*buttonrelease_func) (buttonrelease_data, ev->xbutton.window, 
				       ev->xbutton.x, ev->xbutton.y, 
				       ev->xbutton.button);	     
	  }
	break;
     case MotionNotify:
	if (XFindContext(disp, ev->xkey.window, xid_context, 
			 (XPointer *) &g) == XCNOENT)
	   g = NULL;
	if (g)
	   Epplet_event(g, ev);
	else	
	  {
	     if (mousemotion_func)
		(*mousemotion_func) (mousemotion_data, ev->xmotion.window, 
				     ev->xmotion.x, ev->xmotion.y);
	  }
	break;
     case EnterNotify:
	if (XFindContext(disp, ev->xkey.window, xid_context, 
			 (XPointer *) &g) == XCNOENT)
	   g = NULL;
	if (g)
	   Epplet_event(g, ev);
	else	
	  {
	     if (enter_func)
		(*enter_func) (enter_data, ev->xcrossing.window);
	  }
	break;
     case LeaveNotify:
	if (XFindContext(disp, ev->xkey.window, xid_context, 
			 (XPointer *) &g) == XCNOENT)
	   g = NULL;
	if (g)
	   Epplet_event(g, ev);
	else	
	  {
	     if (leave_func)
		(*leave_func) (leave_data, ev->xcrossing.window);
	  }
	break;
     case FocusIn:
	if (focusin_func)
	   (*focusin_func) (focusin_data, ev->xfocus.window);
	break;
     case FocusOut:
	if (focusout_func)
	   (*focusout_func) (focusout_data, ev->xfocus.window);
	break;
     case Expose:
	if (expose_func)
	   (*expose_func) (expose_data, ev->xexpose.window,
			   ev->xexpose.x, ev->xexpose.y, 
			   ev->xexpose.width, ev->xexpose.height);
	break;
     case ConfigureNotify:
	if (moveresize_func)
	   (*moveresize_func) (moveresize_data, ev->xconfigure.window,
			       ev->xconfigure.x, ev->xconfigure.y, 
			       ev->xconfigure.width, ev->xconfigure.height);
	break;
     case PropertyNotify:
	if (CommsHandlePropertyNotify(ev))
	   Epplet_redraw();
	break;
     case DestroyNotify:
	CommsHandleDestroy(ev->xdestroywindow.window);
	break;
     default:
	break;
     }
}

void 
Epplet_timer(void (*func) (void *data), void *data, double in, char *name)
{
   ETimer *et, *ptr, *pptr;
   double  tally;

   Epplet_remove_timer(name);
   et = malloc(sizeof(ETimer));
   et->next = NULL;
   et->func = func;
   et->data = data;
   et->name = malloc(strlen(name) + 1);
   et->just_added = 1;
   et->in = in;
   memcpy(et->name, name, strlen(name) + 1);
   tally = 0.0;
   if (!q_first)
      q_first = et;
   else
     {
	pptr = NULL;
	ptr = q_first;
	tally = 0.0;
	while (ptr)
	  {
	     tally += ptr->in;
	     if (tally > in)
	       {
		  tally -= ptr->in;
		  et->next = ptr;
		  if (pptr)
		     pptr->next = et;
		  else
		     q_first = et;
		  et->in -= tally;
		  if (et->next)
		     et->next->in -= et->in;
		  return;
	       }
	     pptr = ptr;
	     ptr = ptr->next;
	  }
	if (pptr)
	   pptr->next = et;
	else
	   q_first = et;	
	et->in -= tally;
     }
}

void
Epplet_remove_timer(char *name)
{
   ETimer  *et, *ptr, *pptr;

   pptr = NULL;
   ptr = q_first;
   while (ptr)
     {
	et = ptr;
	if (!strcmp(et->name, name))
	  {
	     if (pptr)
		pptr->next = et->next;
	     else
		q_first = et->next;
	     if (et->next)
		et->next->in += et->in;
	     if (et->name)
		free(et->name);
	     if (et)
		free(et);
	     return;
	  }
	pptr = ptr;
	ptr = ptr->next;
     }
}

void *
Epplet_timer_get_data(char *name)
{
   ETimer  *et, *ptr;

   ptr = q_first;
   while (ptr)
     {
	et = ptr;
	if (!strcmp(et->name, name))
	  {
	 return et->data;
	  }
	ptr = ptr->next;
     }
   return NULL;
}

static void
Epplet_handle_timer(void)
{
   ETimer *et;

   if (!q_first)
      return;
   et = q_first;
   q_first = q_first->next;
   (*(et->func)) (et->data);
   if (et->name)
      free(et->name);
   if (et)
      free(et);
}

double
Epplet_get_time(void)
{
   struct timeval      timev;
   
   gettimeofday(&timev, NULL);
   return (double)timev.tv_sec + (((double)timev.tv_usec) / 1000000);
}

static ETimer *
Epplet_get_first(void)
{
   return q_first;
}		 

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

void
Epplet_prune_events(XEvent *ev, int num)
{
   int     i, j;
   char    found, remember;

   /* find only the last moption event */
   found = 0;
   for (i = num - 1; i >= 0; i--)
     {
	if (ev[i].type == MotionNotify)
	  {
	     if (!found)
		found = 1;
	     else
		ev[i].type = 0;
	  }
     }
   /* find all expose events and then all following expose events for that */
   /* window then do a bounding box grow of the original expose event */
   for (i = num - 1; i >= 0; i--)
     {
	if (ev[i].type == Expose)
	  {
	     for (j = i - 1; j >= 0; j--)
	       {
		  if ((ev[j].type == Expose) && 
		      (ev[j].xexpose.window == ev[i].xexpose.window))
		    {
		       if (ev[j].xexpose.x < ev[i].xexpose.x)
			 {
			    ev[i].xexpose.width += 
			       (ev[i].xexpose.x - ev[j].xexpose.x);
			    ev[i].xexpose.x = ev[j].xexpose.x;
			 }
		       if ((ev[j].xexpose.x + ev[j].xexpose.width) > 
			   (ev[i].xexpose.x + ev[i].xexpose.width))
			  ev[i].xexpose.width += 
			  (ev[j].xexpose.x + ev[j].xexpose.width) -
			  (ev[i].xexpose.x + ev[i].xexpose.width);

		       if (ev[j].xexpose.y < ev[i].xexpose.y)
			 {
			    ev[i].xexpose.height += 
			       (ev[i].xexpose.y - ev[j].xexpose.y);
			    ev[i].xexpose.y = ev[j].xexpose.y;
			 }
		       if ((ev[j].xexpose.y + ev[j].xexpose.height) > 
			   (ev[i].xexpose.y + ev[i].xexpose.height))
			  ev[i].xexpose.height += 
			  (ev[j].xexpose.y + ev[j].xexpose.height) -
			  (ev[i].xexpose.y + ev[i].xexpose.height);
		       
		       ev[j].type = 0;
		    }
	       }
	  }
     }
   /* any reason to remember the window properties? */
   remember = 0;
   for (i = 0; i < num; i++)
     {
	if ((ev[i].type == ConfigureNotify) && (ev->xconfigure.window == win))
	   remember = 1;
	if ((ev[i].type == PropertyNotify) && (ev->xproperty.window == win))
	   remember = 1;
     }
   if (remember)
      Epplet_remember();
}

void
Epplet_Loop(void)
{
   int            xfd, fdsize, count;
   double         t1 = 0.0, t2 = 0.0, pt;
   ETimer        *et;
   fd_set         fdset;
   struct timeval tval;
   
   xfd = ConnectionNumber(disp);
   fdsize = xfd + 1;
   pt = Epplet_get_time();
   for (;;)
     {	
	XEvent *evs = NULL;
	int     evs_num = 0, i;
	
	XFlush(disp);
	t1 = Epplet_get_time();
	t2 = t1 - pt;
	pt = t1;
	while (XPending(disp))
	  {
	     evs_num++;
	     if (evs)
		evs = realloc(evs, sizeof(XEvent) * evs_num);
	     else
		evs = malloc(sizeof(XEvent));
	     XNextEvent(disp, &(evs[evs_num - 1]));
	  }
	if (evs)
	  {
	     Epplet_prune_events(evs, evs_num);
	     for (i = 0; i < evs_num; i++)
	       {
		  if (evs[i].type > 0)
		     Epplet_handle_event(&(evs[i]));
	       }
	     free(evs);
	     evs = NULL;
	     evs_num = 0;
	  }
	XFlush(disp);
	FD_ZERO(&fdset);
	FD_SET(xfd, &fdset);
	et = Epplet_get_first();
	count = 0;
	if (et)
	  {
	     if (et->just_added)
	       {
		  et->just_added = 0;
		  t1 = et->in;
	       }
	     else
	       {
		  t1 = et->in - t2;
		  if (t1 < 0.0)
		     t1 = 0.0;
		  et->in = t1;
	       }
	     tval.tv_sec = (long)t1;
	     tval.tv_usec = (long)((t1 - ((double)tval.tv_sec)) * 1000000);
	     if (tval.tv_sec < 0)
		tval.tv_sec = 0;
	     if (tval.tv_usec <= 1000)
		tval.tv_usec = 1000;
	     count = select(fdsize, &fdset, NULL, NULL, &tval);
	  }
	else
	   count = select(fdsize, &fdset, NULL, NULL, NULL);
	if (count < 0)
	  {
	     if ((errno == ENOMEM) || (errno == EINVAL) || (errno == EBADF))
		exit(1);
	  }
	else
	       {
		if ((et) && (count == 0))
		   Epplet_handle_timer();
	       }
     }
}

static void
ECommsSetup(Display *d)
{
   dd = d;
   root = DefaultRootWindow(dd);
   if (!my_win)
     {
	my_win = XCreateSimpleWindow(dd, root, -100, -100, 5, 5, 0, 0, 0);
	XSelectInput(dd, my_win, StructureNotifyMask | SubstructureNotifyMask);
     }
   CommsFindCommsWindow();
}

static void
CommsHandleDestroy(Window win)
{
   if (win == comms_win)
      comms_win = 0;
}

static int
CommsHandlePropertyNotify(XEvent *ev)
{
   Atom a = 0;
   
   if (comms_win)
      return 0;
   if (!a)
      a = XInternAtom(dd, "ENLIGHTENMENT_COMMS", True);
   if (a == ev->xproperty.atom)
      CommsFindCommsWindow();
   if (comms_win)
      return 1;
   return 0;
}

static void
CommsFindCommsWindow(void)
{
   unsigned char      *s;
   Atom                a, ar;
   unsigned long       num, after;
   int                 format;
   Window              rt;
   int                 dint;
   unsigned int        duint;

   a = XInternAtom(dd, "ENLIGHTENMENT_COMMS", True);
   if (a != None)
     {
	s = NULL;
	XGetWindowProperty(dd, root, a, 0, 14, False, AnyPropertyType, &ar,
			   &format, &num, &after, &s);
	if (s)
	  {
	     sscanf((char *)s, "%*s %x", (unsigned int *)&comms_win);
	     XFree(s);
	  }
	else
	   (comms_win = 0);
	if (comms_win)
	  {
	     if (!XGetGeometry(dd, comms_win, &rt, &dint, &dint,
			       &duint, &duint, &duint, &duint))
		comms_win = 0;
	     s = NULL;
	     if (comms_win)
	       {
		  XGetWindowProperty(dd, comms_win, a, 0, 14, False,
				     AnyPropertyType, &ar, &format, &num, 
				     &after, &s);
		  if (s)
		     XFree(s);
		  else
		     comms_win = 0;
	       }
	  }
     }
   if (comms_win)
      XSelectInput(dd, comms_win, 
		   StructureNotifyMask | SubstructureNotifyMask);
}

static void
ECommsSend(char *s)
{
   char                ss[21];
   int                 i, j, k, len;
   XEvent              ev;
   Atom                a = 0;

   if (!s)
      return;
   len = strlen(s);
   if (!a)
      a = XInternAtom(dd, "ENL_MSG", False);
   ev.xclient.type = ClientMessage;
   ev.xclient.serial = 0;
   ev.xclient.send_event = True;
   ev.xclient.window = comms_win;
   ev.xclient.message_type = a;
   ev.xclient.format = 8;

   for (i = 0; i < len + 1; i += 12)
     {
	Esnprintf(ss, sizeof(ss), "%8x", (int)my_win);
	for (j = 0; j < 12; j++)
	  {
	     ss[8 + j] = s[i + j];
	     if (!s[i + j])
		j = 12;
	  }
	ss[20] = 0;
	for (k = 0; k < 20; k++)
	   ev.xclient.data.b[k] = ss[k];
	XSendEvent(dd, comms_win, False, 0, (XEvent *) & ev);
     }
}


static Bool 
ev_check(Display *d, XEvent *ev, XPointer p)
{
   if (((ev->type == ClientMessage) && (ev->xclient.window == my_win)) ||
       ((ev->type == DestroyNotify) && 
	(ev->xdestroywindow.window == comms_win)))
      return True;
   return False;
   d = NULL;
   p = NULL;
}

static char *
ECommsWaitForMessage(void)
{
   XEvent ev;
   char *msg = NULL;

   while ((!msg) && (comms_win))
     {
	XIfEvent(dd, &ev, ev_check, NULL);
	if (ev.type == DestroyNotify)
	   comms_win = 0;
	else
	   msg = ECommsGet(&ev);
     }
   return msg;
}		      

static char               *
ECommsGet(XEvent * ev)
{
   char                s[13], s2[9], *msg = NULL;
   int                 i;
   Window              win;
   static char        *c_msg = NULL;

   if (!ev) 
      return NULL;
   if (ev->type != ClientMessage)
      return NULL;
   s[12] = 0;
   s2[8] = 0;
   msg = NULL;
   for (i = 0; i < 8; i++)
      s2[i] = ev->xclient.data.b[i];
   for (i = 0; i < 12; i++)
      s[i] = ev->xclient.data.b[i + 8];
   sscanf(s2, "%x", (int *)&win);
   if (win == comms_win)
     {
	if (c_msg)
	  {
	     c_msg = realloc(c_msg, strlen(c_msg) + strlen(s) + 1);
	     if (!c_msg)
		return NULL;
	     strcat(c_msg, s);
	  }
	else
	  {
	     c_msg = malloc(strlen(s) + 1);
	     if (!c_msg)
		return NULL;
	     strcpy(c_msg, s);
	  }
	if (strlen(s) < 12)
	  {
	     msg = c_msg;
	     c_msg = NULL;
	  }
     }
   return msg;
}

int 
Epplet_get_color(int r, int g, int b)
{
   int rr, gg, bb;
   
   rr = r;
   gg = g;
   bb = b;   
   return Imlib_best_color_match(id, &rr, &gg, &bb);
}
















typedef enum gad_type
{
   E_BUTTON,
   E_DRAWINGAREA,
   E_TEXTBOX,
   E_HSLIDER,
   E_VSLIDER,
   E_TOGGLEBUTTON,
   E_POPUPBUTTON,
   E_POPUP,
   E_IMAGE,
   E_LABEL,
   E_HBAR,
   E_VBAR
} GadType;

typedef struct gad_general
{
   GadType type;
   char       visible;
} GadGeneral;

typedef struct
{
   GadGeneral general;
   int        x, y, w, h;
   char      *label;
   char      *image;
   char       hilited;
   char       clicked;
   char       pop;
   Epplet_gadget pop_parent;
   char      *std;
   void     (*func) (void *data);
   void      *data;
   Window     win;
   Pixmap     pmap, mask;
} GadButton;

static char *
Estrdup(char *s)
{
   char *ss;
   int len;
   
   if (!s)
      return NULL;
   len = strlen(s);
   ss = malloc(len + 1);
   memcpy(ss, s, len + 1);
   return ss;
}

void
Epplet_paste_image(char *image, Window ww, int x, int y)
{
   ImlibImage *im;
   
   im = Imlib_load_image(id, image);
   if (im)
     {
	Imlib_paste_image(id, im, ww, x, y, im->rgb_width, im->rgb_height);
	Imlib_destroy_image(id, im);
     }
}

void
Epplet_paste_image_size(char *image, Window ww, int x, int y, int w, int h)
{
   ImlibImage *im;
   
   im = Imlib_load_image(id, image);
   if (im)
     {
	Imlib_paste_image(id, im, ww, x, y, w, h);
	Imlib_destroy_image(id, im);
     }
}

void
Epplet_add_gad(Epplet_gadget gadget)
{
   gad_num++;
   if (gads)
      gads = realloc(gads, gad_num * sizeof(Epplet_gadget));
   else
      gads = malloc(gad_num * sizeof(Epplet_gadget));
   gads[gad_num - 1] = gadget;
}

void
Epplet_del_gad(Epplet_gadget gadget)
{
   int i, j;
   
   for (i = 0; i < gad_num; i++)
     {
	if (gads[i] == gadget)
	  {
	     for (j = i; j < gad_num - 1; j++)
		gads[j] = gads[j + 1];
	     gad_num--;
	     if (gad_num > 0)
		gads = realloc(gads, gad_num * sizeof(Epplet_gadget));
	     else
	       {
		  free(gads);
		  gads = NULL;
	       }
	  }
     }
   
}

typedef struct
{
  GadGeneral          general;
  int                 x, y, w, h, cursor_pos, text_offset;
  char               *image;
  char               *contents;
  char                hilited;
  char                size;
  void                (*func) (void *data);
  void               *data;
  Window              win;
  Pixmap              pmap, mask;
}
GadTextBox;

Epplet_gadget
Epplet_create_textbox(char *image, char *contents, int x, int y,
		      int w, int h, char size, void (*func) (void *data),
		      void *data)
{
  GadTextBox         *g;
  XSetWindowAttributes attr;

  g = malloc(sizeof(GadTextBox));
  g->general.type = E_TEXTBOX;
  g->x = x;
  g->y = y;
  g->contents = Estrdup(contents);
  g->cursor_pos = contents != NULL ? strlen(contents) : 0;
  g->text_offset = 0;
  g->w = w;
  g->h = h;
  g->size = size;
  g->func = func;
  g->data = data;
  g->pmap = 0;
  g->mask = 0;
  g->image = Estrdup(image);
  g->hilited = 0;
  attr.backing_store = NotUseful;
  attr.override_redirect = False;
  attr.colormap = Imlib_get_colormap(id);
  attr.border_pixel = 0;
  attr.background_pixel = 0;
  attr.save_under = False;
  attr.event_mask =
    EnterWindowMask | LeaveWindowMask | KeyPressMask | KeyReleaseMask;
  g->general.visible = 0;
  g->win = XCreateWindow(disp, win, x, y, g->w, g->h, 0,
			 id->x.depth, InputOutput, Imlib_get_visual(id),
			 CWOverrideRedirect | CWSaveUnder | CWBackingStore |
			 CWColormap | CWBackPixel | CWBorderPixel |
			 CWEventMask, &attr);
  XSaveContext(disp, g->win, xid_context, (XPointer) g);
  Epplet_add_gad((Epplet_gadget) g);
  return (Epplet_gadget) g;
}

char               *
Epplet_textbox_contents(Epplet_gadget eg)
{
  GadTextBox         *g;

  g = (GadTextBox *) eg;

  return g->contents;
}

void
Epplet_reset_textbox(Epplet_gadget eg)
{
  GadTextBox *g;

  g = (GadTextBox *) eg;
  if (g->contents) 
    {
      free(g->contents);
      g->contents = NULL;
    }
  g->cursor_pos = g->text_offset = 0;
}

void
Epplet_draw_textbox(Epplet_gadget eg)
{
  GadTextBox         *g;
  char               *state;

  g = (GadTextBox *) eg;

  if (g->hilited)
    state = "hilited";
  else
    state = "normal";

  if (g->pmap)
    XFreePixmap(disp, g->pmap);
  if (g->mask)
    XFreePixmap(disp, g->mask);
  g->pmap = 0;
  g->mask = 0;
  Epplet_imageclass_get_pixmaps("EPPLET_BUTTON", "clicked",
				&(g->pmap), &(g->mask), g->w, g->h);
  if (g->image)
    {
      ImlibImage         *im;

      ESYNC;
      im = Imlib_load_image(id, g->image);
      if (im)
	{
	  int                 x, y;

	  x = (g->w - im->rgb_width) / 2;
	  y = (g->h - im->rgb_height) / 2;
	  Imlib_paste_image(id, im, g->pmap, x, y,
			    im->rgb_width, im->rgb_height);
	  Imlib_destroy_image(id, im);
	}
    }
  if (g->contents)
    {
      int                 x, y, w, h;
      char               *s;

      s = &g->contents[g->text_offset];

      x = 2;

      switch (g->size)
	{
	case 0:
	  Epplet_textclass_get_size("EPPLET_BUTTON", &w, &h, s);
	  s[w] = '\0';
	  y = (g->h - h) / 2;
	  Epplet_textclass_draw("EPPLET_BUTTON", state, g->pmap, x, y, s);
	  break;
	case 1:
	  Epplet_textclass_get_size("EPPLET_TEXT_TINY", &w, &h, s);
	  //s[w-1] = '\0';
	  y = (g->h - h) / 2;
	  Epplet_textclass_draw("EPPLET_TEXT_TINY", state, g->pmap, x, y, s);
	  break;
	case 2:
	  Epplet_textclass_get_size("EPPLET_TEXT_MEDIUM", &w, &h, g->contents);
	  s[w] = '\0';
	  y = (g->h - h) / 2;
	  Epplet_textclass_draw("EPPLET_TEXT_MEDIUM", state, g->pmap, x, y, s);
	  break;
	case 3:
	  Epplet_textclass_get_size("EPPLET_TEXT_LARGE", &w, &h, g->contents);
	  s[w] = '\0';
	  y = (g->h - h) / 2;
	  Epplet_textclass_draw("EPPLET_TEXT_LARGE", state, g->pmap, x, y, s);
	  break;
	}
    }
  ESYNC;
  XSetWindowBackgroundPixmap(disp, g->win, g->pmap);
  XShapeCombineMask(disp, g->win, ShapeBounding, 0, 0, g->mask, ShapeSet);
  XClearWindow(disp, g->win);
}

static void
Epplet_textbox_handle_keyevent(XEvent *ev, Epplet_gadget gadget)
{
  int                 len;
  static char         kbuf[20];
  KeySym              keysym;
  XKeyEvent          *kev;
  GadTextBox         *g;

  kev = (XKeyEvent *) ev;
  g = (GadTextBox *) gadget;

  len = XLookupString(&ev->xkey, (char *) kbuf, sizeof(kbuf), &keysym, NULL);
  /* Convert unmapped Latin2-4 keysyms into Latin1 characters */
  if (!len && (keysym >= 0x0100) && (keysym < 0x0400)) {
    len = 1;
    kbuf[0] = (keysym & 0xff);
  }

  if (len <= 0 || len > (int) sizeof(kbuf))
    return;
  kbuf[len] = 0;

  if (*kbuf == '\r' || *kbuf == '\n')
    {
      if (g->func)
        (*(g->func)) (g->data);
    }
  else if (*kbuf == '\b')
    {
      if (g->contents && *(g->contents))
        {
          len = strlen(g->contents) - 1;
          g->contents[len] = 0;
	  g->cursor_pos--;
        }
    }
  else 
    {
      if (g->contents != NULL)
        {
          g->contents = (char *) realloc(g->contents, (strlen(g->contents) + len + 1));
        }
      else
        {
          g->contents = (char *) malloc(len + 1);
          *(g->contents) = 0;
        }
      strcat(g->contents, kbuf);
      if (g->cursor_pos <= g->w)
        g->cursor_pos += len;
      else
        g->text_offset += len;
    }
}

Epplet_gadget 
Epplet_create_button(char *label, char *image, int x, int y,
		     int w, int h, char *std, Window parent, 
		     Epplet_gadget pop_parent, 
		     void (*func) (void *data), void *data)
{
   GadButton *g;
   XSetWindowAttributes attr;
   
   g = malloc(sizeof(GadButton));
   g->general.type = E_BUTTON;
   g->x = x;
   g->y = y;
   g->std = Estrdup(std);
   if (g->std)
     {
	g->w = 12;
	g->h = 12;
     }
   else
     {
	g->w = w;
	g->h = h;
     }
   g->func = func;
   g->data = data;
   g->pmap = 0;
   g->mask = 0;
   g->label = Estrdup(label);
   g->image = Estrdup(image);
   g->hilited = 0;
   g->clicked = 0;
   g->pop = 0;
   g->pop_parent = pop_parent;
   attr.backing_store     = NotUseful;
   attr.override_redirect = False;
   attr.colormap          = Imlib_get_colormap(id);
   attr.border_pixel      = 0;
   attr.background_pixel  = 0;
   attr.save_under        = False;
   attr.event_mask        = ButtonPressMask | ButtonReleaseMask | 
      EnterWindowMask | LeaveWindowMask;
   g->general.visible = 0;
   if (parent)
     {
	g->win = XCreateWindow(disp, parent, x, y, g->w, g->h, 0, 
			       id->x.depth, InputOutput, Imlib_get_visual(id),
			       CWOverrideRedirect | CWSaveUnder | CWBackingStore |
			       CWColormap | CWBackPixel | CWBorderPixel |
			       CWEventMask, &attr);
	g->pop = 1;
     }
   else
      g->win = XCreateWindow(disp, win, x, y, g->w, g->h, 0, 
			     id->x.depth, InputOutput, Imlib_get_visual(id),
			     CWOverrideRedirect | CWSaveUnder | CWBackingStore |
			     CWColormap | CWBackPixel | CWBorderPixel |
			     CWEventMask, &attr);
   XSaveContext(disp, g->win, xid_context, (XPointer) g);
   Epplet_add_gad((Epplet_gadget)g);
   return (Epplet_gadget) g;
}

void
Epplet_draw_button(Epplet_gadget eg)
{
   GadButton *g;
   char *state;
   
   g = (GadButton *)eg;
   if (g->hilited)
     {
	if (g->clicked)
	   state = "clicked";
	else
	   state = "hilited";
     }
   else
     {
	if (g->clicked)
	   state = "clicked";
	else
	   state = "normal";
     }
   if (g->pmap)
      XFreePixmap(disp, g->pmap);
   if (g->mask)
      XFreePixmap(disp, g->mask);
   g->pmap = 0;
   g->mask = 0;
   if (g->std)
     {
	char s[1024];
	
	Esnprintf(s, sizeof(s), "EPPLET_%s", g->std);
	Epplet_imageclass_get_pixmaps(s, state, 
				      &(g->pmap), &(g->mask), g->w, g->h);
     }
   else if (g->pop)
     {
	Epplet_imageclass_get_pixmaps("EPPLET_POPUP_ENTRY", state, 
				      &(g->pmap), &(g->mask), g->w, g->h);
	if (g->image)
	  {
	     ImlibImage *im;
	     
	     ESYNC;
	     im = Imlib_load_image(id, g->image);
	     if (im)
	       {
		  int x, y;
		  
		  x = (g->w - im->rgb_width) / 2;
		  y = (g->h - im->rgb_height) / 2;
		  Imlib_paste_image(id, im, g->pmap, x, y, 
				    im->rgb_width, im->rgb_height);
		  Imlib_destroy_image(id, im);
	       }
	  }
	if (g->label)
	  {
	     int x, y, w, h;
	     
	     Epplet_textclass_get_size("EPPLET_POPUP", &w, &h, g->label);
	     x = (g->w - w) / 2;
	     y = (g->h - h) / 2;
	     Epplet_textclass_draw("EPPLET_POPUP", state, g->pmap, x, y, g->label);
	  }
     }
   else
     {
	Epplet_imageclass_get_pixmaps("EPPLET_BUTTON", state, 
				      &(g->pmap), &(g->mask), g->w, g->h);
	if (g->image)
	  {
	     ImlibImage *im;
	     
	     ESYNC;
	     im = Imlib_load_image(id, g->image);
	     if (im)
	       {
		  int x, y;
		  
		  x = (g->w - im->rgb_width) / 2;
		  y = (g->h - im->rgb_height) / 2;
		  Imlib_paste_image(id, im, g->pmap, x, y, 
				    im->rgb_width, im->rgb_height);
		  Imlib_destroy_image(id, im);
	       }
	  }
	if (g->label)
	  {
	     int x, y, w, h;
	     
	     Epplet_textclass_get_size("EPPLET_BUTTON", &w, &h, g->label);
	     x = (g->w - w) / 2;
	     y = (g->h - h) / 2;
	     Epplet_textclass_draw("EPPLET_BUTTON", state, g->pmap, x, y, g->label);
	  }
     }
   ESYNC;
   XSetWindowBackgroundPixmap(disp, g->win, g->pmap);
   XShapeCombineMask(disp, g->win, ShapeBounding, 0, 0, g->mask, ShapeSet);   
   XClearWindow(disp, g->win);
}

typedef struct
{
   GadGeneral general;
   int        x, y, w, h;
   char      *label;
   char      *image;
   char       hilited;
   char       clicked;
   int       *val;
   void     (*func) (void *data);
   void      *data;
   Window     win;
   Pixmap     pmap, mask;
} GadToggleButton;

Epplet_gadget 
Epplet_create_togglebutton(char *label, char *image, int x,
			   int y, int w, int h, int *val,
			   void (*func) (void *data),
			   void *data)
{
   GadToggleButton *g;
   XSetWindowAttributes attr;
   
   g = malloc(sizeof(GadToggleButton));
   g->general.type = E_TOGGLEBUTTON;
   g->x = x;
   g->y = y;
   g->w = w;
   g->h = h;
   g->func = func;
   g->data = data;
   g->pmap = 0;
   g->mask = 0;
   g->val = val;
   g->label = Estrdup(label);
   g->image = Estrdup(image);
   g->hilited = 0;
   g->clicked = 0;
   attr.backing_store     = NotUseful;
   attr.override_redirect = False;
   attr.colormap          = Imlib_get_colormap(id);
   attr.border_pixel      = 0;
   attr.background_pixel  = 0;
   attr.save_under        = False;
   attr.event_mask        = ButtonPressMask | ButtonReleaseMask | 
      EnterWindowMask | LeaveWindowMask;
   g->general.visible = 0;
   g->win = XCreateWindow(disp, win, x, y, w, h, 0, 
			  id->x.depth, InputOutput, Imlib_get_visual(id),
			  CWOverrideRedirect | CWSaveUnder | CWBackingStore |
			  CWColormap | CWBackPixel | CWBorderPixel |
			  CWEventMask, &attr);
   XSaveContext(disp, g->win, xid_context, (XPointer) g);
   Epplet_add_gad((Epplet_gadget)g);
   return (Epplet_gadget) g;
}

void
Epplet_draw_togglebutton(Epplet_gadget eg)
{
   GadToggleButton *g;
   char *state;
   
   g = (GadToggleButton *)eg;
   if (g->hilited)
     {
	if (g->clicked)
	   state = "clicked";
	else
	   state = "hilited";
     }
   else
     {
	if (g->clicked)
	   state = "clicked";
	else
	   state = "normal";
     }
   if (g->pmap)
      XFreePixmap(disp, g->pmap);
   if (g->mask)
      XFreePixmap(disp, g->mask);
   g->pmap = 0;
   g->mask = 0;
   if (*(g->val))
      Epplet_imageclass_get_pixmaps("EPPLET_TOGGLEBUTTON_ON", state, 
				    &(g->pmap), &(g->mask), g->w, g->h);
   else
      Epplet_imageclass_get_pixmaps("EPPLET_TOGGLEBUTTON_OFF", state, 
				    &(g->pmap), &(g->mask), g->w, g->h);
   if (g->image)
     {
	ImlibImage *im;
	
	ESYNC;
	im = Imlib_load_image(id, g->image);
	if (im)
	  {
	     int x, y;
	     
	     x = (g->w - im->rgb_width) / 2;
	     y = (g->h - im->rgb_height) / 2;
	     Imlib_paste_image(id, im, g->pmap, x, y, 
			       im->rgb_width, im->rgb_height);
	     Imlib_destroy_image(id, im);
	  }
     }
   if (g->label)
     {
	int x, y, w, h;
	
	if (*(g->val))
	  {
	     Epplet_textclass_get_size("EPPLET_TOGGLEBUTTON_ON", &w, &h, g->label);
	     x = (g->w - w) / 2;
	     y = (g->h - h) / 2;
	     Epplet_textclass_draw("EPPLET_TOGGLEBUTTON_ON", state, g->pmap, x, y, g->label);
	  }
	else
	  {
	     Epplet_textclass_get_size("EPPLET_TOGGLEBUTTON_OFF", &w, &h, g->label);
	     x = (g->w - w) / 2;
	     y = (g->h - h) / 2;
	     Epplet_textclass_draw("EPPLET_TOGGLEBUTTON_OFF", state, g->pmap, x, y, g->label);
	  }
     }
   ESYNC;
   XSetWindowBackgroundPixmap(disp, g->win, g->pmap);
   XShapeCombineMask(disp, g->win, ShapeBounding, 0, 0, g->mask, ShapeSet);   
   XClearWindow(disp, g->win);
}

typedef struct
{
   GadGeneral general;
   int        x, y, w, h;
   Window     win;
   Window     win_in;
} GadDrawingArea;

Epplet_gadget 
Epplet_create_drawingarea(int x, int y, int w, int h)
{
   GadDrawingArea *g;
   XSetWindowAttributes attr;
   
   g = malloc(sizeof(GadDrawingArea));
   g->general.type = E_DRAWINGAREA;
   g->x = x;
   g->y = y;
   g->w = w;
   g->h = h;
   attr.backing_store     = NotUseful;
   attr.override_redirect = False;
   attr.colormap          = Imlib_get_colormap(id);
   attr.border_pixel      = 0;
   attr.background_pixel  = 0;
   attr.save_under        = False;
   attr.event_mask        = 0;
   g->general.visible = 0;
   g->win = XCreateWindow(disp, win, x, y, w, h, 0, 
			  id->x.depth, InputOutput, Imlib_get_visual(id),
			  CWOverrideRedirect | CWSaveUnder | CWBackingStore |
			  CWColormap | CWBackPixel | CWBorderPixel |
			  CWEventMask, &attr);
   attr.event_mask        = ButtonPressMask |
      ButtonReleaseMask | PointerMotionMask | EnterWindowMask |
      LeaveWindowMask | KeyPressMask | KeyReleaseMask | ButtonMotionMask |
      ExposureMask;
   g->win_in = XCreateWindow(disp, g->win, 2, 2, w - 4, h - 4, 0, 
			     id->x.depth, InputOutput, Imlib_get_visual(id),
			     CWOverrideRedirect | CWSaveUnder | CWBackingStore |
			     CWColormap | CWBackPixel | CWBorderPixel |
			     CWEventMask, &attr);
   XSetWindowBackgroundPixmap(disp, g->win_in, ParentRelative);
   XMapWindow(disp, g->win_in);
   Epplet_add_gad((Epplet_gadget)g);
   return (Epplet_gadget) g;
}

void
Epplet_draw_drawingarea(Epplet_gadget eg)
{
   GadDrawingArea *g;
   
   g = (GadDrawingArea *)eg;
   Epplet_imageclass_apply("EPPLET_DRAWINGAREA", "normal", g->win);
}

typedef struct
{
   GadGeneral general;
   int        x, y, w, h;
   int        min, max;
   int        step, jump;
   char       hilited;
   char       clicked;
   int       *val;
   void     (*func) (void *data);
   void      *data;
   Window     win;
   Window     win_knob;
} GadHSlider;

Epplet_gadget 
Epplet_create_hslider(int x, int y, int len, int min, int max,
		      int step, int jump, int *val,
		      void (*func) (void *data), void *data)
{
   GadHSlider *g;
   XSetWindowAttributes attr;

   if (len < 9)
      len = 9;
   g = malloc(sizeof(GadHSlider));
   g->general.type = E_HSLIDER;
   g->x = x;
   g->y = y;
   g->w = len;
   g->h = 8;
   g->max = max;
   g->min = min;
   g->step = step;
   g->jump = jump;
   g->val = val;
   g->func = func;
   g->data = data;
   g->hilited = 0;
   g->clicked = 0;
   attr.backing_store     = NotUseful;
   attr.override_redirect = False;
   attr.colormap          = Imlib_get_colormap(id);
   attr.border_pixel      = 0;
   attr.background_pixel  = 0;
   attr.save_under        = False;
   attr.event_mask        = ButtonPressMask | ButtonReleaseMask;
   g->general.visible = 0;
   g->win = XCreateWindow(disp, win, x, y, len, 8, 0, 
			  id->x.depth, InputOutput, Imlib_get_visual(id),
			  CWOverrideRedirect | CWSaveUnder | CWBackingStore |
			  CWColormap | CWBackPixel | CWBorderPixel |
			  CWEventMask, &attr);
   attr.event_mask        = ButtonPressMask | ButtonReleaseMask | 
      PointerMotionMask | EnterWindowMask | LeaveWindowMask | ButtonMotionMask;
   g->win_knob = XCreateWindow(disp, win, x, y, 8, 8, 0, 
			       id->x.depth, InputOutput, Imlib_get_visual(id),
			       CWOverrideRedirect | CWSaveUnder | CWBackingStore |
			       CWColormap | CWBackPixel | CWBorderPixel |
			       CWEventMask, &attr);
   XSaveContext(disp, g->win, xid_context, (XPointer) g);
   XSaveContext(disp, g->win_knob, xid_context, (XPointer) g);
   Epplet_add_gad((Epplet_gadget)g);
   return (Epplet_gadget) g;
}

void
Epplet_draw_hslider(Epplet_gadget eg)
{
   GadHSlider *g;
   char *state;
      
   g = (GadHSlider *)eg;
   if (g->hilited)
     {
	if (g->clicked)
	   state = "clicked";
	else
	   state = "hilited";
     }
   else
     {
	if (g->clicked)
	   state = "clicked";
	else
	   state = "normal";
     }
   Epplet_imageclass_apply("EPPLET_HSLIDER_BASE", "normal", 
			   g->win);
   XMoveWindow(disp, g->win_knob, 
	       g->x + ((g->w - 8) * (*(g->val))) / (g->max - g->min + 1),
	       g->y);
   Epplet_imageclass_apply("EPPLET_HSLIDER_KNOB", state, 
			   g->win_knob);
}

typedef struct
{
   GadGeneral general;
   int        x, y, w, h;
   int        min, max;
   int        step, jump;
   char       hilited;
   char       clicked;
   int       *val;
   void     (*func) (void *data);
   void      *data;
   Window     win;
   Window     win_knob;
} GadVSlider;


Epplet_gadget 
Epplet_create_vslider(int x, int y, int len, int min, int max,
		      int step, int jump, int *val,
		      void (*func) (void *data), void *data)
{
   GadVSlider *g;
   XSetWindowAttributes attr;

   if (len < 9)
      len = 9;
   g = malloc(sizeof(GadVSlider));
   g->general.type = E_VSLIDER;
   g->x = x;
   g->y = y;
   g->w = 8;
   g->h = len;
   g->max = max;
   g->min = min;
   g->step = step;
   g->jump = jump;
   g->val = val;
   g->func = func;
   g->data = data;
   g->hilited = 0;
   g->clicked = 0;
   attr.backing_store     = NotUseful;
   attr.override_redirect = False;
   attr.colormap          = Imlib_get_colormap(id);
   attr.border_pixel      = 0;
   attr.background_pixel  = 0;
   attr.save_under        = False;
   attr.event_mask        = ButtonPressMask | ButtonReleaseMask;
   g->general.visible = 0;
   g->win = XCreateWindow(disp, win, x, y, 8, len, 0, 
			  id->x.depth, InputOutput, Imlib_get_visual(id),
			  CWOverrideRedirect | CWSaveUnder | CWBackingStore |
			  CWColormap | CWBackPixel | CWBorderPixel |
			  CWEventMask, &attr);
   attr.event_mask        = ButtonPressMask | ButtonReleaseMask | 
      PointerMotionMask | EnterWindowMask | LeaveWindowMask | ButtonMotionMask;
   g->win_knob = XCreateWindow(disp, win, x, y, 8, 8, 0, 
			       id->x.depth, InputOutput, Imlib_get_visual(id),
			       CWOverrideRedirect | CWSaveUnder | CWBackingStore |
			       CWColormap | CWBackPixel | CWBorderPixel |
			       CWEventMask, &attr);
   XSaveContext(disp, g->win, xid_context, (XPointer) g);
   XSaveContext(disp, g->win_knob, xid_context, (XPointer) g);
   Epplet_add_gad((Epplet_gadget)g);
   return (Epplet_gadget) g;
}

void
Epplet_draw_vslider(Epplet_gadget eg)
{
   GadVSlider *g;
   char *state;
      
   g = (GadVSlider *)eg;
   if (g->hilited)
     {
	if (g->clicked)
	   state = "clicked";
	else
	   state = "hilited";
     }
   else
     {
	if (g->clicked)
	   state = "clicked";
	else
	   state = "normal";
     }
   Epplet_imageclass_apply("EPPLET_VSLIDER_BASE", "normal", 
			   g->win);
   XMoveWindow(disp, g->win_knob, 
	       g->x, 
	       g->y + ((g->h - 8) * (*(g->val))) / (g->max - g->min + 1));
   Epplet_imageclass_apply("EPPLET_VSLIDER_KNOB", state, 
			   g->win_knob);
}

typedef struct
{
   GadGeneral general;
   int        x, y, w, h;
   int       *val;
   char       dir;
   Window     win;
   Window     win_in;
} GadHBar;

Epplet_gadget 
Epplet_create_hbar(int x, int y, int w, int h, char dir,
		   int *val)
{
   GadHBar *g;
   XSetWindowAttributes attr;
   
   g = malloc(sizeof(GadHBar));
   g->general.type = E_HBAR;
   g->x = x;
   g->y = y;
   g->w = w;
   g->h = h;
   g->dir = dir;
   g->val = val;
   attr.backing_store     = NotUseful;
   attr.override_redirect = False;
   attr.colormap          = Imlib_get_colormap(id);
   attr.border_pixel      = 0;
   attr.background_pixel  = 0;
   attr.save_under        = False;
   attr.event_mask        = 0;
   g->general.visible = 0;
   g->win = XCreateWindow(disp, win, x, y, w, h, 0, 
			  id->x.depth, InputOutput, Imlib_get_visual(id),
			  CWOverrideRedirect | CWSaveUnder | CWBackingStore |
			  CWColormap | CWBackPixel | CWBorderPixel |
			  CWEventMask, &attr);
   g->win_in = XCreateWindow(disp, g->win, 2, 2, w - 4, h - 4, 0, 
			     id->x.depth, InputOutput, Imlib_get_visual(id),
			     CWOverrideRedirect | CWSaveUnder | CWBackingStore |
			     CWColormap | CWBackPixel | CWBorderPixel |
			     CWEventMask, &attr);
   XMapWindow(disp, g->win_in);
   Epplet_add_gad((Epplet_gadget)g);
   return (Epplet_gadget) g;
}

void
Epplet_draw_hbar(Epplet_gadget eg)
{
   GadHBar *g;
   int l;
      
   g = (GadHBar *)eg;
   l = (((g->w - 4) * (*(g->val))) / 100);
   if (l < 1)
      l = 1;
   if (l > (g->w - 4))
      l = g->w - 4;
   if (g->dir)
      XMoveResizeWindow(disp, g->win_in, g->w - 2 - l, 2, l, g->h - 4);
   else
      XMoveResizeWindow(disp, g->win_in, 2, 2, l, g->h - 4);
   XSync(disp, False);
   Epplet_imageclass_apply("EPPLET_HBAR_BASE", "normal", g->win);
   Epplet_imageclass_apply("EPPLET_HBAR_BAR", "normal", g->win_in);
}

typedef struct
{
   GadGeneral general;
   int        x, y, w, h;
   int       *val;
   char       dir;
   Window     win;
   Window     win_in;
} GadVBar;

Epplet_gadget 
Epplet_create_vbar(int x, int y, int w, int h, char dir,
		   int *val)
{
   GadHBar *g;
   XSetWindowAttributes attr;
   
   g = malloc(sizeof(GadVBar));
   g->general.type = E_VBAR;
   g->x = x;
   g->y = y;
   g->w = w;
   g->h = h;
   g->dir = dir;
   g->val = val;
   attr.backing_store     = NotUseful;
   attr.override_redirect = False;
   attr.colormap          = Imlib_get_colormap(id);
   attr.border_pixel      = 0;
   attr.background_pixel  = 0;
   attr.save_under        = False;
   attr.event_mask        = 0;
   g->general.visible = 0;
   g->win = XCreateWindow(disp, win, x, y, w, h, 0, 
			  id->x.depth, InputOutput, Imlib_get_visual(id),
			  CWOverrideRedirect | CWSaveUnder | CWBackingStore |
			  CWColormap | CWBackPixel | CWBorderPixel |
			  CWEventMask, &attr);
   g->win_in = XCreateWindow(disp, g->win, 2, 2, w - 4, h - 4, 0, 
			     id->x.depth, InputOutput, Imlib_get_visual(id),
			     CWOverrideRedirect | CWSaveUnder | CWBackingStore |
			     CWColormap | CWBackPixel | CWBorderPixel |
			     CWEventMask, &attr);
   XMapWindow(disp, g->win_in);
   Epplet_add_gad((Epplet_gadget)g);
   return (Epplet_gadget) g;
}

void
Epplet_draw_vbar(Epplet_gadget eg)
{
   GadVBar *g;
   int l;
      
   g = (GadVBar *)eg;
   l = (((g->h - 4) * (*(g->val))) / 100);
   if (l < 1)
      l = 1;
   if (l > (g->h - 4))
      l = g->h - 4;
   if (g->dir)
      XMoveResizeWindow(disp, g->win_in, 2, g->h - 2 - l, g->w - 4, l);
   else
      XMoveResizeWindow(disp, g->win_in, 2, 2, g->w - 4, l);
   XSync(disp, False);
   Epplet_imageclass_apply("EPPLET_VBAR_BASE", "normal", g->win);
   Epplet_imageclass_apply("EPPLET_VBAR_BAR", "normal", g->win_in);
}

typedef struct
{
   GadGeneral general;
   int        x, y, w, h, pw, ph;
   char      *image;
} GadImage;

Epplet_gadget 
Epplet_create_image(int x, int y, int w, int h, char *image)
{
   GadImage *g;
   
   g = malloc(sizeof(GadImage));
   g->general.type = E_IMAGE;
   g->general.visible = 0;
   g->x = x;
   g->y = y;
   g->w = w;
   g->h = h;
   g->pw = 0;
   g->ph = 0;
   g->image = Estrdup(image);
   Epplet_add_gad((Epplet_gadget)g);
   return (Epplet_gadget) g;
}

void
Epplet_draw_image(Epplet_gadget eg, char un_only)
{
   GadImage *g;
   static GC gc = 0;
   XGCValues gcv;
   ImlibImage *im;
      
   g = (GadImage *)eg;
   if (!gc)
      gc = XCreateGC(disp, bg_pmap, 0, &gcv);
   if ((g->pw > 0) && (g->ph > 0))
      XCopyArea(disp, bg_bg, bg_pmap, gc, g->x, g->y, g->pw, g->ph, 
		g->x, g->y);
   if (!un_only)
     {
	im = Imlib_load_image(id, g->image);
	if (im)
	  {
	     if ((g->w > 0) && (g->h > 0))
	       {
		  Imlib_paste_image(id, im, bg_pmap, g->x, g->y, g->w, g->h);
		  g->pw = g->w;
		  g->ph = g->h;
	       }
	     else
	       {
		  Imlib_paste_image(id, im, bg_pmap, g->x, g->y, 
				    im->rgb_width, im->rgb_height);
		  g->pw = im->rgb_width;
		  g->ph = im->rgb_height;
	       }
	     Imlib_destroy_image(id, im);
	  }
     }
   XSetWindowBackgroundPixmap(disp, win, bg_pmap);
   XClearWindow(disp, win);
}

typedef struct
{
   GadGeneral general;
   int        x, y, w, h;
   char       size;
   char      *label;
} GadLabel;

Epplet_gadget 
Epplet_create_label(int x, int y, char *label, char size)
{
   GadLabel *g;
   
   g = malloc(sizeof(GadLabel));
   g->general.type = E_LABEL;
   g->general.visible = 0;
   g->x = x;
   g->y = y;
   g->size = size;
   g->label = Estrdup(label);
   if (g->size == 0)
      Epplet_textclass_get_size("EPPLET_LABEL", &(g->w), &(g->h), g->label);
   else if (g->size == 1)
      Epplet_textclass_get_size("EPPLET_TEXT_TINY", &(g->w), &(g->h), g->label);
   else if (g->size == 2)
      Epplet_textclass_get_size("EPPLET_TEXT_MEDIUM", &(g->w), &(g->h), g->label);
   else
      Epplet_textclass_get_size("EPPLET_TEXT_LARGE", &(g->w), &(g->h), g->label);
   Epplet_add_gad((Epplet_gadget)g);
   return (Epplet_gadget) g;
}

void
Epplet_draw_label(Epplet_gadget eg, char un_only)
{
   GadLabel *g;
   static GC gc = 0;
   XGCValues gcv;
   int x;

   g = (GadLabel *)eg;
   if (!gc)
     gc = XCreateGC(disp, bg_pmap, 0, &gcv);
   if (g->x < 0)
     {
       x = win_w + g->x - g->w;
       if (x < 0)
         x = 0;
     }
   else
     x = g->x;
   XCopyArea(disp, bg_bg, bg_pmap, gc, 
	     x - 1, g->y - 1, g->w + 2, g->h + 2,
	     x - 1, g->y - 1);
   if (!un_only)
     {
       XSync(disp, False);
       if (g->size == 0)
         {
           Epplet_textclass_get_size("EPPLET_LABEL", &(g->w), &(g->h), g->label);
           if (g->x < 0)
             {
               x = win_w + g->x - g->w;
               if (x < 0)
                 x = 0;
             }
           else
             x = g->x;
           Epplet_textclass_draw("EPPLET_LABEL", "normal", bg_pmap, x, g->y, 
                                 g->label);
         }
       else if (g->size == 1)
         {
           Epplet_textclass_get_size("EPPLET_TEXT_TINY", &(g->w), &(g->h), g->label);
           if (g->x < 0)
             {
               x = win_w + g->x - g->w;
               if (x < 0)
                 x = 0;
             }
           else
             x = g->x;
           Epplet_textclass_draw("EPPLET_TEXT_TINY", "normal", bg_pmap, x, g->y, 
                                 g->label);
         }
       else if (g->size == 2)
         {
           Epplet_textclass_get_size("EPPLET_TEXT_MEDIUM", &(g->w), &(g->h), g->label);
           if (g->x < 0)
             {
               x = win_w + g->x - g->w;
               if (x < 0)
                 x = 0;
             }
           else
             x = g->x;
           Epplet_textclass_draw("EPPLET_TEXT_MEDIUM", "normal", bg_pmap, x, g->y, 
                                 g->label);
         }
       else
         {
           Epplet_textclass_get_size("EPPLET_TEXT_LARGE", &(g->w), &(g->h), g->label);
           if (g->x < 0)
             {
               x = win_w + g->x - g->w;
               if (x < 0)
                 x = 0;
             }
           else
             x = g->x;
           Epplet_textclass_draw("EPPLET_TEXT_LARGE", "normal", bg_pmap, x, g->y, 
                                 g->label);
         }
       ESYNC;
     }
   XSetWindowBackgroundPixmap(disp, win, bg_pmap);
   XClearWindow(disp, win);
}

typedef struct
{
   char           *label;
   char           *image;
   int             w, h;
   void          (*func) (void *data);
   void           *data;
   Epplet_gadget   gadget;
} GadPopEntry;

typedef struct _gadpopupbutton GadPopupButton;
typedef struct
{
   GadGeneral     general;
   int            x, y, w, h;
   Epplet_gadget  popbutton;
   int            entry_num;
   GadPopEntry   *entry;   
   Window         win;
   char           changed; 
} GadPopup;

struct _gadpopupbutton
{
   GadGeneral general;
   int        x, y, w, h;
   char      *label;
   char      *image;
   char       hilited;
   char       clicked;
   Epplet_gadget popup;
   char       popped;
   char      *std;
   Window     win;
   Pixmap     pmap, mask;
};

Epplet_gadget 
Epplet_create_popup(void)
{
   GadPopup *g;
   XSetWindowAttributes attr;
   
   g = malloc(sizeof(GadPopup));
   g->general.type = E_POPUP;
   g->general.visible = 0;
   g->x = 0;
   g->y = 0;
   g->w = 5;
   g->h = 5;
   g->popbutton = NULL;
   g->entry_num = 0;
   g->entry = NULL;
   g->changed = 1;
   attr.backing_store     = NotUseful;
   attr.override_redirect = True;
   attr.colormap          = Imlib_get_colormap(id);
   attr.border_pixel      = 0;
   attr.background_pixel  = 0;
   attr.save_under        = False;
   attr.event_mask        = ButtonPressMask | ButtonReleaseMask |
      PointerMotionMask | EnterWindowMask | LeaveWindowMask | ButtonMotionMask;
   g->win = XCreateWindow(disp, root, 0, 0, 5, 5, 0, 
			  id->x.depth, InputOutput, Imlib_get_visual(id),
			  CWOverrideRedirect | CWSaveUnder | CWBackingStore |
			  CWColormap | CWBackPixel | CWBorderPixel |
			  CWEventMask, &attr);
   Epplet_add_gad((Epplet_gadget)g);
   return (Epplet_gadget) g;   
}

void
Epplet_add_popup_entry(Epplet_gadget gadget, char *label, char *pixmap,
		       void (*func) (void *data), void *data)
{
   GadPopup *g;
   
   g = (GadPopup *)gadget;
   g->entry_num++;
   if (g->entry)
      g->entry = realloc(g->entry, sizeof(GadPopup) * g->entry_num);
   else
      g->entry = malloc(sizeof(GadPopup));
   g->entry[g->entry_num - 1].label = Estrdup(label);
   g->entry[g->entry_num - 1].image = Estrdup(pixmap);
   g->entry[g->entry_num - 1].w = 0;
   g->entry[g->entry_num - 1].h = 0;
   g->entry[g->entry_num - 1].func = func;
   g->entry[g->entry_num - 1].data = data;
   g->entry[g->entry_num - 1].gadget = NULL;
   if (g->entry[g->entry_num - 1].image)
     {
	ImlibImage *im;
	
	im = Imlib_load_image(id, g->entry[g->entry_num - 1].image);
	g->w = im->rgb_width;
	g->h = im->rgb_height;
	Imlib_destroy_image(id, im);
     }
   else if (g->entry[g->entry_num - 1].label)
     {
	int w, h;
	
	Epplet_textclass_get_size("EPPLET_POPUP",
				  &w, &h, 
				  g->entry[g->entry_num - 1].label);
	g->entry[g->entry_num - 1].w = w;
	g->entry[g->entry_num - 1].h = h;
     }
   g->changed = 1;
}

void
Epplet_remove_popup_entry(Epplet_gadget gadget, int entry_num)
{
   GadPopup *g;
   int i;
   
   g = (GadPopup *)gadget;
   if (!g->entry)
     return;

   if(entry_num<0)
     entry_num=g->entry_num+entry_num;
   if(g->entry_num<entry_num)
     return;

   if(g->entry[entry_num].label)
   {
     free(g->entry[entry_num].label);
     g->entry[entry_num].label=NULL;
   }
   if(g->entry[entry_num].image)
   {
     free(g->entry[entry_num].image);
     g->entry[entry_num].image=NULL;
   }

  g->entry_num--;
  for(i=entry_num;i<g->entry_num;i++){
     *((g->entry)+i)=*((g->entry)+i+1);
  }

  if(g->entry_num)
    g->entry = realloc(g->entry, sizeof(GadPopup) * g->entry_num);
  else
  {
    free(g->entry);
    g->entry=NULL;
  }
  g->changed = 1;
}

void *
Epplet_popup_entry_get_data(Epplet_gadget gadget, int entry_num)
{
   GadPopup *g;
   /* int i; */
   
   g = (GadPopup *)gadget;
   if (!g->entry)
     return NULL;

   if(entry_num<0)
     entry_num=g->entry_num+entry_num;
   if(g->entry_num<entry_num)
     return NULL;

   return ((g->entry)[entry_num]).data;
}

int
Epplet_popup_entry_num( Epplet_gadget gadget )
{
   if ( ((GadGeneral*)gadget)->type == E_POPUP )
     return ((GadPopup*)gadget)->entry_num;
   else
     return 0;
}

void
Epplet_popup_arrange_contents(Epplet_gadget gadget)
{
   GadPopup *g;
   int i, mw, mh, x, y;
   
   g = (GadPopup *)gadget;
   mw = 0;
   mh = 0;
   for (i = 0; i < g->entry_num; i++)
     {
	if (g->entry[i].w > mw)
	   mw = g->entry[i].w;
	if (g->entry[i].h > mh)
	   mh = g->entry[i].h;
     }
   x = 0;
   y = 0;
   XResizeWindow(disp, g->win, mw + 8, ((mh + 4) * g->entry_num) + 4);
   for (i = 0; i < g->entry_num; i++)
     {
	g->entry[i].gadget = Epplet_create_button(g->entry[i].label,
						  g->entry[i].image,
						  x + 2, y + 2, 
						  mw + 4, mh + 4, NULL,
						  g->win, gadget,
						  g->entry[i].func,
						  g->entry[i].data);
	Epplet_gadget_show(g->entry[i].gadget);
	y += mh + 4;
     }
   g->x = 0;
   g->y = 0;
   g->w = mw + 8;
   g->h = ((mh + 4) * g->entry_num) + 4;
   XSync(disp, False);
}

void
Epplet_draw_popup(Epplet_gadget gadget)
{
   GadPopup *g;
   
   g = (GadPopup *)gadget;
   if (g->changed)
     {
	g->changed = 0;
	Epplet_imageclass_apply("EPPLET_POPUP_BASE", "normal", g->win);
     }
}

void
Epplet_pop_popup(Epplet_gadget gadget, Window ww)
{
   GadPopup *g;
   
   g = (GadPopup *)gadget;
   if (g->changed)
      Epplet_popup_arrange_contents(gadget);
   if (ww)
     {
	int px, py, rw, rh, x, y;
	Window dw;
	unsigned int w, h, b, d;
	
	XGetGeometry(disp, root, &dw, &x, &y, 
		     (unsigned int *)&rw, 
		     (unsigned int *)&rh, &b, &d);
	XGetGeometry(disp, ww, &dw, &x, &y, &w, &h, &b, &d);
	XTranslateCoordinates(disp, ww, root, 0, 0, &px, &py, &dw);
	if ((py + ((int)h / 2)) > (rh / 2))
	  {
	     g->x = px + ((w - g->w) / 2);
	     g->y = py - g->h;
	  }
	else
	  {
	     g->x = px + ((w - g->w) / 2);
	     g->y = py + h;
	  }
     }
   else
     {
	int rw, rh, dd, x, y;
	Window dw;
	unsigned int b, d;
	
	XGetGeometry(disp, root, &dw, &x, &y, 
		     (unsigned int *)&rw, 
		     (unsigned int *)&rh, &b, &d);
	XQueryPointer(disp, root, &dw, &dw, &dd, &dd, &x, &y, &b);	
	g->x = x - (g->w / 2);
	g->y = y - 8;
	if (g->x < 0)
	   g->x = 0;
	if (g->y < 0)
	   g->y = 0;
	if ((g->x + g->w) > rw)
	   g->x = rw - g->w;
	if ((g->y + g->h) > rh)
	   g->y = rh - g->h;
     }
   XMoveWindow(disp, g->win, g->x, g->y);
   Epplet_gadget_show(gadget);
}

Epplet_gadget 
Epplet_create_popupbutton(char *label, char *image, int x,
			  int y, int w, int h, char *std,
			  Epplet_gadget popup)
{
   GadPopupButton *g;
   XSetWindowAttributes attr;
   
   g = malloc(sizeof(GadPopupButton));
   g->general.type = E_POPUPBUTTON;
   g->x = x;
   g->y = y;
   g->std = Estrdup(std);
   if (g->std)
     {
        g->w = 12;
        g->h = 12;
     }
   else
     {
        g->w = w;
        g->h = h;
     }
   g->pmap = 0;
   g->mask = 0;
   g->label = Estrdup(label);
   g->image = Estrdup(image);
   g->hilited = 0;
   g->clicked = 0;
   g->popped = 0;
   g->popup = popup;
   attr.backing_store     = NotUseful;
   attr.override_redirect = False;
   attr.colormap          = Imlib_get_colormap(id);
   attr.border_pixel      = 0;
   attr.background_pixel  = 0;
   attr.save_under        = False;
   attr.event_mask        = ButtonPressMask | ButtonReleaseMask | 
      EnterWindowMask | LeaveWindowMask;
   g->general.visible = 0;
   g->win = XCreateWindow(disp, win, x, y, g->w, g->h, 0, 
			  id->x.depth, InputOutput, Imlib_get_visual(id),
			  CWOverrideRedirect | CWSaveUnder | CWBackingStore |
			  CWColormap | CWBackPixel | CWBorderPixel |
			  CWEventMask, &attr);
   XSaveContext(disp, g->win, xid_context, (XPointer) g);
   Epplet_add_gad((Epplet_gadget)g);
   return (Epplet_gadget) g;
}

void
Epplet_draw_popupbutton(Epplet_gadget eg)
{
   GadPopupButton *g;
   char *state;
   
   g = (GadPopupButton *)eg;
   if (g->hilited)
     {
	if (g->clicked)
	   state = "clicked";
	else
	   state = "hilited";
     }
   else
     {
	if (g->clicked)
	   state = "clicked";
	else
	   state = "normal";
     }
   if (g->popped)
      state = "clicked";
   if (g->pmap)
      XFreePixmap(disp, g->pmap);
   if (g->mask)
      XFreePixmap(disp, g->mask);
   g->pmap = 0;
   g->mask = 0;
   if (g->std)
     {
	char s[1024];

	Esnprintf(s, sizeof(s), "EPPLET_%s", g->std);
	Epplet_imageclass_get_pixmaps(s, state,
                                      &(g->pmap), &(g->mask), g->w, g->h);
     }
   else
     {
	Epplet_imageclass_get_pixmaps("EPPLET_BUTTON", state, 
				 &(g->pmap), &(g->mask), g->w, g->h);
	if (g->image)
	  {
	     ImlibImage *im;

	     ESYNC;
	     im = Imlib_load_image(id, g->image);
	     if (im)
	       {
	     int x, y;
	     
	     x = (g->w - im->rgb_width) / 2;
	     y = (g->h - im->rgb_height) / 2;
	     Imlib_paste_image(id, im, g->pmap, x, y, 
			       im->rgb_width, im->rgb_height);
	     Imlib_destroy_image(id, im);
	       }
	  }
	if (g->label)
          {
	     int x, y, w, h;
	
	     Epplet_textclass_get_size("EPPLET_BUTTON", &w, &h, g->label);
	     x = (g->w - w) / 2;
	     y = (g->h - h) / 2;
	     Epplet_textclass_draw("EPPLET_BUTTON", state, g->pmap, x, y, g->label);
	  }
     }
   ESYNC;
   XSetWindowBackgroundPixmap(disp, g->win, g->pmap);
   XShapeCombineMask(disp, g->win, ShapeBounding, 0, 0, g->mask, ShapeSet);   
   XClearWindow(disp, g->win);
}

void
Epplet_change_popbutton_popup(Epplet_gadget gadget, Epplet_gadget popup)
{
   GadPopupButton *g;
   
   g = (GadPopupButton *)gadget;
   Epplet_gadget_destroy(g->popup);   
   g->popped = 0;
   g->popup = popup;
   Epplet_draw_popupbutton(gadget);
}

void
Epplet_change_image(Epplet_gadget gadget, int w, int h, char *image)
{
   GadImage *g;
   
   g = (GadImage *)gadget;
   if (g->image)
      free(g->image);
   g->image = Estrdup(image);
   g->w = w;
   g->h = h;
   Epplet_draw_image(gadget, 0);
}

void
Epplet_change_label(Epplet_gadget gadget, char *label)
{
   GadLabel *g;
   
   g = (GadLabel *)gadget;
   if (g->label)
     {
       if (label && !strcmp(g->label, label))
         return;  /* The labels are identical, so no sense in redrawing */
       else
         free(g->label);  /* The labels are different.  Proceed. */
     }
   g->label = Estrdup(label);
   Epplet_draw_label(gadget, 0);
}

Window 
Epplet_get_drawingarea_window(Epplet_gadget gadget)
{
   GadDrawingArea *g;
   
   g = (GadDrawingArea *)gadget;
   return g->win_in;
}

void 
Epplet_event(Epplet_gadget gadget, XEvent *ev)
{
   static int rx = 0, ry = 0, dx, dy;
   GadGeneral *gg;

   gg = (GadGeneral *)gadget;
   switch (ev->type)
     {
    case KeyPress:
      rx = ev->xbutton.x_root;
      ry = ev->xbutton.y_root;
      switch (gg->type)
	{
	case E_BUTTON:
	  break;
	case E_TEXTBOX:
	  {
	    GadTextBox         *g;

	    g = (GadTextBox *) gadget;

	    Epplet_textbox_handle_keyevent(ev, gadget);

	    Epplet_draw_textbox(g);
	  }
	  break;
	case E_VSLIDER:
	  break;
	case E_HSLIDER:
	  break;
	case E_POPUPBUTTON:
	  break;
	case E_DRAWINGAREA:
	  break;
	case E_IMAGE:
	  break;
	case E_LABEL:
	  break;
	case E_VBAR:
	  break;
	case E_HBAR:
	  break;
	case E_TOGGLEBUTTON:
	  break;
	case E_POPUP:
	  break;
	}
      break;
     case ButtonPress:
	rx = ev->xbutton.x_root;
	ry = ev->xbutton.y_root;
	switch (gg->type)
	  {
	  case E_BUTTON:
	       {
		  GadButton *g;
		  
		  g = (GadButton *)gadget;
		  g->clicked = 1;
		  Epplet_draw_button(gadget);
	       }
	     break;
	  case E_HSLIDER:
	       {
		  GadHSlider *g;
		  
		  g = (GadHSlider *)gadget;
		  g->clicked = 1;
		  if (ev->xbutton.window == g->win)
		    {
		       if (ev->xbutton.x > (((*(g->val)) * g->w) / 
					    (g->max - g->min + 1)))
			  (*(g->val)) += g->jump;
		       else
			  (*(g->val)) -= g->jump;
		       if ((*(g->val)) < g->min)
			  (*(g->val)) = g->min;
		       if ((*(g->val)) > g->max)
			  (*(g->val)) = g->max;
		    }
		  if (g->func)
		     (*(g->func)) (g->data);
		  Epplet_draw_hslider(gadget);
	       }
	     break;
	  case E_VSLIDER:
	       {
		  GadVSlider *g;
		  
		  g = (GadVSlider *)gadget;
		  g->clicked = 1;
		  if (ev->xbutton.window == g->win)
		    {
		       if (ev->xbutton.y > (((*(g->val)) * g->h) / 
					    (g->max - g->min + 1)))
			  (*(g->val)) += g->jump;
		       else
			  (*(g->val)) -= g->jump;
		       if ((*(g->val)) < g->min)
			  (*(g->val)) = g->min;
		       if ((*(g->val)) > g->max)
			  (*(g->val)) = g->max;
		    }
		  if (g->func)
		     (*(g->func)) (g->data);
		  Epplet_draw_vslider(gadget);
	       }
	     break;
	  case E_TOGGLEBUTTON:
	       {
		  GadToggleButton *g;
		  
		  g = (GadToggleButton *)gadget;
		  g->clicked = 1;
		  Epplet_draw_togglebutton(gadget);
	       }
	     break;
	  case E_POPUPBUTTON:
	       {
		  GadPopupButton *g;
		  
		  g = (GadPopupButton *)gadget;
		  g->clicked = 1;
		  Epplet_draw_popupbutton(gadget);
	       }
	     break;
	  case E_POPUP:
	     break;
	  default:
	     break;
	  }
	break;
     case ButtonRelease:
	rx = ev->xbutton.x_root;
	ry = ev->xbutton.y_root;
	switch (gg->type)
	  {
	  case E_BUTTON:
	       {
		  GadButton *g;
		  
		  g = (GadButton *)gadget;
		  g->clicked = 0;
		  Epplet_draw_button(gadget);
		  if (g->pop_parent)
		     Epplet_gadget_hide(g->pop_parent);
		  if ((g->hilited) && (g->func))
		     (*(g->func)) (g->data);
	       }
	     break;
	  case E_HSLIDER:
	       {
		  GadHSlider *g;
		  
		  g = (GadHSlider *)gadget;
		  g->clicked = 0;
		  Epplet_draw_hslider(gadget);
	       }
	     break;
	  case E_VSLIDER:
	       {
		  GadVSlider *g;
		  
		  g = (GadVSlider *)gadget;
		  g->clicked = 0;
		  Epplet_draw_vslider(gadget);
	       }
	     break;
	  case E_TOGGLEBUTTON:
	       {
		  GadToggleButton *g;
		  
		  g = (GadToggleButton *)gadget;
		  g->clicked = 0;
		  if (g->hilited)
		    {
		       if (*(g->val))
			  *(g->val) = 0;
		       else
			  *(g->val) = 1;
		    }
		  Epplet_draw_togglebutton(gadget);
		  if ((g->hilited) && (g->func))
		     (*(g->func)) (g->data);
	       }
	     break;
	  case E_POPUPBUTTON:
	       {
		  GadPopupButton *g;
		  
		  g = (GadPopupButton *)gadget;
		  g->clicked = 0;
		  if (g->popped)
		    {
		       if (g->popup)
			  Epplet_gadget_hide(g->popup);
		    }
		  else
		    {
		       if (g->popup)
			 {
			    Epplet_pop_popup(g->popup, g->win);
			    ((GadPopup *)g->popup)->popbutton = gadget;
			    g->popped = 1;
			 }
		       Epplet_draw_popupbutton(gadget);
		    }
	       }
	     break;
	  case E_POPUP:
	     break;
	  default:
	     break;
	  }
	break;
     case MotionNotify:
	dx = ev->xbutton.x_root - rx;
	dy = ev->xbutton.y_root - ry;
	rx = ev->xbutton.x_root;
	ry = ev->xbutton.y_root;
	switch (gg->type)
	  {
	  case E_HSLIDER:
	       {
		  GadHSlider *g;
		  int v, x, xx;
		  
		  g = (GadHSlider *)gadget;
		  if (g->clicked)
		    {
		       x = ((g->w - 8) * (*(g->val))) / (g->max - g->min + 1);
		       xx = x + dx;
		       (*(g->val)) = g->min +  ((xx * (g->max - g->min)) / (g->w - 8));
		       v = (*(g->val)) / g->step;
		       if ((*(g->val)) - (v * g->step) >= (g->step / 2))
			  v++;
		       (*(g->val)) = v * g->step;		       
		       if ((*(g->val)) < g->min)
			  (*(g->val)) = g->min;
		       if ((*(g->val)) > g->max)
			  (*(g->val)) = g->max;
		       xx = ((g->w - 8) * (*(g->val))) / (g->max - g->min + 1);
		       rx = rx - dx + (xx - x); 
		       if (g->func)
			  (*(g->func)) (g->data);
		       Epplet_draw_hslider(gadget);
		    }
	       }
	     break;
	  case E_VSLIDER:
	       {
		  GadVSlider *g;
		  int v, y, yy;
		  
		  g = (GadVSlider *)gadget;
		  if (g->clicked)
		    {
		       y = ((g->h - 8) * (*(g->val))) / (g->max - g->min + 1);
		       yy = y + dy;
		       (*(g->val)) = g->min +  ((yy * (g->max - g->min)) / (g->h - 8));
		       v = (*(g->val)) / g->step;
		       if ((*(g->val)) - (v * g->step) >= (g->step / 2))
			  v++;
		       (*(g->val)) = v * g->step;		       
		       if ((*(g->val)) < g->min)
			  (*(g->val)) = g->min;
		       if ((*(g->val)) > g->max)
			  (*(g->val)) = g->max;
		       yy = ((g->h - 8) * (*(g->val))) / (g->max - g->min + 1);
		       ry = ry - dy + (yy - y); 
		       if (g->func)
			  (*(g->func)) (g->data);
		       Epplet_draw_vslider(gadget);
		    }
	       }
	     break;
	  default:
	     break;
	  }
	break;
     case EnterNotify:
	switch (gg->type)
	  {
	  case E_BUTTON:
	       {
		  GadButton *g;
		  
		  g = (GadButton *)gadget;
		  g->hilited = 1;
		  Epplet_draw_button(gadget);
	       }
	     break;
	case E_TEXTBOX:
	  {
	    GadTextBox         *g;

	    g = (GadTextBox *) gadget;
	    g->hilited = 1;
	    Epplet_draw_textbox(gadget);
	  }
	  break;
	  case E_HSLIDER:
	       {
		  GadHSlider *g;
		  
		  g = (GadHSlider *)gadget;
		  g->hilited = 1;
		  Epplet_draw_hslider(gadget);
	       }
	     break;
	  case E_VSLIDER:
	       {
		  GadVSlider *g;
		  
		  g = (GadVSlider *)gadget;
		  g->hilited = 1;
		  Epplet_draw_vslider(gadget);
	       }
	     break;
	  case E_TOGGLEBUTTON:
	       {
		  GadToggleButton *g;
		  
		  g = (GadToggleButton *)gadget;
		  g->hilited = 1;
		  Epplet_draw_togglebutton(gadget);
	       }
	     break;
	  case E_POPUPBUTTON:
	       {
		  GadPopupButton *g;
		  
		  g = (GadPopupButton *)gadget;
		  g->hilited = 1;
		  Epplet_draw_popupbutton(gadget);
	       }
	     break;
	  case E_POPUP:
	     break;
	  default:
	     break;
	  }
	break;
     case LeaveNotify:
	switch (gg->type)
	  {
	  case E_BUTTON:
	       {
		  GadButton *g;
		  
		  g = (GadButton *)gadget;
		  g->hilited = 0;
		  Epplet_draw_button(gadget);
	       }
	     break;
	case E_TEXTBOX:
	  {
	    GadTextBox         *g;

	    g = (GadTextBox *) gadget;
	    g->hilited = 0;
	    Epplet_draw_textbox(gadget);
	  }
	  break;
	  case E_HSLIDER:
	       {
		  GadHSlider *g;
		  
		  g = (GadHSlider *)gadget;
		  g->hilited = 0;
		  Epplet_draw_hslider(gadget);
	       }
	     break;
	  case E_VSLIDER:
	       {
		  GadVSlider *g;
		  
		  g = (GadVSlider *)gadget;
		  g->hilited = 0;
		  Epplet_draw_vslider(gadget);
	       }
	     break;
	  case E_TOGGLEBUTTON:
	       {
		  GadToggleButton *g;
		  
		  g = (GadToggleButton *)gadget;
		  g->hilited = 0;
		  Epplet_draw_togglebutton(gadget);
	       }
	     break;
	  case E_POPUPBUTTON:
	       {
		  GadPopupButton *g;
		  
		  g = (GadPopupButton *)gadget;
		  g->hilited = 0;
		  Epplet_draw_popupbutton(gadget);
	       }
	     break;
	  case E_POPUP:
	     break;
	  default:
	     break;
	  }
	break;
     default:
	break;
     }
}

void 
Epplet_background_properties(char vertical)
{
   static GC gc = 0;
   XGCValues gcv;
   
   if (bg_pmap)
      XFreePixmap(disp, bg_pmap);
   if (bg_bg)
      XFreePixmap(disp, bg_bg);
   if (bg_mask)
      XFreePixmap(disp, bg_mask);
   bg_pmap = 0;
   bg_mask = 0;
   bg_bg = 0;
   if (vertical)
      Epplet_imageclass_get_pixmaps("EPPLET_BACKGROUND_VERTICAL", "normal", 
				    &bg_bg, &bg_mask, win_w, win_h);
   else
      Epplet_imageclass_get_pixmaps("EPPLET_BACKGROUND_HORIZONTAL", "normal", 
				    &bg_bg, &bg_mask, win_w, win_h);
   bg_pmap = XCreatePixmap(disp, win, win_w, win_h, id->x.depth);
   if (!gc)
      gc = XCreateGC(disp, bg_pmap, 0, &gcv);
   XCopyArea(disp, bg_bg, bg_pmap, gc, 0, 0, win_w, win_h, 0, 0);   
   XSetWindowBackgroundPixmap(disp, win, bg_pmap);
   XShapeCombineMask(disp, win, ShapeBounding, 0, 0, bg_mask, ShapeSet);   
   XClearWindow(disp, win);
   win_vert = vertical;
}

void Epplet_gadget_destroy(Epplet_gadget gadget)
{
   GadGeneral *gg;
   
   Epplet_gadget_hide(gadget);
   gg = (GadGeneral *)gadget;
   Epplet_del_gad(gadget);
   switch(gg->type)
     {
     case E_BUTTON:
	  {
	     GadButton *g;
	     
	     g = (GadButton *)gadget;
	     XDestroyWindow(disp, g->win);
	     XDeleteContext(disp, g->win, xid_context);
	     if (g->label)
		free(g->label);
	     if (g->image)
		free(g->image);
	     if (g->std)
		free(g->std);
	     free(g);
	  }
	break;
     case E_DRAWINGAREA:
	  {
	     GadDrawingArea *g;
	     
	     g = (GadDrawingArea *)gadget;
	     XDestroyWindow(disp, g->win);
	     free(g);
	  }
	break;
     case E_HSLIDER:
	  {
	     GadHSlider *g;
	     
	     g = (GadHSlider *)gadget;
	     XDestroyWindow(disp, g->win);
	     XDestroyWindow(disp, g->win_knob);
	     XDeleteContext(disp, g->win, xid_context);
	     XDeleteContext(disp, g->win_knob, xid_context);
	     free(g);
	  }
	break;
     case E_VSLIDER:
	  {
	     GadVSlider *g;
	     
	     g = (GadVSlider *)gadget;
	     XDestroyWindow(disp, g->win);
	     XDestroyWindow(disp, g->win_knob);
	     XDeleteContext(disp, g->win, xid_context);
	     XDeleteContext(disp, g->win_knob, xid_context);
	     free(g);
	  }
	break;
     case E_TOGGLEBUTTON:
	  {
	     GadToggleButton *g;
	     
	     g = (GadToggleButton *)gadget;
	     XDestroyWindow(disp, g->win);
	     XDeleteContext(disp, g->win, xid_context);
	     if (g->label)
		free(g->label);
	     if (g->image)
		free(g->image);
	     free(g);
	  }
	break;
     case E_POPUPBUTTON:
	  {
	     GadPopupButton *g;
	     
	     g = (GadPopupButton *)gadget;
	     XDestroyWindow(disp, g->win);
	     XDeleteContext(disp, g->win, xid_context);
	     if (g->label)
		free(g->label);
	     if (g->image)
		free(g->image);
	     free(g);
	  }
	break;
     case E_POPUP:
	  {
	     GadPopup *g;
	     int i;
	     
	     g = (GadPopup *)gadget;
	     for (i = 0; i < g->entry_num; i++)
	       {
		  if (g->entry[i].label)
		     free(g->entry[i].label);
		  if (g->entry[i].image)
		     free(g->entry[i].image);
		  Epplet_gadget_destroy(g->entry[i].gadget);
	       }
	     XDestroyWindow(disp, g->win);
	     XDeleteContext(disp, g->win, xid_context);
	     free(g);
	  }
	break;
     case E_IMAGE:
	  {
	     GadImage *g;
	     
	     g = (GadImage *)gadget;
	     if (g->image)
		free(g->image);
	     free(g);
	  }
	break;
     case E_LABEL:
	  {
	     GadLabel *g;
	     
	     g = (GadLabel *)gadget;
	     if (g->label)
		free(g->label);
	     free(g);
	  }
	break;
     case E_HBAR:
	  {
	     GadHBar *g;
	     
	     g = (GadHBar *)gadget;
	     XDestroyWindow(disp, g->win);
	     free(g);
	  }
	break;
     case E_VBAR:
	  {
	     GadVBar *g;
	     
	     g = (GadVBar *)gadget;
	     XDestroyWindow(disp, g->win);
	     free(g);
	  }
	break;
     default:
	break;	
     }   
}

void
Epplet_gadget_hide(Epplet_gadget gadget)
{
   GadGeneral *gg;
   
   gg = (GadGeneral *)gadget;
   if (!(gg->visible))
      return;
   gg->visible = 0;
   switch(gg->type)
     {
     case E_BUTTON:
	  {
	     GadButton *g;
	     
	     g = (GadButton *)gadget;
	     XUnmapWindow(disp, g->win);
	  }
	break;
     case E_DRAWINGAREA:
	  {
	     GadDrawingArea *g;
	     
	     g = (GadDrawingArea *)gadget;
	     XUnmapWindow(disp, g->win);
	  }
	break;
     case E_HSLIDER:
	  {
	     GadHSlider *g;
	     
	     g = (GadHSlider *)gadget;
	     XUnmapWindow(disp, g->win);
	     XUnmapWindow(disp, g->win_knob);
	  }
	break;
     case E_VSLIDER:
	  {
	     GadVSlider *g;
	     
	     g = (GadVSlider *)gadget;
	     XUnmapWindow(disp, g->win);
	     XUnmapWindow(disp, g->win_knob);
	  }
	break;
     case E_TOGGLEBUTTON:
	  {
	     GadToggleButton *g;
	     
	     g = (GadToggleButton *)gadget;
	     XUnmapWindow(disp, g->win);
	  }
	break;
     case E_POPUPBUTTON:
	  {
	     GadPopupButton *g;
	     
	     g = (GadPopupButton *)gadget;
	     XUnmapWindow(disp, g->win);
	  }
	break;
     case E_POPUP:
	  {
	     GadPopup *g;
	     
	     g = (GadPopup *)gadget;
	     XUnmapWindow(disp, g->win);
	     if (g->popbutton)
	       {
		  ((GadPopupButton *)g->popbutton)->popped = 0;
		  Epplet_draw_popupbutton(g->popbutton);
		  g->popbutton = NULL;
	       }
	  }
	break;
     case E_IMAGE:
	  {
	     GadImage *g;
	     
	     g = (GadImage *)gadget;
	     Epplet_draw_image(gadget, 1);
	  }
	break;
     case E_LABEL:
	  {
	     GadLabel *g;
	     
	     g = (GadLabel *)gadget;
	     Epplet_draw_label(gadget, 1);
	  }
	break;
     case E_HBAR:
	  {
	     GadHBar *g;
	     
	     g = (GadHBar *)gadget;
	     XUnmapWindow(disp, g->win);
	  }
	break;
     case E_VBAR:
	  {
	     GadVBar *g;
	     
	     g = (GadVBar *)gadget;
	     XUnmapWindow(disp, g->win);
	  }
	break;
     default:
	break;	
     }
}

void 
Epplet_gadget_show(Epplet_gadget gadget)
{
   GadGeneral *gg;
   
   gg = (GadGeneral *)gadget;
   if (gg->visible)
      return;
   gg->visible = 1;
   switch(gg->type)
     {
     case E_BUTTON:
	  {
	     GadButton *g;
	     
	     g = (GadButton *)gadget;
	     Epplet_draw_button(gadget);
	     XMapWindow(disp, g->win);
	  }
	break;
    case E_TEXTBOX:
      {
	GadTextBox         *g;

	g = (GadTextBox *) gadget;
	Epplet_draw_textbox(gadget);
	XMapWindow(disp, g->win);
      }
      break;
     case E_DRAWINGAREA:
	  {
	     GadDrawingArea *g;
	     
	     g = (GadDrawingArea *)gadget;
	     Epplet_draw_drawingarea(gadget);
	     XMapWindow(disp, g->win);
	  }
	break;
     case E_HSLIDER:
	  {
	     GadHSlider *g;
	     
	     g = (GadHSlider *)gadget;
	     Epplet_draw_hslider(gadget);
	     XMapWindow(disp, g->win);
	     XMapRaised(disp, g->win_knob);
	  }
	break;
     case E_VSLIDER:
	  {
	     GadVSlider *g;
	     
	     g = (GadVSlider *)gadget;
	     Epplet_draw_vslider(gadget);
	     XMapWindow(disp, g->win);
	     XMapRaised(disp, g->win_knob);
	  }
	break;
     case E_TOGGLEBUTTON:
	  {
	     GadToggleButton *g;
	     
	     g = (GadToggleButton *)gadget;
	     Epplet_draw_togglebutton(gadget);
	     XMapWindow(disp, g->win);
	  }
	break;
     case E_POPUPBUTTON:
	  {
	     GadPopupButton *g;
	     
	     g = (GadPopupButton *)gadget;
	     Epplet_draw_popupbutton(gadget);
	     XMapWindow(disp, g->win);
	  }
	break;
     case E_POPUP:
	  {
	     GadPopup *g;
	     
	     g = (GadPopup *)gadget;
	     Epplet_popup_arrange_contents(gadget);
	     Epplet_draw_popup(gadget);
	     XMapRaised(disp, g->win);
	  }
	break;
     case E_IMAGE:
	  {
	     GadImage *g;
	     
	     g = (GadImage *)gadget;
	     Epplet_draw_image(gadget, 0);
	  }
	break;
     case E_LABEL:
	  {
	     GadLabel *g;
	     
	     g = (GadLabel *)gadget;
	     Epplet_draw_label(gadget, 0);
	  }
	break;
     case E_HBAR:
	  {
	     GadHBar *g;
	     
	     g = (GadHBar *)gadget;
	     Epplet_draw_hbar(gadget);
	     XMapWindow(disp, g->win);
	  }
	break;
     case E_VBAR:
	  {
	     GadVBar *g;
	     
	     g = (GadVBar *)gadget;
	     Epplet_draw_vbar(gadget);
	     XMapWindow(disp, g->win);
	  }
	break;
     default:
	break;	
     }
}

void *
Epplet_gadget_get_data( Epplet_gadget gadget )
{
   if (! gadget) return NULL;

   switch ( ((GadGeneral*)gadget)->type )
     {
     case E_BUTTON:
       return ((GadButton*)gadget)->data;
     case E_TOGGLEBUTTON:
       return ((GadToggleButton*)gadget)->data;
     case E_HSLIDER:
       return ((GadHSlider*)gadget)->data;
     case E_VSLIDER:
       return ((GadVSlider*)gadget)->data;
     default:
       return NULL;
     }
}

void 
Epplet_gadget_data_changed(Epplet_gadget gadget)
{
   GadGeneral *gg;
   
   gg = (GadGeneral *)gadget;
   if (!gg->visible)
      return;
   switch(gg->type)
     {
     case E_HSLIDER:
	  {
	     GadHSlider *g;
	     
	     g = (GadHSlider *)gadget;
	     Epplet_draw_hslider(gadget);
	  }
	break;
     case E_VSLIDER:
	  {
	     GadVSlider *g;
	     
	     g = (GadVSlider *)gadget;
	     Epplet_draw_vslider(gadget);
	  }
	break;
     case E_TOGGLEBUTTON:
	  {
	     GadToggleButton *g;
	     
	     g = (GadToggleButton *)gadget;
	     Epplet_draw_togglebutton(gadget);
	  }
	break;
     case E_IMAGE:
	  {
	     GadImage *g;
	     
	     g = (GadImage *)gadget;
	     Epplet_draw_image(gadget, 0);
	  }
	break;
     case E_LABEL:
	  {
	     GadLabel *g;
	     
	     g = (GadLabel *)gadget;
	     Epplet_draw_label(gadget, 0);
	  }
	break;
     case E_HBAR:
	  {
	     GadHBar *g;
	     
	     g = (GadHBar *)gadget;
	     Epplet_draw_hbar(gadget);
	  }
	break;
     case E_VBAR:
	  {
	     GadVBar *g;
	     
	     g = (GadVBar *)gadget;
	     Epplet_draw_vbar(gadget);
	  }
	break;
     default:
	break;	
     }
}

void 
Epplet_redraw(void)
{
   int i;
   GadGeneral *gg;

   Epplet_background_properties(win_vert);
   for (i = 0; i < gad_num; i++)
     {
	gg = (GadGeneral *)gads[i];
	if (gg->visible)
	  {
	     switch(gg->type)
	       {
	       case E_BUTTON:
		  Epplet_draw_button(gads[i]);
		  break;
	    case E_TEXTBOX:
	      Epplet_draw_textbox(gads[i]);
	      break;
	       case E_DRAWINGAREA:
		  Epplet_draw_drawingarea(gads[i]);
		  break;
	       case E_HSLIDER:
		  Epplet_draw_hslider(gads[i]);
		  break;
	       case E_VSLIDER:
		  Epplet_draw_vslider(gads[i]);
		  break;
	       case E_TOGGLEBUTTON:
		  Epplet_draw_togglebutton(gads[i]);
		  break;
	       case E_POPUPBUTTON:
		  Epplet_draw_popupbutton(gads[i]);
		  break;
	       case E_POPUP:
		  Epplet_draw_popup(gads[i]);
		  break;
	       case E_IMAGE:
		  Epplet_draw_image(gads[i], 0);
		  break;
	       case E_LABEL:
		  Epplet_draw_label(gads[i], 0);
		  break;
	       case E_HBAR:
		  Epplet_draw_hbar(gads[i]);
		  break;
	       case E_VBAR:
		  Epplet_draw_vbar(gads[i]);
		  break;
	       default:
		  break;	
	       }
	  }
     }
}

void Esync(void)
{
   XSync(disp, False);
}

void Epplet_draw_line(Window win, int x1, int y1, int x2, int y2,
		      int r, int g, int b)
{
   static int cr = -1, cg = -1, cb = -1;
   static GC gc = 0;
   XGCValues gcv;
   
   if (!gc)
      gc = XCreateGC(disp, win, 0, &gcv);   
   if ((cr != r) || (cg != g) || (cb != b))
      XSetForeground(disp, gc, 
		     Epplet_get_color(r, g, b));
   XDrawLine(disp, win, gc, x1, y1, x2, y2);		     
}

void Epplet_draw_box(Window win, int x, int y, int w, int h,
		     int r, int g, int b)
{
   static int cr = -1, cg = -1, cb = -1;
   static GC gc = 0;
   XGCValues gcv;

   if ((w <= 0) || (h <= 0))
      return;
   if (!gc)
      gc = XCreateGC(disp, win, 0, &gcv);   
   if ((cr != r) || (cg != g) || (cb != b))
      XSetForeground(disp, gc, 
		     Epplet_get_color(r, g, b));
   XFillRectangle(disp, win, gc, x, y, (unsigned int)w, (unsigned int)h);
}

void Epplet_draw_outline(Window win, int x, int y, int w, int h,
			 int r, int g, int b)
{
   static int cr = -1, cg = -1, cb = -1;
   static GC gc = 0;
   XGCValues gcv;

   if ((w <= 0) || (h <= 0))
      return;
   if (!gc)
      gc = XCreateGC(disp, win, 0, &gcv);   
   if ((cr != r) || (cg != g) || (cb != b))
      XSetForeground(disp, gc, 
		     Epplet_get_color(r, g, b));
   XDrawRectangle(disp, win, gc, x, y, (unsigned int)(w - 1), 
		  (unsigned int)(h - 1));
}

RGB_buf
Epplet_make_rgb_buf(int w, int h)
{
   RGB_buf buf;
   unsigned char *data;
   
   buf = malloc(sizeof(RGB_buf));
   data = malloc(w * h * 3 * sizeof(unsigned char));
   buf->im = Imlib_create_image_from_data(id, data, NULL, w, h);
   free(data);
   return buf;
}

unsigned char *
Epplet_get_rgb_pointer(RGB_buf buf)
{
   return buf->im->rgb_data;
}

void 
Epplet_paste_buf(RGB_buf buf, Window win, int x, int y)
{
   Imlib_changed_image(id, buf->im);
   Imlib_paste_image(id, buf->im, win, x, y, 
		     buf->im->rgb_width, buf->im->rgb_height);
}

static void
Epplet_handle_child(int num)
{
   int                 status;
   pid_t               pid;
   
   while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
     {
	if (WIFEXITED(status))
	  {
	     int code;
	     
	     code = WEXITSTATUS(status);
	     if (child_func)
		(*child_func) (child_data, pid, code);
	  }
     }
   return;
   num = 0;
}

int
Epplet_run_command(char *cmd)
{
   return system(cmd);
}

char *
Epplet_read_run_command(char *cmd)
{
  return (cmd);
}

int
Epplet_spawn_command(char *cmd)
{
   pid_t pid;
   
   pid = fork();
   if (pid)
      return (int)pid;
   execl("/bin/sh", "/bin/sh", "-c", cmd, NULL);
   exit(0);
}

void
Epplet_pause_spawned_command(int pid)
{
   kill((pid_t)pid, SIGSTOP);
}

void
Epplet_unpause_spawned_command(int pid)
{
   kill((pid_t)pid, SIGCONT);
}

void
Epplet_kill_spawned_command(int pid)
{
   kill((pid_t)pid, SIGINT);
}

void
Epplet_destroy_spawned_command(int pid)
{
   kill((pid_t)pid, SIGKILL);
}

void
Epplet_register_child_handler(void (*func) 
			      (void *data, int pid, int exit_code),
			      void *data)
{
   child_data = data;
   child_func = func;
}

void
Epplet_change_button_label(Epplet_gadget gadget, char *label)
{
   GadButton *g;
   GadGeneral *gg;
   
   g = (GadButton *)gadget;
   gg = (GadGeneral *)gadget;
   if (g->label)
      free(g->label);
   g->label = Estrdup(label);
   if (gg->visible)
      Epplet_draw_button(gadget);
}

void
Epplet_change_button_image(Epplet_gadget gadget, char *image)
{
   GadButton *g;
   GadGeneral *gg;

   g = (GadButton *)gadget;
   gg = (GadGeneral *)gadget;
   if (g->image)
      free(g->image);
   g->image = Estrdup(image);
   if (gg->visible)
      Epplet_draw_button(gadget);
}

void 
Epplet_clear_window(Window ww)
{
   XClearWindow(disp, ww);
}

void
Epplet_show_about(char *name)
{
   char s[1024];
   
   Esnprintf(s, sizeof(s), "%s/dox %s/epplet_icons/%s.ABOUT", EBIN, EROOT, name);
   Epplet_spawn_command(s);
}

void
Epplet_dialog_ok(char *text)
{
   char *s;
   
   s = malloc(strlen(text) + 32);
   sprintf(s, "dialog_ok %s", text);
   ECommsSend(s);
   free(s);
}

static void
Epplet_find_instance(char *name)
{
  struct stat st;
  char s[1024];
  int i = 0, fd;
  pid_t pid;

  /* make sure basic dir exists */
  Esnprintf(s, sizeof(s), "%s/.enlightenment/epplet_config", getenv("HOME"));
  if (stat(s, &st) < 0)
    {
      if (mkdir(s, S_IRWXU) < 0)
	{
	  char err[255];
	  
	  Esnprintf(err, sizeof(err), "Unable to create epplet config directory %s -- %s.\n",
		  s, strerror(errno));
	  Epplet_dialog_ok(err);
	  epplet_instance = 1;
	  return;
	}
    }

  /* make sure this epplets config dir exists */
  Esnprintf(s, sizeof(s), "%s/.enlightenment/epplet_config/%s", getenv("HOME"), name);
  conf_dir = strdup(s);
  if (stat(s, &st) < 0)
    {
      if (mkdir(s, S_IRWXU) < 0)
	{
	  char err[255];
	  
	  Esnprintf(err, sizeof(err), "Unable to create epplet config directory %s -- %s.\n",
		  s, strerror(errno));
	  Epplet_dialog_ok(err);
	  epplet_instance = 1;
	  return;
	}
    }

  /* Pick our instance number.  255 is the max to avoid infinite loops, which could be caused by
     lack of insert permissions in the config directory. */
  for (i = 1; i < 256; i++)
    {
      Esnprintf(s, sizeof(s), "%s/.lock_%i", conf_dir, i);
      if (stat(s, &st) >= 0)
	{
	  /* Lock file exists.  Read from it. */
	  if ((fd = open(s, O_RDONLY)) < 0)
	    {
	      /* Either it's there and we can't read it, or it's gone now.  Next! */
	      fprintf(stderr, "Unable to read lock file %s -- %s\n", s, strerror(errno));
	      continue;
	    }
	  if ((read(fd, &pid, sizeof(pid_t))) < ((int) sizeof(pid_t)))
	    {
	      /* We didn't get enough bytes.  Next! */
	      fprintf(stderr, "Read attempt for lock file %s failed -- %s\n", s, strerror(errno));
	      continue;
	    }
          close(fd);
	  if (pid <= 0)
	    {
	      /* We got a bogus process ID.  Next! */
	      fprintf(stderr, "Lock file %s contained a bogus process ID (%lu)\n", s, (unsigned long) pid);
	      continue;
	    }
	  if ((kill(pid, 0) == 0) || (errno != ESRCH))
	    {
	      /* The process exists.  Next! */
	      continue;
	    }
	  /* Okay, looks like a stale lockfile at this point.  Remove it. */
	  if ((unlink(s)) != 0)
	    {
	      /* Removal failed.  Next! */
	      fprintf(stderr, "Unable to remove stale lock file %s -- %s.  Please remove it manually.\n",
		      s, strerror(errno));
	      continue;
	    }
	}
      if ((fd = open(s, (O_WRONLY | O_EXCL | O_CREAT), 0600)) < 0)
	{
	  /* Apparently another process just came in under us and created it.  Next! */
	  continue;
	}
      pid = getpid();
      write(fd, &pid, sizeof(pid_t));  /* Not sure how best to deal with write errors here */
      close(fd);
      /* If we made it here, we've just written the lock file and saved it.  We have our instance
	 number, so exit the loop. */
      break;
    }

  /* Anything this high is probably an error. */
  if (i >= 255)
    {
      i = 1;
    }
  epplet_instance = i;
  
  /* find out epplet's name */
  if (epplet_instance > 1)
    {
      Esnprintf(s, sizeof(s), "%s-%i", name, epplet_instance);
      epplet_name = strdup(s);
    }
  else
    epplet_name = strdup(name);
}

int
Epplet_get_instance(void)
{
   return epplet_instance;
}

void
Epplet_add_config(char *key, char *value)
{
  if (!config_dict)
    {
      config_dict = (ConfigDict*) malloc(sizeof(ConfigDict));
      memset(config_dict, 0, sizeof(ConfigDict));
    }
  if (config_dict && key)
    {
      config_dict->entries = realloc(config_dict->entries, sizeof(ConfigItem) * (config_dict->num_entries + 1));
      config_dict->entries[config_dict->num_entries].key = strdup(key);
      config_dict->entries[config_dict->num_entries].value = (value ? strdup(value) : strdup(""));
      config_dict->num_entries++;
    }
}

void
Epplet_load_config(void)
{
  char s[1024], s2[1024], s3[1024];
  FILE         *f = NULL;

  /* If they haven't initialized, abort */
  if (epplet_instance == 0)
    return;

  /* create config file name */
  Esnprintf(s, sizeof(s), "%s/%s.cfg", conf_dir, epplet_name);
  epplet_cfg_file = strdup(s);
  
  config_dict = (ConfigDict*) malloc(sizeof(ConfigDict));
  memset(config_dict, 0, sizeof(ConfigDict));

  if ((f = fopen(epplet_cfg_file, "r")) == NULL)
    return;
  *s2 = 0;
  for (; fgets(s, sizeof(s), f);)
    {
      sscanf(s, "%s %[^\n]\n", (char *) &s2, (char *) &s3);
      if (!(*s2) || (!*s3) || (*s2 == '\n') || (*s2 == '#'))
        {
          continue;
        }
      Epplet_add_config(s2, s3);
    }
  fclose(f);
  return;
}

void
Epplet_save_config(void)
{
   FILE *f;
   int i;

   if (!(config_dict) || (config_dict->num_entries <= 0))
     return;

   if (!(f = fopen(epplet_cfg_file, "w")))
     {
       char err[255];
           
       Esnprintf(err, sizeof(err), "Unable to write to config file %s -- %s.\n", epplet_cfg_file, strerror(errno));
       Epplet_dialog_ok(err);
       return;
     }
   fprintf(f, "### Automatically generated Epplet config file for %s.\n\n", epplet_name);
   for (i = 0; i < config_dict->num_entries; i++)
     {
       if (config_dict->entries[i].key)
         {
           fprintf(f, "%s %s\n", config_dict->entries[i].key, config_dict->entries[i].value);
         }
     }
   fclose(f);
}

char *
Epplet_query_config(char *key)
{
  int i;
  ConfigItem *ci;

  for (i = 0; i < config_dict->num_entries; i++)
    {
      ci = &(config_dict->entries[i]);
      if ((ci->key) && !strcmp(key, ci->key))
        /* we've found the key */
        return (ci->value);
    }
  return ((char *) NULL);
}

char *
Epplet_query_config_def(char *key, char *def)
{
  int i;
  ConfigItem *ci;

  for (i = 0; i < config_dict->num_entries; i++)
    {
      ci = &(config_dict->entries[i]);
      if ((ci->key) && !strcmp(key, ci->key))
        /* we've found the key */
        return (ci->value);
    }
  Epplet_add_config(key, def);  /* Not found.  Add the default. */
  return (def);
}

void
Epplet_modify_config(char *key, char *value)
{
  int i;
  ConfigItem *ci;

  for (i = 0; i < config_dict->num_entries; i++)
    {
      ci = &(config_dict->entries[i]);
      if ((ci->key) && !strcmp(key, ci->key))
        /* we've found the key */
        {
          if (ci->value != value)
            {
              free(ci->value);
            }
          else if (value)
            {
              ci->value = strdup(value);
            }
          else
            {
              ci->value = strdup("");
            }
          return;
        }
    }

  /* so we couldn't find the key, thus add it ...*/
  Epplet_add_config(key, value);
}

int 
Epplet_get_hslider_clicked(Epplet_gadget gadget)
{
   GadHSlider *g;
   
   g = (GadHSlider *)gadget;
   return (int)g->clicked;
}

int 
Epplet_get_vslider_clicked(Epplet_gadget gadget)
{
   GadVSlider *g;
   
   g = (GadVSlider *)gadget;
   return (int)g->clicked;
}
