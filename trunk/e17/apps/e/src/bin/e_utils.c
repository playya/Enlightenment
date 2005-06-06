/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

typedef struct _E_Util_Fake_Mouse_Up_Info E_Util_Fake_Mouse_Up_Info;

struct _E_Util_Fake_Mouse_Up_Info
{
   E_Container *con;
   int          button;
};

/* local subsystem functions */
static void _e_util_container_fake_mouse_up_cb(void *data);
static int _e_util_wakeup_cb(void *data);

/* local subsystem globals */
static Ecore_Timer *_e_util_dummy_timer = NULL;

/* externally accessible functions */
void
e_util_container_fake_mouse_up_later(E_Container *con, int button)
{
   E_Util_Fake_Mouse_Up_Info *info;
   
   info = calloc(1, sizeof(E_Util_Fake_Mouse_Up_Info));
   if (info)
     {
	info->con = con;
	info->button = button;
	e_object_ref(E_OBJECT(info->con));
	ecore_job_add(_e_util_container_fake_mouse_up_cb, info);
     }
}

void
e_util_container_fake_mouse_up_all_later(E_Container *con)
{
   e_util_container_fake_mouse_up_later(con, 1);
   e_util_container_fake_mouse_up_later(con, 2);
   e_util_container_fake_mouse_up_later(con, 3);
}

void
e_util_wakeup(void)
{
   if (_e_util_dummy_timer) return;
   _e_util_dummy_timer = ecore_timer_add(0.0, _e_util_wakeup_cb, NULL);
}

void
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

E_Zone *
e_util_zone_current_get(E_Manager *man)
{
   E_Container *con;
   
   con = e_container_current_get(man);
   if (con)
     {
	E_Zone *zone;
	
	zone = e_zone_current_get(con);
	return zone;
     }
   return NULL;
}

/* local subsystem functions */
static void
_e_util_container_fake_mouse_up_cb(void *data)
{
   E_Util_Fake_Mouse_Up_Info *info;
   
   info = data;
   if (info)
     {
	evas_event_feed_mouse_up(info->con->bg_evas, info->button, EVAS_BUTTON_NONE, NULL);
	e_object_unref(E_OBJECT(info->con));
	free(info);
     }
}

static int
_e_util_wakeup_cb(void *data)
{
   _e_util_dummy_timer = NULL;
   return 0;
}

int
e_util_utils_installed(void)
{
   return e_util_app_installed("emblem");
}

int
e_util_app_installed(char *app)
{
   char buf[PATH_MAX];
   Evas_List *list, *l;
   E_Path_Dir *dir;
   int found;

   if (!app)
     return 0;

   found = 0;
   list = e_path_dir_list_get(path_bin);
   for (l = list; l; l = l->next)
     {
	dir = l->data;
	snprintf(buf, strlen(dir->dir) + strlen(app) + 2, "%s/%s", dir->dir,
		 app); // 2 = "/" + "\0"
	if (ecore_file_exists(buf) && ecore_file_can_exec(buf))
	  {
	     found = 1;
	     break;
	  }
     }
   
   e_path_dir_list_free(list);
   return found;
}

