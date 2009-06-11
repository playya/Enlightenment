/*
 * vim:ts=8:sw=3:sts=8:et:cino=>5n-3f0^-2{20(W4
 */

#include <edje_viewer_main.h>

static void viewer_exit(Viewer *v);
static void edje_object_create(Group *grp);
static void create_group_parts_list(Viewer *v);
static void fill_group_parts_list(Viewer *v);
static void create_signals_box(Viewer *v);
static void toolbar_reconfigure(Group *grp);
static void free_group_parts(Group *grp);
static void typebuf_show(Viewer *v);
static void typebuf_hide(Viewer *v);
static void typebuf_char_append(Viewer *v, const char *ch);
static void typebuf_char_backspace(Viewer *v);
static void typebuf_match(Viewer *v, int next);
static const Group * group_next_find(Viewer *v, int next, char *buf);
int util_glob_case_match(const char *str, const char *glob);
static void show_toggles(Viewer *v);
static void create_toggles_win(Viewer *v);
static void group_activate(Viewer *v);

static void on_win_del_req(void *data, Evas_Object *obj, void *event_info);
static void on_key_down(void *data, Evas *e, Evas_Object *obj, void *event_info);
static int on_typebuf_timer(void *data);
static void on_group_check_changed(void *data, Evas_Object *obj, void *event_info);
static void on_toolbar_changed(void *data, Evas_Object *obj, void *event_info);
static int on_entries_sort(const void *d1, const void *d2);
static void on_parts_list_toggle_change(void *data, Evas_Object *obj, void *event_info);
static void on_signals_toggle_change(void *data, Evas_Object *obj, void *event_info);
static void on_group_part_select(void *data, Evas_Object *obj, void *event_info);
static void on_group_part_unselect(void *data, Evas_Object *obj, void *event_info);
static void on_object_signal(void *data, Evas_Object *o, const char *sig, const char *src);
static void on_object_message(void *data, Evas_Object *obj, Edje_Message_Type type, int id, void *msg);

static Evas_Object * gc_icon_get(const void *data, Evas_Object *obj, const char *part);
static char *gc_label_get(const void *data, Evas_Object *obj, const char *part);
static void gc_del(const void *data, Evas_Object *obj);

void
create_main_win(Viewer *v)
{
   Evas_Object *o;
   char buf[4096];
   
   snprintf(buf, sizeof(buf), "%s/edje_viewer.edj", PACKAGE_DATA_DIR);
   elm_theme_extension_add(buf);
   v->theme_file = eina_stringshare_add(buf);

   o = elm_win_add(NULL, "main", ELM_WIN_BASIC);
   elm_win_title_set(o, "Edje Viewer");
   evas_object_smart_callback_add(o, "delete-request", on_win_del_req, v);
   evas_object_resize(o, 600, 400);
   v->gui.win = o;

   o = elm_bg_add(v->gui.win);
   elm_win_resize_object_add(v->gui.win, o);
   evas_object_size_hint_weight_set(o, 1.0, 1.0);
   evas_object_show(o);

   o = elm_layout_add(o);
   elm_layout_file_set(o, buf, "viewer/main");
   evas_object_event_callback_add(o, EVAS_CALLBACK_KEY_DOWN, on_key_down, v);
   evas_object_size_hint_weight_set(o, 1.0, 1.0);
   elm_win_resize_object_add(v->gui.win, o);
   evas_object_focus_set(o, 1);
   evas_object_show(o);
   v->gui.ly = o;

   o = elm_genlist_add(v->gui.ly);
   evas_object_show(o);
   elm_layout_content_set(v->gui.ly, "v.swallow.tree", o);
   elm_genlist_always_select_mode_set(o, 1);
   evas_object_size_hint_align_set(o, -1.0, -1.0);
   evas_object_size_hint_weight_set(o, 1.0, 1.0);
   evas_object_show(o);
   v->gui.tree = o;

   o = elm_toolbar_add(v->gui.ly);
   elm_object_style_set(o, "viewer");
   elm_layout_content_set(v->gui.ly, "v.swallow.tbar", o);
   evas_object_size_hint_weight_set(o, 0.0, 0.0);
   evas_object_size_hint_align_set(o, -1.0, 0.0);
   evas_object_show(o);
   v->gui.tbar = o;

   v->gc = calloc(1, sizeof(Elm_Genlist_Item_Class));
   v->gc->item_style     = "default";
   v->gc->func.icon_get  = gc_icon_get;
   v->gc->func.label_get = gc_label_get;
   v->gc->func.del       = gc_del;

   if (v->config->show_parts)
     create_group_parts_list(v);
   if (v->config->show_signals)
     create_signals_box(v);

   evas_object_show(v->gui.win);
}

void
open_edje_file(Viewer *v)
{
   Eina_List *entries, *l;
   const char *group;

   elm_genlist_clear(v->gui.tree);

   entries = edje_file_collection_list(v->config->edje_file);
   entries = eina_list_sort(entries, eina_list_count(entries), on_entries_sort);
   EINA_LIST_FOREACH(entries, l, group)
     {
	Group *grp = calloc(1, sizeof(Group));
	grp->v = v;
	grp->name = eina_stringshare_ref(group);

	grp->item = elm_genlist_item_append(
	      v->gui.tree, v->gc, grp, NULL,
	      ELM_GENLIST_ITEM_NONE, NULL, NULL);

	v->groups = eina_inlist_append(v->groups, EINA_INLIST_GET(grp));
     }

   edje_file_collection_list_free(entries);
   elm_win_title_set(v->gui.win, v->config->edje_file);
}

static void
viewer_exit(Viewer *v)
{
   config_save(v, 1);
   elm_exit();
}

static void
edje_object_create(Group *grp)
{
   Evas_Object *o = edje_object_add(evas_object_evas_get(grp->v->gui.win));

   edje_object_file_set(o, grp->v->config->edje_file, grp->name);
   edje_object_signal_callback_add(o, "*", "*", on_object_signal, grp);
   edje_object_message_handler_set(o, on_object_message, grp);
   /*
   edje_object_part_drag_size_set(o, "dragable", 0.01, 0.5);
   edje_object_part_drag_step_set(o, "dragable", 0.1, 0.1);
   edje_object_part_drag_page_set(o, "dragable", 0.2, 0.2);
   */

   grp->obj = o;
}

static void
create_group_parts_list(Viewer *v)
{
   Evas_Object *o;

   v->gui.parts_list = o = elm_list_add(v->gui.ly);
   elm_list_always_select_mode_set(o, 1);
   evas_object_size_hint_weight_set(o, 1.0, 1.0);
   evas_object_size_hint_align_set(o, -1.0, -1.0);
   elm_layout_content_set(v->gui.ly, "v.swallow.parts_list", o);
   evas_object_smart_callback_add(o, "unselected", on_group_part_unselect, NULL);

   evas_object_show(o);
}

static void
fill_group_parts_list(Viewer *v)
{
   Eina_List *l, *parts = edje_edit_parts_list_get(v->visible_group->obj);
   const char *name;

   elm_list_clear(v->gui.parts_list);
   parts = eina_list_sort(parts, eina_list_count(parts), on_entries_sort);
   EINA_LIST_FOREACH(parts, l, name)
     {
	Part *prt = calloc(1, sizeof(Part));
	prt->grp = v->visible_group;
	prt->name = eina_stringshare_ref(name);

	v->visible_group->parts = eina_inlist_append(v->visible_group->parts,
						     EINA_INLIST_GET(prt));

	prt->item = elm_list_item_append(v->gui.parts_list, prt->name, NULL, NULL, on_group_part_select, prt);
     }

   elm_list_go(v->gui.parts_list);

   edje_object_signal_emit(elm_layout_edje_get(v->gui.ly), "v,state,parts_list,show", "v");
}

static void
create_signals_box(Viewer *v)
{
   Evas_Object *o;

   v->gui.sig_box = o = elm_box_add(v->gui.ly);
   elm_box_horizontal_set(o, 0);
   elm_box_homogenous_set(o, 0);
   evas_object_size_hint_weight_set(o, 1.0, 1.0);
   elm_layout_content_set(v->gui.ly, "v.swallow.signals", o);
   evas_object_show(o);

   v->gui.sig_list = o = elm_list_add(v->gui.sig_box);
   evas_object_size_hint_weight_set(o, 1.0, 1.0);
   evas_object_size_hint_align_set(o, -1.0, -1.0);
   elm_box_pack_start(v->gui.sig_box, o);

   evas_object_show(o);

   edje_object_signal_emit(elm_layout_edje_get(v->gui.ly), "v,state,signals,show", "v");
}

static void
toolbar_reconfigure(Group *grp)
{
   if (grp->active)
     {
	grp->ti = elm_toolbar_item_add(grp->v->gui.tbar, NULL, grp->name, on_toolbar_changed, grp);
	elm_toolbar_item_select(grp->ti);
	grp->visible = 1;
	grp->v->visible_group = grp;
     }
   else
     {
	Viewer *v = grp->v;
	elm_toolbar_item_del(grp->ti);
	grp->ti = NULL;

	if (grp->visible)
	  {
	     grp->visible = 0;
	     EINA_INLIST_FOREACH(v->groups, grp)
	       {
		  if (grp->ti)
		    {
		       elm_toolbar_item_select(grp->ti);
		       break;
		    }
	       }
	     if (!grp)
	       {
		  /* XXX: workaround for the elm_toolbar issue.
		   *      Remove when it is fixed */
		  Evas_Coord w, h;
		  evas_object_geometry_get(v->gui.tbar, NULL, NULL, &w, &h);
		  evas_object_resize(v->gui.tbar, w - 1, h - 1);
		  evas_object_resize(v->gui.tbar, w + 1, h + 1);

		  edje_object_signal_emit(elm_layout_edje_get(v->gui.ly), "v,state,parts_list,hide", "v");
	       }
	  }
     }
}

static void
free_group_parts(Group *grp)
{
   while (grp->parts)
     {
	Part *prt = (Part *) grp->parts;
	eina_stringshare_del(prt->name);
	grp->parts = eina_inlist_remove(grp->parts, grp->parts);
	if (prt->highlight)
	  prt->highlight = NULL;
	free(prt);
     }
}

static void
typebuf_show(Viewer *v)
{
   FREE(v->typebuf.buf);
   v->typebuf.buf = strdup("");
   edje_object_part_text_set(elm_layout_edje_get(v->gui.ly), "v.text.typebuf_label", v->typebuf.buf);
   edje_object_signal_emit(elm_layout_edje_get(v->gui.ly), "v,state,typebuf,start", "v");
   v->typebuf.visible = 1;
}

static void
typebuf_hide(Viewer *v)
{
   FREE(v->typebuf.buf);
   edje_object_signal_emit(elm_layout_edje_get(v->gui.ly), "v,state,typebuf,stop", "v");
   v->typebuf.visible = 0;
}

static void
typebuf_char_append(Viewer *v, const char *ch)
{
   char *ts;

   if (!v->typebuf.buf) return;
   ts = malloc(strlen(v->typebuf.buf) + strlen(ch) + 1);
   if (!ts) return;
   strcpy(ts, v->typebuf.buf);
   strcat(ts, ch);
   free(v->typebuf.buf);
   v->typebuf.buf = ts;
   typebuf_match(v, 0);
   edje_object_part_text_set(elm_layout_edje_get(v->gui.ly), "v.text.typebuf_label", v->typebuf.buf);
}

static void
typebuf_char_backspace(Viewer *v)
{
   char *ts;
   int len, p, dec;

   if (!v->typebuf.buf) return;
   len = strlen(v->typebuf.buf);
   if (len == 0)
     {
	typebuf_hide(v);
	return;
     }
   p = evas_string_char_prev_get(v->typebuf.buf, len, &dec);
   if (p >= 0) v->typebuf.buf[p] = 0;
   ts = strdup(v->typebuf.buf);
   if (!ts) return;
   free(v->typebuf.buf);
   v->typebuf.buf = ts;
   typebuf_match(v, 0);
   edje_object_part_text_set(elm_layout_edje_get(v->gui.ly), "v.text.typebuf_label", v->typebuf.buf);
}

static void
typebuf_match(Viewer *v, int next)
{
   const Group *grp, *grp_match;
   char *tb;
   int tblen;

   if (!v->typebuf.buf) return;
   if (!v->groups) return;

   tblen = strlen(v->typebuf.buf);
   tb = malloc(tblen + 3);
   if (!tb) return;
   tb[0] = '*';
   memcpy(tb + 1, v->typebuf.buf, tblen);
   tb[tblen + 1] = '*';
   tb[tblen + 2] = '\0';

   if (!next)
     {
	EINA_INLIST_FOREACH(v->groups, grp)
	  {
	     if (util_glob_case_match(grp->name, tb))
	       {
		  grp_match = grp;
		  break;
	       }
	  }
     }
   else
     grp_match = group_next_find(v, next, tb);

   if (grp_match)
     {
	Elm_Genlist_Item *it = elm_genlist_selected_item_get(v->gui.tree);
	if (it)
	  elm_genlist_item_selected_set(it, 0);
	elm_genlist_item_selected_set(grp_match->item, 1);
	elm_genlist_item_show(grp_match->item);
     }

   free(tb);

   if (v->typebuf.timer)
     ecore_timer_del(v->typebuf.timer);

   v->typebuf.timer = ecore_timer_add(3.0, on_typebuf_timer, v);
}

static const Group *
group_next_find(Viewer *v, int next, char *buf)
{
   const Group *grp, *grp_next = NULL;
   const Eina_Inlist *tmp;

   if (!v->groups) return NULL;

   if (next)
     {
	/* find next item */
	if (next == 1)
	  {
	     Elm_Genlist_Item *it = elm_genlist_selected_item_get(v->gui.tree);
	     grp = elm_genlist_item_data_get(it);
	     tmp = EINA_INLIST_GET(grp)->next;
	     EINA_INLIST_FOREACH(tmp, grp)
	       {
		  if (util_glob_case_match(grp->name, buf))
		    {
		       grp_next = grp;
		       break;
		    }
	       }
	     if (!grp_next)
	       {
		  const Group *cur = elm_genlist_item_data_get(it);
		  EINA_INLIST_FOREACH(v->groups, grp)
		    {
		       if (grp == cur) break;
		       if (util_glob_case_match(grp->name, buf))
			 {
			    grp_next = grp;
			    break;
			 }
		    }
	       }
	  }
	/* find previous item */
	else if (next == -1)
	  {
	     Elm_Genlist_Item *it = elm_genlist_selected_item_get(v->gui.tree);
	     grp = elm_genlist_item_data_get(it);
	     tmp = EINA_INLIST_GET(grp);
	     while (tmp && (tmp = tmp->prev))
	       {
		  grp = (Group *) tmp;
		  if (util_glob_case_match(grp->name, buf))
		    {
		       grp_next = grp;
		       break;
		    }
	       }
	     if (!grp_next)
	       {
		  const Group *cur = elm_genlist_item_data_get(it);
		  EINA_INLIST_REVERSE_FOREACH(v->groups, grp)
		    {
		       if (grp == cur) break;
		       if (util_glob_case_match(grp->name, buf))
			 {
			    grp_next = grp;
			    break;
			 }
		    }
	       }
	  }
     }

   return grp_next;
}
int
util_glob_case_match(const char *str, const char *glob)
{
   const char *p;
   char *tstr, *tglob, *tp;

   if (glob[0] == 0)
     {
	if (str[0] == 0) return 1;
	return 0;
     }
   if (!strcmp(glob, "*")) return 1;
   tstr = alloca(strlen(str) + 1);
   for (tp = tstr, p = str; *p != 0; p++, tp++) *tp = tolower(*p);
   *tp = 0;
   tglob = alloca(strlen(glob) + 1);
   for (tp = tglob, p = glob; *p != 0; p++, tp++) *tp = tolower(*p);
   *tp = 0;
   if (!fnmatch(tglob, tstr, 0)) return 1;
   return 0;
}

static void
show_toggles(Viewer *v)
{
   if (!v->gui.toggles_win)
     create_toggles_win(v);

   if (evas_object_visible_get(v->gui.toggles_win))
     evas_object_hide(v->gui.toggles_win);
   else
     elm_win_inwin_activate(v->gui.toggles_win);
}

static void
create_toggles_win(Viewer *v)
{
   Evas_Object *o, *bx;

   v->gui.toggles_win = o = elm_win_inwin_add(v->gui.win);
   elm_object_style_set(o, "minimal_vertical");

   bx = elm_box_add(v->gui.toggles_win);
   evas_object_show(bx);

   o = elm_toggle_add(bx);
   elm_toggle_label_set(o, "Show parts list");
   elm_toggle_state_set(o, v->config->show_parts);
   elm_box_pack_end(bx, o);
   evas_object_smart_callback_add(o, "changed",
				  on_parts_list_toggle_change, v);
   evas_object_show(o);

   o = elm_toggle_add(bx);
   elm_toggle_label_set(o, "Show signals log");
   elm_toggle_state_set(o, v->config->show_signals);
   elm_box_pack_end(bx, o);
   evas_object_smart_callback_add(o, "changed",
				  on_signals_toggle_change, v);
   evas_object_show(o);

   elm_win_inwin_content_set(v->gui.toggles_win, bx);
}

static void
group_activate(Viewer *v)
{
   Elm_Genlist_Item *it = elm_genlist_selected_item_get(v->gui.tree);
   const Group *grp;
   if (!it) return;
   grp = elm_genlist_item_data_get(it);
   if (!grp || !grp->check) return;
   elm_check_state_set(grp->check, !grp->active);
   on_group_check_changed((void *) grp, grp->check, NULL);
}

/* Callbacks */
static void
on_win_del_req(void *data, Evas_Object *obj, void *event_info)
{
   /* called when my_win_main is requested to be deleted */
   viewer_exit(data); /* exit the program's main loop that runs in elm_run() */
}

static void
on_key_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Viewer *v = data;
   Evas_Event_Key_Down *ev = event_info;

   if (!strcmp(ev->keyname, "F9"))
     show_toggles(v);
   else if (!strcmp(ev->key, "Up"))
     {
	if (v->typebuf.visible)
	  typebuf_match(v, -1);
     }
   else if (!strcmp(ev->key, "Down"))
     {
	if (v->typebuf.visible)
	  typebuf_match(v, 1);
     }
   else if (!strcmp(ev->key, "Return"))
     group_activate(v);
   else if (!evas_key_modifier_is_set(ev->modifiers, "Control") &&
       !evas_key_modifier_is_set(ev->modifiers, "Alt"))
     {
	if (!strcmp(ev->key, "BackSpace"))
	  {
	     if (v->typebuf.visible)
	       typebuf_char_backspace(v);
	  }
	else if (ev->string)
	  {
	     if (!v->typebuf.visible) typebuf_show(v);
	     typebuf_char_append(v, ev->string);
	  }
     }
}

static int
on_typebuf_timer(void *data)
{
   Viewer *v = data;

   if (!v) return 0;

   if (!v->typebuf.visible) return 0;

   typebuf_hide(v);
   v->typebuf.timer = NULL;

   return 0;
}

static void
on_group_check_changed(void *data, Evas_Object *obj, void *event_info)
{
   Group *grp = data;
   grp->active = elm_check_state_get(obj);

   if (grp->active)
     {
	if (!grp->obj)
	  edje_object_create(grp);
     }
   else
     {
	edje_object_part_unswallow(elm_layout_edje_get(grp->v->gui.ly), grp->obj);
	evas_object_hide(grp->obj);
     }

   toolbar_reconfigure(grp);
}

static void
on_toolbar_changed(void *data, Evas_Object *obj, void *event_info)
{
   Group *grp = data;
   Evas_Object *o;

   o = edje_object_part_swallow_get(elm_layout_edje_get(grp->v->gui.ly), "v.swallow.main");
   if (o)
     {
	edje_object_part_unswallow(elm_layout_edje_get(grp->v->gui.ly), o);
	evas_object_hide(o);
     }

   elm_layout_content_set(grp->v->gui.ly, "v.swallow.main", grp->obj);
   evas_object_show(grp->obj);

   grp->v->visible_group = grp;
   if (grp->v->gui.parts_list)
     fill_group_parts_list(grp->v);
}

static int
on_entries_sort(const void *d1, const void *d2)
{
   if(!d1) return(1);
   if(!d2) return(-1);

   return(strcmp((const char*)d1, (const char*)d2));
}

static void
on_parts_list_toggle_change(void *data, Evas_Object *obj, void *event_info)
{
   Viewer *v = data;

   v->config->show_parts = elm_toggle_state_get(obj);
   if (v->config->show_parts)
     {
	if (!v->gui.parts_list)
	  create_group_parts_list(v);

	if (v->visible_group)
	  fill_group_parts_list(v);
     }
   else if (v->gui.parts_list)
     {
	evas_object_del(v->gui.parts_list);
	v->gui.parts_list = NULL;
	edje_object_signal_emit(elm_layout_edje_get(v->gui.ly), "v,state,parts_list,hide", "v");
     }
   config_save(v, 0);
}

static void
on_signals_toggle_change(void *data, Evas_Object *obj, void *event_info)
{
   Viewer *v = data;

   v->config->show_signals = elm_toggle_state_get(obj);
   if (v->config->show_signals)
     {
	if (!v->gui.sig_box)
	  create_signals_box(v);
     }
   else if (v->gui.parts_list)
     {
	evas_object_del(v->gui.sig_box);
	v->gui.sig_box = NULL;
	edje_object_signal_emit(elm_layout_edje_get(v->gui.ly), "v,state,signals,hide", "v");
     }
   config_save(v, 0);
}

static void
on_group_part_select(void *data, Evas_Object *obj, void *event_info)
{
   Part *prt = data;
   Evas_Coord x, y, w, h, ox, oy;
   Evas_Object *o;

   edje_object_part_geometry_get(prt->grp->obj, prt->name, &x, &y, &w, &h);
   prt->highlight = o = edje_object_add(evas_object_evas_get(prt->grp->obj));
   evas_object_geometry_get(prt->grp->obj, &ox, &oy, NULL, NULL);
   edje_object_file_set(o, prt->grp->v->theme_file, "viewer/part/highlight");
   evas_object_move(o, x + ox, y + oy);
   evas_object_resize(o, w, h);
   evas_object_show(o);
   edje_object_signal_emit(o, "v,state,highlight,show", "v");
}

static void
on_group_part_unselect(void *data, Evas_Object *obj, void *event_info)
{
   Part *prt = (Part *) elm_list_item_data_get(event_info);

   if (prt->highlight)
     {
	evas_object_del(prt->highlight);
	prt->highlight = NULL;
     }
}

static void
on_object_signal(void *data, Evas_Object *o, const char *sig, const char *src)
{
   Group *grp = data;
   char str[1024];

   if (!grp->v->config->show_signals) return;
   if (eina_list_count(grp->v->signals) > 40)
     {
	elm_list_item_del(grp->v->signals->data);
	grp->v->signals = eina_list_remove_list(grp->v->signals, grp->v->signals);
     }
   
   snprintf(str, sizeof(str), "CALLBACK for \"%s\" \"%s\"", sig, src);
   grp->v->signals = eina_list_append(
       grp->v->signals, elm_list_item_append(
	   grp->v->gui.sig_list, str, NULL, NULL, NULL, NULL));

   if (!strcmp(sig, "drag"))
     {
	double x, y;

	if (eina_list_count(grp->v->signals) > 40)
	  {
	     elm_list_item_del(grp->v->signals->data);
	     grp->v->signals = eina_list_remove_list(grp->v->signals, grp->v->signals);
	  }
   
	edje_object_part_drag_value_get(o, src, &x, &y);
	snprintf(str, sizeof(str), "    Drag %3.3f %3.3f", x, y);
	grp->v->signals = eina_list_append(
	    grp->v->signals, elm_list_item_append(
		grp->v->gui.sig_list, str, NULL, NULL, NULL, NULL));
     }


   elm_list_go(grp->v->gui.sig_list);
}


static void
on_object_message(void *data, Evas_Object *obj, Edje_Message_Type type, int id, void *msg)
{
   Group *grp = data;
   char str[1024];

   if (!grp->v->config->show_signals) return;
   if (eina_list_count(grp->v->signals) > 40)
     {
	elm_list_item_del(grp->v->signals->data);
	grp->v->signals = eina_list_remove_list(grp->v->signals, grp->v->signals);
     }
   
   snprintf(str, sizeof(str), "MESSAGE for %p from script type %i id %i", obj, type, id);
   grp->v->signals = eina_list_append(
       grp->v->signals, elm_list_item_append(
	   grp->v->gui.sig_list, str, NULL, NULL, NULL, NULL));

   if (type == EDJE_MESSAGE_STRING)
     {
	Edje_Message_String *emsg;

	if (eina_list_count(grp->v->signals) > 40)
	  {
	     elm_list_item_del(grp->v->signals->data);
	     grp->v->signals = eina_list_remove_list(grp->v->signals, grp->v->signals);
	  }

	emsg = (Edje_Message_String *)msg;
	snprintf(str, 1024, "    String: \"%s\"\n", emsg->str);
	grp->v->signals = eina_list_append(
	    grp->v->signals, elm_list_item_append(
		grp->v->gui.sig_list, str, NULL, NULL, NULL, NULL));
     }
   edje_object_message_send(obj, EDJE_MESSAGE_NONE, 12345, NULL);

   elm_list_go(grp->v->gui.sig_list);
}

/* Genlist functions */
static Evas_Object *
gc_icon_get(const void *data, Evas_Object *obj, const char *part)
{
   Group *grp = (Group *) data;

   if (!strcmp(part, "elm.swallow.icon"))
     {
	Evas_Object *ck;
	ck = elm_check_add(obj);
	evas_object_propagate_events_set(ck, 0);
	elm_check_state_set(ck, grp->active);
	evas_object_smart_callback_add(ck, "changed", on_group_check_changed, data);
	evas_object_show(ck);
	grp->check = ck;
	return ck;
     }
   return NULL;
}

static char *
gc_label_get(const void *data, Evas_Object *obj, const char *part)
{
   const Group *grp = data;

   return strdup(grp->name);
}

static void
gc_del(const void *data, Evas_Object *obj)
{
   Group *grp = (Group *) data;
   eina_stringshare_del(grp->name);

   free_group_parts(grp);

   grp->v->groups = eina_inlist_remove(grp->v->groups, EINA_INLIST_GET(grp));
   free(grp);
}
