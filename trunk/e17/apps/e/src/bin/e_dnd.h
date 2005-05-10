/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef E_TYPEDEFS

typedef struct _E_Drag             E_Drag;
typedef struct _E_Drop_Handler     E_Drop_Handler;
typedef struct _E_Event_Dnd_Enter  E_Event_Dnd_Enter;
typedef struct _E_Event_Dnd_Move   E_Event_Dnd_Move;
typedef struct _E_Event_Dnd_Leave  E_Event_Dnd_Leave;
typedef struct _E_Event_Dnd_Drop   E_Event_Dnd_Drop;

#else
#ifndef E_DND_H
#define E_DND_H

struct _E_Drag
{
   char *type;
   void *data;
   struct {
	void (*finished)(E_Drag *drag, int dropped);
   } cb;
   E_Container   *container;
   Ecore_Evas    *ee;
   unsigned char  visible : 1;
   Evas_Object    *object;
};

struct _E_Drop_Handler
{
   void *data;
   struct {
	void (*enter)(void *data, const char *type, void *event);
	void (*move)(void *data, const char *type, void *event);
	void (*leave)(void *data, const char *type, void *event);
	void (*drop)(void *data, const char *type, void *event);
   } cb;
   char *type;
   int x, y, w, h;
   unsigned char active : 1;
   unsigned char entered : 1;
};

struct _E_Event_Dnd_Enter
{
   int x, y;
};

struct _E_Event_Dnd_Move
{
   int x, y;
};

struct _E_Event_Dnd_Leave
{
   int x, y;
};

struct _E_Event_Dnd_Drop
{
   void *data;
   int x, y;
};

EAPI int  e_dnd_init(void);
EAPI int  e_dnd_shutdown(void);

EAPI int  e_dnd_active(void);

EAPI E_Drag* e_drag_new(E_Container *con, const char *type, void *data,
			void (*finished_cb)(E_Drag *drag, int dropped),
			const char *icon_path, const char *icon);
EAPI void    e_drag_del(E_Drag *drag);
EAPI void    e_drag_resize(E_Drag *drag, Evas_Coord w, Evas_Coord h);

EAPI void e_drag_start(E_Drag *drag);
EAPI void e_drag_update(int x, int y);
EAPI void e_drag_end(int x, int y);

EAPI E_Drop_Handler *e_drop_handler_add(void *data,
					void (*enter_cb)(void *data, const char *type, void *event),
					void (*move_cb)(void *data, const char *type, void *event),
					void (*leave_cb)(void *data, const char *type, void *event),
					void (*drop_cb)(void *data, const char *type, void *event),
				       	const char *type, int x, int y, int w, int h);
EAPI void e_drop_handler_del(E_Drop_Handler *handler);

#endif
#endif
