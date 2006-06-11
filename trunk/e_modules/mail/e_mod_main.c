/*
 * Copyright (C) 2006 Christopher Michael
 *
 * Portions of this code Copyright (C) 2004 Embrace project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include <e.h>
#include "e_mod_main.h"
#include "imap.h"
#include "pop.h"
#include "mdir.h"
#include "mbox.h"

typedef struct _Instance Instance;
typedef struct _Mail Mail;

struct _Instance 
{
   E_Gadcon_Client *gcc;
   Evas_Object *mail_obj;
   Mail *mail;
   Ecore_Exe *exe;
   Ecore_Timer *check_timer;
};

struct _Mail
{
   Instance *inst;
   Evas_Object *mail_obj;
};

/* Func Protos for Gadcon */
static E_Gadcon_Client *_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style);
static void _gc_shutdown(E_Gadcon_Client *gcc);
static void _gc_orient(E_Gadcon_Client *gcc);
static char *_gc_label(void);
static Evas_Object *_gc_icon(Evas *evas);

/* Func Protos for Module */
static void _mail_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _mail_cb_mouse_in(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _mail_cb_mouse_out(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _mail_menu_cb_configure(void *data, E_Menu *m, E_Menu_Item *mi);
static void _mail_menu_cb_post(void *data, E_Menu *m);
static void _mail_menu_cb_exec(void *data, E_Menu *m, E_Menu_Item *mi);
static Config_Item *_mail_config_item_get(const char *id);
static Mail *_mail_new(Evas *evas);
static void _mail_free(Mail *mail);
static int _mail_cb_check(void *data);
static int _mail_cb_exe_exit(void *data, int type, void *event);

static E_Config_DD *conf_edd = NULL;
static E_Config_DD *conf_item_edd = NULL;
static E_Config_DD *conf_box_edd = NULL;

Config *mail_config = NULL;

static Ecore_Event_Handler *exit_handler;

static const E_Gadcon_Client_Class _gc_class = 
{
   GADCON_CLIENT_CLASS_VERSION,
     "mail", {_gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon}
};

static E_Gadcon_Client *
_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style) 
{
   E_Gadcon_Client *gcc;
   Instance *inst;
   Mail *mail;
   Config_Item *ci;
   Evas_List *l, *j;
   
   inst = E_NEW(Instance, 1);
   ci = _mail_config_item_get(id);
   if (!ci->id) ci->id = evas_stringshare_add(id);
   
   mail = _mail_new(gc->evas);
   mail->inst = inst;
   inst->mail = mail;
   
   gcc = e_gadcon_client_new(gc, name, id, style, mail->mail_obj);
   gcc->data = inst;
   inst->gcc = gcc;
   inst->mail_obj = mail->mail_obj;

   evas_object_event_callback_add(inst->mail_obj, EVAS_CALLBACK_MOUSE_DOWN, _mail_cb_mouse_down, inst);
   evas_object_event_callback_add(inst->mail_obj, EVAS_CALLBACK_MOUSE_IN, _mail_cb_mouse_in, inst);
   evas_object_event_callback_add(inst->mail_obj, EVAS_CALLBACK_MOUSE_OUT, _mail_cb_mouse_out, inst);

   if (ci->show_label)
     edje_object_signal_emit(inst->mail_obj, "label_active", "");
   else
     edje_object_signal_emit(inst->mail_obj, "label_passive", "");

   mail_config->instances = evas_list_append(mail_config->instances, inst);
   for (l = mail_config->items; l; l = l->next) 
     {
	Config_Item *ci;
	
	ci = l->data;
	for (j = ci->boxes; j; j = j->next) 
	  {
	     Config_Box *cb;
	     
	     cb = j->data;
	     switch (cb->type) 
	       {
		case MAIL_TYPE_IMAP:
		  _mail_imap_add_mailbox(cb);
		  if (!inst->check_timer)
		    inst->check_timer = ecore_timer_add((ci->check_time * 60.0), _mail_cb_check, inst);
		  break;
		case MAIL_TYPE_POP:
		  if (!inst->check_timer)
		    inst->check_timer = ecore_timer_add((ci->check_time * 60.0), _mail_cb_check, inst);
		  break;
		case MAIL_TYPE_MDIR:
		  _mail_mdir_add_mailbox(inst, cb);
		  break;
		case MAIL_TYPE_MBOX:
		  _mail_mbox_add_mailbox(inst, cb);
		  break;
	       }
	  }	
     }
   return gcc;
}

static void
_gc_shutdown(E_Gadcon_Client *gcc) 
{
   Instance *inst;
   
   inst = gcc->data;
   
   evas_object_event_callback_del(inst->mail_obj, EVAS_CALLBACK_MOUSE_DOWN, _mail_cb_mouse_down);
   evas_object_event_callback_del(inst->mail_obj, EVAS_CALLBACK_MOUSE_IN, _mail_cb_mouse_in);
   evas_object_event_callback_del(inst->mail_obj, EVAS_CALLBACK_MOUSE_OUT, _mail_cb_mouse_out);
   
   mail_config->instances = evas_list_remove(mail_config->instances, inst);
   _mail_free(inst->mail);
   free(inst);
}

static void
_gc_orient(E_Gadcon_Client *gcc) 
{
   e_gadcon_client_aspect_set(gcc, 16, 16);
   e_gadcon_client_min_size_set(gcc, 16, 16);
}

static char *
_gc_label(void) 
{
   return D_("Mail");
}

static Evas_Object *
_gc_icon(Evas *evas) 
{
   Evas_Object *o;
   char buf[4096];
   
   o = edje_object_add(evas);
   snprintf(buf, sizeof(buf), "%s/module.eap", e_module_dir_get(mail_config->module));
   edje_object_file_set(o, buf, "icon");
   return o;
}

static void
_mail_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info) 
{
   Instance *inst = data;
   Config_Item *ci;
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_List *l;
   
   if (!inst) return;
   if ((ev->button == 3) && (!mail_config->menu))
     {
	E_Menu *mn, *sn;
	E_Menu_Item *mi;
	int x, y, w, h;
	char buf[1024];
	
	mn = e_menu_new();
	e_menu_post_deactivate_callback_set(mn, _mail_menu_cb_post, inst);
	mail_config->menu = mn;

	ci = _mail_config_item_get(inst->gcc->id);
	if ((ci->boxes) && (evas_list_count(ci->boxes) > 0)) 
	  {
	     E_Menu_Item *mm;
	     
	     snprintf(buf, sizeof(buf), "%s/module.eap", e_module_dir_get(mail_config->module));
	     mm = e_menu_item_new(mn);
	     e_menu_item_label_set(mm, _("Mailboxes"));
	     e_menu_item_icon_edje_set(mm, buf, "icon");
	     
	     sn = e_menu_new();
	     for (l = ci->boxes; l; l = l->next) 
	       {
		  Config_Box *cb;
		  
		  cb = l->data;
		  if (!cb) continue;
		  mi = e_menu_item_new(sn);
		  snprintf(buf, sizeof(buf), "%s: %d/%d", cb->name, cb->num_new, cb->num_total);
		  e_menu_item_label_set(mi, buf);
		  if ((cb->exec) && (cb->use_exec))
		    e_menu_item_callback_set(mi, _mail_menu_cb_exec, cb);
	       }
	     e_menu_item_submenu_set(mm, sn);
	     mi = e_menu_item_new(mn);
	     e_menu_item_separator_set(mi, 1);
	  }
	
	mi = e_menu_item_new(mn);
	e_menu_item_label_set(mi, _("Configuration"));
	e_util_menu_item_edje_icon_set(mi, "enlightenment/configuration");
	e_menu_item_callback_set(mi, _mail_menu_cb_configure, inst);
	
	mi = e_menu_item_new(mn);
	e_menu_item_separator_set(mi, 1);
	
	e_gadcon_client_util_menu_items_append(inst->gcc, mn, 0);
	e_gadcon_canvas_zone_geometry_get(inst->gcc->gadcon, &x, &y, &w, &h);
	e_menu_activate_mouse(mn,
			      e_util_zone_current_get(e_manager_current_get()),
			      x + ev->output.x, y + ev->output.y, 1, 1, E_MENU_POP_DIRECTION_DOWN,
			      ev->timestamp);
	evas_event_feed_mouse_up(inst->gcc->gadcon->evas, ev->button, EVAS_BUTTON_NONE, ev->timestamp, NULL);
     }
   else if (ev->button == 1) 
     _mail_cb_check(inst);	
}

static void 
_mail_cb_mouse_in(void *data, Evas *e, Evas_Object *obj, void *event_info) 
{
   Instance *inst = data;
   
   if (!inst) return;
   edje_object_signal_emit(inst->mail_obj, "label_active", "");
}

static void 
_mail_cb_mouse_out(void *data, Evas *e, Evas_Object *obj, void *event_info) 
{
   Instance *inst = data;
   Config_Item *ci;
   
   if (!inst) return;
   ci = _mail_config_item_get(inst->gcc->id);
   if (!ci->show_label)
     edje_object_signal_emit(inst->mail_obj, "label_passive", "");
}

static void
_mail_menu_cb_post(void *data, E_Menu *m) 
{
   if (!mail_config->menu) return;
   e_object_del(E_OBJECT(mail_config->menu));
   mail_config->menu = NULL;
}

static void
_mail_menu_cb_configure(void *data, E_Menu *m, E_Menu_Item *mi) 
{
   Instance *inst = data;
   Config_Item *ci;
   
   if (!inst) return;
   ci = _mail_config_item_get(inst->gcc->id);
   _config_mail_module(ci);
}

static Config_Item *
_mail_config_item_get(const char *id) 
{
   Evas_List *l;
   Config_Item *ci;
   
   for (l = mail_config->items; l; l = l->next) 
     {
	ci = l->data;
	if (!ci->id) continue;
	if (!strcmp(ci->id, id)) return ci;
     }
   
   ci = E_NEW(Config_Item, 1);
   ci->id = evas_stringshare_add(id);
   ci->show_label = 1;
   ci->check_time = 15.0;
   ci->boxes = NULL;
   
   mail_config->items = evas_list_append(mail_config->items, ci);
   return ci;
}

EAPI E_Module_Api e_modapi = 
{
   E_MODULE_API_VERSION,
     "Mail"
};

EAPI void *
e_modapi_init(E_Module *m) 
{
   conf_box_edd = E_CONFIG_DD_NEW("Mail_Box_Config", Config_Box);
   #undef T
   #undef D
   #define T Config_Box
   #define D conf_box_edd
   E_CONFIG_VAL(D, T, name, STR);
   E_CONFIG_VAL(D, T, type, INT);
   E_CONFIG_VAL(D, T, port, INT);
   E_CONFIG_VAL(D, T, ssl, UCHAR);
   E_CONFIG_VAL(D, T, host, STR);
   E_CONFIG_VAL(D, T, user, STR);
   E_CONFIG_VAL(D, T, pass, STR);
   E_CONFIG_VAL(D, T, new_path, STR);
   E_CONFIG_VAL(D, T, cur_path, STR);
   E_CONFIG_VAL(D, T, use_exec, UCHAR);
   E_CONFIG_VAL(D, T, exec, STR);
   
   conf_item_edd = E_CONFIG_DD_NEW("Mail_Config_Item", Config_Item);
   #undef T
   #undef D
   #define T Config_Item
   #define D conf_item_edd
   E_CONFIG_VAL(D, T, id, STR);
   E_CONFIG_VAL(D, T, show_label, UCHAR); 
   E_CONFIG_VAL(D, T, check_time, DOUBLE);
   E_CONFIG_LIST(D, T, boxes, conf_box_edd);
   
   conf_edd = E_CONFIG_DD_NEW("Mail_Config", Config);
   #undef T
   #undef D
   #define T Config
   #define D conf_edd
   E_CONFIG_LIST(D, T, items, conf_item_edd);
   
   mail_config = e_config_domain_load("module.mail", conf_edd);
   if (!mail_config) 
     {
	Config_Item *ci;
	
	mail_config = E_NEW(Config, 1);
	ci = E_NEW(Config_Item, 1);
	
	ci->id = evas_stringshare_add("0");
	ci->show_label = 1;
	ci->check_time = 15.0;
	ci->boxes = NULL;
	
	mail_config->items = evas_list_append(mail_config->items, ci);
     }
   mail_config->module = m;
   e_gadcon_provider_register(&_gc_class);
   return 1;
}

EAPI int
e_modapi_shutdown(E_Module *m) 
{
   mail_config->module = NULL;
   e_gadcon_provider_unregister(&_gc_class);

   if (exit_handler)
     ecore_event_handler_del(exit_handler);
   
   if (mail_config->config_dialog)
     e_object_del(E_OBJECT(mail_config->config_dialog));
   if (mail_config->menu) 
     {
	e_menu_post_deactivate_callback_set(mail_config->menu, NULL, NULL);
	e_object_del(E_OBJECT(mail_config->menu));
	mail_config->menu = NULL;
     }
   while (mail_config->items) 
     {
	Config_Item *ci;
	
	ci = mail_config->items->data;
	while (ci->boxes) 
	  {
	     Config_Box *cb;
	     
	     cb = ci->boxes->data;
	     switch (cb->type) 
	       {
		case MAIL_TYPE_IMAP:
		  _mail_imap_del_mailbox(cb);
		  break;
		case MAIL_TYPE_POP:
		  break;
		case MAIL_TYPE_MDIR:
		  _mail_mdir_del_mailbox(cb);
		  break;
		case MAIL_TYPE_MBOX:
		  _mail_mbox_del_mailbox(cb);
		  break;
	       }
	     if (cb->name) evas_stringshare_del(cb->name);
	     if (cb->host) evas_stringshare_del(cb->host);
	     if (cb->user) evas_stringshare_del(cb->user);
	     if (cb->pass) evas_stringshare_del(cb->pass);
	     if (cb->new_path) evas_stringshare_del(cb->new_path);
	     if (cb->cur_path) evas_stringshare_del(cb->cur_path);
	     if (cb->exec) evas_stringshare_del(cb->exec);
	     ci->boxes = evas_list_remove_list(ci->boxes, ci->boxes);
	     free(cb);
	  }
	if (ci->id) evas_stringshare_del(ci->id);
	mail_config->items = evas_list_remove_list(mail_config->items, mail_config->items);
	free(ci);
     }
   free(mail_config);
   mail_config = NULL;
   E_CONFIG_DD_FREE(conf_box_edd);
   E_CONFIG_DD_FREE(conf_item_edd);
   E_CONFIG_DD_FREE(conf_edd);
   return 1;
}

EAPI int
e_modapi_save(E_Module *m) 
{
   Evas_List *l;
   
   for (l = mail_config->instances; l; l = l->next) 
     {
	Instance *inst;
	Config_Item *ci;
	
	inst = l->data;
	ci = _mail_config_item_get(inst->gcc->id);
	if (ci->id) evas_stringshare_del(ci->id);
	ci->id = evas_stringshare_add(inst->gcc->id);
     }
   e_config_domain_save("module.mail", conf_edd, mail_config);
   return 1;
}

EAPI int
e_modapi_about(E_Module *m) 
{
   e_module_dialog_show(D_("Enlightenment Mail Module"), 
			D_("This is a module to notify when you have new mail."));
   return 1;
}

static Mail *
_mail_new(Evas *evas)
{
   Mail *mail;
   char buf[4096];

   mail = E_NEW(Mail, 1);
   mail->mail_obj = edje_object_add(evas);

   snprintf(buf, sizeof(buf), "%s/mail.edj", e_module_dir_get(mail_config->module));
   if (!e_theme_edje_object_set(mail->mail_obj, "base/theme/modules/mail", "modules/mail/main"))
     edje_object_file_set(mail->mail_obj, buf, "modules/mail/main");
   evas_object_show(mail->mail_obj);
   
   return mail;
}

static void
_mail_free(Mail *mail) 
{
   evas_object_del(mail->mail_obj);
   free(mail);
}

static int 
_mail_cb_check(void *data) 
{
   Instance *inst = data;
   Config_Item *ci;
   Evas_List *l;
   int have_imap = 0;
   
   if (!inst) return 1;
   ci = _mail_config_item_get(inst->gcc->id);
   if (!ci->boxes) return 1;
   for (l = ci->boxes; l; l = l->next) 
     {
	Config_Box *cb;
	
	cb = l->data;
	switch (cb->type) 
	  {
	   case MAIL_TYPE_MDIR:
	     break;
	   case MAIL_TYPE_MBOX:
	     break;
	   case MAIL_TYPE_POP:
	     _mail_pop_check_mail(inst, cb);
	     break;
	   case MAIL_TYPE_IMAP:
	     have_imap = 1;
	     break;
	  }
     }
   if (have_imap) _mail_imap_check_mail(inst);
   return 1;
}

void 
_mail_set_text(void *data, int count) 
{
   Instance *inst = data;
   char buf[1024];
   
   if (!inst) return;
   
   if (count > 0) 
     {
	snprintf(buf, sizeof(buf), "%d", count);
	edje_object_part_text_set(inst->mail->mail_obj, "new_label", buf);
	edje_object_signal_emit(inst->mail->mail_obj, "new_mail", "");
     }
   else 
     {
	edje_object_signal_emit(inst->mail->mail_obj, "no_mail", "");
	edje_object_part_text_set(inst->mail->mail_obj, "new_label", "");
     }
}

static int 
_mail_cb_exe_exit(void *data, int type, void *event) 
{
   Config_Box *cb;
   
   cb = data;
   if (!cb) return;
   cb->exe = NULL;
   ecore_event_handler_del(exit_handler);
}

void
_mail_box_added(const char *ci_name, const char *box_name) 
{
   Evas_List *l, *b;
   
   for (l = mail_config->instances; l; l = l->next) 
     {
	Instance *inst;
	Config_Item *ci;
	
	inst = l->data;
	ci = _mail_config_item_get(inst->gcc->id);
	if ((ci->id) && (!strcmp(ci->id, ci_name))) 
	  {
	     for (b = ci->boxes; b; b = b->next) 
	       {
		  Config_Box *cb;
		  
		  cb = b->data;
		  if ((cb->name) && (!strcmp(cb->name, box_name))) 
		    {
		       switch (cb->type) 
			 {
			  case MAIL_TYPE_IMAP:
			    _mail_imap_add_mailbox(cb);
			    break;
			  case MAIL_TYPE_POP:
			    break;
			  case MAIL_TYPE_MDIR:
			    _mail_mdir_add_mailbox(inst, cb);
			    break;
			  case MAIL_TYPE_MBOX:
			    _mail_mbox_add_mailbox(inst, cb);
			    break;
			 }
		       break;
		    }
	       }
	     break;
	  }
     }
}

void 
_mail_box_deleted(const char *ci_name, const char *box_name)
{
   Evas_List *l, *d, *i;
   Config_Box *cb;
   int found = 0;
   
   for (i = mail_config->instances; i; i = i->next) 
     {
	Instance *inst;
	
	inst = i->data;
	if (!inst->gcc->id) continue;
	if (!strcmp(inst->gcc->id, ci_name)) 
	  {
	     Config_Item *ci;
	     
	     ci = _mail_config_item_get(inst->gcc->id);
	     for (d = ci->boxes; d; d = d->next) 
	       {
		  cb = d->data;
		  if ((cb->name) && (box_name)) 
		    {
		       if (!strcmp(cb->name, box_name)) 
			 {
			    found = 1;
			    break;
			 }
		    }
	       }
	     if (found) 
	       {
		  switch (cb->type) 
		    {
		     case MAIL_TYPE_IMAP:
		       _mail_imap_del_mailbox(cb);
		       break;
		     case MAIL_TYPE_POP:
		       break;
		     case MAIL_TYPE_MDIR:
		       _mail_mdir_del_mailbox(cb);
		       break;
		     case MAIL_TYPE_MBOX:
		       _mail_mbox_del_mailbox(cb);
		       break;
		    }
		  ci->boxes = evas_list_remove(ci->boxes, cb);
		  e_config_save_queue();
		  break;
	       }
	     break;
	  }
     }
}

void 
_mail_config_updated(const char *id) 
{
   Evas_List *l;
   Config_Item *ci;
   
   if (!mail_config) return;

   ci = _mail_config_item_get(id);
   for (l = mail_config->instances; l; l = l->next) 
     {
	Instance *inst;
	
	inst = l->data;
	if (!inst->gcc->id) continue;
	if (!strcmp(inst->gcc->id, ci->id)) 
	  {
	     if (inst->check_timer) 
	       {
		  ecore_timer_del(inst->check_timer);
		  inst->check_timer = ecore_timer_add((ci->check_time * 60.0), _mail_cb_check, inst);
	       }
	     
	     if (ci->show_label)
	       edje_object_signal_emit(inst->mail_obj, "label_active", "");
	     else
	       edje_object_signal_emit(inst->mail_obj, "label_passive", "");
	     break;
	  }
     }
}

static void 
_mail_menu_cb_exec(void *data, E_Menu *m, E_Menu_Item *mi) 
{
   Config_Box *cb;
   
   cb = data;
   if (!cb) return;
   
   exit_handler = ecore_event_handler_add(ECORE_EXE_EVENT_DEL, _mail_cb_exe_exit, cb);
   cb->exe = ecore_exe_run(cb->exec, cb);
}
