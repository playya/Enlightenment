/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

/* IGNORE this code for now! */

typedef struct _E_Fm2_Smart_Data E_Fm2_Smart_Data;
typedef struct _E_Fm2_Region     E_Fm2_Region;
typedef struct _E_Fm2_Icon       E_Fm2_Icon;

struct _E_Fm2_Smart_Data
{
   Evas_Coord        x, y, w, h;
   Evas_Object      *obj;
   Evas_Object      *clip;
   char             *dev;
   char             *path;
   char             *realpath;
   E_Fm2_View_Mode   view_mode;
   struct {
      int            w, h;
      int            list_w, list_h;
      unsigned char  fixed_w : 1;
      unsigned char  fixed_h : 1;
   } icon;
   struct {
      Evas_Coord     w, h;
   } max;
   struct {
      Evas_Coord     x, y;
   } pos;
   struct {
      Evas_List     *list;
      int            member_max;
   } regions;
   Evas_List        *icons;
   Evas_List        *queue;
   Ecore_Idler      *scan_idler;
   Ecore_Timer      *scan_timer;
   Ecore_Job        *scroll_job;
   Ecore_Job        *resize_job;
   DIR              *dir;
   unsigned char     no_case_sort : 1;
   unsigned char     iconlist_changed : 1;
   unsigned char     show_extension : 1;
   unsigned char     dirs_first : 1;
   unsigned char     dirs_last : 1;
   unsigned char     single_select : 1;
   unsigned char     windows_multi_select_modifiers : 1;
//   unsigned char     no_dnd : 1;
//   unsigned char     single_click : 1;
};

struct _E_Fm2_Region
{
   E_Fm2_Smart_Data *sd;
   Evas_Coord        x, y, w, h;
   Evas_List        *list;
   unsigned char     realized : 1;
};

struct _E_Fm2_Icon
{
   E_Fm2_Smart_Data *sd;
   E_Fm2_Region     *region;
   Evas_Coord        x, y, w, h, min_w, min_h;
   Evas_Object      *obj, *obj_icon;
   int               saved_x, saved_y;
   int               saved_rel;
   char             *file;
   char             *mime;
   struct stat       st;
   unsigned char     realized : 1;
   unsigned char     selected : 1;
   unsigned char     last_selected : 1;
   unsigned char     saved_pos : 1;
   unsigned char     odd : 1;
//   unsigned char     single_click : 1;
};

static char *_e_fm2_dev_path_map(char *dev, char *path);
static void _e_fm2_file_add(Evas_Object *obj, char *file);
static void _e_fm2_file_del(Evas_Object *obj, char *file);
static void _e_fm2_scan_start(Evas_Object *obj);
static void _e_fm2_scan_stop(Evas_Object *obj);
static void _e_fm2_queue_process(Evas_Object *obj); 
static void _e_fm2_queue_free(Evas_Object *obj);
static void _e_fm2_regions_free(Evas_Object *obj);
static void _e_fm2_regions_populate(Evas_Object *obj);
static void _e_fm2_icons_place(Evas_Object *obj);
static void _e_fm2_icons_free(Evas_Object *obj);
static void _e_fm2_regions_eval(Evas_Object *obj);

static E_Fm2_Icon *_e_fm2_icon_new(E_Fm2_Smart_Data *sd, char *file);
static void _e_fm2_icon_free(E_Fm2_Icon *ic);
static void _e_fm2_icon_realize(E_Fm2_Icon *ic);
static void _e_fm2_icon_unrealize(E_Fm2_Icon *ic);
static int _e_fm2_icon_visible(E_Fm2_Icon *ic);
static void _e_fm2_icon_label_set(E_Fm2_Icon *ic, Evas_Object *obj);
static void _e_fm2_icon_icon_set(E_Fm2_Icon *ic);
static void _e_fm2_icon_select(E_Fm2_Icon *ic);
static void _e_fm2_icon_deselect(E_Fm2_Icon *ic);

static E_Fm2_Region *_e_fm2_region_new(E_Fm2_Smart_Data *sd);
static void _e_fm2_region_free(E_Fm2_Region *rg);
static void _e_fm2_region_realize(E_Fm2_Region *rg);
static void _e_fm2_region_unrealize(E_Fm2_Region *rg);
static int _e_fm2_region_visible(E_Fm2_Region *rg);

static void _e_fm2_cb_icon_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _e_fm2_cb_icon_mouse_up(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _e_fm2_cb_icon_mouse_move(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _e_fm2_cb_icon_thumb_gen(void *data, Evas_Object *obj, void *event_info);
static void _e_fm2_cb_scroll_job(void *data);
static void _e_fm2_cb_resize_job(void *data);
static int _e_fm2_cb_icon_sort(void *data1, void *data2);
static int _e_fm2_cb_scan_idler(void *data);
static int _e_fm2_cb_scan_timer(void *data);

static void _e_fm2_obj_icons_place(E_Fm2_Smart_Data *sd);

static void _e_fm2_smart_add(Evas_Object *object);
static void _e_fm2_smart_del(Evas_Object *object);
static void _e_fm2_smart_move(Evas_Object *object, Evas_Coord x, Evas_Coord y);
static void _e_fm2_smart_resize(Evas_Object *object, Evas_Coord w, Evas_Coord h);
static void _e_fm2_smart_show(Evas_Object *object);
static void _e_fm2_smart_hide(Evas_Object *object);
static void _e_fm2_smart_color_set(Evas_Object *obj, int r, int g, int b, int a);
static void _e_fm2_smart_clip_set(Evas_Object *obj, Evas_Object * clip);
static void _e_fm2_smart_clip_unset(Evas_Object *obj);

static char *_meta_path = NULL;
static Evas_Smart *_e_fm2_smart = NULL;

/* externally accessible functions */
EAPI int
e_fm2_init(void)
{
   char *homedir;
   char  path[PATH_MAX];

   homedir = e_user_homedir_get();
   if (homedir)
     {
	snprintf(path, sizeof(path), "%s/.e/e/fileman/metadata", homedir);
	ecore_file_mkpath(path);
	_meta_path = strdup(path);
	free(homedir);
     }
   else return 0;

   _e_fm2_smart = evas_smart_new("e_fm",
				 _e_fm2_smart_add, /* add */
				 _e_fm2_smart_del, /* del */
				 NULL, NULL, NULL, NULL, NULL,
				 _e_fm2_smart_move, /* move */
				 _e_fm2_smart_resize, /* resize */
				 _e_fm2_smart_show,/* show */
				 _e_fm2_smart_hide,/* hide */
				 _e_fm2_smart_color_set, /* color_set */
				 _e_fm2_smart_clip_set, /* clip_set */
				 _e_fm2_smart_clip_unset, /* clip_unset */
				 NULL); /* data*/
   return 1;
}

EAPI int
e_fm2_shutdown(void)
{
   evas_smart_free(_e_fm2_smart);
   _e_fm2_smart = NULL;
   E_FREE(_meta_path);
   return 1;
}

EAPI Evas_Object *
e_fm2_add(Evas *evas)
{
   return evas_object_smart_add(evas, _e_fm2_smart);
}

EAPI void
e_fm2_path_set(Evas_Object *obj, char *dev, char *path)
{
   E_Fm2_Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return; // safety
   if (!evas_object_type_get(obj)) return; // safety
   if (strcmp(evas_object_type_get(obj), "e_fm")) return; // safety

   /* we split up files into regions. each region can have a maximum of 128 
    * icons and we realize/unrealize whole regions at once when that region 
    * becomes visible - this saves of object  count and memory */
   sd->regions.member_max = 32;
//   sd->view_mode = E_FM2_VIEW_MODE_ICONS;
   sd->view_mode = E_FM2_VIEW_MODE_LIST;
   sd->icon.w = 64;
   sd->icon.h = 64;
   sd->icon.list_w = 24;
   sd->icon.list_h = 24;
   sd->icon.fixed_w = 0;
   sd->icon.fixed_h = 1;
   sd->no_case_sort = 1;
   sd->show_extension = 0;
   sd->dirs_first = 1;
   sd->dirs_last = 1;
   sd->single_select = 0;
   sd->windows_multi_select_modifiers = 0;
   
   _e_fm2_scan_stop(obj);
   _e_fm2_queue_free(obj);
   _e_fm2_regions_free(obj);
   _e_fm2_icons_free(obj);
   E_FREE(sd->dev);
   E_FREE(sd->path);
   E_FREE(sd->realpath);
   if (dev) sd->dev = strdup(dev);
   sd->path = strdup(path);
   sd->realpath = _e_fm2_dev_path_map(sd->dev, sd->path);
   // FIXME: begin dir scan/build
   printf("FM: %s\n", sd->realpath);
   _e_fm2_scan_start(obj);
}

EAPI void
e_fm2_path_get(Evas_Object *obj, char **dev, char **path)
{
   E_Fm2_Smart_Data *sd;

   if (dev) *dev = NULL;
   if (path) *path = NULL;
   sd = evas_object_smart_data_get(obj);
   if (!sd) return; // safety
   if (!evas_object_type_get(obj)) return; // safety
   if (strcmp(evas_object_type_get(obj), "e_fm")) return; // safety
   if (dev) *dev = sd->dev;
   if (path) *path = sd->path;
}

EAPI void
e_fm2_pan_set(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   E_Fm2_Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return; // safety
   if (!evas_object_type_get(obj)) return; // safety
   if (strcmp(evas_object_type_get(obj), "e_fm")) return; // safety
   if (x > (sd->max.w - sd->w)) x = sd->max.w - sd->w;
   if (x < 0) x = 0;
   if (y > (sd->max.h - sd->h)) y = sd->max.h - sd->h;
   if (y < 0) y = 0;
   if ((sd->pos.x == x) && (sd->pos.y == y)) return;
   sd->pos.x = x;
   sd->pos.y = y;
   printf("POS %i %i\n", x, y);
   if (sd->scroll_job) ecore_job_del(sd->scroll_job);
   sd->scroll_job = ecore_job_add(_e_fm2_cb_scroll_job, obj);
}

EAPI void
e_fm2_pan_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y)
{
   E_Fm2_Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return; // safety
   if (!evas_object_type_get(obj)) return; // safety
   if (strcmp(evas_object_type_get(obj), "e_fm")) return; // safety
   if (x) *x = sd->pos.x;
   if (y) *y = sd->pos.y;
}

EAPI void
e_fm2_pan_max_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y)
{
   E_Fm2_Smart_Data *sd;
   Evas_Coord mx, my;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return; // safety
   if (!evas_object_type_get(obj)) return; // safety
   if (strcmp(evas_object_type_get(obj), "e_fm")) return; // safety
   mx = sd->max.w - sd->w;
   if (mx < 0) mx = 0;
   my = sd->max.h - sd->h;
   if (my < 0) my = 0;
   if (x) *x = mx;
   if (y) *y = my;
}

EAPI void
e_fm2_pan_child_size_get(Evas_Object *obj, Evas_Coord *w, Evas_Coord *h)
{
   E_Fm2_Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return; // safety
   if (!evas_object_type_get(obj)) return; // safety
   if (strcmp(evas_object_type_get(obj), "e_fm")) return; // safety
   if (w) *w = sd->max.w;
   if (h) *h = sd->max.h;
}




/* local subsystem functions */
static char *
_e_fm2_dev_path_map(char *dev, char *path)
{
   char buf[4096] = "", *s;
   int len;
   
   /* map a device name to a mount point/path on the os (and append path) */
   if (!dev) return strdup(path);

   /* FIXME: load mappings from config and use them first - maybe device
    * discovery should be done through config and not the below (except
    * maybe for home directory and root fs and other simple thngs */
   /* FIXME: also add other virtualized dirs like "backgrounds", "themes",
    * "favorites" */
#define CMP(x) (e_util_glob_case_match(dev, x))   
#define PRT(args...) snprintf(buf, sizeof(buf), ##args)
   
   if      (CMP("/")) {
      PRT("%s", path);
   }
   else if (CMP("~/")) {
      s = e_user_homedir_get();
      PRT("%s%s", s, path);
      free(s);
   }
   else if (CMP("dvd") || CMP("dvd-*"))  {
      /* FIXME: find dvd mountpoint optionally for dvd no. X */
      /* maybe make part of the device mappings config? */
   }
   else if (CMP("cd") || CMP("cd-*") || CMP("cdrom") || CMP("cdrom-*") ||
	    CMP("dvd") || CMP("dvd-*")) {
      /* FIXME: find cdrom or dvd mountpoint optionally for cd/dvd no. X */
      /* maybe make part of the device mappings config? */
   }
   /* FIXME: add code to find USB devices (multi-card readers or single,
    * usb thumb drives, other usb storage devices (usb hd's etc.)
    */
   /* maybe make part of the device mappings config? */
   /* FIXME: add code for finding nfs shares, smb shares etc. */
   /* maybe make part of the device mappings config? */
   
   len = strlen(buf);
   while ((len > 1) && (buf[len - 1] == '/'))
     {
	buf[len - 1] = 0;
	len--;
     }
   return strdup(buf);
}

static void
_e_fm2_file_add(Evas_Object *obj, char *file)
{
   E_Fm2_Smart_Data *sd;
   E_Fm2_Icon *ic;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   /* create icon obj and append to unsorted list */
   ic = _e_fm2_icon_new(sd, file);
   if (ic)
     {
	sd->queue = evas_list_append(sd->queue, ic);
	sd->iconlist_changed = 1;
     }
}

static void
_e_fm2_file_del(Evas_Object *obj, char *file)
{
   E_Fm2_Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   /* find icon of file and remove from unsorted or main list */
   /* FIXME: find and remove */
   sd->iconlist_changed = 1;
}

static void
_e_fm2_scan_start(Evas_Object *obj)
{
   E_Fm2_Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   /* start scanning the dir in an idler and putting found items on the */
   /* queue as icons - waiting for sorting and region insertion by a */
   /* timer that ticks off taking queued icons and putting them in a */
   /* sorted position in the icon list */
   /* this system is great - it completely does the dir scanning in idle
    * time when e has nothing better to do - BUt it will mean that the dir
    * will first draw with an empty view then slowly fill up as the scan
    * happens - this means e remains interactive, but could mean for more
    * redraws that we want. what we want to do is maybe bring some of the
    * scan forward to here for a short period of time so when the window
    * is drawn - its drawn with contents ... if they didnt take too long
    * to fill
    */
   /* if i add the above pre-scan and it doesnt finish - continue here */
   sd->scan_idler = ecore_idler_add(_e_fm2_cb_scan_idler, obj);
   sd->scan_timer = ecore_timer_add(0.2, _e_fm2_cb_scan_timer, obj);
}

static void
_e_fm2_scan_stop(Evas_Object *obj)
{
   E_Fm2_Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   /* stop the scan idler, the sort timer and free the queue */
   if (sd->dir)
     {
	closedir(sd->dir);
	sd->dir = NULL;
     }
   if (sd->scan_idler)
     {
	ecore_idler_del(sd->scan_idler);
	sd->scan_idler = NULL;
     }
   if (sd->scan_timer)
     {
	ecore_timer_del(sd->scan_timer);
	sd->scan_timer = NULL;
     }
   _e_fm2_queue_free(obj);
}

static void
_e_fm2_queue_process(Evas_Object *obj)
{
   E_Fm2_Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   /* take unsorted and insert into the icon list - reprocess regions */
   while (sd->queue)
     {
	E_Fm2_Icon *ic;

	ic = sd->queue->data;
	sd->icons = evas_list_append(sd->icons, ic);
	/* insertion sort - better than qsort for the way we are doing
	 * things - incrimentally scan and sort as we go */
	sd->queue = evas_list_remove_list(sd->queue, sd->queue);
     }
   sd->icons = evas_list_sort(sd->icons, evas_list_count(sd->icons), _e_fm2_cb_icon_sort);
   if (sd->resize_job) ecore_job_del(sd->resize_job);
   sd->resize_job = ecore_job_add(_e_fm2_cb_resize_job, obj);
}

static void
_e_fm2_queue_free(Evas_Object *obj)
{
   E_Fm2_Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   /* just free the icons in the queue  and the queue itself */
   while (sd->queue)
     {
	_e_fm2_icon_free(sd->queue->data);
	sd->queue = evas_list_remove_list(sd->queue, sd->queue);
     }
}

static void
_e_fm2_regions_free(Evas_Object *obj)
{
   E_Fm2_Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   /* free up all regions */
   while (sd->regions.list)
     {
	_e_fm2_region_free(sd->regions.list->data);
        sd->regions.list = evas_list_remove_list(sd->regions.list, sd->regions.list);
     }
}

static void
_e_fm2_regions_populate(Evas_Object *obj)
{
   E_Fm2_Smart_Data *sd;
   Evas_List *l;
   E_Fm2_Region *rg;
   E_Fm2_Icon *ic;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   /* take the icon list and split into regions */
   rg = NULL;
   evas_event_freeze(evas_object_evas_get(obj));
   edje_freeze();
   for (l = sd->icons; l; l = l->next)
     {
	ic = l->data;
	if (!rg)
	  {
	     rg = _e_fm2_region_new(sd);
	     sd->regions.list = evas_list_append(sd->regions.list, rg);
	  }
	ic->region = rg;
	rg->list = evas_list_append(rg->list, ic);
	if (rg->w == 0)
	  {
	     rg->x = ic->x;
	     rg->y = ic->y;
	     rg->w = ic->w;
	     rg->h = ic->h;
	  }
	else
	  {
	     if (ic->x < rg->x)
	       {
		  rg->w += rg->x - ic->x;
		  rg->x = ic->x;
	       }
	     if ((ic->x + ic->w) > (rg->x + rg->w))
	       {
		  rg->w += (ic->x + ic->w) - (rg->x + rg->w);
	       }
	     if (ic->y < rg->y)
	       {
		  rg->h += rg->y - ic->y;
		  rg->y = ic->y;
	       }
	     if ((ic->y + ic->h) > (rg->y + rg->h))
	       {
		  rg->h += (ic->y + ic->h) - (rg->y + rg->h);
	       }
	  }
	if (evas_list_count(rg->list) > sd->regions.member_max)
	  rg = NULL;
     }
   _e_fm2_regions_eval(obj);
   for (l = sd->icons; l; l = l->next)
     {
        ic = l->data;
	if ((!ic->region->realized) && (ic->realized))
	  _e_fm2_icon_unrealize(ic);
     }
   _e_fm2_obj_icons_place(sd);
   edje_thaw();
   evas_event_thaw(evas_object_evas_get(obj));
}

static void
_e_fm2_icons_place_icons(E_Fm2_Smart_Data *sd)
{
   Evas_List *l;
   E_Fm2_Icon *ic;
   Evas_Coord x, y, rh;

   x = 0; y = 0;
   rh = 0;
   for (l = sd->icons; l; l = l->next)
     {
	ic = l->data;
	if ((x > 0) && ((x + ic->w) > sd->w))
	  {
	     x = 0;
	     y += rh;
	     rh = 0;
	  }
	ic->x = x;
	ic->y = y;
	x += ic->w;
	if (ic->h > rh) rh = ic->h;
	if ((ic->x + ic->w) > sd->max.w) sd->max.w = ic->x + ic->w;
	if ((ic->y + ic->h) > sd->max.h) sd->max.h = ic->y + ic->h;
     }
}

static void
_e_fm2_icons_place_grid_icons(E_Fm2_Smart_Data *sd)
{
   Evas_List *l;
   E_Fm2_Icon *ic;
   Evas_Coord x, y, gw, gh;

   gw = 0; gh = 0;
   for (l = sd->icons; l; l = l->next)
     {
	ic = l->data;
	if (ic->w > gw) gw = ic->w;
	if (ic->h > gh) gh = ic->h;
     }
   x = 0; y = 0;
   for (l = sd->icons; l; l = l->next)
     {
	ic = l->data;
	if ((x > 0) && ((x + ic->w) > sd->w))
	  {
	     x = 0;
	     y += gh;
	  }
	ic->x = x + (gw - ic->w);
	ic->y = y + (gh - ic->h);
	x += gw;
	if ((ic->x + ic->w) > sd->max.w) sd->max.w = ic->x + ic->w;
	if ((ic->y + ic->h) > sd->max.h) sd->max.h = ic->y + ic->h;
     }
}

static void
_e_fm2_icons_place_custom_icons(E_Fm2_Smart_Data *sd)
{
   Evas_List *l;
   E_Fm2_Icon *ic;

   for (l = sd->icons; l; l = l->next)
     {
	ic = l->data;

	if (!ic->saved_pos)
	  {
	     /* FIXME: place using smart place fn */
	  }
	
	if ((ic->x + ic->w) > sd->max.w) sd->max.w = ic->x + ic->w;
	if ((ic->y + ic->h) > sd->max.h) sd->max.h = ic->y + ic->h;
     }
}

static void
_e_fm2_icons_place_custom_grid_icons(E_Fm2_Smart_Data *sd)
{
   Evas_List *l;
   E_Fm2_Icon *ic;

   for (l = sd->icons; l; l = l->next)
     {
	ic = l->data;
	
	if (!ic->saved_pos)
	  {
	     /* FIXME: place using grid fn */
	  }
	
	if ((ic->x + ic->w) > sd->max.w) sd->max.w = ic->x + ic->w;
	if ((ic->y + ic->h) > sd->max.h) sd->max.h = ic->y + ic->h;
     }
}

static void
_e_fm2_icons_place_custom_smart_grid_icons(E_Fm2_Smart_Data *sd)
{
   Evas_List *l;
   E_Fm2_Icon *ic;

   for (l = sd->icons; l; l = l->next)
     {
	ic = l->data;
	
	if (!ic->saved_pos)
	  {
	     /* FIXME: place using smart grid fn */
	  }
	
	if ((ic->x + ic->w) > sd->max.w) sd->max.w = ic->x + ic->w;
	if ((ic->y + ic->h) > sd->max.h) sd->max.h = ic->y + ic->h;
     }
}

static void
_e_fm2_icons_place_list(E_Fm2_Smart_Data *sd)
{
   Evas_List *l;
   E_Fm2_Icon *ic;
   Evas_Coord x, y;
   int i;

   x = y = 0;
   for (i = 0, l = sd->icons; l; l = l->next, i++)
     {
	ic = l->data;
	
	/* FIXME: place in vertical list */
	ic->x = x;
	ic->y = y;
	if (sd->w > ic->min_w)
	  ic->w = sd->w;
	else
	  ic->w = ic->min_w;
	y += ic->h;
	ic->odd = (i & 0x01);
	if ((ic->x + ic->w) > sd->max.w) sd->max.w = ic->x + ic->w;
	if ((ic->y + ic->h) > sd->max.h) sd->max.h = ic->y + ic->h;
     }
}

static void
_e_fm2_icons_place(Evas_Object *obj)
{
   E_Fm2_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   /* take the icon list and find a location for them */
   sd->max.w = 0;
   sd->max.h = 0;
   switch (sd->view_mode)
     {
      case E_FM2_VIEW_MODE_ICONS:
	_e_fm2_icons_place_icons(sd);
	break;
      case E_FM2_VIEW_MODE_GRID_ICONS:
	_e_fm2_icons_place_grid_icons(sd);
	break;
      case E_FM2_VIEW_MODE_CUSTOM_ICONS:
	_e_fm2_icons_place_custom_icons(sd);
	break;
      case E_FM2_VIEW_MODE_CUSTOM_GRID_ICONS:
	_e_fm2_icons_place_custom_smart_grid_icons(sd);
	break;
      case E_FM2_VIEW_MODE_CUSTOM_SMART_GRID_ICONS:
	_e_fm2_icons_place_custom_smart_grid_icons(sd);
	break;
      case E_FM2_VIEW_MODE_LIST:
	_e_fm2_icons_place_list(sd);
	break;
      default:
	break;
     }
   /* tell our parent scrollview - if any, that we have changed */
   evas_object_smart_callback_call(sd->obj, "changed", NULL);
}

static void
_e_fm2_icons_free(Evas_Object *obj)
{
   E_Fm2_Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   _e_fm2_queue_free(obj);
   /* free all icons */
   while (sd->icons)
     {
	_e_fm2_icon_free(sd->icons->data);
        sd->icons = evas_list_remove_list(sd->icons, sd->icons);
     }
}

static void
_e_fm2_regions_eval(Evas_Object *obj)
{
   E_Fm2_Smart_Data *sd;
   Evas_List *l;
   E_Fm2_Region *rg;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_event_freeze(evas_object_evas_get(obj));
   edje_freeze();
   for (l = sd->regions.list; l; l = l->next)
     {
	rg = l->data;
	
	if (_e_fm2_region_visible(rg))
	  _e_fm2_region_realize(rg);
	else
	  _e_fm2_region_unrealize(rg);
     }
   edje_thaw();
   evas_event_thaw(evas_object_evas_get(obj));
}

/**************************/

static E_Fm2_Icon *
_e_fm2_icon_new(E_Fm2_Smart_Data *sd, char *file)
{
   E_Fm2_Icon *ic;
   Evas_Coord mw = 0, mh = 0;
   Evas_Object *obj, *obj2;
   char buf[4096];
   
   /* create icon */
   ic = E_NEW(E_Fm2_Icon, 1);
   snprintf(buf, sizeof(buf), "%s/%s", sd->realpath, file);
   if (stat(buf, &(ic->st)) == -1)
     {
	free(ic);
	return NULL;
     }
   ic->sd = sd;
   ic->file = strdup(file);
   /* FIXME: have many icon size policies. fixed, max, auto-calc etc. */
   switch (sd->view_mode)
     {
      case E_FM2_VIEW_MODE_ICONS:
      case E_FM2_VIEW_MODE_GRID_ICONS:
      case E_FM2_VIEW_MODE_CUSTOM_ICONS:
      case E_FM2_VIEW_MODE_CUSTOM_GRID_ICONS:
      case E_FM2_VIEW_MODE_CUSTOM_SMART_GRID_ICONS:
	/* FIXME: need to define icon edjes. here goes:
	 * 
	 * fileman/icon/fixed
	 * fileman/icon/variable
	 * fileman/list/fixed
	 * fileman/list/variable
	 * fileman/list_odd/fixed
	 * fileman/list_odd/variable
	 * 
	 * and now list other things i will need
	 * 
	 * fileman/background
	 * fileman/selection
	 * fileman/scrollframe
	 *
	 */
	if ((!sd->icon.fixed_w) || (!sd->icon.fixed_h))
	  {
	     obj = edje_object_add(evas_object_evas_get(sd->obj));
	     e_theme_edje_object_set(obj, "base/theme/fileman",
				     "fileman/icon/variable");
	     edje_object_part_text_set(obj, "label", ic->file);
	     edje_object_size_min_calc(obj, &mw, &mh);
	     evas_object_del(obj);
	  }
	if (sd->icon.fixed_w) ic->w = sd->icon.w;
	if (sd->icon.fixed_h) ic->h = sd->icon.h;
	ic->min_w = mw;
	ic->min_h = mh;
	break;
      case E_FM2_VIEW_MODE_LIST:
	  {
             obj = edje_object_add(evas_object_evas_get(sd->obj));
	     if (sd->icon.fixed_w)
	       e_theme_edje_object_set(obj, "base/theme/fileman",
				       "fileman/list/fixed");
	     else
	       e_theme_edje_object_set(obj, "base/theme/fileman",
				       "fileman/list/variable");
	     _e_fm2_icon_label_set(ic, obj);
	     obj2 = evas_object_rectangle_add(evas_object_evas_get(sd->obj));
	     edje_extern_object_min_size_set(obj2, sd->icon.list_w, sd->icon.list_h);
	     edje_extern_object_max_size_set(obj2, sd->icon.list_w, sd->icon.list_h);
	     edje_object_part_swallow(obj, "icon_swallow", obj2);
	     edje_object_size_min_calc(obj, &mw, &mh);
	     evas_object_del(obj2);
	     evas_object_del(obj);
	  }
	if (mw < sd->w) ic->w = sd->w;
	else ic->w = mw;
	ic->h = mh;
	ic->min_w = mw;
	ic->min_h = mh;
	break;
      default:
	break;
     }
   return ic;
}

static void
_e_fm2_icon_free(E_Fm2_Icon *ic)
{
   /* free icon, object data etc. etc. */
   _e_fm2_icon_unrealize(ic);
   E_FREE(ic->file);
   free(ic);
}

static void
_e_fm2_icon_realize(E_Fm2_Icon *ic)
{
   if (ic->realized) return;
   /* actually create evas objects etc. */
   ic->realized = 1;
   evas_event_freeze(evas_object_evas_get(ic->sd->obj));
   ic->obj = edje_object_add(evas_object_evas_get(ic->sd->obj));
   edje_object_freeze(ic->obj);
   evas_object_smart_member_add(ic->obj, ic->sd->obj);
   /* FIXME: this is currently a hack just to get a display working - go back
    * and do proper icon stuff later */
   if (ic->sd->view_mode == E_FM2_VIEW_MODE_LIST)
     {
        if (ic->sd->icon.fixed_w)
	  {
	     if (ic->odd)
	       e_theme_edje_object_set(ic->obj, "base/theme/widgets",
				       "fileman/list_odd/fixed");
	     else
	       e_theme_edje_object_set(ic->obj, "base/theme/widgets",
				       "fileman/list/fixed");
	  }
	else
	  {
	     if (ic->odd)
	       e_theme_edje_object_set(ic->obj, "base/theme/widgets",
				       "fileman/list_odd/variable");
	     else
	       e_theme_edje_object_set(ic->obj, "base/theme/widgets",
				       "fileman/list/variable");
	  }
	_e_fm2_icon_label_set(ic, ic->obj);
     }
   else
     {
	e_theme_edje_object_set(ic->obj, "base/theme/fileman",
				"fileman/icon_normal");
	edje_object_part_text_set(ic->obj, "icon_title", ic->file);
     }
   evas_object_clip_set(ic->obj, ic->sd->clip);
   evas_object_move(ic->obj,
		    ic->sd->x + ic->x - ic->sd->pos.x,
		    ic->sd->y + ic->y - ic->sd->pos.y);
   evas_object_resize(ic->obj, ic->w, ic->h);

   evas_object_event_callback_add(ic->obj, EVAS_CALLBACK_MOUSE_DOWN, _e_fm2_cb_icon_mouse_down, ic);
   evas_object_event_callback_add(ic->obj, EVAS_CALLBACK_MOUSE_UP, _e_fm2_cb_icon_mouse_up, ic);
   evas_object_event_callback_add(ic->obj, EVAS_CALLBACK_MOUSE_MOVE, _e_fm2_cb_icon_mouse_move, ic);
   
   _e_fm2_icon_icon_set(ic);
   
   edje_object_thaw(ic->obj);
   evas_event_thaw(evas_object_evas_get(ic->sd->obj));
   evas_object_show(ic->obj);

   if (ic->selected)
     {
	/* FIXME: need new signal to INSTANTLY activate - no anim */
	edje_object_signal_emit(ic->obj, "active", "");
	edje_object_signal_emit(ic->obj_icon, "active", "");
     }
}

static void
_e_fm2_icon_unrealize(E_Fm2_Icon *ic)
{
   if (!ic->realized) return;
   /* delete evas objects */
   ic->realized = 0;
   evas_object_del(ic->obj);
   ic->obj = NULL;
   evas_object_del(ic->obj_icon);
   ic->obj_icon = NULL;
}

static int
_e_fm2_icon_visible(E_Fm2_Icon *ic)
{
   /* return if the icon is visible */
   if (
       ((ic->x - ic->sd->pos.x) < (ic->sd->w)) &&
       ((ic->x + ic->w - ic->sd->pos.x) > 0) &&
       ((ic->y - ic->sd->pos.y) < (ic->sd->h)) &&
       ((ic->y + ic->h - ic->sd->pos.y) > 0)
       )
     return 1;
   return 0;
}

static void
_e_fm2_icon_label_set(E_Fm2_Icon *ic, Evas_Object *obj)
{
   char buf[4096], *p;
   int len;
   
   if (ic->sd->show_extension)
     edje_object_part_text_set(obj, "label", ic->file);
   else
     {
	/* remove extension. handle double extensions like .tar.gz too
	 * also be fuzzy - up to 4 chars of extn is ok - eg .html but 5 or
	 * more is considered part of the name
	 */
	strncpy(buf, ic->file, sizeof(buf) - 2);
	buf[sizeof(buf) - 1] = 0;
	
	len = strlen(buf);
	p = strrchr(buf, '.');
	if ((p) && ((len - (p - buf)) < 6))
	  {
	     *p = 0;
	
	     len = strlen(buf);
	     p = strrchr(buf, '.');
	     if ((p) && ((len - (p - buf)) < 6)) *p = 0;
	  }
	edje_object_part_text_set(obj, "label", buf);
     }
}

static void
_e_fm2_icon_icon_set(E_Fm2_Icon *ic)
{
   char buf[4096];
   
   if (!ic->realized) return;
   /* FIXME: use mimetype - for now just using extension glob */
   if (S_ISDIR(ic->st.st_mode))
     {
	ic->obj_icon = edje_object_add(evas_object_evas_get(ic->sd->obj));
	e_theme_edje_object_set(ic->obj_icon, "base/theme/fileman",
				"icons/fileman/folder");
	edje_object_part_swallow(ic->obj, "icon_swallow", ic->obj_icon);
	evas_object_show(ic->obj_icon);
     }
   else
     {
	if (
	    (e_util_glob_case_match(ic->file, "*.jpg")) ||
	    (e_util_glob_case_match(ic->file, "*.jpeg")) ||
	    (e_util_glob_case_match(ic->file, "*.jfif")) ||
	    (e_util_glob_case_match(ic->file, "*.jpe")) ||
	    (e_util_glob_case_match(ic->file, "*.png")) ||
	    (e_util_glob_case_match(ic->file, "*.gif")) ||
	    (e_util_glob_case_match(ic->file, "*.tif")) ||
	    (e_util_glob_case_match(ic->file, "*.tiff"))
	    )
	  {
	     snprintf(buf, sizeof(buf), "%s/%s", ic->sd->realpath, ic->file);
	     ic->obj_icon = e_thumb_icon_add(evas_object_evas_get(ic->sd->obj));
	     e_thumb_icon_file_set(ic->obj_icon, buf, NULL);
	     e_thumb_icon_size_set(ic->obj_icon, 64, 64);
	     evas_object_smart_callback_add(ic->obj_icon, "e_thumb_gen", _e_fm2_cb_icon_thumb_gen, ic);
	     e_thumb_icon_begin(ic->obj_icon);
	     edje_object_part_swallow(ic->obj, "icon_swallow", ic->obj_icon);
	     evas_object_show(ic->obj_icon);
	  }
	else if (
		 (e_util_glob_case_match(ic->file, "*.edj"))
		 )
	  {
	     snprintf(buf, sizeof(buf), "%s/%s", ic->sd->realpath, ic->file);
	     ic->obj_icon = e_thumb_icon_add(evas_object_evas_get(ic->sd->obj));
	     e_thumb_icon_file_set(ic->obj_icon, buf, "desktop/background");
	     e_thumb_icon_size_set(ic->obj_icon, 64, 64);
	     evas_object_smart_callback_add(ic->obj_icon, "e_thumb_gen", _e_fm2_cb_icon_thumb_gen, ic);
	     e_thumb_icon_begin(ic->obj_icon);
	     edje_object_part_swallow(ic->obj, "icon_swallow", ic->obj_icon);
	     evas_object_show(ic->obj_icon);
	  }
	else if (
		 (e_util_glob_case_match(ic->file, "*.eap"))
		 )
	  {
	     snprintf(buf, sizeof(buf), "%s/%s", ic->sd->realpath, ic->file);
	     ic->obj_icon = e_thumb_icon_add(evas_object_evas_get(ic->sd->obj));
	     e_thumb_icon_file_set(ic->obj_icon, buf, "icon");
	     e_thumb_icon_size_set(ic->obj_icon, 64, 64);
	     evas_object_smart_callback_add(ic->obj_icon, "e_thumb_gen", _e_fm2_cb_icon_thumb_gen, ic);
	     e_thumb_icon_begin(ic->obj_icon);
	     edje_object_part_swallow(ic->obj, "icon_swallow", ic->obj_icon);
	     evas_object_show(ic->obj_icon);
	  }
	else
	  {
	     ic->obj_icon = edje_object_add(evas_object_evas_get(ic->sd->obj));
	     e_theme_edje_object_set(ic->obj_icon, "base/theme/fileman",
				     "icons/fileman/file");
	     edje_object_part_swallow(ic->obj, "icon_swallow", ic->obj_icon);
	     evas_object_show(ic->obj_icon);
	  }
     }
}

static void
_e_fm2_icon_select(E_Fm2_Icon *ic)
{
   if (ic->selected) return;
   ic->selected = 1;
   ic->last_selected = 1;
   if (ic->realized)
     {
	edje_object_signal_emit(ic->obj, "active", "");
	edje_object_signal_emit(ic->obj_icon, "active", "");
	evas_object_raise(ic->obj);
     }
}

static void
_e_fm2_icon_deselect(E_Fm2_Icon *ic)
{
   if (!ic->selected) return;
   ic->selected = 0;
   ic->last_selected = 0;
   if (ic->realized)
     {
	edje_object_signal_emit(ic->obj, "passive", "");
	edje_object_signal_emit(ic->obj_icon, "passive", "");
     }
}

/**************************/
static E_Fm2_Region *
_e_fm2_region_new(E_Fm2_Smart_Data *sd)
{
   E_Fm2_Region *rg;
   
   rg = E_NEW(E_Fm2_Region, 1);
   rg->sd = sd;
   return rg;
}

static void
_e_fm2_region_free(E_Fm2_Region *rg)
{
   E_Fm2_Icon *ic;
   
   while (rg->list)
     {
	ic = rg->list->data;
	ic->region = NULL;
	rg->list = evas_list_remove_list(rg->list, rg->list);
     }
   free(rg);
}

static void
_e_fm2_region_realize(E_Fm2_Region *rg)
{
   Evas_List *l;
   
   if (rg->realized) return;
   /* actually create evas objects etc. */
   rg->realized = 1;
   printf("REG %p REALIZE\n", rg);
   edje_freeze();
   for (l = rg->list; l; l = l->next) _e_fm2_icon_realize(l->data);
   edje_thaw();
}

static void
_e_fm2_region_unrealize(E_Fm2_Region *rg)
{
   Evas_List *l;
   
   if (!rg->realized) return;
   /* delete evas objects */
   rg->realized = 0;
   printf("REG %p UNREALIZE\n", rg);
   edje_freeze();
   for (l = rg->list; l; l = l->next) _e_fm2_icon_unrealize(l->data);
   edje_thaw();
}

static int
_e_fm2_region_visible(E_Fm2_Region *rg)
{
   /* return if the icon is visible */
   if (
       ((rg->x - rg->sd->pos.x) < (rg->sd->w)) &&
       ((rg->x + rg->w - rg->sd->pos.x) > 0) &&
       ((rg->y - rg->sd->pos.y) < (rg->sd->h)) &&
       ((rg->y + rg->h - rg->sd->pos.y) > 0)
       )
     return 1;
   return 0;
}

/**************************/

static void
_e_fm2_cb_icon_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Down *ev;
   E_Fm2_Icon *ic, *ic2;
   int multi_sel = 0, range_sel = 0, seen = 0;
   Evas_List *l;
   
   ic = data;
   ev = event_info;
   if (ev->button == 1)
     {
	if (ic->sd->windows_multi_select_modifiers)
	  {
	     if (evas_key_modifier_is_set(ev->modifiers, "Shift"))
	       range_sel = 1;
	     else if (evas_key_modifier_is_set(ev->modifiers, "Control"))
	       multi_sel = 1;
	  }
	else
	  {
	     if (evas_key_modifier_is_set(ev->modifiers, "Control"))
	       range_sel = 1;
	     else if (evas_key_modifier_is_set(ev->modifiers, "Shift"))
	       multi_sel = 1;
	  }
	if (ic->sd->single_select)
	  {
	     multi_sel = 0;
	     range_sel = 0;
	  }
	if (range_sel)
	  {
	     /* find last selected - if any, and seletc all icons between */
	     for (l = ic->sd->icons; l; l = l->next)
	       {
		  ic2 = l->data;
		  if (ic2 == ic) seen = 1;
		  if (ic2->last_selected)
		    {
		       ic2->last_selected = 0;
		       if (seen)
			 {
			    for (; (l) && (l->data != ic); l = l->prev)
			      {
				 ic2 = l->data;
				 _e_fm2_icon_select(ic2);
				 ic2->last_selected = 0;
			      }
			 }
		       else
			 {
			    for (; (l) && (l->data != ic); l = l->next)
			      {
				 ic2 = l->data;
				 _e_fm2_icon_select(ic2);
				 ic2->last_selected = 0;
			      }
			 }
		       break;
		    }
	       }
	  }
	else if (!multi_sel)
	  {
	     /* desel others */
	     for (l = ic->sd->icons; l; l = l->next)
	       {
		  ic2 = l->data;
		  if (ic2 != ic)
		    {
		       if (ic2->selected) _e_fm2_icon_deselect(ic2);
		    }
	       }
	  }
	else
	  {
	     for (l = ic->sd->icons; l; l = l->next)
	       {
		  ic2 = l->data;
		  ic2->last_selected = 0;
	       }
	  }
	if ((!range_sel) && (ic->selected))
	  _e_fm2_icon_deselect(ic);
	else
	  _e_fm2_icon_select(ic);
     }
}
    
static void
_e_fm2_cb_icon_mouse_up(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Up *ev;
   E_Fm2_Icon *ic;
   
   ic = data;
   ev = event_info;
}
    
static void
_e_fm2_cb_icon_mouse_move(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Move *ev;
   E_Fm2_Icon *ic;
   
   ic = data;
   ev = event_info;
}
    
static void
_e_fm2_cb_icon_thumb_gen(void *data, Evas_Object *obj, void *event_info)
{
   E_Fm2_Icon *ic;
   
   ic = data;
   edje_object_signal_emit(ic->obj, "thumb", "gen");
}
    
static void
_e_fm2_cb_scroll_job(void *data)
{
   E_Fm2_Smart_Data *sd;
   
   sd = evas_object_smart_data_get(data);
   if (!sd) return;
   sd->scroll_job = NULL;
   printf("DO scroll!\n");
   evas_event_freeze(evas_object_evas_get(sd->obj));
   edje_freeze();
   _e_fm2_regions_eval(sd->obj);
   _e_fm2_obj_icons_place(sd);
   edje_thaw();
   evas_event_thaw(evas_object_evas_get(sd->obj));
}

static void
_e_fm2_cb_resize_job(void *data)
{
   E_Fm2_Smart_Data *sd;
   Evas_List *l;
   
   sd = evas_object_smart_data_get(data);
   if (!sd) return;
   sd->resize_job = NULL;
   evas_event_freeze(evas_object_evas_get(sd->obj));
   edje_freeze();
   switch (sd->view_mode)
     {
      case E_FM2_VIEW_MODE_ICONS:
	_e_fm2_regions_free(sd->obj);
	_e_fm2_icons_place(sd->obj);
	_e_fm2_regions_populate(sd->obj);
	break;
      case E_FM2_VIEW_MODE_GRID_ICONS:
	_e_fm2_regions_free(sd->obj);
	_e_fm2_icons_place(sd->obj);
	_e_fm2_regions_populate(sd->obj);
	break;
      case E_FM2_VIEW_MODE_CUSTOM_ICONS:
	_e_fm2_regions_eval(sd->obj);
	_e_fm2_obj_icons_place(sd);
	break;
      case E_FM2_VIEW_MODE_CUSTOM_GRID_ICONS:
	_e_fm2_regions_eval(sd->obj);
	_e_fm2_obj_icons_place(sd);
	break;
      case E_FM2_VIEW_MODE_CUSTOM_SMART_GRID_ICONS:
	_e_fm2_regions_eval(sd->obj);
	_e_fm2_obj_icons_place(sd);
	break;
      case E_FM2_VIEW_MODE_LIST:
	if (sd->iconlist_changed)
	  {
	     for (l = sd->icons; l; l = l->next)
	       _e_fm2_icon_unrealize(l->data);
	  }
        _e_fm2_regions_free(sd->obj);
	_e_fm2_icons_place(sd->obj);
        _e_fm2_regions_populate(sd->obj);
	break;
      default:
	break;
     }
   edje_thaw();
   evas_event_thaw(evas_object_evas_get(sd->obj));
   sd->iconlist_changed = 0;
}

static int
_e_fm2_cb_icon_sort(void *data1, void *data2)
{
   E_Fm2_Icon *ic1, *ic2;
   
   ic1 = data1;
   ic2 = data2;
   if (ic1->sd->dirs_last)
     {
	if ((S_ISDIR(ic1->st.st_mode)) != (S_ISDIR(ic2->st.st_mode)))
	  {
	     if (S_ISDIR(ic1->st.st_mode)) return -1;
	     else return 1;
	  }
     }
   else if (ic1->sd->dirs_first)
     {
	if ((S_ISDIR(ic1->st.st_mode)) != (S_ISDIR(ic2->st.st_mode)))
	  {
	     if (S_ISDIR(ic1->st.st_mode)) return 1;
	     else return -1;
	  }
     }
   if (ic1->sd->no_case_sort)
     {
	char buf1[4096], buf2[4096], *p;
	
	strncpy(buf1, ic1->file, sizeof(buf1) - 2);
	strncpy(buf2, ic2->file, sizeof(buf2) - 2);
	buf1[sizeof(buf1) - 1] = 0;
	buf2[sizeof(buf2) - 1] = 0;
	p = buf1;
	while (*p)
	  {
	     *p = tolower(*p);
	     p++;
	  }
	p = buf2;
	while (*p)
	  {
	     *p = tolower(*p);
	     p++;
	  }
	return strcmp(buf1, buf2);
     }
   return strcmp(ic1->file, ic2->file);
}

static int
_e_fm2_cb_scan_idler(void *data)
{
   E_Fm2_Smart_Data *sd;
   struct dirent *dp;

   sd = evas_object_smart_data_get(data);
   if (!sd) return 0;
   if (!sd->dir) sd->dir = opendir(sd->realpath);
   if (!sd->dir) goto endscan;
   
   dp = readdir(sd->dir);
   if (!dp) goto endscan;
   /* no - you don't want the cuirrent and parent dir links listed */
   if ((!strcmp(dp->d_name, ".")) || (!strcmp(dp->d_name, ".."))) return 1;
   /* skip dotfiles */
   if (dp->d_name[0] == '.') return 1;
   _e_fm2_file_add(data, dp->d_name);
   return 1;
   
   endscan:
   if (sd->dir)
     {
	closedir(sd->dir);
	sd->dir = NULL;
     }
   sd->scan_idler = NULL;
   if (sd->scan_timer)
     {
	ecore_timer_del(sd->scan_timer);
	sd->scan_timer = ecore_timer_add(0.0001, _e_fm2_cb_scan_timer, sd->obj);
     }
   return 0;
}

static int
_e_fm2_cb_scan_timer(void *data)
{
   E_Fm2_Smart_Data *sd;

   sd = evas_object_smart_data_get(data);
   if (!sd) return 0;
   _e_fm2_queue_process(data);
   if (!sd->scan_idler)
     {
	sd->scan_timer = NULL;
	return 0;
     }
   return 1;
}

/**************************/
static void
_e_fm2_obj_icons_place(E_Fm2_Smart_Data *sd)
{
   Evas_List *l, *ll;
   E_Fm2_Region *rg;
   E_Fm2_Icon *ic;

   evas_event_freeze(evas_object_evas_get(sd->obj));
   edje_freeze();
   for (l = sd->regions.list; l; l = l->next)
     {
	rg = l->data;
	if (rg->realized)
	  {
	     for (ll = rg->list; ll; ll = ll->next)
	       {
		  ic = ll->data;
		  if (ic->realized)
		    {
		       evas_object_move(ic->obj, 
					sd->x + ic->x - sd->pos.x, 
					sd->y + ic->y - sd->pos.y);
		       evas_object_resize(ic->obj, ic->w, ic->h);
		    }
	       }
	  }
     }
   edje_thaw();
   evas_event_thaw(evas_object_evas_get(sd->obj));
}

/**************************/

static void
_e_fm2_smart_add(Evas_Object *obj)
{
   E_Fm2_Smart_Data *sd;

   sd = E_NEW(E_Fm2_Smart_Data, 1);
   if (!sd) return;
   sd->obj = obj;
   sd->clip = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_smart_member_add(sd->clip, obj);
   evas_object_move(sd->clip, sd->x, sd->y);
   evas_object_resize(sd->clip, sd->w, sd->h);
   evas_object_color_set(sd->clip, 255, 255, 255, 255);
   evas_object_smart_data_set(obj, sd);
   evas_object_move(obj, 0, 0);
   evas_object_resize(obj, 0, 0);
}

static void
_e_fm2_smart_del(Evas_Object *obj)
{
   E_Fm2_Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   
   _e_fm2_scan_stop(obj);
   _e_fm2_queue_free(obj);
   _e_fm2_regions_free(obj);
   _e_fm2_icons_free(obj);
   if (sd->scroll_job) ecore_job_del(sd->scroll_job);
   if (sd->resize_job) ecore_job_del(sd->resize_job);
   E_FREE(sd->dev);
   E_FREE(sd->path);
   E_FREE(sd->realpath);
   
   evas_object_del(sd->clip);
   free(sd);
}

static void
_e_fm2_smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   E_Fm2_Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if ((sd->x == x) && (sd->y == y)) return;
   sd->x = x;
   sd->y = y;
   evas_object_move(sd->clip, x, y);
   _e_fm2_obj_icons_place(sd);
}

static void
_e_fm2_smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   E_Fm2_Smart_Data *sd;
   int wch = 0, hch = 0;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if ((sd->w == w) && (sd->h == h)) return;
   if (w != sd->w) wch = 1;
   if (h != sd->h) hch = 1;
   sd->w = w;
   sd->h = h;
   evas_object_resize(sd->clip, w, h);

   /* for automatic layout - do this - NB; we could put this on a timer delay */
   if (wch)
     {
	if (sd->resize_job) ecore_job_del(sd->resize_job);
	sd->resize_job = ecore_job_add(_e_fm2_cb_resize_job, obj);
     }
   else
     {
	if (sd->scroll_job) ecore_job_del(sd->scroll_job);
	sd->scroll_job = ecore_job_add(_e_fm2_cb_scroll_job, obj);
     }
}

static void
_e_fm2_smart_show(Evas_Object *obj)
{
   E_Fm2_Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_show(sd->clip);
}

static void
_e_fm2_smart_hide(Evas_Object *obj)
{
   E_Fm2_Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_hide(sd->clip);
}

static void
_e_fm2_smart_color_set(Evas_Object *obj, int r, int g, int b, int a)
{
   E_Fm2_Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_color_set(sd->clip, r, g, b, a);
}

static void
_e_fm2_smart_clip_set(Evas_Object *obj, Evas_Object *clip)
{
   E_Fm2_Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_clip_set(sd->clip, clip);
}

static void
_e_fm2_smart_clip_unset(Evas_Object *obj)
{
   E_Fm2_Smart_Data *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   evas_object_clip_unset(sd->clip);
}
