#include "Elation.h"

#include <dvdnav/dvdnav.h>
#include <dvdnav/nav_read.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/cdrom.h>

/* external module symbols. the rest is private */
void *init(Elation_Module *em);

/********************/
typedef struct _Elation_Module_Private Elation_Module_Private;

struct _Elation_Module_Private
{
   Evas_Object *overlay;
   Evas_Object *background1;
   Evas_Object *background2;
   Evas_Object *video;
   
   Ecore_Timer *media_check_timer;
   Ecore_Timer *media_play_timer;
   double       media_fade_in_start;
   Ecore_Timer *media_fade_in_timer;
   
   unsigned char have_media : 1;
   unsigned char check_media_done : 1;
   unsigned char menu_visible : 1;
};

static void shutdown(Elation_Module *em);
static void resize(Elation_Module *em);
static void show(Elation_Module *em);
static void hide(Elation_Module *em);
static void focus(Elation_Module *em);
static void unfocus(Elation_Module *em);
static void action(Elation_Module *em, int action);

static void frame_decode_cb(void *data, Evas_Object *obj, void *event_info);
static void frame_resize_cb(void *data, Evas_Object *obj, void *event_info);
static void length_change_cb(void *data, Evas_Object *obj, void *event_info);
static void decode_stop_cb(void *data, Evas_Object *obj, void *event_info);
static void button_num_change_cb(void *data, Evas_Object *obj, void *event_info);
static void key_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static int  media_check_timer_cb(void *data);
static int  media_play_timer_cb(void *data);
static int  media_fade_in_timer_cb(void *data);

void *
init(Elation_Module *em)
{
   Elation_Module_Private *pr;
   
   pr = calloc(1, sizeof(Elation_Module_Private));
   if (!pr) return NULL;
   
   /* set up module methods */
   em->shutdown = shutdown;
   em->resize = resize;
   em->show = show;
   em->hide = hide;
   em->focus = focus;
   em->unfocus = unfocus;
   em->action = action;

   pr->background1 = evas_object_rectangle_add(em->info->evas);
   evas_object_color_set(pr->background1, 0, 0, 0, 255);
   pr->background2 = evas_object_rectangle_add(em->info->evas);
   evas_object_color_set(pr->background2, 0, 0, 0, 255);
   
   pr->video = emotion_object_add(em->info->evas);
   evas_object_event_callback_add(pr->video, EVAS_CALLBACK_KEY_DOWN, key_down_cb, em);
   
   evas_object_smart_callback_add(pr->video, "frame_decode", frame_decode_cb, em);
   evas_object_smart_callback_add(pr->video, "frame_resize", frame_resize_cb, em);
   evas_object_smart_callback_add(pr->video, "length_change",length_change_cb, em);
   evas_object_smart_callback_add(pr->video, "decode_stop", decode_stop_cb, em);
   evas_object_smart_callback_add(pr->video, "button_num_change", button_num_change_cb, em);
   
   emotion_object_smooth_scale_set(pr->video, 1);
   
   pr->overlay = edje_object_add(em->info->evas);
   edje_object_file_set(pr->overlay, PACKAGE_DATA_DIR"/data/theme.eet", "dvd");
   edje_object_signal_emit(pr->overlay, "media", "0");

   pr->media_check_timer = ecore_timer_add(0.5, media_check_timer_cb, em);
   
   return pr;
}

static void
shutdown(Elation_Module *em)
{
   Elation_Module_Private *pr;
   
   pr = em->data;
   evas_object_del(pr->background1);
   evas_object_del(pr->background2);
   evas_object_del(pr->video);
   evas_object_del(pr->overlay);
   if (pr->media_check_timer) ecore_timer_del(pr->media_check_timer);
   if (pr->media_play_timer) ecore_timer_del(pr->media_play_timer);
   if (pr->media_fade_in_timer) ecore_timer_del(pr->media_fade_in_timer);
   free(pr);
}

static void
resize(Elation_Module *em)
{
   Elation_Module_Private *pr;
   Evas_Coord x, y, w, h, ww, hh;
   double ratio;
   int vw, vh;
   
   pr = em->data;
   evas_output_viewport_get(em->info->evas, NULL, NULL, &w, &h);
   
   ww = w;
   hh = h;
   
   evas_object_move(pr->overlay, 0, 0);
   evas_object_resize(pr->overlay, w, h);
   
   emotion_object_size_get(pr->video, &vw, &vh);
   ratio = emotion_object_ratio_get(pr->video);
   if (ratio > 0.0)
     {
	x = 0;
	y = (h - (w / ratio)) / 2;
	if (y < 0)
	  {
	     y = 0;
	     x = (w - (h * ratio)) / 2;
	     w = h * ratio;
	  }
	else
	  h = w / ratio;
	evas_object_move(pr->video, x, y);
	evas_object_resize(pr->video, w, h);
     }
   else
     {
	if (vh > 1)
	  {
	     ratio = (double)vw / (double)vh;
	     x = 0;
	     y = (h - (w / ratio)) / 2;
	     if (y < 0)
	       {
		  y = 0;
		  x = (w - (h * ratio)) / 2;
		  w = h * ratio;
	       }
	     else
	       h = w / ratio;
	  }
	evas_object_move(pr->video, 0, 0);
	evas_object_resize(pr->video, w, h);
     }
   
   if (w == ww)
     {
	evas_object_move(pr->background1, 0, 0);
	evas_object_resize(pr->background1, ww, y);
	evas_object_move(pr->background2, 0, y + h);
	evas_object_resize(pr->background2, ww, hh - h - y);
     }
   else
     {
	evas_object_move(pr->background1, 0, 0);
	evas_object_resize(pr->background1, x, hh);
	evas_object_move(pr->background2, x + w, 0);
	evas_object_resize(pr->background2, ww - w - x, hh);
     }
}

static void
show(Elation_Module *em)
{
   Elation_Module_Private *pr;
   
   pr = em->data;
   if (pr->have_media)
     {
	evas_object_show(pr->background1);
	evas_object_show(pr->background2);
	evas_object_show(pr->video);
     }
   evas_object_show(pr->overlay);
}

static void
hide(Elation_Module *em)
{
   Elation_Module_Private *pr;
   
   pr = em->data;
   evas_object_hide(pr->background1);
   evas_object_hide(pr->background2);
   evas_object_hide(pr->video);
   evas_object_hide(pr->overlay);
}

static void
focus(Elation_Module *em)
{
   Elation_Module_Private *pr;
   
   pr = em->data;
   evas_object_focus_set(pr->video, 1);
}

static void
unfocus(Elation_Module *em)
{
   Elation_Module_Private *pr;
   
   pr = em->data;
   evas_object_focus_set(pr->video, 0);
}
   
static void
action(Elation_Module *em, int action)
{
   Elation_Module_Private *pr;
   
   pr = em->data;
   switch (action)
     {
      case ELATION_ACT_NEXT:
	if (pr->menu_visible)
	  {
	  }
	else
	  emotion_object_event_simple_send(pr->video, EMOTION_EVENT_NEXT);
	break;
      case ELATION_ACT_PREV:
	if (pr->menu_visible)
	  {
	  }
	else
	  emotion_object_event_simple_send(pr->video, EMOTION_EVENT_PREV);
	break;
      case ELATION_ACT_SELECT:
	if (pr->menu_visible)
	  emotion_object_event_simple_send(pr->video, EMOTION_EVENT_SELECT);
	else
	  {
	     if (emotion_object_play_get(pr->video))
	       emotion_object_play_set(pr->video, 0);
	     else
	       emotion_object_play_set(pr->video, 1);
	  }
	break;
      case ELATION_ACT_EXIT:
	  {
	     int fd;
	     
	     printf("stop...\n");
	     emotion_object_play_set(pr->video, 0);
	     printf("rewind...\n");
	     while (emotion_object_position_get(pr->video) != 0.0)
	       emotion_object_position_set(pr->video, 0.0);
	     printf("eject...\n");
//	     emotion_object_eject(pr->video);
	     printf("fset...\n");
	     if (0)
	       {
		  evas_object_del(pr->video);
		  pr->video = emotion_object_add(em->info->evas);
		  evas_object_event_callback_add(pr->video, EVAS_CALLBACK_KEY_DOWN, key_down_cb, em);
		  
		  evas_object_smart_callback_add(pr->video, "frame_decode", frame_decode_cb, em);
		  evas_object_smart_callback_add(pr->video, "frame_resize", frame_resize_cb, em);
		  evas_object_smart_callback_add(pr->video, "length_change",length_change_cb, em);
		  evas_object_smart_callback_add(pr->video, "decode_stop", decode_stop_cb, em);
		  evas_object_smart_callback_add(pr->video, "button_num_change", button_num_change_cb, em);
		  
		  emotion_object_smooth_scale_set(pr->video, 1);
		  
		  evas_object_stack_above(pr->video, pr->background1);
	       }
	     else
	       emotion_object_file_set(pr->video, NULL);
	     printf("emit..\n");
	     edje_object_signal_emit(pr->overlay, "media", "0");
	     evas_object_hide(pr->background1);
	     evas_object_hide(pr->background2);
	     evas_object_hide(pr->video);
	     pr->have_media = 0;
	     fd = open("/dev/dvd", O_RDONLY | O_NONBLOCK);
	     if (fd >= 0)
	       {
		  int i;
		  printf("eject disk!\n");
		  
		  for (i = 0; i < 5; i++)
		    {
		       if (ioctl(fd, CDROMEJECT, 0) == 0) break;
		       perror("ioctl");
		       sleep(1);
		    }
		  close(fd);
	       }
	  }
	break;
      case ELATION_ACT_UP:
	if (pr->menu_visible)
	  emotion_object_event_simple_send(pr->video, EMOTION_EVENT_UP);
	else
	  {
	     // FIXME: bring up config menu
	  }
	break;
      case ELATION_ACT_DOWN:
	if (pr->menu_visible)
	  emotion_object_event_simple_send(pr->video, EMOTION_EVENT_DOWN);
	else
	  {
	     // FIXME: bring up config menu
	  }
	break;
      case ELATION_ACT_LEFT:
	if (pr->menu_visible)
	  emotion_object_event_simple_send(pr->video, EMOTION_EVENT_LEFT);
	else
	  {
	     double pos;
	     
	     pos = emotion_object_position_get(pr->video);
	     emotion_object_position_set(pr->video, pos - 30.0);
	  }
	break;
      case ELATION_ACT_RIGHT:
	if (pr->menu_visible)
	  emotion_object_event_simple_send(pr->video, EMOTION_EVENT_RIGHT);
	else
	  {
	     double pos;
	     
	     pos = emotion_object_position_get(pr->video);
	     emotion_object_position_set(pr->video, pos + 30.0);
	  }
	break;
      case ELATION_ACT_MENU:
	if (pr->menu_visible)
	  {
	     // FIXME: bring up config menu
	  }
	else
	  emotion_object_event_simple_send(pr->video, EMOTION_EVENT_MENU1);
	break;
      case ELATION_ACT_INFO:
	if (pr->menu_visible)
	  {
	     // FIXME: bring up config menu
	  }
	else
	  {
	  }
	break;
      case ELATION_ACT_INPUT:
	if (pr->menu_visible)
	  {
	     // FIXME: bring up config menu
	  }
	else
	  {
	  }
	break;
      case ELATION_ACT_PLAY:
	if (pr->menu_visible)
	  {
	     emotion_object_event_simple_send(pr->video, EMOTION_EVENT_NEXT);
	  }
	else
	  {
	     emotion_object_play_set(pr->video, 1);
	  }
	break;
      case ELATION_ACT_PAUSE:
	if (pr->menu_visible)
	  {
	  }
	else
	  {
	     if (emotion_object_play_get(pr->video))
	       emotion_object_play_set(pr->video, 0);
	     else
	       emotion_object_play_set(pr->video, 1);
	  }
	break;
      case ELATION_ACT_STOP:
	if (pr->menu_visible)
	  {
	  }
	else
	  {
	     emotion_object_play_set(pr->video, 0);
	     emotion_object_position_set(pr->video, 0.0);
	  }
	break;
      case ELATION_ACT_REC:
	break;
      case ELATION_ACT_SKIP:
	break;
      case ELATION_ACT_NONE:
      default:
	break;
     }
}

/*** private stuff ***/

#if 0
static void
dvd_info_get(Elation_Module *em)
{
   dvdnav_t *dvd;
   int title_num = 0;
   int i;
   char *str = NULL;
   
   if (dvdnav_open(&dvd, "/dev/dvd") == DVDNAV_STATUS_ERR) return;
   dvdnav_get_title_string(dvd, &str);
   printf("TITLE: %s\n", str);
   // strdup str as its an internal string in the dvd handle
   dvdnav_get_number_of_titles(dvd, &title_num);
   for (i = 0; i < title_num; i++)
     {
	int part_num;
	
	dvdnav_get_number_of_parts(dvd, i, &part_num);
     }
   dvdnav_close(dvd);
}
#endif

static void
frame_decode_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elation_Module *em;
   double pos, len;
   
   em = data;
   pos = emotion_object_position_get(obj);
   len = emotion_object_play_length_get(obj);
}

static void
frame_resize_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elation_Module *em;
   
   em = data;
   em->resize(em);
}

static void
length_change_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elation_Module *em;
   double pos, len;
                
   em = data;
   pos = emotion_object_position_get(obj);
   len = emotion_object_play_length_get(obj);
}

static void
decode_stop_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elation_Module *em;
   Elation_Module_Private *pr;
   
   em = data;
   pr = em->data;
//   emotion_object_position_set(pr->video, 0.0);
//   emotion_object_play_set(pr->video, 1);
}
   
static void
button_num_change_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elation_Module *em;
   Elation_Module_Private *pr;
   
   em = data;
   pr = em->data;
   if (emotion_object_spu_button_count_get(pr->video) > 0)
     pr->menu_visible = 1;
   else
     pr->menu_visible = 0;
}

static void
key_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Key_Down *ev;
   Elation_Module *em;
   Elation_Module_Private *pr;
   int action = ELATION_ACT_NONE;
   
   ev = (Evas_Event_Key_Down *)event_info;
   em = data;
   pr = em->data;
   
   /* translator */
   /* FIXME: create proper translator sys later to amke remote config easy */
   
   if (!strcmp(ev->keyname, "Escape"))      action = ELATION_ACT_EXIT;
   else if (!strcmp(ev->keyname, "Up"))     action = ELATION_ACT_UP;
   else if (!strcmp(ev->keyname, "Down"))   action = ELATION_ACT_DOWN;
   else if (!strcmp(ev->keyname, "Left"))   action = ELATION_ACT_LEFT;
   else if (!strcmp(ev->keyname, "Right"))  action = ELATION_ACT_RIGHT;
   else if (!strcmp(ev->keyname, "Return")) action = ELATION_ACT_SELECT;
   else if (!strcmp(ev->keyname, "Prior"))  action = ELATION_ACT_PREV;
   else if (!strcmp(ev->keyname, "Next"))   action = ELATION_ACT_NEXT;
   else if (!strcmp(ev->keyname, "m"))      action = ELATION_ACT_MENU;
   else if (!strcmp(ev->keyname, "i"))      action = ELATION_ACT_INFO;
   else if (!strcmp(ev->keyname, "o"))      action = ELATION_ACT_INPUT;
   else if (!strcmp(ev->keyname, "p"))      action = ELATION_ACT_PLAY;
   else if (!strcmp(ev->keyname, "a"))      action = ELATION_ACT_PAUSE;
   else if (!strcmp(ev->keyname, "s"))      action = ELATION_ACT_STOP;
   else if (!strcmp(ev->keyname, "k"))      action = ELATION_ACT_SKIP;
   em->action(em, action);
}

/* seriously - there should be a media detect module all on its own */
static int
media_check_timer_cb(void *data)
{
   Elation_Module *em;
   Elation_Module_Private *pr;
   dvdnav_t *dvd;
   int ok = 0;
   int fd;
   
   em = data;
   pr = em->data;

   if (pr->have_media) return 1;
   fd = open("/dev/dvd", O_RDONLY | O_NONBLOCK);
   if (fd >= 0)
     {
	if (pr->check_media_done)
	  {
	     struct cdrom_generic_command cgc;
	     struct request_sense sense;
	     unsigned char buffer[8];
	     int ret;
	     
	     memset(&sense, 0, sizeof(sense));
	     cgc.cmd[0] = GPCMD_GET_EVENT_STATUS_NOTIFICATION;
	     cgc.cmd[1] = 1;
	     cgc.cmd[4] = 16;
	     cgc.cmd[8] = sizeof(buffer);
	     cgc.timeout = 600;
	     cgc.buffer = buffer;
	     cgc.buflen = sizeof(buffer);
	     cgc.data_direction = CGC_DATA_READ;
	     cgc.sense = &sense;
	     cgc.quiet = 1;
	     ret = ioctl(fd, CDROM_SEND_PACKET, &cgc);
	     if (ret == -1)
	       ok = 0;
	     else
	       {
		  int val;
		  
		  val = buffer[4] & 0xf;
		  /* 3 = eject request */
		  /* 2 = inseted new disk */
		  if ((val == 2) || (val == 4)) ok = 1;
	       }
	  }
	else
	  {
	     unsigned char buffer[8];
	     int ret;
	     
	     ret = read(fd, buffer, 8);
	     if (ret == -1)
	       ok = 0;
	     else
	       ok = 1;
	     pr->check_media_done = 1;
	  }
	close(fd);
     }
   if (ok)
     {
	pr->have_media = 1;
	printf("have media\n");
	edje_object_signal_emit(pr->overlay, "media", "1");
	pr->media_play_timer = ecore_timer_add(2.0, media_play_timer_cb, em);
     }
   else
     {
	pr->have_media = 0;
     }
   return 1;
}

static int
media_play_timer_cb(void *data)
{
   Elation_Module *em;
   Elation_Module_Private *pr;
   
   em = data;
   pr = em->data;
   pr->media_play_timer = NULL;
   if (!pr->have_media) return 0;
   
   edje_object_signal_emit(pr->overlay, "media", "ok");
   emotion_object_file_set(pr->video, "dvd:/");
   emotion_object_play_set(pr->video, 1);
   if (evas_object_visible_get(pr->overlay))
     {
	evas_object_color_set(pr->background1, 0, 0, 0, 0);
	evas_object_color_set(pr->background2, 0, 0, 0, 0);
	evas_object_color_set(pr->video, 255, 255, 255, 0);
	evas_object_show(pr->background1);
	evas_object_show(pr->background2);
	evas_object_show(pr->video);
     }
   em->resize(em);
   
   pr->media_fade_in_timer = ecore_timer_add(1.0 / 30.0, media_fade_in_timer_cb, em);
   pr->media_fade_in_start = ecore_time_get();
   
   return 0;
}

static int
media_fade_in_timer_cb(void *data)
{
   Elation_Module *em;
   Elation_Module_Private *pr;
   int a;
   double t;
   
   em = data;
   pr = em->data;
   t = ecore_time_get() - pr->media_fade_in_start;
   a = (t * 255) / 10.0;
   if (a > 255) a = 255;
   evas_object_color_set(pr->background1, 0, 0, 0, a);
   evas_object_color_set(pr->background2, 0, 0, 0, a);
   evas_object_color_set(pr->video, 255, 255, 255, a);
   if (a < 255) return 1;
   pr->media_fade_in_timer = NULL;
   return 0;
}
