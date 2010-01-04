#include "E_Illume.h"
#include "layout.h"

/* local function prototypes */
static void _border_resize_fx(E_Border *bd, int bx, int by, int bw, int bh);
static void _border_resize_move(E_Border *bd, int bx, int by, int bw, int bh);
static void _zone_layout_single(E_Border *bd);
static void _zone_layout_dual(E_Border *bd);
static void _zone_layout_dual_top(E_Border *bd);
static void _zone_layout_dual_top_custom(E_Border *bd);
static void _zone_layout_dual_left(E_Border *bd);

/* local variables */
static int shelfsize = 0;
static int kbdsize = 0;
static int panelsize = 0;

void 
_layout_border_add(E_Border *bd) 
{
   /* HANDLE A BORDER BEING ADDED */

   int conform;

   /* skip new clients and invisible borders */
   if ((bd->new_client) || (!bd->visible)) return;

   /* check if this border is conformant */
   conform = e_illume_border_is_conformant(bd);

   /* is this a fullscreen border ? */
   if ((bd->need_fullscreen) || (bd->fullscreen)) 
     {
        E_Border *b;

        if (bd->layer != IL_FULLSCREEN_LAYER) 
          e_border_layer_set(bd, IL_FULLSCREEN_LAYER);

        /* we lock stacking so that the keyboard does not get put 
         * under the window (if it's needed) */
        bd->lock_user_stacking = 1;

        /* conformant fullscreen borders just hide bottom panel */
        b = e_illume_border_bottom_panel_get(bd->zone);
        if (b) e_border_fx_offset(b, 0, -panelsize);

        /* for non-conformant fullscreen borders, 
         * we hide top shelf and bottom panel in all cases */
        if (!conform) 
          {
             b = e_illume_border_top_shelf_get(bd->zone);
             if (b) e_border_fx_offset(b, 0, -shelfsize);
          }
     }
   else if (conform) 
     {
        if (bd->layer != IL_CONFORM_LAYER) 
          e_border_layer_set(bd, IL_CONFORM_LAYER);
        bd->lock_user_stacking = 1;
     }

   /* only set focus if border accepts it and it's not locked out */
   if ((bd->client.icccm.accepts_focus) && (bd->client.icccm.take_focus) 
       && (!bd->lock_focus_out))
     e_border_focus_set(bd, 1, 1);
}

void 
_layout_border_del(E_Border *bd) 
{
   /* HANDLE A BORDER BEING DELETED */

   if ((bd->need_fullscreen) || (bd->fullscreen)) 
     {
        E_Border *b;

        /* conformant fullscreen borders just get bottom panel shown */
        b = e_illume_border_bottom_panel_get(bd->zone);
        if (b) e_border_fx_offset(b, 0, 0);

        /* for non-conformant fullscreen borders, 
         * we show top shelf and bottom panel in all cases */
        if (!e_illume_border_is_conformant(bd)) 
          {
             b = e_illume_border_top_shelf_get(bd->zone);
             if (b) e_border_fx_offset(b, 0, 0);
          }
     }
}

void 
_layout_border_focus_in(E_Border *bd) 
{
   /* do something if focus enters a window */
}

void 
_layout_border_focus_out(E_Border *bd) 
{
   /* do something if focus exits a window */
}

void 
_layout_border_activate(E_Border *bd) 
{
   /* HANDLE A BORDER BEING ACTIVATED */

   if (bd->stolen) return;

   /* only set focus if border accepts it and it's not locked out */
   if (((!bd->client.icccm.accepts_focus) && (!bd->client.icccm.take_focus)) ||
       (bd->lock_focus_out))
     return;

   /* if the border is not focused, check focus settings */
   if ((bd) && (!bd->focused)) 
     {
        if ((e_config->focus_setting == E_FOCUS_NEW_WINDOW) || 
            ((bd->parent) && 
             ((e_config->focus_setting == E_FOCUS_NEW_DIALOG) || 
              ((bd->parent->focused) && 
               (e_config->focus_setting == E_FOCUS_NEW_DIALOG_IF_OWNER_FOCUSED))))) 
          {
             if (bd->iconic) 
               {
                  /* if it's iconic, then uniconify */
                  if (!bd->lock_user_iconify) e_border_uniconify(bd);
               }
             /* if we can, raise the border */
             if (!bd->lock_user_stacking) e_border_raise(bd);
             /* if we can, focus the border */
             if (!bd->lock_focus_out) e_border_focus_set(bd, 1, 1);
          }
     }
}

void 
_layout_zone_layout(E_Zone *zone) 
{
   Eina_List *l, *borders;
   E_Border *bd;

   /* ACTUALLY APPLY THE SIZING, POSITIONING AND LAYERING */

   /* grab list of borders */
   borders = e_border_client_list();
   EINA_LIST_FOREACH(borders, l, bd) 
     {
        int mh;

        /* skip border if: zone not the same, a new client, or invisible */
        if ((bd->zone != zone) || (bd->new_client) || (!bd->visible)) continue;

        /* check for special windows to get their size(s) */
        e_illume_border_min_get(bd, NULL, &mh);
        if (e_illume_border_is_top_shelf(bd)) 
          {
             if (shelfsize < mh) shelfsize = mh;
          }
        else if (e_illume_border_is_bottom_panel(bd)) 
          {
             if (panelsize < mh) panelsize = mh;
          }
        else if (e_illume_border_is_keyboard(bd)) 
          {
             if (kbdsize < mh) kbdsize = mh;
          }
     }

   /* grab list of borders */
   borders = e_border_client_list();
   EINA_LIST_FOREACH(borders, l, bd) 
     {
        /* skip border if: zone not the same, a new client, or invisible */
        if ((bd->zone != zone) || (bd->new_client) || (!bd->visible)) continue;

        /* trap 'special' windows as they need special treatment */
        if (e_illume_border_is_top_shelf(bd)) 
          {
             /* make sure we are not dragging the shelf */
             if (!bd->client.illume.drag.drag)
               {
                  /* if we are not in dual mode, then set shelf to top */
                  if (!il_cfg->policy.mode.dual)
                    _border_resize_move(bd, zone->x, zone->y, zone->w, shelfsize);
                  else 
                    {
                       /* make sure we are in landscape mode */
                       if (il_cfg->policy.mode.side == 0) 
                         _border_resize_move(bd, zone->x, bd->y, zone->w, shelfsize);
                       else 
                         _border_resize_move(bd, zone->x, zone->y, zone->w, shelfsize);
                    }
               }
             e_border_stick(bd);
             if (bd->layer != IL_TOP_SHELF_LAYER) 
               e_border_layer_set(bd, IL_TOP_SHELF_LAYER);
          }
        else if (e_illume_border_is_bottom_panel(bd)) 
          {
             /* make sure we are not dragging the bottom panel */
             if (!bd->client.illume.drag.drag)
               _border_resize_fx(bd, zone->x, (zone->y + zone->h - panelsize), 
                                 zone->w, panelsize);
             e_border_stick(bd);
             if (bd->layer != IL_BOTTOM_PANEL_LAYER) 
               e_border_layer_set(bd, IL_BOTTOM_PANEL_LAYER);
          }
        else if (e_illume_border_is_keyboard(bd)) 
          {
             if ((bd->h != kbdsize) || (bd->w != zone->w))
               e_border_resize(bd, zone->w, kbdsize);
             if ((bd->x != zone->x) || (bd->y != (zone->y + zone->h - kbdsize)) || 
                 (bd->fx.y != (zone->y + zone->h - kbdsize)))
               e_border_move(bd, zone->x, (zone->y + zone->h - kbdsize));
             e_border_stick(bd);
             if (bd->layer != IL_KEYBOARD_LAYER) 
               e_border_layer_set(bd, IL_KEYBOARD_LAYER);
          }
        else if (e_illume_border_is_dialog(bd)) 
          {
             int mw, mh;

             e_illume_border_min_get(bd, &mw, &mh);
             if (mw > zone->w) mw = zone->w;
             if (mh > zone->h) mh = zone->h;
             _border_resize_fx(bd, (zone->x + ((zone->w - mw) / 2)), 
                               (zone->y + ((zone->h - mh) / 2)), mw, mh);
             if (bd->layer != IL_DIALOG_LAYER) 
               e_border_layer_set(bd, IL_DIALOG_LAYER);
          }
        else if (e_illume_border_is_quickpanel(bd)) 
          {
             int mh;

             e_illume_border_min_get(bd, NULL, &mh);
             if ((bd->w != bd->zone->w) || (bd->h != mh)) 
               e_border_resize(bd, bd->zone->w, mh);
             if (bd->layer != IL_QUICKPANEL_LAYER) 
               e_border_layer_set(bd, IL_QUICKPANEL_LAYER);
             bd->lock_user_stacking = 1;
          }
        else 
          {
             if (bd->layer != IL_APP_LAYER) 
               e_border_layer_set(bd, IL_APP_LAYER);

             /* normal border, handle layout based on policy mode */
             if (il_cfg->policy.mode.dual) _zone_layout_dual(bd);
             else _zone_layout_single(bd);
          }
     }
}

void 
_layout_zone_move_resize(E_Zone *zone) 
{
   /* the zone was moved or resized - re-configure all windows in this zone */
   _layout_zone_layout(zone);
}

void 
_layout_drag_start(E_Border *bd) 
{
   /* HANDLE A BORDER DRAG BEING STARTED */
   ecore_x_e_illume_drag_set(bd->client.win, 1);
}

void 
_layout_drag_end(E_Border *bd) 
{
   /* HANDLE A BORDER DRAG BEING ENDED */
   ecore_x_e_illume_drag_set(bd->client.win, 0);
}

/* local functions */
static void 
_border_resize_fx(E_Border *bd, int bx, int by, int bw, int bh) 
{
   /* CONVENIENCE FUNCTION TO REMOVE DUPLICATED CODE */

   if ((bd->need_fullscreen) || (bd->fullscreen)) 
     {
        if ((bd->w != bw) || (bd->h != bh)) 
          {
             bd->w = bw;
             bd->h = bh;
             bd->client.w = bw;
             bd->client.h = bh;
             bd->changes.size = 1;
             bd->changed = 1;
          }
        if ((bd->x != bx) || (bd->y != by)) 
          {
             bd->x = bx;
             bd->y = by;
             bd->changes.pos = 1;
             bd->changed = 1;
          }
     }
   else 
     {
        if ((bd->w != bw) || (bd->h != bh)) 
          e_border_resize(bd, bw, bh);
        if ((bd->x != bx) || (bd->y != by)) 
          e_border_fx_offset(bd, bx, by);
     }
}

static void 
_border_resize_move(E_Border *bd, int bx, int by, int bw, int bh) 
{
   if ((bd->w != bw) || (bd->h != bh)) 
     e_border_resize(bd, bw, bh);
   if ((bd->x != bx) || (bd->y != by)) 
     e_border_move(bd, bx, by);
}

static void 
_zone_layout_single(E_Border *bd) 
{
   int kx, ky, kw, kh, ss, ps;

   /* grab the 'safe' region. Safe region is space not occupied by keyboard */
   e_illume_kbd_safe_app_region_get(bd->zone, &kx, &ky, &kw, &kh);
   if (!e_illume_border_is_conformant(bd)) 
     {
        if (!((bd->need_fullscreen) || (bd->fullscreen))) 
          {
             if (kh >= bd->zone->h) ps = panelsize;
             ss = shelfsize;
          }
     }
   else 
     kh = bd->zone->h;
   _border_resize_fx(bd, kx, (ky + ss), kw, (kh - ss - ps));
}

static void 
_zone_layout_dual(E_Border *bd) 
{
   if (il_cfg->policy.mode.side == 0) 
     {
        int ty;

        e_illume_border_top_shelf_pos_get(bd->zone, NULL, &ty);
        if (ty <= bd->zone->y)
          _zone_layout_dual_top(bd);
        else
          _zone_layout_dual_top_custom(bd);
     }
   else if (il_cfg->policy.mode.side == 1) 
     _zone_layout_dual_left(bd);
}

static void 
_zone_layout_dual_top(E_Border *bd) 
{
   int kx, ky, kw, kh, ss, ps;
   int conform;

   /* fetch if this border is conformant */
   conform = e_illume_border_is_conformant(bd);

   /* grab the 'safe' region. Safe region is space not occupied by keyboard */
   e_illume_kbd_safe_app_region_get(bd->zone, &kx, &ky, &kw, &kh);
   if (!conform) 
     {
        /* if the border is not conformant and doesn't need fullscreen, then 
         * we account for shelf & panel sizes */
        if (!((bd->need_fullscreen) || (bd->fullscreen))) 
          {
             if (kh >= bd->zone->h) ps = panelsize;
             ss = shelfsize;
          }
     }

   /* if there are no other borders, than give this one all available space */
   if (e_illume_border_valid_count_get(bd->zone) < 2) 
     _border_resize_fx(bd, kx, (ky + ss), kw, (kh - ss - ps));
   else 
     {
        E_Border *b;
        int bx, by, bw, bh;

        /* more than one valid border */
        bx = kx;
        by = (ky + ss);
        bw = kw;
        bh = (kh - ss - ps);

        /* grab the border at this location */
        b = e_illume_border_at_xy_get(bd->zone, kx, shelfsize);

        if ((b) && (bd != b)) 
          {
             /* we have a border there, and it's not the current one */
             if (!e_illume_border_is_conformant(b)) 
               {
                  /* border in this location is not conformant */
                  bh = ((kh - ss - ps) / 2);
                  by = (b->fx.y + b->h);
               }
             else 
               {
                  /* border there is conformant */
                  if (conform) 
                    {
                       /* if current border is conformant, divide zone in half */
                       bh = ((bd->zone->h - ss) / 2);
                       by = by + bh;
                    }
                  else 
                    {
                       /* current border is not conformant */
                       by = (b->fx.y + b->h);
                       bh = (kh - by - ps);
                    }
               }
          }
        else if (b) 
          {
             /* border at this location and it's the current border */
             by = bd->fx.y;
             bh = ((kh - ss - ps) / 2);
          }
        else 
          {
             /* no border at this location */
             b = e_illume_border_valid_border_get(bd->zone);
             by = ky + ss;
             bh = (ky - b->h);
          }
        _border_resize_fx(bd, bx, by, bw, bh);
     }
}

static void 
_zone_layout_dual_top_custom(E_Border *bd) 
{
   int kx, kw;
   int count, conform;
   int ax, ay, aw, ah;
   int zx, zy, zw, zh;

   /* get count of valid borders */
   count = e_illume_border_valid_count_get(bd->zone);

   /* fetch if this border is conformant */
   conform = e_illume_border_is_conformant(bd);

   /* grab the 'safe' region. Safe region is space not occupied by keyboard */
   e_illume_kbd_safe_app_region_get(bd->zone, &kx, NULL, &kw, NULL);

   e_illume_border_app1_safe_region_get(bd->zone, &ax, &ay, &aw, &ah);
   e_illume_border_app2_safe_region_get(bd->zone, &zx, &zy, &zw, &zh);

   /* if there are no other borders, than give this one all available space */
   if (count < 2) 
     {
        if (ah >= zh) 
          {
             zx = ax;
             zy = ax;
             zw = aw;
             zh = ah;
          }
        if ((bd->w != zw) || (bd->h != zh))
          e_border_resize(bd, zw, zh);
        if ((bd->x != zx) || (bd->y != zy) || (bd->fx.y != zy))
          e_border_fx_offset(bd, zx, zy);
     }
   else 
     {
        E_Border *bt, *bb;
        int bh, by;

        /* more than one valid border */

        if (bd->client.vkbd.state == ECORE_X_VIRTUAL_KEYBOARD_STATE_OFF)
//        if (ecore_x_e_virtual_keyboard_state_get(bd->client.win) > 
//            ECORE_X_VIRTUAL_KEYBOARD_STATE_OFF) 
          {
             bh = ah;
             by = ay;
          }
        else 
          {
             /* grab the border at this location */
             bt = e_illume_border_at_xy_get(bd->zone, kx, ay);

             if ((bt) && (bd != bt)) 
               {
                  /* is there a border in the bottom section */
                  bb = e_illume_border_at_xy_get(bd->zone, kx, zy);
                  if (!bb) 
                    {
                       bh = zh;
                       by = zy;
                    }
                  else if ((bb) && (bd != bb))
                    {
                       if (bt == e_border_focused_get()) 
                         {
                            if (bd->client.vkbd.state == ECORE_X_VIRTUAL_KEYBOARD_STATE_OFF)
//                            if (ecore_x_e_virtual_keyboard_state_get(bd->client.win) <= 
//                                ECORE_X_VIRTUAL_KEYBOARD_STATE_OFF) 
                              {
                                 bh = zh;
                                 by = zy;
                              }
                            else 
                              {
                                 bh = ah;
                                 by = ay;
                              }
                         }
                       else if (bb = e_border_focused_get()) 
                         {
                            if (bd->client.vkbd.state == ECORE_X_VIRTUAL_KEYBOARD_STATE_OFF)
//                            if (ecore_x_e_virtual_keyboard_state_get(bd->client.win) <= 
//                                ECORE_X_VIRTUAL_KEYBOARD_STATE_OFF) 
                              {
                                 bh = ah;
                                 by = ay;
                              }
                         }
                    }
                  else if (bb) 
                    {
                       bh = zh;
                       by = zy;
                    }
               }
             else 
               {
                  bh = ah;
                  by = ay;
               }
          }
        if ((bd->w != kw) || (bd->h != bh))
          e_border_resize(bd, kw, bh);
        if ((bd->x != kx) || (bd->y != by) || (bd->fx.y != by))
          e_border_fx_offset(bd, kx, by);
     }
}

static void 
_zone_layout_dual_left(E_Border *bd) 
{
   int kx, ky, kw, kh, ss, ps;
   int conform;

   /* fetch if this border is conformant */
   conform = e_illume_border_is_conformant(bd);

   /* grab the 'safe' region. Safe region is space not occupied by keyboard */
   e_illume_kbd_safe_app_region_get(bd->zone, &kx, &ky, &kw, &kh);
   if (!conform) 
     {
        /* if the border is not conformant and doesn't need fullscreen, then 
         * we account for shelf & panel sizes */
        if (!((bd->need_fullscreen) || (bd->fullscreen))) 
          {
             if (kh >= bd->zone->h) ps = panelsize;
             ss = shelfsize;
          }
     }

   /* if there are no other borders, than give this one all available space */
   if (e_illume_border_valid_count_get(bd->zone) < 2) 
     _border_resize_fx(bd, kx, (ky + ss), kw, (kh - ss - ps));
   else 
     {
        E_Border *b;
        int bx, by, bw, bh;

        /* more than one valid border */
        bx = kx;
        by = (ky + ss);
        bw = kw;
        bh = (kh - ss - ps);

        /* grab the border at this location */
        b = e_illume_border_at_xy_get(bd->zone, kx, shelfsize);

        if ((b) && (bd != b)) 
          {
             /* we have a border there, and it's not the current one */
             if (!e_illume_border_is_conformant(b)) 
               {
                  /* border in this location is not conformant */
                  bw = (kw / 2);
                  bx = (b->fx.x + b->w);
               }
             else 
               {
                  /* border there is conformant */
                  if (conform) 
                    {
                       /* if current border is conformant, divide zone in half */
                       bw = (bd->zone->w / 2);
                       bx = bx + bw;
                    }
                  else 
                    {
                       /* current border is not conformant */
                       bx = (b->fx.x + b->w);
                       bw = (kw - bx);
                    }
               }
          }
        else if (b) 
          {
             /* border at this location and it's the current border */
             bx = bd->fx.x;
             bw = (kw / 2);
          }
        else 
          {
             /* no border at this location */
             b = e_illume_border_valid_border_get(bd->zone);
             bx = kx;
             bw = (kw - b->w);
          }
        _border_resize_fx(bd, bx, by, bw, bh);
     }
}
