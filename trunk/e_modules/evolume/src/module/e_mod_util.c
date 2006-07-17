
#include <e_mixer.h>
#include "e_mod_main.h"
#include "e_mod_config.h"
#include "e_mod_util.h"
#include "e_mod_slider.h"

Mixer *
e_util_search_mixer_by_name(Evas_List *mixers, Mixer_Name *name)
{
   Mixer *mixer;

   for (; mixers; mixers = evas_list_next(mixers))
     {
        mixer = evas_list_data(mixers);
        DBG(stderr, "mixer->name->real = %s, name->real = %s\n", mixer->name->real, name->real);
        if (mixer->name == name)
           return mixer;
     }

   return NULL;
}

int
e_volume_first_run()
{
   return 1;
}

static int
_conf_sort_cb(void *d1, void *d2)
{
   Config_Mixer_Elem *c1;
   Config_Mixer_Elem *c2;

   c1 = (Config_Mixer_Elem *)d1;
   c2 = (Config_Mixer_Elem *)d2;

   if (c1->active && !c2->active)
      return -1;

   if (!c1->active && c2->active)
      return 1;

   if (c1->weight > c2->weight)
      return -1;

   if (c1->weight < c2->weight)
      return 1;

   return 0;
}

Evas_List *
e_util_wlist_get(Evas_List *melems, Config_Face *conf)
{
   Config_Mixer_Elem *melem_conf;
   Mixer_Elem *melem;
   Evas_List *wlist = NULL;

   for (; melems; melems = evas_list_next(melems))
     {
        melem = evas_list_data(melems);
        melem_conf = e_volume_config_melem_get(melem, conf);

        if (melem_conf->active)
           wlist = evas_list_append(wlist, melem_conf);
     }

   wlist = evas_list_sort(wlist, evas_list_count(wlist), _conf_sort_cb);

   return wlist;
}

static int
_slider_sort_cb(void *d1, void *d2)
{
   Mixer_Slider *s1, *s2;
   Config_Mixer_Elem *c1;
   Config_Mixer_Elem *c2;

   s1 = (Mixer_Slider *)d1;
   s2 = (Mixer_Slider *)d2;

   c1 = s1->conf;
   c2 = s2->conf;

   if (!c1 || !c2)
      return 0;

   if (c1->active && !c2->active)
      return 1;

   if (!c1->active && c2->active)
      return -1;

   if (c1->weight > c2->weight)
      return 1;

   if (c1->weight < c2->weight)
      return -1;

   return 0;
}

Evas_List *
e_util_sliders_sort(Evas_List *sliders)
{
   Evas_List *ret;

   ret = evas_list_sort(sliders, evas_list_count(sliders), _slider_sort_cb);

   return ret;
}

Mixer_Elem *
e_util_melem_get(Config_Mixer_Elem *conf, Evas_List *melems)
{
   Mixer_Elem *melem;

   for (; melems; melems = evas_list_next(melems))
     {
        int elem_id;

        melem = evas_list_data(melems);

        elem_id = GET_ELEM_ID(melem);

        if (elem_id == conf->elem_id)
	  return melem;
     }

   return NULL;
}
