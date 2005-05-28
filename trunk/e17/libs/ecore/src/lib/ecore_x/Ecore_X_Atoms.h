/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifndef _ECORE_X_ATOMS_H
#define _ECORE_X_ATOMS_H

#include "Ecore_X.h"

/**
 * @file
 * @brief Ecore X atoms
 */

/* General */
extern Ecore_X_Atom ECORE_X_ATOM_UTF8_STRING;
extern Ecore_X_Atom ECORE_X_ATOM_FILE_NAME;
extern Ecore_X_Atom ECORE_X_ATOM_STRING;
extern Ecore_X_Atom ECORE_X_ATOM_TEXT;
extern Ecore_X_Atom ECORE_X_ATOM_COMPOUND_TEXT;

/* ICCCM */
extern Ecore_X_Atom ECORE_X_ATOM_WM_STATE;
extern Ecore_X_Atom ECORE_X_ATOM_WM_DELETE_WINDOW;
extern Ecore_X_Atom ECORE_X_ATOM_WM_TAKE_FOCUS;
extern Ecore_X_Atom ECORE_X_ATOM_WM_PROTOCOLS;
extern Ecore_X_Atom ECORE_X_ATOM_WM_CLASS;
extern Ecore_X_Atom ECORE_X_ATOM_WM_NAME;
extern Ecore_X_Atom ECORE_X_ATOM_WM_COMMAND;
extern Ecore_X_Atom ECORE_X_ATOM_WM_ICON_NAME;
extern Ecore_X_Atom ECORE_X_ATOM_WM_CLIENT_MACHINE;
extern Ecore_X_Atom ECORE_X_ATOM_WM_CHANGE_STATE;
extern Ecore_X_Atom ECORE_X_ATOM_WM_COLORMAP_WINDOWS;
extern Ecore_X_Atom ECORE_X_ATOM_WM_WINDOW_ROLE;
extern Ecore_X_Atom ECORE_X_ATOM_WM_HINTS;
extern Ecore_X_Atom ECORE_X_ATOM_WM_NORMAL_HINTS;
extern Ecore_X_Atom ECORE_X_ATOM_WM_CLIENT_LEADER;
extern Ecore_X_Atom ECORE_X_ATOM_WM_TRANSIENT_FOR;
extern Ecore_X_Atom ECORE_X_ATOM_WM_SAVE_YOURSELF;

/* MWM */
extern Ecore_X_Atom ECORE_X_ATOM_MOTIF_WM_HINTS;

/* GNOME */
extern Ecore_X_Atom ECORE_X_ATOM_WIN_LAYER;

/* EWMH */
extern Ecore_X_Atom ECORE_X_ATOM_NET_SUPPORTED;
extern Ecore_X_Atom ECORE_X_ATOM_NET_SUPPORTING_WM_CHECK;

extern Ecore_X_Atom ECORE_X_ATOM_NET_NUMBER_OF_DESKTOPS;
extern Ecore_X_Atom ECORE_X_ATOM_NET_VIRTUAL_ROOTS;
extern Ecore_X_Atom ECORE_X_ATOM_NET_DESKTOP_NAMES;
extern Ecore_X_Atom ECORE_X_ATOM_NET_DESKTOP_GEOMETRY;
extern Ecore_X_Atom ECORE_X_ATOM_NET_DESKTOP_VIEWPORT;
extern Ecore_X_Atom ECORE_X_ATOM_NET_DESKTOP_LAYOUT;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WORKAREA;

extern Ecore_X_Atom ECORE_X_ATOM_NET_CURRENT_DESKTOP;
extern Ecore_X_Atom ECORE_X_ATOM_NET_SHOWING_DESKTOP;

extern Ecore_X_Atom ECORE_X_ATOM_NET_CLIENT_LIST;
extern Ecore_X_Atom ECORE_X_ATOM_NET_CLIENT_LIST_STACKING;
extern Ecore_X_Atom ECORE_X_ATOM_NET_ACTIVE_WINDOW;

extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_NAME;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_VISIBLE_NAME;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_ICON_NAME;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_VISIBLE_ICON_NAME;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_DESKTOP;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_STRUT;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_STRUT_PARTIAL;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_ICON_GEOMETRY;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_ICON;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_PID;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_HANDLED_ICONS;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_USER_TIME;

extern Ecore_X_Atom ECORE_X_ATOM_NET_CLOSE_WINDOW;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_MOVERESIZE;

extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_ALLOWED_ACTIONS;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_ACTION_MOVE;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_ACTION_RESIZE;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_ACTION_MINIMIZE;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_ACTION_SHADE;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_ACTION_STICK;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_ACTION_MAXIMIZE_HORZ;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_ACTION_MAXIMIZE_VERT;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_ACTION_FULLSCREEN;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_ACTION_CHANGE_DESKTOP;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_ACTION_CLOSE; /*x*/

extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_WINDOW_TYPE;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DESKTOP;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DOCK;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_WINDOW_TYPE_TOOLBAR;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_WINDOW_TYPE_MENU;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_WINDOW_TYPE_UTILITY;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_WINDOW_TYPE_SPLASH;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_WINDOW_TYPE_DIALOG;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_WINDOW_TYPE_NORMAL;

extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_STATE;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_STATE_MODAL;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_STATE_STICKY;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_VERT;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_STATE_MAXIMIZED_HORZ;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_STATE_SHADED;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_STATE_SKIP_TASKBAR;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_STATE_SKIP_PAGER;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_STATE_HIDDEN;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_STATE_FULLSCREEN;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_STATE_ABOVE;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_STATE_BELOW;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_STATE_DEMANDS_ATTENTION;

extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_WINDOW_OPACITY;

extern Ecore_X_Atom ECORE_X_ATOM_NET_FRAME_EXTENTS;
extern Ecore_X_Atom ECORE_X_ATOM_NET_REQUEST_FRAME_EXTENTS;

extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_PING;
extern Ecore_X_Atom ECORE_X_ATOM_NET_WM_SYNC_REQUEST;

/* Selections */
extern Ecore_X_Atom ECORE_X_ATOM_SELECTION_TARGETS;
extern Ecore_X_Atom ECORE_X_ATOM_SELECTION_PRIMARY;
extern Ecore_X_Atom ECORE_X_ATOM_SELECTION_SECONDARY;
extern Ecore_X_Atom ECORE_X_ATOM_SELECTION_CLIPBOARD;
extern Ecore_X_Atom ECORE_X_ATOM_SELECTION_PROP_PRIMARY;
extern Ecore_X_Atom ECORE_X_ATOM_SELECTION_PROP_SECONDARY;
extern Ecore_X_Atom ECORE_X_ATOM_SELECTION_PROP_CLIPBOARD;

/* DND */
extern Ecore_X_Atom ECORE_X_ATOM_SELECTION_XDND;
extern Ecore_X_Atom ECORE_X_ATOM_SELECTION_PROP_XDND;
extern Ecore_X_Atom ECORE_X_ATOM_XDND_AWARE;
extern Ecore_X_Atom ECORE_X_ATOM_XDND_TYPE_LIST;
extern Ecore_X_Atom ECORE_X_ATOM_XDND_ACTION_COPY;
extern Ecore_X_Atom ECORE_X_ATOM_XDND_ACTION_PRIVATE;
extern Ecore_X_Atom ECORE_X_ATOM_XDND_ACTION_ASK;
extern Ecore_X_Atom ECORE_X_ATOM_XDND_ACTION_LIST;
extern Ecore_X_Atom ECORE_X_ATOM_XDND_ACTION_DESCRIPTION;
extern Ecore_X_Atom ECORE_X_ATOM_XDND_ENTER;
extern Ecore_X_Atom ECORE_X_ATOM_XDND_LEAVE;
extern Ecore_X_Atom ECORE_X_ATOM_XDND_STATUS;
extern Ecore_X_Atom ECORE_X_ATOM_XDND_POSITION;
extern Ecore_X_Atom ECORE_X_ATOM_XDND_DROP;
extern Ecore_X_Atom ECORE_X_ATOM_XDND_FINISHED;

#endif /* _ECORE_X_ATOMS_H */
