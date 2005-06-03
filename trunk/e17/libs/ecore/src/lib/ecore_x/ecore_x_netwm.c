/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
/*
 * _NET_WM... aka Extended Window Manager Hint (EWMH) functions.
 */
#include "config.h"
#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"
#include "Ecore_X_Atoms.h"

/*
 * Convenience macros
 */
#define _ATOM_GET(name) \
   XInternAtom(_ecore_x_disp, name, False)

#define _ATOM_SET_UTF8_STRING(win, atom, string) \
   XChangeProperty(_ecore_x_disp, win, atom, ECORE_X_ATOM_UTF8_STRING, 8, PropModeReplace, \
                   (unsigned char *)string, strlen(string))
#define _ATOM_SET_UTF8_STRING_LIST(win, atom, string, cnt) \
   XChangeProperty(_ecore_x_disp, win, atom, ECORE_X_ATOM_UTF8_STRING, 8, PropModeReplace, \
                   (unsigned char *)string, cnt)
#define _ATOM_SET_WINDOW(win, atom, p_wins, cnt) \
   XChangeProperty(_ecore_x_disp, win, atom, XA_WINDOW, 32, PropModeReplace, \
                   (unsigned char *)p_wins, cnt)
#define _ATOM_SET_ATOM(win, atom, p_atom, cnt) \
   XChangeProperty(_ecore_x_disp, win, atom, XA_ATOM, 32, PropModeReplace, \
                   (unsigned char *)p_atom, cnt)
#define _ATOM_SET_CARD32(win, atom, p_val, cnt) \
   XChangeProperty(_ecore_x_disp, win, atom, XA_CARDINAL, 32, PropModeReplace, \
                   (unsigned char *)p_val, cnt)

/*
 * Convenience functions. Should probably go elsewhere.
 */

/*
 * Set CARD32 (array) property
 */
void
ecore_x_window_prop_card32_set(Ecore_X_Window win, Ecore_X_Atom atom,
			       unsigned int *val, unsigned int num)
{
#if SIZEOF_INT == 4
   _ATOM_SET_CARD32(win, atom, val, num);
#else
   CARD32             *c32;
   unsigned int        i;

   c32 = malloc(num * sizeof(CARD32));
   if (!c32)
      return;
   for (i = 0; i < num; i++)
      c32[i] = val[i];
   _ATOM_SET_CARD32(win, atom, c32, num);
   free(c32);
#endif
}

/*
 * Get CARD32 (array) property
 *
 * At most len items are returned in val.
 * If the property was successfully fetched the number of items stored in
 * val is returned, otherwise -1 is returned.
 * Note: Return value 0 means that the property exists but has no elements.
 */
int
ecore_x_window_prop_card32_get(Ecore_X_Window win, Ecore_X_Atom atom,
			       unsigned int *val, unsigned int len)
{
   unsigned char      *prop_ret;
   Atom                type_ret;
   unsigned long       bytes_after, num_ret;
   int                 format_ret;
   unsigned int        i;
   int                 num;

   prop_ret = NULL;
   XGetWindowProperty(_ecore_x_disp, win, atom, 0, 0x7fffffff, False,
		      XA_CARDINAL, &type_ret, &format_ret, &num_ret,
		      &bytes_after, &prop_ret);
   if (prop_ret && type_ret == XA_CARDINAL && format_ret == 32)
     {
	if (num_ret < len)
	   len = num_ret;
	for (i = 0; i < len; i++)
	   val[i] = ((unsigned long *)prop_ret)[i];
	num = len;
     }
   else
     {
	num = -1;
     }
   if (prop_ret)
      XFree(prop_ret);

   return num;
}

/*
 * Set UTF-8 string property
 */
static void
_ecore_x_window_prop_string_utf8_set(Ecore_X_Window win, Ecore_X_Atom atom,
				     const char *str)
{
   _ATOM_SET_UTF8_STRING(win, atom, str);
}

/*
 * Get UTF-8 string property
 */
static char *
_ecore_x_window_prop_string_utf8_get(Ecore_X_Window win, Ecore_X_Atom atom)
{
   char               *str;
   unsigned char      *prop_ret;
   Atom                type_ret;
   unsigned long       bytes_after, num_ret;
   int                 format_ret;

   str = NULL;
   prop_ret = NULL;
   XGetWindowProperty(_ecore_x_disp, win, atom, 0, 0x7fffffff, False,
		      ECORE_X_ATOM_UTF8_STRING, &type_ret,
		      &format_ret, &num_ret, &bytes_after, &prop_ret);
   if (prop_ret && num_ret > 0 && format_ret == 8)
     {
	str = malloc(num_ret + 1);
	if (str)
	  {
	     memcpy(str, prop_ret, num_ret);
	     str[num_ret] = '\0';
	  }
     }
   if (prop_ret)
      XFree(prop_ret);

   return str;
}

#if 0 /* Unused */
/* Set/clear atom in list */
static void
_ecore_x_netwm_atom_list_set(Ecore_X_Atom *atoms, int size, int *count,
                             Ecore_X_Atom atom, int set)
{
   int   i, n, in_list = 0;

   n = *count;
   /* Check if atom is in list or not (+get index) */
   for (i = 0; i < n; i++)
      if (atoms[i] == atom)
	{
	   in_list = 1;
	   break;
	}

   if (set && !in_list)
   {
      /* Add it (if space left) */
	   if (n < size)
	      atoms[n++] = atom;
	   *count = n;
   }
   else if (!set && in_list)
   {
	   /* Remove it */
   	atoms[i] = atoms[--n];
	   *count = n;
   }
}
#endif


/*
 * Root window NetWM hints.
 */
Ecore_X_Atom        ECORE_X_ATOM_NET_SUPPORTED = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_SUPPORTING_WM_CHECK = 0;

Ecore_X_Atom        ECORE_X_ATOM_NET_NUMBER_OF_DESKTOPS = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_VIRTUAL_ROOTS = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_DESKTOP_NAMES = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_DESKTOP_GEOMETRY = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_DESKTOP_VIEWPORT = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_DESKTOP_LAYOUT = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WORKAREA = 0;

Ecore_X_Atom        ECORE_X_ATOM_NET_CURRENT_DESKTOP = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_SHOWING_DESKTOP = 0;

Ecore_X_Atom        ECORE_X_ATOM_NET_CLIENT_LIST = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_CLIENT_LIST_STACKING = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_ACTIVE_WINDOW = 0;

/*
 * Client message types.
 */
Ecore_X_Atom        ECORE_X_ATOM_NET_CLOSE_WINDOW = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_MOVERESIZE = 0;

/*
 * Pagers
 */
Ecore_X_Atom        ECORE_X_ATOM_NET_MOVERESIZE_WINDOW = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_RESTACK_WINDOW = 0;

/*
 * Application window specific NetWM hints.
 */
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_NAME = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_VISIBLE_NAME = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_ICON_NAME = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_VISIBLE_ICON_NAME = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_DESKTOP = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_STRUT = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_STRUT_PARTIAL = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_ICON_GEOMETRY = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_ICON = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_PID = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_HANDLED_ICONS = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_USER_TIME = 0;

Ecore_X_Atom        ECORE_X_ATOM_NET_WM_ALLOWED_ACTIONS = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_ACTION_MOVE = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_ACTION_RESIZE = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_ACTION_MINIMIZE = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_ACTION_SHADE = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_ACTION_STICK = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_ACTION_MAXIMIZE_HORZ = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_ACTION_MAXIMIZE_VERT = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_ACTION_FULLSCREEN = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_ACTION_CHANGE_DESKTOP = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_ACTION_CLOSE = 0;

Ecore_X_Atom        ECORE_X_ATOM_NET_WM_WINDOW_TYPE = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DESKTOP = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DOCK = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_WINDOW_TYPE_TOOLBAR = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_WINDOW_TYPE_MENU = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_WINDOW_TYPE_UTILITY = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_WINDOW_TYPE_SPLASH = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DIALOG = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_WINDOW_TYPE_NORMAL = 0;

Ecore_X_Atom        ECORE_X_ATOM_NET_WM_STATE = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_STATE_MODAL = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_STATE_STICKY = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_VERT = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_HORZ = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_STATE_SHADED = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_STATE_SKIP_TASKBAR = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_STATE_SKIP_PAGER = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_STATE_HIDDEN = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_STATE_FULLSCREEN = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_STATE_ABOVE = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_STATE_BELOW = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_STATE_DEMANDS_ATTENTION = 0;

Ecore_X_Atom        ECORE_X_ATOM_NET_WM_WINDOW_OPACITY = 0;

Ecore_X_Atom        ECORE_X_ATOM_NET_FRAME_EXTENTS = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_REQUEST_FRAME_EXTENTS = 0;

Ecore_X_Atom        ECORE_X_ATOM_NET_WM_PING = 0;
Ecore_X_Atom        ECORE_X_ATOM_NET_WM_SYNC_REQUEST = 0;

void
ecore_x_netwm_init(void)
{
   ECORE_X_ATOM_NET_SUPPORTED = _ATOM_GET("_NET_SUPPORTED");
   ECORE_X_ATOM_NET_SUPPORTING_WM_CHECK = _ATOM_GET("_NET_SUPPORTING_WM_CHECK");

   ECORE_X_ATOM_NET_NUMBER_OF_DESKTOPS = _ATOM_GET("_NET_NUMBER_OF_DESKTOPS");
   ECORE_X_ATOM_NET_VIRTUAL_ROOTS = _ATOM_GET("_NET_VIRTUAL_ROOTS");
   ECORE_X_ATOM_NET_DESKTOP_NAMES = _ATOM_GET("_NET_DESKTOP_NAMES");
   ECORE_X_ATOM_NET_DESKTOP_GEOMETRY = _ATOM_GET("_NET_DESKTOP_GEOMETRY");
   ECORE_X_ATOM_NET_DESKTOP_VIEWPORT = _ATOM_GET("_NET_DESKTOP_VIEWPORT");
   ECORE_X_ATOM_NET_DESKTOP_LAYOUT = _ATOM_GET("_NET_DESKTOP_LAYOUT");
   ECORE_X_ATOM_NET_WORKAREA = _ATOM_GET("_NET_WORKAREA");

   ECORE_X_ATOM_NET_CURRENT_DESKTOP = _ATOM_GET("_NET_CURRENT_DESKTOP");
   ECORE_X_ATOM_NET_SHOWING_DESKTOP = _ATOM_GET("_NET_SHOWING_DESKTOP");

   ECORE_X_ATOM_NET_CLIENT_LIST = _ATOM_GET("_NET_CLIENT_LIST");
   ECORE_X_ATOM_NET_CLIENT_LIST_STACKING =
      _ATOM_GET("_NET_CLIENT_LIST_STACKING");
   ECORE_X_ATOM_NET_ACTIVE_WINDOW = _ATOM_GET("_NET_ACTIVE_WINDOW");

   ECORE_X_ATOM_NET_CLOSE_WINDOW = _ATOM_GET("_NET_CLOSE_WINDOW");
   ECORE_X_ATOM_NET_WM_MOVERESIZE = _ATOM_GET("_NET_WM_MOVERESIZE");

   ECORE_X_ATOM_NET_MOVERESIZE_WINDOW = _ATOM_GET("_NET_MOVERESIZE_WINDOW");
   ECORE_X_ATOM_NET_RESTACK_WINDOW = _ATOM_GET("_NET_RESTACK_WINDOW");

   ECORE_X_ATOM_NET_WM_NAME = _ATOM_GET("_NET_WM_NAME");
   ECORE_X_ATOM_NET_WM_VISIBLE_NAME = _ATOM_GET("_NET_WM_VISIBLE_NAME");
   ECORE_X_ATOM_NET_WM_ICON_NAME = _ATOM_GET("_NET_WM_ICON_NAME");
   ECORE_X_ATOM_NET_WM_VISIBLE_ICON_NAME =
      _ATOM_GET("_NET_WM_VISIBLE_ICON_NAME");
   ECORE_X_ATOM_NET_WM_DESKTOP = _ATOM_GET("_NET_WM_DESKTOP");
   ECORE_X_ATOM_NET_WM_STRUT = _ATOM_GET("_NET_WM_STRUT");
   ECORE_X_ATOM_NET_WM_STRUT_PARTIAL = _ATOM_GET("_NET_WM_STRUT_PARTIAL");
   ECORE_X_ATOM_NET_WM_ICON_GEOMETRY = _ATOM_GET("_NET_WM_ICON_GEOMETRY");
   ECORE_X_ATOM_NET_WM_ICON = _ATOM_GET("_NET_WM_ICON");
   ECORE_X_ATOM_NET_WM_PID = _ATOM_GET("_NET_WM_PID");
   ECORE_X_ATOM_NET_WM_HANDLED_ICONS = _ATOM_GET("_NET_WM_HANDLED_ICONS");
   ECORE_X_ATOM_NET_WM_USER_TIME = _ATOM_GET("_NET_WM_USER_TIME");

   ECORE_X_ATOM_NET_WM_ALLOWED_ACTIONS = _ATOM_GET("_NET_WM_ALLOWED_ACTIONS");
   ECORE_X_ATOM_NET_WM_ACTION_MOVE = _ATOM_GET("_NET_WM_ACTION_MOVE");
   ECORE_X_ATOM_NET_WM_ACTION_RESIZE = _ATOM_GET("_NET_WM_ACTION_RESIZE");
   ECORE_X_ATOM_NET_WM_ACTION_MINIMIZE = _ATOM_GET("_NET_WM_ACTION_MINIMIZE");
   ECORE_X_ATOM_NET_WM_ACTION_SHADE = _ATOM_GET("_NET_WM_ACTION_SHADE");
   ECORE_X_ATOM_NET_WM_ACTION_STICK = _ATOM_GET("_NET_WM_ACTION_STICK");
   ECORE_X_ATOM_NET_WM_ACTION_MAXIMIZE_HORZ = _ATOM_GET("_NET_WM_ACTION_MAXIMIZE_HORZ");
   ECORE_X_ATOM_NET_WM_ACTION_MAXIMIZE_VERT = _ATOM_GET("_NET_WM_ACTION_MAXIMIZE_VERT");
   ECORE_X_ATOM_NET_WM_ACTION_FULLSCREEN = _ATOM_GET("_NET_WM_ACTION_FULLSCREEN");
   ECORE_X_ATOM_NET_WM_ACTION_CHANGE_DESKTOP = _ATOM_GET("_NET_WM_ACTION_CHANGE_DESKTOP");
   ECORE_X_ATOM_NET_WM_ACTION_CLOSE = _ATOM_GET("_NET_WM_ACTION_CLOSE");

   ECORE_X_ATOM_NET_WM_WINDOW_TYPE = _ATOM_GET("_NET_WM_WINDOW_TYPE");
   ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DESKTOP =
      _ATOM_GET("_NET_WM_WINDOW_TYPE_DESKTOP");
   ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DOCK = _ATOM_GET("_NET_WM_WINDOW_TYPE_DOCK");
   ECORE_X_ATOM_NET_WM_WINDOW_TYPE_TOOLBAR =
      _ATOM_GET("_NET_WM_WINDOW_TYPE_TOOLBAR");
   ECORE_X_ATOM_NET_WM_WINDOW_TYPE_MENU = _ATOM_GET("_NET_WM_WINDOW_TYPE_MENU");
   ECORE_X_ATOM_NET_WM_WINDOW_TYPE_UTILITY =
      _ATOM_GET("_NET_WM_WINDOW_TYPE_UTILITY");
   ECORE_X_ATOM_NET_WM_WINDOW_TYPE_SPLASH =
      _ATOM_GET("_NET_WM_WINDOW_TYPE_SPLASH");
   ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DIALOG =
      _ATOM_GET("_NET_WM_WINDOW_TYPE_DIALOG");
   ECORE_X_ATOM_NET_WM_WINDOW_TYPE_NORMAL =
      _ATOM_GET("_NET_WM_WINDOW_TYPE_NORMAL");

   ECORE_X_ATOM_NET_WM_STATE = _ATOM_GET("_NET_WM_STATE");
   ECORE_X_ATOM_NET_WM_STATE_MODAL = _ATOM_GET("_NET_WM_STATE_MODAL");
   ECORE_X_ATOM_NET_WM_STATE_STICKY = _ATOM_GET("_NET_WM_STATE_STICKY");
   ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_VERT =
      _ATOM_GET("_NET_WM_STATE_MAXIMIZED_VERT");
   ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_HORZ =
      _ATOM_GET("_NET_WM_STATE_MAXIMIZED_HORZ");
   ECORE_X_ATOM_NET_WM_STATE_SHADED = _ATOM_GET("_NET_WM_STATE_SHADED");
   ECORE_X_ATOM_NET_WM_STATE_SKIP_TASKBAR =
      _ATOM_GET("_NET_WM_STATE_SKIP_TASKBAR");
   ECORE_X_ATOM_NET_WM_STATE_SKIP_PAGER = _ATOM_GET("_NET_WM_STATE_SKIP_PAGER");
   ECORE_X_ATOM_NET_WM_STATE_HIDDEN = _ATOM_GET("_NET_WM_STATE_HIDDEN");
   ECORE_X_ATOM_NET_WM_STATE_FULLSCREEN = _ATOM_GET("_NET_WM_STATE_FULLSCREEN");
   ECORE_X_ATOM_NET_WM_STATE_ABOVE = _ATOM_GET("_NET_WM_STATE_ABOVE");
   ECORE_X_ATOM_NET_WM_STATE_BELOW = _ATOM_GET("_NET_WM_STATE_BELOW");
   ECORE_X_ATOM_NET_WM_STATE_DEMANDS_ATTENTION = _ATOM_GET("_NET_WM_STATE_DEMANDS_ATTENTION");

   ECORE_X_ATOM_NET_WM_WINDOW_OPACITY = _ATOM_GET("_NET_WM_WINDOW_OPACITY");

   ECORE_X_ATOM_NET_FRAME_EXTENTS = _ATOM_GET("_NET_FRAME_EXTENTS");
   ECORE_X_ATOM_NET_REQUEST_FRAME_EXTENTS = _ATOM_GET("_NET_REQUEST_FRAME_EXTENTS");

   ECORE_X_ATOM_NET_WM_PING = _ATOM_GET("_NET_WM_PING");
   ECORE_X_ATOM_NET_WM_SYNC_REQUEST = _ATOM_GET("_NET_WM_SYNC_REQUEST");
}

/*
 * WM identification
 */
void
ecore_x_netwm_wm_identify(Ecore_X_Window root, Ecore_X_Window check,
			  const char *wm_name)
{
   _ATOM_SET_WINDOW(root, ECORE_X_ATOM_NET_SUPPORTING_WM_CHECK, &check, 1);
   _ATOM_SET_WINDOW(check, ECORE_X_ATOM_NET_SUPPORTING_WM_CHECK, &check, 1);
   _ATOM_SET_UTF8_STRING(check, ECORE_X_ATOM_NET_WM_NAME, wm_name);
   /* This one isn't mandatory */
   _ATOM_SET_UTF8_STRING(root, ECORE_X_ATOM_NET_WM_NAME, wm_name);
}

/*
 * Set supported atoms
 */
void
ecore_x_netwm_supported(Ecore_X_Window root, Ecore_X_Atom atom, int supported)
{
   Ecore_X_Atom      *oldset = NULL, *newset = NULL;
   int               i, j = 0, num = 0;
   unsigned char     *data = NULL;
   unsigned char     *old_data = NULL;

   ecore_x_window_prop_property_get(root, ECORE_X_ATOM_NET_SUPPORTED,
                                    XA_ATOM, 32, &old_data, &num);
   oldset = (Ecore_X_Atom *)old_data;

   if (supported)
     {
	for (i = 0; i < num; ++i)
	  {
	     if (oldset[i] == atom)
	       goto done;
	  }

	newset = calloc(num + 1, sizeof(Ecore_X_Atom));
	if (!newset)
	  goto done;

	data = (unsigned char *) newset;
	for (i = 0; i < num; i++)
	  newset[i] = oldset[i];
	newset[num] = atom;

	ecore_x_window_prop_property_set(root, ECORE_X_ATOM_NET_SUPPORTED,
					 XA_ATOM, 32, data, num + 1);
     }
   else
     {
	int has;

	has = 0;
	for (i = 0; i < num; ++i)
	  {
	     if (oldset[i] == atom)
	       has = 1;
	  }
	if (!has)
	  goto done;

	newset = calloc(num - 1, sizeof(Ecore_X_Atom));
	if (!newset)
	  goto done;

	data = (unsigned char *) newset;
	for (i = 0; i < num; i++)
	  if (oldset[i] != atom)
	    newset[j++] = oldset[i];

	ecore_x_window_prop_property_set(root, ECORE_X_ATOM_NET_SUPPORTED,
					 XA_ATOM, 32, data, num - 1);
     }
   free(newset);
done:
   XFree(oldset);
}


/*
 * Desktop configuration and status
 */
void
ecore_x_netwm_desk_count_set(Ecore_X_Window root, unsigned int n_desks)
{
   ecore_x_window_prop_card32_set(root, ECORE_X_ATOM_NET_NUMBER_OF_DESKTOPS,
				  &n_desks, 1);
}

void
ecore_x_netwm_desk_roots_set(Ecore_X_Window root, unsigned int n_desks,
			     Ecore_X_Window * vroots)
{
   _ATOM_SET_WINDOW(root, ECORE_X_ATOM_NET_VIRTUAL_ROOTS, vroots, n_desks);
}

void
ecore_x_netwm_desk_names_set(Ecore_X_Window root, unsigned int n_desks,
			     const char **names)
{
   char                ss[32], *buf;
   const char         *s;
   unsigned int        i;
   int                 l, len;

   buf = NULL;
   len = 0;

   for (i = 0; i < n_desks; i++)
     {
	s = (names) ? names[i] : NULL;
	if (!s)
	  {
	     /* Default to "Desk-<number>" */
	     sprintf(ss, "Desk-%d", i);
	     s = ss;
	  }

	l = strlen(s) + 1;
	buf = realloc(buf, len + l);
	memcpy(buf + len, s, l);
	len += l;
     }

   _ATOM_SET_UTF8_STRING_LIST(root, ECORE_X_ATOM_NET_DESKTOP_NAMES, buf, len);

   free(buf);
}

void
ecore_x_netwm_desk_size_set(Ecore_X_Window root, unsigned int width,
			    unsigned int height)
{
   unsigned int        size[2];

   size[0] = width;
   size[1] = height;
   ecore_x_window_prop_card32_set(root, ECORE_X_ATOM_NET_DESKTOP_GEOMETRY, size,
				  2);
}

void
ecore_x_netwm_desk_viewports_set(Ecore_X_Window root, unsigned int n_desks,
				 unsigned int *origins)
{
   ecore_x_window_prop_card32_set(root, ECORE_X_ATOM_NET_DESKTOP_VIEWPORT,
				  origins, 2 * n_desks);
}

void
ecore_x_netwm_desk_layout_set(Ecore_X_Window root, int orientation,
			      int columns, int rows,
			      int starting_corner)
{
   unsigned int layout[4];

   layout[0] = orientation;
   layout[1] = columns;
   layout[2] = rows;
   layout[3] = starting_corner;
   ecore_x_window_prop_card32_set(root, ECORE_X_ATOM_NET_DESKTOP_LAYOUT,
				  layout, 4);
}

void
ecore_x_netwm_desk_workareas_set(Ecore_X_Window root, unsigned int n_desks,
				 unsigned int *areas)
{
   ecore_x_window_prop_card32_set(root, ECORE_X_ATOM_NET_WORKAREA, areas,
				  4 * n_desks);
}

void
ecore_x_netwm_desk_current_set(Ecore_X_Window root, unsigned int desk)
{
   ecore_x_window_prop_card32_set(root, ECORE_X_ATOM_NET_CURRENT_DESKTOP, &desk,
				  1);
}

void
ecore_x_netwm_showing_desktop_set(Ecore_X_Window root, int on)
{
   unsigned int        val;

   val = (on) ? 1 : 0;
   ecore_x_window_prop_card32_set(root, ECORE_X_ATOM_NET_SHOWING_DESKTOP, &val,
				  1);
}

/*
 * Client status
 */

/* Mapping order */
void
ecore_x_netwm_client_list_set(Ecore_X_Window root, unsigned int n_clients,
			      Ecore_X_Window * p_clients)
{
   _ATOM_SET_WINDOW(root, ECORE_X_ATOM_NET_CLIENT_LIST, p_clients, n_clients);
}

/* Stacking order */
void
ecore_x_netwm_client_list_stacking_set(Ecore_X_Window root,
				       unsigned int n_clients,
				       Ecore_X_Window * p_clients)
{
   _ATOM_SET_WINDOW(root, ECORE_X_ATOM_NET_CLIENT_LIST_STACKING, p_clients,
		    n_clients);
}

void
ecore_x_netwm_client_active_set(Ecore_X_Window root, Ecore_X_Window win)
{
   _ATOM_SET_WINDOW(root, ECORE_X_ATOM_NET_ACTIVE_WINDOW, &win, 1);
}

void
ecore_x_netwm_name_set(Ecore_X_Window win, const char *name)
{
   _ecore_x_window_prop_string_utf8_set(win, ECORE_X_ATOM_NET_WM_NAME, name);
}

char *
ecore_x_netwm_name_get(Ecore_X_Window win)
{
   return _ecore_x_window_prop_string_utf8_get(win, ECORE_X_ATOM_NET_WM_NAME);
}

void
ecore_x_netwm_visible_name_set(Ecore_X_Window win, const char *name)
{
   _ecore_x_window_prop_string_utf8_set(win, ECORE_X_ATOM_NET_WM_VISIBLE_NAME,
					name);
}

char *
ecore_x_netwm_visible_name_get(Ecore_X_Window win)
{
   return _ecore_x_window_prop_string_utf8_get(win,
					       ECORE_X_ATOM_NET_WM_VISIBLE_NAME);
}

void
ecore_x_netwm_icon_name_set(Ecore_X_Window win, const char *name)
{
   _ecore_x_window_prop_string_utf8_set(win, ECORE_X_ATOM_NET_WM_ICON_NAME,
					name);
}

char *
ecore_x_netwm_icon_name_get(Ecore_X_Window win)
{
   return _ecore_x_window_prop_string_utf8_get(win,
					       ECORE_X_ATOM_NET_WM_ICON_NAME);
}

void
ecore_x_netwm_visible_icon_name_set(Ecore_X_Window win, const char *name)
{
   _ecore_x_window_prop_string_utf8_set(win,
					ECORE_X_ATOM_NET_WM_VISIBLE_ICON_NAME,
					name);
}

char *
ecore_x_netwm_visible_icon_name_get(Ecore_X_Window win)
{
   return _ecore_x_window_prop_string_utf8_get(win,
					       ECORE_X_ATOM_NET_WM_VISIBLE_ICON_NAME);
}

void
ecore_x_netwm_desktop_set(Ecore_X_Window win, unsigned int desk)
{
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_NET_WM_DESKTOP, &desk, 1);
}

int
ecore_x_netwm_desktop_get(Ecore_X_Window win, unsigned int *desk)
{
   int ret;
   unsigned int tmp;

   ret = ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_NET_WM_DESKTOP,
					&tmp, 1);

   if (desk) *desk = tmp;
   return ret == 1 ? 1 : 0;
}

/*
 * _NET_WM_STRUT is deprecated
 */
void
ecore_x_netwm_strut_set(Ecore_X_Window win, int left, int right,
			int top, int bottom)
{
   unsigned int strut[4];

   strut[0] = left;
   strut[1] = right;
   strut[2] = top;
   strut[3] = bottom;
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_NET_WM_STRUT, strut, 4);
}

/*
 * _NET_WM_STRUT is deprecated
 */
int
ecore_x_netwm_strut_get(Ecore_X_Window win, int *left, int *right,
			int *top, int *bottom)
{
   int ret = 0;
   unsigned int strut[4];

   ret = ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_NET_WM_STRUT, strut, 4);
   if (ret != 4)
     return 0;

   if (left) *left = strut[0];
   if (right) *right = strut[1];
   if (top) *top = strut[2];
   if (bottom) *bottom = strut[3];
   return 1;
}

void
ecore_x_netwm_strut_partial_set(Ecore_X_Window win, int left, int right,
				int top, int bottom, int left_start_y, int left_end_y,
			       	int right_start_y, int right_end_y, int top_start_x,
			       	int top_end_x, int bottom_start_x, int bottom_end_x)
{
   unsigned int strut[12];

   strut[0] = left;
   strut[1] = right;
   strut[2] = top;
   strut[3] = bottom;
   strut[4] = left_start_y;
   strut[5] = left_end_y;
   strut[6] = right_start_y;
   strut[7] = right_end_y;
   strut[8] = top_start_x;
   strut[9] = top_end_x;
   strut[10] = bottom_start_x;
   strut[11] = bottom_end_x;
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_NET_WM_STRUT_PARTIAL, strut, 12);
}

int
ecore_x_netwm_strut_partial_get(Ecore_X_Window win, int *left, int *right,
				int *top, int *bottom, int *left_start_y, int *left_end_y,
			       	int *right_start_y, int *right_end_y, int *top_start_x,
			       	int *top_end_x, int *bottom_start_x, int *bottom_end_x)
{
   int ret = 0;
   unsigned int strut[12];

   ret = ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_NET_WM_STRUT_PARTIAL, strut, 12);
   if (ret != 12)
     return 0;

   if (left) *left = strut[0];
   if (right) *right = strut[1];
   if (top) *top = strut[2];
   if (bottom) *bottom = strut[3];
   if (left_start_y) *left_start_y = strut[4];
   if (left_end_y) *left_end_y = strut[5];
   if (right_start_y) *right_start_y = strut[6];
   if (right_end_y) *right_end_y = strut[7];
   if (top_start_x) *top_start_x = strut[8];
   if (top_end_x) *top_end_x = strut[9];
   if (bottom_start_x) *bottom_start_x = strut[10];
   if (bottom_end_x) *bottom_end_x = strut[11];
   return 1;
}

int
ecore_x_netwm_icon_get(Ecore_X_Window win, int *width, int *height, unsigned int **data, int *num)
{
   unsigned char *data_ret;
   unsigned int  *icon;
   unsigned int  *src;
   int            num_ret, len;

   if (width) *width = 0;
   if (height) *height = 0;
   if (num) *num = 0;

   if (!ecore_x_window_prop_property_get(win, ECORE_X_ATOM_NET_WM_ICON,
					 XA_CARDINAL, 32, &data_ret, &num_ret))
     return 0;
   icon = (unsigned int *)data_ret;

   if (data)
     {
	*data = malloc((num_ret - 2) * sizeof(unsigned int));
	if (!(*data)) return 0;
     }

   if (num) *num = (num_ret - 2);
   if (width) *width = icon[0];
   if (height) *height = icon[1];

   len = icon[0] * icon[1];
   src = &(icon[2]);
   if (data)
     memcpy(*data, &(icon[2]), len * sizeof(unsigned int));
   free(data_ret);

   return 1;
}

void
ecore_x_netwm_icon_geometry_set(Ecore_X_Window win, int x, int y, int width, int height)
{
   unsigned int geometry[4];

   geometry[0] = x;
   geometry[1] = y;
   geometry[2] = width;
   geometry[3] = height;
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_NET_WM_ICON_GEOMETRY, geometry, 4);
}

int
ecore_x_netwm_icon_geometry_get(Ecore_X_Window win, int *x, int *y, int *width, int *height)
{
   int ret;
   unsigned int geometry[4];

   ret = ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_NET_WM_ICON_GEOMETRY, geometry, 4);
   if (ret != 4)
     return 0;

   if (x) *x = geometry[0];
   if (y) *y = geometry[1];
   if (width) *width = geometry[2];
   if (height) *height = geometry[3];
   return 1;
}

void
ecore_x_netwm_pid_set(Ecore_X_Window win, int pid)
{
   unsigned int tmp;

   tmp = pid;
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_NET_WM_PID,
				  &tmp, 1);
}

int
ecore_x_netwm_pid_get(Ecore_X_Window win, int *pid)
{
   int ret;
   unsigned int tmp;

   ret = ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_NET_WM_PID,
					&tmp, 1);
   if (pid) *pid = tmp;
   return ret == 1 ? 1 : 0;
}

void
ecore_x_netwm_handled_icons_set(Ecore_X_Window win)
{
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_NET_WM_HANDLED_ICONS,
				  NULL, 0);
}

int
ecore_x_netwm_handled_icons_get(Ecore_X_Window win)
{
   int ret = 0;
   ret = ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_NET_WM_HANDLED_ICONS,
					NULL, 0);
   return ret == 0 ? 1 : 0;
}

void
ecore_x_netwm_user_time_set(Ecore_X_Window win, unsigned int time)
{
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_NET_WM_USER_TIME,
				  &time, 1);
}

int
ecore_x_netwm_user_time_get(Ecore_X_Window win, unsigned int *time)
{
   int ret;
   unsigned int tmp;

   ret = ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_NET_WM_USER_TIME,
					&tmp, 1);
   if (time) *time = tmp;
   return ret == 1 ? 1 : 0;
}

Ecore_X_Window_State
_ecore_x_netwm_state_get(Ecore_X_Atom a)
{
   if (a == ECORE_X_ATOM_NET_WM_STATE_MODAL)
     return ECORE_X_WINDOW_STATE_MODAL;
   else if (a == ECORE_X_ATOM_NET_WM_STATE_STICKY)
     return ECORE_X_WINDOW_STATE_STICKY;
   else if (a == ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_VERT)
     return ECORE_X_WINDOW_STATE_MAXIMIZED_VERT;
   else if (a == ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_HORZ)
     return ECORE_X_WINDOW_STATE_MAXIMIZED_HORZ;
   else if (a == ECORE_X_ATOM_NET_WM_STATE_SHADED)
     return ECORE_X_WINDOW_STATE_SHADED;
   else if (a == ECORE_X_ATOM_NET_WM_STATE_SKIP_TASKBAR)
     return ECORE_X_WINDOW_STATE_SKIP_TASKBAR;
   else if (a == ECORE_X_ATOM_NET_WM_STATE_SKIP_PAGER)
     return ECORE_X_WINDOW_STATE_SKIP_PAGER;
   else if (a == ECORE_X_ATOM_NET_WM_STATE_HIDDEN)
     return ECORE_X_WINDOW_STATE_HIDDEN;
   else if (a == ECORE_X_ATOM_NET_WM_STATE_FULLSCREEN)
     return ECORE_X_WINDOW_STATE_FULLSCREEN;
   else if (a == ECORE_X_ATOM_NET_WM_STATE_ABOVE)
     return ECORE_X_WINDOW_STATE_ABOVE;
   else if (a == ECORE_X_ATOM_NET_WM_STATE_BELOW)
     return ECORE_X_WINDOW_STATE_BELOW;
   else if (a == ECORE_X_ATOM_NET_WM_STATE_DEMANDS_ATTENTION)
     return ECORE_X_WINDOW_STATE_DEMANDS_ATTENTION;
   else
     return ECORE_X_WINDOW_STATE_UNKNOWN;
}

static Ecore_X_Atom
_ecore_x_netwm_state_atom_get(Ecore_X_Window_State s)
{
   switch(s)
     {
      case ECORE_X_WINDOW_STATE_MODAL:
	 return ECORE_X_ATOM_NET_WM_STATE_MODAL;
      case ECORE_X_WINDOW_STATE_STICKY:
	 return ECORE_X_ATOM_NET_WM_STATE_STICKY;
      case ECORE_X_WINDOW_STATE_MAXIMIZED_VERT:
	 return ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_VERT;
      case ECORE_X_WINDOW_STATE_MAXIMIZED_HORZ:
	 return ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_HORZ;
      case ECORE_X_WINDOW_STATE_SHADED:
	 return ECORE_X_ATOM_NET_WM_STATE_SHADED;
      case ECORE_X_WINDOW_STATE_SKIP_TASKBAR:
	 return ECORE_X_ATOM_NET_WM_STATE_SKIP_TASKBAR;
      case ECORE_X_WINDOW_STATE_SKIP_PAGER:
	 return ECORE_X_ATOM_NET_WM_STATE_SKIP_PAGER;
      case ECORE_X_WINDOW_STATE_HIDDEN:
	 return ECORE_X_ATOM_NET_WM_STATE_HIDDEN;
      case ECORE_X_WINDOW_STATE_FULLSCREEN:
	 return ECORE_X_ATOM_NET_WM_STATE_FULLSCREEN;
      case ECORE_X_WINDOW_STATE_ABOVE:
	 return ECORE_X_ATOM_NET_WM_STATE_ABOVE;
      case ECORE_X_WINDOW_STATE_BELOW:
	 return ECORE_X_ATOM_NET_WM_STATE_BELOW;
      case ECORE_X_WINDOW_STATE_DEMANDS_ATTENTION:
	 return ECORE_X_ATOM_NET_WM_STATE_DEMANDS_ATTENTION;
      default:
	 return 0;
     }
}

Ecore_X_Window_State *
ecore_x_netwm_window_state_list_get(Ecore_X_Window win, int *num)
{
   int                   num_ret, i;
   unsigned char        *data;
   Ecore_X_Atom         *atoms;
   Ecore_X_Window_State *state;

   if (num) *num = 0;

   if (!ecore_x_window_prop_property_get(win, ECORE_X_ATOM_NET_WM_STATE,
					 XA_ATOM, 32, &data, &num_ret))
      return NULL;

   if ((!data) || (!num_ret)) return NULL;

   atoms = (Ecore_X_Atom *) data;
   state = malloc(num_ret * sizeof(Ecore_X_Window_State));
   if (state)
     {
	for (i = 0; i < num_ret; ++i)
	  state[i] = _ecore_x_netwm_state_get(atoms[i]);

	if (num) *num = num_ret;
     }

   XFree(data);
   return state;
}

int
ecore_x_netwm_window_state_list_set(Ecore_X_Window win, Ecore_X_Window_State *state, int num)
{
   unsigned char *data;
   Ecore_X_Atom  *set;
   int            i;

   if (num == 0)
     {
	XDeleteProperty(_ecore_x_disp, win, ECORE_X_ATOM_NET_WM_STATE);
	return 1;
     }

   data = malloc(num * sizeof(Ecore_X_Atom));
   if (!data) return 1;

   set = (Ecore_X_Window_State *) data;
   for (i = 0; i < num; i++)
     set[i] = _ecore_x_netwm_state_atom_get(state[i]);

   ecore_x_window_prop_property_set(win, ECORE_X_ATOM_NET_WM_STATE,
				    XA_ATOM, 32, data, num);
   free(data);
   return 1;
}

int
ecore_x_netwm_window_state_isset(Ecore_X_Window win, Ecore_X_Window_State s)
{
   int            num, i, ret = 0;
   unsigned char  *data;
   Ecore_X_Atom   *atoms, atom;

   if (!ecore_x_window_prop_property_get(win, ECORE_X_ATOM_NET_WM_STATE,
					 XA_ATOM, 32, &data, &num))
      return ret;

   atom = _ecore_x_netwm_state_atom_get(s);
   atoms = (Ecore_X_Atom *) data;

   for (i = 0; i < num; ++i)
     {
	if (atoms[i] == atom)
	  {
	     ret = 1;
	     break;
	  }
     }

   XFree(data);
   return ret;
}

void
ecore_x_netwm_window_state_set(Ecore_X_Window win, Ecore_X_Window_State state, int on)
{
   Ecore_X_Atom      atom;
   Ecore_X_Atom      *oldset = NULL, *newset = NULL;
   int               i, j = 0, num = 0;
   unsigned char     *data = NULL;
   unsigned char     *old_data = NULL;

   atom = _ecore_x_netwm_state_atom_get(state);

   ecore_x_window_prop_property_get(win, ECORE_X_ATOM_NET_WM_STATE,
                                    XA_ATOM, 32, &old_data, &num);
   oldset = (Ecore_X_Atom *) old_data;

   if (on)
     {
	for (i = 0; i < num; ++i)
	  {
	     if (oldset[i] == atom)
	       goto done;
	  }

	newset = calloc(num + 1, sizeof(Ecore_X_Atom));
	if (!newset)
	  goto done;

	data = (unsigned char *) newset;

	for (i = 0; i < num; i++)
	  newset[i] = oldset[i];
	newset[num] = atom;

	ecore_x_window_prop_property_set(win, ECORE_X_ATOM_NET_WM_STATE,
					 XA_ATOM, 32, data, num + 1);
     }
   else
     {
	int has;

	has = 0;
	for (i = 0; i < num; ++i)
	  {
	     if (oldset[i] == atom)
	       has = 1;
	  }
	if (!has)
	  goto done;

	newset = calloc(num - 1, sizeof(Ecore_X_Atom));
	if (!newset)
	  goto done;

	data = (unsigned char *) newset;
	for (i = 0; i < num; i++)
	  if (oldset[i] != atom)
	    newset[j++] = oldset[i];

	ecore_x_window_prop_property_set(win, ECORE_X_ATOM_NET_WM_STATE,
					 XA_ATOM, 32, data, num - 1);
     }
   free(newset);
done:
   XFree(oldset);
}

static Ecore_X_Window_Type
_ecore_x_netwm_window_type_type_get(Ecore_X_Atom atom)
{
   if (atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DESKTOP)
     return ECORE_X_WINDOW_TYPE_DESKTOP;
   else if (atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DOCK)
     return ECORE_X_WINDOW_TYPE_DOCK;
   else if (atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE_TOOLBAR)
     return ECORE_X_WINDOW_TYPE_TOOLBAR;
   else if (atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE_MENU)
     return ECORE_X_WINDOW_TYPE_MENU;
   else if (atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE_UTILITY)
     return ECORE_X_WINDOW_TYPE_UTILITY;
   else if (atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE_SPLASH)
     return ECORE_X_WINDOW_TYPE_SPLASH;
   else if (atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DIALOG)
     return ECORE_X_WINDOW_TYPE_DIALOG;
   else if (atom == ECORE_X_ATOM_NET_WM_WINDOW_TYPE_NORMAL)
     return ECORE_X_WINDOW_TYPE_NORMAL;
   else
     return 0;
}

static Ecore_X_Atom 
_ecore_x_netwm_window_type_atom_get(Ecore_X_Window_Type type)
{
   switch (type)
     {
      case ECORE_X_WINDOW_TYPE_DESKTOP:
	 return ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DESKTOP;
      case ECORE_X_WINDOW_TYPE_DOCK:
	 return ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DOCK;
      case ECORE_X_WINDOW_TYPE_TOOLBAR:
	 return ECORE_X_ATOM_NET_WM_WINDOW_TYPE_TOOLBAR;
      case ECORE_X_WINDOW_TYPE_MENU:
	 return ECORE_X_ATOM_NET_WM_WINDOW_TYPE_MENU;
      case ECORE_X_WINDOW_TYPE_UTILITY:
	 return ECORE_X_ATOM_NET_WM_WINDOW_TYPE_UTILITY;
      case ECORE_X_WINDOW_TYPE_SPLASH:
	 return ECORE_X_ATOM_NET_WM_WINDOW_TYPE_SPLASH;
      case ECORE_X_WINDOW_TYPE_DIALOG:
	 return ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DIALOG;
      case ECORE_X_WINDOW_TYPE_NORMAL:
	 return ECORE_X_ATOM_NET_WM_WINDOW_TYPE_NORMAL;
      default:
	 return 0;
     }
}

/*
 * FIXME: We should set WM_TRANSIENT_FOR if type is ECORE_X_WINDOW_TYPE_TOOLBAR
 * , ECORE_X_WINDOW_TYPE_MENU or ECORE_X_WINDOW_TYPE_DIALOG
 */
void
ecore_x_netwm_window_type_set(Ecore_X_Window win, Ecore_X_Window_Type type)
{
   Ecore_X_Atom atom;

   atom = _ecore_x_netwm_window_type_atom_get(type);
   ecore_x_window_prop_property_set(win, ECORE_X_ATOM_NET_WM_WINDOW_TYPE,
				    XA_ATOM, 32, (unsigned char *)&atom, 1);
}

Ecore_X_Window_Type
ecore_x_netwm_window_type_get(Ecore_X_Window win)
{
   int                  num, i;
   Ecore_X_Window_Type  ret = ECORE_X_WINDOW_TYPE_NORMAL, type;
   unsigned char       *data;
   Ecore_X_Atom        *atoms;

   if (!ecore_x_window_prop_property_get(win, ECORE_X_ATOM_NET_WM_WINDOW_TYPE,
					 XA_ATOM, 32, &data, &num))
     {
	/* Check if WM_TRANSIENT_FOR is set */
	if (ecore_x_icccm_transient_for_get(win))
	  ret = ECORE_X_WINDOW_TYPE_DIALOG;
	return ret;
     }

   atoms = (Ecore_X_Atom *) data;

   /*
    * FIXME: Why does nautilus and gkrellm report NORMAL as the
    * first atom, and not the last?
   for (i = 0; i < num; ++i)
     {
	char *s;
	s = XGetAtomName(_ecore_x_disp, atoms[i]);
	printf("window type: %s\n", s);
	XFree(s);
     }
   */
   for (i = 0; i < num; ++i)
     {
	type = _ecore_x_netwm_window_type_type_get(atoms[i]);
	if (type)
	  {
	     ret = type;
	     break;
	  }
     }

   XFree(data);
   return ret;
}

static Ecore_X_Atom
_ecore_x_netwm_action_atom_get(Ecore_X_Action action)
{
   switch (action)
     {
      case ECORE_X_ACTION_MOVE:
	 return ECORE_X_ATOM_NET_WM_ACTION_MOVE;
      case ECORE_X_ACTION_RESIZE:
	 return ECORE_X_ATOM_NET_WM_ACTION_RESIZE;
      case ECORE_X_ACTION_MINIMIZE:
	 return ECORE_X_ATOM_NET_WM_ACTION_MINIMIZE;
      case ECORE_X_ACTION_SHADE:
	 return ECORE_X_ATOM_NET_WM_ACTION_SHADE;
      case ECORE_X_ACTION_STICK:
	 return ECORE_X_ATOM_NET_WM_ACTION_STICK;
      case ECORE_X_ACTION_MAXIMIZE_HORZ:
	 return ECORE_X_ATOM_NET_WM_ACTION_MAXIMIZE_HORZ;
      case ECORE_X_ACTION_MAXIMIZE_VERT:
	 return ECORE_X_ATOM_NET_WM_ACTION_MAXIMIZE_VERT;
      case ECORE_X_ACTION_FULLSCREEN:
	 return ECORE_X_ATOM_NET_WM_ACTION_FULLSCREEN;
      case ECORE_X_ACTION_CHANGE_DESKTOP:
	 return ECORE_X_ATOM_NET_WM_ACTION_CHANGE_DESKTOP;
      case ECORE_X_ACTION_CLOSE:
	 return ECORE_X_ATOM_NET_WM_ACTION_CLOSE;
      default:
	 return 0;
     }
}

int
ecore_x_netwm_allowed_action_isset(Ecore_X_Window win, Ecore_X_Action action)
{
   int                  num, i, ret = 0;
   unsigned char       *data;
   Ecore_X_Atom        *atoms, atom;

   if (!ecore_x_window_prop_property_get(win, ECORE_X_ATOM_NET_WM_WINDOW_TYPE,
					 XA_ATOM, 32, &data, &num))
     return ret;

   atom = _ecore_x_netwm_action_atom_get(action);
   atoms = (Ecore_X_Atom *) data;

   for (i = 0; i < num; ++i)
     {
	if (atom == atoms[i])
	  {
	     ret = 1;
	     break;
	  }
     }

   XFree(data);
   return ret;
}

void
ecore_x_netwm_allowed_action_set(Ecore_X_Window win, Ecore_X_Action action, int on)
{
   Ecore_X_Atom      atom;
   Ecore_X_Atom      *oldset = NULL, *newset = NULL;
   int               i, j = 0, num = 0;
   unsigned char     *data = NULL;
   unsigned char     *old_data = NULL;

   atom = _ecore_x_netwm_action_atom_get(action);

   ecore_x_window_prop_property_get(win, ECORE_X_ATOM_NET_WM_ALLOWED_ACTIONS,
                                    XA_ATOM, 32, &old_data, &num);
   oldset = (Ecore_X_Atom *) old_data;

   if (on)
     {
	for (i = 0; i < num; ++i)
	  {
	     if (oldset[i] == atom)
	       goto done;
	  }

	newset = calloc(num + 1, sizeof(Ecore_X_Atom));
	if (!newset)
	  goto done;

	data = (unsigned char *) newset;
	for (i = 0; i < num; i++)
	  newset[i] = oldset[i];
	newset[num] = atom;

	ecore_x_window_prop_property_set(win, ECORE_X_ATOM_NET_WM_ALLOWED_ACTIONS,
					 XA_ATOM, 32, data, num + 1);
     }
   else
     {
	int has;

	has = 0;
	for (i = 0; i < num; ++i)
	  {
	     if (oldset[i] == atom)
	       has = 1;
	  }
	if (!has)
	  goto done;

	newset = calloc(num - 1, sizeof(Ecore_X_Atom));
	if (!newset)
	  goto done;

	data = (unsigned char *) newset;
	for (i = 0; i < num; i++)
	  if (oldset[i] != atom)
	    newset[j++] = oldset[i];

	ecore_x_window_prop_property_set(win, ECORE_X_ATOM_NET_WM_ALLOWED_ACTIONS,
					 XA_ATOM, 32, data, num - 1);
     }
   free(newset);
done:
   XFree(oldset);
}

void
ecore_x_netwm_opacity_set(Ecore_X_Window win, unsigned int opacity)
{
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_NET_WM_WINDOW_OPACITY,
				  &opacity, 1);
}

int
ecore_x_netwm_opacity_get(Ecore_X_Window win, unsigned int *opacity)
{
   int ret;
   unsigned int tmp;

   ret = ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_NET_WM_WINDOW_OPACITY,
					&tmp, 1);
   if (opacity) *opacity = tmp;
   return ret == 1 ? 1 : 0;
}

void
ecore_x_netwm_frame_size_set(Ecore_X_Window win, int fl, int fr, int ft, int fb)
{
   unsigned int frames[4];

   frames[0] = fl;
   frames[1] = fr;
   frames[2] = ft;
   frames[3] = fb;
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_NET_FRAME_EXTENTS, frames, 4);
}

int
ecore_x_netwm_frame_size_get(Ecore_X_Window win, int *fl, int *fr, int *ft, int *fb)
{
   int ret = 0;
   unsigned int frames[4];

   ret = ecore_x_window_prop_card32_get(win, ECORE_X_ATOM_NET_FRAME_EXTENTS, frames, 4);
   if (ret != 4)
     return 0;

   if (fl) *fl = frames[0];
   if (fr) *fr = frames[1];
   if (ft) *ft = frames[2];
   if (fb) *fb = frames[3];
   return 1;
}

