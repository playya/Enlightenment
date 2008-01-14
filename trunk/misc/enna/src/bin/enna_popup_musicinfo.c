/*
 * enna_popup_musicinfo.c
 * Copyright (C) Nicolas Aguirre 2006,2007,2008 <aguirre.nicolas@gmail.com>
 *
 * enna_popyp_musicinfo.c is free software copyrighted by Nicolas Aguirre.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name ``Nicolas Aguirre'' nor the name of any other
 *    contributor may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * enna_popup_musicinfo.c IS PROVIDED BY Nicolas Aguirre ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Nicolas Aguirre OR ANY OTHER CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "enna_popup.h"
#include "enna_popup_musicinfo.h"
#include <stdlib.h>
#include <stdio.h>

#define SMART_NAME "enna_popup_musicinfo"

typedef struct _E_Smart_Data E_Smart_Data;

struct _E_Smart_Data
{
   Evas_Coord          x, y, w, h;
   Evas_Object        *obj;
   char                fill_inside:1;
   Enna               *enna;

};

/* local subsystem functions */
static void         _enna_popup_musicinfo_smart_reconfigure(E_Smart_Data * sd);
static void         _enna_popup_musicinfo_smart_init(void);
static void         _e_smart_add(Evas_Object * obj);
static void         _e_smart_del(Evas_Object * obj);
static void         _e_smart_move(Evas_Object * obj, Evas_Coord x,
				  Evas_Coord y);
static void         _e_smart_resize(Evas_Object * obj, Evas_Coord w,
				    Evas_Coord h);
static void         _e_smart_show(Evas_Object * obj);
static void         _e_smart_hide(Evas_Object * obj);
static void         _e_smart_color_set(Evas_Object * obj, int r, int g,
				       int b, int a);
static void         _e_smart_clip_set(Evas_Object * obj, Evas_Object * clip);
static void         _e_smart_clip_unset(Evas_Object * obj);

/* local subsystem globals */
static Evas_Smart  *_e_smart = NULL;

/* externally accessible functions */
EAPI Evas_Object   *
enna_popup_musicinfo_add(Evas * evas)
{
   _enna_popup_musicinfo_smart_init();
   return evas_object_smart_add(evas, _e_smart);
}

/* local subsystem globals */
static void
_enna_popup_musicinfo_smart_reconfigure(E_Smart_Data * sd)
{
   int                 iw, ih;
   Evas_Coord          x, y, w, h;

   ih = 0;
   ih = 0;
   evas_object_image_size_get(sd->obj, &iw, &ih);
   if (iw < 1)
      iw = 1;
   if (ih < 1)
      ih = 1;

   if (sd->fill_inside)
     {
	w = sd->w;
	h = ((double)ih * w) / (double)iw;
	if (h > sd->h)
	  {
	     h = sd->h;
	     w = ((double)iw * h) / (double)ih;
	  }
     }
   else
     {
	w = sd->w;
	h = ((double)ih * w) / (double)iw;
	if (h < sd->h)
	  {
	     h = sd->h;
	     w = ((double)iw * h) / (double)ih;
	  }
     }
   x = sd->x + ((sd->w - w) / 2);
   y = sd->y + ((sd->h - h) / 2);
   evas_object_move(sd->obj, x, y);
   evas_object_image_fill_set(sd->obj, 0, 0, w, h);
   evas_object_resize(sd->obj, w, h);

}

static void
_enna_popup_musicinfo_smart_init(void)
{
   if (_e_smart)
      return;
   static const Evas_Smart_Class sc = {
      SMART_NAME,
      EVAS_SMART_CLASS_VERSION,
      _e_smart_add,
      _e_smart_del,
      _e_smart_move,
      _e_smart_resize,
      _e_smart_show,
      _e_smart_hide,
      _e_smart_color_set,
      _e_smart_clip_set,
      _e_smart_clip_unset,
      NULL
   };
   _e_smart = evas_smart_class_new(&sc);
}

static void
_e_smart_add(Evas_Object * obj)
{
   E_Smart_Data       *sd;

   sd = calloc(1, sizeof(E_Smart_Data));
   if (!sd)
      return;
   sd->obj = edje_object_add(evas_object_evas_get(obj));
   sd->enna = evas_data_attach_get(evas_object_evas_get(sd->obj));
   sd->x = 0;
   sd->y = 0;
   sd->w = 0;
   sd->h = 0;
   sd->fill_inside = 1;
   edje_object_file_set(sd->obj, enna_config_theme_get(), "enna/music_info");
   edje_object_signal_emit(sd->obj, "enna,state,show", "enna");
   edje_object_part_swallow(sd->enna->edje, "enna.fullscreen", sd->obj);
   evas_object_smart_member_add(sd->obj, obj);
   evas_object_smart_data_set(obj, sd);
}

static void
_e_smart_del(Evas_Object * obj)
{
   E_Smart_Data       *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd)
      return;
   evas_object_del(sd->obj);
   free(sd);
}

static void
_e_smart_move(Evas_Object * obj, Evas_Coord x, Evas_Coord y)
{
   E_Smart_Data       *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd)
      return;
   if ((sd->x == x) && (sd->y == y))
      return;
   sd->x = x;
   sd->y = y;
   _enna_popup_musicinfo_smart_reconfigure(sd);
}

static void
_e_smart_resize(Evas_Object * obj, Evas_Coord w, Evas_Coord h)
{
   E_Smart_Data       *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd)
      return;
   if ((sd->w == w) && (sd->h == h))
      return;
   sd->w = w;
   sd->h = h;
   _enna_popup_musicinfo_smart_reconfigure(sd);
}

static void
_e_smart_show(Evas_Object * obj)
{
   E_Smart_Data       *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd)
      return;
   evas_object_show(sd->obj);
}

static void
_e_smart_hide(Evas_Object * obj)
{
   E_Smart_Data       *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd)
      return;
   evas_object_hide(sd->obj);
}

static void
_e_smart_color_set(Evas_Object * obj, int r, int g, int b, int a)
{
   E_Smart_Data       *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd)
      return;
   evas_object_color_set(sd->obj, r, g, b, a);
}

static void
_e_smart_clip_set(Evas_Object * obj, Evas_Object * clip)
{
   E_Smart_Data       *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd)
      return;
   evas_object_clip_set(sd->obj, clip);
}

static void
_e_smart_clip_unset(Evas_Object * obj)
{
   E_Smart_Data       *sd;

   sd = evas_object_smart_data_get(obj);
   if (!sd)
      return;
   evas_object_clip_unset(sd->obj);
}
