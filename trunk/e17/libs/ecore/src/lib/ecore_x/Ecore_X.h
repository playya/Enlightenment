/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifndef _ECORE_X_H
#define _ECORE_X_H

#ifdef EAPI
#undef EAPI
#endif
#ifdef WIN32
# ifdef BUILDING_DLL
#  define EAPI __declspec(dllexport)
# else
#  define EAPI __declspec(dllimport)
# endif
#else
# ifdef GCC_HASCLASSVISIBILITY
#  define EAPI __attribute__ ((visibility("default")))
# else
#  define EAPI
# endif
#endif

/**
 * @file
 * @brief Ecore functions for dealing with the X Windows System
 *
 * Ecore_X provides a wrapper and convenience functions for using the
 * X Windows System.  Function groups for this part of the library 
 * include the following:
 * @li @ref Ecore_X_Init_Group
 * @li @ref Ecore_X_Display_Attr_Group
 * @li @ref Ecore_X_Flush_Group
 */

typedef unsigned int Ecore_X_ID;
#ifndef _ECORE_X_WINDOW_PREDEF
typedef Ecore_X_ID   Ecore_X_Window;
#endif
typedef Ecore_X_ID   Ecore_X_Pixmap;
typedef Ecore_X_ID   Ecore_X_Drawable;
typedef void       * Ecore_X_GC;
typedef Ecore_X_ID   Ecore_X_Atom;
typedef Ecore_X_ID   Ecore_X_Colormap;
typedef Ecore_X_ID   Ecore_X_Time;
typedef Ecore_X_ID   Ecore_X_Cursor;
typedef void         Ecore_X_Display;

#ifdef __cplusplus
extern "C" {
#endif
   
typedef struct _Ecore_X_Rectangle {
   int x, y;
   unsigned int width, height;
} Ecore_X_Rectangle;

#define ECORE_X_SELECTION_TARGET_TEXT "TEXT"
#define ECORE_X_SELECTION_TARGET_COMPOUND_TEXT "COMPOUND_TEXT"
#define ECORE_X_SELECTION_TARGET_STRING "STRING"
#define ECORE_X_SELECTION_TARGET_UTF8_STRING "UTF8_STRING"
#define ECORE_X_SELECTION_TARGET_FILENAME "FILENAME"

#define ECORE_X_DND_VERSION 5

#define ECORE_X_CURRENT_TIME 0
   
extern EAPI Ecore_X_Atom ECORE_X_DND_ACTION_COPY;
extern EAPI Ecore_X_Atom ECORE_X_DND_ACTION_MOVE;
extern EAPI Ecore_X_Atom ECORE_X_DND_ACTION_LINK;
extern EAPI Ecore_X_Atom ECORE_X_DND_ACTION_ASK;
extern EAPI Ecore_X_Atom ECORE_X_DND_ACTION_PRIVATE;

typedef enum _Ecore_X_Selection {
   ECORE_X_SELECTION_PRIMARY,
   ECORE_X_SELECTION_SECONDARY,
   ECORE_X_SELECTION_CLIPBOARD
} Ecore_X_Selection;

typedef enum _Ecore_X_Event_Mode
{
   ECORE_X_EVENT_MODE_NORMAL,
   ECORE_X_EVENT_MODE_WHILE_GRABBED,
   ECORE_X_EVENT_MODE_GRAB,
   ECORE_X_EVENT_MODE_UNGRAB
} Ecore_X_Event_Mode;

typedef enum _Ecore_X_Event_Detail
{
   ECORE_X_EVENT_DETAIL_ANCESTOR,
   ECORE_X_EVENT_DETAIL_VIRTUAL,
   ECORE_X_EVENT_DETAIL_INFERIOR,
   ECORE_X_EVENT_DETAIL_NON_LINEAR,
   ECORE_X_EVENT_DETAIL_NON_LINEAR_VIRTUAL,
   ECORE_X_EVENT_DETAIL_POINTER,
   ECORE_X_EVENT_DETAIL_POINTER_ROOT,
   ECORE_X_EVENT_DETAIL_DETAIL_NONE
} Ecore_X_Event_Detail;

typedef enum _Ecore_X_Event_Mask
{
   ECORE_X_EVENT_MASK_NONE                   = 0L,
   ECORE_X_EVENT_MASK_KEY_DOWN               = (1L << 0),
   ECORE_X_EVENT_MASK_KEY_UP                 = (1L << 1),
   ECORE_X_EVENT_MASK_MOUSE_DOWN             = (1L << 2),
   ECORE_X_EVENT_MASK_MOUSE_UP               = (1L << 3),
   ECORE_X_EVENT_MASK_MOUSE_IN               = (1L << 4),
   ECORE_X_EVENT_MASK_MOUSE_OUT              = (1L << 5),
   ECORE_X_EVENT_MASK_MOUSE_MOVE             = (1L << 6),
   ECORE_X_EVENT_MASK_WINDOW_DAMAGE          = (1L << 15),
   ECORE_X_EVENT_MASK_WINDOW_VISIBILITY      = (1L << 16),
   ECORE_X_EVENT_MASK_WINDOW_CONFIGURE       = (1L << 17),
   ECORE_X_EVENT_MASK_WINDOW_RESIZE_MANAGE   = (1L << 18),
   ECORE_X_EVENT_MASK_WINDOW_MANAGE          = (1L << 19),
   ECORE_X_EVENT_MASK_WINDOW_CHILD_CONFIGURE = (1L << 20),
   ECORE_X_EVENT_MASK_WINDOW_FOCUS_CHANGE    = (1L << 21),
   ECORE_X_EVENT_MASK_WINDOW_PROPERTY        = (1L << 22),
   ECORE_X_EVENT_MASK_WINDOW_COLORMAP        = (1L << 23),
   ECORE_X_EVENT_MASK_WINDOW_GRAB            = (1L << 24),
   ECORE_X_EVENT_MASK_MOUSE_WHEEL            = (1L << 29),
   ECORE_X_EVENT_MASK_WINDOW_FOCUS_IN        = (1L << 30),
   ECORE_X_EVENT_MASK_WINDOW_FOCUS_OUT       = (1L << 31)
} Ecore_X_Event_Mask;

   typedef enum _Ecore_X_Gravity {
      ECORE_X_GRAVITY_FORGET = 0,
	ECORE_X_GRAVITY_UNMAP = 0,
	ECORE_X_GRAVITY_NW = 1,
	ECORE_X_GRAVITY_N = 2,
	ECORE_X_GRAVITY_NE = 3,
	ECORE_X_GRAVITY_W = 4,
	ECORE_X_GRAVITY_CENTER = 5,
	ECORE_X_GRAVITY_E = 6,
	ECORE_X_GRAVITY_SW = 7,
	ECORE_X_GRAVITY_S = 8,
	ECORE_X_GRAVITY_SE = 9,
	ECORE_X_GRAVITY_STATIC = 10
   } Ecore_X_Gravity;

typedef struct _Ecore_X_Event_Key_Down                 Ecore_X_Event_Key_Down;
typedef struct _Ecore_X_Event_Key_Up                   Ecore_X_Event_Key_Up;
typedef struct _Ecore_X_Event_Mouse_Button_Down        Ecore_X_Event_Mouse_Button_Down;
typedef struct _Ecore_X_Event_Mouse_Button_Up          Ecore_X_Event_Mouse_Button_Up;
typedef struct _Ecore_X_Event_Mouse_Move               Ecore_X_Event_Mouse_Move;
typedef struct _Ecore_X_Event_Mouse_In                 Ecore_X_Event_Mouse_In;
typedef struct _Ecore_X_Event_Mouse_Out                Ecore_X_Event_Mouse_Out;
typedef struct _Ecore_X_Event_Mouse_Wheel              Ecore_X_Event_Mouse_Wheel;
typedef struct _Ecore_X_Event_Window_Focus_In          Ecore_X_Event_Window_Focus_In;
typedef struct _Ecore_X_Event_Window_Focus_Out         Ecore_X_Event_Window_Focus_Out;
typedef struct _Ecore_X_Event_Window_Keymap            Ecore_X_Event_Window_Keymap;
typedef struct _Ecore_X_Event_Window_Damage            Ecore_X_Event_Window_Damage;
typedef struct _Ecore_X_Event_Window_Visibility_Change Ecore_X_Event_Window_Visibility_Change;
typedef struct _Ecore_X_Event_Window_Create            Ecore_X_Event_Window_Create;
typedef struct _Ecore_X_Event_Window_Destroy           Ecore_X_Event_Window_Destroy;
typedef struct _Ecore_X_Event_Window_Hide              Ecore_X_Event_Window_Hide;
typedef struct _Ecore_X_Event_Window_Show              Ecore_X_Event_Window_Show;
typedef struct _Ecore_X_Event_Window_Show_Request      Ecore_X_Event_Window_Show_Request;
typedef struct _Ecore_X_Event_Window_Reparent          Ecore_X_Event_Window_Reparent;
typedef struct _Ecore_X_Event_Window_Configure         Ecore_X_Event_Window_Configure;
typedef struct _Ecore_X_Event_Window_Configure_Request Ecore_X_Event_Window_Configure_Request;
typedef struct _Ecore_X_Event_Window_Gravity           Ecore_X_Event_Window_Gravity;
typedef struct _Ecore_X_Event_Window_Resize_Request    Ecore_X_Event_Window_Resize_Request;
typedef struct _Ecore_X_Event_Window_Stack             Ecore_X_Event_Window_Stack;
typedef struct _Ecore_X_Event_Window_Stack_Request     Ecore_X_Event_Window_Stack_Request;
typedef struct _Ecore_X_Event_Window_Property          Ecore_X_Event_Window_Property;
typedef struct _Ecore_X_Event_Window_Colormap          Ecore_X_Event_Window_Colormap;
typedef struct _Ecore_X_Event_Window_Mapping           Ecore_X_Event_Window_Mapping;
typedef struct _Ecore_X_Event_Selection_Clear          Ecore_X_Event_Selection_Clear;
typedef struct _Ecore_X_Event_Selection_Request        Ecore_X_Event_Selection_Request;
typedef struct _Ecore_X_Event_Selection_Notify         Ecore_X_Event_Selection_Notify;
typedef struct _Ecore_X_Event_Xdnd_Enter               Ecore_X_Event_Xdnd_Enter;
typedef struct _Ecore_X_Event_Xdnd_Position            Ecore_X_Event_Xdnd_Position;
typedef struct _Ecore_X_Event_Xdnd_Status              Ecore_X_Event_Xdnd_Status;
typedef struct _Ecore_X_Event_Xdnd_Leave               Ecore_X_Event_Xdnd_Leave;
typedef struct _Ecore_X_Event_Xdnd_Drop                Ecore_X_Event_Xdnd_Drop;
typedef struct _Ecore_X_Event_Xdnd_Finished            Ecore_X_Event_Xdnd_Finished;
typedef struct _Ecore_X_Event_Client_Message           Ecore_X_Event_Client_Message;
typedef struct _Ecore_X_Event_Window_Shape             Ecore_X_Event_Window_Shape;

typedef struct _Ecore_X_Event_Window_Delete_Request                Ecore_X_Event_Window_Delete_Request;
typedef struct _Ecore_X_Event_Window_Prop_Title_Change             Ecore_X_Event_Window_Prop_Title_Change;
typedef struct _Ecore_X_Event_Window_Prop_Visible_Title_Change     Ecore_X_Event_Window_Prop_Visible_Title_Change;
typedef struct _Ecore_X_Event_Window_Prop_Icon_Name_Change         Ecore_X_Event_Window_Prop_Icon_Name_Change;
typedef struct _Ecore_X_Event_Window_Prop_Visible_Icon_Name_Change Ecore_X_Event_Window_Prop_Visible_Icon_Name_Change;
typedef struct _Ecore_X_Event_Window_Prop_Client_Machine_Change      Ecore_X_Event_Window_Prop_Client_Machine_Change;
typedef struct _Ecore_X_Event_Window_Prop_Name_Class_Change        Ecore_X_Event_Window_Prop_Name_Class_Change;
typedef struct _Ecore_X_Event_Window_Prop_Pid_Change      Ecore_X_Event_Window_Prop_Pid_Change;
typedef struct _Ecore_X_Event_Window_Prop_Desktop_Change      Ecore_X_Event_Window_Prop_Desktop_Change;
     
struct _Ecore_X_Event_Key_Down
{
   char   *keyname;
   char   *keysymbol;
   char   *key_compose;
   int     modifiers;
   Ecore_X_Window  win;
   Ecore_X_Window  event_win;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Key_Up
{
   char   *keyname;
   char   *keysymbol;
   char   *key_compose;
   int     modifiers;
   Ecore_X_Window  win;
   Ecore_X_Window  event_win;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Mouse_Button_Down
{
   int     button;
   int     modifiers;
   int     x, y;
   struct {
      int  x, y;
   } root;
   Ecore_X_Window  win;
   Ecore_X_Window  event_win;
   Ecore_X_Time    time;
   int     double_click : 1;
   int     triple_click : 1;
};

struct _Ecore_X_Event_Mouse_Button_Up
{
   int     button;
   int     modifiers;
   int     x, y;
   struct {
      int  x, y;
   } root;
   Ecore_X_Window  win;
   Ecore_X_Window  event_win;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Mouse_Move
{
   int     modifiers;
   int     x, y;
   struct {
      int  x, y;
   } root;
   Ecore_X_Window  win;
   Ecore_X_Window  event_win;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Mouse_In
{
   int                  modifiers;
   int                  x, y;
   struct {
      int  x, y;
   } root;
   Ecore_X_Window               win;
   Ecore_X_Window               event_win;
   Ecore_X_Event_Mode   mode;
   Ecore_X_Event_Detail detail;
   Ecore_X_Time                 time;
};

struct _Ecore_X_Event_Mouse_Out
{
   int                  modifiers;
   int                  x, y;
   struct {
      int  x, y;
   } root;
   Ecore_X_Window               win;
   Ecore_X_Window               event_win;
   Ecore_X_Event_Mode   mode;
   Ecore_X_Event_Detail detail;
   Ecore_X_Time                 time;
};

struct _Ecore_X_Event_Mouse_Wheel
{
   int direction; /* 0 = default up/down wheel FIXME: more wheel types */
   int z; /* ...,-2,-1 = down, 1,2,... = up */
   int modifiers;
   int x, y;

   struct {
      int x, y;
   } root;

   Ecore_X_Window win;
   Ecore_X_Window event_win;
   Ecore_X_Time time;
};

struct _Ecore_X_Event_Window_Focus_In
{
   Ecore_X_Window               win;
   Ecore_X_Event_Mode   mode;
   Ecore_X_Event_Detail detail;
   Ecore_X_Time                 time;
};

struct _Ecore_X_Event_Window_Focus_Out
{
   Ecore_X_Window               win;
   Ecore_X_Event_Mode   mode;
   Ecore_X_Event_Detail detail;
   Ecore_X_Time                 time;
};

struct _Ecore_X_Event_Window_Keymap
{
   Ecore_X_Window  win;
};

struct _Ecore_X_Event_Window_Damage
{
   Ecore_X_Window  win;
   int     x, y, w, h;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Visibility_Change
{
   Ecore_X_Window  win;
   int             fully_obscured;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Create
{
   Ecore_X_Window  win;
   int             override;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Destroy
{
   Ecore_X_Window  win;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Hide
{
   Ecore_X_Window  win;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Show
{
   Ecore_X_Window  win;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Show_Request
{
   Ecore_X_Window  win;
   Ecore_X_Window  parent;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Reparent
{
   Ecore_X_Window  win;
   Ecore_X_Window  parent;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Configure
{
   Ecore_X_Window  win;
   Ecore_X_Window  abovewin;
   int     x, y, w, h;
   int     border;
   int     override : 1;
   int     from_wm : 1;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Configure_Request
{
   Ecore_X_Window  win;
   Ecore_X_Window  abovewin;
   int     x, y, w, h;
   int     border;
   int     detail;
   unsigned long value_mask;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Gravity
{
   Ecore_X_Window  win;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Resize_Request
{
   Ecore_X_Window  win;
   int     w, h;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Stack
{
   Ecore_X_Window  win;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Stack_Request
{
   Ecore_X_Window  win;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Property
{
   Ecore_X_Window  win;
   Ecore_X_Atom    atom;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Colormap
{
   Ecore_X_Window  win;
   Ecore_X_Colormap cmap;
   int             installed;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Selection_Clear
{
   Ecore_X_Window    win;
   Ecore_X_Selection selection;
   Ecore_X_Time      time;
};

struct _Ecore_X_Event_Selection_Request
{
   Ecore_X_Window  win;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Selection_Notify
{
   Ecore_X_Window             win;
   Ecore_X_Time               time;
   Ecore_X_Selection          selection;
   char                       *target;
};

struct _Ecore_X_Event_Xdnd_Enter
{
   Ecore_X_Window       win, source;
   Ecore_X_Time         time;
};

struct _Ecore_X_Event_Xdnd_Position
{
   Ecore_X_Window       win, source;
   struct {
      int x, y;
   } position;
   Ecore_X_Time         time;
   Ecore_X_Atom         action;
};

struct _Ecore_X_Event_Xdnd_Status
{
   Ecore_X_Window       win, target;
   int                  will_accept;
   Ecore_X_Rectangle    rectangle;
   Ecore_X_Atom         action;
};

struct _Ecore_X_Event_Xdnd_Leave
{
   Ecore_X_Window       win, source;
};

struct _Ecore_X_Event_Xdnd_Drop
{
   Ecore_X_Window       win, source;
   Ecore_X_Time         time;
   Ecore_X_Atom         action;
   struct {
      int x, y;
   } position;
};

struct _Ecore_X_Event_Xdnd_Finished
{
   Ecore_X_Window       win, target;
   int                  completed;
   Ecore_X_Atom         action;
};

struct _Ecore_X_Event_Client_Message
{
   Ecore_X_Window       win;
   Ecore_X_Atom         message_type;
   int                  format;
   union {
      char              b[20];
      short             s[10];
      long              l[5];
   }                    data;
   Ecore_X_Time         time;
};

struct _Ecore_X_Event_Window_Shape
{
   Ecore_X_Window  win;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Delete_Request
{
   Ecore_X_Window  win;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Prop_Title_Change
{
   Ecore_X_Window  win;
   char   *title;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Prop_Visible_Title_Change
{
   Ecore_X_Window  win;
   char   *title;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Prop_Icon_Name_Change
{
   Ecore_X_Window  win;
   char   *name;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Prop_Visible_Icon_Name_Change
{
   Ecore_X_Window  win;
   char   *name;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Prop_Client_Machine_Change
{
   Ecore_X_Window  win;
   char   *name;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Prop_Name_Class_Change
{
   Ecore_X_Window  win;
   char   *name;
   char   *clas;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Prop_Pid_Change
{
   Ecore_X_Window  win;
   pid_t   pid;
   Ecore_X_Time    time;
};

struct _Ecore_X_Event_Window_Prop_Desktop_Change
{
   Ecore_X_Window  win;
   long    desktop;
   Ecore_X_Time    time;
};

extern EAPI int ECORE_X_EVENT_KEY_DOWN;
extern EAPI int ECORE_X_EVENT_KEY_UP;
extern EAPI int ECORE_X_EVENT_MOUSE_BUTTON_DOWN;
extern EAPI int ECORE_X_EVENT_MOUSE_BUTTON_UP;
extern EAPI int ECORE_X_EVENT_MOUSE_MOVE;
extern EAPI int ECORE_X_EVENT_MOUSE_IN;
extern EAPI int ECORE_X_EVENT_MOUSE_OUT;
extern EAPI int ECORE_X_EVENT_MOUSE_WHEEL;
extern EAPI int ECORE_X_EVENT_WINDOW_FOCUS_IN;
extern EAPI int ECORE_X_EVENT_WINDOW_FOCUS_OUT;
extern EAPI int ECORE_X_EVENT_WINDOW_KEYMAP;
extern EAPI int ECORE_X_EVENT_WINDOW_DAMAGE;
extern EAPI int ECORE_X_EVENT_WINDOW_VISIBILITY_CHANGE;
extern EAPI int ECORE_X_EVENT_WINDOW_CREATE;
extern EAPI int ECORE_X_EVENT_WINDOW_DESTROY;
extern EAPI int ECORE_X_EVENT_WINDOW_HIDE;
extern EAPI int ECORE_X_EVENT_WINDOW_SHOW;
extern EAPI int ECORE_X_EVENT_WINDOW_SHOW_REQUEST;
extern EAPI int ECORE_X_EVENT_WINDOW_REPARENT;
extern EAPI int ECORE_X_EVENT_WINDOW_CONFIGURE;
extern EAPI int ECORE_X_EVENT_WINDOW_CONFIGURE_REQUEST;
extern EAPI int ECORE_X_EVENT_WINDOW_GRAVITY;
extern EAPI int ECORE_X_EVENT_WINDOW_RESIZE_REQUEST;
extern EAPI int ECORE_X_EVENT_WINDOW_STACK;
extern EAPI int ECORE_X_EVENT_WINDOW_STACK_REQUEST;
extern EAPI int ECORE_X_EVENT_WINDOW_PROPERTY;
extern EAPI int ECORE_X_EVENT_WINDOW_COLORMAP;
extern EAPI int ECORE_X_EVENT_WINDOW_MAPPING;
extern EAPI int ECORE_X_EVENT_SELECTION_CLEAR;
extern EAPI int ECORE_X_EVENT_SELECTION_REQUEST;
extern EAPI int ECORE_X_EVENT_SELECTION_NOTIFY;
extern EAPI int ECORE_X_EVENT_CLIENT_MESSAGE;
extern EAPI int ECORE_X_EVENT_WINDOW_SHAPE;

extern EAPI int ECORE_X_EVENT_WINDOW_DELETE_REQUEST;
extern EAPI int ECORE_X_EVENT_WINDOW_PROP_TITLE_CHANGE;
extern EAPI int ECORE_X_EVENT_WINDOW_PROP_VISIBLE_TITLE_CHANGE;
extern EAPI int ECORE_X_EVENT_WINDOW_PROP_ICON_NAME_CHANGE;
extern EAPI int ECORE_X_EVENT_WINDOW_PROP_VISIBLE_ICON_NAME_CHANGE;
extern EAPI int ECORE_X_EVENT_WINDOW_PROP_CLIENT_MACHINE_CHANGE;
extern EAPI int ECORE_X_EVENT_WINDOW_PROP_NAME_CLASS_CHANGE;
extern EAPI int ECORE_X_EVENT_WINDOW_PROP_PID_CHANGE;
extern EAPI int ECORE_X_EVENT_WINDOW_PROP_DESKTOP_CHANGE;

extern EAPI int ECORE_X_EVENT_XDND_ENTER;
extern EAPI int ECORE_X_EVENT_XDND_POSITION;
extern EAPI int ECORE_X_EVENT_XDND_STATUS;
extern EAPI int ECORE_X_EVENT_XDND_LEAVE;
extern EAPI int ECORE_X_EVENT_XDND_DROP;
extern EAPI int ECORE_X_EVENT_XDND_FINISHED;
   
extern EAPI int ECORE_X_MODIFIER_SHIFT;
extern EAPI int ECORE_X_MODIFIER_CTRL;
extern EAPI int ECORE_X_MODIFIER_ALT;
extern EAPI int ECORE_X_MODIFIER_WIN;

extern EAPI int ECORE_X_LOCK_SCROLL;
extern EAPI int ECORE_X_LOCK_NUM;
extern EAPI int ECORE_X_LOCK_CAPS;

#ifndef _ECORE_X_PRIVATE_H
typedef enum _Ecore_X_WM_Protocol {
	/**
	 * If enabled the window manager will be asked to send a
	 * delete message instead of just closing (destroying) the window.
	 */
	ECORE_X_WM_PROTOCOL_DELETE_REQUEST,

	/**
	 * If enabled the window manager will be told that the window
	 * explicitly sets input focus.
	 */
	ECORE_X_WM_PROTOCOL_TAKE_FOCUS
} Ecore_X_WM_Protocol;
#endif

typedef enum _Ecore_X_Window_Input_Mode {
	/** The window can never be focused */
	ECORE_X_WINDOW_INPUT_MODE_NONE,
	
	/** The window can be focused by the WM but doesn't focus itself */
	ECORE_X_WINDOW_INPUT_MODE_PASSIVE,

	/** The window sets the focus itself if one of its sub-windows
	 * already is focused
	 */
	ECORE_X_WINDOW_INPUT_MODE_ACTIVE_LOCAL,

	/** The window sets the focus itself even if another window
	 * is currently focused
	 */
	ECORE_X_WINDOW_INPUT_MODE_ACTIVE_GLOBAL
} Ecore_X_Window_Input_Mode;

typedef enum _Ecore_X_Window_State_Hint {
   /** Do not provide any state hint to the window manager */
   ECORE_X_WINDOW_STATE_HINT_NONE = -1,
   
   /** The window wants to remain hidden and NOT iconified */
   ECORE_X_WINDOW_STATE_HINT_WITHDRAWN,
   
   /** The window wants to be mapped normally */
   ECORE_X_WINDOW_STATE_HINT_NORMAL,
   
   /** The window wants to start in an iconified state */
   ECORE_X_WINDOW_STATE_HINT_ICONIC,
} Ecore_X_Window_State_Hint;

typedef enum _Ecore_X_Window_State {
    /** The window is iconified. */
    ECORE_X_WINDOW_STATE_ICONIFIED,

    /** The window is a modal dialog box. */
    ECORE_X_WINDOW_STATE_MODAL,

    /** The window manager should keep the window's position fixed
     * even if the virtual desktop scrolls. */
    ECORE_X_WINDOW_STATE_STICKY,

    /** The window has the maximum vertical size. */
    ECORE_X_WINDOW_STATE_MAXIMIZED_VERT,

    /** The window has the maximum horizontal size. */
    ECORE_X_WINDOW_STATE_MAXIMIZED_HORZ,

    /** The window is shaded. */
    ECORE_X_WINDOW_STATE_SHADED,

    /** The window should not be included in the taskbar. */
    ECORE_X_WINDOW_STATE_SKIP_TASKBAR,

    /** The window should not be included in the pager. */
    ECORE_X_WINDOW_STATE_SKIP_PAGER,

    /** The window is invisible (i.e. minimized/iconified) */
    ECORE_X_WINDOW_STATE_HIDDEN,

    /** The window should fill the entire screen and have no
     * window border/decorations */
    ECORE_X_WINDOW_STATE_FULLSCREEN,

    /* The following are not documented because they are not
     * intended for use in applications. */
    ECORE_X_WINDOW_STATE_ABOVE,
    ECORE_X_WINDOW_STATE_BELOW

} Ecore_X_Window_State;

typedef enum _Ecore_X_Window_Type {
    ECORE_X_WINDOW_TYPE_DESKTOP,
    ECORE_X_WINDOW_TYPE_DOCK,
    ECORE_X_WINDOW_TYPE_TOOLBAR,
    ECORE_X_WINDOW_TYPE_MENU,
    ECORE_X_WINDOW_TYPE_UTILITY,
    ECORE_X_WINDOW_TYPE_SPLASH,
    ECORE_X_WINDOW_TYPE_DIALOG,
    ECORE_X_WINDOW_TYPE_NORMAL
} Ecore_X_Window_Type;

typedef enum _Ecore_X_Window_Configure_Mask {
   ECORE_X_WINDOW_CONFIGURE_MASK_X              = (1 << 0),
   ECORE_X_WINDOW_CONFIGURE_MASK_Y              = (1 << 1),
   ECORE_X_WINDOW_CONFIGURE_MASK_W              = (1 << 2),
   ECORE_X_WINDOW_CONFIGURE_MASK_H              = (1 << 3),
   ECORE_X_WINDOW_CONFIGURE_MASK_BORDER_WIDTH   = (1 << 4),
   ECORE_X_WINDOW_CONFIGURE_MASK_SIBLING        = (1 << 5),
   ECORE_X_WINDOW_CONFIGURE_MASK_STACK_MODE     = (1 << 6)
} Ecore_X_Window_Configure_Mask;

typedef enum _Ecore_X_Window_Stack_Mode {
   ECORE_X_WINDOW_STACK_ABOVE = 0,
   ECORE_X_WINDOW_STACK_BELOW = 1,
   ECORE_X_WINDOW_STACK_TOP_IF = 2,
   ECORE_X_WINDOW_STACK_BOTTOM_IF = 3,
   ECORE_X_WINDOW_STACK_OPPOSITE = 4
} Ecore_X_Window_Stack_Mode;

/* Window layer constants */
#define ECORE_X_WINDOW_LAYER_BELOW 2
#define ECORE_X_WINDOW_LAYER_NORMAL 4
#define ECORE_X_WINDOW_LAYER_ABOVE 6

EAPI int              ecore_x_init(const char *name);
EAPI int              ecore_x_shutdown(void);       
EAPI int              ecore_x_disconnect(void);       
EAPI Ecore_X_Display *ecore_x_display_get(void);
EAPI int              ecore_x_fd_get(void);
EAPI void             ecore_x_double_click_time_set(double t);
EAPI double           ecore_x_double_click_time_get(void);
EAPI void             ecore_x_flush(void);
EAPI void             ecore_x_sync(void);
EAPI void             ecore_x_killall(Ecore_X_Window root);
       
EAPI void             ecore_x_error_handler_set(void (*func) (void *data), const void *data);
EAPI void             ecore_x_io_error_handler_set(void (*func) (void *data), const void *data);
EAPI int              ecore_x_error_request_get(void);
EAPI int              ecore_x_error_code_get(void);

EAPI void             ecore_x_event_mask_set(Ecore_X_Window w, Ecore_X_Event_Mask mask);
EAPI void             ecore_x_event_mask_unset(Ecore_X_Window w, Ecore_X_Event_Mask mask);

EAPI int              ecore_x_selection_primary_set(Ecore_X_Window w, unsigned char *data, int size);
EAPI int              ecore_x_selection_primary_clear(void);
EAPI int              ecore_x_selection_secondary_set(Ecore_X_Window w, unsigned char *data, int size);
EAPI int              ecore_x_selection_secondary_clear(void);
EAPI int              ecore_x_selection_clipboard_set(Ecore_X_Window w, unsigned char *data, int size);
EAPI int              ecore_x_selection_clipboard_clear(void);
EAPI void             ecore_x_selection_primary_request(Ecore_X_Window w, char *target);
EAPI void             ecore_x_selection_secondary_request(Ecore_X_Window w, char *target);
EAPI void             ecore_x_selection_clipboard_request(Ecore_X_Window w, char *target);
EAPI void             ecore_x_selection_primary_request_data_get(void **buf, int *len);
EAPI void             ecore_x_selection_secondary_request_data_get(void **buf, int *len);
EAPI void             ecore_x_selection_clipboard_request_data_get(void **buf, int *len);
EAPI void             ecore_x_selection_converter_add(char *target, int (*func)(char *target, void *data, int size, void **data_ret, int *size_ret));
EAPI void             ecore_x_selection_converter_atom_add(Ecore_X_Atom target, int (*func)(char *target, void *data, int size, void **data_ret, int *size_ret));
EAPI void             ecore_x_selection_converter_del(char *target);
EAPI void             ecore_x_selection_converter_atom_del(Ecore_X_Atom target);

EAPI void             ecore_x_dnd_aware_set(Ecore_X_Window win, int on);
EAPI int              ecore_x_dnd_version_get(Ecore_X_Window win);
EAPI int              ecore_x_dnd_begin (Ecore_X_Window source, unsigned char *data, int size);
EAPI void             ecore_x_dnd_send_status(int will_accept, int suppress, Ecore_X_Rectangle rectangle, Ecore_X_Atom action);
                 
EAPI Ecore_X_Window   ecore_x_window_new(Ecore_X_Window parent, int x, int y, int w, int h);
EAPI Ecore_X_Window   ecore_x_window_override_new(Ecore_X_Window parent, int x, int y, int w, int h);
EAPI Ecore_X_Window   ecore_x_window_input_new(Ecore_X_Window parent, int x, int y, int w, int h);
EAPI void             ecore_x_window_configure(Ecore_X_Window win,
                                          Ecore_X_Window_Configure_Mask mask,
                                          int x, int y, int w, int h,
                                          int border_width,
                                          Ecore_X_Window sibling,
                                          int stack_mode);
EAPI void             ecore_x_window_cursor_set(Ecore_X_Window win,
                                           Ecore_X_Cursor c);
EAPI void             ecore_x_window_del(Ecore_X_Window win);
EAPI void             ecore_x_window_delete_request_send(Ecore_X_Window win);
EAPI void             ecore_x_window_show(Ecore_X_Window win);
EAPI void             ecore_x_window_hide(Ecore_X_Window win);
EAPI void             ecore_x_window_move(Ecore_X_Window win, int x, int y);
EAPI void             ecore_x_window_resize(Ecore_X_Window win, int w, int h);
EAPI void             ecore_x_window_move_resize(Ecore_X_Window win, int x, int y, int w, int h);
EAPI void             ecore_x_window_focus(Ecore_X_Window win);
EAPI void             ecore_x_window_focus_at_time(Ecore_X_Window win, Ecore_X_Time t);
EAPI Ecore_X_Window   ecore_x_window_focus_get(void);
EAPI void             ecore_x_window_raise(Ecore_X_Window win);
EAPI void             ecore_x_window_lower(Ecore_X_Window win);
EAPI void             ecore_x_window_reparent(Ecore_X_Window win, Ecore_X_Window new_parent, int x, int y);
EAPI void             ecore_x_window_size_get(Ecore_X_Window win, int *w, int *h);
EAPI void             ecore_x_window_geometry_get(Ecore_X_Window win, int *x, int *y, int *w, int *h);
EAPI int              ecore_x_window_border_width_get(Ecore_X_Window win);
EAPI void             ecore_x_window_border_width_set(Ecore_X_Window win, int width);
EAPI int              ecore_x_window_depth_get(Ecore_X_Window win);
EAPI void             ecore_x_window_cursor_show(Ecore_X_Window win, int show);
EAPI void             ecore_x_window_defaults_set(Ecore_X_Window win);
EAPI int              ecore_x_window_visible_get(Ecore_X_Window win);
EAPI Ecore_X_Window   ecore_x_window_at_xy_get(int x, int y);
EAPI Ecore_X_Window   ecore_x_window_parent_get(Ecore_X_Window win);

EAPI void             ecore_x_window_background_color_set(Ecore_X_Window win,
                                                     unsigned long color);
EAPI void             ecore_x_window_gravity_set(Ecore_X_Window win,
					    Ecore_X_Gravity grav);
EAPI void             ecore_x_window_pixel_gravity_set(Ecore_X_Window win,
						  Ecore_X_Gravity grav);
       
EAPI Ecore_X_Atom     ecore_x_window_prop_any_type(void);
EAPI void             ecore_x_window_prop_property_set(Ecore_X_Window win, Ecore_X_Atom type, Ecore_X_Atom format, int size, void *data, int number);
EAPI int              ecore_x_window_prop_property_get(Ecore_X_Window win, Ecore_X_Atom property, Ecore_X_Atom type, int size, unsigned char **data, int *num);
EAPI void             ecore_x_window_prop_property_del(Ecore_X_Window win, Ecore_X_Atom property);
EAPI void             ecore_x_window_prop_property_notify(Ecore_X_Window win, const char *type, long *data);
EAPI void             ecore_x_window_prop_string_set(Ecore_X_Window win, Ecore_X_Atom type, char *str);
EAPI char            *ecore_x_window_prop_string_get(Ecore_X_Window win, Ecore_X_Atom type);
EAPI void             ecore_x_window_prop_title_set(Ecore_X_Window win, const char *t);
EAPI char            *ecore_x_window_prop_title_get(Ecore_X_Window win);
EAPI void             ecore_x_window_prop_command_set(Ecore_X_Window win, int argc, char **argv);
EAPI void             ecore_x_window_prop_command_get(Ecore_X_Window win, int *argc, char ***argv);
EAPI void             ecore_x_window_prop_visible_title_set(Ecore_X_Window win, const char *t);
EAPI char            *ecore_x_window_prop_visible_title_get(Ecore_X_Window win);
EAPI void             ecore_x_window_prop_icon_name_set(Ecore_X_Window win, const char *t);
EAPI char            *ecore_x_window_prop_icon_name_get(Ecore_X_Window win);
EAPI void             ecore_x_window_prop_visible_icon_name_set(Ecore_X_Window win, const char *t);
EAPI char            *ecore_x_window_prop_visible_icon_name_get(Ecore_X_Window win);
EAPI char            *ecore_x_window_prop_client_machine_get(Ecore_X_Window win);
EAPI pid_t            ecore_x_window_prop_pid_get(Ecore_X_Window win);
EAPI void             ecore_x_window_prop_name_class_set(Ecore_X_Window win, const char *n, const char *c);
EAPI void             ecore_x_window_prop_name_class_get(Ecore_X_Window win, char **n, char **c);
EAPI void             ecore_x_window_prop_protocol_set(Ecore_X_Window win, Ecore_X_WM_Protocol protocol, int on);
EAPI int              ecore_x_window_prop_protocol_isset(Ecore_X_Window win, Ecore_X_WM_Protocol protocol);
EAPI Ecore_X_WM_Protocol *ecore_x_window_prop_protocol_list_get(Ecore_X_Window win, int *num_ret);
EAPI void             ecore_x_window_prop_sticky_set(Ecore_X_Window win, int on);
EAPI int              ecore_x_window_prop_input_mode_set(Ecore_X_Window win, Ecore_X_Window_Input_Mode mode);
EAPI int              ecore_x_window_prop_initial_state_set(Ecore_X_Window win, Ecore_X_Window_State_Hint state);
EAPI void             ecore_x_window_prop_min_size_set(Ecore_X_Window win, int w, int h);
EAPI void             ecore_x_window_prop_max_size_set(Ecore_X_Window win, int w, int h);
EAPI void             ecore_x_window_prop_base_size_set(Ecore_X_Window win, int w, int h);
EAPI void             ecore_x_window_prop_step_size_set(Ecore_X_Window win, int x, int y);
EAPI void             ecore_x_window_prop_xy_set(Ecore_X_Window win, int x, int y);
EAPI void             ecore_x_window_prop_borderless_set(Ecore_X_Window win, int borderless);
EAPI int              ecore_x_window_prop_borderless_get(Ecore_X_Window win);
EAPI int              ecore_x_window_prop_layer_set(Ecore_X_Window win, int layer);
EAPI void             ecore_x_window_prop_withdrawn_set(Ecore_X_Window win, int withdrawn);
EAPI void             ecore_x_window_prop_desktop_request(Ecore_X_Window win, long desktop);
EAPI void             ecore_x_window_prop_state_request(Ecore_X_Window win, Ecore_X_Window_State state, int action);
EAPI void             ecore_x_window_prop_desktop_set(Ecore_X_Window win, long desktop);
EAPI long             ecore_x_window_prop_desktop_get(Ecore_X_Window win);
/* API Change: use enum Ecore_X_Window_Type instead */
EAPI void             ecore_x_window_prop_window_type_set(Ecore_X_Window win, Ecore_X_Window_Type type);
#if 0
EAPI void             ecore_x_window_prop_window_type_set(Ecore_X_Window win, Ecore_X_Atom type);
EAPI void             ecore_x_window_prop_window_type_desktop_set(Ecore_X_Window win);
EAPI void             ecore_x_window_prop_window_type_dock_set(Ecore_X_Window win);
EAPI void             ecore_x_window_prop_window_type_toolbar_set(Ecore_X_Window win);
EAPI void             ecore_x_window_prop_window_type_menu_set(Ecore_X_Window win);
EAPI void             ecore_x_window_prop_window_type_utility_set(Ecore_X_Window win);
EAPI void             ecore_x_window_prop_window_type_splash_set(Ecore_X_Window win);
EAPI void             ecore_x_window_prop_window_type_dialog_set(Ecore_X_Window win);
EAPI void             ecore_x_window_prop_window_type_normal_set(Ecore_X_Window win);
#endif
EAPI void             ecore_x_window_prop_window_opacity_set(Ecore_X_Window win, int opacity);
EAPI int              ecore_x_window_prop_window_opacity_get(Ecore_X_Window win);
EAPI void             ecore_x_window_prop_state_set(Ecore_X_Window win, Ecore_X_Window_State s);
EAPI int              ecore_x_window_prop_state_isset(Ecore_X_Window win, Ecore_X_Window_State s);
EAPI void             ecore_x_window_prop_state_unset(Ecore_X_Window win, Ecore_X_Window_State s);
EAPI void             ecore_x_window_shape_mask_set(Ecore_X_Window win, Ecore_X_Pixmap mask);
       
EAPI Ecore_X_Pixmap   ecore_x_pixmap_new(Ecore_X_Window win, int w, int h, int dep);
EAPI void             ecore_x_pixmap_del(Ecore_X_Pixmap pmap);
EAPI void             ecore_x_pixmap_paste(Ecore_X_Pixmap pmap, Ecore_X_Drawable dest, Ecore_X_GC gc, int sx, int sy, int w, int h, int dx, int dy);
EAPI void             ecore_x_pixmap_geometry_get(Ecore_X_Pixmap pmap, int *x, int *y, int *w, int *h);
EAPI int              ecore_x_pixmap_depth_get(Ecore_X_Pixmap pmap);

EAPI Ecore_X_GC       ecore_x_gc_new(Ecore_X_Drawable draw);
EAPI void             ecore_x_gc_del(Ecore_X_GC gc);

EAPI int              ecore_x_client_message32_send(Ecore_X_Window win, Ecore_X_Atom type, long d0, long d1, long d2, long d3, long d4);
EAPI int              ecore_x_client_message8_send(Ecore_X_Window win, Ecore_X_Atom type, const void *data, int len);

   
   /* FIXME: these funcs need categorising */
   EAPI void            ecore_x_drawable_geometry_get(Ecore_X_Drawable d, int *x, int *y, int *w, int *h);
   EAPI int             ecore_x_drawable_border_width_get(Ecore_X_Drawable d);
   EAPI int             ecore_x_drawable_depth_get(Ecore_X_Drawable d);
   EAPI Ecore_X_Window *ecore_x_window_root_list(int *num_ret);
   EAPI int             ecore_x_window_manage(Ecore_X_Window win);
   EAPI void            ecore_x_window_container_manage(Ecore_X_Window win);
   EAPI void            ecore_x_window_client_manage(Ecore_X_Window win);
   EAPI void            ecore_x_window_sniff(Ecore_X_Window win);
   EAPI void            ecore_x_window_client_sniff(Ecore_X_Window win);
   EAPI Ecore_X_Atom    ecore_x_atom_get(const char *name);

   EAPI void
     ecore_x_icccm_state_set(Ecore_X_Window win, Ecore_X_Window_State_Hint state);
   EAPI void
     ecore_x_icccm_delete_window_send(Ecore_X_Window win, Ecore_X_Time t);
   EAPI void
     ecore_x_icccm_take_focus_send(Ecore_X_Window win, Ecore_X_Time t);
   EAPI void
     ecore_x_icccm_save_yourself_send(Ecore_X_Window win, Ecore_X_Time t);
   EAPI void
     ecore_x_icccm_move_resize_send(Ecore_X_Window win, 
				    int x, int y, int w, int h);
   EAPI void
     ecore_x_icccm_hints_set(Ecore_X_Window win,
			     int accepts_focus,
			     Ecore_X_Window_State_Hint initial_state,
			     Ecore_X_Pixmap icon_pixmap,
			     Ecore_X_Pixmap icon_mask,
			     Ecore_X_Window icon_window,
			     Ecore_X_Window window_group,
			     int is_urgent);
   EAPI int
     ecore_x_icccm_hints_get(Ecore_X_Window win,
			     int *accepts_focus,
			     Ecore_X_Window_State_Hint *initial_state,
			     Ecore_X_Pixmap *icon_pixmap,
			     Ecore_X_Pixmap *icon_mask,
			     Ecore_X_Window *icon_window,
			     Ecore_X_Window *window_group,
			     int *is_urgent);
   EAPI void
     ecore_x_icccm_size_pos_hints_set(Ecore_X_Window win,
				      int request_pos,
				      Ecore_X_Gravity gravity,
				      int min_w, int min_h,
				      int max_w, int max_h,
				      int base_w, int base_h,
				      int step_x, int step_y,
				      double min_aspect,
				      double max_aspect);
       
   EAPI int
     ecore_x_icccm_size_pos_hints_get(Ecore_X_Window win,
				      int *request_pos,
				      Ecore_X_Gravity *gravity,
				      int *min_w, int *min_h,
				      int *max_w, int *max_h,
				      int *base_w, int *base_h,
				      int *step_x, int *step_y,
				      double *min_aspect,
				      double *max_aspect);
   EAPI void
     ecore_x_icccm_title_set(Ecore_X_Window win, const char *t);
   EAPI char *
     ecore_x_icccm_title_get(Ecore_X_Window win);
   EAPI void
     ecore_x_icccm_protocol_set(Ecore_X_Window win,
                                Ecore_X_WM_Protocol protocol,
                                int on);
   EAPI int
     ecore_x_icccm_protocol_isset(Ecore_X_Window win,
                                  Ecore_X_WM_Protocol protocol);
   EAPI void
     ecore_x_icccm_name_class_set(Ecore_X_Window win,
                                  const char *n,
                                  const char *c);
   EAPI char *
     ecore_x_icccm_client_machine_get(Ecore_X_Window win);
   EAPI void
     ecore_x_icccm_command_set(Ecore_X_Window win, int argc, char **argv);
   EAPI void
     ecore_x_icccm_command_get(Ecore_X_Window win, int *argc, char ***argv);
   EAPI char *
     ecore_x_icccm_icon_name_get(Ecore_X_Window win);
   EAPI void
     ecore_x_icccm_icon_name_set(Ecore_X_Window win, const char *t);
   EAPI void
     ecore_x_icccm_colormap_window_set(Ecore_X_Window win, Ecore_X_Window subwin);
   EAPI void
     ecore_x_icccm_colormap_window_unset(Ecore_X_Window win, Ecore_X_Window subwin);
   EAPI void
     ecore_x_icccm_transient_for_set(Ecore_X_Window win, Ecore_X_Window forwin);
   EAPI void
     ecore_x_icccm_transient_for_unset(Ecore_X_Window win);
   EAPI Ecore_X_Window
     ecore_x_icccm_transient_for_get(Ecore_X_Window win);
   EAPI void
     ecore_x_icccm_window_role_set(Ecore_X_Window win, const char *role);
   EAPI char *
     ecore_x_icccm_window_role_get(Ecore_X_Window win);
   EAPI void
     ecore_x_icccm_client_leader_set(Ecore_X_Window win, Ecore_X_Window l);
   EAPI Ecore_X_Window
     ecore_x_icccm_client_leader_get(Ecore_X_Window win);

   
   typedef enum _Ecore_X_MWM_Hint_Func
     {
	ECORE_X_MWM_HINT_FUNC_ALL = (1 << 0),
	ECORE_X_MWM_HINT_FUNC_RESIZE = (1 << 1),
	ECORE_X_MWM_HINT_FUNC_MOVE = (1 << 2),
	ECORE_X_MWM_HINT_FUNC_MINIMIZE = (1 << 3),
	ECORE_X_MWM_HINT_FUNC_MAXIMIZE = (1 << 4),
	ECORE_X_MWM_HINT_FUNC_CLOSE = (1 << 5)
     }
   Ecore_X_MWM_Hint_Func;
   
   typedef enum _Ecore_X_MWM_Hint_Decor
     {
	ECORE_X_MWM_HINT_DECOR_ALL = (1 << 0),
	ECORE_X_MWM_HINT_DECOR_BORDER = (1 << 1),
	ECORE_X_MWM_HINT_DECOR_RESIZEH = (1 << 2),
	ECORE_X_MWM_HINT_DECOR_TITLE = (1 << 3),
	ECORE_X_MWM_HINT_DECOR_MENU = (1 << 4),
	ECORE_X_MWM_HINT_DECOR_MINIMIZE = (1 << 5),
	ECORE_X_MWM_HINT_DECOR_MAXIMIZE = (1 << 6)
     }
   Ecore_X_MWM_Hint_Decor;
   
   typedef enum _Ecore_X_MWM_Hint_Input
     {
	ECORE_X_MWM_HINT_INPUT_MODELESS = 0,
	ECORE_X_MWM_HINT_INPUT_PRIMARY_APPLICATION_MODAL = 1,
	ECORE_X_MWM_HINT_INPUT_SYSTEM_MODAL = 2,
	ECORE_X_MWM_HINT_INPUT_FULL_APPLICATION_MODAL = 3,
     }
   Ecore_X_MWM_Hint_Input;
   
   EAPI int
     ecore_x_mwm_hints_get(Ecore_X_Window win,
			   Ecore_X_MWM_Hint_Func *fhint,
			   Ecore_X_MWM_Hint_Decor *dhint,
			   Ecore_X_MWM_Hint_Input *ihint);
       
   EAPI void                ecore_x_netwm_init(void);
   EAPI void                ecore_x_netwm_wm_identify(Ecore_X_Window root, Ecore_X_Window check, const char *wm_name);
   EAPI void                ecore_x_netwm_desk_count_set(Ecore_X_Window root, int n_desks);
   EAPI void                ecore_x_netwm_desk_roots_set(Ecore_X_Window root, int n_desks, Ecore_X_Window * vroots);
   EAPI void                ecore_x_netwm_desk_names_set(Ecore_X_Window root, int n_desks, const char **names);
   EAPI void                ecore_x_netwm_desk_size_set(Ecore_X_Window root, int width, int height);
   EAPI void                ecore_x_netwm_desk_workareas_set(Ecore_X_Window root, int n_desks, int *areas);
   EAPI void                ecore_x_netwm_desk_current_set(Ecore_X_Window root, int desk);
   EAPI void                ecore_x_netwm_desk_viewports_set(Ecore_X_Window root, int n_desks, int *origins);
   EAPI void                ecore_x_netwm_showing_desktop_set(Ecore_X_Window root, int on);
   EAPI void                ecore_x_netwm_client_list_set(Ecore_X_Window root, int n_clients, Ecore_X_Window * p_clients);
   EAPI void                ecore_x_netwm_client_list_stacking_set(Ecore_X_Window root, int n_clients, Ecore_X_Window * p_clients);
   EAPI void                ecore_x_netwm_client_active_set(Ecore_X_Window root, Ecore_X_Window win);
   EAPI void                ecore_x_netwm_name_set(Ecore_X_Window win, const char *name);
   EAPI char               *ecore_x_netwm_name_get(Ecore_X_Window win);
   EAPI void                ecore_x_netwm_visible_name_set(Ecore_X_Window win, const char *name);
   EAPI char               *ecore_x_netwm_visible_name_get(Ecore_X_Window win);
   EAPI void                ecore_x_netwm_icon_name_set(Ecore_X_Window win, const char *name);
   EAPI char               *ecore_x_netwm_icon_name_get(Ecore_X_Window win);
   EAPI void                ecore_x_netwm_visible_icon_name_set(Ecore_X_Window win, const char *name);
   EAPI char               *ecore_x_netwm_visible_icon_name_get(Ecore_X_Window win);
   
   /* FIXME: these funcs need categorising */
   EAPI void            ecore_x_drawable_geometry_get(Ecore_X_Drawable d, int *x, int *y, int *w, int *h);
   EAPI int             ecore_x_drawable_border_width_get(Ecore_X_Drawable d);
   EAPI int             ecore_x_drawable_depth_get(Ecore_X_Drawable d);
   EAPI Ecore_X_Window *ecore_x_window_root_list(int *num_ret);
   EAPI int             ecore_x_window_manage(Ecore_X_Window win);
   EAPI void            ecore_x_window_container_manage(Ecore_X_Window win);
   EAPI void            ecore_x_window_client_manage(Ecore_X_Window win);
   EAPI void            ecore_x_window_sniff(Ecore_X_Window win);
   EAPI void            ecore_x_window_client_sniff(Ecore_X_Window win);
   EAPI Ecore_X_Atom    ecore_x_atom_get(const char *name);

   typedef struct _Ecore_X_Window_Attributes
     {
	Ecore_X_Window     root;
	int                x, y, w, h;
	int                border;
	int                depth;
	char               visible : 1;
	char               viewable : 1;
	char               override : 1;
	char               input_only : 1;
	char               save_under : 1;
	struct {
	   Ecore_X_Event_Mask mine;
	   Ecore_X_Event_Mask all;
	   Ecore_X_Event_Mask no_propagate;
	} event_mask;
	Ecore_X_Gravity    window_gravity;
	Ecore_X_Gravity    pixel_gravity;
	Ecore_X_Colormap   colormap;
	/* FIXME: missing
	 * Colormap comormap;
	 * int map_installed;
	 * Screen *screen;
	 * Visual *visual;
	 */
     }
   Ecore_X_Window_Attributes;
   
   EAPI int
     ecore_x_window_attributes_get(Ecore_X_Window win, Ecore_X_Window_Attributes *att_ret);
   EAPI void
     ecore_x_window_save_set_add(Ecore_X_Window win);
   EAPI void
     ecore_x_window_save_set_del(Ecore_X_Window win);
   EAPI Ecore_X_Window *
     ecore_x_window_children_get(Ecore_X_Window win, int *num);
   
   EAPI Ecore_X_Cursor
     ecore_x_cursor_new(Ecore_X_Window win, int *pixels, int w, int h, int hot_x, int hot_y);
   EAPI void
     ecore_x_cursor_free(Ecore_X_Cursor c);
   EAPI Ecore_X_Cursor
     ecore_x_cursor_shape_get(int shape);
   
   EAPI int
     ecore_x_pointer_grab(Ecore_X_Window win);
   EAPI void
     ecore_x_pointer_ungrab(void);
   EAPI int
     ecore_x_keyboard_grab(Ecore_X_Window win);
   EAPI void
     ecore_x_keyboard_ungrab(void);
   EAPI void
     ecore_x_grab(void);
   EAPI void
     ecore_x_ungrab(void);
       
       
#ifdef __cplusplus
}
#endif

#endif
