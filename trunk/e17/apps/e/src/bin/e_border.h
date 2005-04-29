/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifdef E_TYPEDEFS

typedef enum _E_Direction
{
   E_DIRECTION_UP,
   E_DIRECTION_DOWN,
   E_DIRECTION_LEFT,
   E_DIRECTION_RIGHT
} E_Direction;

typedef enum _E_Transition
{
   E_TRANSITION_LINEAR,
   E_TRANSITION_SINUSOIDAL,
   E_TRANSITION_ACCELERATE,
   E_TRANSITION_DECELERATE
} E_Transition;

typedef struct _E_Border                     E_Border;
typedef struct _E_Border_Pending_Move_Resize E_Border_Pending_Move_Resize;
typedef struct _E_Event_Border_Resize        E_Event_Border_Resize;
typedef struct _E_Event_Border_Move          E_Event_Border_Move;
typedef struct _E_Event_Border_Add           E_Event_Border_Add;
typedef struct _E_Event_Border_Remove        E_Event_Border_Remove;
typedef struct _E_Event_Border_Show          E_Event_Border_Show;
typedef struct _E_Event_Border_Hide          E_Event_Border_Hide;
typedef struct _E_Event_Border_Iconify       E_Event_Border_Iconify;
typedef struct _E_Event_Border_Uniconify     E_Event_Border_Uniconify;
typedef struct _E_Event_Border_Stick         E_Event_Border_Stick;
typedef struct _E_Event_Border_Unstick       E_Event_Border_Unstick;
typedef struct _E_Event_Border_Zone_Set      E_Event_Border_Zone_Set;
typedef struct _E_Event_Border_Desk_Set      E_Event_Border_Desk_Set;
typedef struct _E_Event_Border_Raise         E_Event_Border_Raise;
typedef struct _E_Event_Border_Lower         E_Event_Border_Lower;
typedef struct _E_Event_Border_Icon_Change   E_Event_Border_Icon_Change;

#else
#ifndef E_BORDER_H
#define E_BORDER_H

#define E_BORDER_TYPE 0xE0b01002

struct _E_Border
{
   E_Object             e_obj_inherit;

   struct {
	struct {
	     int x, y, w, h;
	     int mx, my;
	} current, last_down[3], last_up[3];
   } mouse;

   struct {
	struct {
	     int x, y, w, h;
	     int mx, my;
	     int button;
	} down;
   } moveinfo;

   Ecore_X_Window  win;
   int             x, y, w, h;
   int             ref;
   E_Container    *container;
   E_Zone         *zone;
   E_Desk         *desk;
   Evas_List      *handlers;

   struct {
      int          l, r, t, b;
   } client_inset;

   Ecore_Evas     *bg_ecore_evas;
   Evas           *bg_evas;
   Ecore_X_Window  bg_win;
   Evas_Object    *bg_object;
   Evas_Object    *icon_object;
   Ecore_X_Window  event_win;
   
   struct {
      Ecore_X_Window shell_win;
      Ecore_X_Window win;
      
      int x, y, w, h;
      
      struct {
	 unsigned char changed : 1;
	 char *name;
      } border;
      
      unsigned char shaped : 1;
      
      struct {
	 char *title;
	 char *name;
	 char *class;
	 char *icon_name;
	 char *machine;
	 int min_w, min_h;
	 int max_w, max_h;
	 int base_w, base_h;
	 int step_w, step_h;
	 int start_x, start_y;
	 double min_aspect, max_aspect;
	 Ecore_X_Window_State_Hint initial_state;
	 Ecore_X_Window_State_Hint state;
	 Ecore_X_Pixmap icon_pixmap;
	 Ecore_X_Pixmap icon_mask;
	 Ecore_X_Window icon_window;
	 Ecore_X_Window window_group;
	 Ecore_X_Gravity gravity;
	 unsigned char take_focus : 1;
	 unsigned char accepts_focus : 1;
	 unsigned char urgent : 1;
	 unsigned char delete_request : 1;
	 unsigned char request_pos : 1;
	 struct {
	    unsigned int title : 1;
	    unsigned int name_class : 1;
	    unsigned int icon_name : 1;
	    unsigned int machine : 1;
	    unsigned int hints : 1;
	    unsigned int size_pos_hints : 1;
	    unsigned int protocol : 1;
	 } fetch;
      } icccm;
      struct {
	 Ecore_X_MWM_Hint_Func func;
	 Ecore_X_MWM_Hint_Decor decor;
	 Ecore_X_MWM_Hint_Input input;
	 unsigned char exists : 1;
	 unsigned char borderless : 1;
	 struct {
	    unsigned int hints : 1;
	 } fetch;
      } mwm;
      struct {
	 pid_t pid;
	 int desktop;
	 struct {
	    unsigned int pid : 1;
	    unsigned int desktop : 1;
	 } fetch;
	 
	 /* NetWM Window state */
	 struct {
	    unsigned char modal : 1;
	    unsigned char sticky : 1;
	    unsigned char shaded : 1;
	    unsigned char hidden : 1;
	    unsigned char maximized_v : 1;
	    unsigned char maximized_h : 1;
	    unsigned char skip_taskbar : 1;
	    unsigned char skip_pager : 1;
	    unsigned char fullscreen : 1;
	    unsigned char stacking : 2; /* 0 = None, 1 = Above, 2 = Below */
	 } state;

	 Ecore_X_Window_Type type;
	 
      } netwm;
      Ecore_X_Window_Attributes initial_attributes;
   } client;
   
   E_Container_Shape *shape;
   
   unsigned char   visible : 1;
   unsigned char   moving : 1;
   unsigned char   focused : 1;
   unsigned char   new_client : 1;
   unsigned char   re_manage : 1;
   unsigned char   shading : 1;
   unsigned char   shaded : 1;
   unsigned char   maximized : 1;
   unsigned char   iconic : 1;
   unsigned char   sticky : 1;
   unsigned char   shaped : 1;
   unsigned char   need_shape_merge : 1;
   unsigned char   need_shape_export : 1;
   unsigned char   fullscreen : 1;
   
   unsigned char   changed : 1;
   
   unsigned char   ignore_first_unmap;
   unsigned char   resize_mode;
   
   struct {
      int x, y, w, h;
   } saved;

   struct {
      double start;
      double val;
      int x, y;
      E_Direction dir;
      Ecore_Animator *anim;
   } shade;
   
   Evas_List *stick_desks;
   E_Menu *border_menu;
   Evas_List *pending_move_resize;
   
   struct {
      unsigned int visible : 1;
      unsigned int pos : 1;
      unsigned int size : 1;
      unsigned int stack : 1;
      unsigned int prop : 1;
      unsigned int border : 1;
      unsigned int reset_gravity : 1;
      unsigned int shading : 1;
      unsigned int shaded : 1;
      unsigned int shape : 1;
   } changes;

   struct {
	unsigned char start : 1;
	int x, y;
   } drag;

   unsigned int layer;
};

struct _E_Border_Pending_Move_Resize 
{
   int x, y, w, h;
   unsigned char move : 1;
   unsigned char resize : 1;
};

struct _E_Event_Border_Resize
{
   E_Border *border;
};

struct _E_Event_Border_Move
{
   E_Border *border;
};

struct _E_Event_Border_Add
{
   E_Border *border;
};

struct _E_Event_Border_Remove
{
   E_Border *border;
};

struct _E_Event_Border_Show
{
   E_Border *border;
};

struct _E_Event_Border_Hide
{
   E_Border *border;
};

struct _E_Event_Border_Iconify
{
   E_Border *border;
};

struct _E_Event_Border_Uniconify
{
   E_Border *border;
};

struct _E_Event_Border_Stick
{
   E_Border *border;
};

struct _E_Event_Border_Unstick
{
   E_Border *border;
};

struct _E_Event_Border_Zone_Set
{
   E_Border *border;
   E_Zone   *zone;
};

struct _E_Event_Border_Desk_Set
{
   E_Border *border;
   E_Desk   *desk;
};

struct _E_Event_Border_Raise
{
   E_Border *border, *above;
};

struct _E_Event_Border_Lower
{
   E_Border *border, *below;
};

struct _E_Event_Border_Icon_Change
{
   E_Border *border;
};


EAPI int       e_border_init(void);
EAPI int       e_border_shutdown(void);

EAPI E_Border *e_border_new(E_Container *con, Ecore_X_Window win, int first_map);
EAPI void      e_border_free(E_Border *bd);
EAPI void      e_border_ref(E_Border *bd);
EAPI void      e_border_unref(E_Border *bd);
EAPI void      e_border_zone_set(E_Border *bd, E_Zone *zone);
EAPI void      e_border_desk_set(E_Border *bd, E_Desk *desk);
EAPI void      e_border_show(E_Border *bd);
EAPI void      e_border_hide(E_Border *bd, int manage);
EAPI void      e_border_move(E_Border *bd, int x, int y);
EAPI void      e_border_resize(E_Border *bd, int w, int h);
EAPI void      e_border_move_resize(E_Border *bd, int x, int y, int w, int h);
EAPI void      e_border_raise(E_Border *bd);
EAPI void      e_border_lower(E_Border *bd);
EAPI void      e_border_stack_above(E_Border *bd, E_Border *above);
EAPI void      e_border_stack_below(E_Border *bd, E_Border *below);
EAPI void      e_border_focus_set(E_Border *bd, int focus, int set);
EAPI void      e_border_shade(E_Border *bd, E_Direction dir);
EAPI void      e_border_unshade(E_Border *bd, E_Direction dir);
EAPI void      e_border_maximize(E_Border *bd);
EAPI void      e_border_unmaximize(E_Border *bd);
EAPI void      e_border_iconify(E_Border *bd);
EAPI void      e_border_uniconify(E_Border *bd);
EAPI void      e_border_stick(E_Border *bd);
EAPI void      e_border_unstick(E_Border *bd);

EAPI E_Border *e_border_find_by_client_window(Ecore_X_Window win);
EAPI E_Border *e_border_find_by_frame_window(Ecore_X_Window win);
EAPI E_Border *e_border_find_by_window(Ecore_X_Window win);
EAPI E_Border *e_border_focused_get(void);

EAPI void      e_border_idler_before(void);

EAPI Evas_List *e_border_clients_get();

extern EAPI int E_EVENT_BORDER_RESIZE;
extern EAPI int E_EVENT_BORDER_MOVE;
extern EAPI int E_EVENT_BORDER_ADD;
extern EAPI int E_EVENT_BORDER_SHOW;
extern EAPI int E_EVENT_BORDER_HIDE;
extern EAPI int E_EVENT_BORDER_REMOVE;
extern EAPI int E_EVENT_BORDER_ICONIFY;
extern EAPI int E_EVENT_BORDER_UNICONIFY;
extern EAPI int E_EVENT_BORDER_STICK;
extern EAPI int E_EVENT_BORDER_UNSTICK;
extern EAPI int E_EVENT_BORDER_ZONE_SET;
extern EAPI int E_EVENT_BORDER_DESK_SET;
extern EAPI int E_EVENT_BORDER_RAISE;
extern EAPI int E_EVENT_BORDER_LOWER;
extern EAPI int E_EVENT_BORDER_ICON_CHANGE;

#endif
#endif
