/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifndef __ECORE_WINCE_PRIVATE_H__
#define __ECORE_WINCE_PRIVATE_H__


/* logging messages macros */
extern int _ecore_wince_log_dom;

#define ECORE_WINCE_MSG_ERR(...) EINA_LOG_DOM_ERR(_ecore_wince_log_dom , __VA_ARGS__)
#define ECORE_WINCE_MSG_DBG(...) EINA_LOG_DOM_DBG(_ecore_wince_log_dom , __VA_ARGS__)
#define ECORE_WINCE_MSG_INFO(...) EINA_LOG_DOM_INFO(_ecore_wince_log_dom , __VA_ARGS__)

#define ECORE_WINCE_WINDOW_CLASS L"Ecore_WinCE_Window_Class"


typedef struct _Ecore_WinCE_Callback_Data Ecore_WinCE_Callback_Data;

struct _Ecore_WinCE_Callback_Data
{
   RECT         update;
   HWND         window;
   unsigned int message;
   WPARAM       window_param;
   LPARAM       data_param;
   long         time;
   int          x;
   int          y;
};


typedef int (*ecore_wince_suspend) (int);
typedef int (*ecore_wince_resume)  (int);


struct _Ecore_WinCE_Window
{
   HWND                window;

   int                 backend;
   ecore_wince_suspend suspend;
   ecore_wince_resume  resume;

   RECT                rect;           /* used to go fullscreen to normal */

   unsigned int        pointer_is_in : 1;
   unsigned int        fullscreen    : 1;
};

extern HINSTANCE           _ecore_wince_instance;
extern double              _ecore_wince_double_click_time;
extern long                _ecore_wince_event_last_time;
extern Ecore_WinCE_Window *_ecore_wince_event_last_window;


void  _ecore_wince_event_handle_key_press(Ecore_WinCE_Callback_Data *msg, int is_keystroke);
void  _ecore_wince_event_handle_key_release(Ecore_WinCE_Callback_Data *msg, int is_keystroke);
void  _ecore_wince_event_handle_button_press(Ecore_WinCE_Callback_Data *msg, int button);
void  _ecore_wince_event_handle_button_release(Ecore_WinCE_Callback_Data *msg, int button);
void  _ecore_wince_event_handle_motion_notify(Ecore_WinCE_Callback_Data *msg);
void  _ecore_wince_event_handle_enter_notify(Ecore_WinCE_Callback_Data *msg);
void  _ecore_wince_event_handle_leave_notify(Ecore_WinCE_Callback_Data *msg);
void  _ecore_wince_event_handle_focus_in(Ecore_WinCE_Callback_Data *msg);
void  _ecore_wince_event_handle_focus_out(Ecore_WinCE_Callback_Data *msg);
void  _ecore_wince_event_handle_expose(Ecore_WinCE_Callback_Data *msg);
void  _ecore_wince_event_handle_create_notify(Ecore_WinCE_Callback_Data *msg);
void  _ecore_wince_event_handle_destroy_notify(Ecore_WinCE_Callback_Data *msg);
void  _ecore_wince_event_handle_map_notify(Ecore_WinCE_Callback_Data *msg);
void  _ecore_wince_event_handle_unmap_notify(Ecore_WinCE_Callback_Data *msg);
void  _ecore_wince_event_handle_delete_request(Ecore_WinCE_Callback_Data *msg);


#endif /* __ECORE_WINCE_PRIVATE_H__ */
