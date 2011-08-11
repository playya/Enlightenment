#include "e.h"
#include "e_mod_main.h"
#include "evry_api.h"

typedef struct _Plugin Plugin;
typedef struct _View View;

struct _Plugin
{
  Evry_Plugin base;
  Ecore_Event_Handler *handle_msg;

  const char *contact;
};

struct _View
{
  Evry_View base;
  Evas_Object *o_text;
  Plugin *plugin;
};

static Evry_Plugin *_plugin;

static int
_cb_key_down(Evry_View *v, const Ecore_Event_Key *ev)
{
   Evas_Object *o;
   double align;
   int h;

   if (!strcmp(ev->key, "Down"))
     {
	o = v->o_list;
	evas_object_geometry_get(o, NULL, NULL, NULL, &h);
	if (!h) h = 1;
	e_box_align_get(o, NULL, &align);

	align = align - 10.0/(double)h;
	if (align < 0.0) align = 0.0;

	e_box_align_set(v->o_list, 0.5, align);

	return 1;
     }
   else if (!strcmp(ev->key, "Up"))
     {
	o = v->o_list;
	evas_object_geometry_get(o, NULL, NULL, NULL, &h);
	if (!h) h = 1;
	e_box_align_get(o, NULL, &align);

	align = align + 10.0/(double)h;
	if (align > 1.0) align = 1.0;

	e_box_align_set(v->o_list, 0.5, align);
	return 1;
     }

   return 0;
}

static void
_view_clear(Evry_View *view)
{
}

static int
_view_update(Evry_View *view)
{
   int mw, mh;
   Eina_List *l;
   Message *msg;
   Eina_Strbuf *buf;

   GET_VIEW(v, view);

   buf = eina_strbuf_new();

   EINA_LIST_FOREACH(messages, l, msg)
     {
	if (msg->contact != v->plugin->contact)
	  continue;

	eina_strbuf_append(buf, "<hilight>");
	if (msg->self)
	  eina_strbuf_append(buf, _("Me"));
	else
	  eina_strbuf_append(buf, msg->contact);
	eina_strbuf_append(buf, "</hilight><br>");
	eina_strbuf_append(buf, msg->msg);
	eina_strbuf_append(buf, "<br><br>");
     }

   const char *text = eina_strbuf_string_get(buf);

   edje_object_part_text_set(v->o_text, "e.textblock.text", text);
   edje_object_size_min_calc(v->o_text, &mw, &mh);
   e_box_pack_options_set(v->o_text, 1, 1, 1, 0, 0.0, 0.0, mw, mh, mw, mh);

   eina_strbuf_free(buf);

   return 1;
}

static Evry_View *
_view_create(Evry_View *view, const Evry_State *s, const Evas_Object *swallow)
{
   Evas_Object *o;

   GET_VIEW(v, view);

   o = e_box_add(evas_object_evas_get(swallow));
   e_box_orientation_set(o, 0);
   e_box_align_set(o, 0.5, 0.0);
   view->o_list = o;
   e_box_freeze(view->o_list);
   o = edje_object_add(evas_object_evas_get(swallow));
   e_theme_edje_object_set(o, "base/theme/widgets",
			   "e/modules/everything/textblock");

   e_box_pack_start(view->o_list, o);
   e_box_thaw(view->o_list);

   evas_object_show(o);
   v->o_text = o;

   return view;
}

static void
_view_destroy(Evry_View *view)
{
   printf("view destroy\n");

   GET_VIEW(v, view);

   evas_object_del(view->o_list);
   evas_object_del(v->o_text);

}

static Evas_Object *
_icon_get(Evry_Item *it, Evas *e)
{
   Evas_Object *o;

   o = edje_object_add(e);
   edje_object_file_set(o, theme_file, "contact_icon");

   return o;
}

static int
_fetch(Evry_Plugin *plugin, const char *input)
{
   Evry_Item *it;
   const char *text = input;;
   
   GET_PLUGIN(p, plugin);

   if (!text) text = "";
   
   if (!p->base.items)
     {
	it = evry->item_new(NULL, EVRY_PLUGIN(p), text, _icon_get, NULL);
	it->fuzzy_match = 999;
	EVRY_PLUGIN_ITEM_APPEND(p, it);
     }
   else
     {
	it = eina_list_data_get(p->base.items);
	EVRY_ITEM_LABEL_SET(it, text);
	/* evry->item_changed(it, 0, 0); */
     }

   return 1;
}

static Eina_Bool
_cb_message_add(void *data, int type __UNUSED__, void *event __UNUSED__)
{
   Evry_View *v = data;

   if (v) v->update(v);

   return ECORE_CALLBACK_PASS_ON;
}

static Evry_Plugin *
_inst_new(Evry_Plugin *plugin, const Evry_Item *it)
{
   Plugin *p;
   View *view;

   GET_ACTION(act, it);
   GET_CONTACT(c, act->it1.item);

   EVRY_PLUGIN_INSTANCE(p, plugin);

   view = E_NEW(View, 1);
   /* view->base.id = view; */
   view->base.name = "Text";
   view->base.create = &_view_create;
   view->base.destroy = &_view_destroy;
   view->base.update = &_view_update;
   view->base.clear = &_view_clear;
   view->base.cb_key_down = &_cb_key_down;

   view->plugin = p;
   p->base.view = EVRY_VIEW(view);

   p->contact = eina_stringshare_ref(c->id);

   p->handle_msg = ecore_event_handler_add(SHOTGUN_EVENT_MESSAGE_ADD,
					   _cb_message_add, view);

   return EVRY_PLUGIN(p);
}

static void
_inst_free(Evry_Plugin *plugin)
{
   GET_PLUGIN(p, plugin);

   EVRY_PLUGIN_ITEMS_FREE(p);

   eina_stringshare_del(p->contact);

   ecore_event_handler_del(p->handle_msg);
   
   E_FREE(plugin->view);
   E_FREE(p);
}

Eina_Bool
evry_plug_msg_init(void)
{
   _plugin = EVRY_PLUGIN_NEW(Evry_Plugin, N_("Shotgun Message"), "folder",
			   SHOTGUN_MESSAGE,
			   _inst_new, _inst_free, _fetch, NULL);

   evry->plugin_register(_plugin, EVRY_PLUGIN_OBJECT, 1);
   _plugin->config->aggregate = EINA_FALSE;

   return EINA_TRUE;
}

void
evry_plug_msg_shutdown(void)
{
   EVRY_PLUGIN_FREE(_plugin);
}
