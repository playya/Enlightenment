#include "engage.h"
#include "config.h"
#include "Ecore_X.h"
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#define SYSTEM_TRAY_REQUEST_DOCK    0
#define SYSTEM_TRAY_BEGIN_MESSAGE   1
#define SYSTEM_TRAY_CANCEL_MESSAGE  2

#define XEMBED_EMBEDDED_NOTIFY      0

typedef struct _Window_List Window_List;
struct _Window_List {
  Ecore_X_Window win;
  char          *title;
  Window_List   *next;
};

int tray_count = 0;
Window_List *tray_list = NULL;

/*
 * This or something similar should probably go into ecore_x
 */
extern Display *_ecore_x_disp;

int
ecore_x_client_message_send(Window win, Atom type, long d0, long d1,
                            long d2, long d3, long d4)
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

    XSendEvent(_ecore_x_disp, win, False, NoEventMask, &xev);
}

static Display *display;
static Window root;
static int tray_init;

void
od_tray_layout() {
  Window_List *tmp;
  int xpos;
  int oddflag;

  tmp = tray_list;
  xpos = 0;
  oddflag = 0;
  while(tmp) {
    /* this line sets some (skype...) to the correct position in the engage win
       but others (psi...) to the same position rel to the main screen - bum */
    ecore_x_window_prop_xy_set(tmp->win, xpos, options.height - oddflag - 24);

    tmp = tmp->next;
    if (oddflag) {
      oddflag = 0;
      xpos += 24;
    } else {
      oddflag = 24;
    }
  }
  
}

void
od_tray_add(Ecore_X_Window win) {
  Window_List *new, *insert_after;
  
  new = malloc(sizeof(Window_List));
  new->win = win;
  new->title = strdup(ecore_x_window_prop_title_get(win));
  new->next = NULL;

  /* we want to insert at the end, so as not to move all icons on each add */
  insert_after = tray_list;
  if (insert_after)
    while(insert_after->next)
      insert_after = insert_after->next;
  if (!insert_after)
    tray_list = new;
  else
    insert_after->next = new;
  tray_count++;
  
  printf("adding icon %x for %s\n", win, new->title);
  ecore_x_event_mask_set(win, ECORE_X_EVENT_MASK_WINDOW_CONFIGURE);
  XReparentWindow (display, win, od_window, 0, 0);  
  ecore_x_window_resize(win, 24, 24);
  od_tray_layout();

  /* Map the window (will go top-level until reparented) */
  ecore_x_window_show(win);
}

void
od_tray_remove(Ecore_X_Window win) {
  Window_List *tmp, *ptr;

  ptr = NULL;
  tmp = tray_list;
  while (tmp) {
    if (tmp->win == win)
      break;
    ptr = tmp;
    tmp = tmp->next;
  }
  if (!tmp)
    return;
  
  tray_count--;
  printf("removing icon %x for %s\n", win, tmp->title);
  if (ptr)
    ptr->next = tmp->next;
  else
    tray_list = tmp->next;
  if (tmp->title)
    free(tmp->title);
  free(tmp);
  od_tray_layout();
}

int
od_tray_msg_cb(void *data, int type, void *event)
{
  Ecore_X_Event_Client_Message *ev = event;
  Ecore_X_Event_Window_Destroy *dst = event;

  if (type == ECORE_X_EVENT_CLIENT_MESSAGE) {
    if (ev->message_type == ecore_x_atom_get("_NET_SYSTEM_TRAY_OPCODE")) {
      XEvent xevent;

      od_tray_add((Ecore_X_Window) ev->data.l[2]);
      
      /* Should proto be set according to clients _XEMBED_INFO? */
      ecore_x_client_message_send(ev->data.l[2], ecore_x_atom_get("_XEMBED"),
                                  CurrentTime, XEMBED_EMBEDDED_NOTIFY,
                                  0, od_window, /*proto*/1);
    } else if (ev->message_type == ecore_x_atom_get("_NET_SYSTEM_TRAY_MESSAGE_DATA")) {
      printf("got message\n");
    }
  } else if (type == ECORE_X_EVENT_WINDOW_DESTROY ||
             type == ECORE_X_EVENT_WINDOW_HIDE) {
    od_tray_remove((Ecore_X_Window) dst->win);
  }

  return 1;

}

void
od_tray_move(OD_Icon *icon)
{
  int x, y, w, h;
  Screen         *scr;
  int             def;
  int             res_x, res_y;

  /* small check, as we are not really integrating yet */
  if (!tray_init)
    return;
  // ecore_x_window_geometry_get(od_window, &x, &y, &w, &h); // no work :(
  def = DefaultScreen(display);
  scr = ScreenOfDisplay(display, def);
  res_x = scr->width;
  res_y = scr->height;
  x = (res_x - options.width) / 2;
  y = res_y - options.height;

  if (icon->data.minwin.window) {
    ecore_x_window_prop_xy_set(icon->data.minwin.window, x + (int) icon->x - options.size + 4, y + (int) icon->y - (options.size / 2));
    /* hack to update icon background */
    ecore_x_window_resize(icon->data.minwin.window, 0, 0);
    ecore_x_window_resize(icon->data.minwin.window, options.size, options.size);
  }

}

void
od_tray_init()
{
  Atom selection_atom;
  int scr;

  tray_init = 1;
  display = ecore_x_display_get();
  root = RootWindow (display, DefaultScreen(display));

  selection_atom = ecore_x_atom_get("_NET_SYSTEM_TRAY_S0");
  XSetSelectionOwner (display, selection_atom, od_window, CurrentTime);

  if (XGetSelectionOwner (display, selection_atom) == od_window) {
    printf("am a system tray :) :)\n");

    ecore_x_client_message_send(root, ecore_x_atom_get("MANAGER"),
                                CurrentTime, selection_atom,
                                od_window, 0, 0);
  }

  ecore_event_handler_add(ECORE_X_EVENT_CLIENT_MESSAGE, od_tray_msg_cb, NULL);
  ecore_event_handler_add(ECORE_X_EVENT_WINDOW_DESTROY, od_tray_msg_cb, NULL);
}
