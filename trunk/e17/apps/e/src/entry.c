#include "entry.h"
#include "config.h"
#include "util.h"

static Evas_List entries;

static void e_clear_selection(Eevent * ev);
static void e_paste_request(Eevent * ev);

static void e_entry_down_cb(void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y);
static void e_entry_up_cb(void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y);
static void e_entry_move_cb(void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y);
static void e_entry_realize(E_Entry *entry);
static void e_entry_unrealize(E_Entry *entry);
static void e_entry_configure(E_Entry *entry);

static void
e_clear_selection(Eevent * ev)
{
   Ev_Clear_Selection *e;
   Evas_List l;
   
   e = ev->event;
   for (l = entries; l; l = l->next)
     {
	E_Entry *entry;
	
	entry = l->data;
	if (entry->selection_win == e->win)
	  {
	     e_window_destroy(entry->selection_win);
	     entry->selection_win = 0;
	     entry->select.start = -1;
	     entry->select.length = 0;
	     e_entry_configure(entry);	     
	  }
     }
}

static void
e_paste_request(Eevent * ev)
{
   Ev_Paste_Request *e;
   Evas_List l;
   
   e = ev->event;
   for (l = entries; l; l = l->next)
     {
	E_Entry *entry;
	
	entry = l->data;
	if (entry->paste_win == e->win)
	  {
	     char *type;
	     
	     type = e->string;
	     e_entry_clear_selection(entry);
	     e_entry_insert_text(entry, type);
	  }
     }
}

static void
e_entry_down_cb(void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y)
{
   E_Entry *entry;
   int pos;
   
   entry = _data;
   if ((_b == 2) && (!entry->mouse_down))       
     {
	if (entry->paste_win) e_window_destroy(entry->paste_win);
	entry->paste_win = e_selection_request();
     }
   else if (!entry->mouse_down)
     {
	entry->focused = 1;
	pos = evas_text_at_position(_e, entry->text, _x, _y,
				    NULL, NULL, NULL, NULL);
	if (pos < 0)
	  {
	     int tw;
	     
	     tw = evas_get_text_width(_e, entry->text);
	     if (_x > entry->x + tw)
	       {
		  entry->cursor_pos = strlen(entry->buffer);
	       }
	     else if (_x < entry->x)
	       {
		  entry->cursor_pos = 0;
	       }
	  }
	else
	  {
	     entry->cursor_pos = pos;
	  }
	entry->mouse_down = _b;
	entry->select.start = -1;
	e_entry_configure(entry);
     }
   UN(_o);
}

static void
e_entry_up_cb(void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y)
{
   E_Entry *entry;
   /* int pos; */
   
   entry = _data;
   if (_b == entry->mouse_down) entry->mouse_down = 0;
   e_entry_configure(entry);
   UN(_e);
   UN(_o);
   UN(_x);
   UN(_y);
}

static void
e_entry_move_cb(void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y)
{
   E_Entry *entry;
   
   entry = _data;
   if (entry->mouse_down > 0)
     {
	int pos, ppos;
	double ty;
	
	ppos = entry->cursor_pos;
	evas_get_geometry(entry->evas, entry->text, NULL, &ty, NULL, NULL);
	pos = evas_text_at_position(_e, entry->text, _x, ty,
				    NULL, NULL, NULL, NULL);
	if (pos < 0)
	  {
	     int tw;
	     
	     tw = evas_get_text_width(_e, entry->text);
	     if (_x > entry->x + tw)
	       {
		  entry->cursor_pos = strlen(entry->buffer);
	       }
	     else if (_x < entry->x)
	       {
		  entry->cursor_pos = 0;
	       }
	  }
	else
	  {
	     entry->cursor_pos = pos;
	  }
	if ((entry->select.start < 0) && (ppos != entry->cursor_pos))
	  {
	     if (ppos < entry->cursor_pos)
	       {
		  entry->select.down = ppos;
		  entry->select.start = ppos;
		  entry->select.length = entry->cursor_pos - ppos +1;
	       }
	     else
	       {
		  entry->select.down = ppos;
		  entry->select.start = entry->cursor_pos;
		  entry->select.length = ppos - entry->cursor_pos +1;
	       }
	  }
	else if (entry->select.start >= 0)
	  {
	     if (entry->cursor_pos < entry->select.down)
	       {
		  entry->select.start = entry->cursor_pos;
		  entry->select.length = entry->select.down - entry->cursor_pos + 1;
	       }
	     else
	       {
		  entry->select.start = entry->select.down;
		  entry->select.length = entry->cursor_pos - entry->select.down + 1;
	       }
	  }
	if (entry->select.start >= 0)
	  {
	     char *str2;
	     
	     str2 = e_entry_get_selection(entry);
	     if (str2)
	       {
		  if (entry->selection_win) e_window_destroy(entry->selection_win);
		  entry->selection_win = e_selection_set(str2);
		  free(str2);
	       }
	  }
	e_entry_configure(entry);   
     }
   UN(_o);
   UN(_b);
   UN(_y);
}

static void 
e_entry_realize(E_Entry *entry)
{
   char *entries;
   char buf[PATH_MAX];
   
   entries = e_config_get("entries");
   sprintf(buf, "%s/%s", entries, "base.bits.db");
   entry->obj_base = ebits_load(buf); 
   if (entry->obj_base) 
    {
	ebits_add_to_evas(entry->obj_base, entry->evas);
/*	ebits_set_color_class(entry->obj_base, "Base FG", 100, 200, 255, 255);*/
     }   
   sprintf(buf, "%s/%s", entries, "cursor.bits.db");
   entry->obj_cursor = ebits_load(buf); 
   if (entry->obj_cursor) 
    {
	ebits_add_to_evas(entry->obj_cursor, entry->evas);
/*	ebits_set_color_class(entry->obj_base, "Base FG", 100, 200, 255, 255);*/
     }   
   sprintf(buf, "%s/%s", entries, "selection.bits.db");
   entry->obj_selection = ebits_load(buf); 
   if (entry->obj_selection) 
    {
	ebits_add_to_evas(entry->obj_selection, entry->evas);
/*	ebits_set_color_class(entry->obj_base, "Base FG", 100, 200, 255, 255);*/
     }   
   
   entry->clip_box = evas_add_rectangle(entry->evas);
   entry->text = evas_add_text(entry->evas, "borzoib", 8, "");
   if (entry->obj_cursor) 
     ebits_set_clip(entry->obj_cursor, entry->clip_box);
   if (entry->obj_selection) 
     ebits_set_clip(entry->obj_selection, entry->clip_box);
   entry->event_box = evas_add_rectangle(entry->evas);
   evas_set_color(entry->evas, entry->clip_box, 255, 255, 255, 255);
   evas_set_color(entry->evas, entry->event_box, 0, 0, 0, 0);
   evas_set_color(entry->evas, entry->text, 0, 0, 0, 255);
   evas_set_clip(entry->evas, entry->text, entry->clip_box);
   evas_set_clip(entry->evas, entry->event_box, entry->clip_box);
   evas_callback_add(entry->evas, entry->event_box, CALLBACK_MOUSE_DOWN, e_entry_down_cb, entry);
   evas_callback_add(entry->evas, entry->event_box, CALLBACK_MOUSE_UP, e_entry_up_cb, entry);
   evas_callback_add(entry->evas, entry->event_box, CALLBACK_MOUSE_MOVE, e_entry_move_cb, entry);
}

static void 
e_entry_unrealize(E_Entry *entry)
{
   if (entry->event_box) evas_del_object(entry->evas, entry->event_box);
   if (entry->text) evas_del_object(entry->evas, entry->text);
   if (entry->clip_box) evas_del_object(entry->evas, entry->clip_box);
   if (entry->obj_base) ebits_free(entry->obj_base);
   if (entry->obj_cursor) ebits_free(entry->obj_cursor);
   if (entry->obj_selection) ebits_free(entry->obj_selection);
   entry->event_box = NULL;
   entry->text = NULL;
   entry->clip_box = NULL;
   entry->obj_base = NULL;
   entry->obj_cursor = NULL;
   entry->obj_selection = NULL;
}

static void 
e_entry_configure(E_Entry *entry)
{
   int p1l, p1r, p1t, p1b;
   int p2l, p2r, p2t, p2b;
   
   if (!entry->evas) return;
   if (!entry->event_box) return;
   p1l = p1r = p1t = p1b = 0;
   if (entry->obj_base) ebits_get_insets(entry->obj_base, &p1l, &p1r, &p1t, &p1b);
   p2l = p2r = p2t = p2b = 0;
   if (entry->obj_cursor) ebits_get_insets(entry->obj_cursor, &p2l, &p2r, &p2t, &p2b);
   if (entry->obj_base) 
     {
	ebits_move(entry->obj_base, entry->x, entry->y);
	ebits_resize(entry->obj_base, entry->w, entry->h);
     }
   evas_move(entry->evas, entry->clip_box, entry->x + p1l, entry->y + p1t);
   evas_resize(entry->evas, entry->clip_box, entry->w - p1l - p1r, entry->h - p1t - p1b);
   evas_move(entry->evas, entry->event_box, entry->x + p1l + p2l, entry->y + p1t + p2t);
   evas_resize(entry->evas, entry->event_box, entry->w - p1l - p1r - p2l - p2r, entry->h - p1t - p1b - p2t - p2b);
   if ((entry->buffer) && (entry->buffer[0] != 0) && (entry->focused))
     {
	double tx, ty, tw, th;
	
	if (entry->cursor_pos < (int)strlen(entry->buffer))
	  {
	     evas_text_at(entry->evas, entry->text, entry->cursor_pos, &tx, &ty, &tw, &th);
	  }
	else
	  {
	     entry->cursor_pos = strlen(entry->buffer);
	     evas_text_at(entry->evas, entry->text, entry->cursor_pos - 1, &tx, &ty, &tw, &th);
	     tx += tw;
	     tw = entry->end_width;
	  }
	th = evas_get_text_height(entry->evas, entry->text);
	if (tx + tw + entry->scroll_pos > entry->w - p1l - p1r)
	  entry->scroll_pos = entry->w - tx - tw - p1l - p1r - p1l - p2l;
	else if (tx + entry->scroll_pos < p1l)
	  entry->scroll_pos = 0 - tx;
	if (entry->obj_cursor)
	  {
	     ebits_move(entry->obj_cursor, entry->x + tx + entry->scroll_pos + p1l, entry->y + ty + p1t);
	     ebits_resize(entry->obj_cursor, tw + p2l + p2r, th + p2t + p2b);
	     ebits_show(entry->obj_cursor);
	  }
     }
   else if (entry->focused)
     {
	int tx, tw, th;
	
	entry->scroll_pos = 0;
	tw = 4;
	tx = 0;
	th = evas_get_text_height(entry->evas, entry->text);
	if (entry->obj_cursor)
	  {
	     ebits_move(entry->obj_cursor, entry->x + tx + entry->scroll_pos + p1l, entry->y + p1t);
	     ebits_resize(entry->obj_cursor, entry->end_width + p2l + p2r, th + p2t + p2b);
	     ebits_show(entry->obj_cursor);
	  }
     }
   else
     {
	if (entry->obj_cursor)
	  ebits_hide(entry->obj_cursor);	
     }
   evas_move(entry->evas, entry->text, entry->x + entry->scroll_pos + p1l + p2l, entry->y + p1t + p2t);
   if (entry->select.start >= 0)
     {
	double x1, y1, x2, tw, th;
	
	evas_text_at(entry->evas, entry->text, entry->select.start, &x1, &y1, NULL, NULL);
	if (entry->select.start + entry->select.length <= (int)strlen(entry->buffer))
	  evas_text_at(entry->evas, entry->text, entry->select.start + entry->select.length - 1, &x2, NULL, &tw, &th);
	else
	  {
	     evas_text_at(entry->evas, entry->text, entry->select.start + entry->select.length - 2, &x2, NULL, &tw, &th);
	     tw += entry->end_width;
	  }
	th = evas_get_text_height(entry->evas, entry->text);
	if (entry->obj_selection)
	  {
	     ebits_move(entry->obj_selection, entry->x + x1 + entry->scroll_pos + p1l, entry->y + y1 + p1t);
	     ebits_resize(entry->obj_selection, x2 + tw - x1 + p2l + p2r, th + p2t + p2b);
	     ebits_show(entry->obj_selection);
	  }
     }
   else
     {
	if (entry->obj_selection)
	  ebits_hide(entry->obj_selection);
     }
}

void
e_entry_init(void)
{
   e_event_filter_handler_add(EV_PASTE_REQUEST,               e_paste_request);
   e_event_filter_handler_add(EV_CLEAR_SELECTION,             e_clear_selection);
}

void
e_entry_free(E_Entry *entry)
{
   entries = evas_list_remove(entries, entry);
   e_entry_unrealize(entry);
   IF_FREE(entry->buffer);
   FREE(entry);
}

E_Entry *
e_entry_new(void)
{
   E_Entry *entry;
   
   entry = NEW(E_Entry, 1);
   ZERO(entry, E_Entry, 1);
   e_strdup(entry->buffer, "");
   entry->select.start = -1;
   entry->end_width = 4;
   entries = evas_list_prepend(entries, entry);
   return entry;
}

void
e_entry_handle_keypress(E_Entry *entry, Ev_Key_Down *e)
{
   if (!entry->focused) return;
   if (!strcmp(e->key, "Up"))
     {
     }
   else if (!strcmp(e->key, "Down"))
     {
     }
   else if (!strcmp(e->key, "Left"))
     {
	entry->cursor_pos--;
	if (entry->cursor_pos < 0) 
	  entry->cursor_pos = 0;
     }
   else if (!strcmp(e->key, "Right"))
     {
	entry->cursor_pos++;
	if (entry->cursor_pos > (int)strlen(entry->buffer)) 
	  entry->cursor_pos = strlen(entry->buffer);
     }
   else if (!strcmp(e->key, "Escape"))
     {
	entry->focused = 0;
     }
   else if (!strcmp(e->key, "BackSpace"))
     {
	/* char *str2; */
	
	if (entry->select.start >= 0) e_entry_clear_selection(entry);
	else if (entry->cursor_pos > 0) e_entry_delete_to_left(entry);
     }
   else if (!strcmp(e->key, "Delete"))
     {
	/* char *str2; */
	
	if (entry->select.start >= 0) e_entry_clear_selection(entry);
	else if (entry->cursor_pos < (int)strlen(entry->buffer)) e_entry_delete_to_right(entry);
     }
   else if (!strcmp(e->key, "Insert"))
     {
	if (entry->paste_win) e_window_destroy(entry->paste_win);
	entry->paste_win = e_selection_request();
     }
   else if (!strcmp(e->key, "Home"))
     {
	entry->cursor_pos = 0;
     }
   else if (!strcmp(e->key, "End"))
     {
	entry->cursor_pos = strlen(entry->buffer);	
     }
   else if (!strcmp(e->key, "Prior"))
     {
	entry->cursor_pos = 0;
     }
   else if (!strcmp(e->key, "Next"))
     {
	entry->cursor_pos = strlen(entry->buffer);	
     }
   else if (!strcmp(e->key, "Return"))
     {
	entry->focused = 0;
	if (entry->func_enter) 
	  entry->func_enter(entry, entry->data_enter);
     }
   else
     {
	char *type;
	
	type = e_key_press_translate_into_typeable(e);
	if (type)
	  {
	     printf("%0x\n", type[0]);
	     if ((strlen(type) == 1) && (type[0] == 0x01)) /* ctrl+a */
	       {
		  entry->cursor_pos = 0;
	       }
	     else if ((strlen(type) == 1) && (type[0] == 0x05)) /* ctrl+e */
	       {
		  entry->cursor_pos = strlen(entry->buffer);
	       }
	     else if ((strlen(type) == 1) && (type[0] == 0x0b)) /* ctk+k */
	       {
		  char *str2;
		  
		  e_strdup(str2, e_entry_get_text(entry));
		  str2[entry->cursor_pos] = 0;
		  e_entry_set_text(entry, str2);
		  free(str2);		  
	       }
	     else if ((strlen(type) == 1) && (type[0] == 0x06)) /* ctrl+f */
	       {
		  entry->cursor_pos++;
		  if (entry->cursor_pos > (int)strlen(entry->buffer))
		    entry->cursor_pos = strlen(entry->buffer);
	       }
	     else if ((strlen(type) == 1) && (type[0] == 0x02)) /* ctrl+b */
	       {
		  entry->cursor_pos--;
		  if (entry->cursor_pos < 0) entry->cursor_pos = 0;
	       }
	     else if (strlen(type) > 0)
	       {
		  e_entry_clear_selection(entry);
		  e_entry_insert_text(entry, type);
	       }
	  }
     }   
   e_entry_configure(entry);
}

void
e_entry_set_evas(E_Entry *entry, Evas evas)
{
   if (entry->evas) e_entry_unrealize(entry);
   entry->evas = evas;
   e_entry_realize(entry);
   e_entry_configure(entry);
   if (entry->visible)
     {
	entry->visible = 0;
	e_entry_show(entry);
     }
}

void
e_entry_show(E_Entry *entry)
{
   if (entry->visible) return;
   entry->visible = 1;
   if (!entry->evas) return;
   if (entry->obj_base) ebits_show(entry->obj_base);
   if (entry->obj_cursor) ebits_show(entry->obj_cursor);
   if (entry->obj_selection) ebits_show(entry->obj_selection);
   evas_show(entry->evas, entry->event_box);
   evas_show(entry->evas, entry->clip_box);
   evas_show(entry->evas, entry->text);
}

void
e_entry_hide(E_Entry *entry)
{
   if (!entry->visible) return;
   entry->visible = 0;
   if (!entry->evas) return;
   if (entry->obj_base) ebits_hide(entry->obj_base);
   if (entry->obj_cursor) ebits_hide(entry->obj_cursor);
   if (entry->obj_selection) ebits_hide(entry->obj_selection);
   evas_hide(entry->evas, entry->event_box);
   evas_hide(entry->evas, entry->clip_box);
   evas_hide(entry->evas, entry->text);
}

void
e_entry_raise(E_Entry *entry)
{
   if (entry->obj_base) ebits_raise(entry->obj_base);
   evas_raise(entry->evas, entry->clip_box);
   evas_raise(entry->evas, entry->text);
   if (entry->obj_selection) ebits_raise(entry->obj_selection);
   if (entry->obj_cursor) ebits_raise(entry->obj_cursor);
   evas_raise(entry->evas, entry->event_box);
}

void
e_entry_lower(E_Entry *entry)
{
   evas_lower(entry->evas, entry->event_box);
   if (entry->obj_cursor) ebits_lower(entry->obj_cursor);
   if (entry->obj_selection) ebits_lower(entry->obj_selection);
   evas_lower(entry->evas, entry->text);
   evas_lower(entry->evas, entry->clip_box);
   if (entry->obj_base) ebits_lower(entry->obj_base);
}

void
e_entry_set_layer(E_Entry *entry, int l)
{
   if (entry->obj_base) ebits_set_layer(entry->obj_base, l);
   evas_set_layer(entry->evas, entry->clip_box, l);
   evas_set_layer(entry->evas, entry->text, l);
   if (entry->obj_selection) ebits_set_layer(entry->obj_selection, l);
   if (entry->obj_cursor) ebits_set_layer(entry->obj_cursor, l);
   evas_set_layer(entry->evas, entry->event_box, l);
}

void
e_entry_set_clip(E_Entry *entry, Evas_Object clip)
{
   evas_set_clip(entry->evas, entry->clip_box, clip);
   if (entry->obj_base) 
     ebits_set_clip(entry->obj_base, clip);
}

void
e_entry_unset_clip(E_Entry *entry)
{
   evas_unset_clip(entry->evas, entry->clip_box);
   if (entry->obj_base) 
     ebits_unset_clip(entry->obj_base);
}

void
e_entry_move(E_Entry *entry, int x, int y)
{
   entry->x = x;
   entry->y = y;
   e_entry_configure(entry);
}

void
e_entry_resize(E_Entry *entry, int w, int h)
{
   entry->w = w;
   entry->h = h;
   e_entry_configure(entry);
}

void
e_entry_query_max_size(E_Entry *entry, int *w, int *h)
{
   int p1l, p1r, p1t, p1b;
   int p2l, p2r, p2t, p2b;
   
   p1l = p1r = p1t = p1b = 0;
   if (entry->obj_base) ebits_get_insets(entry->obj_base, &p1l, &p1r, &p1t, &p1b);
   p2l = p2r = p2t = p2b = 0;
   if (entry->obj_cursor) ebits_get_insets(entry->obj_cursor, &p2l, &p2r, &p2t, &p2b);
   
   if (w) *w = evas_get_text_width(entry->evas, entry->text) + p1l + p1r + p2l + p2r;
   if (h) *h = evas_get_text_height(entry->evas, entry->text) + p1t + p1b + p2t + p2b;
}

void
e_entry_max_size(E_Entry *entry, int *w, int *h)
{
   int p1l, p1r, p1t, p1b;
   int p2l, p2r, p2t, p2b;
   
   p1l = p1r = p1t = p1b = 0;
   if (entry->obj_base) ebits_get_insets(entry->obj_base, &p1l, &p1r, &p1t, &p1b);
   p2l = p2r = p2t = p2b = 0;
   if (entry->obj_cursor) ebits_get_insets(entry->obj_cursor, &p2l, &p2r, &p2t, &p2b);
   if (w) *w = 8000;
   if (h) *h = evas_get_text_height(entry->evas, entry->text) + p1t + p1b + p2t + p2b;
}

void
e_entry_min_size(E_Entry *entry, int *w, int *h)
{
   int p1l, p1r, p1t, p1b;
   int p2l, p2r, p2t, p2b;
   
   p1l = p1r = p1t = p1b = 0;
   if (entry->obj_base) ebits_get_insets(entry->obj_base, &p1l, &p1r, &p1t, &p1b);
   p2l = p2r = p2t = p2b = 0;
   if (entry->obj_cursor) ebits_get_insets(entry->obj_cursor, &p2l, &p2r, &p2t, &p2b);
   if (w) *w = p1l + p1r + p2l + p2r + entry->min_size;
   if (h) *h = evas_get_text_height(entry->evas, entry->text) + p1t + p1b + p2t + p2b;
}

void
e_entry_set_size(E_Entry *entry, int w, int h)
{
   int p1l, p1r, p1t, p1b;
   int p2l, p2r, p2t, p2b;
   
   p1l = p1r = p1t = p1b = 0;
   if (entry->obj_base) ebits_get_insets(entry->obj_base, &p1l, &p1r, &p1t, &p1b);
   p2l = p2r = p2t = p2b = 0;
   if (entry->obj_cursor) ebits_get_insets(entry->obj_cursor, &p2l, &p2r, &p2t, &p2b);
   if (p1l + p1r + p2l + p2r + w > entry->w)
     {
	entry->min_size = w;
	e_entry_configure(entry);
     }
   UN(h);
}

void
e_entry_set_focus(E_Entry *entry, int focused)
{
   if (entry->focused == focused) return;
   entry->focused = focused;
   e_entry_configure(entry);
   if (entry->focused)
     {
	if (entry->func_focus_in) 
	  entry->func_focus_in(entry, entry->data_focus_in);
     }
   else
     {
	if (entry->func_focus_out) 
	  entry->func_focus_out(entry, entry->data_focus_out);
     }
}

void
e_entry_set_text(E_Entry *entry, const char *text)
{
   IF_FREE(entry->buffer);
   e_strdup(entry->buffer, text);
   evas_set_text(entry->evas, entry->text, entry->buffer);
   if (entry->cursor_pos > (int)strlen(entry->buffer))
     entry->cursor_pos = strlen(entry->buffer);
   e_entry_configure(entry);
   if (entry->func_changed) 
     entry->func_changed(entry, entry->data_changed);
}

const char *
e_entry_get_text(E_Entry *entry)
{
   return entry->buffer;
}

void
e_entry_set_cursor(E_Entry *entry, int cursor_pos)
{
   entry->cursor_pos = cursor_pos;
   e_entry_configure(entry);   
}

int
e_entry_get_cursor(E_Entry *entry)
{
   return entry->cursor_pos;
}

void
e_entry_set_changed_callback(E_Entry *entry, void (*func) (E_Entry *_entry, void *_data), void *data)
{
   entry->func_changed = func;
   entry->data_changed = data;
}

void
e_entry_set_enter_callback(E_Entry *entry, void (*func) (E_Entry *_entry, void *_data), void *data)
{
   entry->func_enter = func;
   entry->data_enter = data;
}

void
e_entry_set_focus_in_callback(E_Entry *entry, void (*func) (E_Entry *_entry, void *_data), void *data)
{
   entry->func_focus_in = func;
   entry->data_focus_in = data;
}

void
e_entry_set_focus_out_callback(E_Entry *entry, void (*func) (E_Entry *_entry, void *_data), void *data)
{
   entry->func_focus_out = func;
   entry->data_focus_out = data;
}

void
e_entry_insert_text(E_Entry *entry, char *text)
{
   char *str2;
   
   if (!text) return;
   str2 = malloc(strlen(e_entry_get_text(entry)) + 1 + strlen(text));
   str2[0] = 0;
   strncat(str2, entry->buffer, entry->cursor_pos);
   strcat(str2, text);
   strcat(str2, &(entry->buffer[entry->cursor_pos]));
   e_entry_set_text(entry, str2);
   free(str2);
   entry->cursor_pos+=strlen(text);
   e_entry_configure(entry);
}

void
e_entry_clear_selection(E_Entry *entry)
{
   char *str2;
   
   if (entry->select.start >= 0)
     {
	e_strdup(str2, e_entry_get_text(entry));
	if (entry->select.start + entry->select.length > (int)strlen(entry->buffer))
	  entry->select.length = strlen(entry->buffer) - entry->select.start;
	strcpy(&(str2[entry->select.start]), &(entry->buffer[entry->select.start + entry->select.length]));
	e_entry_set_text(entry, str2);
	free(str2);	     
	entry->cursor_pos = entry->select.start;
	entry->select.start = -1;
     }
   e_entry_configure(entry);
}

void
e_entry_delete_to_left(E_Entry *entry)
{
   char *str2;
   
   e_strdup(str2, e_entry_get_text(entry));
   strcpy(&(str2[entry->cursor_pos - 1]), &(entry->buffer[entry->cursor_pos]));
   entry->cursor_pos--;
   e_entry_set_text(entry, str2);
   e_entry_configure(entry);
}

void
e_entry_delete_to_right(E_Entry *entry)
{
   char *str2;
   
   e_strdup(str2, e_entry_get_text(entry));
   strcpy(&(str2[entry->cursor_pos]), &(entry->buffer[entry->cursor_pos + 1]));
   e_entry_set_text(entry, str2);
   free(str2);
   e_entry_configure(entry);
}

char *
e_entry_get_selection(E_Entry *entry)
{
   if (entry->select.start >= 0)
     {
	char *str2;
	int len;
	
	len = entry->select.length;
	if (entry->select.start + entry->select.length >= (int)strlen(entry->buffer))
	  len = strlen(entry->buffer) - entry->select.start;
	str2 = e_memdup(&(entry->buffer[entry->select.start]), len + 1);
	str2[len] = 0;
	return str2;
     }
   return NULL;
}
