#include "E_Notify.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <Ecore_Data.h>
#include "e_notify_private.h"

/* private functions */
static Ecore_List * e_notification_action_list_new();
static E_Notification_Action *e_notification_action_new(const char *id, const char *name);
static void e_notification_action_free(E_Notification_Action *act);

/* (con|de)structor */

E_Notification *
e_notification_full_new(const char *app_name, unsigned int replaces_id, const char *app_icon, const char *summary, const char *body, int expire_timeout)
{
  E_Notification *n;

  n = e_notification_new();
  if (!n) return NULL;

  if (app_name) n->app_name = strdup(app_name); 
  n->replaces_id = replaces_id;
  if (app_icon) n->app_icon = strdup(app_icon); 
  if (summary) n->summary = strdup(summary); 
  if (body) n->body = strdup(body);
  n->expire_timeout = expire_timeout;


  return n;
}

E_Notification *
e_notification_new()
{
  E_Notification *n;
  n = calloc(1, sizeof(E_Notification));
  if (!n) return NULL;
  n->refcount = 1;

  return n;
}

void
e_notification_ref(E_Notification *n)
{
  n->refcount++;
}

void
e_notification_unref(E_Notification *n)
{
  if (--n->refcount == 0) e_notification_free(n);
}

void
e_notification_free(E_Notification *n)
{
  if (!n) return;

  if (n->app_name) free(n->app_name);
  if (n->app_icon) free(n->app_icon);
  if (n->summary) free(n->summary);
  if (n->body) free(n->body);

  if (n->actions) ecore_list_destroy(n->actions);

  if (n->hints.category) free(n->hints.category);
  if (n->hints.desktop) free(n->hints.desktop);
  if (n->hints.sound_file) free(n->hints.sound_file);
  if (n->hints.image_data) e_notification_image_free(n->hints.image_data);
  free(n);
}

/* mutators */
void
e_notification_id_set(E_Notification *note, unsigned int id)
{
  note->id = id;
}

void
e_notification_app_name_set(E_Notification *note, const char *app_name)
{
  if (note->app_name) free(note->app_name);
  if (app_name) note->app_name = strdup(app_name);
}

void
e_notification_app_icon_set(E_Notification *note, const char *app_icon)
{
  if (note->app_icon) free(note->app_icon);
  if (app_icon) note->app_icon = strdup(app_icon);
}

void
e_notification_summary_set(E_Notification *note, const char *summary)
{
  if (note->summary) free(note->summary);
  if (summary) note->summary = strdup(summary);
}

void
e_notification_body_set(E_Notification *note, const char *body)
{
  if (note->body) free(note->body);
  if (body) note->body = strdup(body);
}

void
e_notification_action_add(E_Notification *n, const char *action_id, const char *action_name)
{
  E_Notification_Action *a;

  if (!n->actions) 
    n->actions = e_notification_action_list_new();

  a = e_notification_action_new(action_id, action_name);
  ecore_list_append(n->actions, a);
}


void
e_notification_replaces_id_set(E_Notification *note, int replaces_id)
{
  note->replaces_id = replaces_id;
}

void
e_notification_timeout_set(E_Notification *note, int timeout)
{
  note->expire_timeout = timeout;
}

void
e_notification_closed_set(E_Notification *note, unsigned char closed)
{
  note->closed = closed;
}


/* accessors */
unsigned int
e_notification_id_get(E_Notification *note)
{
  return note->id;
}

const char *
e_notification_app_name_get(E_Notification *note)
{
  return note->app_name;
}

const char *
e_notification_app_icon_get(E_Notification *note)
{
  return note->app_icon;
}

const char *
e_notification_summary_get(E_Notification *note)
{
  return note->summary;
}

const char *
e_notification_body_get(E_Notification *note)
{
  return note->body;
}

Ecore_List *
e_notification_actions_get(E_Notification *note)
{
  return note->actions;
}

int
e_notification_replaces_id_get(E_Notification *note)
{
  return note->replaces_id;
}

int
e_notification_timeout_get(E_Notification *note)
{
  return note->expire_timeout;
}

unsigned char
e_notification_closed_get(E_Notification *note)
{
  return note->closed;
}

/***** actions *****/

static Ecore_List *
e_notification_action_list_new()
{
  Ecore_List *alist;
  alist = ecore_list_new();
  ecore_list_set_free_cb(alist, (Ecore_Free_Cb)e_notification_action_free);
  return alist;
}

static E_Notification_Action *
e_notification_action_new(const char *id, const char *name)
{
  E_Notification_Action *act;
  act = malloc(sizeof(E_Notification_Action));
  act->id = strdup(id);
  act->name = strdup(name);
  return act;
}


static void
e_notification_action_free(E_Notification_Action *act)
{
  if (!act) return;
  if (act->id) free(act->id);
  if (act->name) free(act->name);
  free(act);
}


/********* hints *******/


void 
e_notification_hint_urgency_set(E_Notification *n, char urgency)
{
  n->hints.urgency = urgency;
  n->hint_flags |= E_NOTIFICATION_HINT_URGENCY;
}

void 
e_notification_hint_category_set(E_Notification *n, const char *category)
{
  if (n->hints.category) free(n->hints.category);
  n->hints.category = strdup(category);
  n->hint_flags |= E_NOTIFICATION_HINT_CATEGORY;
}

void 
e_notification_hint_desktop_set(E_Notification *n, const char *desktop)
{
  if (n->hints.desktop) free(n->hints.desktop);
  n->hints.desktop = strdup(desktop);
  n->hint_flags |= E_NOTIFICATION_HINT_DESKTOP;
}

void 
e_notification_hint_sound_file_set(E_Notification *n, const char *sound_file)
{
  if (n->hints.sound_file) free(n->hints.sound_file);
  n->hints.sound_file = strdup(sound_file);
  n->hint_flags |= E_NOTIFICATION_HINT_SOUND_FILE;
}

void 
e_notification_hint_suppress_sound_set(E_Notification *n, char suppress_sound)
{
  n->hints.suppress_sound = suppress_sound;
  n->hint_flags |= E_NOTIFICATION_HINT_SUPPRESS_SOUND;
}

void 
e_notification_hint_xy_set(E_Notification *n, int x, int y)
{
  n->hints.x = x;
  n->hints.y = y;
  n->hint_flags |= E_NOTIFICATION_HINT_XY;
}

void 
e_notification_hint_image_data_set(E_Notification *n, E_Notification_Image *image)
{
  n->hints.image_data = image;
}


char  
e_notification_hint_urgency_get(E_Notification *n)
{
  return (n->hint_flags & E_NOTIFICATION_HINT_URGENCY ? n->hints.urgency : 1);
}

const char *
e_notification_hint_category_get(E_Notification *n)
{
  return n->hints.category;
}

const char *
e_notification_hint_desktop_get(E_Notification *n)
{
  return n->hints.desktop;
}

const char *
e_notification_hint_sound_file_get(E_Notification *n)
{
  return n->hints.sound_file;
}

char  
e_notification_hint_suppress_sound_get(E_Notification *n)
{
  return n->hints.suppress_sound;
}

int  
e_notification_hint_xy_get(E_Notification *n, int *x, int *y)
{
  if (x) *x = n->hints.x;
  if (y) *y = n->hints.y;

  return (n->hint_flags & E_NOTIFICATION_HINT_XY ? 1 : 0);
}

E_Notification_Image *
e_notification_hint_image_data_get(E_Notification *n)
{
  return n->hints.image_data;
}


E_Notification_Image *
e_notification_image_new()
{
  E_Notification_Image *img;

  img = calloc(1, sizeof(E_Notification_Image));
  return img;
}

void
e_notification_image_free(E_Notification_Image *img)
{
  if (img->data) free(img->data);
  if (img) free(img);
}


Evas_Object *
e_notification_image_evas_object_add(Evas *evas, E_Notification_Image *img)
{
#if 0 
  unsigned int *imgdata;
  
  imgdata = malloc(img.width * img.height * 4);

  // evas requires 32 bit RGBA data, with no padding after rows
  if (img->bits_per_sample == 8)
  {
    if (img->channels == 4)
    {
      /* RGBA data */
      if (img->rowstride == 4 * width)
      {
        // data is already in format we need
        memcpy(imgdata, img->data, 4 * img.width * img.height)
      }
      else
      {
        /* rowstride unneccesarily too long? */
        int i;
        for (i = 0; i < img->height; i++)
        {
        }
      }
    } 
    else if (img->channels == 3)
    {
      unsigned int val;
      unsigned int i, j;

      for ()
      {
      }
    }
  }
  else
  {
  }
#endif 
  return NULL;
}
