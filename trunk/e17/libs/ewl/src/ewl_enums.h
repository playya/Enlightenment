#ifndef __EWL_ENUMS_H__
#define __EWL_ENUMS_H__

/**
 * @defgroup Ewl_Enums Various Flags and Enumerations used in EWL
 * Provides bitmasks, flags, and other enumerations for use by widgets in EWL.
 */

/**
 * This defines the various types of callbacks that can be hooked up for each
 * widget.
 */
typedef enum Ewl_Callback_Type Ewl_Callback_Type;

enum Ewl_Callback_Type
{
	EWL_CALLBACK_NONE, /**< A placeholder FIXME: Is this necessary still? */
	EWL_CALLBACK_EXPOSE, /**< Triggered when the window needs redrawing */
	EWL_CALLBACK_REALIZE, /**< Event when a widget is first drawn */
	EWL_CALLBACK_UNREALIZE, /**< When a widget is no longer drawn */
	EWL_CALLBACK_SHOW, /**< A widget has been marked visible */
	EWL_CALLBACK_HIDE, /**< A widget is marked hidden */
	EWL_CALLBACK_DESTROY, /**< The widget is freed */
	EWL_CALLBACK_DELETE_WINDOW, /**< The window is being closed */
	EWL_CALLBACK_CONFIGURE, /**< The object is being resized */
	EWL_CALLBACK_REPARENT, /**< A widget has been placed in a container */
	EWL_CALLBACK_KEY_DOWN, /**< A key was pressed down */
	EWL_CALLBACK_KEY_UP, /**< A key was released */
	EWL_CALLBACK_MOUSE_DOWN, /**< Mouse was pressed down */
	EWL_CALLBACK_MOUSE_UP, /**< Mouse was released */
	EWL_CALLBACK_MOUSE_MOVE, /**< Mouse was moved */
	EWL_CALLBACK_FOCUS_IN, /**< Mouse was placed over the widget */
	EWL_CALLBACK_FOCUS_OUT, /**< Mouse was moved away from the widget */
	EWL_CALLBACK_SELECT, /**< Widget was selected by mouse or key */
	EWL_CALLBACK_DESELECT, /**< Widget was deselected by mouse or key */
	EWL_CALLBACK_CLICKED, /**< Mouse was pressed and released on a widget */
	EWL_CALLBACK_DOUBLE_CLICKED, /**< Mouse was clicked twice quickly */
	EWL_CALLBACK_HILITED, /**< Mouse is over the widget */
	EWL_CALLBACK_VALUE_CHANGED, /**< Value in widget changed */
	EWL_CALLBACK_STATE_CHANGED, /**< Alter the state of the appearance */
	EWL_CALLBACK_APPEARANCE_CHANGED, /**< Theme key of widget changed */
	EWL_CALLBACK_WIDGET_ENABLE, /**< Widget has been re-enabled */
	EWL_CALLBACK_WIDGET_DISABLE, /**< Widget no longer takes input */
	EWL_CALLBACK_MAX /**< Flag to indicate last value */
};

/**
 * Flags for the callbacks to indicate interception or notification of the
 * parent.
 */
typedef enum Ewl_Event_Notify Ewl_Event_Notify;

enum Ewl_Event_Notify
{
	EWL_CALLBACK_NOTIFY_NONE = 0,
	EWL_CALLBACK_NOTIFY_NOTIFY = 1,
	EWL_CALLBACK_NOTIFY_INTERCEPT = 2
};

/**
 * The orientation enum is used in a few widgets to specify whether the widget
 * should be laid out in a horizontal or vertical fashion.
 */
typedef enum Ewl_Orientation Ewl_Orientation;

enum Ewl_Orientation
{
	EWL_ORIENTATION_HORIZONTAL,
	EWL_ORIENTATION_VERTICAL
};

enum Ewl_Flags
{
	/*
	 * The alignment enumeration allows for specifying how an element is
	 * aligned within it's container.
	 */
	EWL_FLAG_ALIGN_CENTER = ETOX_ALIGN_CENTER,
	EWL_FLAG_ALIGN_LEFT = ETOX_ALIGN_LEFT,
	EWL_FLAG_ALIGN_RIGHT = ETOX_ALIGN_RIGHT,
	EWL_FLAG_ALIGN_TOP = ETOX_ALIGN_TOP,
	EWL_FLAG_ALIGN_BOTTOM = ETOX_ALIGN_BOTTOM,

	/*
	 * Fill policy identifies to containers whether child widgets should be
	 * stretched to fill available space or keep their current size.
	 */
	EWL_FLAG_FILL_NONE = 0,
	EWL_FLAG_FILL_HSHRINK = 0x1000,
	EWL_FLAG_FILL_VSHRINK = 0x2000,
	EWL_FLAG_FILL_SHRINK =
	    EWL_FLAG_FILL_HSHRINK | EWL_FLAG_FILL_VSHRINK,
	EWL_FLAG_FILL_HFILL = 0x4000,
	EWL_FLAG_FILL_VFILL = 0x8000,
	EWL_FLAG_FILL_FILL = EWL_FLAG_FILL_HFILL | EWL_FLAG_FILL_VFILL,
	EWL_FLAG_FILL_ALL = EWL_FLAG_FILL_FILL | EWL_FLAG_FILL_SHRINK,

	/*
	 * Flags identifying the visibility status of the widget
	 */
	EWL_FLAG_VISIBLE_HIDDEN = 0,
	EWL_FLAG_VISIBLE_SHOWN = 0x10000,
	EWL_FLAG_VISIBLE_REALIZED = 0x20000,
	EWL_FLAG_VISIBLE_OBSCURED = 0x40000,

	EWL_FLAG_PROPERTY_RECURSIVE = 0x80000,
	EWL_FLAG_PROPERTY_TOPLEVEL = 0x100000,

	/*
	 * Flags to indicate queues this object is on.
	 */
	EWL_FLAG_QUEUED_CSCHEDULED = 0x200000,
	EWL_FLAG_QUEUED_RSCHEDULED = 0x400000,
	EWL_FLAG_QUEUED_DSCHEDULED = 0x800000,

	/*
	 * The state enum specifies the current state of a widget, ie. has it
	 * been clicked, does it have the keyboard focus, etc.
	 */
	EWL_FLAG_STATE_NORMAL = 0,
	EWL_FLAG_STATE_HILITED = 0x1000000,
	EWL_FLAG_STATE_PRESSED = 0x2000000,
	EWL_FLAG_STATE_SELECTED = 0x4000000,
	EWL_FLAG_STATE_DND = 0x8000000,
	EWL_FLAG_STATE_DISABLED = 0x10000000
};

#define EWL_FLAG_FILL_NORMAL (EWL_FLAG_FILL_FILL)

#define EWL_FLAGS_ALIGN_MASK (EWL_FLAG_ALIGN_CENTER | EWL_FLAG_ALIGN_LEFT | \
		EWL_FLAG_ALIGN_RIGHT | EWL_FLAG_ALIGN_TOP | \
		EWL_FLAG_ALIGN_BOTTOM)

#define EWL_FLAGS_FILL_MASK (EWL_FLAG_FILL_NONE | EWL_FLAG_FILL_SHRINK | \
		EWL_FLAG_FILL_FILL)

#define EWL_FLAGS_VISIBLE_MASK (EWL_FLAG_VISIBLE_HIDDEN | \
		EWL_FLAG_VISIBLE_SHOWN | EWL_FLAG_VISIBLE_REALIZED | \
		EWL_FLAG_VISIBLE_OBSCURED)

#define  EWL_FLAGS_PROPERTY_MASK (EWL_FLAG_PROPERTY_RECURSIVE | \
		EWL_FLAG_PROPERTY_TOPLEVEL)

#define  EWL_FLAGS_QUEUED_MASK (EWL_FLAG_QUEUED_CSCHEDULED | \
		EWL_FLAG_QUEUED_RSCHEDULED | EWL_FLAG_QUEUED_DSCHEDULED)

#define  EWL_FLAGS_STATE_MASK (EWL_FLAG_STATE_NORMAL | \
		EWL_FLAG_STATE_HILITED | EWL_FLAG_STATE_PRESSED | \
		EWL_FLAG_STATE_SELECTED | EWL_FLAG_STATE_DND | \
		EWL_FLAG_STATE_DISABLED)

typedef enum Ewl_Position Ewl_Position;

enum Ewl_Position
{
	EWL_POSITION_LEFT = 0x1,
	EWL_POSITION_RIGHT = 0x2,
	EWL_POSITION_TOP = 0x4,
	EWL_POSITION_BOTTOM = 0x8
};

#define EWL_POSITION_MASK (0xf)

typedef enum Ewl_Window_Flags Ewl_Window_Flags;

enum Ewl_Window_Flags
{
	EWL_WINDOW_AUTO_SIZE = 1,
	EWL_WINDOW_BORDERLESS = 2
};

typedef enum Ewl_Tree_Node_Flags Ewl_Tree_Node_Flags;

enum Ewl_Tree_Node_Flags
{
	EWL_TREE_NODE_NOEXPAND = 0,
	EWL_TREE_NODE_COLLAPSED = 1,
	EWL_TREE_NODE_EXPANDED = 2
};

typedef enum Ewl_Notebook_Flags Ewl_Notebook_Flags;

enum Ewl_Notebook_Flags
{
	EWL_NOTEBOOK_FLAG_TABS_HIDDEN = 0x10
};

typedef enum Ewl_Scrollbar_Flags Ewl_ScrollBar_Flags;

enum Ewl_Scrollbar_Flags
{
	EWL_SCROLLBAR_FLAG_NONE,
	EWL_SCROLLBAR_FLAG_AUTO_VISIBLE,
	EWL_SCROLLBAR_FLAG_ALWAYS_HIDDEN
};


typedef enum Ewl_Filedialog_Type Ewl_Filedialog_Type;

enum Ewl_Filedialog_Type
{
	EWL_FILEDIALOG_TYPE_OPEN,
	EWL_FILEDIALOG_TYPE_CLOSE
};

#endif				/* __EWL_ENUMS_H__ */
