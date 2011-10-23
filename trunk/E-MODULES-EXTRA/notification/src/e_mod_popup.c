#include "e_mod_main.h"

/* Popup function protos */
static Popup_Data *_notification_popup_new(E_Notification *n);
static Popup_Data *_notification_popup_find(unsigned int id);
static Popup_Data *_notification_popup_merge(E_Notification *n);

static int         _notification_popup_place(Popup_Data *popup,
                                             int         num);
static void        _notification_popup_refresh(Popup_Data *popup);
static void        _notification_popup_del(unsigned int                 id,
                                           E_Notification_Closed_Reason reason);
static void        _notification_popdown(Popup_Data                  *popup,
                                         E_Notification_Closed_Reason reason);

/* Util function protos */
static void _notification_format_message(Popup_Data *popup);

/* Callbacks */
static void _notification_theme_cb_deleted(void        *data,
                                           Evas_Object *obj,
                                           const char  *emission,
                                           const char  *source);
static void _notification_theme_cb_close(void        *data,
                                         Evas_Object *obj,
                                         const char  *emission,
                                         const char  *source);
static void _notification_theme_cb_find(void        *data,
                                        Evas_Object *obj,
                                        const char  *emission,
                                        const char  *source);
static Eina_Bool _notification_timer_cb(void *data);

static int next_pos = 0;

int
notification_popup_notify(E_Notification *n,
                          unsigned int    replaces_id,
                          unsigned int    id __UNUSED__,
                          const char     *appname)
{
   double timeout;
   Popup_Data *popup = NULL;
   char urgency;

   urgency = e_notification_hint_urgency_get(n);

   switch (urgency)
     {
      case E_NOTIFICATION_URGENCY_LOW:
        if (!notification_cfg->show_low)
          return 0;
        break;

      case E_NOTIFICATION_URGENCY_NORMAL:
        if (!notification_cfg->show_normal)
          return 0;
        break;

      case E_NOTIFICATION_URGENCY_CRITICAL:
        if (!notification_cfg->show_critical)
          return 0;
        break;

      default:
        break;
     }

   if (replaces_id && (popup = _notification_popup_find(replaces_id)))
     {
        e_notification_ref(n);

        if (popup->notif)
          e_notification_unref(popup->notif);

        popup->notif = n;
        _notification_popup_refresh(popup);
     }
   else if (!replaces_id)
     {
        if ((popup = _notification_popup_merge(n)))
          _notification_popup_refresh(popup);
     }

   if (!popup)
     {
        popup = _notification_popup_new(n);
        notification_cfg->popups = eina_list_append(notification_cfg->popups, popup);
        edje_object_signal_emit(popup->theme, "notification,new", "notification");
     }

   if (popup->timer)
     {
        ecore_timer_del(popup->timer);
        popup->timer = NULL;
     }

   timeout = e_notification_timeout_get(popup->notif);

   if (timeout < 0 || notification_cfg->force_timeout)
     timeout = notification_cfg->timeout;
   else
     timeout = (double)timeout / 1000.0;

   if (timeout > 0)
     popup->timer = ecore_timer_add(timeout, _notification_timer_cb, popup);

   return 1;
}

void
notification_popup_shutdown(void)
{
   Eina_List *l, *next;
   Popup_Data *popup;

   for (l = notification_cfg->popups; l && (popup = l->data); l = next)
     {
        next = l->next;
        _notification_popdown(popup, E_NOTIFICATION_CLOSED_REQUESTED);
        notification_cfg->popups = eina_list_remove_list(notification_cfg->popups, l);
     }
}

void
notification_popup_close(unsigned int id)
{
   _notification_popup_del(id, E_NOTIFICATION_CLOSED_REQUESTED);
}

static Popup_Data *
_notification_popup_merge(E_Notification *n)
{
   Eina_List *l;
   Popup_Data *popup;
   const char *str1, *str2;
   const char *body_old;
   const char *body_new;
   char *body_final;
   int body_old_len, body_new_len;

   str1 = e_notification_app_name_get(n);
   /* printf("merge %s\n", str1); */

   if (!str1)
     return NULL;

   EINA_LIST_FOREACH(notification_cfg->popups, l, popup)
     {
        if (!popup->notif)
          continue;

        if (!(str2 = e_notification_app_name_get(popup->notif)))
          continue;

        if (!strcmp(str1, str2))
          break;
     }

   if (!popup)
     {
        /* printf("- no poup to merge\n"); */
         return NULL;
     }

   str1 = e_notification_summary_get(n);
   str2 = e_notification_summary_get(popup->notif);

   /* if ((str1 && !str2) || (!str1 && str2)) */
   if (!(!(str1) == !(str2)))
     {
        /* printf("1- summary doesn match, %s, %s\n", str1, str2); */
         return NULL;
     }

   if (str1 && str2 && strcmp(str1, str2))
     {
        /* printf("- summary doesn match, %s, %s\n", str1, str2); */
         return NULL;
     }

   /* TODO if actions check if they are equal... */
   if (!(e_notification_actions_get(popup->notif)) !=
       (!(e_notification_actions_get(n))))
     {
        /* printf("- actions dont match\n"); */
         return NULL;
     }

   /* TODO  p->n is not fallback alert..*/
   /* TODO  both allow merging */

   body_old = e_notification_body_get(popup->notif);
   body_new = e_notification_body_get(n);

   body_old_len = strlen(body_old);
   body_new_len = strlen(body_new);
   body_final = alloca(body_old_len + body_new_len + 5);
   sprintf(body_final, "%s<br>%s", body_old, body_new);
   /* printf("set body %s\n", body_final); */

   e_notification_body_set(n, body_final);

   e_notification_unref(popup->notif);
   popup->notif = n;
   e_notification_ref(popup->notif);

   return popup;
}

static Popup_Data *
_notification_popup_new(E_Notification *n)
{
   E_Container *con;
   Popup_Data *popup;
   char buf[PATH_MAX];
   int shaped;

   popup = E_NEW(Popup_Data, 1);
   if (!popup) return NULL;
   e_notification_ref(n);
   popup->notif = n;

   con = e_container_current_get(e_manager_current_get());

   /* Create the popup window */
   popup->win = e_popup_new(e_zone_current_get(con), 0, 0, 0, 0);
   e_popup_edje_bg_object_set(popup->win, popup->theme);
   popup->e = popup->win->evas;

   /* Setup the theme */
   snprintf(buf, sizeof(buf), "%s/e-module-notification.edj", notification_mod->dir);
   popup->theme = edje_object_add(popup->e);

   if (!e_theme_edje_object_set(popup->theme,
                                "base/theme/modules/notification",
                                "modules/notification/main"))
     edje_object_file_set(popup->theme, buf, "modules/notification/main");

   e_popup_edje_bg_object_set(popup->win, popup->theme);

   evas_object_show(popup->theme);
   edje_object_signal_callback_add
     (popup->theme, "notification,deleted", "theme",
     _notification_theme_cb_deleted, popup);
   edje_object_signal_callback_add
     (popup->theme, "notification,close", "theme",
     _notification_theme_cb_close, popup);
   edje_object_signal_callback_add
     (popup->theme, "notification,find", "theme",
     _notification_theme_cb_find, popup);

   _notification_popup_refresh(popup);
   next_pos = _notification_popup_place(popup, next_pos);
   e_popup_show(popup->win);
   e_popup_layer_set(popup->win, 999);

   return popup;
}

static int
_notification_popup_place(Popup_Data *popup,
                          int         pos)
{
   int x, y, w, h, dir = 0;
   E_Container *con;

   con = e_container_current_get(e_manager_current_get());
   evas_object_geometry_get(popup->theme, NULL, NULL, &w, &h);
   int gap = 10;
   int to_edge = 15;

   /* XXX for now ignore placement requests */

   switch (notification_cfg->corner)
     {
      case CORNER_TL:
        e_popup_move(popup->win,
                     to_edge, to_edge + pos);
        break;

      case CORNER_TR:
        e_popup_move(popup->win,
                     con->w - (w + to_edge),
                     to_edge + pos);
        break;

      case CORNER_BL:
        e_popup_move(popup->win,
                     to_edge,
                     (con->h - h) - (to_edge + pos));
        break;

      case CORNER_BR:
        e_popup_move(popup->win,
                     con->w - (w + to_edge),
                     (con->h - h) - (to_edge + pos));
        break;
     }

   return pos + h + gap;
}

static void
_notification_popups_place()
{
   Popup_Data *popup;
   Eina_List *l, *next;
   int pos = 0;

   EINA_LIST_FOREACH(notification_cfg->popups, l, popup)
     {
        pos = _notification_popup_place(popup, pos);
     }

   next_pos = pos;
}

static void
_notification_popup_refresh(Popup_Data *popup)
{
   const char *icon_path;
   const char *app_icon_max;
   char *msg;
   void *img;
   int w, h, width = 80, height = 80;

   if (!popup) return;

   popup->app_name = e_notification_app_name_get(popup->notif);

   if (popup->app_icon)
     {
        edje_object_part_unswallow(popup->theme, popup->app_icon);
        evas_object_del(popup->app_icon);
        popup->app_icon = NULL;
     }

   app_icon_max = edje_object_data_get(popup->theme, "app_icon_max");
   if (app_icon_max)
     {
        char *endptr;

        errno = 0;
        width = strtol(app_icon_max, &endptr, 10);
        if (errno || (width < 1) || (endptr == app_icon_max))
          {
             width = 80;
             height = 80;
          }
        else
          {
             endptr++;
             if (endptr)
               {
                  height = strtol(endptr, NULL, 10);
                  if (errno || (height < 1))
                    height = 80;
               }
             else height = 80;
          }
     }

   /* Check if the app specify an icon either by a path or by a hint */
   if ((icon_path = e_notification_app_icon_get(popup->notif)) && *icon_path)
     {
        if (!strncmp(icon_path, "file://", 7)) icon_path += 7;
        if (!ecore_file_exists(icon_path))
          {
             const char *new_path;
             unsigned int size;

             size = e_util_icon_size_normalize(width * e_scale);
             new_path = efreet_icon_path_find(e_config->icon_theme, icon_path, size);
             if (new_path)
               icon_path = new_path;
             else
               {
                  Evas_Object *o = e_icon_add(popup->e);
                  if (!e_util_icon_theme_set(o, icon_path))
                    evas_object_del(o);
                  else
                    {
                       popup->app_icon = o;
                       w = width;
                       h = height;
                    }
               }
          }

        if (!popup->app_icon)
          {
             popup->app_icon = evas_object_image_add(popup->e);
             evas_object_image_alpha_set(popup->app_icon, 1);
             evas_object_image_file_set(popup->app_icon, icon_path, NULL);

             if (evas_object_image_load_error_get(popup->app_icon))
               {
                  evas_object_del(popup->app_icon);
                  popup->app_icon = NULL;
               }
             else
               {
                  evas_object_image_size_get(popup->app_icon, &w, &h);
                  evas_object_image_fill_set(popup->app_icon, 0, 0, w, h);
               }
          }
     }
   else
     {
        img = e_notification_hint_icon_data_get(popup->notif);
        if (!img) img = e_notification_hint_image_data_get(popup->notif);
        if (img)
          {
             popup->app_icon = e_notification_image_evas_object_add(popup->e, img);
             evas_object_image_alpha_set(popup->app_icon, 1);
             evas_object_image_size_get(popup->app_icon, &w, &h);
          }
     }

   if (!popup->app_icon)
     {
        char buf[PATH_MAX];

        snprintf(buf, sizeof(buf), "%s/e-module-notification.edj", notification_mod->dir);
        popup->app_icon = edje_object_add(popup->e);
        if (!e_theme_edje_object_set(popup->app_icon, "base/theme/modules/notification",
                                     "modules/notification/logo"))
          edje_object_file_set(popup->app_icon, buf, "modules/notification/logo");
        w = width; h = height;
     }

   if (w > width || h > height)
     {
        int v;
        v = w > h ? w : h;
        h = h * height / v;
        w = w * width / v;
        evas_object_image_fill_set(popup->app_icon, 0, 0, w, h);
        evas_object_resize(popup->app_icon, w, h);
        edje_extern_object_min_size_set(popup->app_icon, w, h);
        edje_extern_object_max_size_set(popup->app_icon, w, h);
     }
   else
     {
        evas_object_resize(popup->app_icon, w, h);
        edje_extern_object_min_size_set(popup->app_icon, w, h);
        edje_extern_object_max_size_set(popup->app_icon, w, h);
     }

   edje_object_calc_force(popup->theme);
   edje_object_part_swallow(popup->theme, "notification.swallow.app_icon", popup->app_icon);
   edje_object_signal_emit(popup->theme, "notification,icon", "notification");

   /* Fill up the event message */
   _notification_format_message(popup);

   /* Compute the new size of the popup */
   edje_object_calc_force(popup->theme);
   edje_object_size_min_calc(popup->theme, &w, &h);
   e_popup_resize(popup->win, w, h);
   evas_object_resize(popup->theme, w, h);

   _notification_popups_place();
}

static Popup_Data *
_notification_popup_find(unsigned int id)
{
   Eina_List *l;
   Popup_Data *popup;

   EINA_LIST_FOREACH(notification_cfg->popups, l, popup)
     if (e_notification_id_get(popup->notif) == id)
       return popup;

   return NULL;
}

static void
_notification_popup_del(unsigned int                 id,
                        E_Notification_Closed_Reason reason)
{
   Popup_Data *popup;
   Eina_List *l, *next;
   int pos = 0;

   EINA_LIST_FOREACH(notification_cfg->popups, l, popup)
     {
        if (e_notification_id_get(popup->notif) == id)
          {
             _notification_popdown(popup, reason);
             notification_cfg->popups = eina_list_remove_list(notification_cfg->popups, l);
          }
        else
          {
             pos = _notification_popup_place(popup, pos);
          }
     }

   next_pos = pos;
}

static void
_notification_popdown(Popup_Data                  *popup,
                      E_Notification_Closed_Reason reason)
{
   if (popup->timer) ecore_timer_del(popup->timer);
   e_popup_hide(popup->win);
   evas_object_del(popup->app_icon);
   evas_object_del(popup->theme);
   e_object_del(E_OBJECT(popup->win));
   e_notification_closed_set(popup->notif, 1);
   e_notification_daemon_signal_notification_closed(notification_cfg->daemon,
                                                    e_notification_id_get(popup->notif),
                                                    reason);
   e_notification_unref(popup->notif);
   free(popup);
}

static void
_notification_format_message(Popup_Data *popup)
{
   Evas_Object *o = popup->theme;
   const char *title = e_notification_summary_get(popup->notif);
   const char *b = e_notification_body_get(popup->notif);
   edje_object_part_text_set(o, "notification.textblock.message", b);
   edje_object_part_text_set(o, "notification.text.title", title);
}

static void
_notification_theme_cb_deleted(void        *data,
                               Evas_Object *obj __UNUSED__,
                               const char  *emission __UNUSED__,
                               const char  *source __UNUSED__)
{
   Popup_Data *popup = data;
   _notification_popup_refresh(popup);
   edje_object_signal_emit(popup->theme, "notification,new", "notification");
}

static void
_notification_theme_cb_close(void        *data,
                             Evas_Object *obj __UNUSED__,
                             const char  *emission __UNUSED__,
                             const char  *source __UNUSED__)
{
   Popup_Data *popup = data;
   _notification_popup_del(e_notification_id_get(popup->notif),
                           E_NOTIFICATION_CLOSED_DISMISSED);
}

static void
_notification_theme_cb_find(void        *data,
                            Evas_Object *obj __UNUSED__,
                            const char  *emission __UNUSED__,
                            const char  *source __UNUSED__)
{
   Popup_Data *popup = data;
   Eina_List *l;

   if (!popup->app_name) return;

   for (l = e_border_client_list(); l; l = l->next)
     {
        size_t compare_len;
        E_Border *bd = l->data;

        compare_len = strlen(popup->app_name);
        if (strlen(bd->client.icccm.name) < compare_len)
          compare_len = strlen(bd->client.icccm.name);

        /* We can't be sure that the app_name really match the application name.
         * Some plugin put their name instead. But this search gives some good
         * results.
         */
        if (!strncasecmp(bd->client.icccm.name, popup->app_name, compare_len))
          {
             e_desk_show(bd->desk);
             e_border_show(bd);
             e_border_raise(bd);
             e_border_focus_set_with_pointer(bd);
             break;
          }
     }
}

static Eina_Bool
_notification_timer_cb(void *data)
{
   Popup_Data *popup = data;
   _notification_popup_del(e_notification_id_get(popup->notif),
                           E_NOTIFICATION_CLOSED_EXPIRED);
   return EINA_FALSE;
}

