#include "e_illume.h"
#include "policy.h"

/* NB: DIALOG_USES_PIXEL_BORDER is an experiment in setting dialog windows 
 * to use the 'pixel' type border. This is done because some dialogs, 
 * when shown, blend into other windows too much. Pixel border adds a 
 * little distinction between the dialog window and an app window.
 * Disable if this is not wanted */
#define DIALOG_USES_PIXEL_BORDER 1

/* local function prototypes */
static void _policy_border_set_focus(E_Border *bd);
static void _policy_border_move(E_Border *bd, int x, int y);
static void _policy_border_resize(E_Border *bd, int w, int h);
static void _policy_border_hide_below(E_Border *bd);
static void _policy_border_show_below(E_Border *bd);
static void _policy_zone_layout_update(E_Zone *zone);
static void _policy_zone_layout_indicator(E_Border *bd, E_Illume_Config_Zone *cz);
static void _policy_zone_layout_quickpanel(E_Border *bd);
static void _policy_zone_layout_softkey(E_Border *bd, E_Illume_Config_Zone *cz);
static void _policy_zone_layout_keyboard(E_Border *bd, E_Illume_Config_Zone *cz);
static void _policy_zone_layout_home_single(E_Border *bd, E_Illume_Config_Zone *cz);
static void _policy_zone_layout_home_dual_top(E_Border *bd, E_Illume_Config_Zone *cz);
static void _policy_zone_layout_home_dual_custom(E_Border *bd, E_Illume_Config_Zone *cz);
static void _policy_zone_layout_home_dual_left(E_Border *bd, E_Illume_Config_Zone *cz);
static void _policy_zone_layout_fullscreen(E_Border *bd);
static void _policy_zone_layout_app_single(E_Border *bd, E_Illume_Config_Zone *cz);
static void _policy_zone_layout_app_dual_top(E_Border *bd, E_Illume_Config_Zone *cz);
static void _policy_zone_layout_app_dual_custom(E_Border *bd, E_Illume_Config_Zone *cz);
static void _policy_zone_layout_app_dual_left(E_Border *bd, E_Illume_Config_Zone *cz);
static void _policy_zone_layout_dialog(E_Border *bd, E_Illume_Config_Zone *cz);
static void _policy_zone_layout_splash(E_Border *bd, E_Illume_Config_Zone *cz);
static void _policy_zone_layout_conformant_single(E_Border *bd, E_Illume_Config_Zone *cz);
static void _policy_zone_layout_conformant_dual_top(E_Border *bd, E_Illume_Config_Zone *cz);
static void _policy_zone_layout_conformant_dual_custom(E_Border *bd, E_Illume_Config_Zone *cz);
static void _policy_zone_layout_conformant_dual_left(E_Border *bd, E_Illume_Config_Zone *cz);

/* local functions */
static void 
_policy_border_set_focus(E_Border *bd) 
{
   /* if focus is locked out then get out */
   if (bd->lock_focus_out) return;

   /* make sure the border can accept or take focus */
   if ((bd->client.icccm.accepts_focus) || (bd->client.icccm.take_focus)) 
     {
        /* check E's focus settings */
        if ((e_config->focus_setting == E_FOCUS_NEW_WINDOW) || 
            ((bd->parent) && 
             ((e_config->focus_setting == E_FOCUS_NEW_DIALOG) || 
              ((bd->parent->focused) && 
               (e_config->focus_setting == E_FOCUS_NEW_DIALOG_IF_OWNER_FOCUSED))))) 
          {
             /* if the border is iconified then uniconify */
             if (bd->iconic) 
               {
                  /* if the user is allowed to uniconify, then do it */
                  if (!bd->lock_user_iconify) e_border_uniconify(bd);
               }

             /* if we can raise the border do it */
             if (!bd->lock_user_stacking) e_border_raise(bd);

             /* focus the border */
             e_border_focus_set(bd, 1, 1);

             /* NB: since we skip needless border evals when container layout 
              * is called (to save cpu cycles), we need to 
              * signal this border that it's focused so that the edj gets 
              * updated. 
              * 
              * This is potentially useless as THIS policy 
              * makes all windows borderless anyway, but it's in here for 
              * completeness */
             e_border_focus_latest_set(bd);
             if (bd->bg_object) 
               edje_object_signal_emit(bd->bg_object, "e,state,focused", "e");
             if (bd->icon_object) 
               edje_object_signal_emit(bd->icon_object, "e,state,focused", "e");
             e_focus_event_focus_in(bd);
          }
     }
}

static void 
_policy_border_move(E_Border *bd, int x, int y) 
{
   /* NB: Qt uses a weird window type called 'VCLSalFrame' that needs to 
    * have bd->placed set else it doesn't position correctly...
    * this could be a result of E honoring the icccm request position, 
    * not sure */

   /* NB: Seems something similiar happens with elementary windows also 
    * so for now just set bd->placed on all windows until this 
    * gets investigated */
   bd->placed = 1;
   bd->x = x;
   bd->y = y;
   bd->changes.pos = 1;
   bd->changed = 1;
}

static void 
_policy_border_resize(E_Border *bd, int w, int h) 
{
   bd->w = w;
   bd->h = h;
   bd->client.w = (bd->w - (bd->client_inset.l + bd->client_inset.r));
   bd->client.h = (bd->h - (bd->client_inset.t + bd->client_inset.b));
   bd->changes.size = 1;
   bd->changed = 1;
}

static void 
_policy_border_hide_below(E_Border *bd) 
{
   int pos = 0, i;

//   printf("Hide Borders Below: %s %d %d\n", 
//          bd->client.icccm.class, bd->x, bd->y);

   /* determine layering position */
   if (bd->layer <= 0) pos = 0;
   else if ((bd->layer > 0) && (bd->layer <= 50)) pos = 1;
   else if ((bd->layer > 50) && (bd->layer <= 100)) pos = 2;
   else if ((bd->layer > 100) && (bd->layer <= 150)) pos = 3;
   else if ((bd->layer > 150) && (bd->layer <= 200)) pos = 4;
   else pos = 5;

   /* Find the windows below this one */
   for (i = pos; i >= 0; i--) 
     {
        Eina_List *l;
        E_Border *b;

        EINA_LIST_FOREACH(bd->zone->container->layers[i].clients, l, b) 
          {
             /* skip if it's not on this zone */
             if (b->zone != bd->zone) continue;

             /* skip if it's the same border */
             if (b == bd) continue;

             /* skip special borders */
             if (e_illume_border_is_indicator(b)) continue;
             if (e_illume_border_is_softkey(b)) continue;
             if (e_illume_border_is_keyboard(b)) continue;
             if (e_illume_border_is_quickpanel(b)) continue;

             if ((bd->fullscreen) || (bd->need_fullscreen)) 
               {
                  if (b->visible) e_border_hide(b, 2);
               }
             else 
               {
                  /* we need to check x/y position */
                  if ((b->x == bd->x) && (b->y == bd->y)) 
                    {
                       if (b->visible) e_border_hide(b, 2);
                    }
               }
          }
     }
}

static void 
_policy_border_show_below(E_Border *bd) 
{
   int pos = 0, i;

//   printf("Show Borders Below: %s %d %d\n", 
//          bd->client.icccm.name, bd->x, bd->y);

   /* determine layering position */
   if (bd->layer <= 0) pos = 0;
   else if ((bd->layer > 0) && (bd->layer <= 50)) pos = 1;
   else if ((bd->layer > 50) && (bd->layer <= 100)) pos = 2;
   else if ((bd->layer > 100) && (bd->layer <= 150)) pos = 3;
   else if ((bd->layer > 150) && (bd->layer <= 200)) pos = 4;
   else pos = 5;

   /* Find the windows below this one */
   for (i = pos; i >= 0; i--) 
     {
        Eina_List *l;
        E_Border *b;

        EINA_LIST_REVERSE_FOREACH(bd->zone->container->layers[i].clients, l, b) 
          {
             /* skip if it's not on this zone */
             if (b->zone != bd->zone) continue;

             /* skip if it's the same border */
             if (b == bd) continue;

             /* skip special borders */
             if (e_illume_border_is_indicator(b)) continue;
             if (e_illume_border_is_softkey(b)) continue;
             if (e_illume_border_is_keyboard(b)) continue;
             if (e_illume_border_is_quickpanel(b)) continue;

             if ((bd->fullscreen) || (bd->need_fullscreen)) 
               {
                  if (!b->visible) 
                    e_border_show(b);
                  b->changes.visible = 1;
                  b->changed = 1;
               }
             else 
               {
                  /* need to check x/y position */
                  if ((b->x == bd->x) && (b->y == bd->y)) 
                    {
                       if (!b->visible) 
                         e_border_show(b);
                       b->changes.visible = 1;
                       b->changed = 1;
                    }
               }
          }
     }
}

static void 
_policy_zone_layout_update(E_Zone *zone) 
{
   Eina_List *l;
   E_Border *bd;

   EINA_LIST_FOREACH(e_border_client_list(), l, bd) 
     {
        /* skip borders not on this zone */
        if (bd->zone != zone) continue;

        /* skip special windows */
        /* NB: We trigger a layout update on indicator windows so that 
         * quickpanel position gets updated so don't include indicators here */
//        if (e_illume_border_is_indicator(bd)) continue;
        if (e_illume_border_is_softkey(bd)) continue;
        if (e_illume_border_is_keyboard(bd)) continue;
        if (e_illume_border_is_quickpanel(bd)) continue;

        /* signal a changed pos here so layout gets updated */
        bd->changes.pos = 1;
        bd->changed = 1;
     }
}

static void 
_policy_zone_layout_indicator(E_Border *bd, E_Illume_Config_Zone *cz) 
{
   /* grab minimum indicator size */
   e_illume_border_min_get(bd, NULL, &cz->indicator.size);

   /* no point in doing anything here if indicator is hidden */
   if (!bd->visible) return;

   /* if we are dragging, then skip it for now */
   if (bd->client.illume.drag.drag) 
     {
        /* when dragging indicator, we need to trigger a layout update */
        _policy_zone_layout_update(bd->zone);
        return;
     }

//   printf("\tLayout Indicator: %d\n", bd->zone->id);

   /* lock indicator window from dragging if we need to */
   if ((cz->mode.dual == 1) && (cz->mode.side == 0)) 
     ecore_x_e_illume_drag_locked_set(bd->client.win, 0);
   else 
     ecore_x_e_illume_drag_locked_set(bd->client.win, 1);

   /* make sure it's the required width & height */
   if ((bd->w != bd->zone->w) || (bd->h != cz->indicator.size))
     _policy_border_resize(bd, bd->zone->w, cz->indicator.size);

   /* are we in single mode ? */
   if (!cz->mode.dual) 
     {
        /* move to 0, 0 (relative to zone) if needed */
        if ((bd->x != bd->zone->x) || (bd->y != bd->zone->y)) 
          {
             _policy_border_move(bd, bd->zone->x, bd->zone->y);
             ecore_x_e_illume_quickpanel_position_update_send(bd->client.win);
          }
     }
   else 
     {
        /* dual app mode top */
        if (cz->mode.side == 0) 
          {
             /* top mode...indicator is draggable so just set X */
             if (bd->x != bd->zone->x) 
               _policy_border_move(bd, bd->zone->x, bd->y);
          }
        else 
          {
             /* move to 0, 0 (relative to zone) if needed */
             if ((bd->x != bd->zone->x) || (bd->y != bd->zone->y)) 
               {
                  _policy_border_move(bd, bd->zone->x, bd->zone->y);
                  ecore_x_e_illume_quickpanel_position_update_send(bd->client.win);
               }
          }
     }

   /* set layer if needed */
   if (bd->layer != POL_INDICATOR_LAYER) 
     e_border_layer_set(bd, POL_INDICATOR_LAYER);
}

static void 
_policy_zone_layout_quickpanel(E_Border *bd) 
{
   int mh;

   /* grab minimum size */
   e_illume_border_min_get(bd, NULL, &mh);

   /* resize if needed */
   if ((bd->w != bd->zone->w) || (bd->h != mh))
     _policy_border_resize(bd, bd->zone->w, mh);

   /* set layer if needed */
   if (bd->layer != POL_QUICKPANEL_LAYER) 
     e_border_layer_set(bd, POL_QUICKPANEL_LAYER);
}

static void 
_policy_zone_layout_softkey(E_Border *bd, E_Illume_Config_Zone *cz) 
{
   int ny;

//   printf("\tLayout Softkey\n");

   /* grab minimum softkey size */
   e_illume_border_min_get(bd, NULL, &cz->softkey.size);

   /* no point in doing anything here if softkey is hidden */
   if (!bd->visible) return;

   /* if we are dragging, then skip it for now */
   /* NB: Disabled currently until we confirm that softkey should be draggable */
//   if (bd->client.illume.drag.drag) return;

   /* make sure it's the required width & height */
   if ((bd->w != bd->zone->w) || (bd->h != cz->softkey.size))
     _policy_border_resize(bd, bd->zone->w, cz->softkey.size);

   /* make sure it's in the correct position */
   ny = ((bd->zone->y + bd->zone->h) - cz->softkey.size);

   /* NB: not sure why yet, but on startup the border->y is reporting 
    * that it is already in this position...but it's actually not. 
    * So for now, just disable the ny check until this gets sorted out */
//   if ((bd->x != bd->zone->x) || (bd->y != ny))
   _policy_border_move(bd, bd->zone->x, ny);

   /* set layer if needed */
   if (bd->layer != POL_SOFTKEY_LAYER) 
     e_border_layer_set(bd, POL_SOFTKEY_LAYER);
}

static void 
_policy_zone_layout_keyboard(E_Border *bd, E_Illume_Config_Zone *cz) 
{
   int ny, layer;

//   printf("\tLayout Keyboard\n");

   /* grab minimum keyboard size */
   e_illume_border_min_get(bd, NULL, &cz->vkbd.size);

   /* no point in adjusting size or position if it's not visible */
   if (!bd->visible) return;

   /* make sure it's the required width & height */
   if ((bd->w != bd->zone->w) || (bd->h != cz->vkbd.size))
     _policy_border_resize(bd, bd->zone->w, cz->vkbd.size);

   /* make sure it's in the correct position */
   ny = ((bd->zone->y + bd->zone->h) - cz->vkbd.size);
   if ((bd->x != bd->zone->x) || (bd->y != ny)) 
     _policy_border_move(bd, bd->zone->x, ny);

   /* check layer according to fullscreen state */
   if ((bd->fullscreen) || (bd->need_fullscreen)) 
     {
        printf("Keyboard needs fullscreen\n");
        layer = POL_FULLSCREEN_LAYER;
     }
   else
     layer = POL_KEYBOARD_LAYER;

   /* set layer if needed */
   if (bd->layer != layer) 
     e_border_layer_set(bd, layer);
}

static void 
_policy_zone_layout_home_single(E_Border *bd, E_Illume_Config_Zone *cz) 
{
   int ny, nh;

//   printf("\tLayout Home Single: %s\n", bd->client.icccm.class);

   /* no point in adjusting size or position if it's not visible */
   if ((!bd->new_client) && (!bd->visible)) return;

   /* make sure it's the required width & height */
   nh = (bd->zone->h - cz->indicator.size - cz->softkey.size);
   if ((bd->w != bd->zone->w) || (bd->h != nh)) 
     _policy_border_resize(bd, bd->zone->w, nh);

   /* move to correct position (relative to zone) if needed */
   ny = (bd->zone->y + cz->indicator.size);
   if ((bd->x != bd->zone->x) || (bd->y != ny)) 
     _policy_border_move(bd, bd->zone->x, ny);

   /* set layer if needed */
   if (bd->layer != POL_HOME_LAYER) 
     e_border_layer_set(bd, POL_HOME_LAYER);
}

static void 
_policy_zone_layout_home_dual_top(E_Border *bd, E_Illume_Config_Zone *cz) 
{
   E_Border *home;
   int ny, nh;

   /* no point in adjusting size or position if it's not visible */
   if ((!bd->new_client) && (!bd->visible)) return;

   /* set some defaults */
   ny = (bd->zone->y + cz->indicator.size);
   nh = ((bd->zone->h - cz->indicator.size - cz->softkey.size) / 2);

   /* see if there are any other home windows */
   home = e_illume_border_home_get(bd->zone);
   if (home) 
     {
        if (bd != home) ny = (home->y + nh);
     }

   /* make sure it's the required width & height */
   if ((bd->w != bd->zone->w) || (bd->h != nh)) 
     _policy_border_resize(bd, bd->zone->w, nh);

   /* move to correct position (relative to zone) if needed */
   if ((bd->x != bd->zone->x) || (bd->y != ny)) 
     _policy_border_move(bd, bd->zone->x, ny);

   /* set layer if needed */
   if (bd->layer != POL_HOME_LAYER) 
     e_border_layer_set(bd, POL_HOME_LAYER);
}

static void 
_policy_zone_layout_home_dual_custom(E_Border *bd, E_Illume_Config_Zone *cz) 
{
   E_Border *home;
   int iy, ny, nh;

//   printf("\tLayout Home Dual Custom: %s\n", bd->client.icccm.class);

   /* no point in adjusting size or position if it's not visible */
   if ((!bd->new_client) && (!bd->visible)) return;

   /* grab indicator position */
   e_illume_border_indicator_pos_get(bd->zone, NULL, &iy);

   /* set some defaults */
   ny = bd->zone->y;
   nh = iy;

   /* see if there are any other home windows */
   home = e_illume_border_home_get(bd->zone);
   if (home) 
     {
        if (bd != home) 
          {
             ny = (iy + cz->indicator.size);
             nh = ((bd->zone->y + bd->zone->h) - ny - cz->softkey.size);
          }
     }

   /* make sure it's the required width & height */
   if ((bd->w != bd->zone->w) || (bd->h != nh)) 
     _policy_border_resize(bd, bd->zone->w, nh);

   /* move to correct position (relative to zone) if needed */
   if ((bd->x != bd->zone->x) || (bd->y != ny))
     _policy_border_move(bd, bd->zone->x, ny);

   /* set layer if needed */
   if (bd->layer != POL_HOME_LAYER) 
     e_border_layer_set(bd, POL_HOME_LAYER);
}

static void 
_policy_zone_layout_home_dual_left(E_Border *bd, E_Illume_Config_Zone *cz) 
{
   E_Border *home;
   int nx, nw, nh;

//   printf("\tLayout Home Dual Left: %s\n", bd->client.icccm.class);

   /* no point in adjusting size or position if it's not visible */
   if ((!bd->new_client) && (!bd->visible)) return;

   nh = (bd->zone->h - cz->indicator.size - cz->softkey.size);

   /* set some defaults */
   nx = bd->zone->x;
   nw = (bd->zone->w / 2);

   /* see if there are any other home windows */
   home = e_illume_border_home_get(bd->zone);
   if (home) 
     {
        if (bd != home) nx = (home->x + nw);
     }

   /* make sure it's the required width & height */
   if ((bd->w != nw) || (bd->h != nh))
     _policy_border_resize(bd, nw, nh);

   /* move to correct position (relative to zone) if needed */
   if ((bd->x != nx) || (bd->y != (bd->zone->y + cz->indicator.size)))
     _policy_border_move(bd, nx, (bd->zone->y + cz->indicator.size));

   /* set layer if needed */
   if (bd->layer != POL_HOME_LAYER) 
     e_border_layer_set(bd, POL_HOME_LAYER);
}

static void 
_policy_zone_layout_fullscreen(E_Border *bd) 
{
   int kh, ny, nh;

//   printf("\tLayout Fullscreen: %s\n", bd->client.icccm.name);

   /* grab keyboard safe region */
   e_illume_keyboard_safe_app_region_get(bd->zone, NULL, NULL, NULL, &kh);

   /* make sure it's the required width & height */
   if ((bd->w != bd->zone->w) || (bd->h != kh))
     _policy_border_resize(bd, bd->zone->w, kh);

   /* set layer if needed */
   if (bd->layer != POL_FULLSCREEN_LAYER) 
     e_border_layer_set(bd, POL_FULLSCREEN_LAYER);

   /* hide any apps/windows below this one */
   _policy_border_hide_below(bd);
}

static void 
_policy_zone_layout_app_single(E_Border *bd, E_Illume_Config_Zone *cz) 
{
   E_Border *home;
   int ky, kh, ny, nh;

//   printf("\tLayout App Single: %s\n", bd->client.icccm.name);

   if ((!bd->new_client) && (!bd->visible)) return;

   /* grab keyboard safe region */
   e_illume_keyboard_safe_app_region_get(bd->zone, NULL, &ky, NULL, &kh);

   /* make sure it's the required width & height */
   if (kh >= bd->zone->h)
     nh = (kh - cz->indicator.size - cz->softkey.size);
   else
     nh = (kh - cz->indicator.size);

   /* resize if needed */
   if ((bd->w != bd->zone->w) || (bd->h != nh))
     _policy_border_resize(bd, bd->zone->w, nh);

   /* move to correct position (relative to zone) if needed */
   ny = (bd->zone->y + cz->indicator.size);
   if ((bd->x != bd->zone->x) || (bd->y != ny)) 
     _policy_border_move(bd, bd->zone->x, ny);

   /* set layer if needed */
   if (bd->layer != POL_APP_LAYER) 
     e_border_layer_set(bd, POL_APP_LAYER);

   /* hide any apps/windows below this one */
   _policy_border_hide_below(bd);
}

static void 
_policy_zone_layout_app_dual_top(E_Border *bd, E_Illume_Config_Zone *cz) 
{
   E_Border *b;
   int kh, ny, nh;

//   printf("\tLayout App Dual Top: %s\n", bd->client.icccm.name);

   if ((!bd->new_client) && (!bd->visible)) return;

   /* set a default Y position */
   ny = (bd->zone->y + cz->indicator.size);

   if ((bd->focused) && 
       (bd->client.vkbd.state > ECORE_X_VIRTUAL_KEYBOARD_STATE_OFF)) 
     {
        /* grab keyboard safe region */
        e_illume_keyboard_safe_app_region_get(bd->zone, NULL, NULL, NULL, &kh);

        nh = (kh - cz->indicator.size);
     }
   else 
     {
        /* make sure it's the required width & height */
        nh = ((bd->zone->h - cz->indicator.size - cz->softkey.size) / 2);
     }

   /* see if there is a border already there. if so, check placement based on 
    * virtual keyboard usage */
   b = e_illume_border_at_xy_get(bd->zone, bd->zone->x, ny);
   if (b) 
     {
        if (b != bd) 
          {
             /* does this border need keyboard ? */
             if ((bd->focused) && 
                 (bd->client.vkbd.state > ECORE_X_VIRTUAL_KEYBOARD_STATE_OFF)) 
               {
                  int h;

                  /* move existing border to bottom if needed */
                  h = ((bd->zone->h - cz->indicator.size - cz->softkey.size) / 2);
                  if ((b->x != b->zone->x) || (b->y != (ny + h)))
                    _policy_border_move(b, b->zone->x, (ny + h));

                  /* resize existing border if needed */
                  if ((b->w != b->zone->w) || (b->h != h))
                    _policy_border_resize(b, b->zone->w, h);
               }
             else
               ny = b->y + nh;
          }
     }

   /* resize if needed */
   if ((bd->w != bd->zone->w) || (bd->h != nh))
     _policy_border_resize(bd, bd->zone->w, nh);

   /* move to correct position (relative to zone) if needed */
   if ((bd->x != bd->zone->x) || (bd->y != ny)) 
     _policy_border_move(bd, bd->zone->x, ny);

   /* set layer if needed */
   if (bd->layer != POL_APP_LAYER) 
     e_border_layer_set(bd, POL_APP_LAYER);

   /* hide any apps/windows below this one */
   _policy_border_hide_below(bd);
}

static void 
_policy_zone_layout_app_dual_custom(E_Border *bd, E_Illume_Config_Zone *cz) 
{
   E_Border *app;
   int iy, ny, nh;

//   printf("\tLayout App Dual Custom: %s\n", bd->client.icccm.class);

   if ((!bd->new_client) && (!bd->visible)) return;

   /* grab indicator position */
   e_illume_border_indicator_pos_get(bd->zone, NULL, &iy);

   /* set a default position */
   ny = bd->zone->y;
   nh = iy;

   app = e_illume_border_at_xy_get(bd->zone, bd->zone->x, bd->zone->y);
   if (app) 
     {
        if (bd != app) 
          {
             ny = (iy + cz->indicator.size);
             nh = ((bd->zone->y + bd->zone->h) - ny - cz->softkey.size);
          }
     }

   /* make sure it's the required width & height */
   if ((bd->w != bd->zone->w) || (bd->h != nh)) 
     _policy_border_resize(bd, bd->zone->w, nh);

   /* move to correct position (relative to zone) if needed */
   if ((bd->x != bd->zone->x) || (bd->y != ny))
     _policy_border_move(bd, bd->zone->x, ny);

   /* set layer if needed */
   if (bd->layer != POL_APP_LAYER) 
     e_border_layer_set(bd, POL_APP_LAYER);

   /* hide any apps/windows below this one */
   _policy_border_hide_below(bd);
}

static void 
_policy_zone_layout_app_dual_left(E_Border *bd, E_Illume_Config_Zone *cz) 
{
   E_Border *b;
   int ky, kh, nx, nw;

//   printf("\tLayout App Dual Left: %s\n", bd->client.icccm.name);

   if ((!bd->new_client) && (!bd->visible)) return;

   /* grab keyboard safe region */
   e_illume_keyboard_safe_app_region_get(bd->zone, NULL, &ky, NULL, &kh);

   if (kh >= bd->zone->h)
     kh = (kh - cz->indicator.size - cz->softkey.size);
   else
     kh = (kh - cz->indicator.size);

   /* set some defaults */
   nx = bd->zone->x;
   nw = (bd->zone->w / 2);

   /* see if there is a border already there. if so, place at right */
   b = e_illume_border_at_xy_get(bd->zone, nx, (ky + cz->indicator.size));
   if (b) 
     {
        if (bd != b) nx = b->x + nw;
     }

   /* resize if needed */
   if ((bd->w != nw) || (bd->h != kh))
     _policy_border_resize(bd, nw, kh);

   /* move to correct position (relative to zone) if needed */
   if ((bd->x != nx) || (bd->y != (ky + cz->indicator.size)))
     _policy_border_move(bd, nx, (ky + cz->indicator.size));

   /* set layer if needed */
   if (bd->layer != POL_APP_LAYER) 
     e_border_layer_set(bd, POL_APP_LAYER);

   /* hide any apps/windows below this one */
   _policy_border_hide_below(bd);
}

static void 
_policy_zone_layout_dialog(E_Border *bd, E_Illume_Config_Zone *cz) 
{
   E_Border *parent;
   int mw, mh, nx, ny;

//   printf("\tLayout Dialog: %s\n", bd->client.icccm.name);

   /* NB: This policy ignores any ICCCM requested positions and centers the 
    * dialog on it's parent (if it exists) or on the zone */

   /* grab minimum size */
   e_illume_border_min_get(bd, &mw, &mh);

   /* make sure it fits in this zone */
   if (mw > bd->zone->w) mw = bd->zone->w;
   if (mh > bd->zone->h) mh = bd->zone->h;

   /* try to get this dialog's parent if it exists */
   parent = e_illume_border_parent_get(bd);

   /* if we have no parent, or we are in dual mode, then center on zone */
   /* NB: we check dual mode because if we are in dual mode, dialogs tend to 
    * be too small to be useful when positioned on the parent, so center 
    * on zone. We could check their size first here tho */
   if ((!parent) || (cz->mode.dual == 1)) 
     {
        /* no parent or dual mode, center on screen */
        nx = (bd->zone->x + ((bd->zone->w - mw) / 2));
        ny = (bd->zone->y + ((bd->zone->h - mh) / 2));
     }
   else 
     {
        /* NB: there is an assumption here that the parent has already been 
         * layed out on screen. This could be bad. Needs Testing */

        /* make sure we are not larger than the parent window */
        if (mw > parent->w) mw = parent->w;
        if (mh > parent->h) mh = parent->h;

        /* center on parent */
        nx = (parent->x + ((parent->w - mw) / 2));
        ny = (parent->y + ((parent->h - mh) / 2));
     }

   /* make sure it's the required width & height */
   if ((bd->w != mw) || (bd->h != mh))
     _policy_border_resize(bd, mw, mh);

   /* make sure it's in the correct position */
   if ((bd->x != nx) || (bd->y != ny)) 
     _policy_border_move(bd, nx, ny);

   /* set layer if needed */
   if (bd->layer != POL_DIALOG_LAYER) 
     e_border_layer_set(bd, POL_DIALOG_LAYER);
}

static void 
_policy_zone_layout_splash(E_Border *bd, E_Illume_Config_Zone *cz) 
{
   E_Border *parent;
   int mw, mh, nx, ny;

   /* NB: This code is almost exactly the same as the dialog layout code 
    * (_policy_zone_layout_dialog) except for setting a different layer */

//   printf("\tLayout Splash: %s\n", bd->client.icccm.name);

   /* NB: This policy ignores any ICCCM requested positions and centers the 
    * splash screen on it's parent (if it exists) or on the zone */

   /* grab minimum size */
   e_illume_border_min_get(bd, &mw, &mh);

   /* make sure it fits in this zone */
   if (mw > bd->zone->w) mw = bd->zone->w;
   if (mh > bd->zone->h) mh = bd->zone->h;

   /* if we have no parent, or we are in dual mode, then center on zone */
   /* NB: we check dual mode because if we are in dual mode, dialogs tend to 
    * be too small to be useful when positioned on the parent, 
    * so center on zone instead */
   if ((!parent) || (cz->mode.dual == 1)) 
     {
        /* no parent or in dual mode, center on screen */
        nx = (bd->zone->x + ((bd->zone->w - mw) / 2));
        ny = (bd->zone->y + ((bd->zone->h - mh) / 2));
     }
   else 
     {
        /* NB: there is an assumption here that the parent has already been 
         * layed out on screen. This could be bad. Needs Testing */

        /* make sure we are not larger than the parent window */
        if (mw > parent->w) mw = parent->w;
        if (mh > parent->h) mh = parent->h;

        /* center on parent */
        nx = (parent->x + ((parent->w - mw) / 2));
        ny = (parent->y + ((parent->h - mh) / 2));
     }

   /* make sure it's the required width & height */
   if ((bd->w != mw) || (bd->h != mh))
     _policy_border_resize(bd, mw, mh);

   /* make sure it's in the correct position */
   if ((bd->x != nx) || (bd->y != ny)) 
     _policy_border_move(bd, nx, ny);

   /* set layer if needed */
   if (bd->layer != POL_SPLASH_LAYER) 
     e_border_layer_set(bd, POL_SPLASH_LAYER);
}

static void 
_policy_zone_layout_conformant_single(E_Border *bd, E_Illume_Config_Zone *cz) 
{
   if ((!bd->new_client) && (!bd->visible)) return;

   /* make sure it's the required width & height */
   if ((bd->w != bd->zone->w) || (bd->h != bd->zone->h))
     _policy_border_resize(bd, bd->zone->w, bd->zone->h);

   /* move to correct position (relative to zone) if needed */
   if ((bd->x != bd->zone->x) || (bd->y != bd->zone->y))
     _policy_border_move(bd, bd->zone->x, bd->zone->y);

   /* set layer if needed */
   if (bd->layer != POL_CONFORMANT_LAYER) 
     e_border_layer_set(bd, POL_CONFORMANT_LAYER);

   /* hide any apps/windows below this one */
   _policy_border_hide_below(bd);
}

static void 
_policy_zone_layout_conformant_dual_top(E_Border *bd, E_Illume_Config_Zone *cz) 
{
   int nh, ny;

   /* according to the docs I have, conformant windows are always on the 
    * bottom in dual-top mode */

   if ((!bd->new_client) && (!bd->visible)) return;

   /* set some defaults */
   nh = ((bd->zone->h - cz->indicator.size - cz->softkey.size) / 2);
   ny = (bd->zone->y + cz->indicator.size) + nh;
   nh += cz->softkey.size;

   /* make sure it's the required width & height */
   if ((bd->w != bd->zone->w) || (bd->h != nh))
     _policy_border_resize(bd, bd->zone->w, nh);

   /* move to correct position (relative to zone) if needed */
   if ((bd->x != bd->zone->x) || (bd->y != ny))
     _policy_border_move(bd, bd->zone->x, ny);

   /* set layer if needed */
   if (bd->layer != POL_CONFORMANT_LAYER) 
     e_border_layer_set(bd, POL_CONFORMANT_LAYER);

   /* hide any apps/windows below this one */
   _policy_border_hide_below(bd);
}

static void 
_policy_zone_layout_conformant_dual_custom(E_Border *bd, E_Illume_Config_Zone *cz) 
{
   int iy, nh;

//   printf("\tLayout Conformant Dual Custom: %s\n", bd->client.icccm.class);

   if ((!bd->new_client) && (!bd->visible)) return;

   /* grab indicator position */
   e_illume_border_indicator_pos_get(bd->zone, NULL, &iy);

   /* set some defaults */
   nh = ((bd->zone->y + bd->zone->h) - iy);

   /* make sure it's the required width & height */
   if ((bd->w != bd->zone->w) || (bd->h != nh)) 
     _policy_border_resize(bd, bd->zone->w, nh);

   /* move to correct position (relative to zone) if needed */
   if ((bd->x != bd->zone->x) || (bd->y != iy))
     _policy_border_move(bd, bd->zone->x, iy);

   /* set layer if needed */
   if (bd->layer != POL_CONFORMANT_LAYER) 
     e_border_layer_set(bd, POL_CONFORMANT_LAYER);

   /* hide any apps/windows below this one */
   _policy_border_hide_below(bd);
}

static void 
_policy_zone_layout_conformant_dual_left(E_Border *bd, E_Illume_Config_Zone *cz) 
{
   int nw, nx;

   /* according to the docs I have, conformant windows are always on the 
    * left in dual-left mode */

   if ((!bd->new_client) && (!bd->visible)) return;

   /* set some defaults */
   nw = (bd->zone->w / 2);
   nx = (bd->zone->x);

   /* make sure it's the required width & height */
   if ((bd->w != nw) || (bd->h != bd->zone->h))
     _policy_border_resize(bd, nw, bd->zone->h);

   /* move to correct position (relative to zone) if needed */
   if ((bd->x != nx) || (bd->y != bd->zone->y))
     _policy_border_move(bd, nx, bd->zone->y);

   /* set layer if needed */
   if (bd->layer != POL_CONFORMANT_LAYER) 
     e_border_layer_set(bd, POL_CONFORMANT_LAYER);

   /* hide any apps/windows below this one */
   _policy_border_hide_below(bd);
}


/* policy functions */
void 
_policy_border_add(E_Border *bd) 
{
//   printf("Border added: %s\n", bd->client.icccm.class);

   /* NB: this call sets an atom on the window that specifices the zone.
    * the logic here is that any new windows created can access the zone 
    * window by a 'get' call. This is useful for elementary apps as they 
    * normally would not have access to the zone window. Typical use case 
    * is for indicator & softkey windows so that they can send messages 
    * that apply to their respective zone only. Example: softkey sends close 
    * messages (or back messages to cycle focus) that should pertain just 
    * to it's current zone */
   ecore_x_e_illume_zone_set(bd->client.win, bd->zone->black_win);

   /* ignore stolen borders. These are typically quickpanel or keyboards */
   if (bd->stolen) return;

   /* if this is a fullscreen window, than we need to hide indicator window */
   /* NB: we could use the e_illume_border_is_fullscreen function here 
    * but we save ourselves a function call this way */
   if ((bd->fullscreen) || (bd->need_fullscreen)) 
     {
        E_Border *ind;

        /* try to get the Indicator on this zone */
        if (ind = e_illume_border_indicator_get(bd->zone)) 
          {
             /* we have the indicator, hide it if needed */
             if (ind->visible) e_border_hide(ind, 2);
          }
     }

   /* set focus on new border if we can */
   _policy_border_set_focus(bd);
}

void 
_policy_border_del(E_Border *bd) 
{
//   printf("Border deleted: %s\n", bd->client.icccm.class);

   /* if this is a fullscreen window, than we need to show indicator window */
   /* NB: we could use the e_illume_border_is_fullscreen function here 
    * but we save ourselves a function call this way */
   if ((bd->fullscreen) || (bd->need_fullscreen)) 
     {
        E_Border *ind;

        /* try to get the Indicator on this zone */
        if (ind = e_illume_border_indicator_get(bd->zone)) 
          {
             /* we have the indicator, show it if needed */
             if (!ind->visible) e_border_show(ind);
          }
     }

   /* show borders below this one */
   _policy_border_show_below(bd);
}

void 
_policy_border_focus_in(E_Border *bd) 
{
//   printf("Border focus in: %s\n", bd->client.icccm.name);
}

void 
_policy_border_focus_out(E_Border *bd) 
{
//   printf("Border focus out: %s\n", bd->client.icccm.name);

   /* NB: if we got this focus_out event on a deleted border, we check if 
    * it is a transient (child) of another window. If it is, then we 
    * transfer focus back to the parent window */
   if (e_object_is_del(E_OBJECT(bd))) 
     {
        if (e_illume_border_is_dialog(bd)) 
          {
             E_Border *parent;

             parent = e_illume_border_parent_get(bd);
             if (parent) _policy_border_set_focus(parent);
          }
     }
}

void 
_policy_border_activate(E_Border *bd) 
{
   E_Border *sft;

//   printf("Border Activate: %s\n", bd->client.icccm.name);

   /* NB: stolen borders may or may not need focus call...have to test */
//   if (bd->stolen) return;

   /* conformant windows hide the softkey */
   sft = e_illume_border_softkey_get(bd->zone);
   if (sft) 
     {
        if (e_illume_border_is_conformant(bd)) 
          {
             if (sft->visible) e_border_hide(sft, 2);
          }
        else 
          {
             if (!sft->visible) e_border_show(sft);
          }
     }

   /* set focus on new border is we can */
   _policy_border_set_focus(bd);
}

void 
_policy_border_post_fetch(E_Border *bd) 
{
//   printf("Border post fetch\n");

   /* NB: for this policy we disable all remembers set on a border */
   if (bd->remember) e_remember_del(bd->remember);
   bd->remember = NULL;

   /* set this border to borderless */
#ifdef DIALOG_USES_PIXEL_BORDER
   if ((e_illume_border_is_dialog(bd)) && 
       (e_illume_border_parent_get(bd)))
     eina_stringshare_replace(&bd->bordername, "pixel");
   else
     bd->borderless = 1;
#else
   bd->borderless = 1;
#endif

   /* tell E the border has changed */
   bd->client.border.changed = 1;
}

void 
_policy_border_post_assign(E_Border *bd) 
{
//   printf("Border post assign\n");

   bd->internal_no_remember = 1;

   /* do not allow client to change these properties */
   bd->lock_client_size = 1;
   bd->lock_client_shade = 1;
   bd->lock_client_maximize = 1;
   bd->lock_client_location = 1;
   bd->lock_client_stacking = 1;

   /* do not allow the user to change these properties */
   bd->lock_user_location = 1;
   bd->lock_user_size = 1;
   bd->lock_user_shade = 1;

   /* clear any centered states */
   /* NB: this is mainly needed for E's main config dialog */
   bd->client.e.state.centered = 0;

   /* lock the border type so user/client cannot change */
   bd->lock_border = 1;
}

void 
_policy_zone_layout(E_Zone *zone) 
{
   E_Illume_Config_Zone *cz;
   Eina_List *l;
   E_Border *bd;

//   printf("Zone Layout: %d\n", zone->id);

   /* get the config for this zone */
   cz = e_illume_zone_config_get(zone->id);

   /* loop through border list and update layout */
   EINA_LIST_FOREACH(e_border_client_list(), l, bd) 
     {
        /* skip borders not on this zone */
        if (bd->zone != zone) continue;

        /* skip borders that are being deleted */
        if (e_object_is_del(E_OBJECT(bd))) continue;

        /* only update layout for this border if it really needs it */
        if ((!bd->new_client) && (!bd->changes.pos) && (!bd->changes.size) && 
            (!bd->changes.visible) && (!bd->pending_move_resize)) continue;

        /* are we laying out an indicator ? */
        if (e_illume_border_is_indicator(bd)) 
          _policy_zone_layout_indicator(bd, cz);

        /* are we layout out a quickpanel ? */
        else if (e_illume_border_is_quickpanel(bd)) 
          _policy_zone_layout_quickpanel(bd);

        /* are we laying out a softkey ? */
        else if (e_illume_border_is_softkey(bd)) 
          _policy_zone_layout_softkey(bd, cz);

        /* are we laying out a keyboard ? */
        else if (e_illume_border_is_keyboard(bd)) 
          _policy_zone_layout_keyboard(bd, cz);

        /* are we layout out a home window ? */
        else if (e_illume_border_is_home(bd)) 
          {
             if (!cz->mode.dual)
               _policy_zone_layout_home_single(bd, cz);
             else 
               {
                  /* we are in dual-mode, check orientation */
                  if (cz->mode.side == 0) 
                    {
                       int ty;

                       e_illume_border_indicator_pos_get(bd->zone, NULL, &ty);
                       if (ty <= bd->zone->y)
                         _policy_zone_layout_home_dual_top(bd, cz);
                       else
                         _policy_zone_layout_home_dual_custom(bd, cz);
                    }
                  else 
                    _policy_zone_layout_home_dual_left(bd, cz);
               }
          }

        /* are we laying out a fullscreen window ? */
        /* NB: we could use the e_illume_border_is_fullscreen function here 
         * but we save ourselves a function call this way. */
        else if ((bd->fullscreen) || (bd->need_fullscreen)) 
          _policy_zone_layout_fullscreen(bd);

        /* are we laying out a splash screen ? */
        /* NB: we check splash before dialog so if a splash screen does not 
         * register as a splash, than the dialog trap should catch it */
        else if (e_illume_border_is_splash(bd)) 
          _policy_zone_layout_splash(bd, cz);

        /* are we laying out a dialog ? */
        else if (e_illume_border_is_dialog(bd)) 
          _policy_zone_layout_dialog(bd, cz);

        /* are we layout out a conformant window ? */
        else if (e_illume_border_is_conformant(bd)) 
          {
             if (!cz->mode.dual)
               _policy_zone_layout_conformant_single(bd, cz);
             else 
               {
                  /* we are in dual-mode, check orientation */
                  if (cz->mode.side == 0) 
                    {
                       int ty;

                       e_illume_border_indicator_pos_get(bd->zone, NULL, &ty);
                       if (ty <= bd->zone->y)
                         _policy_zone_layout_conformant_dual_top(bd, cz);
                       else
                         _policy_zone_layout_conformant_dual_custom(bd, cz);
                    }
                  else 
                    _policy_zone_layout_conformant_dual_left(bd, cz);
               }
          }

        /* must be an app */
        else 
          {
             /* are we in single mode ? */
             if (!cz->mode.dual) 
               _policy_zone_layout_app_single(bd, cz);
             else 
               {
                  /* we are in dual-mode, check orientation */
                  if (cz->mode.side == 0) 
                    {
                       int ty;

                       /* grab the indicator position so we can tell if it 
                        * is in a custom position or not (user dragged it) */
                       e_illume_border_indicator_pos_get(bd->zone, NULL, &ty);
                       if (ty <= bd->zone->y)
                         _policy_zone_layout_app_dual_top(bd, cz);
                       else
                         _policy_zone_layout_app_dual_custom(bd, cz);
                    }
                  else 
                    _policy_zone_layout_app_dual_left(bd, cz);
               }
          }

     }
}

void 
_policy_zone_move_resize(E_Zone *zone) 
{
   printf("Zone move resize\n");

   /* zone size or position changed, tell layout to update */
   _policy_zone_layout(zone);
}

void 
_policy_zone_mode_change(E_Zone *zone, Ecore_X_Atom mode) 
{
   E_Illume_Config_Zone *cz;
   E_Border *bd;
   int count;

//   printf("Zone mode change\n");

   /* get the config for this zone */
   cz = e_illume_zone_config_get(zone->id);

   /* update config with new mode */
   if (mode == ECORE_X_ATOM_E_ILLUME_MODE_SINGLE) 
     cz->mode.dual = 0;
   else 
     {
        cz->mode.dual = 1;
        if (mode == ECORE_X_ATOM_E_ILLUME_MODE_DUAL_TOP) 
          cz->mode.side = 0;
        else if (mode == ECORE_X_ATOM_E_ILLUME_MODE_DUAL_LEFT) 
          cz->mode.side = 1;
     }
   e_config_save_queue();

   /* lock indicator window from dragging if we need to */
   bd = e_illume_border_indicator_get(zone);
   if (bd) 
     {
        /* only dual-top mode can drag */
        if ((cz->mode.dual == 1) && (cz->mode.side == 0))
          {
             /* only set locked if we need to */
             if (bd->client.illume.drag.locked != 0) 
               ecore_x_e_illume_drag_locked_set(bd->client.win, 0);
          }
        else
          {
             /* only set locked if we need to */
             if (bd->client.illume.drag.locked != 1) 
             ecore_x_e_illume_drag_locked_set(bd->client.win, 1);
          }
     }

   count = eina_list_count(e_illume_border_home_borders_get(zone));

   /* create a new home window (if needed) for dual mode */
   if (cz->mode.dual == 1) 
     {
        if (count < 2)
          ecore_x_e_illume_home_new_send(zone->black_win);
     }
   else if (cz->mode.dual == 0) 
     {
        /* if we went to single mode, delete any extra home windows */
        if (count >= 2) 
          {
             E_Border *home;

             /* try to get a home window on this zone and remove it */
             if (home = e_illume_border_home_get(zone))
               ecore_x_e_illume_home_del_send(home->client.win);
          }
     }

   /* Need to trigger a layout update here */
   _policy_zone_layout_update(zone);
}

void 
_policy_zone_close(E_Zone *zone) 
{
   E_Border *bd;

//   printf("Zone close\n");

   /* make sure we have a focused border */
   if (!(bd = e_border_focused_get())) return;

   /* make sure focused border is on this zone */
   if (bd->zone != zone) return;

   /* close this border */
   e_border_act_close_begin(bd);

   /* revert focus to previous border (if possible) */
   _policy_focus_back(zone);
}

void 
_policy_drag_start(E_Border *bd) 
{
//   printf("Drag start\n");

   /* ignore stolen borders */
   if (bd->stolen) return;

   /* set property on this border to say we are dragging */
   ecore_x_e_illume_drag_set(bd->client.win, 1);

   /* set property on zone window that a drag is happening */
   ecore_x_e_illume_drag_set(bd->zone->black_win, 1);
}

void 
_policy_drag_end(E_Border *bd) 
{
//   printf("Drag end\n");

   /* ignore stolen borders */
   if (bd->stolen) return;

   /* set property on this border to say we are done dragging */
   ecore_x_e_illume_drag_set(bd->client.win, 0);

   /* set property on zone window that a drag is finished */
   ecore_x_e_illume_drag_set(bd->zone->black_win, 0);
}

void 
_policy_focus_back(E_Zone *zone) 
{
   Eina_List *l, *fl = NULL;
   E_Border *bd;

//   printf("Focus back\n");

   /* loop the focus stack and check for borders on this zone */
   EINA_LIST_FOREACH(e_border_focus_stack_get(), l, bd) 
     {
        /* not on this zone, skip */
        if (bd->zone != zone) continue;

        /* on this zone but being deleted, skip it */
        if (e_object_is_del(E_OBJECT(bd))) continue;

        /* doesn't accept or take focus, skip it */
        if ((!bd->client.icccm.accepts_focus) && 
            (!bd->client.icccm.take_focus)) continue;

        /* doesn't show in taskbar, skip it */
        if (bd->client.netwm.state.skip_taskbar) continue;

        /* add it to our list */
        fl = eina_list_append(fl, bd);
     }

   /* if there are none on this zone then get out */
   if (!fl) return;

   /* if there is only one border then focus it and get out */
   if (eina_list_count(fl) < 2) 
     {
        E_Border *fbd;

        if (fbd = fl->data) _policy_border_set_focus(fbd);

        /* cleanup our list variable */
        fl = eina_list_free(fl);

        return;
     }

   /* loop focused borders on this zone */
   /* NB: This is done in reverse order so we get the most recently focused 
    * border first....saves time and iterations */
   EINA_LIST_REVERSE_FOREACH(fl, l, bd) 
     {
        /* is this the currently focused border ? */
        if (bd == e_border_focused_get()) 
          {
             E_Border *fbd;

             /* see if we have another border previous to this one */
             if (!(fbd = fl->next->data)) continue;

             /* focus the previous border */
             _policy_border_set_focus(fbd);

             /* we're done, get out */
             break;
          }
     }

   /* cleanup our list variable */
   fl = eina_list_free(fl);
}

void 
_policy_focus_forward(E_Zone *zone) 
{
   printf("Focus forward\n");
}

void 
_policy_focus_home(E_Zone *zone) 
{
   E_Border *bd;

//   printf("Focus home\n");
   if (!(bd = e_illume_border_home_get(zone))) return;

   if (!bd->visible) e_border_show(bd);

   /* no point in calling set_focus here as home windows do not accept it 
    * anyway, so just raise */
   e_border_raise(bd);
}

void 
_policy_property_change(Ecore_X_Event_Window_Property *event) 
{
//   printf("Property Change\n");

   /* we are interested in state changes here */
   if (event->atom == ECORE_X_ATOM_NET_WM_STATE) 
     {
        E_Border *bd, *ind;

        if (!(bd = e_border_find_by_client_window(event->win))) return;

        /* not interested in stolen or invisible borders */
        if ((bd->stolen) || (!bd->visible)) return;

        /* make sure the border has a name or class */
        /* NB: this check is here because some E borders get State Changes 
         * but do not have a name/class associated with them. Not entirely sure 
         * which ones they are, but I would guess Managers, Containers, or Zones.
         * At any rate, we're not interested in those types of borders */
        if ((!bd->client.icccm.name) || (!bd->client.icccm.class)) return;

        /* NB: If we have reached this point, then it should be a fullscreen 
         * border that has toggled fullscreen on/off */

        /* try to get the Indicator on this zone */
        if (!(ind = e_illume_border_indicator_get(bd->zone))) return;

        /* if we are fullscreen, hide the indicator...else we show it */
        /* NB: we could use the e_illume_border_is_fullscreen function here 
         * but we save ourselves a function call this way */
        if ((bd->fullscreen) || (bd->need_fullscreen)) 
          {
             if (ind->visible) e_border_hide(ind, 2);
          }
        else 
          {
             if (!ind->visible) e_border_show(ind);
          }
     }
   else if (event->atom == ECORE_X_ATOM_E_ILLUME_INDICATOR_GEOMETRY) 
     {
        Eina_List *l;
        E_Zone *zone;
        E_Border *bd;
        int x, y, w, h;

        /* make sure this property changed on a zone */
        if (!(zone = e_util_zone_window_find(event->win))) return;

        /* get the geometry. This is X round-trip :( */
        ecore_x_e_illume_indicator_geometry_get(zone->black_win, &x, &y, &w, &h);

        /* look for conformant borders */
        EINA_LIST_FOREACH(e_border_client_list(), l, bd) 
          {
             if (bd->zone != zone) continue;
             if (!e_illume_border_is_conformant(bd)) continue;
             /* set indicator geometry on conformant window */
             /* NB: This is needed so that conformant apps get told about 
              * the indicator size/position...else they have no way of 
              * knowing that the geometry has been updated */
             ecore_x_e_illume_indicator_geometry_set(bd->client.win, x, y, w, h);
          }
     }
   else if (event->atom == ECORE_X_ATOM_E_ILLUME_SOFTKEY_GEOMETRY) 
     {
        Eina_List *l;
        E_Zone *zone;
        E_Border *bd;
        int x, y, w, h;

        /* make sure this property changed on a zone */
        if (!(zone = e_util_zone_window_find(event->win))) return;

        /* get the geometry. This is X round-trip :( */
        ecore_x_e_illume_softkey_geometry_get(zone->black_win, &x, &y, &w, &h);

        /* look for conformant borders */
        EINA_LIST_FOREACH(e_border_client_list(), l, bd) 
          {
             if (bd->zone != zone) continue;
             if (!e_illume_border_is_conformant(bd)) continue;
             /* set softkey geometry on conformant window */
             /* NB: This is needed so that conformant apps get told about 
              * the softkey size/position...else they have no way of 
              * knowing that the geometry has been updated */
             ecore_x_e_illume_softkey_geometry_set(bd->client.win, x, y, w, h);
          }
     }
   else if (!(strcmp(ecore_x_atom_name_get(event->atom), "ENLIGHTENMENT_SCALE"))) 
     {
        Eina_List *ml;
        E_Manager *man;

        EINA_LIST_FOREACH(e_manager_list(), ml, man) 
          {
             Eina_List *cl;
             E_Container *con;

             if (event->win != man->root) continue;
             EINA_LIST_FOREACH(man->containers, cl, con) 
               {
                  Eina_List *zl;
                  E_Zone *zone;

                  EINA_LIST_FOREACH(con->zones, zl, zone) 
                    _policy_zone_layout_update(zone);
               }
          }
     }
}
