#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"

static int _ecore_x_fd_handler(void *data, Ecore_Fd_Handler *fd_handler);
static int _ecore_x_fd_handler_buf(void *data, Ecore_Fd_Handler *fd_handler);
static int _ecore_x_key_mask_get(KeySym sym);
static void *_ecore_x_event_filter_start(void *data);
static int   _ecore_x_event_filter_filter(void *data, void *loop_data,int type, void *event);
static void  _ecore_x_event_filter_end(void *data, void *loop_data);

static Ecore_Fd_Handler *_ecore_x_fd_handler_handle = NULL;
static Ecore_Event_Filter *_ecore_x_filter_handler = NULL;
static int _ecore_x_event_shape_id = 0;
static int _ecore_x_event_handlers_num = 0;
static void (**_ecore_x_event_handlers) (XEvent * event) = NULL;

static int _ecore_x_init_count = 0;
static int _ecore_x_grab_count = 0;

Display *_ecore_x_disp = NULL;
double   _ecore_x_double_click_time = 0.25;
Time     _ecore_x_event_last_time = 0;
Window   _ecore_x_event_last_win = 0;
int      _ecore_x_event_last_root_x = 0;
int      _ecore_x_event_last_root_y = 0;

/*
 * ICCCM and Motif hints.
 */
Atom     _ecore_x_atom_wm_state = 0;
Atom     _ecore_x_atom_wm_delete_window = 0;
Atom     _ecore_x_atom_wm_take_focus = 0;
Atom     _ecore_x_atom_wm_protocols = 0;
Atom     _ecore_x_atom_wm_class = 0;
Atom     _ecore_x_atom_wm_name = 0;
Atom     _ecore_x_atom_wm_command = 0;
Atom     _ecore_x_atom_wm_icon_name = 0;
Atom     _ecore_x_atom_wm_client_machine = 0;
Atom     _ecore_x_atom_wm_change_state = 0;

Atom     _ecore_x_atom_motif_wm_hints = 0;

Atom     _ecore_x_atom_win_layer = 0;

Atom     _ecore_x_atom_selection_primary = 0;
Atom     _ecore_x_atom_selection_secondary = 0;
Atom     _ecore_x_atom_selection_clipboard = 0;
Atom     _ecore_x_atom_selection_prop_primary = 0;
Atom     _ecore_x_atom_selection_prop_secondary = 0;
Atom     _ecore_x_atom_selection_prop_clipboard = 0;

Atom     _ecore_x_atom_selection_xdnd = 0;
Atom     _ecore_x_atom_selection_prop_xdnd = 0;
Atom     _ecore_x_atom_xdnd_aware = 0;
Atom     _ecore_x_atom_xdnd_enter = 0;
Atom     _ecore_x_atom_xdnd_type_list = 0;
Atom     _ecore_x_atom_xdnd_position = 0;
Atom     _ecore_x_atom_xdnd_action_copy = 0;
Atom     _ecore_x_atom_xdnd_action_move = 0;
Atom     _ecore_x_atom_xdnd_action_link = 0;
Atom     _ecore_x_atom_xdnd_action_private = 0;
Atom     _ecore_x_atom_xdnd_action_ask = 0;
Atom     _ecore_x_atom_xdnd_action_list = 0;
Atom     _ecore_x_atom_xdnd_action_description = 0;
Atom     _ecore_x_atom_xdnd_proxy = 0;
Atom     _ecore_x_atom_xdnd_status = 0;
Atom     _ecore_x_atom_xdnd_drop = 0;
Atom     _ecore_x_atom_xdnd_finished = 0;
Atom     _ecore_x_atom_xdnd_leave = 0;

/*
 * Root window NetWM hints.
 */
Atom     _ecore_x_atom_net_supported = 0;
Atom     _ecore_x_atom_net_client_list = 0;
Atom     _ecore_x_atom_net_client_list_stacking = 0;
Atom     _ecore_x_atom_net_number_of_desktops = 0;
Atom     _ecore_x_atom_net_desktop_geometry = 0;
Atom     _ecore_x_atom_net_desktop_viewport = 0;
Atom     _ecore_x_atom_net_current_desktop = 0;
Atom     _ecore_x_atom_net_desktop_names = 0;
Atom     _ecore_x_atom_net_active_window = 0;
Atom     _ecore_x_atom_net_workarea = 0;
Atom     _ecore_x_atom_net_supporting_wm_check = 0;
Atom     _ecore_x_atom_net_virtual_roots = 0;
Atom     _ecore_x_atom_net_desktop_layout = 0;
Atom     _ecore_x_atom_net_showing_desktop = 0;

/*
 * Client message types.
 */
Atom     _ecore_x_atom_net_close_window = 0;
Atom     _ecore_x_atom_net_wm_moveresize = 0;

/*
 * Application window specific NetWM hints.
 */
Atom     _ecore_x_atom_net_wm_desktop = 0;
Atom     _ecore_x_atom_net_wm_name = 0;
Atom     _ecore_x_atom_net_wm_visible_name = 0;
Atom     _ecore_x_atom_net_wm_icon_name = 0;
Atom     _ecore_x_atom_net_wm_visible_icon_name = 0;
Atom     _ecore_x_atom_net_wm_window_type = 0;
Atom     _ecore_x_atom_net_wm_state = 0;
Atom     _ecore_x_atom_net_wm_allowed_actions = 0;
Atom     _ecore_x_atom_net_wm_strut = 0;
Atom     _ecore_x_atom_net_wm_strut_partial = 0;
Atom     _ecore_x_atom_net_wm_icon_geometry = 0;
Atom     _ecore_x_atom_net_wm_icon = 0;
Atom     _ecore_x_atom_net_wm_pid = 0;
Atom     _ecore_x_atom_net_wm_handle_icons = 0;
Atom     _ecore_x_atom_net_wm_user_time = 0;

Atom     _ecore_x_atom_net_wm_window_type_desktop = 0;
Atom     _ecore_x_atom_net_wm_window_type_dock = 0;
Atom     _ecore_x_atom_net_wm_window_type_toolbar = 0;
Atom     _ecore_x_atom_net_wm_window_type_menu = 0;
Atom     _ecore_x_atom_net_wm_window_type_utility = 0;
Atom     _ecore_x_atom_net_wm_window_type_splash = 0;
Atom     _ecore_x_atom_net_wm_window_type_dialog = 0;
Atom     _ecore_x_atom_net_wm_window_type_normal = 0;

Atom     _ecore_x_atom_net_wm_state_modal = 0;
Atom     _ecore_x_atom_net_wm_state_sticky = 0;
Atom     _ecore_x_atom_net_wm_state_maximized_vert = 0;
Atom     _ecore_x_atom_net_wm_state_maximized_horz = 0;
Atom     _ecore_x_atom_net_wm_state_shaded = 0;
Atom     _ecore_x_atom_net_wm_state_skip_taskbar = 0;
Atom     _ecore_x_atom_net_wm_state_skip_pager = 0;
Atom     _ecore_x_atom_net_wm_state_hidden = 0;
Atom     _ecore_x_atom_net_wm_state_fullscreen = 0;
Atom     _ecore_x_atom_net_wm_state_above = 0;
Atom     _ecore_x_atom_net_wm_state_below = 0;

Atom     _ecore_x_atom_net_wm_window_opacity = 0;

Atom     _ecore_x_atom_file_name = 0;
Atom     _ecore_x_atom_string = 0;
Atom     _ecore_x_atom_text = 0;
Atom     _ecore_x_atom_utf8_string = 0;
Atom     _ecore_x_atom_compound_text = 0;

Atom     _ecore_x_atoms_wm_protocols[ECORE_X_WM_PROTOCOL_NUM];

/* Xdnd atoms that need to be exposed to the application interface */
Ecore_X_Atom  ECORE_X_DND_ACTION_COPY = 0;
Ecore_X_Atom  ECORE_X_DND_ACTION_MOVE = 0;
Ecore_X_Atom  ECORE_X_DND_ACTION_LINK = 0;
Ecore_X_Atom  ECORE_X_DND_ACTION_ASK = 0;
Ecore_X_Atom  ECORE_X_DND_ACTION_PRIVATE = 0;

int ECORE_X_EVENT_KEY_DOWN = 0;
int ECORE_X_EVENT_KEY_UP = 0;
int ECORE_X_EVENT_MOUSE_BUTTON_DOWN = 0;
int ECORE_X_EVENT_MOUSE_BUTTON_UP = 0;
int ECORE_X_EVENT_MOUSE_MOVE = 0;
int ECORE_X_EVENT_MOUSE_IN = 0;
int ECORE_X_EVENT_MOUSE_OUT = 0;
int ECORE_X_EVENT_MOUSE_WHEEL = 0;
int ECORE_X_EVENT_WINDOW_FOCUS_IN = 0;
int ECORE_X_EVENT_WINDOW_FOCUS_OUT = 0;
int ECORE_X_EVENT_WINDOW_KEYMAP = 0;
int ECORE_X_EVENT_WINDOW_DAMAGE = 0;
int ECORE_X_EVENT_WINDOW_VISIBILITY_CHANGE = 0;
int ECORE_X_EVENT_WINDOW_CREATE = 0;
int ECORE_X_EVENT_WINDOW_DESTROY = 0;
int ECORE_X_EVENT_WINDOW_HIDE = 0;
int ECORE_X_EVENT_WINDOW_SHOW = 0;
int ECORE_X_EVENT_WINDOW_SHOW_REQUEST = 0;
int ECORE_X_EVENT_WINDOW_REPARENT = 0;
int ECORE_X_EVENT_WINDOW_CONFIGURE = 0;
int ECORE_X_EVENT_WINDOW_CONFIGURE_REQUEST = 0;
int ECORE_X_EVENT_WINDOW_GRAVITY = 0;
int ECORE_X_EVENT_WINDOW_RESIZE_REQUEST = 0;
int ECORE_X_EVENT_WINDOW_STACK = 0;
int ECORE_X_EVENT_WINDOW_STACK_REQUEST = 0;
int ECORE_X_EVENT_WINDOW_PROPERTY = 0;
int ECORE_X_EVENT_WINDOW_COLORMAP = 0;
int ECORE_X_EVENT_WINDOW_MAPPING = 0;
int ECORE_X_EVENT_SELECTION_CLEAR = 0;
int ECORE_X_EVENT_SELECTION_REQUEST = 0;
int ECORE_X_EVENT_SELECTION_NOTIFY = 0;
int ECORE_X_EVENT_CLIENT_MESSAGE = 0;
int ECORE_X_EVENT_WINDOW_SHAPE = 0;

int ECORE_X_EVENT_WINDOW_DELETE_REQUEST = 0;
int ECORE_X_EVENT_WINDOW_PROP_TITLE_CHANGE = 0;
int ECORE_X_EVENT_WINDOW_PROP_VISIBLE_TITLE_CHANGE = 0;
int ECORE_X_EVENT_WINDOW_PROP_NAME_CLASS_CHANGE = 0;
int ECORE_X_EVENT_WINDOW_PROP_ICON_NAME_CHANGE = 0;
int ECORE_X_EVENT_WINDOW_PROP_VISIBLE_ICON_NAME_CHANGE = 0;
int ECORE_X_EVENT_WINDOW_PROP_CLIENT_MACHINE_CHANGE = 0;
int ECORE_X_EVENT_WINDOW_PROP_PID_CHANGE = 0;
int ECORE_X_EVENT_WINDOW_PROP_DESKTOP_CHANGE = 0;

int ECORE_X_EVENT_XDND_ENTER = 0;
int ECORE_X_EVENT_XDND_POSITION = 0;
int ECORE_X_EVENT_XDND_STATUS = 0;
int ECORE_X_EVENT_XDND_LEAVE = 0;
int ECORE_X_EVENT_XDND_DROP = 0;
int ECORE_X_EVENT_XDND_FINISHED = 0;

int ECORE_X_MODIFIER_SHIFT = 0;
int ECORE_X_MODIFIER_CTRL = 0;
int ECORE_X_MODIFIER_ALT = 0;
int ECORE_X_MODIFIER_WIN = 0;

int ECORE_X_LOCK_SCROLL = 0;
int ECORE_X_LOCK_NUM = 0;
int ECORE_X_LOCK_CAPS = 0;

/**
 * @defgroup Ecore_X_Init_Group X Library Init and Shutdown Functions
 *
 * Functions that start and shut down the Ecore X Library.
 */

/**
 * Initialize the X display connection to the given display.
 *
 * @param   name Display target name.  If @c NULL, the default display is
 *               assumed.
 * @return  The number of times the library has been initialized without
 *          being shut down.  0 is returned if an error occurs.
 * @ingroup Ecore_X_Init_Group
 */
int
ecore_x_init(const char *name)
{
   int shape_base = 0;
   int shape_err_base = 0;
   
   if (_ecore_x_init_count > 0) 
     {
	_ecore_x_init_count++;
	return _ecore_x_init_count;
     }
   _ecore_x_disp = XOpenDisplay((char *)name);
   if (!_ecore_x_disp) return 0;
   _ecore_x_error_handler_init();
   _ecore_x_event_handlers_num = LASTEvent;
   if (XShapeQueryExtension(_ecore_x_disp, &shape_base, &shape_err_base))
     _ecore_x_event_shape_id = shape_base + ShapeNotify;
   if (_ecore_x_event_shape_id >= LASTEvent)
     _ecore_x_event_handlers_num = _ecore_x_event_shape_id + 1;
   _ecore_x_event_handlers = calloc(_ecore_x_event_handlers_num, sizeof(void *));
   if (!_ecore_x_event_handlers)
     {
        XCloseDisplay(_ecore_x_disp);
	_ecore_x_fd_handler_handle = NULL;
	_ecore_x_disp = NULL;
	return 0;	
     }
   _ecore_x_event_handlers[KeyPress]         = _ecore_x_event_handle_key_press;
   _ecore_x_event_handlers[KeyRelease]       = _ecore_x_event_handle_key_release;
   _ecore_x_event_handlers[ButtonPress]      = _ecore_x_event_handle_button_press;
   _ecore_x_event_handlers[ButtonRelease]    = _ecore_x_event_handle_button_release;
   _ecore_x_event_handlers[MotionNotify]     = _ecore_x_event_handle_motion_notify;
   _ecore_x_event_handlers[EnterNotify]      = _ecore_x_event_handle_enter_notify;
   _ecore_x_event_handlers[LeaveNotify]      = _ecore_x_event_handle_leave_notify;
   _ecore_x_event_handlers[FocusIn]          = _ecore_x_event_handle_focus_in;
   _ecore_x_event_handlers[FocusOut]         = _ecore_x_event_handle_focus_out;
   _ecore_x_event_handlers[KeymapNotify]     = _ecore_x_event_handle_keymap_notify;
   _ecore_x_event_handlers[Expose]           = _ecore_x_event_handle_expose;
   _ecore_x_event_handlers[GraphicsExpose]   = _ecore_x_event_handle_graphics_expose;
   _ecore_x_event_handlers[VisibilityNotify] = _ecore_x_event_handle_visibility_notify;
   _ecore_x_event_handlers[CreateNotify]     = _ecore_x_event_handle_create_notify;
   _ecore_x_event_handlers[DestroyNotify]    = _ecore_x_event_handle_destroy_notify;
   _ecore_x_event_handlers[UnmapNotify]      = _ecore_x_event_handle_unmap_notify;
   _ecore_x_event_handlers[MapNotify]        = _ecore_x_event_handle_map_notify;
   _ecore_x_event_handlers[MapRequest]       = _ecore_x_event_handle_map_request;
   _ecore_x_event_handlers[ReparentNotify]   = _ecore_x_event_handle_reparent_notify;
   _ecore_x_event_handlers[ConfigureNotify]  = _ecore_x_event_handle_configure_notify;
   _ecore_x_event_handlers[ConfigureRequest] = _ecore_x_event_handle_configure_request;
   _ecore_x_event_handlers[GravityNotify]    = _ecore_x_event_handle_gravity_notify;
   _ecore_x_event_handlers[ResizeRequest]    = _ecore_x_event_handle_resize_request;
   _ecore_x_event_handlers[CirculateNotify]  = _ecore_x_event_handle_circulate_notify;
   _ecore_x_event_handlers[CirculateRequest] = _ecore_x_event_handle_circulate_request;
   _ecore_x_event_handlers[PropertyNotify]   = _ecore_x_event_handle_property_notify;
   _ecore_x_event_handlers[SelectionClear]   = _ecore_x_event_handle_selection_clear;
   _ecore_x_event_handlers[SelectionRequest] = _ecore_x_event_handle_selection_request;
   _ecore_x_event_handlers[SelectionNotify]  = _ecore_x_event_handle_selection_notify;
   _ecore_x_event_handlers[ColormapNotify]   = _ecore_x_event_handle_colormap_notify;
   _ecore_x_event_handlers[MappingNotify]    = _ecore_x_event_handle_mapping_notify;
   _ecore_x_event_handlers[ClientMessage]    = _ecore_x_event_handle_client_message;
   if (_ecore_x_event_shape_id)
     _ecore_x_event_handlers[_ecore_x_event_shape_id] = _ecore_x_event_handle_shape_change;
   if (!ECORE_X_EVENT_KEY_DOWN)
     {
	ECORE_X_EVENT_KEY_DOWN                 = ecore_event_type_new();
	ECORE_X_EVENT_KEY_UP                   = ecore_event_type_new();
	ECORE_X_EVENT_MOUSE_BUTTON_DOWN        = ecore_event_type_new();
	ECORE_X_EVENT_MOUSE_BUTTON_UP          = ecore_event_type_new();
	ECORE_X_EVENT_MOUSE_MOVE               = ecore_event_type_new();
	ECORE_X_EVENT_MOUSE_IN                 = ecore_event_type_new();
	ECORE_X_EVENT_MOUSE_OUT                = ecore_event_type_new();
	ECORE_X_EVENT_MOUSE_WHEEL              = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_FOCUS_IN          = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_FOCUS_OUT         = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_KEYMAP            = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_DAMAGE            = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_VISIBILITY_CHANGE = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_CREATE            = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_DESTROY           = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_HIDE              = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_SHOW              = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_SHOW_REQUEST      = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_REPARENT          = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_CONFIGURE         = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_CONFIGURE_REQUEST = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_GRAVITY           = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_RESIZE_REQUEST    = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_STACK             = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_STACK_REQUEST     = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_PROPERTY          = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_COLORMAP          = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_MAPPING           = ecore_event_type_new();
	ECORE_X_EVENT_SELECTION_CLEAR          = ecore_event_type_new();
	ECORE_X_EVENT_SELECTION_REQUEST        = ecore_event_type_new();
	ECORE_X_EVENT_SELECTION_NOTIFY         = ecore_event_type_new();
	ECORE_X_EVENT_CLIENT_MESSAGE           = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_SHAPE             = ecore_event_type_new();
	
	ECORE_X_EVENT_WINDOW_DELETE_REQUEST                = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_PROP_TITLE_CHANGE             = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_PROP_VISIBLE_TITLE_CHANGE     = ecore_event_type_new();
        ECORE_X_EVENT_WINDOW_PROP_NAME_CLASS_CHANGE        = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_PROP_ICON_NAME_CHANGE         = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_PROP_VISIBLE_ICON_NAME_CHANGE = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_PROP_CLIENT_MACHINE_CHANGE    = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_PROP_PID_CHANGE               = ecore_event_type_new();
	ECORE_X_EVENT_WINDOW_PROP_DESKTOP_CHANGE               = ecore_event_type_new();

   ECORE_X_EVENT_XDND_ENTER               = ecore_event_type_new();
   ECORE_X_EVENT_XDND_POSITION            = ecore_event_type_new();
   ECORE_X_EVENT_XDND_STATUS              = ecore_event_type_new();
   ECORE_X_EVENT_XDND_LEAVE               = ecore_event_type_new();
   ECORE_X_EVENT_XDND_DROP                = ecore_event_type_new();
   ECORE_X_EVENT_XDND_FINISHED            = ecore_event_type_new();
     }
   
   ECORE_X_MODIFIER_SHIFT = _ecore_x_key_mask_get(XK_Shift_L);
   ECORE_X_MODIFIER_CTRL  = _ecore_x_key_mask_get(XK_Control_L);
   ECORE_X_MODIFIER_ALT   = _ecore_x_key_mask_get(XK_Alt_L);
   ECORE_X_MODIFIER_WIN   = _ecore_x_key_mask_get(XK_Super_L);
   if (!ECORE_X_MODIFIER_WIN) 
     ECORE_X_MODIFIER_WIN = _ecore_x_key_mask_get(XK_Meta_L);   
   
   ECORE_X_LOCK_SCROLL    = _ecore_x_key_mask_get(XK_Scroll_Lock);
   ECORE_X_LOCK_NUM       = _ecore_x_key_mask_get(XK_Num_Lock);
   ECORE_X_LOCK_CAPS      = _ecore_x_key_mask_get(XK_Caps_Lock);
   
   _ecore_x_fd_handler_handle = 
     ecore_main_fd_handler_add(ConnectionNumber(_ecore_x_disp),
			       ECORE_FD_READ,
			       _ecore_x_fd_handler, _ecore_x_disp,
			       _ecore_x_fd_handler_buf, _ecore_x_disp);
   if (!_ecore_x_fd_handler_handle)
     {
	XCloseDisplay(_ecore_x_disp);
	free(_ecore_x_event_handlers);
	_ecore_x_fd_handler_handle = NULL;
	_ecore_x_disp = NULL;
	_ecore_x_event_handlers = NULL;
	return 0;
     }
   _ecore_x_filter_handler = ecore_event_filter_add(_ecore_x_event_filter_start, _ecore_x_event_filter_filter, _ecore_x_event_filter_end, NULL);
   _ecore_x_atom_wm_state                 = XInternAtom(_ecore_x_disp, "WM_STATE", False);
   _ecore_x_atom_wm_delete_window         = XInternAtom(_ecore_x_disp, "WM_DELETE_WINDOW", False);
   _ecore_x_atom_wm_take_focus            = XInternAtom(_ecore_x_disp, "WM_TAKE_FOCUS", False);
   _ecore_x_atom_wm_protocols             = XInternAtom(_ecore_x_disp, "WM_PROTOCOLS", False);
   _ecore_x_atom_wm_class                 = XInternAtom(_ecore_x_disp, "WM_CLASS", False);
   _ecore_x_atom_wm_name                  = XInternAtom(_ecore_x_disp, "WM_NAME", False);
   _ecore_x_atom_wm_command               = XInternAtom(_ecore_x_disp, "WM_COMMAND", False);
   _ecore_x_atom_wm_icon_name             = XInternAtom(_ecore_x_disp, "WM_ICON_NAME", False);
   _ecore_x_atom_wm_client_machine        = XInternAtom(_ecore_x_disp, "WM_CLIENT_MACHINE", False);
   _ecore_x_atom_wm_change_state          = XInternAtom(_ecore_x_disp, "WM_CHANGE_STATE", False);

   _ecore_x_atom_motif_wm_hints           = XInternAtom(_ecore_x_disp, "_MOTIF_WM_HINTS", False);

   _ecore_x_atom_win_layer                = XInternAtom(_ecore_x_disp, "_WIN_LAYER", False);

   /* This is just to be anal about naming conventions */
   _ecore_x_atom_selection_primary        = XA_PRIMARY;
   _ecore_x_atom_selection_secondary      = XA_SECONDARY;
   _ecore_x_atom_selection_clipboard      = XInternAtom(_ecore_x_disp, "CLIPBOARD", False);
   _ecore_x_atom_selection_prop_primary   = XInternAtom(_ecore_x_disp, "_ECORE_SELECTION_PRIMARY", False);
   _ecore_x_atom_selection_prop_secondary = XInternAtom(_ecore_x_disp, "_ECORE_SELECTION_SECONDARY", False);
   _ecore_x_atom_selection_prop_clipboard = XInternAtom(_ecore_x_disp, "_ECORE_SELECTION_CLIPBOARD", False);
   _ecore_x_atom_selection_prop_xdnd      = XInternAtom(_ecore_x_disp, "_ECORE_SELECTION_XDND", False);
   _ecore_x_atom_selection_xdnd           = XInternAtom(_ecore_x_disp, "XdndSelection", False);
   _ecore_x_atom_xdnd_aware               = XInternAtom(_ecore_x_disp, "XdndAware", False);
   _ecore_x_atom_xdnd_type_list           = XInternAtom(_ecore_x_disp, "XdndTypeList", False);
   _ecore_x_atom_xdnd_enter               = XInternAtom(_ecore_x_disp, "XdndEnter", False);
   _ecore_x_atom_xdnd_position            = XInternAtom(_ecore_x_disp, "XdndPosition", False);
   _ecore_x_atom_xdnd_action_copy         = XInternAtom(_ecore_x_disp, "XdndActionCopy", False);
   _ecore_x_atom_xdnd_action_move         = XInternAtom(_ecore_x_disp, "XdndActionMove", False);
   _ecore_x_atom_xdnd_action_private      = XInternAtom(_ecore_x_disp, "XdndActionPrivate", False);
   _ecore_x_atom_xdnd_action_ask          = XInternAtom(_ecore_x_disp, "XdndActionAsk", False);
   _ecore_x_atom_xdnd_action_list         = XInternAtom(_ecore_x_disp, "XdndActionList", False);
   _ecore_x_atom_xdnd_action_link         = XInternAtom(_ecore_x_disp, "XdndActionLink", False);
   _ecore_x_atom_xdnd_action_description  = XInternAtom(_ecore_x_disp, "XdndActionDescription", False);
   _ecore_x_atom_xdnd_proxy               = XInternAtom(_ecore_x_disp, "XdndProxy", False);
   _ecore_x_atom_xdnd_status              = XInternAtom(_ecore_x_disp, "XdndStatus", False);
   _ecore_x_atom_xdnd_leave               = XInternAtom(_ecore_x_disp, "XdndLeave", False);
   _ecore_x_atom_xdnd_drop                = XInternAtom(_ecore_x_disp, "XdndDrop", False);
   _ecore_x_atom_xdnd_finished            = XInternAtom(_ecore_x_disp, "XdndFinished", False);

   /* Initialize the globally defined xdnd atoms */
   ECORE_X_DND_ACTION_COPY                = _ecore_x_atom_xdnd_action_copy;
   ECORE_X_DND_ACTION_MOVE                = _ecore_x_atom_xdnd_action_move;
   ECORE_X_DND_ACTION_LINK                = _ecore_x_atom_xdnd_action_link;
   ECORE_X_DND_ACTION_ASK                 = _ecore_x_atom_xdnd_action_ask;
   ECORE_X_DND_ACTION_PRIVATE             = _ecore_x_atom_xdnd_action_private;
   
   _ecore_x_atom_net_supported            = XInternAtom(_ecore_x_disp, "_NET_SUPPORTED", False);
   _ecore_x_atom_net_supporting_wm_check  = XInternAtom(_ecore_x_disp, "_NET_SUPPORTING_WM_CHECK", False);

   _ecore_x_atom_net_number_of_desktops   = XInternAtom(_ecore_x_disp, "_NET_NUMBER_OF_DESKTOPS", False);
   _ecore_x_atom_net_desktop_geometry     = XInternAtom(_ecore_x_disp, "_NET_DESKTOP_GEOMETRY", False);
   _ecore_x_atom_net_desktop_names        = XInternAtom(_ecore_x_disp, "_NET_DESKTOP_NAMES", False);
   _ecore_x_atom_net_current_desktop      = XInternAtom(_ecore_x_disp, "_NET_CURRENT_DESKTOP", False);
   _ecore_x_atom_net_desktop_viewport     = XInternAtom(_ecore_x_disp, "_NET_DESKTOP_VIEWPORT", False);
   _ecore_x_atom_net_workarea             = XInternAtom(_ecore_x_disp, "_NET_WORKAREA", False);
   _ecore_x_atom_net_virtual_roots        = XInternAtom(_ecore_x_disp, "_NET_VIRTUAL_ROOTS", False);

   _ecore_x_atom_net_client_list          = XInternAtom(_ecore_x_disp, "_NET_CLIENT_LIST", False);
   _ecore_x_atom_net_client_list_stacking = XInternAtom(_ecore_x_disp, "_NET_CLIENT_LIST_STACKING", False);
   _ecore_x_atom_net_active_window        = XInternAtom(_ecore_x_disp, "_NET_ACTIVE_WINDOW", False);

   _ecore_x_atom_net_close_window         = XInternAtom(_ecore_x_disp, "_NET_CLOSE_WINDOW", False);
   _ecore_x_atom_net_wm_moveresize        = XInternAtom(_ecore_x_disp, "_NET_WM_MOVERESIZE", False);

   _ecore_x_atom_net_wm_name              = XInternAtom(_ecore_x_disp, "_NET_WM_NAME", False);
   _ecore_x_atom_net_wm_visible_name      = XInternAtom(_ecore_x_disp, "_NET_WM_VISIBLE_NAME", False);
   _ecore_x_atom_net_wm_icon_name         = XInternAtom(_ecore_x_disp, "_NET_WM_ICON_NAME", False);
   _ecore_x_atom_net_wm_visible_icon_name = XInternAtom(_ecore_x_disp, "_NET_WM_VISIBLE_ICON_NAME", False);
   _ecore_x_atom_net_wm_desktop           = XInternAtom(_ecore_x_disp, "_NET_WM_DESKTOP", False);
   _ecore_x_atom_net_wm_window_type       = XInternAtom(_ecore_x_disp, "_NET_WM_WINDOW_TYPE", False);
   _ecore_x_atom_net_wm_state             = XInternAtom(_ecore_x_disp, "_NET_WM_STATE", False);
   _ecore_x_atom_net_wm_allowed_actions   = XInternAtom(_ecore_x_disp, "_NET_WM_ALLOWED_ACTIONS", False);
   _ecore_x_atom_net_wm_strut             = XInternAtom(_ecore_x_disp, "_NET_WM_STRUT", False);
   _ecore_x_atom_net_wm_strut_partial     = XInternAtom(_ecore_x_disp, "_NET_WM_STRUT_PARTIAL", False);
   _ecore_x_atom_net_wm_icon_geometry     = XInternAtom(_ecore_x_disp, "_NET_WM_ICON_GEOMETRY", False);
   _ecore_x_atom_net_wm_icon              = XInternAtom(_ecore_x_disp, "_NET_WM_ICON", False);
   _ecore_x_atom_net_wm_pid               = XInternAtom(_ecore_x_disp, "_NET_WM_PID", False);
   _ecore_x_atom_net_wm_user_time         = XInternAtom(_ecore_x_disp, "_NET_WM_USER_TIME", False);

   _ecore_x_atom_net_wm_window_type_desktop = XInternAtom(_ecore_x_disp, "_NET_WM_WINDOW_TYPE_DESKTOP", False);
   _ecore_x_atom_net_wm_window_type_dock    = XInternAtom(_ecore_x_disp, "_NET_WM_WINDOW_TYPE_DOCK", False);
   _ecore_x_atom_net_wm_window_type_toolbar = XInternAtom(_ecore_x_disp, "_NET_WM_WINDOW_TYPE_TOOLBAR", False);
   _ecore_x_atom_net_wm_window_type_menu    = XInternAtom(_ecore_x_disp, "_NET_WM_WINDOW_TYPE_MENU", False);
   _ecore_x_atom_net_wm_window_type_utility = XInternAtom(_ecore_x_disp, "_NET_WM_WINDOW_TYPE_UTILITY", False);
   _ecore_x_atom_net_wm_window_type_splash  = XInternAtom(_ecore_x_disp, "_NET_WM_WINDOW_TYPE_SPLASH", False);
   _ecore_x_atom_net_wm_window_type_dialog  = XInternAtom(_ecore_x_disp, "_NET_WM_WINDOW_TYPE_DIALOG", False);
   _ecore_x_atom_net_wm_window_type_normal  = XInternAtom(_ecore_x_disp, "_NET_WM_WINDOW_TYPE_NORMAL", False);

   _ecore_x_atom_net_wm_state_modal          = XInternAtom(_ecore_x_disp, "_NET_WM_STATE_MODAL", False);
   _ecore_x_atom_net_wm_state_sticky         = XInternAtom(_ecore_x_disp, "_NET_WM_STATE_STICKY", False);
   _ecore_x_atom_net_wm_state_maximized_vert = XInternAtom(_ecore_x_disp, "_NET_WM_STATE_MAXIMIZED_VERT", False);
   _ecore_x_atom_net_wm_state_maximized_horz = XInternAtom(_ecore_x_disp, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
   _ecore_x_atom_net_wm_state_shaded         = XInternAtom(_ecore_x_disp, "_NET_WM_STATE_SHADED", False);
   _ecore_x_atom_net_wm_state_skip_taskbar   = XInternAtom(_ecore_x_disp, "_NET_WM_STATE_SKIP_TASKBAR", False);
   _ecore_x_atom_net_wm_state_skip_pager     = XInternAtom(_ecore_x_disp, "_NET_WM_STATE_SKIP_PAGER", False);
   _ecore_x_atom_net_wm_state_hidden         = XInternAtom(_ecore_x_disp, "_NET_WM_STATE_HIDDEN", False);
   _ecore_x_atom_net_wm_state_fullscreen     = XInternAtom(_ecore_x_disp, "_NET_WM_STATE_FULLSCREEN", False);
   _ecore_x_atom_net_wm_state_above          = XInternAtom(_ecore_x_disp, "_NET_WM_STATE_ABOVE", False);
   _ecore_x_atom_net_wm_state_below          = XInternAtom(_ecore_x_disp, "_NET_WM_STATE_BELOW", False);
   _ecore_x_atom_net_wm_window_opacity       = XInternAtom(_ecore_x_disp, "_NET_WM_WINDOW_OPACITY", False);

   _ecore_x_atom_compound_text      = XInternAtom(_ecore_x_disp, "COMPOUND_TEXT", False);
   _ecore_x_atom_utf8_string        = XInternAtom(_ecore_x_disp, "UTF8_STRING", False);
   _ecore_x_atom_file_name          = XInternAtom(_ecore_x_disp, "FILE_NAME", False);
   _ecore_x_atom_string             = XInternAtom(_ecore_x_disp, "STRING", False);
   _ecore_x_atom_text               = XInternAtom(_ecore_x_disp, "TEXT", False);

   _ecore_x_atoms_wm_protocols[ECORE_X_WM_PROTOCOL_DELETE_REQUEST] = _ecore_x_atom_wm_delete_window;
   _ecore_x_atoms_wm_protocols[ECORE_X_WM_PROTOCOL_TAKE_FOCUS] = _ecore_x_atom_wm_take_focus;

   _ecore_x_selection_data_init();
   _ecore_x_dnd_init();
   
   _ecore_x_init_count++;
   return _ecore_x_init_count;
}

static int
_ecore_x_shutdown(int close_display)
{
   _ecore_x_init_count--;
   if (_ecore_x_init_count > 0) return _ecore_x_init_count;
   if (!_ecore_x_disp) return _ecore_x_init_count;
   if (close_display)
      XCloseDisplay(_ecore_x_disp);
   else
      close(ConnectionNumber(_ecore_x_disp));
   free(_ecore_x_event_handlers);
   ecore_main_fd_handler_del(_ecore_x_fd_handler_handle);
   ecore_event_filter_del(_ecore_x_filter_handler);
   _ecore_x_fd_handler_handle = NULL;
   _ecore_x_filter_handler = NULL;
   _ecore_x_disp = NULL;
   _ecore_x_event_handlers = NULL;
   _ecore_x_selection_shutdown();
   if (_ecore_x_init_count < 0) _ecore_x_init_count = 0;
   return _ecore_x_init_count;
}

/**
 * Shuts down the Ecore X library.
 *
 * In shutting down the library, the X display connection is terminated
 * and any event handlers for it are removed.
 *
 * @return  The number of times the library has been initialized without
 *          being shut down.
 * @ingroup Ecore_X_Init_Group
 */
int
ecore_x_shutdown(void)
{
   return _ecore_x_shutdown(1);
}

/**
 * Shuts down the Ecore X library.
 *
 * As ecore_x_shutdown, except do not close Display, only connection.
 *
 * @ingroup Ecore_X_Init_Group
 */
int
ecore_x_disconnect(void)
{
   return _ecore_x_shutdown(0);
}

/**
 * @defgroup Ecore_X_Display_Attr_Group X Display Attributes
 *
 * Functions that set and retrieve X display attributes.
 */

/**
 * Retrieves the Ecore_X_Display handle used for the current X connection.
 * @return  The current X display.
 * @ingroup Ecore_X_Display_Attr_Group
 */
Ecore_X_Display *
ecore_x_display_get(void)
{
   return (Ecore_X_Display *)_ecore_x_disp;
}

/**
 * Retrieves the X display file descriptor.
 * @return  The current X display file descriptor.
 * @ingroup Ecore_X_Display_Attr_Group
 */
int
ecore_x_fd_get(void)
{
   return ConnectionNumber(_ecore_x_disp);
}

/**
 * Sets the timeout for a double and triple clicks to be flagged.
 * 
 * This sets the time between clicks before the double_click flag is
 * set in a button down event. If 3 clicks occur within double this
 * time, the triple_click flag is also set.
 *
 * @param   t The time in seconds
 * @ingroup Ecore_X_Display_Attr_Group
 */
void
ecore_x_double_click_time_set(double t)
{
   if (t < 0.0) t = 0.0;
   _ecore_x_double_click_time = t;
}

/**
 * Retrieves the double and triple click flag timeout.
 *
 * See @ref ecore_x_double_click_time_set for more information.
 *
 * @return  The timeout for double clicks in seconds.
 * @ingroup Ecore_X_Display_Attr_Group
 */
double
ecore_x_double_click_time_get(void)
{
   return _ecore_x_double_click_time;
}

/**
 * @defgroup Ecore_X_Flush_Group X Synchronization Functions
 *
 * Functions that ensure that all commands that have been issued by the
 * Ecore X library have been sent to the server.
 */

/**
 * Sends all X commands in the X Display buffer.
 * @ingroup Ecore_X_Flush_Group
 */
void
ecore_x_flush(void)
{
   XFlush(_ecore_x_disp);
}

/**
 * Flushes the command buffer and waits until all requests have been
 * processed by the server.
 * @ingroup Ecore_X_Flush_Group
 */
void
ecore_x_sync(void)
{
   XSync(_ecore_x_disp, False);
}

/**
 * Kill all clients with subwindows under a given window.
 *
 * You can kill all clients connected to the X server by using
 * @ref ecore_x_window_root_list to get a list of root windows, and
 * then passing each root window to this function.
 *
 * @param root The window whose children will be killed.
 */
void
ecore_x_killall(Ecore_X_Window root)
{
   int screens;
   int i, j;
   
   XGrabServer(_ecore_x_disp);
   screens = ScreenCount(_ecore_x_disp);

   /* Tranverse window tree starting from root, and drag each
    * before the firing squad */
   for (i = 0; i < screens; ++i)
   {
      Window root_r;
      Window parent_r;
      Window *children_r = NULL;
      int num_children = 0;

      while (XQueryTree(_ecore_x_disp, root, &root_r, &parent_r,
               &children_r, &num_children) && num_children > 0)
      {
         for (j = 0; j < num_children; ++j)
         {
            XKillClient(_ecore_x_disp, children_r[j]);
         }

         XFree(children_r);
      }
   }

   XUngrabServer(_ecore_x_disp);
   XSync(_ecore_x_disp, False);
}

static int
_ecore_x_fd_handler(void *data, Ecore_Fd_Handler *fd_handler)
{
   Display *d;
   
   d = data;
   while (XPending(d))
     {
	XEvent ev;
	
	XNextEvent(d, &ev);
	if ((ev.type >= 0) && (ev.type < _ecore_x_event_handlers_num))
	  {
	     if (_ecore_x_event_handlers[ev.type])
	       _ecore_x_event_handlers[ev.type] (&ev);
	  }
     }
   return 1;
}

static int
_ecore_x_fd_handler_buf(void *data, Ecore_Fd_Handler *fd_handler)
{
   Display *d;

   d = data;
   if (XPending(d)) return 1;
   return 0;
}

static int
_ecore_x_key_mask_get(KeySym sym)
{
   XModifierKeymap    *mod;
   KeyCode             nl;
   int                 i;
   const int           masks[8] = 
     {
	ShiftMask, LockMask, ControlMask, 
	  Mod1Mask, Mod2Mask, Mod3Mask, Mod4Mask, Mod5Mask
     };
   
   mod = XGetModifierMapping(_ecore_x_disp);
   nl = XKeysymToKeycode(_ecore_x_disp, sym);
   if ((mod) && (mod->max_keypermod > 0))
     {
	for (i = 0; i < (8 * mod->max_keypermod); i++)
	  {
	     if ((nl) && (mod->modifiermap[i] == nl))
	       {
		  int mask;
		  
		  mask = masks[i / mod->max_keypermod];
		  if (mod->modifiermap) XFree(mod->modifiermap);
		  XFree(mod);
		  return mask;
	       }
	  }
     }
   if (mod)
     {
	if (mod->modifiermap) XFree(mod->modifiermap);
	XFree(mod);
     }
  return 0;
}

typedef struct _Ecore_X_Filter_Data Ecore_X_Filter_Data;

struct _Ecore_X_Filter_Data
{
   int last_event_type;
};

static void *
_ecore_x_event_filter_start(void *data)
{
   Ecore_X_Filter_Data *filter_data;
   
   filter_data = calloc(1, sizeof(Ecore_X_Filter_Data));
   return filter_data;
}

static int
_ecore_x_event_filter_filter(void *data, void *loop_data,int type, void *event)
{
   Ecore_X_Filter_Data *filter_data;
   
   filter_data = loop_data;
   if (!filter_data) return 1;
   if (type == ECORE_X_EVENT_MOUSE_MOVE)
     {
	if ((filter_data->last_event_type) == ECORE_X_EVENT_MOUSE_MOVE) 
	  {
	     filter_data->last_event_type = type;
	     return 0;
	  }
     }
   filter_data->last_event_type = type;
   return 1;
}

static void
_ecore_x_event_filter_end(void *data, void *loop_data)
{
   Ecore_X_Filter_Data *filter_data;
   
   filter_data = loop_data;
   if (filter_data) free(filter_data);
}






















/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/* FIXME: these funcs need categorising */
/*****************************************************************************/

/**
 * Retrieves the geometry of the given drawable.
 * @param d The given drawable.
 * @param x Pointer to an integer into which the X position is to be stored.
 * @param y Pointer to an integer into which the Y position is to be stored.
 * @param w Pointer to an integer into which the width is to be stored.
 * @param h Pointer to an integer into which the height is to be stored.
 */
void
ecore_x_drawable_geometry_get(Ecore_X_Drawable d, int *x, int *y, int *w, int *h)
{
   Window         dummy_win;
   int            ret_x, ret_y;
   unsigned int   ret_w, ret_h, dummy_border, dummy_depth;

   if (!XGetGeometry(_ecore_x_disp, d, &dummy_win, &ret_x, &ret_y,
                     &ret_w, &ret_h, &dummy_border, &dummy_depth))
   {
      ret_x = 0;
      ret_y = 0;
      ret_w = 0;
      ret_h = 0;
   }

   if (x) *x = ret_x;
   if (y) *y = ret_y;
   if (w) *w = (int) ret_w;
   if (h) *h = (int) ret_h;
}

/**
 * Retrieves the width of the border of the given drawable.
 * @param  d The given drawable.
 * @return The border width of the given drawable.
 */
int
ecore_x_drawable_border_width_get(Ecore_X_Drawable d)
{
   Window         dummy_win;
   int            dummy_x, dummy_y;
   unsigned int   dummy_w, dummy_h, border_ret, dummy_depth;

   if (!XGetGeometry(_ecore_x_disp, d, &dummy_win, &dummy_x, &dummy_y,
                     &dummy_w, &dummy_h, &border_ret, &dummy_depth))
      border_ret = 0;

   return (int) border_ret;
}

/**
 * Retrieves the depth of the given drawable.
 * @param  d The given drawable.
 * @return The depth of the given drawable.
 */
int
ecore_x_drawable_depth_get(Ecore_X_Drawable d)
{
   Window         dummy_win;
   int            dummy_x, dummy_y;
   unsigned int   dummy_w, dummy_h, dummy_border, depth_ret;

   if (!XGetGeometry(_ecore_x_disp, d, &dummy_win, &dummy_x, &dummy_y,
                     &dummy_w, &dummy_h, &dummy_border, &depth_ret))
      depth_ret = 0;

   return (int) depth_ret;
}

/**
 * Get a list of all the root windows on the server.
 *
 * @note   The returned array will need to be freed after use.
 * @param  num_ret Pointer to integer to put number of windows returned in.
 * @return An array of all the root windows.  @c NULL is returned if memory
 *         could not be allocated for the list, or if @p num_ret is @c NULL.
 */
Ecore_X_Window *
ecore_x_window_root_list(int *num_ret)
{
   int num, i;
   Ecore_X_Window *roots;
   
   if (!num_ret) return NULL;
   *num_ret = 0;
   num = ScreenCount(_ecore_x_disp);
   roots = malloc(num * sizeof(Window));
   if (!roots) return NULL;
   *num_ret = num;
   for (i = 0; i < num; i++) roots[i] = RootWindow(_ecore_x_disp, i);
   return roots;
}

static void _ecore_x_window_manage_error(void *data);

static int _ecore_x_window_manage_failed = 0;
static void
_ecore_x_window_manage_error(void *data)
{
   if ((ecore_x_error_request_get() == X_ChangeWindowAttributes) &&
       (ecore_x_error_code_get() == BadAccess))
     _ecore_x_window_manage_failed = 1;
}

int
ecore_x_window_manage(Ecore_X_Window win)
{
   XWindowAttributes   att;
   
   if (XGetWindowAttributes(_ecore_x_disp, win, &att) != True) return 0;
   ecore_x_sync();
   _ecore_x_window_manage_failed = 0;
   ecore_x_error_handler_set(_ecore_x_window_manage_error, NULL);
   XSelectInput(_ecore_x_disp, win, 
		EnterWindowMask | 
		LeaveWindowMask | 
		PropertyChangeMask | 
		ResizeRedirectMask |
		SubstructureRedirectMask | 
		SubstructureNotifyMask |
		KeyPressMask | 
		KeyReleaseMask |
		att.your_event_mask);
   ecore_x_sync();
   ecore_x_error_handler_set(NULL, NULL);
   if (_ecore_x_window_manage_failed)
     {
	_ecore_x_window_manage_failed = 0;
	return 0;
     }
   return 1;
}

void
ecore_x_window_container_manage(Ecore_X_Window win)
{
   XSelectInput(_ecore_x_disp, win, 
		ResizeRedirectMask |
		SubstructureRedirectMask | 
		SubstructureNotifyMask);
}

void
ecore_x_window_client_manage(Ecore_X_Window win)
{
   XSelectInput(_ecore_x_disp, win, 
		PropertyChangeMask | 
		ResizeRedirectMask |
		FocusChangeMask |
		ColormapChangeMask |
		VisibilityChangeMask |
		StructureNotifyMask
		);
}

void
ecore_x_window_sniff(Ecore_X_Window win)
{
   XSelectInput(_ecore_x_disp, win,
		PropertyChangeMask |
		SubstructureNotifyMask);
}

void
ecore_x_window_client_sniff(Ecore_X_Window win)
{
   XSelectInput(_ecore_x_disp, win,
		PropertyChangeMask |
		StructureNotifyMask |
		FocusChangeMask |
		ColormapChangeMask |
		VisibilityChangeMask |
		StructureNotifyMask);
}

/**
 * Retrieves the atom value associated with the given name.
 * @param  name The given name.
 * @return Associated atom value.
 */
Ecore_X_Atom    
ecore_x_atom_get(char *name)
{
   if (!_ecore_x_disp) return 0;
   return XInternAtom(_ecore_x_disp, name, False);
}






int
ecore_x_window_attributes_get(Ecore_X_Window win, Ecore_X_Window_Attributes *att_ret)
{
   XWindowAttributes att;
   
   if (!XGetWindowAttributes(_ecore_x_disp, win, &att)) return 0;
   memset(att_ret, 0, sizeof(Ecore_X_Window_Attributes));
   att_ret->root = att.root;
   att_ret->x = att.x;
   att_ret->y = att.y;
   att_ret->w = att.width;
   att_ret->h = att.height;
   att_ret->border = att.border_width;
   att_ret->depth = att.depth;
   if (att.map_state != IsUnmapped) att_ret->visible = 1;
   if (att.map_state == IsViewable) att_ret->viewable = 1;
   if (att.override_redirect) att_ret->override = 1;
   if (att.class == InputOnly) att_ret->input_only = 1;
   if (att.save_under) att_ret->save_under = 1;
   att_ret->event_mask.mine = att.your_event_mask;
   att_ret->event_mask.all = att.your_event_mask;
   att_ret->event_mask.no_propagate = att.do_not_propagate_mask;
   return 1;
}





Ecore_X_Cursor
ecore_x_cursor_new(Ecore_X_Window win, int *pixels, int w, int h, int hot_x, int hot_y)
{
   XColor c1, c2;
   Cursor c;
   Pixmap pmap, mask;
   GC gc;
   XGCValues gcv;
   XImage *xim;
   unsigned int *pix;
   int fr, fg, fb, br, bg, bb;
   int brightest = 0;
   int darkest = 255 * 3;
   int x, y;
   const int dither[2][2] =
     { 
	  {0, 2},
	  {3, 1}
     };

   
   pmap = XCreatePixmap(_ecore_x_disp, win, w, h, 1);
   mask = XCreatePixmap(_ecore_x_disp, win, w, h, 1);
   xim = XCreateImage(_ecore_x_disp, 
		      DefaultVisual(_ecore_x_disp, 0), 
		      1, ZPixmap, 0, NULL, w, h, 32, 0);
   xim->data = malloc(xim->bytes_per_line * xim->height);
   
   fr = 0x00; fg = 0x00; fb = 0x00;
   br = 0xff; bg = 0xff; bb = 0xff;
   pix = pixels;
   for (y = 0; y < h; y++)
     {
	for (x = 0; x < w; x++)
	  {
	     int r, g, b, a;
	     
	     a = (pix[0] >> 24) & 0xff;
	     r = (pix[0] >> 16) & 0xff;
	     g = (pix[0] >> 8 ) & 0xff;
	     b = (pix[0]      ) & 0xff;
	     if (a > 0)
	       {
		  if ((r + g + b) > brightest)
		    {
		       brightest = r + g + b; 
		       br = r;
		       bg = g;
		       bb = b;
		    }
		  if ((r + g + b) < darkest)
		    {
		       darkest = r + g + b; 
		       fr = r;
		       fg = g;
		       fb = b;
		    }
	       }
	     pix++;
	  }
     }
   
   pix = pixels;
   for (y = 0; y < h; y++)
     {
	for (x = 0; x < w; x++)
	  {
	     int v;
	     int r, g, b;
	     int d1, d2;
	     
	     r = (pix[0] >> 16) & 0xff;
	     g = (pix[0] >> 8 ) & 0xff;
	     b = (pix[0]      ) & 0xff;
	     d1 = 
	       ((r - fr) * (r - fr)) +
	       ((g - fg) * (g - fg)) +
	       ((b - fb) * (b - fb));
	     d2 = 
	       ((r - br) * (r - br)) +
	       ((g - bg) * (g - bg)) +
	       ((b - bb) * (b - bb));
	     v = (((d2 * 255) / (d1 + d2)) * 5) / 256;
	     if (v > dither[x & 0x1][y & 0x1]) v = 1;
	     else v = 0;
	     XPutPixel(xim, x, y, v);
	     pix++;
	  }
     }
   gc = XCreateGC(_ecore_x_disp, pmap, 0, &gcv);
   XPutImage(_ecore_x_disp, pmap, gc, xim, 0, 0, 0, 0, w, h);   
   XFreeGC(_ecore_x_disp, gc);

   pix = pixels;
   for (y = 0; y < h; y++)
     {
	for (x = 0; x < w; x++)
	  {
	     int v;
	     
	     v = (((pix[0] >> 24) & 0xff) * 5) / 256;
	     if (v > dither[x & 0x1][y & 0x1]) v = 1;
	     else v = 0;
	     XPutPixel(xim, x, y, v);
	     pix++;
	  }
     }
   gc = XCreateGC(_ecore_x_disp, mask, 0, &gcv);
   XPutImage(_ecore_x_disp, mask, gc, xim, 0, 0, 0, 0, w, h);   
   XFreeGC(_ecore_x_disp, gc);

   free(xim->data);
   xim->data = NULL;
   XDestroyImage(xim);
   
   c1.pixel = 0;
   c1.red   = fr << 8 | fr;
   c1.green = fg << 8 | fg;
   c1.blue  = fb << 8 | fb;
   c1.flags = DoRed | DoGreen | DoBlue;
   
   c2.pixel = 0;
   c2.red   = br << 8 | br;
   c2.green = bg << 8 | bg;
   c2.blue  = bb << 8 | bb;
   c2.flags = DoRed | DoGreen | DoBlue;   
   
   c = XCreatePixmapCursor(_ecore_x_disp, 
			   pmap, mask, 
			   &c1, &c2, 
			   hot_x, hot_y);   
   XFreePixmap(_ecore_x_disp, pmap);
   XFreePixmap(_ecore_x_disp, mask);
   return c;
}

void
ecore_x_cursor_free(Ecore_X_Cursor c)
{
   XFreeCursor(_ecore_x_disp, c);
}

/*
 * Returns the cursor for the given shape.
 * Note that the return value must not be freed with
 * ecore_x_cursor_free()!
 */
Ecore_X_Cursor
ecore_x_cursor_shape_get(int shape)
{
   /* Shapes are defined in Ecore_X_Cursor.h */
   return XCreateFontCursor(_ecore_x_disp, shape);
}

int
ecore_x_pointer_grab(Ecore_X_Window win)
{
   return XGrabPointer(_ecore_x_disp, win, False,
		       ButtonPressMask | ButtonReleaseMask | 
		       EnterWindowMask | LeaveWindowMask | PointerMotionMask,
		       GrabModeAsync, GrabModeAsync,
		       None, None, CurrentTime);
}

void
ecore_x_pointer_ungrab(void)
{
   XUngrabPointer(_ecore_x_disp, CurrentTime);
}

int
ecore_x_keyboard_grab(Ecore_X_Window win)
{
   return XGrabKeyboard(_ecore_x_disp, win, False,
			GrabModeAsync, GrabModeAsync,
			CurrentTime);
}

void ecore_x_keyboard_ungrab(void)
{
   XUngrabKeyboard(_ecore_x_disp, CurrentTime);   
}

void
ecore_x_grab(void)
{
   _ecore_x_grab_count++;
   
   if (_ecore_x_grab_count == 1)
      XGrabServer(_ecore_x_disp);
}

void
ecore_x_ungrab(void)
{
   _ecore_x_grab_count--;
   if (_ecore_x_grab_count < 0)
      _ecore_x_grab_count = 0;

   if (_ecore_x_grab_count == 0)
   {
      XUngrabServer(_ecore_x_disp);
      XSync(_ecore_x_disp, False);
   }
}


/**
 * Send client message with given type and format 32.
 *
 * @param win     The window the message is sent to.
 * @param type    The client message type.
 * @param d0...d4 The client message data items.
 *
 * @return !0 on success.
 */
int
ecore_x_client_message32_send(Ecore_X_Window win, Ecore_X_Atom type,
			      long d0, long d1, long d2, long d3, long d4)
{
    XEvent xev;

    xev.xclient.window = win;
    xev.xclient.type = ClientMessage;
    xev.xclient.message_type = type;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = d0;
    xev.xclient.data.l[1] = d1;
    xev.xclient.data.l[2] = d2;
    xev.xclient.data.l[3] = d3;
    xev.xclient.data.l[4] = d4;

    return XSendEvent(_ecore_x_disp, win, False, NoEventMask, &xev);
}

/**
 * Send client message with given type and format 8.
 *
 * @param win     The window the message is sent to.
 * @param type    The client message type.
 * @param data    Data to be sent.
 * @param len     Number of data bytes, max 20.
 *
 * @return !0 on success.
 */
int
ecore_x_client_message8_send(Ecore_X_Window win, Ecore_X_Atom type,
			     const void *data, int len)
{
    XEvent xev;

    xev.xclient.window = win;
    xev.xclient.type = ClientMessage;
    xev.xclient.message_type = type;
    xev.xclient.format = 8;
    if (len > 20)
        len = 20;
    memcpy(xev.xclient.data.b, data, len);
    memset(xev.xclient.data.b + len, 0, 20 - len);

    return XSendEvent(_ecore_x_disp, win, False, NoEventMask, &xev);
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
