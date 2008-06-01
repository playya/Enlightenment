#include <e.h>
//#include <X11/extensions/shape.h>
#include "config.h"
#include "e_mod_main.h"
#include "e_mod_gadman.h"
#include "e_mod_config.h"

/* local protos */
static void _attach_menu(void *data, E_Gadcon_Client *gcc, E_Menu *menu);
static void _save_widget_position(E_Gadcon_Client *gcc);
static void _apply_widget_position(E_Gadcon_Client *gcc);
static char *_get_bind_text(const char* action);

static void _hide_finished(void *data, Evas_Object *o, const char *em, const char *src);

static Evas_Object* _create_mover(E_Gadcon *gc);
static Evas_Object* _get_mover(E_Gadcon_Client *gcc);
static E_Gadcon* _gadman_gadcon_new(const char* name, int ontop);

static void on_shape_change(void *data, E_Container_Shape *es, E_Container_Shape_Change ch);

static void on_top(void *data, Evas_Object *o, const char *em, const char *src);
static void on_right(void *data, Evas_Object *o, const char *em, const char *src);
static void on_down(void *data, Evas_Object *o, const char *em, const char *src);
static void on_left(void *data, Evas_Object *o, const char *em, const char *src);
static void on_move(void *data, Evas_Object *o, const char *em, const char *src);

static void on_frame_click(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void on_bg_click(void *data, Evas_Object *o, const char *em, const char *src);

static void on_menu_style_plain(void *data, E_Menu *m, E_Menu_Item *mi);
static void on_menu_style_inset(void *data, E_Menu *m, E_Menu_Item *mi);
static void on_menu_layer_bg(void *data, E_Menu *m, E_Menu_Item *mi);
static void on_menu_layer_top(void *data, E_Menu *m, E_Menu_Item *mi);
static void on_menu_delete(void *data, E_Menu *m, E_Menu_Item *mi);
static void on_menu_edit(void *data, E_Menu *m, E_Menu_Item *mi);
static void on_menu_add(void *data, E_Menu *m, E_Menu_Item *mi);

E_Gadcon_Client *current;

/* Implementation */
void
gadman_init(E_Module *m)
{
   Evas_List *managers, *l;

   /* Create Manager */
   Man = calloc(1, sizeof(Manager));
   if (!Man) return;

   Man->module = m;
   Man->container = e_container_current_get(e_manager_current_get());
   Man->width = Man->container->w;
   Man->height = Man->container->h;
   Man->gadgets = NULL;
   Man->top_ee = NULL;
   Man->visible = 0;

   /* Check if composite is enable */
   if (ecore_x_screen_is_composited(0) || e_config->use_composite)
     Man->use_composite = 1;
   else
     Man->use_composite = 0;

   /* with this we can trap screen resolution change (a better way?)*/
   e_container_shape_change_callback_add(Man->container, on_shape_change, NULL);

   /* Create Gadcon for background and top */
   Man->gc = _gadman_gadcon_new("gadman", 0);
   Man->gc_top = _gadman_gadcon_new("gadman_top", 1);

   /* Create 2 mover objects */
   Man->mover = _create_mover(Man->gc);
   Man->mover_top = _create_mover(Man->gc_top);

   /* Start existing gadgets */
   for (l = Man->gc->cf->clients; l; l = l->next)
     {
        E_Config_Gadcon_Client *cf_gcc;

        if (!(cf_gcc = l->data)) continue;
        gadman_gadget_place(cf_gcc, 0);
     }

   for (l = Man->gc_top->cf->clients; l; l = l->next)
     {
        E_Config_Gadcon_Client *cf_gcc;

        if(!(cf_gcc = l->data)) continue;
        gadman_gadget_place(cf_gcc, 1);
     }
}

void
gadman_shutdown(void)
{
   e_container_shape_change_callback_del(Man->container, on_shape_change, NULL);
   printf("++++++++    UNPOPULATE BG\n");
   e_gadcon_unpopulate(Man->gc); // TODO FIX ME !!
   printf("++++++++    UNPOPULATE TOP\n");
   e_gadcon_unpopulate(Man->gc_top);

   /* free gadcons */
   e_config->gadcons = evas_list_remove(e_config->gadcons, Man->gc);
   e_config->gadcons = evas_list_remove(e_config->gadcons, Man->gc_top);
   evas_stringshare_del(Man->gc->name);
   evas_stringshare_del(Man->gc_top->name);
   if (Man->gc->config_dialog) e_object_del(E_OBJECT(Man->gc->config_dialog));
   if (Man->icon_name) evas_stringshare_del(Man->icon_name);
   free(Man->gc);
   free(Man->gc_top);

   /* free manager */
   evas_object_del(Man->mover);
   evas_object_del(Man->mover_top);
   evas_list_free(Man->gadgets);
   if (Man->top_ee)
     {
        e_canvas_del(Man->top_ee);
        //ecore_evas_free(Man->top_ee);
     }
   free(Man);
   Man = NULL;
}

E_Gadcon_Client *
gadman_gadget_place(E_Config_Gadcon_Client *cf, int ontop)
{
   E_Gadcon *gc;
   E_Gadcon_Client *gcc;
   E_Gadcon_Client_Class *cc = NULL;
   Evas_List *l = NULL;

   if (!cf->name) return NULL;

   if (ontop) gc = Man->gc_top;
   else gc = Man->gc;

   /* Find provider */
   for (l = e_gadcon_provider_list(); l; l = l->next) 
     {
        cc = l->data;
        if (!strcmp(cc->name, cf->name))
          break;
        else
          cc = NULL;
     }
   if (!cc) return NULL;

   /* init Gadcon_Client */
   gcc = cc->func.init(gc, cf->name, cf->id, cc->default_style);
   gcc->cf = cf;
   gcc->client_class = cc;

   Man->gadgets = evas_list_append(Man->gadgets, gcc);

   //printf("Place Gadget %s (style: %s id: %s) (gadcon: %s)\n", gcc->name, cf->style, cf->id, gc->name);

   /* create frame */
   gcc->o_frame = edje_object_add(gc->evas);
   e_theme_edje_object_set(gcc->o_frame, "base/theme/gadman", "e/gadman/frame");

   if ((cf->style) && (!strcmp(cf->style, E_GADCON_CLIENT_STYLE_INSET)))
     edje_object_signal_emit(gcc->o_frame, "e,state,visibility,inset", "e");
   else
     edje_object_signal_emit(gcc->o_frame, "e,state,visibility,plain", "e");

   /* swallow the client inside the frame */
   edje_object_part_swallow(gcc->o_frame, "e.swallow.content", gcc->o_base);
   evas_object_event_callback_add(gcc->o_frame, EVAS_CALLBACK_MOUSE_DOWN, 
                                  on_frame_click, gcc);

   _apply_widget_position(gcc);

   if (gcc->gadcon == Man->gc_top)
     edje_object_signal_emit(gcc->o_frame, "e,state,visibility,hide", "e");

   evas_object_show(gcc->o_frame);

   return gcc;
}

E_Gadcon_Client *
gadman_gadget_add(E_Gadcon_Client_Class *cc, int ontop)
{
   E_Config_Gadcon_Client *cf = NULL;
   E_Gadcon_Client *gcc;
   E_Gadcon *gc;
   char *id;

   if (ontop)
     gc = Man->gc_top;
   else
     gc = Man->gc;

   /* Create Config_Gadcon_Client */
   cf = e_gadcon_client_config_new(gc, cc->name);
   cf->style = evas_stringshare_add(cc->default_style);
   cf->geom.pos_x = DEFAULT_POS_X;
   cf->geom.pos_y = DEFAULT_POS_Y;
   cf->geom.size_w = DEFAULT_SIZE_W;
   cf->geom.size_h = DEFAULT_SIZE_H;

   /* Place the new gadget */
   gcc = gadman_gadget_place(cf, ontop);

   return gcc;
}

void
gadman_gadget_remove(E_Gadcon_Client *gcc)
{
   Man->gadgets = evas_list_remove(Man->gadgets, gcc);

   edje_object_part_unswallow(gcc->o_frame, gcc->o_base);
   evas_object_del(gcc->o_frame);

   gcc->gadcon->clients = evas_list_remove(gcc->gadcon->clients, gcc);

   e_object_del(E_OBJECT(gcc));
   current = NULL;
}

void
gadman_gadget_del(E_Gadcon_Client *gcc)
{
   Man->gadgets = evas_list_remove(Man->gadgets, gcc);

   edje_object_part_unswallow(gcc->o_frame, gcc->o_base);
   evas_object_del(gcc->o_frame);

   e_gadcon_client_config_del(current->gadcon->cf, gcc->cf);
   gcc->gadcon->clients = evas_list_remove(gcc->gadcon->clients, gcc);
   e_object_del(E_OBJECT(gcc));

   current = NULL;
}

void
gadman_gadget_edit_start(E_Gadcon_Client *gcc)
{
   E_Gadcon *gc;
   Evas_Object *mover;
   int x, y, w, h;

   current = gcc;

   gc = gcc->gadcon;
   gc->editing = 1;

   /* Move/resize the correct mover */
   evas_object_geometry_get(gcc->o_frame, &x, &y, &w, &h);
   mover = _get_mover(gcc);

   evas_object_move(mover, x, y);
   evas_object_resize(mover, w, h);
   evas_object_raise(mover);
   evas_object_show(mover);
}

void
gadman_gadget_edit_end(void)
{
   evas_object_hide(Man->mover);
   evas_object_hide(Man->mover_top);

   Man->gc->editing = 0;
   Man->gc_top->editing = 0;

   _save_widget_position(current);
}

void
gadman_gadgets_show(void)
{
   Evas_List *l = NULL;

   Man->visible = 1;
   ecore_evas_show(Man->top_ee);

   edje_object_signal_emit(Man->full_bg, "e,state,visibility,show", "e");

   for (l = Man->gadgets; l; l = l->next)
     {
        E_Gadcon_Client *gcc;

        if (!(gcc = l->data)) continue;
        if (gcc->gadcon == Man->gc_top)
          edje_object_signal_emit(gcc->o_frame, "e,state,visibility,show", "e");
     }
}

void
gadman_gadgets_hide(void)
{
   Evas_List *l = NULL;

   Man->visible = 0;

   edje_object_signal_emit(Man->full_bg, "e,state,visibility,hide", "e");

   for (l = Man->gadgets; l; l = l->next)
     {
        E_Gadcon_Client *gcc;

        if (!(gcc = l->data)) continue;
        if (gcc->gadcon == Man->gc_top)
          edje_object_signal_emit(gcc->o_frame, "e,state,visibility,hide", "e");
     }
}

void
gadman_gadgets_toggle(void)
{
   if (Man->visible)
     gadman_gadgets_hide();
   else
     gadman_gadgets_show();
}

/* Internals */
static E_Gadcon*
_gadman_gadcon_new(const char* name, int ontop)
{
   E_Gadcon *gc;
   Evas_List *l = NULL;

   /* Create Gadcon */
   gc = E_OBJECT_ALLOC(E_Gadcon, E_GADCON_TYPE, NULL);
   if (!gc) return NULL;

   gc->name = evas_stringshare_add(name);
   gc->layout_policy = E_GADCON_LAYOUT_POLICY_PANEL;
   gc->orient = E_GADCON_ORIENT_FLOAT;

   /* Create ecore fullscreen window */
   if (ontop)
     {
        Man->top_ee = 
          e_canvas_new(e_config->evas_engine_popups, Man->container->win, 
                       0, 0, 0, 0, 1, 1, &(Man->top_win), NULL);

        if (Man->use_composite)
          {
             ecore_evas_alpha_set(Man->top_ee, 1);
             Man->top_win = ecore_evas_software_x11_window_get(Man->top_ee);
             // Leave 1px to switch desktop when mouse is on border
             ecore_x_window_shape_rectangle_set(Man->top_win, 1, 1, 
                                                (Man->width - 2), 
                                                (Man->height - 2));
          }
        else
          ecore_evas_shaped_set(Man->top_ee, 1);

// this isn't needed - we don't want to keep a pixmap of the whole canvas around!
//        ecore_evas_avoid_damage_set(Man->top_ee, 1); //??
        e_canvas_add(Man->top_ee); //??

        e_container_window_raise(Man->container, Man->top_win, 250);

        ecore_evas_move_resize(Man->top_ee, 0, 0, Man->width, Man->height);
        ecore_evas_hide(Man->top_ee);

        gc->evas = ecore_evas_get(Man->top_ee);
        e_gadcon_ecore_evas_set(gc, Man->top_ee);

        /* create full background object */
        Man->full_bg = edje_object_add(gc->evas);
        e_theme_edje_object_set(Man->full_bg, "base/theme/gadman", 
                                "e/gadman/full_bg");
        edje_object_signal_callback_add(Man->full_bg, "mouse,down,*", "bg",
                                        on_bg_click, NULL);
        edje_object_signal_callback_add(Man->full_bg, "program,stop", "hide",
                                        _hide_finished, NULL);
        evas_object_move(Man->full_bg, 0, 0);
        evas_object_resize(Man->full_bg, Man->width, Man->height);
        evas_object_show(Man->full_bg);
     }
   /* ... or use the e background window */
   else
     {
        gc->evas = Man->container->bg_evas;
        e_gadcon_ecore_evas_set(gc, Man->container->bg_ecore_evas);
     }

   e_gadcon_zone_set(gc, e_zone_current_get(Man->container));
   e_gadcon_util_menu_attach_func_set(gc, _attach_menu, NULL);

   gc->id = 114 + ontop; // TODO what's this ??????? 114 is a random number
   gc->edje.o_parent = NULL;
   gc->edje.swallow_name = NULL;
   gc->shelf = NULL;
   gc->toolbar = NULL;
   gc->editing = 0;
   gc->o_container = NULL;
   gc->frame_request.func = NULL;
   gc->resize_request.func = NULL;
   gc->min_size_request.func = NULL;

   /* Search for existing gadcon config */
   gc->cf = NULL;
   for (l = e_config->gadcons; l; l=l->next)
     {
        E_Config_Gadcon *cg;

        if (!(cg = l->data)) continue;
        if (!strcmp(cg->name, name))
          {
             gc->cf = cg;
             break;
          }
     }

   /* ... or create a new one */
   if (!gc->cf)
     {
        gc->cf = E_NEW(E_Config_Gadcon, 1);
        gc->cf->name = evas_stringshare_add(name);
        gc->cf->id = gc->id;
        gc->cf->clients = NULL;
        e_config->gadcons = evas_list_append(e_config->gadcons, gc->cf);
        e_config_save_queue();
     }

   return gc;
}

static Evas_Object *
_create_mover(E_Gadcon *gc)
{
   Evas_Object *mover;

   /* create mover object */
   mover = edje_object_add(gc->evas);
   e_theme_edje_object_set(mover, "base/theme/gadman", "e/gadman/control");

   edje_object_signal_callback_add(mover, "mouse,down,1", "overlay",
                                   on_move, (void*)DRAG_START);
   edje_object_signal_callback_add(mover, "mouse,up,1", "overlay",
                                   on_move, (void*)DRAG_STOP);
   edje_object_signal_callback_add(mover, "mouse,down,3", "overlay",
                                   gadman_gadget_edit_end, NULL);

   edje_object_signal_callback_add(mover, "mouse,down,1", "h1",
                                   on_left, (void*)DRAG_START);
   edje_object_signal_callback_add(mover, "mouse,up,1", "h1",
                                   on_left, (void*)DRAG_STOP);
   edje_object_signal_callback_add(mover, "mouse,down,1", "v2",
                                   on_down, (void*)DRAG_START);
   edje_object_signal_callback_add(mover, "mouse,up,1", "v2",
                                   on_down, (void*)DRAG_STOP);
   edje_object_signal_callback_add(mover, "mouse,down,1", "h2",
                                   on_right, (void*)DRAG_START);
   edje_object_signal_callback_add(mover, "mouse,up,1", "h2",
                                   on_right, (void*)DRAG_STOP);
   edje_object_signal_callback_add(mover, "mouse,down,1", "v1",
                                   on_top, (void*)DRAG_START);
   edje_object_signal_callback_add(mover, "mouse,up,1", "v1",
                                   on_top, (void*)DRAG_STOP);

   evas_object_move(mover, 20, 30);
   evas_object_resize(mover, 100, 100);
   evas_object_hide(mover);

   return mover;
}

static Evas_Object *
_get_mover(E_Gadcon_Client *gcc)
{
   if (gcc->gadcon == Man->gc_top)
     return Man->mover_top;
   else
     return Man->mover;
}

static void
_save_widget_position(E_Gadcon_Client *gcc)
{
   int x, y, w, h;

   evas_object_geometry_get(gcc->o_frame, &x, &y, &w, &h);
   current->cf->geom.pos_x = (double)x / (double)Man->width;
   current->cf->geom.pos_y = (double)y / (double)Man->height;
   current->cf->geom.size_w = (double)w / (double)Man->width;;
   current->cf->geom.size_h = (double)h / (double)Man->height;

   e_config_save_queue();
}

static void
_apply_widget_position(E_Gadcon_Client *gcc)
{
   Evas_List *l;
   int x, y, w, h;
   int fx, fy, fw, fh;

   x = gcc->cf->geom.pos_x * Man->width;
   y = gcc->cf->geom.pos_y * Man->height;
   w = gcc->cf->geom.size_w * Man->width;
   h = gcc->cf->geom.size_h * Man->height;

   /* Respect min sizes */
   if (h < gcc->min.h) h = gcc->min.h;
   if (w < gcc->min.w) w = gcc->min.w;
   if (h < 1) h = 100;
   if (w < 1) w = 100;

   /* Respect screen margin */
   if (x < 0) x = 0;
   if (y < 0) y = 0;
   if (x > Man->width) x = 0;
   if (y > Man->height) y = 0;

   if ((y + h) > Man->height) h = (Man->height - y);
   if ((x + w) > Man->width) w = (Man->width - x);

   evas_object_move(gcc->o_frame, x, y);
   evas_object_resize(gcc->o_frame, w, h);
}

static void
_attach_menu(void *data, E_Gadcon_Client *gcc, E_Menu *menu)
{
   E_Menu *mn;
   E_Menu_Item *mi;
   char buf[128];

   //printf("Attach menu (gcc: %x id: %s) [%s]\n", gcc, gcc->cf->id, gcc->cf->style);
   if (!gcc) return;

   if (!gcc->cf->style)
     gcc->cf->style = evas_stringshare_add(E_GADCON_CLIENT_STYLE_INSET);

   /* plain / inset */
   mn = e_menu_new();
   mi = e_menu_item_new(mn);
   e_menu_item_label_set(mi, _("Plain"));
   e_menu_item_radio_set(mi, 1);
   e_menu_item_radio_group_set(mi, 1);
   if (!strcmp(gcc->cf->style, E_GADCON_CLIENT_STYLE_PLAIN))
     e_menu_item_toggle_set(mi, 1);
   e_menu_item_callback_set(mi, on_menu_style_plain, gcc);

   mi = e_menu_item_new(mn);
   e_menu_item_label_set(mi, _("Inset"));
   e_menu_item_radio_set(mi, 1);
   e_menu_item_radio_group_set(mi, 1);
   if (!strcmp(gcc->cf->style, E_GADCON_CLIENT_STYLE_INSET))
     e_menu_item_toggle_set(mi, 1);
   e_menu_item_callback_set(mi, on_menu_style_inset, gcc);

   mi = e_menu_item_new(menu);
   e_menu_item_label_set(mi, _("Appearance"));
   e_util_menu_item_edje_icon_set(mi, "enlightenment/appearance");
   e_menu_item_submenu_set(mi, mn);
   e_object_del(E_OBJECT(mn));

   /* bg / ontop */
   mn = e_menu_new();
   mi = e_menu_item_new(mn);
   e_menu_item_label_set(mi, _("Always on desktop"));
   e_menu_item_radio_set(mi, 1);
   e_menu_item_radio_group_set(mi, 2);
   if (gcc->gadcon == Man->gc)
     e_menu_item_toggle_set(mi, 1);
   e_menu_item_callback_set(mi, on_menu_layer_bg, gcc);

   mi = e_menu_item_new(mn);
   snprintf(buf, sizeof(buf), "%s %s",
            _("On top pressing"), _get_bind_text("gadman_toggle"));
   e_menu_item_label_set(mi, buf);
   e_menu_item_radio_set(mi, 1);
   e_menu_item_radio_group_set(mi, 2);
   if (gcc->gadcon == Man->gc_top)
     e_menu_item_toggle_set(mi, 1);
   e_menu_item_callback_set(mi, on_menu_layer_top, gcc);

   mi = e_menu_item_new(menu);
   e_menu_item_label_set(mi, _("Behavior"));
   e_util_menu_item_edje_icon_set(mi, "enlightenment/appearance");
   e_menu_item_submenu_set(mi, mn);
   e_object_del(E_OBJECT(mn));

   /* Move / resize*/
   mi = e_menu_item_new(menu);
   e_menu_item_label_set(mi, _("Begin move/resize this gadget"));
   e_menu_item_icon_edje_set(mi, Man->icon_name, "move_icon");
   e_menu_item_callback_set(mi, on_menu_edit, gcc);

   /* Remove this gadgets */
   mi = e_menu_item_new(menu);
   e_menu_item_label_set(mi, _("Remove this gadget"));
   e_menu_item_callback_set(mi, on_menu_delete, gcc);

   /* Add other gadgets */
   mi = e_menu_item_new(menu);
   e_menu_item_label_set(mi, _("Add other gadgets"));
   e_menu_item_icon_edje_set(mi, Man->icon_name, "icon");
   e_menu_item_callback_set(mi, on_menu_add, gcc);
}

static char *
_get_bind_text(const char* action)
{
   E_Binding_Key *bind;
   char b[256] = "";

   bind = e_bindings_key_get(action);   
   if ((bind) && (bind->key))
     {
        if ((bind->mod) & (E_BINDING_MODIFIER_CTRL))
          strcat(b, _("CTRL"));

        if ((bind->mod) & (E_BINDING_MODIFIER_ALT))
          {
             if (b[0]) strcat(b, " + ");
             strcat(b, _("ALT"));
          }

        if ((bind->mod) & (E_BINDING_MODIFIER_SHIFT))
          {
             if (b[0]) strcat(b, " + ");
             strcat(b, _("SHIFT"));
          }

        if ((bind->mod) & (E_BINDING_MODIFIER_WIN))
          {
             if (b[0]) strcat(b, " + ");
             strcat(b, _("WIN"));
          }

        if ((bind->key) && (bind->key[0]))
          {
             char *l;

             if (b[0]) strcat(b, " + ");
             l = strdup(bind->key);
             l[0] = (char)toupper(bind->key[0]);
             strcat(b, l);
             free(l);
          }
        return strdup(b);
     }
   return "(You must define a binding)";
}

static void
_hide_finished(void *data, Evas_Object *o, const char *em, const char *src)
{
   ecore_evas_hide(Man->top_ee);
}

/* Callbacks */
static void
on_shape_change(void *data, E_Container_Shape *es, E_Container_Shape_Change ch)
{
   Evas_List *l = NULL;
   E_Container  *con;

   con = e_container_shape_container_get(es);
   if ((con->w == Man->width) && (con->h == Man->height)) return;

   /* The screen size is changed */
   Man->width = con->w;
   Man->height = con->h;

   /* ReStart gadgets */
   e_gadcon_unpopulate(Man->gc);
   e_gadcon_unpopulate(Man->gc_top);
   for (l = Man->gc->cf->clients; l; l = l->next)
     {
        E_Config_Gadcon_Client *cf_gcc;

        if (!(cf_gcc = l->data)) continue;
        gadman_gadget_place(cf_gcc, 0);
     }

   for (l = Man->gc_top->cf->clients; l; l = l->next)
     {
        E_Config_Gadcon_Client *cf_gcc;

        if (!(cf_gcc = l->data)) continue;
        gadman_gadget_place(cf_gcc, 1);
     }
}

static void
on_menu_style_plain(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Gadcon_Client *gcc;

   gcc = current;
   if (gcc->style) evas_stringshare_del(gcc->style);
   gcc->style = evas_stringshare_add(E_GADCON_CLIENT_STYLE_PLAIN);

   if (gcc->cf->style) evas_stringshare_del(gcc->cf->style);
   gcc->cf->style = evas_stringshare_add(E_GADCON_CLIENT_STYLE_PLAIN);

   edje_object_signal_emit(gcc->o_frame, "e,state,visibility,plain", "e");

   e_config_save_queue();
}

static void
on_menu_style_inset(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Gadcon_Client *gcc;

   gcc = current;
//   printf("Inset (%s)\n", gcc->name);

   if (gcc->style) evas_stringshare_del(gcc->style);
   gcc->style = evas_stringshare_add(E_GADCON_CLIENT_STYLE_INSET);

   if (gcc->cf->style) evas_stringshare_del(gcc->cf->style);
   gcc->cf->style = evas_stringshare_add(E_GADCON_CLIENT_STYLE_INSET);

   edje_object_signal_emit(gcc->o_frame, "e,state,visibility,inset", "e");

   e_config_save_queue();
}

static void
on_menu_layer_bg(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Config_Gadcon_Client *cf;

   if (!current) return;
   cf = current->cf;

   gadman_gadget_remove(current);
   current = gadman_gadget_place(cf, 0);

   Man->gc_top->cf->clients = evas_list_remove(Man->gc_top->cf->clients, cf);
   Man->gc->cf->clients = evas_list_append(Man->gc->cf->clients, cf);

   e_config_save_queue();
}

static void
on_menu_layer_top(void *data, E_Menu *m, E_Menu_Item *mi)
{
   E_Config_Gadcon_Client *cf;

   if (!current) return;
   cf = current->cf;

   gadman_gadget_remove(current);
   current = gadman_gadget_place(cf, 1);

   Man->gc->cf->clients = evas_list_remove(Man->gc->cf->clients, cf);
   Man->gc_top->cf->clients = evas_list_append(Man->gc_top->cf->clients, cf);

   e_config_save_queue();

   gadman_gadgets_show();
}

static void
on_menu_edit(void *data, E_Menu *m, E_Menu_Item *mi)
{
   gadman_gadget_edit_start(data);
}

static void
on_menu_add(void *data, E_Menu *m, E_Menu_Item *mi)
{
   if (Man->visible)
     gadman_gadgets_hide();
   e_configure_registry_call("extensions/gadman", m->zone->container, NULL);
}

static void
on_menu_delete(void *data, E_Menu *m, E_Menu_Item *mi)
{
   gadman_gadget_del(data);
   e_config_save_queue();
}

static void
on_frame_click(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Down *ev;
   E_Gadcon_Client *gcc;

   ev = event_info;

   if (Man->gc->editing) gadman_gadget_edit_end();

   gcc = data;
   current = gcc;

   if (ev->button == 5)
     {
	E_Menu *mn;
	int cx, cy, cw, ch;

	mn = e_menu_new();
	//e_menu_post_deactivate_callback_set(mn, _e_gadcon_client_cb_menu_post,
	//				    gcc);
	gcc->menu = mn;
	e_gadcon_client_util_menu_items_append(gcc, mn, 0);
	e_gadcon_canvas_zone_geometry_get(gcc->gadcon, &cx, &cy, &cw, &ch);
	e_menu_activate_mouse(mn,
			      e_util_zone_current_get(e_manager_current_get()),
			      cx + ev->output.x, cy + ev->output.y, 1, 1,
			      E_MENU_POP_DIRECTION_DOWN, ev->timestamp);
	e_util_evas_fake_mouse_up_later(gcc->gadcon->evas,
					ev->button);
     }
}

static void
on_top(void *data, Evas_Object *o, const char *em, const char *src)
{
   static int ox, oy, ow, oh; //Object coord
   int mx, my;                //Mouse coord
   int action = (int)data;
   Evas_Object *mover;

   mover = _get_mover(current);

   if (action == DRAG_START)
     {
        evas_pointer_output_xy_get(current->gadcon->evas, &mx, &my);
        evas_object_geometry_get(mover, &ox, &oy, &ow, &oh);
        edje_object_signal_callback_add(o, "mouse,move", "v1",
                                        on_top,(void*)DRAG_MOVE);
     }
   else if (action == DRAG_STOP)
     {
        edje_object_signal_callback_del(o, "mouse,move", "v1", on_top);
        _save_widget_position(current);
     }
   else if (action == DRAG_MOVE)
     {
        int w, h;

        evas_pointer_output_xy_get(current->gadcon->evas, &mx, &my);

        h = (oy + oh) - my - 15;
        if (h < current->min.h) h = current->min.h;

        evas_object_resize(mover, ow, h);
        evas_object_move(mover, ox, my + 15);

        evas_object_resize(current->o_frame, ow, h);
        evas_object_move(current->o_frame, ox, my + 15);
     }
}

static void
on_right(void *data, Evas_Object *o, const char *em, const char *src)
{
   Evas_Object *mover;
   static int ox, oy, ow, oh; //Object coord
   int mx, my;                //Mouse coord
   int action;

   mover = _get_mover(current);

   action = (int)data;
   if (action == DRAG_START)
     {
        evas_pointer_output_xy_get(current->gadcon->evas, &mx, &my);
        evas_object_geometry_get(mover, &ox, &oy, &ow, &oh);
        edje_object_signal_callback_add(o, "mouse,move", "h2",
                                        on_right,(void*)DRAG_MOVE);
     }
   else if (action == DRAG_STOP)
     {
        edje_object_signal_callback_del(o, "mouse,move", "h2", on_right);
        _save_widget_position(current);
     }
   else if (action == DRAG_MOVE)
     {
        int w, h;

        evas_pointer_output_xy_get(current->gadcon->evas, &mx, &my);

        w = mx - ox - 15;
        if (w < current->min.w) w = current->min.w;

        evas_object_resize(mover, w, oh);
        evas_object_resize(current->o_frame, w, oh);
     }
}

static void
on_down(void *data, Evas_Object *o, const char *em, const char *src)
{
   Evas_Object *mover;
   static int ox, oy, ow, oh; //Object coord
   int mx, my;                //Mouse coord
   int action;

   action = (int)data;
   mover = _get_mover(current);

   if (action == DRAG_START)
     {
        evas_pointer_output_xy_get(current->gadcon->evas, &mx, &my);
        evas_object_geometry_get(mover, &ox, &oy, &ow, &oh);
        edje_object_signal_callback_add(o, "mouse,move", "v2",
                                        on_down,(void*)DRAG_MOVE);
     }
   else if (action == DRAG_STOP)
     {
        edje_object_signal_callback_del(o, "mouse,move", "v2", on_down);
        _save_widget_position(current);
     }
   else if (action == DRAG_MOVE)
     {
        int w, h;

        evas_pointer_output_xy_get(current->gadcon->evas, &mx, &my);
        h = my - oy - 15;
        if (h < current->min.h) h = current->min.h;

        evas_object_resize(mover, ow, h);
        evas_object_resize(current->o_frame, ow, h);
     }
}

static void
on_left(void *data, Evas_Object *o, const char *em, const char *src)
{
   Evas_Object *mover;
   static int ox, oy, ow, oh; //Object coord
   int mx, my;                //Mouse coord
   int action;

   action = (int)data;
   mover = _get_mover(current);

   if (action == DRAG_START)
     {
        evas_pointer_output_xy_get(current->gadcon->evas, &mx, &my);
        evas_object_geometry_get(mover, &ox, &oy, &ow, &oh);
        edje_object_signal_callback_add(o, "mouse,move", "h1",
                                        on_left,(void*)DRAG_MOVE);
     }
   else if (action == DRAG_STOP)
     {
        edje_object_signal_callback_del(o, "mouse,move", "h1", on_left);
        _save_widget_position(current);
     }
   else if (action == DRAG_MOVE)
     {
        int w, h;

        evas_pointer_output_xy_get(current->gadcon->evas, &mx, &my);

        w = (ox + ow) - mx - 15;
        if (w < current->min.w) w = current->min.w;

        evas_object_move(mover, mx + 15, oy);
        evas_object_resize(mover, w, oh);

        evas_object_move(current->o_frame, mx + 15, oy);
        evas_object_resize(current->o_frame, w, oh);
     }
}

static void
on_move(void *data, Evas_Object *o, const char *em, const char *src)
{
   Evas_Object *mover;
   static int dx, dy;  //Offset of mouse pointer inside the mover
   static int ox, oy;  //Starting object position
   static int ow, oh;  //Starting object size
   int mx, my;         //Mouse coord
   int action;

   action = (int)data;
   mover = _get_mover(current);

   /* DRAG_START */
   if (action == DRAG_START)
     {
        evas_pointer_output_xy_get(current->gadcon->evas, &mx, &my);
        evas_object_geometry_get(mover, &ox, &oy, &ow, &oh);

        dx = mx - ox;
        dy = my - oy;

        edje_object_signal_callback_add(o, "mouse,move", "overlay",
                                        on_move,(void*)DRAG_MOVE);
        return;
     }

   /* DRAG_STOP */
   if (action == DRAG_STOP)
     {
        edje_object_signal_callback_del(o, "mouse,move", "overlay", on_move);
        dx = dy = 0;
        _save_widget_position(current);
        return;
     }

   /* DRAG_MOVE */
   if (action == DRAG_MOVE)
     {
        int x, y;

        evas_pointer_output_xy_get(current->gadcon->evas, &mx, &my);

        x = mx - dx;
        y = my - dy;

        /* don't go out of the screen */
        if (x < 0) x = 0;
        if (x > (Man->width - ow)) x = Man->width - ow;
        if (y < 0) y = 0;
        if (y > (Man->height - oh)) y = Man->height - oh;

        evas_object_move(current->o_frame, x , y);
        evas_object_move(mover, x, y);
        evas_object_raise(current->o_frame);
        evas_object_raise(mover);
     }
}

static void
on_bg_click(void *data, Evas_Object *o, const char *em, const char *src)
{
   gadman_gadgets_hide();
}
