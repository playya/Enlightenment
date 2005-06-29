/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

/* local subsystem functions */
static int _e_bg_animator(void *data);

/* local subsystem globals */

/* externally accessible functions */
void
e_bg_zone_update(E_Zone *zone, E_Bg_Transition transition)
{
   Evas_Object *o;

   if (transition == E_BG_TRANSITION_START)
     {
	zone->bg_transition_mode = e_config->desktop_bg_start_transition;
	zone->bg_transition_time = e_config->desktop_bg_start_transition_time;
     }
   else if (transition == E_BG_TRANSITION_DESK)
     {
	zone->bg_transition_mode = e_config->desktop_bg_desk_transition;
	zone->bg_transition_time = e_config->desktop_bg_desk_transition_time;
     }
   else if (transition == E_BG_TRANSITION_CHANGE)
     {
	zone->bg_transition_mode = e_config->desktop_bg_change_transition;
	zone->bg_transition_time = e_config->desktop_bg_change_transition_time;
     }
   if ((zone->bg_transition_mode == E_BG_TRANSITION_MODE_NONE) ||
       (zone->bg_transition_time == 0.0))
     transition = E_BG_TRANSITION_NONE;
   if (zone->bg_transition_mode == E_BG_TRANSITION_MODE_RANDOM)
     {
	zone->bg_transition_mode = 
	  (rand() % (E_BG_TRANSITION_MODE_LAST - E_BG_TRANSITION_MODE_RANDOM))
	  + E_BG_TRANSITION_MODE_RANDOM + 1;
	if (zone->bg_transition_mode <= E_BG_TRANSITION_MODE_RANDOM)
	  zone->bg_transition_mode = E_BG_TRANSITION_MODE_RANDOM + 1;
	else if (zone->bg_transition_mode >= E_BG_TRANSITION_MODE_LAST)
	  zone->bg_transition_mode = E_BG_TRANSITION_MODE_LAST - 1;
     }
   if (transition == E_BG_TRANSITION_NONE)
     {
	if (zone->bg_object)
	  {
	     evas_object_del(zone->bg_object);
	     zone->bg_object = NULL;
	  }
     }
   if (transition != E_BG_TRANSITION_NONE)
     {
	if (zone->bg_object)
	  {
	     if (zone->prev_bg_object)
	       evas_object_del(zone->prev_bg_object);
	     zone->prev_bg_object = zone->bg_object;
	     zone->bg_object = NULL;
	  }
     }
   o = edje_object_add(zone->container->bg_evas);
   zone->bg_object = o;
   evas_object_data_set(o, "e_zone", zone);
   evas_object_move(o, zone->x, zone->y);
   evas_object_resize(o, zone->w, zone->h);
   
   /* FIXME: check config and look for a special bg for the current desk */
   if (!edje_object_file_set(o,
			     e_config->desktop_default_background,
			     "desktop/background"))
     {
	e_theme_edje_object_set(o, "base/theme/background",
				"desktop/background");
     }
   evas_object_layer_set(o, -1);
   evas_object_lower(o);

   evas_object_clip_set(o, zone->bg_clip_object);
   evas_object_show(o);
   
   if (zone->prev_bg_object)
     {
	const char *pfile =  "", *pgroup = "", *file = "", *group = "";
	
	edje_object_file_get(zone->prev_bg_object, &pfile, &pgroup);
	edje_object_file_get(zone->bg_object, &file, &group);
	if ((pfile) && (file) && (!strcmp(pfile, file)) &&
	    (pgroup) && (group) && (!strcmp(pgroup, group)))
	  {
	     evas_object_del(zone->prev_bg_object);
	     zone->prev_bg_object = NULL;
	     return;
	  }
     }
   
   if (transition != E_BG_TRANSITION_NONE)
     {
	if (!zone->bg_animator)
	  zone->bg_animator= ecore_animator_add(_e_bg_animator, zone);
	zone->bg_set_time = ecore_time_get();
     }
}

/* local subsystem functions */

static int
_e_bg_animator(void *data)
{
   E_Zone *zone;
   double t;
   int a;
   
   zone = data;
   /* t is an animating INDEX 0.0 - 1.0, it is used as a lookup into
    * the effect. 1.0 == finished */
   t = (ecore_time_get() - zone->bg_set_time) / zone->bg_transition_time;
   if (t < 0.0) t = 0.0;
   else if (t > 1.0) t = 1.0;
   
   if (zone->bg_transition_mode == E_BG_TRANSITION_MODE_FADE)
     {
	a = (1.0 - t) * 255.0;
	if (a < 0) a = 0;
	else if (a > 255) a = 255;
	evas_object_color_set(zone->prev_bg_object,
			      255, 255, 255, a);
     }
   else if (zone->bg_transition_mode == E_BG_TRANSITION_MODE_SINUSOUDAL_FADE)
     {
	double t2;
	
	t2 = (1.0 - cos(t * M_PI)) / 2.0;
	
	a = (1.0 - t2) * 255.0;
	if (a < 0) a = 0;
	else if (a > 255) a = 255;
	evas_object_color_set(zone->prev_bg_object,
			      255, 255, 255, a);
     }
     
   /* if we still animate.. */
   if (t < 1.0) return 1;
   
   if (zone->prev_bg_object)
     {
	evas_object_del(zone->prev_bg_object);
	zone->prev_bg_object = NULL;
     }
   zone->bg_animator = NULL;
   return 0;
}
