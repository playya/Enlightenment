#include "Edje.h"
#include "edje_private.h"

Ejde_Text_Style _edje_text_styles[EDJE_TEXT_EFFECT_LAST];

void
_edje_text_init(void)
{
   int i, j, n;
   const vals[5][5] = 
     {
	  {0, 1, 2, 1, 0},
	  {1, 3, 4, 3, 1},
	  {2, 4, 5, 4, 2},
	  {1, 3, 4, 3, 1},
	  {0, 1, 2, 1, 0}
     };
   
   memset(_edje_text_styles, 0, sizeof(_edje_text_styles));
   
   _edje_text_styles[EDJE_TEXT_EFFECT_NONE].num = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_NONE].members[0].alpha = 255;
   
   _edje_text_styles[EDJE_TEXT_EFFECT_PLAIN].num = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_PLAIN].members[0].alpha = 255;

   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].offset.x = 1;   
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].offset.y = 1;   
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].pad.l = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].pad.r = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].pad.t = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].pad.b = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].num = 5;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].members[0].alpha = 255;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].members[1].color = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].members[1].x = 0;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].members[1].y = -1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].members[1].alpha = 255;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].members[2].color = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].members[2].x = 0;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].members[2].y = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].members[2].alpha = 255;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].members[3].color = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].members[3].x = -1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].members[3].y = 0;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].members[3].alpha = 255;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].members[4].color = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].members[4].x = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].members[4].y = 0;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE].members[4].alpha = 255;
   
   _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_OUTLINE].offset.x = 2;
   _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_OUTLINE].offset.y = 2;
   _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_OUTLINE].pad.l = 2;
   _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_OUTLINE].pad.r = 2;
   _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_OUTLINE].pad.t = 2;
   _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_OUTLINE].pad.b = 2;
   _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_OUTLINE].num = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_OUTLINE].members[0].alpha = 255;
   for (j = 0; j < 5; j++)
     {
	for (i = 0; i < 5; i++)
	  {
	     if (((i == 2) && (j == 2)) || (vals[i][j] == 0)) continue;
	     n = _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_OUTLINE].num;
	     _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_OUTLINE].members[n].color = 1;
	     _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_OUTLINE].members[n].x = i - 2;
	     _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_OUTLINE].members[n].y = j - 2;
	     _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_OUTLINE].members[n].alpha = vals[i][j] * 50;
	     _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_OUTLINE].num = n + 1;
	  }
     }
   
   _edje_text_styles[EDJE_TEXT_EFFECT_SHADOW].pad.r = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_SHADOW].pad.b = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_SHADOW].num = 2;
   _edje_text_styles[EDJE_TEXT_EFFECT_SHADOW].members[0].alpha = 255;
   _edje_text_styles[EDJE_TEXT_EFFECT_SHADOW].members[1].color = 2;
   _edje_text_styles[EDJE_TEXT_EFFECT_SHADOW].members[1].x = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_SHADOW].members[1].y = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_SHADOW].members[1].alpha = 255;

   _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_SHADOW].offset.x = 1;   
   _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_SHADOW].offset.y = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_SHADOW].pad.l = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_SHADOW].pad.r = 3;
   _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_SHADOW].pad.t = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_SHADOW].pad.b = 3;
   _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_SHADOW].num = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_SHADOW].members[0].alpha = 255;
   for (j = 0; j < 5; j++)
     {
	for (i = 0; i < 5; i++)
	  {
	     if (vals[i][j] == 0) continue;
	     n = _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_SHADOW].num;
	     _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_SHADOW].members[n].color = 2;
	     _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_SHADOW].members[n].x = i - 1;
	     _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_SHADOW].members[n].y = j - 1;
	     _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_SHADOW].members[n].alpha = vals[i][j] * 50;
	     _edje_text_styles[EDJE_TEXT_EFFECT_SOFT_SHADOW].num = n + 1;
	  }
     }

   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].offset.x = 1;   
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].offset.y = 1;   
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].pad.l = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].pad.r = 2;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].pad.t = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].pad.b = 2;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].num = 6;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[0].alpha = 255;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[1].color = 2;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[1].x = 2;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[1].y = 2;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[1].alpha = 255;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[2].color = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[2].x = 0;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[2].y = -1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[2].alpha = 255;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[3].color = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[3].x = 0;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[3].y = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[3].alpha = 255;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[4].color = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[4].x = -1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[4].y = 0;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[4].alpha = 255;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[5].color = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[5].x = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[5].y = 0;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SHADOW].members[5].alpha = 255;
   
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].offset.x = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].offset.y = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].pad.l = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].pad.r = 3;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].pad.t = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].pad.b = 3;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].num = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[0].alpha = 255;
   for (j = 0; j < 5; j++)
     {
	for (i = 0; i < 5; i++)
	  {
	     if (vals[i][j] == 0) continue;
	     n = _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].num;
	     _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n].color = 2;
	     _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n].x = i - 1;
	     _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n].y = j - 1;
	     _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n].alpha = vals[i][j] * 50;
	     _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].num = n + 1;
	  }
     }
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n + 1].color = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n + 1].x = 0;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n + 1].y = -1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n + 1].alpha = 255;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n + 2].color = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n + 2].x = 0;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n + 2].y = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n + 2].alpha = 255;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n + 3].color = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n + 3].x = -1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n + 3].y = 0;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n + 3].alpha = 255;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n + 4].color = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n + 4].x = 1;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n + 4].y = 0;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].members[n + 4].alpha = 255;
   _edje_text_styles[EDJE_TEXT_EFFECT_OUTLINE_SOFT_SHADOW].num += 4;
}

void
_edje_text_part_on_add(Edje *ed, Edje_Real_Part *ep)
{
   int i;

   if (ep->part->type != EDJE_PART_TYPE_TEXT) return;
   if (ep->part->effect >= EDJE_TEXT_EFFECT_LAST) return;
   for (i = 1; i < _edje_text_styles[ep->part->effect].num; i++)
     {
	Evas_Object *o;
	
	o = evas_object_text_add(ed->evas);
	evas_object_smart_member_add(o, ed->obj);
	evas_object_pass_events_set(o, 1);
	evas_object_clip_set(o, ed->clipper);
	evas_object_show(o);
	ep->extra_objects = evas_list_append(ep->extra_objects, o);
     }
}

void
_edje_text_part_on_add_clippers(Edje *ed, Edje_Real_Part *ep)
{
   Evas_List *l;
   
   for (l = ep->extra_objects; l; l = l->next)
     {
	Evas_Object *o;
	
	o = l->data;
	if (ep->part->clip_to_id >= 0)
	  {
	     ep->clip_to = evas_list_nth(ed->parts, ep->part->clip_to_id);
	     if (ep->clip_to)
	       {
		  evas_object_pass_events_set(ep->clip_to->object, 1);
		  evas_object_clip_set(o, ep->clip_to->object);
	       }
	  }
	
     }
}

void
_edje_text_part_on_del(Edje *ed, Edje_Real_Part *ep)
{
   while (ep->extra_objects)
     {
	Evas_Object *o;
	
	o = ep->extra_objects->data;
	ep->extra_objects = evas_list_remove(ep->extra_objects, o);
	evas_object_del(o);
     }
}

void
_edje_text_recalc_apply(Edje *ed, Edje_Real_Part *ep,
			Edje_Calc_Params *params,
			Edje_Part_Description *chosen_desc)
{
   char   *text;
   char   *font;
   int     size;
   double  tw, th;
   double  ox, oy, sw, sh;
   char    *buf = NULL;
   
   text = chosen_desc->text.text;
   font = chosen_desc->text.font;
   size = chosen_desc->text.size;
   if (ep->text.text) text = ep->text.text;
   if (ep->text.font) font = ep->text.font;
   if (ep->text.size > 0) size = ep->text.size;
   ox = _edje_text_styles[ep->part->effect].offset.x;
   oy = _edje_text_styles[ep->part->effect].offset.y;
   sw = params->w - (_edje_text_styles[ep->part->effect].pad.l + _edje_text_styles[ep->part->effect].pad.r);
   sh = params->h - (_edje_text_styles[ep->part->effect].pad.t + _edje_text_styles[ep->part->effect].pad.b);
   if ((ep->text.cache.in_size == size) &&
       (ep->text.cache.in_w == sw) &&
       (ep->text.cache.in_h == sh) &&
       (ep->text.cache.in_str) &&
       (text) &&
       (!strcmp(ep->text.cache.in_str, text)))
     {
	text = ep->text.cache.out_str;
	size = ep->text.cache.out_size;
	goto arrange_text;
     }
   if (ep->text.cache.in_str) free(ep->text.cache.in_str);
   ep->text.cache.in_str = strdup(text);
   ep->text.cache.in_size = size;
   if (chosen_desc->text.fit_x)
     {
	evas_object_text_font_set(ep->object, font, size);
	evas_object_text_text_set(ep->object, text);
	evas_object_geometry_get(ep->object, NULL, NULL, &tw, &th);
	if (tw > sw)
	  {
	     int psize;
	     
	     psize = size;
	     while ((tw > sw) && (size > 0))
	       {
		  psize = size;
		  size = (size * sw) / tw;
		  if ((psize - size) <= 0) size = psize - 1;
		  evas_object_text_font_set(ep->object, font, size);
		  evas_object_geometry_get(ep->object, NULL, NULL, &tw, &th);
	       }
	  }
	else if (tw < sw)
	  {
	     int psize;
	     
	     psize = size;
	     while ((tw < sw) && (size > 0))
	       {
		  psize = size;
		  size = (size * sw) / tw;
		  if ((psize - size) >= 0) size = psize + 1;
		  evas_object_text_font_set(ep->object, font, size);
		  evas_object_geometry_get(ep->object, NULL, NULL, &tw, &th);
	       }
	     size = psize;
	  }
     }
   if (chosen_desc->text.fit_y)
     {
	size = sh;
	evas_object_text_font_set(ep->object, font, size);
	evas_object_text_text_set(ep->object, text);
	evas_object_geometry_get(ep->object, NULL, NULL, &tw, &th);
	if (th < sh)
	  {
	     int dif;
	     
	     dif = (th - sh) / 4;
	     if (dif < 1) dif = 1;
	     while ((th < sh) && (sw > 0.0))
	       {
		  size += dif;
		  evas_object_text_font_set(ep->object, font, size);
		  evas_object_geometry_get(ep->object, NULL, NULL, &tw, &th);
	       }
	     size -= dif;
	  }
	else if (th > sh)
	  {
	     int dif;
	     
	     dif = (th - sh) / 4;
	     if (dif < 1) dif = 1;
	     while ((th < sh) && (sw >= 0.0))
	       {
		  size -= dif;
		  evas_object_text_font_set(ep->object, font, size);
		  evas_object_geometry_get(ep->object, NULL, NULL, &tw, &th);
	       }
	  }
     }
   if (size < 1) size = 1;
   if (!chosen_desc->text.fit_x)
     {
	double p;
	int    c1, c2;
	int    loop;
	int    orig_len;
	
	evas_object_text_font_set(ep->object, font, size);
	evas_object_text_text_set(ep->object, text);
	evas_object_geometry_get(ep->object, NULL, NULL, &tw, &th);
	p = ((sw - tw) * chosen_desc->text.align.x);
	c1 = -1;
	c2 = -1;
	/* chop chop */
	if (tw > sw)
	  {
	     if (chosen_desc->text.align.x != 0.0)
	       c1 = evas_object_text_char_coords_get(ep->object,
						     -p, th / 2,
						     NULL, NULL, NULL, NULL);
	     if (chosen_desc->text.align.x != 1.0)
	       c2 = evas_object_text_char_coords_get(ep->object,
						     -p + sw, th / 2,
						     NULL, NULL, NULL, NULL);
	     if ((c1 < 0) && (c2 < 0))
	       {
		  c1 = 0;
		  c2 = 0;
	       }
	  }
	loop = 0;
	buf = malloc(strlen(text) + 1 + 3 + 3);
	orig_len = strlen(text);
	while (((c1 >= 0) || (c2 >= 0)) && (tw > sw))
	  {
	     loop++;
	     if (sw <= 0.0)
	       {
		  buf[0] = 0;
		  break;
	       }
	     if ((c1 >= 0) && (c2 >= 0))
	       {
		  if ((loop & 0x1))
		    {
		       if (c1 >= 0)
			 c1 = evas_string_char_next_get(text, c1, NULL);
		    }
		  else
		    {
		       if (c2 >= 0)
			 {
			    c2 = evas_string_char_prev_get(text, c2, NULL);
			    if (c2 < 0)
			      {
				 buf[0] = 0;
				 break;
			      }
			 }
		    }
	       }
	     else
	       {
		  if (c1 >= 0)
		    c1 = evas_string_char_next_get(text, c1, NULL);
		  else if (c2 >= 0)
		    {
		       c2 = evas_string_char_prev_get(text, c2, NULL);
		       if (c2 < 0)
			 {
			    buf[0] = 0;
			    break;
			 }
		    }
	       }
	     if ((c1 >= 0) && (c2 >= 0))
	       {
		  if (c1 >= c2)
		    {
		       buf[0] = 0;
		       break;
		    }
	       }
	     else
	       {
		  if (c1 >= orig_len)
		    {
		       buf[0] = 0;
		       break;
		    }
		  else if (c2 == 0)
		    {
		       buf[0] = 0;
		       break;
		    }
	       }
	     buf[0] = 0;
	     if (c1 >= 0)
	       {
		  strcpy(buf, "...");
		  if (c2 >= 0)
		    {
		       strncat(buf, text + c1, c2 - c1);
		       strcat(buf, "...");
		    }
		  else
		    strcat(buf, text + c1);
	       }
	     else
	       {
		  if (c2 >= 0)
		    {
		       strncpy(buf, text, c2);
		       buf[c2] = 0;
		       strcat(buf, "...");
		    }
		  else strcpy(buf, text);
	       }
	     evas_object_text_text_set(ep->object, buf);
	     evas_object_geometry_get(ep->object, NULL, NULL, &tw, &th);
	  }
	if (loop > 0) text = buf;
     }
   if (ep->text.cache.out_str) free(ep->text.cache.out_str);
   ep->text.cache.out_str = strdup(text);
   ep->text.cache.in_w = sw;
   ep->text.cache.in_h = sh;
   ep->text.cache.out_size = size;
   
   arrange_text:
   
   evas_object_text_font_set(ep->object, font, size);
   evas_object_text_text_set(ep->object, text);
   evas_object_geometry_get(ep->object, NULL, NULL, &tw, &th);
   ep->offset.x = ox + ((sw - tw) * chosen_desc->text.align.x);
   ep->offset.y = oy + ((sh - th) * chosen_desc->text.align.y);
   evas_object_move(ep->object,
		    ed->x + params->x + ep->offset.x,
		    ed->y + params->y + ep->offset.y);
   evas_object_color_set(ep->object, 
			 params->color.r, 
			 params->color.g, 
			 params->color.b,
			 (params->color.a * (_edje_text_styles[ep->part->effect].members[0].alpha + 1)) / 256);
   if (params->visible) evas_object_show(ep->object);
   else evas_object_hide(ep->object);
     {
	Evas_List *l;
	int i;
	
	for (i = 1, l = ep->extra_objects; l; l = l->next, i++)
	  {
	     Evas_Object *o;
	     
	     o = l->data;
	     evas_object_text_font_set(o, font, size);
	     evas_object_text_text_set(o, text);
	     evas_object_move(o, 
			      ed->x + params->x + ep->offset.x + _edje_text_styles[ep->part->effect].members[i].x,
			      ed->y + params->y + ep->offset.y + _edje_text_styles[ep->part->effect].members[i].y);
	     if (_edje_text_styles[ep->part->effect].members[i].color == 0)
	       evas_object_color_set(o, 
				     params->color.r, 
				     params->color.g, 
				     params->color.b,
				     (params->color.a * (_edje_text_styles[ep->part->effect].members[i].alpha + 1)) / 256);
	     else if (_edje_text_styles[ep->part->effect].members[i].color == 1)
	       evas_object_color_set(o, 
				     params->color2.r, 
				     params->color2.g, 
				     params->color2.b,
				     (params->color2.a * (_edje_text_styles[ep->part->effect].members[i].alpha + 1)) / 256);
	     else if (_edje_text_styles[ep->part->effect].members[i].color == 2)
	       evas_object_color_set(o, 
				     params->color3.r, 
				     params->color3.g, 
				     params->color3.b, 
				     (params->color3.a * (_edje_text_styles[ep->part->effect].members[i].alpha + 1)) / 256);
	     if (params->visible) evas_object_show(o);
	     else evas_object_hide(o);
	  }
     }
   if (buf) free(buf);
}
