/*
 * vim:ts=8:sw=3:sts=8:et:cino=>5n-3f0^-2{2t0(0W4
 */
#ifndef LIST_H
#define LIST_H

#include "../../Drawer.h"

EAPI extern Drawer_Plugin_Api drawer_plugin_api;

EAPI void *drawer_plugin_init(Drawer_Plugin *p, const char *id);
EAPI int   drawer_plugin_shutdown(Drawer_Plugin *p);
EAPI Evas_Object * drawer_view_render(Drawer_View *v, Evas *evas, Eina_List *items);

EAPI void  drawer_view_content_size_get(Drawer_View *v, E_Gadcon_Client *gcc, Drawer_Content_Margin *margin, int *w, int *h);
EAPI void drawer_view_orient_set(Drawer_View *v, E_Gadcon_Orient orient);

#endif
