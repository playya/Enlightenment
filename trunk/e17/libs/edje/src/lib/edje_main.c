#include "Edje.h"
#include "edje_private.h"

Edje *_edje_fetch(Evas_Object *obj);

static void _edje_smart_add(Evas_Object * obj);
static void _edje_smart_del(Evas_Object * obj);
static void _edje_smart_layer_set(Evas_Object * obj, int layer);
static void _edje_smart_raise(Evas_Object * obj);
static void _edje_smart_lower(Evas_Object * obj);
static void _edje_smart_stack_above(Evas_Object * obj, Evas_Object * above);
static void _edje_smart_stack_below(Evas_Object * obj, Evas_Object * below);
static void _edje_smart_move(Evas_Object * obj, double x, double y);
static void _edje_smart_resize(Evas_Object * obj, double w, double h);
static void _edje_smart_show(Evas_Object * obj);
static void _edje_smart_hide(Evas_Object * obj);
static void _edje_smart_color_set(Evas_Object * obj, int r, int g, int b, int a);
static void _edje_smart_clip_set(Evas_Object * obj, Evas_Object * clip);
static void _edje_smart_clip_unset(Evas_Object * obj);

static Evas_Smart *_edje_smart = NULL;

Evas_Object *
edje_add(Evas *evas)
{
   if (!_edje_smart)
     _edje_smart = evas_smart_new("edje",
				  _edje_smart_add,
				  _edje_smart_del,
				  _edje_smart_layer_set,
				  _edje_smart_raise,
				  _edje_smart_lower,
				  _edje_smart_stack_above,
				  _edje_smart_stack_below,
				  _edje_smart_move,
				  _edje_smart_resize,
				  _edje_smart_show,
				  _edje_smart_hide,
				  _edje_smart_color_set,
				  _edje_smart_clip_set, 
				  _edje_smart_clip_unset, 
				  NULL);
   return evas_object_smart_add(evas, _edje_smart);
}

void
edje_file_set(Evas_Object *obj, const char *file, const char *part)
{
   Edje *ed;
   
   ed = _edje_fetch(obj);
   if (!ed) return;
}

/*** internal calls ***/

/* manipulation calls */

static void
_edje_part_description_apply(Edje *ed, 
			     Edje_Real_Part *ep, 
			     Edje_Part_Description *de1, 
			     Edje_Part_Description *de2, 
			     double pos)
{
   if ((ep->param1.description == de1) && 
       (ep->param2.description == de2) && 
       (ep->description_pos == pos))
     return;
   
   ep->param1.description = de1;
   ep->param2.description = de2;
   ep->description_pos = pos;
   
   ed->dirty = 1;
   ep->dirty = 1;
}

/* calculation functions */

static void
_edje_part_recalc_single(Edje *ed,
			 Edje_Real_Part *ep, 
			 Edje_Part_Description *desc, 
			 Edje_Real_Part *rel1_to, 
			 Edje_Real_Part *rel2_to, 
			 Edje_Real_Part *confine_to,
			 Edje_Calc_Params *params)
{
   /* relative coords of top left & bottom right */
   if (rel1_to)
     {
	params->x = desc->rel1.offset_x +
	  rel1_to->x + (desc->rel1.relative_x * rel1_to->w);
	params->y = desc->rel1.offset_y +
	  rel1_to->y + (desc->rel1.relative_y * rel1_to->h);
     }
   else
     {
	params->x = desc->rel1.offset_x +
	  (desc->rel1.relative_x * ed->w);
	params->y = desc->rel1.offset_y +
	  (desc->rel1.relative_y * ed->h);
     }
   if (rel2_to)
     {
	params->w = desc->rel2.offset_x +
	  rel2_to->x + (desc->rel2.relative_x * rel2_to->w) -
	  params->x;
	params->h = desc->rel2.offset_y +
	  rel2_to->y + (desc->rel2.relative_y * rel2_to->h) -
	  params->y;
     }
   else
     {
	params->w = desc->rel2.offset_x +
	  (desc->rel2.relative_x * ed->w) -
	  params->x;
	params->h = desc->rel2.offset_y +
	  (desc->rel2.relative_y * ed->h) -
	  params->y;
     }   
   /* aspect */
   if (params->h > 0)
     {
	double aspect;
	int new_h;
   
	new_h = params->h;
	aspect = (double)params->w / (double)params->h;
	/* adjust for max aspect (width / height) */
	if ((desc->aspect.max > 0.0) && (aspect > desc->aspect.max))
	  {
	     new_h = (int)((double)params->w / desc->aspect.max);
	  }
	/* adjust for min aspect (width / height) */
	if ((desc->aspect.min > 0.0) && (aspect < desc->aspect.min))
	  {
	     new_h = (int)((double)params->w / desc->aspect.min);
	  }
	/* do real adjustment */
	if (params->h < new_h)
	  {
	     params->y = params->y +
	       ((params->h - new_h) * (1.0 - desc->align.y));
	     params->h = new_h;
	  }
	else if (params->h > new_h)
	  {
	     params->y = params->y +
	       ((params->h - new_h) * desc->align.y);
	     params->h = new_h;
	  }	  
     }
   /* size step */
   if (desc->step.x > 0)
     {
	int steps;
	int new_w;
	
	steps = params->w / desc->step.x;
	new_w = desc->step.x * steps;
	if (params->w > new_w)
	  {
	     params->x = params->x +
	       ((params->w - new_w) * desc->align.x);
	     params->w = new_w;
	  }	
     }
   if (desc->step.y > 0)
     {
	int steps;
	int new_h;
	
	steps = params->h / desc->step.y;
	new_h = desc->step.y * steps;
	if (params->h > new_h)
	  {
	     params->y = params->y +
	       ((params->h - new_h) * desc->align.y);
	     params->h = new_h;
	  }	
     }
   /* adjust for max size */
   if (desc->max.w >= 0)
     {
	if (params->w > desc->max.w)
	  {
	     params->x = params->x + 
	       ((params->w - desc->max.w) * desc->align.x);
	     params->w = desc->max.w;
	  }
     }
   if (desc->max.h >= 0)
     {
	if (params->h > desc->max.h)
	  {
	     params->y = params->y + 
	       ((params->h - desc->max.h) * desc->align.y);
	     params->h = desc->max.h;
	  }
     }
   /* adjust for min size */
   if (desc->min.w >= 0)
     {
	if (params->w < desc->min.w)
	  {
	     params->x = params->x + 
	       ((params->w - desc->min.w) * (1.0 - desc->align.x));
	     params->w = desc->min.w;
	  }
     }
   if (desc->min.h >= 0)
     {
	if (params->h < desc->min.h)
	  {
	     params->y = params->y + 
	       ((params->h - desc->min.h) * (1.0 - desc->align.y));
	     params->h = desc->min.h;
	  }
     }
   /* confine */
   if (confine_to)
     {
	int offset;
	int step;
	
	/* complex dragable params */
	offset = params->x + ep->drag.x - confine_to->x;
	if (desc->dragable.step_x > 0)
	  {
	     params->x = confine_to->x + 
	       ((offset / desc->dragable.step_x) * desc->dragable.step_x);
	  }
	else if (desc->dragable.count_x > 0)
	  {
	     step = (confine_to->w - params->w) / desc->dragable.count_x;
	     params->x = confine_to->x +
	       ((offset / step) * step);	       
	  }
	offset = params->y + ep->drag.y - confine_to->y;
	if (desc->dragable.step_y > 0)
	  {
	     params->y = confine_to->y + 
	       ((offset / desc->dragable.step_y) * desc->dragable.step_y);
	  }
	else if (desc->dragable.count_y > 0)
	  {
	     step = (confine_to->h - params->h) / desc->dragable.count_y;
	     params->y = confine_to->y +
	       ((offset / step) * step);	       
	  }
	/* limit to confine */
	if (params->x < confine_to->x)
	  {
	     params->x = confine_to->x;
	  }
	if ((params->x + params->w) > (confine_to->x + confine_to->w))
	  {
	     params->x = confine_to->w - params->w;
	  }
	if (params->y < confine_to->y)
	  {
	     params->y = confine_to->y;
	  }
	if ((params->y + params->h) > (confine_to->y + confine_to->h))
	  {
	     params->y = confine_to->h - params->y;
	  }
     }
   else
     {
	/* simple dragable params */
	params->x += ep->drag.x;
	params->y += ep->drag.y;
     }
   /* fill */
   params->fill.x = desc->fill.pos_abs_x + (params->w * desc->fill.pos_rel_x);
   params->fill.w = desc->fill.abs_x + (params->w * desc->fill.rel_x);
   params->fill.y = desc->fill.pos_abs_y + (params->h * desc->fill.pos_rel_y);
   params->fill.h = desc->fill.abs_y + (params->h * desc->fill.rel_y);
   /* colors */
   params->color.r = desc->color.r;
   params->color.g = desc->color.g;
   params->color.b = desc->color.b;
   params->color.a = desc->color.a;
   params->color2.r = desc->color2.r;
   params->color2.g = desc->color2.g;
   params->color2.b = desc->color2.b;
   params->color2.a = desc->color2.a;
   params->color3.r = desc->color3.r;
   params->color3.g = desc->color3.g;
   params->color3.b = desc->color3.b;
   params->color3.a = desc->color3.a;
   /* visible */
   params->visible = desc->visible;
   /* border */
   params->border.l = desc->border.l;
   params->border.r = desc->border.r;
   params->border.t = desc->border.t;
   params->border.b = desc->border.b;
   /* text */
   /* FIXME: do */   
}

static void
_edje_part_recalc(Edje *ed, Edje_Real_Part *ep)
{
   Edje_Calc_Params p1, p2;
   
   if (ep->calculated) return;
   if (ep->param1.rel1_to)    _edje_part_recalc(ed, ep->param1.rel1_to);
   if (ep->param1.rel2_to)    _edje_part_recalc(ed, ep->param1.rel2_to);
   if (ep->param1.confine_to) _edje_part_recalc(ed, ep->param1.confine_to);
   if (ep->param2.rel1_to)    _edje_part_recalc(ed, ep->param2.rel1_to);
   if (ep->param2.rel2_to)    _edje_part_recalc(ed, ep->param2.rel2_to);
   if (ep->param2.confine_to) _edje_part_recalc(ed, ep->param2.confine_to);
   
   /* actually calculate now */
   if (ep->param1.description)
     _edje_part_recalc_single(ed, ep, ep->param1.description, ep->param1.rel1_to, ep->param1.rel2_to, ep->param1.confine_to, &p1);
   if (ep->param2.description)
     _edje_part_recalc_single(ed, ep, ep->param1.description, ep->param2.rel1_to, ep->param2.rel2_to, ep->param2.confine_to, &p2);
   
   ep->calculated = 1;
   ep->dirty = 0;
}

static void
_edje_recalc(Edje *ed)
{
   Evas_List *l;
   
   if (!ed->dirty) return;
   for (l = ed->parts; l; l = l->next)
     {
	Edje_Real_Part *ep;
	
	ep = l->data;
	ep->calculated = 0;
     }
   for (l = ed->parts; l; l = l->next)
     {
	Edje_Real_Part *ep;
	
	ep = l->data;
	if (!ep->calculated) _edje_part_recalc(ed, ep);
     }
   ed->dirty = 0;
}

/* utility functions we will use a lot */

Edje *
_edje_fetch(Evas_Object *obj)
{
   Edje *ed;
   char *type;
   
   type = (char *)evas_object_type_get(obj);
   if (!type) return NULL;
   if (strcmp(type, "edje")) return NULL;
   ed = evas_object_smart_data_get(obj);
   return ed;
}

/* evas smart object methods - required by evas smart objects to do the */
/* dirty work on smrt objects */

static void
_edje_smart_add(Evas_Object * obj)
{
   Edje *ed;
   
   ed = calloc(1, sizeof(Edje));
   if (!ed) return;
   evas_object_smart_data_set(obj, ed);
   ed->evas = evas_object_evas_get(obj);
   ed->clipper = evas_object_rectangle_add(ed->evas);
   evas_object_smart_member_add(ed->clipper, obj);
   evas_object_color_set(ed->clipper, 255, 255, 255, 255);
   evas_object_move(ed->clipper, 0, 0);
   evas_object_resize(ed->clipper, 0, 0);
}

static void
_edje_smart_del(Evas_Object * obj)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   evas_object_del(ed->clipper);
   free(ed);
}

static void
_edje_smart_layer_set(Evas_Object * obj, int layer)
{
   Edje *ed;
   Evas_List *l;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   if (ed->layer == layer) return;
   ed->layer = layer;
   for (l = ed->parts; l; l = l->next)
     {
	Edje_Real_Part *ep;
	
	ep = l->data;
	evas_object_layer_set(ep->object, ed->layer);
     }
}

static void
_edje_smart_raise(Evas_Object * obj)
{
   Edje *ed;
   Evas_List *l;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   for (l = ed->parts; l; l = l->next)
     {
	Edje_Real_Part *ep;
	
	ep = l->data;
	evas_object_raise(ep->object);
     }
}

static void
_edje_smart_lower(Evas_Object * obj)
{
   Edje *ed;
   Evas_List *l;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   for (l = evas_list_last(ed->parts); l; l = l->prev)
     {
	Edje_Real_Part *ep;
	
	ep = l->data;
	evas_object_lower(ep->object);
     }
}

static void 
_edje_smart_stack_above(Evas_Object * obj, Evas_Object * above)
{
   Edje *ed;
   Evas_List *l;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   for (l = evas_list_last(ed->parts); l; l = l->prev)
     {
	Edje_Real_Part *ep;
	
	ep = l->data;
	evas_object_stack_above(ep->object, above);
     }
}

static void
_edje_smart_stack_below(Evas_Object * obj, Evas_Object * below)
{
   Edje *ed;
   Evas_List *l;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   for (l = ed->parts; l; l = l->next)
     {
	Edje_Real_Part *ep;
	
	ep = l->data;
	evas_object_stack_below(ep->object, below);
     }
}

static void 
_edje_smart_move(Evas_Object * obj, double x, double y)
{
   Edje *ed;
   Evas_List *l;
   
   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   ed->x = x;
   ed->y = y;

   evas_object_move(ed->clipper, ed->x, ed->y);
   
   for (l = ed->parts; l; l = l->next)
     {
	Edje_Real_Part *ep;
	
	ep = l->data;
	evas_object_move(ep->object, ed->x + ep->x, ed->y + ep->y);
     }
}

static void 
_edje_smart_resize(Evas_Object * obj, double w, double h)
{
   Edje *ed;
   int nw, nh;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   nw = w;
   nh = h;
   ed->w = w;
   ed->h = h;
   if ((nw == ed->w) || (nh == ed->h)) return;
   evas_object_resize(ed->clipper, ed->w, ed->h);
   ed->dirty = 1;
   _edje_recalc(ed);
}

static void 
_edje_smart_show(Evas_Object * obj)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   evas_object_show(ed->clipper);
}

static void 
_edje_smart_hide(Evas_Object * obj)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   evas_object_hide(ed->clipper);
}

static void 
_edje_smart_color_set(Evas_Object * obj, int r, int g, int b, int a)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   evas_object_color_set(ed->clipper, r, g, b, a);
}

static void 
_edje_smart_clip_set(Evas_Object * obj, Evas_Object * clip)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   evas_object_clip_set(ed->clipper, clip);
}

static void 
_edje_smart_clip_unset(Evas_Object * obj)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   evas_object_clip_unset(ed->clipper);
}
