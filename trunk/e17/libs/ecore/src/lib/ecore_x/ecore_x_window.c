/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"
#include "Ecore_X_Atoms.h"

/**
 * @defgroup Ecore_X_Window_Create_Group X Window Creation Functions
 *
 * Functions that can be used to create an X window.
 */

/**
 * Creates a new window.
 * @param   parent The parent window to use.  If @p parent is @c 0, the root
 *                 window of the default display is used.
 * @param   x      X position.
 * @param   y      Y position.
 * @param   w      Width.
 * @param   h      Height.
 * @return  The new window handle.
 * @ingroup Ecore_X_Window_Create_Group
 */
Ecore_X_Window
ecore_x_window_new(Ecore_X_Window parent, int x, int y, int w, int h)
{
   Window               win;
   XSetWindowAttributes attr;
   
   if (parent == 0) parent = DefaultRootWindow(_ecore_x_disp);
   attr.backing_store         = NotUseful;
   attr.override_redirect     = False;
   attr.colormap              = DefaultColormap(_ecore_x_disp, DefaultScreen(_ecore_x_disp));
   attr.border_pixel          = 0;
   attr.background_pixmap     = None;
   attr.save_under            = False;
   attr.do_not_propagate_mask = True;
   attr.event_mask            = KeyPressMask |
                                KeyReleaseMask |
                                ButtonPressMask |
                                ButtonReleaseMask |
                                EnterWindowMask |
                                LeaveWindowMask |
                                PointerMotionMask |
                                ExposureMask |
                                VisibilityChangeMask |
                                StructureNotifyMask |
                                FocusChangeMask |
                                PropertyChangeMask |
                                ColormapChangeMask;
   win = XCreateWindow(_ecore_x_disp, parent,
		       x, y, w, h, 0,
		       0, /*DefaultDepth(_ecore_x_disp, DefaultScreen(_ecore_x_disp)),*/
		       InputOutput, 
		       CopyFromParent, /*DefaultVisual(_ecore_x_disp, DefaultScreen(_ecore_x_disp)),*/
		       CWBackingStore |
		       CWOverrideRedirect | 
/*		       CWColormap | */
		       CWBorderPixel |
		       CWBackPixmap | 
		       CWSaveUnder | 
		       CWDontPropagate | 
		       CWEventMask,
		       &attr);

   if (parent == DefaultRootWindow(_ecore_x_disp)) ecore_x_window_defaults_set(win);
   return win;
}

/**
 * Creates a window with the override redirect attribute set to @c True.
 * @param   parent The parent window to use.  If @p parent is @c 0, the root
 *                 window of the default display is used.
 * @param   x      X position.
 * @param   y      Y position.
 * @param   w      Width.
 * @param   h      Height.
 * @return  The new window handle.
 * @ingroup Ecore_X_Window_Create_Group
 */
Ecore_X_Window
ecore_x_window_override_new(Ecore_X_Window parent, int x, int y, int w, int h)
{
   Window               win;
   XSetWindowAttributes attr;
   
   if (parent == 0) parent = DefaultRootWindow(_ecore_x_disp);
   attr.backing_store         = NotUseful;
   attr.override_redirect     = True;
   attr.colormap              = DefaultColormap(_ecore_x_disp, DefaultScreen(_ecore_x_disp));
   attr.border_pixel          = 0;
   attr.background_pixmap     = None;
   attr.save_under            = False;
   attr.do_not_propagate_mask = True;
   attr.event_mask            = KeyPressMask |
                                KeyReleaseMask |
                                ButtonPressMask |
                                ButtonReleaseMask |
                                EnterWindowMask |
                                LeaveWindowMask |
                                PointerMotionMask |
                                ExposureMask |
                                VisibilityChangeMask |
                                StructureNotifyMask |
                                FocusChangeMask |
                                PropertyChangeMask |
                                ColormapChangeMask;
   win = XCreateWindow(_ecore_x_disp, parent,
		       x, y, w, h, 0,
		       0, /*DefaultDepth(_ecore_x_disp, DefaultScreen(_ecore_x_disp)),*/
		       InputOutput, 
		       CopyFromParent, /*DefaultVisual(_ecore_x_disp, DefaultScreen(_ecore_x_disp)),*/
		       CWBackingStore |
		       CWOverrideRedirect | 
/*		       CWColormap | */
		       CWBorderPixel |
		       CWBackPixmap | 
		       CWSaveUnder | 
		       CWDontPropagate | 
		       CWEventMask,
		       &attr);

   if (parent == DefaultRootWindow(_ecore_x_disp)) ecore_x_window_defaults_set(win);
   return win;
}

/**
 * Creates a new input window.
 * @param   parent The parent window to use.    If @p parent is @c 0, the root
 *                 window of the default display is used.
 * @param   x      X position.
 * @param   y      Y position.
 * @param   w      Width.
 * @param   h      Height.
 * @return  The new window.
 * @ingroup Ecore_X_Window_Create_Group
 */
Ecore_X_Window
ecore_x_window_input_new(Ecore_X_Window parent, int x, int y, int w, int h)
{
   Window               win;
   XSetWindowAttributes attr;
   
   if (parent == 0) parent = DefaultRootWindow(_ecore_x_disp);
   attr.override_redirect     = True;
   attr.do_not_propagate_mask = True;
   attr.event_mask            = KeyPressMask |
                                KeyReleaseMask |
                                ButtonPressMask |
                                ButtonReleaseMask |
                                EnterWindowMask |
                                LeaveWindowMask |
                                PointerMotionMask |
                                ExposureMask |
                                VisibilityChangeMask |
                                StructureNotifyMask |
                                FocusChangeMask |
                                PropertyChangeMask |
                                ColormapChangeMask;
   win = XCreateWindow(_ecore_x_disp, parent,
		       x, y, w, h, 0,
		       0, 
		       InputOnly,
		       CopyFromParent, /*DefaultVisual(_ecore_x_disp, DefaultScreen(_ecore_x_disp)),*/
		       CWOverrideRedirect | 
		       CWDontPropagate | 
		       CWEventMask,
		       &attr);

   if (parent == DefaultRootWindow(_ecore_x_disp))
     {
     }
   return win;
}

/**
 * @defgroup Evas_X_Window_Properties_Group X Window Property Functions
 *
 * Functions that set window properties.
 */

/**
 * Sets the default properties for the given window.
 *
 * The default properties set for the window are @c WM_CLIENT_MACHINE and
 * @c _NET_WM_PID.
 *
 * @param   win The given window.
 * @ingroup Evas_X_Window_Properties_Groups
 */
void
ecore_x_window_defaults_set(Ecore_X_Window win)
{
   long pid;
   char buf[MAXHOSTNAMELEN];
   char *hostname[1];
   int argc;
   char **argv;
   XTextProperty xprop;

   /*
    * Set WM_CLIENT_MACHINE.
    */
   gethostname(buf, MAXHOSTNAMELEN);
   buf[MAXHOSTNAMELEN - 1] = '\0';
   hostname[0] = buf;
   /* The ecore function uses UTF8 which Xlib may not like (especially
    * with older clients) */
   /* ecore_x_window_prop_string_set(win, ECORE_X_ATOM_WM_CLIENT_MACHINE,
				  (char *)buf); */
   if (XStringListToTextProperty(hostname, 1, &xprop))
     {
	XSetWMClientMachine(_ecore_x_disp, win, &xprop);
	XFree(xprop.value);
     }

   /*
    * Set _NET_WM_PID
    */
   pid = getpid();
   ecore_x_window_prop_property_set(win, ECORE_X_ATOM_NET_WM_PID, XA_CARDINAL,
		                    32, &pid, 1);

   ecore_x_window_prop_window_type_set(win, ECORE_X_WINDOW_TYPE_NORMAL);

   ecore_app_args_get(&argc, &argv);
   ecore_x_window_prop_command_set(win, argc, argv);
}

void
ecore_x_window_configure(Ecore_X_Window win,
                         Ecore_X_Window_Configure_Mask mask,
                         int x, int y, int w, int h,
                         int border_width, Ecore_X_Window sibling,
                         int stack_mode)
{
   XWindowChanges xwc;

   if (!win)
      return;

   xwc.x = x;
   xwc.y = y;
   xwc.width = w;
   xwc.height = h;
   xwc.border_width = border_width;
   xwc.sibling = sibling;
   xwc.stack_mode = stack_mode;

   XConfigureWindow(_ecore_x_disp, win, mask, &xwc);
}

/**
 * @defgroup Evas_X_Window_Destroy_Group X Window Destroy Functions
 *
 * Functions to destroy X windows.
 */

/**
 * Deletes the given window.
 * @param   win The given window.
 * @ingroup Evas_X_Window_Destroy_Group
 */
void
ecore_x_window_del(Ecore_X_Window win)
{
   /* sorry sir, deleting the root window doesn't sound like
    * a smart idea.
    */
   if (win)
      XDestroyWindow(_ecore_x_disp, win);
}

/**
 * Sends a delete request to the given window.
 * @param   win The given window.
 * @ingroup Evas_X_Window_Destroy_Group
 */
void
ecore_x_window_delete_request_send(Ecore_X_Window win)
{
   XEvent xev;

   /* sorry sir, deleting the root window doesn't sound like
    * a smart idea.
    */
   if (!win)
      return;

   xev.xclient.type = ClientMessage;
   xev.xclient.display = _ecore_x_disp;
   xev.xclient.window = win;
   xev.xclient.message_type = ECORE_X_ATOM_WM_PROTOCOLS;
   xev.xclient.format = 32;
   xev.xclient.data.l[0] = ECORE_X_ATOM_WM_DELETE_WINDOW;
   xev.xclient.data.l[1] = CurrentTime;

   XSendEvent(_ecore_x_disp, win, False, NoEventMask, &xev);
}

/**
 * @defgroup Evas_X_Window_Visibility_Group X Window Visibility Functions
 *
 * Functions to access and change the visibility of X windows.
 */

/**
 * Shows a window.
 *
 * Synonymous to "mapping" a window in X Window System terminology.
 *
 * @param   win The window to show.
 * @ingroup Evas_X_Window_Visibility
 */
void
ecore_x_window_show(Ecore_X_Window win)
{
   XMapWindow(_ecore_x_disp, win);
}

/**
 * Hides a window.
 *
 * Synonymous to "unmapping" a window in X Window System terminology.
 *
 * @param   win The window to hide.
 * @ingroup Evas_X_Window_Visibility
 */
void
ecore_x_window_hide(Ecore_X_Window win)
{
   XUnmapWindow(_ecore_x_disp, win);
}

/**
 * @defgroup Ecore_X_Window_Geometry_Group X Window Geometry Functions
 *
 * Functions that change or retrieve the geometry of X windows.
 */

/**
 * Moves a window to the position @p x, @p y.
 *
 * The position is relative to the upper left hand corner of the
 * parent window.
 *
 * @param   win The window to move.
 * @param   x   X position.
 * @param   y   Y position.
 * @ingroup Ecore_X_Window_Geometry_Group
 */
void
ecore_x_window_move(Ecore_X_Window win, int x, int y)
{
   XMoveWindow(_ecore_x_disp, win, x, y);
}

/**
 * Resizes a window.
 * @param   win The window to resize.
 * @param   w   New width of the window.
 * @param   h   New height of the window.
 * @ingroup Ecore_X_Window_Geometry_Group
 */
void
ecore_x_window_resize(Ecore_X_Window win, int w, int h)
{
   if (w < 1) w = 1;
   if (h < 1) h = 1;
   XResizeWindow(_ecore_x_disp, win, w, h);
}

/**
 * Moves and resizes a window.
 * @param   win The window to move and resize.
 * @param   x   New X position of the window.
 * @param   y   New Y position of the window.
 * @param   w   New width of the window.
 * @param   h   New height of the window.
 * @ingroup Ecore_X_Window_Geometry_Group
 */
void
ecore_x_window_move_resize(Ecore_X_Window win, int x, int y, int w, int h)
{
   if (w < 1) w = 1;
   if (h < 1) h = 1;
   XMoveResizeWindow(_ecore_x_disp, win, x, y, w, h);
}

/**
 * @defgroup Ecore_X_Window_Focus_Functions X Window Focus Functions
 *
 * Functions that give the focus to an X Window.
 */

/**
 * Sets the focus to the window @p win.
 * @param   win The window to focus.
 * @ingroup Ecore_X_Window_Focus_Functions
 */
void
ecore_x_window_focus(Ecore_X_Window win)
{
   if (win == 0) win = DefaultRootWindow(_ecore_x_disp);   
   XSetInputFocus(_ecore_x_disp, win, RevertToNone, CurrentTime);
}

/**
 * Sets the focus to the given window at a specific time.
 * @param   win The window to focus.
 * @param   t   When to set the focus to the window.
 * @ingroup Ecore_X_Window_Focus_Functions
 */
void
ecore_x_window_focus_at_time(Ecore_X_Window win, Ecore_X_Time t)
{
   if (win == 0) win = DefaultRootWindow(_ecore_x_disp);   
   XSetInputFocus(_ecore_x_disp, win, RevertToNone, t);
}

/**
 * gets the focus to the window @p win.
 * @return  The window that has focus.
 * @ingroup Ecore_X_Window_Focus_Functions
 */
Ecore_X_Window
ecore_x_window_focus_get(void)
{
   Window win;
   int revert_mode;
   
   win = 0;
   
   XGetInputFocus(_ecore_x_disp, &win, &revert_mode);
   return win;
}

/**
 * @defgroup Ecore_X_Window_Z_Order_Group X Window Z Order Functions
 *
 * Functions that change the Z order of X windows.
 */

/**
 * Raises the given window.
 * @param   win The window to raise.
 * @ingroup Ecore_X_Window_Z_Order_Group
 */
void
ecore_x_window_raise(Ecore_X_Window win)
{
   XRaiseWindow(_ecore_x_disp, win);
}

/**
 * Lowers the given window.
 * @param   win The window to lower.
 * @ingroup Ecore_X_Window_Z_Order_Group
 */
void
ecore_x_window_lower(Ecore_X_Window win)
{
   XLowerWindow(_ecore_x_disp, win);
}

/**
 * @defgroup Ecore_X_Window_Parent_Group X Window Parent Functions
 *
 * Functions that retrieve or changes the parent window of a window.
 */

/**
 * Moves a window to within another window at a given position.
 * @param   win        The window to reparent.
 * @param   new_parent The new parent window.
 * @param   x          X position within new parent window.
 * @param   y          Y position within new parent window.
 * @ingroup Ecore_X_Window_Parent_Group
 */
void
ecore_x_window_reparent(Ecore_X_Window win, Ecore_X_Window new_parent, int x, int y)
{
   if (new_parent == 0) new_parent = DefaultRootWindow(_ecore_x_disp);   
   XReparentWindow(_ecore_x_disp, win, new_parent, x, y);
}

/**
 * Retrieves the size of the given window.
 * @param   win The given window.
 * @param   w   Pointer to an integer into which the width is to be stored.
 * @param   h   Pointer to an integer into which the height is to be stored.
 * @ingroup Ecore_X_Window_Geometry_Group
 */
void
ecore_x_window_size_get(Ecore_X_Window win, int *w, int *h)
{
   int dummy_x, dummy_y;
   
   if (win == 0) 
      win = DefaultRootWindow(_ecore_x_disp);

   ecore_x_drawable_geometry_get(win, &dummy_x, &dummy_y, w, h);
}

/**
 * Retrieves the geometry of the given window.
 * @param   win The given window.
 * @param   x   Pointer to an integer in which the X position is to be stored.
 * @param   y   Pointer to an integer in which the Y position is to be stored.
 * @param   w   Pointer to an integer in which the width is to be stored.
 * @param   h   Pointer to an integer in which the height is to be stored.
 * @ingroup Ecore_X_Window_Geometry_Group
 */
void
ecore_x_window_geometry_get(Ecore_X_Window win, int *x, int *y, int *w, int *h)
{
   if (!win)
      win = DefaultRootWindow(_ecore_x_disp);

   ecore_x_drawable_geometry_get(win, x, y, w, h);
}

/**
 * Retrieves the width of the border of the given window.
 * @param   win The given window.
 * @return  Width of the border of @p win.
 * @ingroup Ecore_X_Window_Geometry_Group
 */
int
ecore_x_window_border_width_get(Ecore_X_Window win)
{
   /* doesn't make sense to call this on a root window */
   if (!win)
      return 0;

   return ecore_x_drawable_border_width_get(win);
}

/**
 * Sets the width of the border of the given window.
 * @param   win The given window.
 * @param   width The new border width.
 * @ingroup Ecore_X_Window_Geometry_Group
 */
void
ecore_x_window_border_width_set(Ecore_X_Window win, int width)
{
   /* doesn't make sense to call this on a root window */
   if (!win)
      return;

   XSetWindowBorderWidth (_ecore_x_disp, win, width);
}

/**
 * Retrieves the depth of the given window.
 * @param  win The given window.
 * @return Depth of the window.
 */
int
ecore_x_window_depth_get(Ecore_X_Window win)
{
   return ecore_x_drawable_depth_get(win);
}
  
/**
 * To be documented.
 *
 * FIXME: To be fixed.
 */
void
ecore_x_window_cursor_show(Ecore_X_Window win, int show)
{
   if (win == 0) win = DefaultRootWindow(_ecore_x_disp);
   if (!show)
     {
	Cursor              c;
	XColor              cl;
	Pixmap              p, m;
	GC                  gc;
	XGCValues           gcv;
	
	p = XCreatePixmap(_ecore_x_disp, win, 1, 1, 1);
	m = XCreatePixmap(_ecore_x_disp, win, 1, 1, 1);
	gc = XCreateGC(_ecore_x_disp, m, 0, &gcv);
	XSetForeground(_ecore_x_disp, gc, 0);
	XDrawPoint(_ecore_x_disp, m, gc, 0, 0);
	XFreeGC(_ecore_x_disp, gc);
	c = XCreatePixmapCursor(_ecore_x_disp, p, m, &cl, &cl, 0, 0);
	XDefineCursor(_ecore_x_disp, win, c);
	XFreeCursor(_ecore_x_disp, c);
	XFreePixmap(_ecore_x_disp, p);
	XFreePixmap(_ecore_x_disp, m);
     }
   else
     {
	XDefineCursor(_ecore_x_disp, win, 0);	
     }
}

void
ecore_x_window_cursor_set(Ecore_X_Window win, Ecore_X_Cursor c)
{
   if (c == 0)
     XUndefineCursor(_ecore_x_disp, win);
   else
     XDefineCursor(_ecore_x_disp, win, c);
}

/**
 * Finds out whether the given window is currently visible.
 * @param   win The given window.
 * @return  1 if the window is visible, otherwise 0.
 * @ingroup Ecore_X_Window_Visibility_Group
 */
int
ecore_x_window_visible_get(Ecore_X_Window win)
{
   XWindowAttributes attr;

   return (XGetWindowAttributes(_ecore_x_disp, win, &attr) &&
           (attr.map_state == IsViewable));
}

static Window
_ecore_x_window_at_xy_get(Window base, int bx, int by, int x, int y)
{
   Window           *list = NULL;
   Window            parent_win = 0, child = 0, root_win = 0;
   int               i, wx, wy, ww, wh;
   unsigned int      num;

   if (!ecore_x_window_visible_get(base))
      return 0;

   ecore_x_window_geometry_get(base, &wx, &wy, &ww, &wh);
   wx += bx;
   wy += by;

   if (!((x >= wx) && (y >= wy) && (x < (wx + ww)) && (y < (wy + wh))))
      return 0;
   
   if (!XQueryTree(_ecore_x_disp, base, &root_win, &parent_win, &list, &num))
      return base;

   if (list)
   {
      for (i = num - 1;; --i)
      {
         if ((child = _ecore_x_window_at_xy_get(list[i], wx, wy, x, y)))
         {
            XFree(list);
            return child;
         }
         if (!i)
            break;
      }
      XFree(list);
   }

   return base;
}

/**
 * Retrieves the top, visible window at the given location.
 * @param   x The given X position.
 * @param   y The given Y position.
 * @return  The window at that position.
 * @ingroup Ecore_X_Window_Geometry_Group
 */
Ecore_X_Window
ecore_x_window_at_xy_get(int x, int y)
{
   Ecore_X_Window    win, root;
   
   /* FIXME: Proper function to determine current root/virtual root
    * window missing here */
   root = DefaultRootWindow(_ecore_x_disp);
   
   ecore_x_grab();
   win = _ecore_x_window_at_xy_get(root, 0, 0, x, y);
   ecore_x_ungrab();
   
   return win ? win : root;
}

/**
 * Retrieves the parent window of the given window.
 * @param   win The given window.
 * @return  The parent window of @p win.
 * @ingroup Ecore_X_Window_Parent_Group
 */
Ecore_X_Window
ecore_x_window_parent_get(Ecore_X_Window win)
{
   Window         root, parent, *children = NULL;
   unsigned int   num;

   if (!XQueryTree(_ecore_x_disp, win, &root, &parent, &children, &num))
      return 0;
   if (children)
      XFree(children);

   return parent;
}

/**
 * Sets the background color of the given window.
 * @param win   The given window
 * @param color The color to set to (i.e. 0xff0000)
 */
void
ecore_x_window_background_color_set(Ecore_X_Window win, unsigned short r,
				    unsigned short g, unsigned short b)
{
   XSetWindowAttributes attr;
   XColor col;

   col.red = r;
   col.green = g;
   col.blue = b;
   
   XAllocColor(_ecore_x_disp, DefaultColormap(_ecore_x_disp,
					      DefaultScreen(_ecore_x_disp)),
	       &col);
   
   attr.background_pixel      = col.pixel;
   XChangeWindowAttributes(_ecore_x_disp, win, CWBackPixel, &attr);
}

void
ecore_x_window_gravity_set(Ecore_X_Window win, Ecore_X_Gravity grav)
{
   XSetWindowAttributes att;
   
   att.win_gravity = grav;
   XChangeWindowAttributes(_ecore_x_disp, win, CWWinGravity, &att);
}

void
ecore_x_window_pixel_gravity_set(Ecore_X_Window win, Ecore_X_Gravity grav)
{
   XSetWindowAttributes att;
   
   att.bit_gravity = grav;
   XChangeWindowAttributes(_ecore_x_disp, win, CWBitGravity, &att);
}
