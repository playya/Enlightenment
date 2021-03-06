#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#elif defined __GNUC__
# define alloca __builtin_alloca
#elif defined _AIX
# define alloca __alloca
#elif defined _MSC_VER
# include <malloc.h>
# define alloca _alloca
#else /* ifdef HAVE_ALLOCA_H */
# include <stddef.h>
# ifdef  __cplusplus
extern "C"
# endif /* ifdef  __cplusplus */
void *    alloca (size_t);
#endif /* ifdef HAVE_ALLOCA_H */

#include <string.h>

#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"
#include "Ecore_X_Atoms.h"

#include "ecore_x_atoms_decl.h"

typedef struct
{
   const char   *name;
   Ecore_X_Atom *atom;
} Atom_Item;

void
_ecore_x_atoms_init(void)
{
   const Atom_Item items[] =
   {
      { "ATOM", &ECORE_X_ATOM_ATOM },
      { "CARDINAL", &ECORE_X_ATOM_CARDINAL },
      { "COMPOUND_TEXT", &ECORE_X_ATOM_COMPOUND_TEXT },
      { "FILE_NAME", &ECORE_X_ATOM_FILE_NAME },
      { "STRING", &ECORE_X_ATOM_STRING },
      { "TEXT", &ECORE_X_ATOM_TEXT },
      { "UTF8_STRING", &ECORE_X_ATOM_UTF8_STRING },
      { "WINDOW", &ECORE_X_ATOM_WINDOW },
      { "PIXMAP", &ECORE_X_ATOM_PIXMAP },

      { "JXSelectionWindowProperty", &ECORE_X_ATOM_SELECTION_PROP_XDND },
      { "XdndSelection", &ECORE_X_ATOM_SELECTION_XDND },
      { "XdndAware", &ECORE_X_ATOM_XDND_AWARE },
      { "XdndEnter", &ECORE_X_ATOM_XDND_ENTER },
      { "XdndTypeList", &ECORE_X_ATOM_XDND_TYPE_LIST },
      { "XdndPosition", &ECORE_X_ATOM_XDND_POSITION },
      { "XdndActionCopy", &ECORE_X_ATOM_XDND_ACTION_COPY },
      { "XdndActionMove", &ECORE_X_ATOM_XDND_ACTION_MOVE },
      { "XdndActionPrivate", &ECORE_X_ATOM_XDND_ACTION_PRIVATE },
      { "XdndActionAsk", &ECORE_X_ATOM_XDND_ACTION_ASK },
      { "XdndActionList", &ECORE_X_ATOM_XDND_ACTION_LIST },
      { "XdndActionLink", &ECORE_X_ATOM_XDND_ACTION_LINK },
      { "XdndActionDescription", &ECORE_X_ATOM_XDND_ACTION_DESCRIPTION },
      { "XdndProxy", &ECORE_X_ATOM_XDND_PROXY },
      { "XdndStatus", &ECORE_X_ATOM_XDND_STATUS },
      { "XdndLeave", &ECORE_X_ATOM_XDND_LEAVE },
      { "XdndDrop", &ECORE_X_ATOM_XDND_DROP },
      { "XdndFinished", &ECORE_X_ATOM_XDND_FINISHED },

      { "XdndActionCopy", &ECORE_X_DND_ACTION_COPY },
      { "XdndActionMove", &ECORE_X_DND_ACTION_MOVE },
      { "XdndActionLink", &ECORE_X_DND_ACTION_LINK },
      { "XdndActionAsk", &ECORE_X_DND_ACTION_ASK },
      { "XdndActionPrivate", &ECORE_X_DND_ACTION_PRIVATE },

      { "_E_FRAME_SIZE", &ECORE_X_ATOM_E_FRAME_SIZE },

      { "_WIN_LAYER", &ECORE_X_ATOM_WIN_LAYER },

      { "WM_NAME", &ECORE_X_ATOM_WM_NAME },
      { "WM_ICON_NAME", &ECORE_X_ATOM_WM_ICON_NAME },
      { "WM_NORMAL_HINTS", &ECORE_X_ATOM_WM_NORMAL_HINTS },
      { "WM_SIZE_HINTS", &ECORE_X_ATOM_WM_SIZE_HINTS },
      { "WM_HINTS", &ECORE_X_ATOM_WM_HINTS },
      { "WM_CLASS", &ECORE_X_ATOM_WM_CLASS },
      { "WM_TRANSIENT_FOR", &ECORE_X_ATOM_WM_TRANSIENT_FOR },
      { "WM_PROTOCOLS", &ECORE_X_ATOM_WM_PROTOCOLS },
      { "WM_COLORMAP_WINDOWS", &ECORE_X_ATOM_WM_COLORMAP_WINDOWS },
      { "WM_COMMAND", &ECORE_X_ATOM_WM_COMMAND },
      { "WM_CLIENT_MACHINE", &ECORE_X_ATOM_WM_CLIENT_MACHINE },

      { "WM_STATE", &ECORE_X_ATOM_WM_STATE },
      { "WM_ICON_SIZE", &ECORE_X_ATOM_WM_ICON_SIZE },

      { "WM_CHANGE_STATE", &ECORE_X_ATOM_WM_CHANGE_STATE },

      { "WM_TAKE_FOCUS", &ECORE_X_ATOM_WM_TAKE_FOCUS },
      { "WM_SAVE_YOURSELF", &ECORE_X_ATOM_WM_SAVE_YOURSELF },
      { "WM_DELETE_WINDOW", &ECORE_X_ATOM_WM_DELETE_WINDOW },

      { "WM_COLORMAP_NOTIFY", &ECORE_X_ATOM_WM_COLORMAP_NOTIFY },

      { "SM_CLIENT_ID", &ECORE_X_ATOM_SM_CLIENT_ID },
      { "WM_CLIENT_LEADER", &ECORE_X_ATOM_WM_CLIENT_LEADER },
      { "WM_WINDOW_ROLE", &ECORE_X_ATOM_WM_WINDOW_ROLE },

      { "_MOTIF_WM_HINTS", &ECORE_X_ATOM_MOTIF_WM_HINTS },

      { "_NET_SUPPORTED", &ECORE_X_ATOM_NET_SUPPORTED },
      { "_NET_CLIENT_LIST", &ECORE_X_ATOM_NET_CLIENT_LIST },
      { "_NET_CLIENT_LIST_STACKING", &ECORE_X_ATOM_NET_CLIENT_LIST_STACKING },
      { "_NET_NUMBER_OF_DESKTOPS", &ECORE_X_ATOM_NET_NUMBER_OF_DESKTOPS },
      { "_NET_DESKTOP_GEOMETRY", &ECORE_X_ATOM_NET_DESKTOP_GEOMETRY },
      { "_NET_DESKTOP_VIEWPORT", &ECORE_X_ATOM_NET_DESKTOP_VIEWPORT },
      { "_NET_CURRENT_DESKTOP", &ECORE_X_ATOM_NET_CURRENT_DESKTOP },
      { "_NET_DESKTOP_NAMES", &ECORE_X_ATOM_NET_DESKTOP_NAMES },
      { "_NET_ACTIVE_WINDOW", &ECORE_X_ATOM_NET_ACTIVE_WINDOW },
      { "_NET_WORKAREA", &ECORE_X_ATOM_NET_WORKAREA },
      { "_NET_SUPPORTING_WM_CHECK", &ECORE_X_ATOM_NET_SUPPORTING_WM_CHECK },
      { "_NET_VIRTUAL_ROOTS", &ECORE_X_ATOM_NET_VIRTUAL_ROOTS },
      { "_NET_DESKTOP_LAYOUT", &ECORE_X_ATOM_NET_DESKTOP_LAYOUT },
      { "_NET_SHOWING_DESKTOP", &ECORE_X_ATOM_NET_SHOWING_DESKTOP },

      { "_NET_CLOSE_WINDOW", &ECORE_X_ATOM_NET_CLOSE_WINDOW },
      { "_NET_MOVERESIZE_WINDOW", &ECORE_X_ATOM_NET_MOVERESIZE_WINDOW },
      { "_NET_WM_MOVERESIZE", &ECORE_X_ATOM_NET_WM_MOVERESIZE },
      { "_NET_RESTACK_WINDOW", &ECORE_X_ATOM_NET_RESTACK_WINDOW },

      { "_NET_REQUEST_FRAME_EXTENTS", &ECORE_X_ATOM_NET_REQUEST_FRAME_EXTENTS },

      { "_NET_WM_NAME", &ECORE_X_ATOM_NET_WM_NAME },
      { "_NET_WM_VISIBLE_NAME", &ECORE_X_ATOM_NET_WM_VISIBLE_NAME },
      { "_NET_WM_ICON_NAME", &ECORE_X_ATOM_NET_WM_ICON_NAME },
      { "_NET_WM_VISIBLE_ICON_NAME", &ECORE_X_ATOM_NET_WM_VISIBLE_ICON_NAME },
      { "_NET_WM_DESKTOP", &ECORE_X_ATOM_NET_WM_DESKTOP },

      { "_NET_WM_WINDOW_TYPE", &ECORE_X_ATOM_NET_WM_WINDOW_TYPE },
      { "_NET_WM_WINDOW_TYPE_DESKTOP", &ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DESKTOP },
      { "_NET_WM_WINDOW_TYPE_DOCK", &ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DOCK },
      { "_NET_WM_WINDOW_TYPE_TOOLBAR", &ECORE_X_ATOM_NET_WM_WINDOW_TYPE_TOOLBAR },
      { "_NET_WM_WINDOW_TYPE_MENU", &ECORE_X_ATOM_NET_WM_WINDOW_TYPE_MENU },
      { "_NET_WM_WINDOW_TYPE_UTILITY", &ECORE_X_ATOM_NET_WM_WINDOW_TYPE_UTILITY },
      { "_NET_WM_WINDOW_TYPE_SPLASH", &ECORE_X_ATOM_NET_WM_WINDOW_TYPE_SPLASH },
      { "_NET_WM_WINDOW_TYPE_DIALOG", &ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DIALOG },
      { "_NET_WM_WINDOW_TYPE_NORMAL", &ECORE_X_ATOM_NET_WM_WINDOW_TYPE_NORMAL },
      { "_NET_WM_WINDOW_TYPE_DROPDOWN_MENU",
        &ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DROPDOWN_MENU },
      { "_NET_WM_WINDOW_TYPE_POPUP_MENU",
        &ECORE_X_ATOM_NET_WM_WINDOW_TYPE_POPUP_MENU },
      { "_NET_WM_WINDOW_TYPE_TOOLTIP", &ECORE_X_ATOM_NET_WM_WINDOW_TYPE_TOOLTIP },
      { "_NET_WM_WINDOW_TYPE_NOTIFICATION",
        &ECORE_X_ATOM_NET_WM_WINDOW_TYPE_NOTIFICATION },
      { "_NET_WM_WINDOW_TYPE_COMBO", &ECORE_X_ATOM_NET_WM_WINDOW_TYPE_COMBO },
      { "_NET_WM_WINDOW_TYPE_DND", &ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DND },

      { "_NET_WM_STATE", &ECORE_X_ATOM_NET_WM_STATE },
      { "_NET_WM_STATE_MODAL", &ECORE_X_ATOM_NET_WM_STATE_MODAL },
      { "_NET_WM_STATE_STICKY", &ECORE_X_ATOM_NET_WM_STATE_STICKY },
      { "_NET_WM_STATE_MAXIMIZED_VERT",
        &ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_VERT },
      { "_NET_WM_STATE_MAXIMIZED_HORZ",
        &ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_HORZ },
      { "_NET_WM_STATE_SHADED", &ECORE_X_ATOM_NET_WM_STATE_SHADED },
      { "_NET_WM_STATE_SKIP_TASKBAR", &ECORE_X_ATOM_NET_WM_STATE_SKIP_TASKBAR },
      { "_NET_WM_STATE_SKIP_PAGER", &ECORE_X_ATOM_NET_WM_STATE_SKIP_PAGER },
      { "_NET_WM_STATE_HIDDEN", &ECORE_X_ATOM_NET_WM_STATE_HIDDEN },
      { "_NET_WM_STATE_FULLSCREEN", &ECORE_X_ATOM_NET_WM_STATE_FULLSCREEN },
      { "_NET_WM_STATE_ABOVE", &ECORE_X_ATOM_NET_WM_STATE_ABOVE },
      { "_NET_WM_STATE_BELOW", &ECORE_X_ATOM_NET_WM_STATE_BELOW },
      { "_NET_WM_STATE_DEMANDS_ATTENTION",
        &ECORE_X_ATOM_NET_WM_STATE_DEMANDS_ATTENTION },

      { "_NET_WM_ALLOWED_ACTIONS", &ECORE_X_ATOM_NET_WM_ALLOWED_ACTIONS },
      { "_NET_WM_ACTION_MOVE", &ECORE_X_ATOM_NET_WM_ACTION_MOVE },
      { "_NET_WM_ACTION_RESIZE", &ECORE_X_ATOM_NET_WM_ACTION_RESIZE },
      { "_NET_WM_ACTION_MINIMIZE", &ECORE_X_ATOM_NET_WM_ACTION_MINIMIZE },
      { "_NET_WM_ACTION_SHADE", &ECORE_X_ATOM_NET_WM_ACTION_SHADE },
      { "_NET_WM_ACTION_STICK", &ECORE_X_ATOM_NET_WM_ACTION_STICK },
      { "_NET_WM_ACTION_MAXIMIZE_HORZ",
        &ECORE_X_ATOM_NET_WM_ACTION_MAXIMIZE_HORZ },
      { "_NET_WM_ACTION_MAXIMIZE_VERT",
        &ECORE_X_ATOM_NET_WM_ACTION_MAXIMIZE_VERT },
      { "_NET_WM_ACTION_FULLSCREEN", &ECORE_X_ATOM_NET_WM_ACTION_FULLSCREEN },
      { "_NET_WM_ACTION_CHANGE_DESKTOP",
        &ECORE_X_ATOM_NET_WM_ACTION_CHANGE_DESKTOP },
      { "_NET_WM_ACTION_CLOSE", &ECORE_X_ATOM_NET_WM_ACTION_CLOSE },
      { "_NET_WM_ACTION_ABOVE", &ECORE_X_ATOM_NET_WM_ACTION_ABOVE },
      { "_NET_WM_ACTION_BELOW", &ECORE_X_ATOM_NET_WM_ACTION_BELOW },

      { "_NET_WM_STRUT", &ECORE_X_ATOM_NET_WM_STRUT },
      { "_NET_WM_STRUT_PARTIAL", &ECORE_X_ATOM_NET_WM_STRUT_PARTIAL },
      { "_NET_WM_ICON_GEOMETRY", &ECORE_X_ATOM_NET_WM_ICON_GEOMETRY },
      { "_NET_WM_ICON", &ECORE_X_ATOM_NET_WM_ICON },
      { "_NET_WM_PID", &ECORE_X_ATOM_NET_WM_PID },
      { "_NET_WM_HANDLED_ICONS", &ECORE_X_ATOM_NET_WM_HANDLED_ICONS },
      { "_NET_WM_USER_TIME", &ECORE_X_ATOM_NET_WM_USER_TIME },
      { "_NET_STARTUP_ID", &ECORE_X_ATOM_NET_STARTUP_ID },
      { "_NET_FRAME_EXTENTS", &ECORE_X_ATOM_NET_FRAME_EXTENTS },

      { "_NET_WM_PING", &ECORE_X_ATOM_NET_WM_PING },
      { "_NET_WM_SYNC_REQUEST", &ECORE_X_ATOM_NET_WM_SYNC_REQUEST },
      { "_NET_WM_SYNC_REQUEST_COUNTER",
        &ECORE_X_ATOM_NET_WM_SYNC_REQUEST_COUNTER },

      { "_NET_WM_WINDOW_OPACITY", &ECORE_X_ATOM_NET_WM_WINDOW_OPACITY },
      { "_NET_WM_WINDOW_SHADOW", &ECORE_X_ATOM_NET_WM_WINDOW_SHADOW },
      { "_NET_WM_WINDOW_SHADE", &ECORE_X_ATOM_NET_WM_WINDOW_SHADE },

      { "TARGETS", &ECORE_X_ATOM_SELECTION_TARGETS },
      { "CLIPBOARD", &ECORE_X_ATOM_SELECTION_CLIPBOARD },
      { "PRIMARY", &ECORE_X_ATOM_SELECTION_PRIMARY },
      { "SECONDARY", &ECORE_X_ATOM_SELECTION_SECONDARY },
      { "_ECORE_SELECTION_PRIMARY", &ECORE_X_ATOM_SELECTION_PROP_PRIMARY },
      { "_ECORE_SELECTION_SECONDARY", &ECORE_X_ATOM_SELECTION_PROP_SECONDARY },
      { "_ECORE_SELECTION_CLIPBOARD", &ECORE_X_ATOM_SELECTION_PROP_CLIPBOARD },

      { "_E_VIRTUAL_KEYBOARD", &ECORE_X_ATOM_E_VIRTUAL_KEYBOARD },
      { "_E_VIRTUAL_KEYBOARD_STATE", &ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_STATE },
      { "_E_VIRTUAL_KEYBOARD_ON", &ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_ON },
      { "_E_VIRTUAL_KEYBOARD_OFF", &ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_OFF },
      { "_E_VIRTUAL_KEYBOARD_ALPHA", &ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_ALPHA },
      { "_E_VIRTUAL_KEYBOARD_NUMERIC", &ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_NUMERIC },
      { "_E_VIRTUAL_KEYBOARD_PIN", &ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_PIN },
      { "_E_VIRTUAL_KEYBOARD_PHONE_NUMBER",
        &ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_PHONE_NUMBER },
      { "_E_VIRTUAL_KEYBOARD_HEX", &ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_HEX },
      { "_E_VIRTUAL_KEYBOARD_TERMINAL",
        &ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_TERMINAL },
      { "_E_VIRTUAL_KEYBOARD_PASSWORD",
        &ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_PASSWORD },
      { "_E_VIRTUAL_KEYBOARD_IP", &ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_IP },
      { "_E_VIRTUAL_KEYBOARD_HOST", &ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_HOST },
      { "_E_VIRTUAL_KEYBOARD_FILE", &ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_FILE },
      { "_E_VIRTUAL_KEYBOARD_URL", &ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_URL },
      { "_E_VIRTUAL_KEYBOARD_KEYPAD", &ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_KEYPAD },
      { "_E_VIRTUAL_KEYBOARD_J2ME", &ECORE_X_ATOM_E_VIRTUAL_KEYBOARD_J2ME },

      { "_E_ILLUME_ZONE", &ECORE_X_ATOM_E_ILLUME_ZONE },
      { "_E_ILLUME_ZONE_LIST", &ECORE_X_ATOM_E_ILLUME_ZONE_LIST },
      { "_E_ILLUME_CONFORMANT", &ECORE_X_ATOM_E_ILLUME_CONFORMANT },
      { "_E_ILLUME_MODE", &ECORE_X_ATOM_E_ILLUME_MODE },
      { "_E_ILLUME_MODE_SINGLE", &ECORE_X_ATOM_E_ILLUME_MODE_SINGLE },
      { "_E_ILLUME_MODE_DUAL_TOP", &ECORE_X_ATOM_E_ILLUME_MODE_DUAL_TOP },
      { "_E_ILLUME_MODE_DUAL_LEFT", &ECORE_X_ATOM_E_ILLUME_MODE_DUAL_LEFT },
      { "_E_ILLUME_FOCUS_BACK", &ECORE_X_ATOM_E_ILLUME_FOCUS_BACK },
      { "_E_ILLUME_FOCUS_FORWARD", &ECORE_X_ATOM_E_ILLUME_FOCUS_FORWARD },
      { "_E_ILLUME_FOCUS_HOME", &ECORE_X_ATOM_E_ILLUME_FOCUS_HOME },
      { "_E_ILLUME_CLOSE", &ECORE_X_ATOM_E_ILLUME_CLOSE },
      { "_E_ILLUME_HOME_NEW", &ECORE_X_ATOM_E_ILLUME_HOME_NEW },
      { "_E_ILLUME_HOME_DEL", &ECORE_X_ATOM_E_ILLUME_HOME_DEL },
      { "_E_ILLUME_DRAG", &ECORE_X_ATOM_E_ILLUME_DRAG },
      { "_E_ILLUME_DRAG_LOCKED", &ECORE_X_ATOM_E_ILLUME_DRAG_LOCKED },
      { "_E_ILLUME_DRAG_START", &ECORE_X_ATOM_E_ILLUME_DRAG_START },
      { "_E_ILLUME_DRAG_END", &ECORE_X_ATOM_E_ILLUME_DRAG_END },
      { "_E_ILLUME_INDICATOR_GEOMETRY",
        &ECORE_X_ATOM_E_ILLUME_INDICATOR_GEOMETRY },
      { "_E_ILLUME_SOFTKEY_GEOMETRY", &ECORE_X_ATOM_E_ILLUME_SOFTKEY_GEOMETRY },
      { "_E_ILLUME_KEYBOARD_GEOMETRY", &ECORE_X_ATOM_E_ILLUME_KEYBOARD_GEOMETRY },
      { "_E_ILLUME_QUICKPANEL", &ECORE_X_ATOM_E_ILLUME_QUICKPANEL },
      { "_E_ILLUME_QUICKPANEL_STATE", &ECORE_X_ATOM_E_ILLUME_QUICKPANEL_STATE },
      { "_E_ILLUME_QUICKPANEL_STATE_TOGGLE",
        &ECORE_X_ATOM_E_ILLUME_QUICKPANEL_STATE_TOGGLE },
      { "_E_ILLUME_QUICKPANEL_ON", &ECORE_X_ATOM_E_ILLUME_QUICKPANEL_ON },
      { "_E_ILLUME_QUICKPANEL_OFF", &ECORE_X_ATOM_E_ILLUME_QUICKPANEL_OFF },
      { "_E_ILLUME_QUICKPANEL_PRIORITY_MAJOR",
        &ECORE_X_ATOM_E_ILLUME_QUICKPANEL_PRIORITY_MAJOR },
      { "_E_ILLUME_QUICKPANEL_PRIORITY_MINOR",
        &ECORE_X_ATOM_E_ILLUME_QUICKPANEL_PRIORITY_MINOR },
      { "_E_ILLUME_QUICKPANEL_ZONE", &ECORE_X_ATOM_E_ILLUME_QUICKPANEL_ZONE },
      { "_E_ILLUME_QUICKPANEL_POSITION_UPDATE",
        &ECORE_X_ATOM_E_ILLUME_QUICKPANEL_POSITION_UPDATE },

      { "_E_COMP_SYNC_COUNTER", &ECORE_X_ATOM_E_COMP_SYNC_COUNTER },
      { "_E_COMP_SYNC_DRAW_DONE", &ECORE_X_ATOM_E_COMP_SYNC_DRAW_DONE },
      { "_E_COMP_SYNC_SUPPORTED", &ECORE_X_ATOM_E_COMP_SYNC_SUPPORTED },
      { "_E_COMP_SYNC_BEGIN", &ECORE_X_ATOM_E_COMP_SYNC_BEGIN },
      { "_E_COMP_SYNC_END", &ECORE_X_ATOM_E_COMP_SYNC_END },
      { "_E_COMP_SYNC_CANCEL", &ECORE_X_ATOM_E_COMP_SYNC_CANCEL },

      { "_E_COMP_FLUSH", &ECORE_X_ATOM_E_COMP_FLUSH },
      { "_E_COMP_DUMP", &ECORE_X_ATOM_E_COMP_DUMP },
      { "_E_COMP_PIXMAP", &ECORE_X_ATOM_E_COMP_PIXMAP }
   };
   Atom *atoms;
   char **names;
   int i, num;

   num = sizeof(items) / sizeof(Atom_Item);
   atoms = alloca(num * sizeof(Atom));
   names = alloca(num * sizeof(char *));
   for (i = 0; i < num; i++) names[i] = (char *)items[i].name;
   XInternAtoms(_ecore_x_disp, names, num, False, atoms);
   for (i = 0; i < num; i++) *(items[i].atom) = atoms[i];
} /* _ecore_x_atoms_init */

/**
 * Retrieves the atom value associated with the given name.
 * @param  name The given name.
 * @return Associated atom value.
 */
EAPI Ecore_X_Atom
ecore_x_atom_get(const char *name)
{
   if (!_ecore_x_disp)
      return 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   return XInternAtom(_ecore_x_disp, name, False);
} /* ecore_x_atom_get */

EAPI void
ecore_x_atoms_get(const char **names, int num, Ecore_X_Atom *atoms)
{
   Atom *atoms_int;
   int i;

   if (!_ecore_x_disp)
      return;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   atoms_int = alloca(num * sizeof(Atom));
   XInternAtoms(_ecore_x_disp, (char **)names, num, False, atoms_int);
   for (i = 0; i < num; i++)
      atoms[i] = atoms_int[i];
} /* ecore_x_atoms_get */

EAPI char *
ecore_x_atom_name_get(Ecore_X_Atom atom)
{
   char *name;
   char *xname;

   if (!_ecore_x_disp)
      return NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   xname = XGetAtomName(_ecore_x_disp, atom);
   if (!xname)
      return NULL;

   name = strdup(xname);
   XFree(xname);

   return name;
} /* ecore_x_atom_name_get */

