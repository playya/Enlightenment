/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

EAPI E_Path *path_data    = NULL;
EAPI E_Path *path_images  = NULL;
EAPI E_Path *path_fonts   = NULL;
EAPI E_Path *path_themes  = NULL;
EAPI E_Path *path_init    = NULL;
EAPI E_Path *path_icons   = NULL;
EAPI E_Path *path_modules = NULL;
EAPI E_Path *path_backgrounds = NULL;
EAPI E_Path *path_input_methods = NULL;
EAPI E_Path *path_messages = NULL;
EAPI int     restart      = 0;
EAPI int     good         = 0;
EAPI int     evil         = 0;
EAPI int     starting     = 1;
/*
typedef struct _E_Util_Fake_Mouse_Up_Info E_Util_Fake_Mouse_Up_Info;

struct _E_Util_Fake_Mouse_Up_Info
{
   E_Container *con;
   int          button;
};
*/
/* local subsystem functions */
#if 0
static void _e_util_container_fake_mouse_up_cb(void *data);
#endif
static int _e_util_wakeup_cb(void *data);

/* local subsystem globals */
static Ecore_Timer *_e_util_dummy_timer = NULL;

/* externally accessible functions */
/*EAPI void
e_util_container_fake_mouse_up_later(E_Container *con, int button)
{
   E_Util_Fake_Mouse_Up_Info *info;
   
   E_OBJECT_CHECK(con);
   E_OBJECT_TYPE_CHECK(con, E_CONTAINER_TYPE);
   
   info = calloc(1, sizeof(E_Util_Fake_Mouse_Up_Info));
   if (info)
     {
	info->con = con;
	info->button = button;
	e_object_ref(E_OBJECT(info->con));
	ecore_job_add(_e_util_container_fake_mouse_up_cb, info);
     }
}

EAPI void
e_util_container_fake_mouse_up_all_later(E_Container *con)
{
   E_OBJECT_CHECK(con);
   E_OBJECT_TYPE_CHECK(con, E_CONTAINER_TYPE);
   
   e_util_container_fake_mouse_up_later(con, 1);
   e_util_container_fake_mouse_up_later(con, 2);
   e_util_container_fake_mouse_up_later(con, 3);
}
*/
EAPI void
e_util_wakeup(void)
{
   if (_e_util_dummy_timer) return;
   _e_util_dummy_timer = ecore_timer_add(0.0, _e_util_wakeup_cb, NULL);
}

EAPI void
e_util_env_set(const char *var, const char *val)
{
   if (val)
     {
#ifdef HAVE_SETENV	
	setenv(var, val, 1);
#else
	char buf[8192];
	
	snprintf(buf, sizeof(buf), "%s=%s", var, val);
	if (getenv(var))
	  putenv(buf);
	else
	  putenv(strdup(buf));
#endif	
     }
   else
     {
#ifdef HAVE_UNSETENV	
	unsetenv(var);
#else
	if (getenv(var)) putenv(var);
#endif	
     }
}
/*
EAPI E_Zone *
e_util_zone_current_get(E_Manager *man)
{
   E_Container *con;
   
   E_OBJECT_CHECK_RETURN(man, NULL);
   E_OBJECT_TYPE_CHECK_RETURN(man, E_MANAGER_TYPE, NULL);
   con = e_container_current_get(man);
   if (con)
     {
	E_Zone *zone;
	
	zone = e_zone_current_get(con);
	return zone;
     }
   return NULL;
}
*/
EAPI int
e_util_utils_installed(void)
{
   return ecore_file_app_installed("emblem");
}

EAPI int
e_util_app_installed(char *app) 
{
   return ecore_file_app_installed(app);
}

EAPI int
e_util_glob_match(const char *str, const char *glob)
{
   if (!str || !glob)
     return 0;
   if (glob[0] == 0)
     {
	if (str[0] == 0) return 1;
	return 0;
     }
   if (!strcmp(glob, "*")) return 1;
   if (!fnmatch(glob, str, 0)) return 1;
   return 0;
}

EAPI int
e_util_glob_case_match(const char *str, const char *glob)
{
   const char *p;
   char *tstr, *tglob, *tp;
   
   if (glob[0] == 0)
     {
	if (str[0] == 0) return 1;
	return 0;
     }
   if (!strcmp(glob, "*")) return 1;
   tstr = alloca(strlen(str) + 1);
   for (tp = tstr, p = str; *p != 0; p++, tp++) *tp = tolower(*p);
   *tp = 0;
   tglob = alloca(strlen(glob) + 1);
   for (tp = tglob, p = glob; *p != 0; p++, tp++) *tp = tolower(*p);
   *tp = 0;
   if (!fnmatch(tglob, tstr, 0)) return 1;
   return 0;
}
/*
EAPI E_Container *
e_util_container_number_get(int num)
{
   Eina_List *l;
   
   for (l = e_manager_list(); l; l = l->next)
     {
	E_Manager *man;
	E_Container *con;
	
	man = l->data;
	con = e_container_number_get(man, num);
	if (con) return con;
     }
   return NULL;
}

EAPI E_Zone *
e_util_container_zone_number_get(int con_num, int zone_num)
{
   E_Container *con;
   
   con = e_util_container_number_get(con_num);
   if (!con) return NULL;
   return e_container_zone_number_get(con, zone_num);
}

EAPI int
e_util_head_exec(int head, const char *cmd)
{
   char *penv_display;
   char *p1, *p2;
   char buf[4096], buf2[32];
   int ok = 0;
   Ecore_Exe *exe;
   
   penv_display = getenv("DISPLAY");
   if (penv_display) penv_display = strdup(penv_display);
   * set env vars *
   p1 = strrchr(penv_display, ':');
   p2 = strrchr(penv_display, '.');
   if ((p1) && (p2) && (p2 > p1)) * "blah:x.y" *
     {
	* yes it could overflow... but who will overflow DISPLAY eh? why? to
	 * "exploit" your own applications running as you?
	 *
	strcpy(buf, penv_display);
	buf[p2 - penv_display + 1] = 0;
	snprintf(buf2, sizeof(buf2), "%i", head);
	strcat(buf, buf2);
     }
   else if (p1) * "blah:x *
     {
	strcpy(buf, penv_display);
	snprintf(buf2, sizeof(buf2), ".%i", head);
	strcat(buf, buf2);
     }
   else
     strcpy(buf, penv_display);
   
   ok = 1;
   exe = ecore_exe_run(cmd, NULL);
   if (!exe)
     {
	e_util_dialog_show(_("Run Error"),
			   _("Enlightenment was unable to fork a child process:<br>"
			     "<br>"
			     "%s<br>"),
			   cmd);
	ok = 0;
     }
   
   * reset env vars *
   if (penv_display)
     {
	e_util_env_set("DISPLAY", penv_display);
	free(penv_display);
     }
   return ok;
}
*/
EAPI int
e_util_strcmp(const char *s1, const char *s2)
{
   if ((s1) && (s2))
     return strcmp(s1, s2);
   return 0x7fffffff;
}

EAPI int
e_util_both_str_empty(const char *s1, const char *s2)
{
   int empty = 0;
   
   if ((!s1) && (!s2)) return 1;
   if ((!s1) || ((s1) && (s1[0] == 0))) empty++;
   if ((!s2) || ((s2) && (s2[0] == 0))) empty++;
   if (empty == 2) return 1;
   return 0;
}
/*
EAPI int
e_util_immortal_check(void)
{
   Eina_List *wins;
   
   wins = e_border_immortal_windows_get();
   if (wins)
     {
	e_util_dialog_show(_("Cannot exit - immortal windows."),
			   _("Some windows are left still around with the Lifespan lock enabled. This means<br>"
			     "that Enlightenment will not allow itself to exit until these windows have<br>"
			     "been closed or have the lifespan lock removed.<br>"));
	* FIXME: should really display a list of these lifespan locked *
	* windows in a dialog and let the user disable their locks in *
	* this dialog *
	eina_list_free(wins);
	return 1;
     }
   return 0;
}

EAPI int
e_util_edje_icon_list_set(Evas_Object *obj, const char *list)
{
   char *buf;
   const char *p;
   const char *c;
   
   if ((!list) || (!list[0])) return 0;
   buf = alloca(strlen(list) + 1);
   p = list;
   while (p)
     {
	c = strchr(p, ',');
	if (c)
	  {
	     strncpy(buf, p, c - p);
	     buf[c - p] = 0;
	     if (e_util_edje_icon_set(obj, buf)) return 1;
	     p = c + 1;
	     if (!*p) return 0;
	  }
	else
	  {
	     strcpy(buf, p);
	     if (e_util_edje_icon_set(obj, buf)) return 1;
	     return 0;
	  }
     }
   return 0;
}

EAPI int
e_util_menu_item_edje_icon_list_set(E_Menu_Item *mi, const char *list)
{
   char *buf;
   const char *p;
   char *c;
   
   if ((!list) || (!list[0])) return 0;
   buf = alloca(strlen(list) + 1);
   p = list;
   while (p)
     {
	c = strchr(p, ',');
	if (c)
	  {
	     strncpy(buf, p, c - p);
	     buf[c - p] = 0;
	     if (e_util_menu_item_edje_icon_set(mi, buf)) return 1;
	     p = c + 1;
	     if (!*p) return 0;
	  }
	else
	  {
	     strcpy(buf, p);
	     if (e_util_menu_item_edje_icon_set(mi, buf)) return 1;
	     return 0;
	  }
     }
   return 0;
}

EAPI int
e_util_edje_icon_set(Evas_Object *obj, const char *name)
{
   const char *file;
   char buf[4096];

   if ((!name) || (!name[0])) return 0;
   snprintf(buf, sizeof(buf), "e/icons/%s", name);
   file = e_theme_edje_file_get("base/theme/icons", buf);
   if (file[0])
     {
	edje_object_file_set(obj, file, buf);
	return 1;
     }
   return 0;
}

EAPI int
e_util_menu_item_edje_icon_set(E_Menu_Item *mi, const char *name)
{
   char *file;
   char buf[4096];
   
   if ((!name) || (!name[0])) return 0;
   snprintf(buf, sizeof(buf), "e/icons/%s", name);
   file = (char *)e_theme_edje_file_get("base/theme/icons", buf);
   if (file[0])
     {
	e_menu_item_icon_edje_set(mi, file, buf);
	return 1;
     }
   return 0;
}

EAPI E_Container *
e_util_container_window_find(Ecore_X_Window win)
{
   Eina_List *l, *ll;

   for (l = e_manager_list(); l; l = l->next)
     {
	E_Manager *man;
	
	man = l->data;
	for (ll = man->containers; ll; ll = ll->next)
          {
	     E_Container *con;
	     
	     con = ll->data;
	     if ((con->win == win) || (con->bg_win == win) || 
		 (con->event_win == win))
	       return con;
	  }
     }
   return NULL;
}

EAPI E_Border *
e_util_desk_border_above(E_Border *bd)
{
   E_Border *above = NULL;
   Eina_List *l;
   int pos, i;

   E_OBJECT_CHECK_RETURN(bd, NULL);
   E_OBJECT_TYPE_CHECK_RETURN(bd, E_BORDER_TYPE, NULL);
   
   if (bd->layer == 0) pos = 0;
   else if ((bd->layer > 0) && (bd->layer <= 50)) pos = 1;
   else if ((bd->layer > 50) && (bd->layer <= 100)) pos = 2;
   else if ((bd->layer > 100) && (bd->layer <= 150)) pos = 3;
   else if ((bd->layer > 150) && (bd->layer <= 200)) pos = 4;
   else pos = 5;

   for (l = eina_list_data_find_list(bd->zone->container->layers[pos].clients, bd);
	(l) && (l->next) && (!above);
	l = l->next)
     {
	above = l->next->data;
	if ((above->desk != bd->desk) && (!above->sticky))
	  above = NULL;
     }
   if (!above)
     {
	* Need to check the layers above *
	for (i = pos + 1; (i < 7) && (!above); i++)
	  {
	     for (l = bd->zone->container->layers[i].clients;
		  (l) && (!above);
		  l = l->next)
	       {
		  above = l->data;
		  if ((above->desk != bd->desk) && (!above->sticky))
		    above = NULL;
	       }
	  }
     }
   return above;
}

EAPI E_Border *
e_util_desk_border_below(E_Border *bd)
{
   E_Border *below = NULL;
   Eina_List *l;
   int pos, i;

   E_OBJECT_CHECK_RETURN(bd, NULL);
   E_OBJECT_TYPE_CHECK_RETURN(bd, E_BORDER_TYPE, NULL);
   
   if (bd->layer == 0) pos = 0;
   else if ((bd->layer > 0) && (bd->layer <= 50)) pos = 1;
   else if ((bd->layer > 50) && (bd->layer <= 100)) pos = 2;
   else if ((bd->layer > 100) && (bd->layer <= 150)) pos = 3;
   else if ((bd->layer > 150) && (bd->layer <= 200)) pos = 4;
   else pos = 5;

   for (l = eina_list_data_find_list(bd->zone->container->layers[pos].clients, bd);
	(l) && (l->prev) && (!below);
	l = l->prev)
     {
	below = l->prev->data;
	if ((below->desk != bd->desk) && (!below->sticky))
	  below = NULL;
     }
   if (!below)
     {
	* Need to check the layers below *
	for (i = pos - 1; (i >= 0) && (!below); i--)
	  {
	     if (bd->zone->container->layers[i].clients)
	       {
		  for (l = eina_list_last(bd->zone->container->layers[i].clients);
		       (l) && (!below);
		       l = l->prev)
		    {
		       below = l->data;
		       if ((below->desk != bd->desk) && (!below->sticky))
			 below = NULL;
		    }
	       }
	  }
     }

   return below;
}
*/
EAPI int
e_util_edje_collection_exists(const char *file, const char *coll)
{
   Eina_List *clist, *l;
   
   clist = edje_file_collection_list(file);
   for (l = clist; l; l = l->next)
     {
	if (!strcmp(coll, l->data))
	  {
	     edje_file_collection_list_free(clist);
	     return 1;
	  }
     }
   edje_file_collection_list_free(clist);
   return 0;
}
/*
EAPI void
e_util_dialog_internal(const char *title, const char *txt)
{
   E_Dialog *dia;
   
   dia = e_dialog_new(e_container_current_get(e_manager_current_get()), "E", "_error_dialog");
   if (!dia) return;
   e_dialog_title_set(dia, title);
   e_dialog_text_set(dia, txt);
   e_dialog_icon_set(dia, "enlightenment/error", 64);
   e_dialog_button_add(dia, _("OK"), NULL, NULL, NULL);
   e_dialog_button_focus_num(dia, 0);
   e_win_centered_set(dia->win, 1);
   e_dialog_show(dia);
}
*/
EAPI const char *
e_util_filename_escape(const char *filename)
{
   const char *p;
   char *q;
   static char buf[4096];
   
   p = filename;
   q = buf;
   while (*p)
     {
	if ((q - buf) > 4090) return NULL;
	if (
	    (*p == ' ') || (*p == '\t') || (*p == '\n') ||
	    (*p == '\\') || (*p == '\'') || (*p == '\"') ||
	    (*p == ';') || (*p == '!') || (*p == '#') ||
	    (*p == '$') || (*p == '%') || (*p == '&') ||
	    (*p == '*') || (*p == '(') || (*p == ')') ||
	    (*p == '[') || (*p == ']') || (*p == '{') ||
	    (*p == '}') || (*p == '|') || (*p == '<') ||
	    (*p == '>') || (*p == '?')
	    )
	  {
	     *q = '\\';
	     q++;
	  }
	*q = *p;
	q++;
	p++;
     }
   *q = 0;
   return buf;
}

EAPI int
e_util_icon_save(Ecore_X_Icon *icon, const char *filename)
{
   Ecore_Evas  *ee;
   Evas        *evas;
   Evas_Object *im;
   int          ret;

   ee = ecore_evas_buffer_new(icon->width, icon->height);
   if (!ee) return 0;
   evas = ecore_evas_get(ee);
   evas_image_cache_set(evas, 0);
   evas_font_cache_set(evas, 0);

   im = evas_object_image_add(evas);
   if (!im)
     {
	ecore_evas_free(ee);
	return 0;
     }
   evas_object_move(im, 0, 0);
   evas_object_resize(im, icon->width, icon->height);
   evas_object_image_size_set(im, icon->width, icon->height);
   evas_object_image_data_copy_set(im, icon->data);
   evas_object_image_alpha_set(im, 1);
   evas_object_show(im);
   ret = evas_object_image_save(im, filename, NULL, NULL);
   evas_object_del(im);
   ecore_evas_free(ee);
   return ret;
}

EAPI char *
e_util_shell_env_path_eval(char *path)
{
   /* evaluate things like:
    * $HOME/bling -> /home/user/bling
    * $HOME/bin/$HOSTNAME/blah -> /home/user/bin/localhost/blah
    * etc. etc.
    */
   char buf[4096], *pd, *p, *v1, *v2, *s, *v, *vp;
   int esc = 0, invar = 0;

   for (p = path, pd = buf; (pd < (buf + sizeof(buf) - 1)); p++)
     {
	if (invar)
	  {
	     if (!((isalnum(*p)) || (*p == '_')))
	       {
		  v2 = p;
		  invar = 0;
		  if ((v2 - v1) > 1)
		    {
		       s = alloca(v2 - v1);
		       strncpy(s, v1 + 1, v2 - v1 - 1);
		       s[v2 - v1 - 1] = 0;
		       v = getenv(s);
		       if (v)
			 {
			    vp = v;
			    while ((*vp) && (pd < (buf + sizeof(buf) - 1)))
			      {
				 *pd = *vp;
				 vp++;
				 pd++;
			      }
			 }
		    }
		  if (pd < (buf + sizeof(buf) - 1))
		    {
		       *pd = *p;
		       pd++;
		    }
	       }
	  }
	else
	  {
	     if (esc)
	       {
		  *pd = *p;
		  pd++;
	       }
	     else
	       {
		  if (*p == '\\') esc = 1;
		  else if (*p == '$')
		    {
		       invar = 1;
		       v1 = p;
		    }
		  else
		    {
		       *pd = *p;
		       pd++;
		    }
	       }
	  }
	if (*p == 0) break;
     }
   *pd = 0;
   return strdup(buf);
}

/* local subsystem functions */
/*static void
_e_util_container_fake_mouse_up_cb(void *data)
{
   E_Util_Fake_Mouse_Up_Info *info;
   
   info = data;
   if (info)
     {
	evas_event_feed_mouse_up(info->con->bg_evas, info->button, EVAS_BUTTON_NONE, ecore_x_current_time_get(), NULL);
	e_object_unref(E_OBJECT(info->con));
	free(info);
     }
}
*/
static int
_e_util_wakeup_cb(void *data)
{
   _e_util_dummy_timer = NULL;
   return 0;
}

