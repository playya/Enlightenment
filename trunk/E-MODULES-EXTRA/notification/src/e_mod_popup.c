/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e_mod_main.h"

/* Popup function protos */
static Popup_Data *_notification_popup_new      (E_Notification *n);
static Popup_Data *_notification_popup_find     (unsigned int id);
static void        _notification_popup_place    (Popup_Data *popup, int num);
static void        _notification_popup_refresh  (Popup_Data *popup);
static void        _notification_popup_del      (unsigned int id, 
                                                 E_Notification_Closed_Reason reason);
static void        _notification_popdown        (Popup_Data *popup, 
                                                 E_Notification_Closed_Reason reason);

/* Util function protos */
static void _notification_format_message    (Popup_Data *popup);

/* Callbacks */
static void _notification_theme_cb_deleted  (void *data, 
                                             Evas_Object *obj, 
                                             const char *emission, 
                                             const char *source);
static void _notification_theme_cb_close    (void *data, 
                                             Evas_Object *obj, 
                                             const char *emission, 
                                             const char *source);
static void _notification_theme_cb_find     (void *data, 
                                             Evas_Object *obj, 
                                             const char *emission, 
                                             const char *source);
static int  _notification_timer_cb          (void *data);

int
notification_popup_notify(E_Notification *n, 
                          unsigned int replaces_id, 
                          unsigned int id __UNUSED__,
                          const char *appname)
{
   int timeout;
   Popup_Data *popup = NULL;
   char urgency;

   urgency = e_notification_hint_urgency_get(n);
   if (urgency == E_NOTIFICATION_URGENCY_LOW && !notification_cfg->show_low)
     return 0;
   else if (urgency == E_NOTIFICATION_URGENCY_NORMAL && !notification_cfg->show_normal)
     return 0;
   else if (urgency == E_NOTIFICATION_URGENCY_CRITICAL && !notification_cfg->show_critical)
     return 0;

   if (replaces_id && (popup = _notification_popup_find(replaces_id))) 
     {
	e_notification_ref(n);
	if (popup->notif)
	  {
	     /* const char *body_old = e_notification_body_get(popup->notif); */
	     /* const char *body_new = e_notification_body_get(n);
	      * char *body_final;
	      * int body_old_len, body_new_len;
	      * 
	      * body_old_len = strlen(body_old);
	      * body_new_len = strlen(body_new);
	      * body_final = alloca(body_old_len + body_new_len + 2);
	      * sprintf(body_final, "%s\n%s", body_old, body_new); */
	     /* e_notification_body_set(n, body_new); */

	     e_notification_unref(popup->notif);
	  }
	popup->notif = n;
	/* edje_object_signal_emit(popup->theme, "notification,del", "notification"); */
	_notification_popup_refresh(popup);
	/* edje_object_signal_emit(popup->theme, "notification,new", "notification"); */
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
   if ((timeout == 0 || timeout == -1) && notification_cfg->timeout == 0.0)
     return 1;
   else
     popup->timer = ecore_timer_add(timeout == -1 ? notification_cfg->timeout : (float)timeout / 1000, 
				    _notification_timer_cb, 
				    popup);
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
_notification_popup_new(E_Notification *n)
{
   E_Container *con;
   Popup_Data *popup;
   char buf[PATH_MAX];
   int shaped;

   popup = calloc(1, sizeof(Popup_Data));
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

   shaped = 0;
   if (e_config->use_composite)
     {
	if (e_theme_edje_object_set(popup->theme,
				    "base/theme/modules/notification",
				    "modules/notification/main/alpha"))
	  shaped = 1;
	else if (edje_object_file_set(popup->theme, buf,
				      "modules/notification/main/alpha"))
	  shaped = 1;
     }

   if ((!e_config->use_composite) ||
       (edje_object_load_error_get(popup->theme) != EDJE_LOAD_ERROR_NONE))
     {
	const char *shape_option;

	if (!e_theme_edje_object_set(popup->theme,
				     "base/theme/modules/notification",
				     "modules/notification/main"))
	  edje_object_file_set(popup->theme, buf, "modules/notification/main");

	shape_option = edje_object_data_get(popup->theme, "shaped");
	if (shape_option)
	  {
	     if (!strcmp(shape_option, "1"))
	       shaped = 1;
	     else
	       shaped = 0;
	  }
     }

   if (e_config->use_composite)
     {
	ecore_evas_alpha_set(popup->win->ecore_evas, shaped);
     }
   else
     {
	ecore_evas_shaped_set(popup->win->ecore_evas, shaped);
	ecore_evas_avoid_damage_set(popup->win->ecore_evas, shaped);
     }

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
   _notification_popup_place(popup, eina_list_count(notification_cfg->popups));
   e_popup_show(popup->win);
   e_popup_layer_set(popup->win, 999);

   return popup;
}

static void
_notification_popup_place(Popup_Data *popup, int num)
{
   int x, y, w, h, dir = 0;
   E_Container *con;
   
   con = e_container_current_get(e_manager_current_get());
   evas_object_geometry_get(popup->theme, NULL, NULL, &w, &h);
   int gap = 10;
   int to_edge = 15;
   
   /* if (e_notification_hint_xy_get(popup->notif, &x, &y))
    *   {
    * 	E_Container *con;
    * 	con = e_container_current_get(e_manager_current_get());
    * 
    * 	if (x + w > con->w)
    * 	  x -= w;
    * 	if (y + h > con->h)
    * 	  y -= h;
    * 	e_popup_move(popup->win, x, y);
    *   }
    * else
    *   { */

   switch (notification_cfg->corner)
     {
      case CORNER_TL:
	 e_popup_move(popup->win,
		      to_edge, to_edge + num * (h + gap));
	 break;
      case CORNER_TR:
	 e_popup_move(popup->win,
		      con->w - (w + to_edge) ,
		      to_edge + num * (h + gap));
	 break;
      case CORNER_BL:
	 e_popup_move(popup->win,
		      to_edge,
		      (con->h - h) - (to_edge + num * (h + gap)));
	 break;
      case CORNER_BR:
	 e_popup_move(popup->win,
		      con->w - (w + to_edge),
		      (con->h - h) - (to_edge + num * (h + gap)));
	 break;
     }
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
	if ((errno != 0 && width == 0) || endptr == app_icon_max) 
	  {
	     width = 80;
	     height = 80;
	  }
	else
	  {
	     endptr++;
	     if (endptr) height = strtol(endptr, NULL, 10);
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
   else if ((img = e_notification_hint_icon_data_get(popup->notif)))
     {
	popup->app_icon = e_notification_image_evas_object_add(popup->e, img);
	evas_object_image_size_get(popup->app_icon, &w, &h);
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
}

static Popup_Data *
_notification_popup_find(unsigned int id)
{
   Eina_List *l;
   Popup_Data *popup;

   EINA_LIST_FOREACH (notification_cfg->popups, l, popup)
     if (e_notification_id_get(popup->notif) == id)
       return popup;

   return NULL;
}

static void
_notification_popup_del(unsigned int id, E_Notification_Closed_Reason reason)
{
   Popup_Data *popup;
   Eina_List *l, *next;
   int i;

   for (l = notification_cfg->popups, i = 0; l && (popup = l->data); l = next)
     {
	next = l->next;
	if (e_notification_id_get(popup->notif) == id)
	  {
	     _notification_popdown(popup, reason);
	     notification_cfg->popups = eina_list_remove_list(notification_cfg->popups, l);
	  }
	else
	  {
	     _notification_popup_place(popup, i);
	     i++;
	  }
     }
}

static void
_notification_popdown(Popup_Data *popup, E_Notification_Closed_Reason reason)
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

/* static char *
 * _str_append(char *str, const char *txt, int *len, int *alloc)
 * {
 *    int txt_len = strlen(txt);
 * 
 *    if (txt_len <= 0) return str;
 *    if ((*len + txt_len) >= *alloc)
 *      {
 * 	char *str2;
 * 	int alloc2;
 * 
 * 	alloc2 = *alloc + txt_len + 128;
 * 	str2 = realloc(str, alloc2);
 * 	if (!str2) return str;
 * 	*alloc = alloc2;
 * 	str = str2;
 *      }
 *    strcpy(str + *len, txt);
 *    *len += txt_len;
 *    return str;
 * }
 * 
 * static char *
 * _text_to_mkup(const char *text)
 * {
 *    char *str = NULL;
 *    int str_len = 0, str_alloc = 0;
 *    int ch, pos = 0, pos2 = 0;
 * 
 *    if (!text) return NULL;
 *    for (;;)
 *      {
 * 	pos = pos2;
 *         pos2 = evas_string_char_next_get((char *)(text), pos2, &ch);
 *         if ((ch <= 0) || (pos2 <= 0)) break;
 * 	if (ch == '\n')
 *           str = _str_append(str, "<br>", &str_len, &str_alloc);
 * 	else if (ch == '\t')
 *           str = _str_append(str, "<\t>", &str_len, &str_alloc);
 * 	else if (ch == '<')
 *           str = _str_append(str, "&lt;", &str_len, &str_alloc);
 * 	else if (ch == '>')
 *           str = _str_append(str, "&gt;", &str_len, &str_alloc);
 * 	else if (ch == '&')
 *           str = _str_append(str, "&amp;", &str_len, &str_alloc);
 * 	else
 * 	  {
 * 	     char tstr[16];
 * 
 * 	     strncpy(tstr, text + pos, pos2 - pos);
 * 	     tstr[pos2 - pos] = 0;
 * 	     str = _str_append(str, tstr, &str_len, &str_alloc);
 * 	  }
 *      }
 *    return str;
 * } */

static void
_notification_format_message(Popup_Data *popup)
{
   Evas_Object *o = popup->theme;
   const char *title = e_notification_summary_get(popup->notif);
   const char *b = e_notification_body_get(popup->notif);
   edje_object_part_text_set(o, "notification.textblock.message", b);
   edje_object_part_text_set(o, "notification.text.title", title);
   /* char *message = _text_to_mkup(b); */
   /* if (message)
    *   {
    * 	edje_object_part_text_set(o, "notification.textblock.message", message);
    * 	free(message);
    *   } */
}

static void
_notification_theme_cb_deleted(void *data, 
                               Evas_Object *obj __UNUSED__, 
                               const char *emission __UNUSED__, 
                               const char *source __UNUSED__)
{
   Popup_Data *popup = data;
   _notification_popup_refresh(popup);
   edje_object_signal_emit(popup->theme, "notification,new", "notification");
}

static void
_notification_theme_cb_close(void *data, 
                             Evas_Object *obj __UNUSED__, 
                             const char *emission __UNUSED__, 
                             const char *source __UNUSED__)
{
   Popup_Data *popup = data;
   _notification_popup_del(e_notification_id_get(popup->notif), 
			   E_NOTIFICATION_CLOSED_DISMISSED);
}

static void
_notification_theme_cb_find(void *data, 
                            Evas_Object *obj __UNUSED__, 
                            const char *emission __UNUSED__, 
                            const char *source __UNUSED__)
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

static int
_notification_timer_cb(void *data)
{
   Popup_Data *popup = data;
   _notification_popup_del(e_notification_id_get(popup->notif), 
			   E_NOTIFICATION_CLOSED_EXPIRED);
   return 0;
}

