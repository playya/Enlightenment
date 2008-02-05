/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "e.h"

/* local subsystem functions */
static void _e_gadcon_popup_free(E_Gadcon_Popup *pop);

/* externally accessible functions */

EAPI E_Gadcon_Popup *
e_gadcon_popup_new(E_Gadcon_Client *gcc, void (*resize_func) (Evas_Object *obj, int *w, int *h))
{
   E_Gadcon_Popup *pop;
   Evas_Object *o;
   E_Zone *zone;

   pop = E_OBJECT_ALLOC(E_Gadcon_Popup, E_GADCON_POPUP_TYPE, _e_gadcon_popup_free);
   if (!pop) return NULL;
   zone = e_util_zone_current_get(e_manager_current_get());
   pop->win = e_popup_new(zone, 0, 0, 0, 0);
   e_popup_layer_set(pop->win, 255);

   o = edje_object_add(pop->win->evas);
   e_theme_edje_object_set(o, "base/theme/gadman", "e/gadman/popup");
   evas_object_show(o);
   evas_object_move(o, 0, 0);
   e_popup_edje_bg_object_set(pop->win, o);
   pop->o_bg = o;

   pop->gcc = gcc;
   pop->resize_func = resize_func;

   return pop;
}

EAPI void
e_gadcon_popup_content_set(E_Gadcon_Popup *pop, Evas_Object *o)
{
   Evas_Coord w = 0, h = 0;
   Evas_Object *old_o;

   if (!pop) return;
   E_OBJECT_CHECK(pop);
   E_OBJECT_TYPE_CHECK(pop, E_GADCON_POPUP_TYPE);

   old_o = edje_object_part_swallow_get(pop->o_bg, "e.swallow.content");
   if (old_o)
     {
	edje_object_part_unswallow(pop->o_bg, old_o);
	evas_object_del(old_o);
     }
   e_widget_min_size_get(o, &w, &h);
   if ((!w) || (!h)) edje_object_size_min_calc(o, &w, &h);
   edje_extern_object_min_size_set(o, w, h);
   edje_object_part_swallow(pop->o_bg, "e.swallow.content", o);
   edje_object_size_min_calc(pop->o_bg, &pop->w, &pop->h);
   evas_object_resize(pop->o_bg, pop->w, pop->h);
}

EAPI void
e_gadcon_popup_show(E_Gadcon_Popup *pop)
{
   Evas_Object *o;
   Evas_Coord gx, gy, gw, gh, zw, zh, zx;
   Evas_Coord px, py, ww, wh;

   if (!pop) return;
   E_OBJECT_CHECK(pop);
   E_OBJECT_TYPE_CHECK(pop, E_GADCON_POPUP_TYPE);

   if (pop->win->visible) return;

   if ((o = edje_object_part_swallow_get(pop->o_bg, "e.swallow.content")))
     {
	Evas_Coord w = 0, h = 0;

	e_widget_min_size_get(o, &w, &h);
	if ((!w) || (!h)) edje_object_size_min_calc(o, &w, &h);
	edje_extern_object_min_size_set(o, w, h);
     }

   evas_object_show(pop->o_bg);
   edje_object_size_min_calc(pop->o_bg, &ww, &wh);
   e_popup_show(pop->win);

   if (pop->resize_func) pop->resize_func(o, &ww, &wh);
   evas_object_resize(pop->o_bg, ww, wh);
   pop->w = ww;
   pop->h = wh;

   /* Popup positioning */
   e_gadcon_client_geometry_get(pop->gcc, &gx, &gy, &gw, &gh);
   zw = pop->gcc->gadcon->zone->w;
   zh = pop->gcc->gadcon->zone->h;
   zx = pop->gcc->gadcon->zone->x;
   switch (pop->gcc->gadcon->orient)
     {
      case E_GADCON_ORIENT_CORNER_RT:
      case E_GADCON_ORIENT_CORNER_RB:
      case E_GADCON_ORIENT_RIGHT:
	px = gx - pop->w;
	py = gy;
	if (py + pop->h >= zh)
	  py = gy + gh - pop->h;
	break;
      case E_GADCON_ORIENT_LEFT:
      case E_GADCON_ORIENT_CORNER_LT:
      case E_GADCON_ORIENT_CORNER_LB:
	px = gx + gw;
	py = gy;
	if (py + pop->h >= zh)
	  py = gy + gh - pop->h;
	break;
	break;
      case E_GADCON_ORIENT_TOP:
      case E_GADCON_ORIENT_CORNER_TL:
      case E_GADCON_ORIENT_CORNER_TR:
	py = gy + gh;
	px = (gx + (gw / 2)) - (pop->w / 2);
	if (px + pop->w >= zw)
	  px = gx + gw - pop->w;
        else if (px < zx)
          px = zx;
	break;
      case E_GADCON_ORIENT_BOTTOM:
      case E_GADCON_ORIENT_CORNER_BL:
      case E_GADCON_ORIENT_CORNER_BR:
	py = gy - pop->h;
	px = (gx + (gw / 2)) - (pop->w / 2);
	if (px + pop->w >= zw)
	  px = gx + gw - pop->w;
        else if (px < zx)
          px = zx;
	break;
      default:
	e_popup_move_resize(pop->win, 50, 50, pop->w, pop->h);
	return;
     }
   e_popup_move_resize(pop->win, px, py, pop->w, pop->h);
}

EAPI void
e_gadcon_popup_hide(E_Gadcon_Popup *pop)
{
   if (!pop) return;
   E_OBJECT_CHECK(pop);
   E_OBJECT_TYPE_CHECK(pop, E_GADCON_POPUP_TYPE);
   if (pop->pinned) return;
   e_popup_hide(pop->win);
}

EAPI void
e_gadcon_popup_toggle_pinned(E_Gadcon_Popup *pop)
{
   if (!pop) return;
   E_OBJECT_CHECK(pop);
   E_OBJECT_TYPE_CHECK(pop, E_GADCON_POPUP_TYPE);

   if (pop->pinned)
     {
	pop->pinned = 0;
	edje_object_signal_emit(pop->o_bg, "e,state,unpinned", "e");
     }
   else
     {
	pop->pinned = 1;
	edje_object_signal_emit(pop->o_bg, "e,state,pinned", "e");
     }
}

/* local subsystem functions */

static void
_e_gadcon_popup_free(E_Gadcon_Popup *pop)
{
   pop->gcc = NULL;
   if (pop->o_con) evas_object_del(pop->o_con);
   e_object_del(E_OBJECT(pop->win));
   free(pop);
}
