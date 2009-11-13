#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;
typedef struct _Elm_Entry_Context_Menu_Item Elm_Entry_Context_Menu_Item;

struct _Widget_Data
{
   Evas_Object *ent;
   Evas_Object *hoversel;
   Ecore_Job *deferred_recalc_job;
   Ecore_Event_Handler *sel_notify_handler;
   Ecore_Event_Handler *sel_clear_handler;
   Ecore_Timer *longpress_timer;
   const char *cut_sel;
   const char *stripped;
   Evas_Coord lastw;
   Evas_Coord downx, downy;
   Evas_Coord cx, cy, cw, ch;
   Eina_List *items;
   Eina_Bool changed : 1;
   Eina_Bool linewrap : 1;
   Eina_Bool char_linewrap : 1;
   Eina_Bool single_line : 1;
   Eina_Bool password : 1;
   Eina_Bool editable : 1;
   Eina_Bool selection_asked : 1;
   Eina_Bool have_selection : 1;
   Eina_Bool selmode : 1;
   Eina_Bool deferred_cur : 1;
   Eina_Bool disabled : 1;
   Eina_Bool context_menu : 1;
};

struct _Elm_Entry_Context_Menu_Item
{
   Evas_Object *obj;
   const char *label;
   const char *icon_file;
   const char *icon_group;
   Elm_Icon_Type icon_type;
   void (*func) (void *data, Evas_Object *obj, void *event_info);
   void *data;
};

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _disable_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _on_focus_hook(void *data, Evas_Object *obj);
static void _resize(void *data, Evas *e, Evas_Object *obj, void *event_info);
static const char *_getbase(Evas_Object *obj);
static void _signal_entry_changed(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _signal_selection_start(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _signal_selection_changed(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _signal_selection_cleared(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _signal_entry_paste_request(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _signal_entry_copy_notify(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _signal_entry_cut_notify(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _signal_cursor_changed(void *data, Evas_Object *obj, const char *emission, const char *source);

static Eina_List *entries = NULL;

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_List *l;
   Elm_Entry_Context_Menu_Item *it;

   entries = eina_list_remove(entries, obj);
#ifdef HAVE_ELEMENTARY_X
   ecore_event_handler_del(wd->sel_notify_handler);
   ecore_event_handler_del(wd->sel_clear_handler);
#endif
   if (wd->cut_sel) eina_stringshare_del(wd->cut_sel);
   if (wd->stripped) eina_stringshare_del(wd->stripped);
   if (wd->deferred_recalc_job) ecore_job_del(wd->deferred_recalc_job);
   if (wd->longpress_timer) ecore_timer_del(wd->longpress_timer);
   EINA_LIST_FREE(wd->items, it)
     {
        eina_stringshare_del(it->label);
        eina_stringshare_del(it->icon_file);
        eina_stringshare_del(it->icon_group);
        free(it);
     }
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   const char *t;

   t = eina_stringshare_add(elm_entry_entry_get(obj));
   _elm_theme_set(wd->ent, "entry", _getbase(obj), elm_widget_style_get(obj));
   elm_entry_entry_set(obj, t);
   eina_stringshare_del(t);
   edje_object_scale_set(wd->ent, elm_widget_scale_get(obj) * _elm_config->scale);
   _sizing_eval(obj);
}

static void
_disable_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (elm_widget_disabled_get(obj))
     {
	edje_object_signal_emit(wd->ent, "elm,state,disabled", "elm");
	wd->disabled = EINA_TRUE;
     }
   else
     {
	edje_object_signal_emit(wd->ent, "elm,state,enabled", "elm");
	wd->disabled = EINA_FALSE;
     }
}

static void
_elm_win_recalc_job(void *data)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   Evas_Coord resw, resh, minminw;

   wd->deferred_recalc_job = NULL;
   evas_object_geometry_get(wd->ent, NULL, NULL, &resw, &resh);
   resh = 0;
   minminw = 0;
   edje_object_size_min_restricted_calc(wd->ent, &minw, &minh, 0, 0);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   minminw = minw;
   edje_object_size_min_restricted_calc(wd->ent, &minw, &minh, resw, 0);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(data, minminw, minh);
   if (wd->single_line) maxh = minh;
   evas_object_size_hint_max_set(data, -1, maxh);
   if (wd->deferred_cur)
     elm_widget_show_region_set(data, wd->cx, wd->cy, wd->cw, wd->ch);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   Evas_Coord resw, resh, minminw;

   if (wd->linewrap || wd->char_linewrap)
     {
	evas_object_geometry_get(wd->ent, NULL, NULL, &resw, &resh);
	if ((resw == wd->lastw) && (!wd->changed)) return;
	wd->changed = EINA_FALSE;
	wd->lastw = resw;
	if (wd->deferred_recalc_job) ecore_job_del(wd->deferred_recalc_job);
	wd->deferred_recalc_job = ecore_job_add(_elm_win_recalc_job, obj);
     }
   else
     {
	evas_object_geometry_get(wd->ent, NULL, NULL, &resw, &resh);
	edje_object_size_min_calc(wd->ent, &minw, &minh);
        elm_coords_finger_size_adjust(1, &minw, 1, &minh);
	evas_object_size_hint_min_set(obj, minw, minh);
        if (wd->single_line) maxh = minh;
	evas_object_size_hint_max_set(obj, maxw, maxh);
     }
}

static void
_on_focus_hook(void *data, Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *top = elm_widget_top_get(obj);

   if (!wd->editable) return;
   if (elm_widget_focus_get(obj))
     {
	evas_object_focus_set(wd->ent, 1);
	edje_object_signal_emit(wd->ent, "elm,action,focus", "elm");
	if (top) elm_win_keyboard_mode_set(top, ELM_WIN_KEYBOARD_ON);
     }
   else
     {
	edje_object_signal_emit(wd->ent, "elm,action,unfocus", "elm");
	evas_object_focus_set(wd->ent, 0);
	if (top) elm_win_keyboard_mode_set(top, ELM_WIN_KEYBOARD_OFF);
     }
}

static void
_hoversel_position(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord cx, cy, cw, ch, x, y, mw, mh;

   evas_object_geometry_get(wd->ent, &x, &y, NULL, NULL);
   edje_object_part_text_cursor_geometry_get(wd->ent, "elm.text",
					     &cx, &cy, &cw, &ch);
   evas_object_size_hint_min_get(wd->hoversel, &mw, &mh);
   if (cw < mw)
     {
	cx += (cw - mw) / 2;
	cw = mw;
     }
   if (ch < mh)
     {
	cy += (ch - mh) / 2;
	ch = mh;
     }
   evas_object_move(wd->hoversel, x + cx, y + cy);
   evas_object_resize(wd->hoversel, cw, ch);
}

static void
_move(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);

   if (wd->hoversel) _hoversel_position(data);
}

static void
_resize(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);

   if (wd->linewrap || wd->char_linewrap) _sizing_eval(data);
   if (wd->hoversel) _hoversel_position(data);
//   Evas_Coord ww, hh;
//   evas_object_geometry_get(wd->ent, NULL, NULL, &ww, &hh);
}

static void
_dismissed(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);

   if (wd->hoversel) evas_object_hide(wd->hoversel);
   if (wd->selmode)
     edje_object_part_text_select_allow_set(wd->ent, "elm.text", 1);
   elm_widget_scroll_freeze_pop(data);
}

static void
_select(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);

   wd->selmode = EINA_TRUE;
   edje_object_part_text_select_none(wd->ent, "elm.text");
   edje_object_part_text_select_allow_set(wd->ent, "elm.text", 1);
   edje_object_signal_emit(wd->ent, "elm,state,select,on", "elm");
   elm_widget_scroll_hold_push(data);
}

static void
_paste(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);

   evas_object_smart_callback_call(data, "selection,paste", NULL);
   if (wd->sel_notify_handler)
     {
#ifdef HAVE_ELEMENTARY_X
	Evas_Object *top;

	top = elm_widget_top_get(data);
	if ((top) && (elm_win_xwindow_get(top)))
	  {
	     ecore_x_selection_primary_request
	       (elm_win_xwindow_get(top),
		ECORE_X_SELECTION_TARGET_UTF8_STRING);
	     wd->selection_asked = EINA_TRUE;
	  }
#endif
     }
}

static void
_store_selection(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   const char *sel = edje_object_part_text_selection_get(wd->ent, "elm.text");

   if (wd->cut_sel) eina_stringshare_del(wd->cut_sel);
   wd->cut_sel = eina_stringshare_add(sel);
}

static void
_cut(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);

   wd->selmode = EINA_FALSE;
   edje_object_part_text_select_allow_set(wd->ent, "elm.text", 0);
   edje_object_signal_emit(wd->ent, "elm,state,select,off", "elm");
   elm_widget_scroll_hold_pop(data);
   _store_selection(data);
   edje_object_part_text_insert(wd->ent, "elm.text", "");
   edje_object_part_text_select_none(wd->ent, "elm.text");
}

static void
_copy(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);

   wd->selmode = EINA_FALSE;
   edje_object_part_text_select_allow_set(wd->ent, "elm.text", 0);
   edje_object_signal_emit(wd->ent, "elm,state,select,off", "elm");
   elm_widget_scroll_hold_pop(data);
   _store_selection(data);
   edje_object_part_text_select_none(wd->ent, "elm.text");
}

static void
_cancel(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);

   wd->selmode = EINA_FALSE;
   edje_object_part_text_select_allow_set(wd->ent, "elm.text", 0);
   edje_object_signal_emit(wd->ent, "elm,state,select,off", "elm");
   elm_widget_scroll_hold_pop(data);
   edje_object_part_text_select_none(wd->ent, "elm.text");
}

static void
_item_clicked(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Entry_Context_Menu_Item *it = data;
   Evas_Object *obj2 = it->obj;

   if (it->func) it->func(it->data, obj2, NULL);
}

static int
_long_press(void *data)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Object *top;
   const Eina_List *l;
   const Elm_Entry_Context_Menu_Item *it;

   if (wd->hoversel) evas_object_del(wd->hoversel);
   wd->hoversel = elm_hoversel_add(data);
   elm_object_style_set(wd->hoversel, "entry");
   elm_widget_sub_object_add(data, wd->hoversel);
   elm_hoversel_label_set(wd->hoversel, "Text");
   top = elm_widget_top_get(data);
   if (top) elm_hoversel_hover_parent_set(wd->hoversel, top);
   evas_object_smart_callback_add(wd->hoversel, "dismissed", _dismissed, data);
   if (!wd->selmode)
     {
	elm_hoversel_item_add(wd->hoversel, "Select", NULL, ELM_ICON_NONE,
                              _select, data);
        if (wd->editable)
          elm_hoversel_item_add(wd->hoversel, "Paste", NULL, ELM_ICON_NONE,
                                _paste, data);
     }
   else
     {
	elm_hoversel_item_add(wd->hoversel, "Copy", NULL, ELM_ICON_NONE,
                              _copy, data);
        if (wd->editable)
          elm_hoversel_item_add(wd->hoversel, "Cut", NULL, ELM_ICON_NONE,
                              _cut, data);
	elm_hoversel_item_add(wd->hoversel, "Cancel", NULL, ELM_ICON_NONE,
                              _cancel, data);
     }
   EINA_LIST_FOREACH(wd->items, l, it)
     {
        elm_hoversel_item_add(wd->hoversel, it->label, it->icon_file,
                              it->icon_type, _item_clicked, it);
     }
   if (wd->hoversel)
     {
	_hoversel_position(data);
	evas_object_show(wd->hoversel);
	elm_hoversel_hover_begin(wd->hoversel);
     }
   wd->longpress_timer = NULL;
   edje_object_part_text_select_allow_set(wd->ent, "elm.text", 0);
   edje_object_part_text_select_abort(wd->ent, "elm.text");
   return 0;
}

static void
_mouse_down(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Event_Mouse_Down *ev = event_info;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   if (ev->button != 1) return;
   //   if (ev->flags & EVAS_BUTTON_DOUBLE_CLICK)
   if (wd->longpress_timer) ecore_timer_del(wd->longpress_timer);
   elm_widget_scroll_freeze_push(data);
   wd->longpress_timer = ecore_timer_add(1.0, _long_press, data);
   wd->downx = ev->canvas.x;
   wd->downy = ev->canvas.y;
}

static void
_mouse_up(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Event_Mouse_Up *ev = event_info;

   if (ev->button != 1) return;
   if (wd->longpress_timer)
     {
	elm_widget_scroll_freeze_pop(data);
	ecore_timer_del(wd->longpress_timer);
	wd->longpress_timer = NULL;
     }
}

static void
_mouse_move(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Event_Mouse_Move *ev = event_info;

   if (!wd->selmode)
     {
	if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
	  {
	     if (wd->longpress_timer)
	       {
		  elm_widget_scroll_freeze_pop(data);
		  ecore_timer_del(wd->longpress_timer);
		  wd->longpress_timer = NULL;
	       }
	  }
	else if (wd->longpress_timer)
	  {
	     Evas_Coord dx, dy;

	     dx = wd->downx - ev->cur.canvas.x;
	     dx *= dx;
	     dy = wd->downy - ev->cur.canvas.y;
	     dy *= dy;
	     if ((dx + dy) >
		 ((_elm_config->finger_size / 2) *
		  (_elm_config->finger_size / 2)))
	       {
		  elm_widget_scroll_freeze_pop(data);
		  ecore_timer_del(wd->longpress_timer);
		  wd->longpress_timer = NULL;
	       }
	  }
     }
   else if (wd->longpress_timer)
     {
	Evas_Coord dx, dy;

	dx = wd->downx - ev->cur.canvas.x;
	dx *= dx;
	dy = wd->downy - ev->cur.canvas.y;
	dy *= dy;
	if ((dx + dy) >
	    ((_elm_config->finger_size / 2) *
	     (_elm_config->finger_size / 2)))
	  {
	     elm_widget_scroll_freeze_pop(data);
	     ecore_timer_del(wd->longpress_timer);
	     wd->longpress_timer = NULL;
	  }
     }
}

static const char *
_getbase(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (wd->editable)
     {
	if (wd->password) return "base-password";
	else
	  {
	     if (wd->single_line) return "base-single";
	     else
	       {
		  if (wd->linewrap) return "base";
        else if (wd->char_linewrap) return "base-charwrap";
		  else  return "base-nowrap";
	       }
	  }
     }
   else
     {
	if (wd->password) return "base-password";
	else
	  {
	     if (wd->single_line) return "base-single-noedit";
	     else
	       {
		  if (wd->linewrap) return "base-noedit";
        else if (wd->char_linewrap) return "base-noedit-charwrap";
		  else  return "base-nowrap-noedit";
	       }
	  }
     }
   return "base";
}

static char *
_str_append(char *str, const char *txt, int *len, int *alloc)
{
   int txt_len = strlen(txt);

   if (txt_len <= 0) return str;
   if ((*len + txt_len) >= *alloc)
     {
	char *str2;
	int alloc2;

	alloc2 = *alloc + txt_len + 128;
	str2 = realloc(str, alloc2);
	if (!str2) return str;
	*alloc = alloc2;
	str = str2;
     }
   strcpy(str + *len, txt);
   *len += txt_len;
   return str;
}

static char *
_mkup_to_text(const char *mkup)
{
   char *str = NULL;
   int str_len = 0, str_alloc = 0;
   char *s, *p;
   char *tag_start, *tag_end, *esc_start, *esc_end, *ts;

   if (!mkup) return NULL;
   tag_start = tag_end = esc_start = esc_end = NULL;
   p = (char *)mkup;
   s = p;
   for (;;)
     {
	if ((*p == 0) ||
	    (tag_end) || (esc_end) ||
	    (tag_start) || (esc_start))
	  {
	     if (tag_end)
	       {
		  char *ttag, *match;

		  ttag = malloc(tag_end - tag_start);
		  if (ttag)
		    {
		       strncpy(ttag, tag_start + 1, tag_end - tag_start - 1);
		       ttag[tag_end - tag_start - 1] = 0;
		       if (!strcmp(ttag, "br"))
			 str = _str_append(str, "\n", &str_len, &str_alloc);
		       else if (!strcmp(ttag, "\n"))
			 str = _str_append(str, "\n", &str_len, &str_alloc);
		       else if (!strcmp(ttag, "\\n"))
			 str = _str_append(str, "\n", &str_len, &str_alloc);
		       else if (!strcmp(ttag, "\t"))
			 str = _str_append(str, "\t", &str_len, &str_alloc);
		       else if (!strcmp(ttag, "\\t"))
			 str = _str_append(str, "\t", &str_len, &str_alloc);
		       free(ttag);
		    }
		  tag_start = tag_end = NULL;
	       }
	     else if (esc_end)
	       {
		  ts = malloc(esc_end - esc_start + 1);
		  if (ts)
		    {
		       const char *esc;
		       strncpy(ts, esc_start, esc_end - esc_start);
		       ts[esc_end - esc_start] = 0;
		       esc = evas_textblock_escape_string_get(ts);
		       if (esc)
			 str = _str_append(str, esc, &str_len, &str_alloc);
		       free(ts);
		    }
		  esc_start = esc_end = NULL;
	       }
	     else if (*p == 0)
	       {
		  ts = malloc(p - s + 1);
		  if (ts)
		    {
		       strncpy(ts, s, p - s);
		       ts[p - s] = 0;
		       str = _str_append(str, ts, &str_len, &str_alloc);
		       free(ts);
		    }
		  s = NULL;
	       }
	     if (*p == 0)
	       break;
	  }
	if (*p == '<')
	  {
	     if (!esc_start)
	       {
		  tag_start = p;
		  tag_end = NULL;
		  ts = malloc(p - s + 1);
		  if (ts)
		    {
		       strncpy(ts, s, p - s);
		       ts[p - s] = 0;
		       str = _str_append(str, ts, &str_len, &str_alloc);
		       free(ts);
		    }
		  s = NULL;
	       }
	  }
	else if (*p == '>')
	  {
	     if (tag_start)
	       {
		  tag_end = p;
		  s = p + 1;
	       }
	  }
	else if (*p == '&')
	  {
	     if (!tag_start)
	       {
		  esc_start = p;
		  esc_end = NULL;
		  ts = malloc(p - s + 1);
		  if (ts)
		    {
		       strncpy(ts, s, p - s);
		       ts[p - s] = 0;
		       str = _str_append(str, ts, &str_len, &str_alloc);
		       free(ts);
		    }
		  s = NULL;
	       }
	  }
	else if (*p == ';')
	  {
	     if (esc_start)
	       {
		  esc_end = p;
		  s = p + 1;
	       }
	  }
	p++;
     }
   return str;
}

static char *
_text_to_mkup(const char *text)
{
   char *str = NULL;
   int str_len = 0, str_alloc = 0;
   int ch, pos = 0, pos2 = 0;

   if (!text) return NULL;
   for (;;)
     {
	pos = pos2;
        pos2 = evas_string_char_next_get((unsigned char *)(text), pos2, &ch);
        if ((ch <= 0) || (pos2 <= 0)) break;
	if (ch == '\n')
          str = _str_append(str, "<br>", &str_len, &str_alloc);
	else if (ch == '\t')
          str = _str_append(str, "<\t>", &str_len, &str_alloc);
	else if (ch == '<')
          str = _str_append(str, "&lt;", &str_len, &str_alloc);
	else if (ch == '>')
          str = _str_append(str, "&gt;", &str_len, &str_alloc);
	else if (ch == '&')
          str = _str_append(str, "&amp;", &str_len, &str_alloc);
	else
	  {
	     char tstr[16];

	     strncpy(tstr, text + pos, pos2 - pos);
	     tstr[pos2 - pos] = 0;
	     str = _str_append(str, tstr, &str_len, &str_alloc);
	  }
     }
   return str;
}

static void
_signal_entry_changed(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);

   wd->changed = EINA_TRUE;
   _sizing_eval(data);
   evas_object_smart_callback_call(data, "changed", NULL);
}

static void
_signal_selection_start(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);
   const Eina_List *l;
   Evas_Object *entry;

   EINA_LIST_FOREACH(entries, l, entry)
     {
	if (entry != data) elm_entry_select_none(entry);
     }
   wd->have_selection = EINA_TRUE;
   evas_object_smart_callback_call(data, "selection,start", NULL);
   if (wd->sel_notify_handler)
     {
	char *txt = _mkup_to_text(elm_entry_selection_get(data));

	if (txt)
	  {
#ifdef HAVE_ELEMENTARY_X
	     Evas_Object *top;

	     top = elm_widget_top_get(data);
	     if ((top) && (elm_win_xwindow_get(top)))
	       ecore_x_selection_primary_set(elm_win_xwindow_get(top), txt,
                                             strlen(txt));
#endif
	     free(txt);
	  }
     }
}

static void
_signal_selection_changed(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);

   wd->have_selection = EINA_TRUE;
   evas_object_smart_callback_call(data, "selection,changed", NULL);
   if (wd->sel_notify_handler)
     {
	char *txt = _mkup_to_text(elm_entry_selection_get(data));

	if (txt)
	  {
#ifdef HAVE_ELEMENTARY_X
	     Evas_Object *top;

	     top = elm_widget_top_get(data);
	     if ((top) && (elm_win_xwindow_get(top)))
	       ecore_x_selection_primary_set(elm_win_xwindow_get(top), txt,
                                             strlen(txt));
#endif
	     free(txt);
	  }
     }
}

static void
_signal_selection_cleared(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);

   if (!wd->have_selection) return;
   wd->have_selection = EINA_FALSE;
   evas_object_smart_callback_call(data, "selection,cleared", NULL);
   if (wd->sel_notify_handler)
     {
	if (wd->cut_sel)
	  {
#ifdef HAVE_ELEMENTARY_X
	     Evas_Object *top;

	     top = elm_widget_top_get(data);
	     if ((top) && (elm_win_xwindow_get(top)))
	       {
		  char *t;

		  t = _mkup_to_text(wd->cut_sel);
		  if (t)
		    {
		       ecore_x_selection_primary_set(elm_win_xwindow_get(top),
                                                     t, strlen(t));
		       free(t);
		    }
	       }
#endif
	     eina_stringshare_del(wd->cut_sel);
	     wd->cut_sel = NULL;
	  }
	else
	  {
#ifdef HAVE_ELEMENTARY_X
	     Evas_Object *top;

	     top = elm_widget_top_get(data);
	     if ((top) && (elm_win_xwindow_get(top)))
	       ecore_x_selection_primary_clear();
#endif
	  }
     }
}

static void
_signal_entry_paste_request(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);

   evas_object_smart_callback_call(data, "selection,paste", NULL);
   if (wd->sel_notify_handler)
     {
#ifdef HAVE_ELEMENTARY_X
	Evas_Object *top;

	top = elm_widget_top_get(data);
	if ((top) && (elm_win_xwindow_get(top)))
	  {
	     ecore_x_selection_primary_request(elm_win_xwindow_get(top),
                                               ECORE_X_SELECTION_TARGET_UTF8_STRING);
	     wd->selection_asked = EINA_TRUE;
	  }
#endif
     }
}

static void
_signal_entry_copy_notify(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);

   evas_object_smart_callback_call(data, "selection,copy", NULL);
}

static void
_signal_entry_cut_notify(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);
   char *txt;

   evas_object_smart_callback_call(data, "selection,cut", NULL);
   if (wd->cut_sel) eina_stringshare_del(wd->cut_sel);
   wd->cut_sel = NULL;
   txt = _mkup_to_text(elm_entry_selection_get(data));
   if (txt)
     {
	wd->cut_sel = eina_stringshare_add(txt);
	free(txt);
     }
   edje_object_part_text_insert(wd->ent, "elm.text", "");
   wd->changed = EINA_TRUE;
   _sizing_eval(data);
}

static void
_signal_cursor_changed(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Coord cx, cy, cw, ch;

   evas_object_smart_callback_call(data, "cursor,changed", NULL);
   edje_object_part_text_cursor_geometry_get(wd->ent, "elm.text",
                                             &cx, &cy, &cw, &ch);
   if (!wd->deferred_recalc_job)
     elm_widget_show_region_set(data, cx, cy, cw, ch);
   else
     {
	wd->deferred_cur = EINA_TRUE;
	wd->cx = cx;
	wd->cy = cy;
	wd->cw = cw;
	wd->ch = ch;
     }
}

static void
_signal_anchor_down(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);
}

static void
_signal_anchor_up(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Elm_Entry_Anchor_Info ei;
   char *buf, *buf2, *p, *p2, *n;
   int buflen;

   p = strrchr(emission, ',');
   if (p)
     {
	const Eina_List *geoms;

	n = p + 1;
	p2 = p -1;
	while (p2 >= emission)
	  {
	     if (*p2 == ',') break;
	     p2--;
	  }
	p2++;
	buf2 = alloca(5 + p - p2);
	strncpy(buf2, p2, p - p2);
	buf2[p - p2] = 0;
	ei.name = n;
	ei.button = atoi(buf2);
	ei.x = ei.y = ei.w = ei.h = 0;
	geoms =
          edje_object_part_text_anchor_geometry_get(wd->ent, "elm.text", ei.name);
	if (geoms)
	  {
	     Evas_Textblock_Rectangle *r;
	     const Eina_List *l;
	     Evas_Coord px, py, x, y;

	     evas_object_geometry_get(wd->ent, &x, &y, NULL, NULL);
	     evas_pointer_output_xy_get(evas_object_evas_get(wd->ent), &px, &py);
	     EINA_LIST_FOREACH(geoms, l, r)
	       {
		  if (((r->x + x) <= px) && ((r->y + y) <= py) &&
		      ((r->x + x + r->w) > px) && ((r->y + y + r->h) > py))
		    {
		       ei.x = r->x + x;
		       ei.y = r->y + y;
		       ei.w = r->w;
		       ei.h = r->h;
		       break;
		    }
	       }
	  }
	if (!wd->disabled)
	  evas_object_smart_callback_call(data, "anchor,clicked", &ei);
     }
}

static void
_signal_anchor_move(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);
}

static void
_signal_anchor_in(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);
}

static void
_signal_anchor_out(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);
}

static void
_signal_key_enter(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);

   evas_object_smart_callback_call(data, "activated", NULL);
}

#ifdef HAVE_ELEMENTARY_X
static int
_event_selection_notify(void *data, int type, void *event)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Ecore_X_Event_Selection_Notify *ev = event;

   if (!wd->selection_asked) return 1;
   if ((ev->selection == ECORE_X_SELECTION_CLIPBOARD) ||
       (ev->selection == ECORE_X_SELECTION_PRIMARY))
     {
	Ecore_X_Selection_Data_Text *text_data;

	text_data = ev->data;
	if (text_data->data.content == ECORE_X_SELECTION_CONTENT_TEXT)
	  {
	     if (text_data->text)
	       {
		  char *txt = _text_to_mkup(text_data->text);

		  if (txt)
		    {
		       elm_entry_entry_insert(data, txt);
		       free(txt);
		    }
	       }
	  }
	wd->selection_asked = EINA_FALSE;
     }
   return 1;
}

static int
_event_selection_clear(void *data, int type, void *event)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Ecore_X_Event_Selection_Clear *ev = event;

   if (!wd->have_selection) return 1;
   if ((ev->selection == ECORE_X_SELECTION_CLIPBOARD) ||
       (ev->selection == ECORE_X_SELECTION_PRIMARY))
     {
	elm_entry_select_none(data);
     }
   return 1;
}
#endif

EAPI Evas_Object *
elm_entry_add(Evas_Object *parent)
{
   Evas_Object *obj, *top;
   Evas *e;
   Widget_Data *wd;

   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   elm_widget_type_set(obj, "entry");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_on_focus_hook_set(obj, _on_focus_hook, NULL);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_disable_hook_set(obj, _disable_hook);
   elm_widget_can_focus_set(obj, 1);

   wd->linewrap     = EINA_TRUE;
   wd->char_linewrap= EINA_FALSE;
   wd->editable     = EINA_TRUE;
   wd->disabled     = EINA_FALSE;
   wd->context_menu = EINA_TRUE;

   wd->ent = edje_object_add(e);
   evas_object_event_callback_add(wd->ent, EVAS_CALLBACK_MOVE, _move, obj);
   evas_object_event_callback_add(wd->ent, EVAS_CALLBACK_RESIZE, _resize, obj);
   evas_object_event_callback_add(wd->ent, EVAS_CALLBACK_MOUSE_DOWN,
                                  _mouse_down, obj);
   evas_object_event_callback_add(wd->ent, EVAS_CALLBACK_MOUSE_UP,
                                  _mouse_up, obj);
   evas_object_event_callback_add(wd->ent, EVAS_CALLBACK_MOUSE_MOVE,
                                  _mouse_move, obj);

   _elm_theme_set(wd->ent, "entry", "base", "default");
   edje_object_signal_callback_add(wd->ent, "entry,changed", "elm.text",
                                   _signal_entry_changed, obj);
   edje_object_signal_callback_add(wd->ent, "selection,start", "elm.text",
                                   _signal_selection_start, obj);
   edje_object_signal_callback_add(wd->ent, "selection,changed", "elm.text",
                                   _signal_selection_changed, obj);
   edje_object_signal_callback_add(wd->ent, "selection,cleared", "elm.text",
                                   _signal_selection_cleared, obj);
   edje_object_signal_callback_add(wd->ent, "entry,paste,request", "elm.text",
                                   _signal_entry_paste_request, obj);
   edje_object_signal_callback_add(wd->ent, "entry,copy,notify", "elm.text",
                                   _signal_entry_copy_notify, obj);
   edje_object_signal_callback_add(wd->ent, "entry,cut,notify", "elm.text",
                                   _signal_entry_cut_notify, obj);
   edje_object_signal_callback_add(wd->ent, "cursor,changed", "elm.text",
                                   _signal_cursor_changed, obj);
   edje_object_signal_callback_add(wd->ent, "anchor,mouse,down,*", "elm.text",
                                   _signal_anchor_down, obj);
   edje_object_signal_callback_add(wd->ent, "anchor,mouse,up,*", "elm.text",
                                   _signal_anchor_up, obj);
   edje_object_signal_callback_add(wd->ent, "anchor,mouse,move,*", "elm.text",
                                   _signal_anchor_move, obj);
   edje_object_signal_callback_add(wd->ent, "anchor,mouse,in,*", "elm.text",
                                   _signal_anchor_in, obj);
   edje_object_signal_callback_add(wd->ent, "anchor,mouse,out,*", "elm.text",
                                   _signal_anchor_out, obj);
   edje_object_signal_callback_add(wd->ent, "entry,key,enter", "elm.text",
                                   _signal_key_enter, obj);
   edje_object_part_text_set(wd->ent, "elm.text", "<br>");
   elm_widget_resize_object_set(obj, wd->ent);
   _sizing_eval(obj);

#ifdef HAVE_ELEMENTARY_X
   top = elm_widget_top_get(obj);
   if ((top) && (elm_win_xwindow_get(top)))
     {
	wd->sel_notify_handler =
	  ecore_event_handler_add(ECORE_X_EVENT_SELECTION_NOTIFY,
				  _event_selection_notify, obj);
	wd->sel_clear_handler =
	  ecore_event_handler_add(ECORE_X_EVENT_SELECTION_CLEAR,
				  _event_selection_clear, obj);
     }
#endif

   entries = eina_list_prepend(entries, obj);
   return obj;
}

EAPI void
elm_entry_single_line_set(Evas_Object *obj, Eina_Bool single_line)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   const char *t;

   if (wd->single_line == single_line) return;
   wd->single_line = single_line;
   wd->linewrap = EINA_FALSE;
   wd->char_linewrap = EINA_FALSE;
   t = eina_stringshare_add(elm_entry_entry_get(obj));
   _elm_theme_set(wd->ent, "entry", _getbase(obj), elm_widget_style_get(obj));
   elm_entry_entry_set(obj, t);
   eina_stringshare_del(t);
   _sizing_eval(obj);
}

EAPI void
elm_entry_password_set(Evas_Object *obj, Eina_Bool password)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   const char *t;

   if (wd->password == password) return;
   wd->password = password;
   wd->single_line = EINA_TRUE;
   wd->linewrap = EINA_FALSE;
   wd->char_linewrap = EINA_FALSE;
   t = eina_stringshare_add(elm_entry_entry_get(obj));
   _elm_theme_set(wd->ent, "entry", _getbase(obj), elm_widget_style_get(obj));
   elm_entry_entry_set(obj, t);
   eina_stringshare_del(t);
   _sizing_eval(obj);
}

EAPI void
elm_entry_entry_set(Evas_Object *obj, const char *entry)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!entry) entry = "<br>";
   edje_object_part_text_set(wd->ent, "elm.text", entry);
   // debug
#if 0
     {
	const Eina_List *l, *an;
	const char *anchor;

	an = edje_object_part_text_anchor_list_get(wd->ent, "elm.text");
	EINA_LIST_FOREACH(an, l, anchor)
	  printf("ANCHOR: %s\n", anchor);
     }
#endif
   wd->changed = EINA_TRUE;
   _sizing_eval(obj);
}

EAPI const char *
elm_entry_entry_get(const Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   const char *text, *s;

   if (!wd) return NULL;
   // Strip ending <br> that is added by the textblock
   // need to check if <br> is present? seems it is always there
   text = edje_object_part_text_get(wd->ent, "elm.text");
   s = eina_stringshare_add_length(text, strlen(text) - 4);

   if (wd->stripped) eina_stringshare_del(wd->stripped);
   wd->stripped = s;
   return s;
}

EAPI const char *
elm_entry_selection_get(const Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return NULL;
   return edje_object_part_text_selection_get(wd->ent, "elm.text");
}

EAPI void
elm_entry_entry_insert(Evas_Object *obj, const char *entry)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   edje_object_part_text_insert(wd->ent, "elm.text", entry);
   wd->changed = EINA_TRUE;
   _sizing_eval(obj);
}

EAPI void
elm_entry_line_wrap_set(Evas_Object *obj, Eina_Bool wrap)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   const char *t;

   if (wd->linewrap == wrap) return;
   wd->linewrap = wrap;
   if(wd->linewrap)
       wd->char_linewrap = EINA_FALSE;
   t = eina_stringshare_add(elm_entry_entry_get(obj));
   _elm_theme_set(wd->ent, "entry", _getbase(obj), elm_widget_style_get(obj));
   elm_entry_entry_set(obj, t);
   eina_stringshare_del(t);
   _sizing_eval(obj);
}

EAPI void
elm_entry_line_char_wrap_set(Evas_Object *obj, Eina_Bool wrap)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   const char *t;

   if (wd->char_linewrap == wrap) return;
   wd->char_linewrap = wrap;
   if(wd->char_linewrap)
       wd->linewrap = EINA_FALSE;
   t = eina_stringshare_add(elm_entry_entry_get(obj));
   _elm_theme_set(wd->ent, "entry", _getbase(obj), elm_widget_style_get(obj));
   elm_entry_entry_set(obj, t);
   eina_stringshare_del(t);
   _sizing_eval(obj);
}

EAPI void
elm_entry_editable_set(Evas_Object *obj, Eina_Bool editable)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   const char *t;

   if (wd->editable == editable) return;
   wd->editable = editable;
   t = eina_stringshare_add(elm_entry_entry_get(obj));
   _elm_theme_set(wd->ent, "entry", _getbase(obj), elm_widget_style_get(obj));
   elm_entry_entry_set(obj, t);
   eina_stringshare_del(t);
   _sizing_eval(obj);
}

EAPI void
elm_entry_select_none(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (wd->selmode)
     {
	wd->selmode = EINA_FALSE;
	edje_object_part_text_select_allow_set(wd->ent, "elm.text", 0);
	edje_object_signal_emit(wd->ent, "elm,state,select,off", "elm");
     }
   wd->have_selection = EINA_FALSE;
   edje_object_part_text_select_none(wd->ent, "elm.text");
}

EAPI void
elm_entry_select_all(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (wd->selmode)
     {
	wd->selmode = EINA_FALSE;
	edje_object_part_text_select_allow_set(wd->ent, "elm.text", 0);
	edje_object_signal_emit(wd->ent, "elm,state,select,off", "elm");
     }
   wd->have_selection = EINA_TRUE;
   edje_object_part_text_select_all(wd->ent, "elm.text");
}

EAPI void
elm_entry_context_menu_clear(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_List *l;
   Elm_Entry_Context_Menu_Item *it;

   if (!wd) return;
   EINA_LIST_FREE(wd->items, it)
     {
        eina_stringshare_del(it->label);
        eina_stringshare_del(it->icon_file);
        eina_stringshare_del(it->icon_group);
        free(it);
     }
}

EAPI void
elm_entry_context_menu_item_add(Evas_Object *obj, const char *label, const char *icon_file, Elm_Icon_Type icon_type, void (*func) (void *data, Evas_Object *obj, void *event_info), const void *data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_Entry_Context_Menu_Item *it;

   if (!wd) return;
   it = calloc(1, sizeof(Elm_Entry_Context_Menu_Item));
   if (!it) return;
   wd->items = eina_list_append(wd->items, it);
   it->obj = obj;
   it->label = eina_stringshare_add(label);
   it->icon_file = eina_stringshare_add(icon_file);
   it->icon_type = icon_type;
   it->func = func;
   it->data = (void *)data;
}

EAPI void
elm_entry_context_menu_disabled_set(Evas_Object *obj, Eina_Bool disabled)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->context_menu == !disabled) return;
   wd->context_menu = !disabled;

   if (wd->context_menu)
     {
        evas_object_event_callback_add(wd->ent, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down, obj);
        evas_object_event_callback_add(wd->ent, EVAS_CALLBACK_MOUSE_UP, _mouse_up, obj);
     }
   else
     {
        evas_object_event_callback_del_full(wd->ent, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down, obj);
        evas_object_event_callback_del_full(wd->ent, EVAS_CALLBACK_MOUSE_UP, _mouse_up, obj);
     }
}

EAPI Eina_Bool
elm_entry_context_menu_disabled_get(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return !wd->context_menu;
}

EAPI char *
elm_entry_markup_to_utf8(const char *s)
{
   return _mkup_to_text(s);
}

EAPI char *
elm_entry_utf8_to_markup(const char *s)
{
   return _text_to_mkup(s);
}
