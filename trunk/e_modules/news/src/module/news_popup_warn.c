#include "News.h"

static Evas_List *_popups_warn;

static void _check_overlap(int *px, int *py, int *pw, int *ph, int tries, int org_x, int org_y);
static void _try_close(News_Popup_Warn *popw);

static int  _cb_timer(void *data);
static void _cb_edje_close(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _cb_edje_desactivate(void *data, Evas_Object *obj, const char *emission, const char *source);
/*
 * Public functions
 */

int
news_popup_warn_init(void)
{
   _popups_warn = NULL;

   return 1;
}

void
news_popup_warn_shutdown(void)
{
   Evas_List *l;

   for (l = _popups_warn; l; l = evas_list_next(l))
     {
        News_Popup_Warn *p;
        p = evas_list_data(l);
        news_popup_warn_del(p);
     }
   evas_list_free(_popups_warn);
   _popups_warn = NULL;
}

News_Popup_Warn *
news_popup_warn_add(int type, const char *title, const char *text, int timer, int (*func_close) (News_Popup_Warn *popw, void *data), void (func_desactivate) (News_Popup_Warn *popw, void *data), void *data)
{
  E_Zone *zone;
  News_Popup_Warn *popw;
  int fw, fh;
  int ecanvas_w, ecanvas_h;

  popw = E_NEW(News_Popup_Warn, 1);

  popw->type = type;
  popw->timer_org = timer;

  zone = e_util_zone_current_get(e_manager_current_get());
  news_util_ecanvas_geometry_get(&ecanvas_w, &ecanvas_h);

  /* pop */
  popw->pop = e_popup_new(zone, 0, 0, 1, 1);
  if (!popw->pop)
    {
      news_popup_warn_del(popw);
      return 0;
    }
  evas_event_freeze(popw->pop->evas);
  e_popup_layer_set(popw->pop, 255);


  /* face and tb */
  popw->face = edje_object_add(popw->pop->evas);
  news_theme_edje_set(popw->face, NEWS_THEME_POPW);
  edje_object_signal_callback_add(popw->face, "close", "popup",
				  _cb_edje_close, popw);
  edje_object_signal_callback_add(popw->face, "desactivate", "popup",
				  _cb_edje_desactivate, popw);
  // TODO

  /* log */
  // TODO

  /* update times */
  // TODO

  /* type */
  edje_object_message_send(popw->face, EDJE_MESSAGE_INT,
			   NEWS_POPUP_WARN_EDJE_MESSAGE_TYPE,
			   &type);

  /* pos */
  popw->x = ecanvas_w - (fw + 20);;
  popw->y = ecanvas_h - (fh + 20);;
  popw->w = fw;
  popw->h = fh;

  /* timer */
  if (timer)
    popw->timer = ecore_timer_add(timer, _cb_timer, popw);

  /* close and desactivate functions */
  popw->func_close = func_close;
  if (func_desactivate)
    {
      int show_desactivate = 1;

      popw->func_desactivate = func_desactivate;
      edje_object_message_send(popw->face, EDJE_MESSAGE_INT,
			       NEWS_POPUP_WARN_EDJE_MESSAGE_SHOW_DESACTIVATE,
			       &show_desactivate);
    }

  /* attach data */
  popw->data = data;

  /* check for popup overlaps */
  _check_overlap(&popw->x, &popw->y, &popw->w, &popw->h, 0, popw->x, popw->y);
  e_popup_move_resize(popw->pop, popw->x, popw->y, popw->w, popw->h);
  DPOPW(("New: %dx%d : %dx%d", popw->x, popw->y, popw->w, popw->h));
  DPOPW(("New face: %dx%d", fw, fh));

  /* go ! */
  evas_object_show(popw->face);
  e_popup_edje_bg_object_set(popw->pop, popw->face);
  evas_event_thaw(popw->pop->evas);
  e_popup_show(popw->pop);
  
  _popups_warn = evas_list_append(_popups_warn, popw);
	
   return popw;
}

void
news_popup_warn_del(News_Popup_Warn *popw)
{
  if (popw->timer)
    ecore_timer_del(popw->timer);
  if (popw->tb)
    evas_object_del(popw->tb);
  if (popw->face)
    evas_object_del(popw->face);
  if (popw->pop)
    e_object_del(E_OBJECT(popw->pop));

  _popups_warn = evas_list_remove(_popups_warn, popw);

  free(popw);
}


/*
 * Private functions
 *
 */

static void
_check_overlap(int *px, int *py, int *pw, int *ph, int tries, int org_x, int org_y)
{
   Evas_List *l;
   News_Popup_Warn *p;
   int pxw, pyh;
   int p_xw, p_yh;

   pxw = *px + *pw;
   pyh = *py + *ph;
   for (l = _popups_warn; l; l = evas_list_next(l))
     {
        p = evas_list_data(l);
        p_xw = p->x + p->w;
        p_yh = p->y + p->h;
        if (((p->x >= *px) && (p->x <= pxw) &&
             (p->y >= *py) && (p->y <= pyh))
            ||
            ((*px >= p->x) && (*px <= p_xw) &&
             (*py >= p->y) && (*py <= p_yh)))
          {
             /* overlap ! correct coords */
             /* try upper, and then on the left */
             /* TODO...: try down and right, maybe placement policy ? */
             DPOPW(("Overlap !"));
             *py = p->y - (*ph + NEWS_POPUP_WARN_OVERLAP_BORDER);
             if (*py < 0)
               {
                  *py = org_y;
                  *px = *px - (*px + NEWS_POPUP_WARN_OVERLAP_BORDER);
                  if (*px < 0) break;
               }
             tries++;
             if (tries > NEWS_POPUP_WARN_OVERLAP_CHECK_MAX)
               return;
             else
               _check_overlap(px, py, pw, ph, tries, org_x, org_y);
             break;
          }
     }
}

static void
_try_close(News_Popup_Warn *popw)
{
  int del = 1;

   if (popw->func_close)
     {
       if (!popw->func_close(popw, popw->data))
	 del = 0;
     }

   if (del)
     news_popup_warn_del(popw);
}

static int
_cb_timer(void *data)
{
   News_Popup_Warn *popw;

   popw = data;
   _try_close(popw);

   return 0;
}

static void
_cb_edje_close(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   News_Popup_Warn *popw;

   popw = data;
   _try_close(popw);
}

static void
_cb_edje_desactivate(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   News_Popup_Warn *popw;

   popw = data;
   if (popw->func_desactivate)
     popw->func_desactivate(popw, popw->data);
}
