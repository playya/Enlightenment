#include "events.h"
#include "callbacks.h"
#include <limits.h>

#define USER_PASS_MAX 32

static struct
{
   char buf[USER_PASS_MAX];
   int index;
}
typebuffer;

extern void set_text_entry_text(int is_pass, char *txt);

static E_Login_Session e_session = NULL;
static Evas evas = NULL;

static void e_idle(void *data);
static void e_window_expose(Ecore_Event * e);
static void e_mouse_move(Ecore_Event * e);
static void e_mouse_down(Ecore_Event * e);
static void e_mouse_up(Ecore_Event * e);
static void e_key_down(Ecore_Event * e);

void
setup_events(E_Login_Session e)
{
   e_session = e;
   evas = e->evas;

   ecore_event_filter_handler_add(ECORE_EVENT_WINDOW_EXPOSE, e_window_expose);
   ecore_event_filter_handler_add(ECORE_EVENT_MOUSE_MOVE, e_mouse_move);
   ecore_event_filter_handler_add(ECORE_EVENT_MOUSE_DOWN, e_mouse_down);
   ecore_event_filter_handler_add(ECORE_EVENT_MOUSE_UP, e_mouse_up);
   ecore_event_filter_handler_add(ECORE_EVENT_KEY_DOWN, e_key_down);
   ecore_event_filter_idle_handler_add(e_idle, NULL);

   typebuffer.index = 0;
   typebuffer.buf[0] = 0;
}

static void
e_idle(void *data)
{
   evas_render(evas);
}


static void
e_window_expose(Ecore_Event * ev)
{
   Ecore_Event_Window_Expose *e;

   e = (Ecore_Event_Window_Expose *) ev->event;
   if ((e->win != evas_get_window(evas)))
      return;
   evas_update_rect(evas, e->x, e->y, e->w, e->h);

}

static void
e_mouse_move(Ecore_Event * ev)
{
   Ecore_Event_Mouse_Move *e;


   e = (Ecore_Event_Mouse_Move *) ev->event;

   if ((e->win != evas_get_window(evas)))
      return;
   evas_event_move(evas, e->x, e->y);
}

static void
e_mouse_down(Ecore_Event * ev)
{
   Ecore_Event_Mouse_Down *e;

   e = (Ecore_Event_Mouse_Down *) ev->event;
   if ((e->win != evas_get_window(evas)))
      return;
   evas_event_button_down(evas, e->x, e->y, e->button);
}

static void
e_mouse_up(Ecore_Event * ev)
{
   Ecore_Event_Mouse_Up *e;

   e = (Ecore_Event_Mouse_Up *) ev->event;
   if ((e->win != evas_get_window(evas)))
      return;
   evas_event_button_up(evas, e->x, e->y, e->button);
}

static void
e_key_down(Ecore_Event * ev)
{
   Ecore_Event_Key_Down *e;

   static int password_or_user = 0;

   e = ev->event;


#if 0
   fprintf(stderr, "typebuffer.index is %d\n", typebuffer.index);
   fprintf(stderr, "e->key is %s\n", e->key);
   fprintf(stderr, "e->compose is %s\n", e->compose);
#endif

   if (e->key)
   {
      int length = 0;

      length = strlen(e->key);
      if (e->mods & ECORE_EVENT_KEY_MODIFIER_CTRL)
      {
         switch (*e->key)
         {
           case 'u':           /* clear the buffer */
              memset(&typebuffer.buf, 0, USER_PASS_MAX);
              typebuffer.index = 0;
              break;
           default:
              break;
         }
      }
      else if (length > 1)
      {
         if (!(strcmp(e->key, "BackSpace")))
         {
            if (typebuffer.index > 0)
            {
               typebuffer.buf[--typebuffer.index] = 0;
            }
         }
         else if (!(strcmp(e->key, "Return")))
         {
            password_or_user =
               elogin_return_key_cb(e_session, typebuffer.buf);
            memset(&typebuffer.buf, 0, USER_PASS_MAX);
            typebuffer.index = 0;
         }

         if (typebuffer.index >= USER_PASS_MAX)
            return;
         else if (!(strcmp(e->key, "space")))
         {
            typebuffer.buf[typebuffer.index++] = ' ';
         }
      }
      else
      {
         if (typebuffer.index >= USER_PASS_MAX)
            return;
         typebuffer.buf[typebuffer.index++] = *e->key;
      }
   }
   else
   {
      if (typebuffer.index >= USER_PASS_MAX)
         return;
      typebuffer.buf[typebuffer.index++] = *e->compose;
   }
   set_text_entry_text(password_or_user, typebuffer.buf);
}
