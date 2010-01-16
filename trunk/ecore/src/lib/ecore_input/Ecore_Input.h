/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifndef _ECORE_INPUT_H
#define _ECORE_INPUT_H


#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_ECORE_INPUT_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_ECORE_INPUT_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

EAPI extern int ECORE_EVENT_KEY_DOWN;
EAPI extern int ECORE_EVENT_KEY_UP;
EAPI extern int ECORE_EVENT_MOUSE_BUTTON_DOWN;
EAPI extern int ECORE_EVENT_MOUSE_BUTTON_UP;
EAPI extern int ECORE_EVENT_MOUSE_MOVE;
EAPI extern int ECORE_EVENT_MOUSE_WHEEL;
EAPI extern int ECORE_EVENT_MOUSE_IN;
EAPI extern int ECORE_EVENT_MOUSE_OUT;

#define ECORE_EVENT_MODIFIER_SHIFT	0x0001
#define ECORE_EVENT_MODIFIER_CTRL	0x0002
#define ECORE_EVENT_MODIFIER_ALT	0x0004
#define ECORE_EVENT_MODIFIER_WIN	0x0008
#define ECORE_EVENT_MODIFIER_SCROLL	0x0010
#define ECORE_EVENT_MODIFIER_NUM	0x0020
#define ECORE_EVENT_MODIFIER_CAPS	0x0040
#define ECORE_EVENT_LOCK_SCROLL		0x0080
#define ECORE_EVENT_LOCK_NUM		0x0100
#define ECORE_EVENT_LOCK_CAPS		0x0200

typedef uintptr_t Ecore_Window;

typedef struct _Ecore_Event_Key Ecore_Event_Key;
struct _Ecore_Event_Key
{
   const char	*keyname;
   const char	*key;
   const char	*string;
   const char	*compose;
   Ecore_Window  window;
   Ecore_Window  root_window;
   Ecore_Window	 event_window;

   unsigned int	 timestamp;
   unsigned int  modifiers;

   int           same_screen;
};

typedef struct _Ecore_Event_Mouse_Button Ecore_Event_Mouse_Button;
struct _Ecore_Event_Mouse_Button
{
   Ecore_Window  window;
   Ecore_Window  root_window;
   Ecore_Window	 event_window;

   unsigned int	 timestamp;
   unsigned int	 modifiers;
   unsigned int  buttons;
   unsigned int  double_click;
   unsigned int  triple_click;
   int           same_screen;

   int		 x;
   int		 y;
   struct
   {
      int	 x;
      int	 y;
   } root;
   
   int           device;
   int           radius;
   int           radius_x;
   int           radius_y;
};

typedef struct _Ecore_Event_Mouse_Wheel Ecore_Event_Mouse_Wheel;
struct _Ecore_Event_Mouse_Wheel
{
   Ecore_Window	 window;
   Ecore_Window  root_window;
   Ecore_Window	 event_window;

   unsigned int	 timestamp;
   unsigned int	 modifiers;

   int           same_screen;
   int		 direction;
   int		 z;

   int		 x;
   int		 y;
   struct
   {
      int	 x;
      int	 y;
   } root;
};

typedef struct _Ecore_Event_Mouse_Move Ecore_Event_Mouse_Move;
struct _Ecore_Event_Mouse_Move
{
   Ecore_Window	 window;
   Ecore_Window  root_window;
   Ecore_Window	 event_window;

   unsigned int	 timestamp;
   unsigned int	 modifiers;

   int           same_screen;

   int		 x;
   int		 y;
   struct
   {
      int	 x;
      int	 y;
   } root;

   int           device;
   int           radius;
   int           radius_x;
   int           radius_y;
};

typedef struct _Ecore_Event_Mouse_IO Ecore_Event_Mouse_IO;
struct _Ecore_Event_Mouse_IO
{
   Ecore_Window	 window;
   Ecore_Window	 event_window;

   unsigned int	 timestamp;
   unsigned int  modifiers;

   int		 x;
   int		 y;
};

enum _Ecore_Event_Modifier
{
  ECORE_NONE,
  ECORE_SHIFT,
  ECORE_CTRL,
  ECORE_ALT,
  ECORE_WIN,
  ECORE_SCROLL,
  ECORE_CAPS,
  ECORE_LAST
};

enum _Ecore_Event_Press
{
  ECORE_DOWN,
  ECORE_UP
};

enum _Ecore_Event_IO
{
  ECORE_IN,
  ECORE_OUT
};

typedef enum _Ecore_Event_IO Ecore_Event_IO;
typedef enum _Ecore_Event_Press Ecore_Event_Press;
typedef enum _Ecore_Event_Modifier Ecore_Event_Modifier;

typedef struct _Ecore_Event_Modifiers Ecore_Event_Modifiers;
struct _Ecore_Event_Modifiers
{
   unsigned int size;
   unsigned int array[ECORE_LAST];
};

EAPI int	 ecore_event_init(void);
EAPI int	 ecore_event_shutdown(void);

EAPI unsigned int ecore_event_modifier_mask(Ecore_Event_Modifier modifier);
EAPI Ecore_Event_Modifier ecore_event_update_modifier(const char *key, Ecore_Event_Modifiers *modifiers, int inc);

#ifdef __cplusplus
}
#endif

#endif
