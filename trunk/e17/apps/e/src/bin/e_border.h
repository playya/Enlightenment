#ifndef E_BORDER_H
#define E_BORDER_H

typedef struct _E_Border E_Border;

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
   Evas_List      *handlers;
   
   struct {
      int          l, r, t, b;
   } client_inset;
   
   Ecore_Evas     *bg_ecore_evas;
   Evas           *bg_evas;
   Ecore_X_Window  bg_win;
   Evas_Object    *bg_object;
   Ecore_X_Window  event_win;
   
   struct {
      Ecore_X_Window shell_win;
      Ecore_X_Window win;
      
      int x, y, w, h;
      
      struct {
	 unsigned char changed : 1;
	 char *name;
      } border;
      
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
	 Ecore_X_Pixmap icon_pixmap;
	 Ecore_X_Pixmap icon_mask;
	 Ecore_X_Window icon_window;
	 Ecore_X_Window window_group;
	 Ecore_X_Gravity gravity;
	 unsigned char take_focus : 1;
	 unsigned char accepts_focus : 1;
	 unsigned char urgent : 1;
	 unsigned char delete_request : 1;
	 unsigned char withdrawn : 1;
	 unsigned char iconic : 1;
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
      } netwm;
      Ecore_X_Window_Attributes initial_attributes;
   } client;
   
   E_Container_Shape *shape;

   unsigned char   visible : 1;
   unsigned char   moving : 1;
   unsigned char   focused : 1;
   unsigned char   new_client : 1;
   unsigned char   re_manage : 1;

   unsigned char   changed : 1;

   unsigned char   ignore_first_unmap;
   unsigned char   resize_mode;
   
   struct {
      unsigned int visible : 1;
      unsigned int pos : 1;
      unsigned int size : 1;
      unsigned int stack : 1;
      unsigned int prop : 1;
      unsigned int border : 1;
      unsigned int reset_gravity : 1;
   } changes;
};

EAPI int       e_border_init(void);
EAPI int       e_border_shutdown(void);

EAPI E_Border *e_border_new(E_Container *con, Ecore_X_Window win, int first_map);
EAPI void      e_border_free(E_Border *bd);
EAPI void      e_border_ref(E_Border *bd);
EAPI void      e_border_unref(E_Border *bd);
EAPI void      e_border_show(E_Border *bd);
EAPI void      e_border_hide(E_Border *bd);
EAPI void      e_border_move(E_Border *bd, int x, int y);
EAPI void      e_border_resize(E_Border *bd, int w, int h);
EAPI void      e_border_move_resize(E_Border *bd, int x, int y, int w, int h);
EAPI void      e_border_raise(E_Border *bd);
EAPI void      e_border_lower(E_Border *bd);
EAPI void      e_border_stack_above(E_Border *bd, E_Border *above);
EAPI void      e_border_stack_below(E_Border *bd, E_Border *below);

EAPI E_Border *e_border_find_by_client_window(Ecore_X_Window win);

EAPI void      e_border_idler_before(void);

#endif
