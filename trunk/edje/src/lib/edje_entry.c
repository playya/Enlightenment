/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "edje_private.h"

typedef struct _Entry Entry;
typedef struct _Sel Sel;

struct _Entry
{
   Evas_Coord   cx, cy;
   Evas_Object *cursor_bg;
   Evas_Object *cursor_fg;
   Evas_Textblock_Cursor *cursor;
   Evas_Textblock_Cursor *sel_start, *sel_end;
   Evas_List *sel;
   Evas_Bool selecting : 1;
   Evas_Bool have_selection : 1;
};

struct _Sel
{
   Evas_Object *obj_bg;
   Evas_Object *obj_fg;
   Evas_Textblock_Rectangle rect;
};

// FIXME: this has to emit signals for "request selection", "set selection"
// so copy & paste work, need api calls to insert text, insert format,
// get text (with markup) delete text, etc. etc. etc.
// 
// FIXME: cursor when at end of text doesnt display right

static void
_edje_focus_in_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Edje *ed = data;
   _edje_emit(ed, "focus,in", "");
   // FIXME: emit focus,in to selections and cursors
}
    
static void
_edje_focus_out_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Edje *ed = data;
   _edje_emit(ed, "focus,out", "");
   // FIXME: emit focus,out to selections and cursors
}

static void
_curs_back(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   if (evas_textblock_cursor_char_prev(c)) return;
   if (!evas_textblock_cursor_node_prev(c)) return;
   while (evas_textblock_cursor_node_format_get(c))
     {
	if (evas_textblock_cursor_node_format_is_visible_get(c)) break;
	if (!evas_textblock_cursor_node_prev(c)) break;
     }
}

static void
_curs_next(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   if (evas_textblock_cursor_char_next(c)) return;
   if (!evas_textblock_cursor_node_next(c)) return;
   while (evas_textblock_cursor_node_format_get(c))
     {
	if (evas_textblock_cursor_node_format_is_visible_get(c)) break;
	if (!evas_textblock_cursor_node_next(c)) break;
     }
}

static int
_curs_line_last_get(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   Evas_Textblock_Cursor *cc;
   int ln;
   
   cc = evas_object_textblock_cursor_new(o);
   evas_textblock_cursor_node_last(cc);
   ln = evas_textblock_cursor_line_geometry_get(cc, NULL, NULL, NULL, NULL);
   evas_textblock_cursor_free(cc);
   return ln;
}

static void
_curs_jump_line(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en, int ln)
{
   Evas_Coord lx, ly, lw, lh, cx, cy, cw, ch;

   if (ln < 0) ln = 0;
   else
     {
	int last = _curs_line_last_get(c, o, en);
	if (ln > last) ln = last;
     }
   if (!evas_object_textblock_line_number_geometry_get(o, ln, &lx, &ly, &lw, &lh))
     return;
   evas_textblock_cursor_char_geometry_get(c, &cx, &cy, &cw, &ch);
   if (evas_textblock_cursor_char_coord_set(c, cx + (cw / 2), ly + (lh / 2)))
     return;
   evas_textblock_cursor_line_coord_set(c, ly + (lh / 2));
   if (cx + (cw / 2) < (lx + (lw / 2))) evas_textblock_cursor_line_first(c);
   else evas_textblock_cursor_line_last(c);
}

static void
_curs_jump_line_by(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en, int by)
{
   int ln;
   
   ln = evas_textblock_cursor_line_geometry_get(c, NULL, NULL, NULL, NULL) + by;
   _curs_jump_line(c, o, en, ln);
}

static void
_curs_up(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   _curs_jump_line_by(c, o, en, -1);
}

static void
_curs_down(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   _curs_jump_line_by(c, o, en, 1);
}

static void
_curs_lin_start(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   evas_textblock_cursor_line_first(c);
}

static void
_curs_lin_end(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   evas_textblock_cursor_line_last(c);
   if (!evas_textblock_cursor_node_format_get(c))
     _curs_next(c, o, en);
}

static void
_curs_start(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   evas_textblock_cursor_line_set(c, 0);
   evas_textblock_cursor_line_first(c);
}

static void
_curs_end(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   evas_textblock_cursor_line_set(c, _curs_line_last_get(c, o, en));
   _curs_lin_end(c, o, en);
}



static void
_sel_start(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   if (en->sel_start) return;
   en->sel_start = evas_object_textblock_cursor_new(o);
   evas_textblock_cursor_copy(en->cursor, en->sel_start);
   en->sel_end = evas_object_textblock_cursor_new(o);
   evas_textblock_cursor_copy(en->cursor, en->sel_start);
   en->have_selection = 0;
}

static void
_sel_enable(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   if (en->have_selection) return;
   en->have_selection = 1;
   // FIXME: emit "selection changed"
}

static void
_sel_extend(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   if (!en->sel_end) return;
   _sel_enable(c, o, en);
   if (!evas_textblock_cursor_compare(c, en->sel_end)) return;
   evas_textblock_cursor_copy(c, en->sel_end);
   // FIXME: emit "selection changed"
}

static void
_sel_clear(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   if (en->sel_start)
     {
	evas_textblock_cursor_free(en->sel_start);
	evas_textblock_cursor_free(en->sel_end);
	en->sel_start = NULL;
	en->sel_end = NULL;
     }
   en->have_selection = 0;
   while (en->sel)
     {
	Sel *sel;
	
	sel = en->sel->data;
	if (sel->obj_bg) evas_object_del(sel->obj_bg);
	if (sel->obj_fg) evas_object_del(sel->obj_fg);
	free(sel);
	en->sel = evas_list_remove_list(en->sel, en->sel);
     }
}

static void
_sel_update(Evas_Textblock_Cursor *c, Evas_Object *o, Entry *en)
{
   Evas_List *range = NULL, *l;
   Sel *sel;
   Evas_Object *ob;
   Evas_Coord x, y, w, h;
   Evas_Object *smart, *clip;
   
   smart = evas_object_smart_parent_get(o);
   clip = evas_object_clip_get(o);
   if (en->sel_start)
     range = evas_textblock_cursor_range_geometry_get(en->sel_start, en->sel_end);
   if (evas_list_count(range) != evas_list_count(en->sel))
     {
	while (en->sel)
	  {
	     sel = en->sel->data;
	     if (sel->obj_bg) evas_object_del(sel->obj_bg);
	     if (sel->obj_fg) evas_object_del(sel->obj_fg);
	     free(sel);
	     en->sel = evas_list_remove_list(en->sel, en->sel);
	  }
	if (en->have_selection)
	  {
	     for (l = range; l; l = l->next)
	       {
		  sel = calloc(1, sizeof(Sel));
		  en->sel = evas_list_append(en->sel, sel);
		  ob = evas_object_rectangle_add(evas_object_evas_get(o));
		  evas_object_smart_member_add(ob, smart);
		  evas_object_stack_below(ob, o);
		  evas_object_clip_set(ob, clip);
		  evas_object_color_set(ob, 20, 20, 255, 150);
		  evas_object_pass_events_set(ob, 1);
		  evas_object_show(ob);
		  sel->obj_bg = ob;
	       }
	  }
     }
   x = y = w = h = -1;
   evas_object_geometry_get(o, &x, &y, &w, &h);
   for (l = en->sel; l; l = l->next)
     {
	Evas_Textblock_Rectangle *r;

	sel = l->data;
	r = range->data;
	if (sel->obj_bg)
	  {
	     evas_object_move(sel->obj_bg, x + r->x, y + r->y);
	     evas_object_resize(sel->obj_bg, r->w, r->h);
	  }
	if (sel->obj_fg)
	  {
	     evas_object_move(sel->obj_fg, x + r->x, y + r->y);
	     evas_object_resize(sel->obj_fg, r->w, r->h);
	  }
	range = evas_list_remove_list(range, range);
	free(r);
     }
}

static void
_edje_key_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Edje *ed = data;
   Evas_Event_Key_Down *ev = event_info;
   Edje_Real_Part *rp = ed->focused_part;
   Entry *en;
   Evas_Bool control, alt, shift;
   Evas_Bool multiline;
   if (!rp) return;
   en = rp->entry_data;
   if ((!en) || (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK) ||
       (rp->part->entry_mode < EDJE_ENTRY_EDIT_MODE_EDITABLE))
     return;
   if (!ev->key) return;
   
   printf("editable: '%s' '%s' '%s'\n", ev->key, ev->string, ev->compose);
   control = evas_key_modifier_is_set(ev->modifiers, "Control");
   alt = evas_key_modifier_is_set(ev->modifiers, "Alt");
   shift = evas_key_modifier_is_set(ev->modifiers, "Shift");
   multiline = rp->part->entry_mode;
   if (!strcmp(ev->key, "Escape"))
     {
	// dead keys here. Escape for now (should emit these)
     }
   else if ((!strcmp(ev->key, "Up")) && (multiline))
     {
	Evas_Coord lx, ly, lw, lh, cx, cy, cw, ch;
	int line_num;
	
	if (shift) _sel_start(en->cursor, rp->object, en);
	_curs_up(en->cursor, rp->object, en);
	if (shift) _sel_extend(en->cursor, rp->object, en);
     }
   else if ((!strcmp(ev->key, "Down")) && (multiline))
     {
	Evas_Coord lx, ly, lw, lh, cx, cy, cw, ch;
	int line_num;
	
	if (shift) _sel_start(en->cursor, rp->object, en);
	_curs_down(en->cursor, rp->object, en);
	if (shift) _sel_extend(en->cursor, rp->object, en);
     }
   else if (!strcmp(ev->key, "Left"))
     {
	if (shift) _sel_start(en->cursor, rp->object, en);
	_curs_back(en->cursor, rp->object, en);
	if (shift) _sel_extend(en->cursor, rp->object, en);
     }
   else if (!strcmp(ev->key, "Right"))
     {
	if (shift) _sel_start(en->cursor, rp->object, en);
	_curs_next(en->cursor, rp->object, en);
	if (shift) _sel_extend(en->cursor, rp->object, en);
     }
   else if (!strcmp(ev->key, "BackSpace"))
     {
	if (control)
	  {
	     // del to start of previous word
	  }
	else if ((alt) && (shift))
	  {
	     // undo last action
	  }
	else
	  {
	     if (en->have_selection)
	       evas_textblock_cursor_range_delete(en->sel_start, en->sel_end);
	     else
	       {
		  _curs_back(en->cursor, rp->object, en);
		  evas_textblock_cursor_char_delete(en->cursor);
	       }
	  }
	_sel_clear(en->cursor, rp->object, en);
     }
   else if (!strcmp(ev->key, "Delete"))
     {
	if (control)
	  {
	     // del to end of next word
	  }
	else if (shift)
	  {
	     // cut
	  }
	else
	  {	
	     if (en->have_selection)
	       evas_textblock_cursor_range_delete(en->sel_start, en->sel_end);
	     else
	       evas_textblock_cursor_char_delete(en->cursor);
	  }
	_sel_clear(en->cursor, rp->object, en);
     }
   else if (!strcmp(ev->key, "Home"))
     {
	if (shift) _sel_start(en->cursor, rp->object, en);
	if ((control) && (multiline))
	  _curs_start(en->cursor, rp->object, en);
	else
	  _curs_lin_start(en->cursor, rp->object, en);
	if (shift) _sel_extend(en->cursor, rp->object, en);
     }
   else if (!strcmp(ev->key, "End"))
     {
	if (shift) _sel_start(en->cursor, rp->object, en);
	if ((control) && (multiline))
	  _curs_end(en->cursor, rp->object, en);
	else
	  _curs_lin_end(en->cursor, rp->object, en);
	if (shift) _sel_extend(en->cursor, rp->object, en);
     }
   else if ((control) && (!strcmp(ev->key, "v")))
     {
	// paste
     }
   else if ((control) && ((!strcmp(ev->key, "c") || (!strcmp(ev->key, "Insert")))))
     {
	// copy
     }
   else if ((control) && ((!strcmp(ev->key, "x") || (!strcmp(ev->key, "m")))))
     {
	// cut
     }
   else if ((control) && (!strcmp(ev->key, "z")))
     {
	if (shift)
	  {
	     // redo
	  }
	else
	  {
	     // undo
	  }
     }
   else if ((control) && (!strcmp(ev->key, "y")))
     {
	// redo
     }
   else if ((control) && (!strcmp(ev->key, "w")))
     {
	_sel_clear(en->cursor, rp->object, en);
	// select current word
     }
   else if ((!strcmp(ev->key, "Tab")) && (multiline))
     {
	if (shift)
	  {
	     // remove a tab
	  }
	else
	  {
	     evas_textblock_cursor_format_prepend(en->cursor, "\t");
	     _curs_next(en->cursor, rp->object, en);
	  }
     }
   else if ((!strcmp(ev->key, "ISO_Left_Tab")) && (multiline))
     { 
	// remove a tab
     }
   else if ((!strcmp(ev->key, "Prior")) && (multiline))
     {
	if (shift) _sel_start(en->cursor, rp->object, en);
	_curs_jump_line_by(en->cursor, rp->object, en, -10);
	if (shift) _sel_extend(en->cursor, rp->object, en);
     }
   else if ((!strcmp(ev->key, "Next")) && (multiline))
     {
	if (shift) _sel_start(en->cursor, rp->object, en);
	_curs_jump_line_by(en->cursor, rp->object, en, 10);
	if (shift) _sel_extend(en->cursor, rp->object, en);
     }
   else if (((!strcmp(ev->key, "Return")) || (!strcmp(ev->key, "KP_Enter"))) && (multiline))
     {
	evas_textblock_cursor_format_prepend(en->cursor, "\n");
        _curs_next(en->cursor, rp->object, en);
     }
   else if ((!strcmp(ev->key, "Multi_key")))
     {
	// FIXME: compose next 2 keystrokes (Examples):
	// a " -> ä
	// o / -> ø
	// a e -> æ
	// e ' -> é
	// s s -> ß
	// etc.
     }
   // FIXME: other input methods? (xim, scim, uim etc.)...
   else
     {
	// FIXME: if composing.. store 2 keys
	if (ev->string)
	  {
            if (en->have_selection)
	       evas_textblock_cursor_range_delete(en->sel_start, en->sel_end);
	     evas_textblock_cursor_text_prepend(en->cursor, ev->string);
	     _sel_clear(en->cursor, rp->object, en);
	  }
     }
   _edje_entry_real_part_configure(rp);
}

static void
_edje_key_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Edje *ed = data;
   Evas_Event_Key_Up *ev = event_info;
   Edje_Real_Part *rp = ed->focused_part;
   Entry *en;
   if (!rp) return;
   en = rp->entry_data;
   if ((!en) || (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK) ||
       (rp->part->entry_mode < EDJE_ENTRY_EDIT_MODE_EDITABLE))
     return;
}

static void
_edje_part_mouse_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Edje_Real_Part *rp = data;
   Evas_Event_Mouse_Down *ev = event_info;
   Entry *en;
   Evas_Coord x, y, w, h;
   Evas_Bool multiline;
   if (!rp) return;
   en = rp->entry_data;
   if ((!en) || (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK) ||
       (rp->part->entry_mode < EDJE_ENTRY_EDIT_MODE_SELECTABLE))
     return;
   // double click -> select word
   // triple click -> select line
   multiline = rp->part->entry_mode;
   evas_object_geometry_get(rp->object, &x, &y, &w, &h);
   en->cx = ev->canvas.x - x;
   en->cy = ev->canvas.y - y;
   if (!evas_textblock_cursor_char_coord_set(en->cursor, en->cx, en->cy))
     {
	Evas_Coord lx, ly, lw, lh;
	
	evas_textblock_cursor_line_coord_set(en->cursor, en->cy);
	evas_textblock_cursor_line_geometry_get(en->cursor, &lx, &ly, &lw, &lh);
	if (en->cx <= lx)
	  _curs_lin_start(en->cursor, rp->object, en);
	else
	  _curs_lin_end(en->cursor, rp->object, en);
     }
   en->selecting = 1;
   _sel_clear(en->cursor, rp->object, en);
   _sel_start(en->cursor, rp->object, en);
   _edje_entry_real_part_configure(rp);
}

static void
_edje_part_mouse_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Edje_Real_Part *rp = data;
   Evas_Event_Mouse_Up *ev = event_info;
   Entry *en;
   Evas_Coord x, y, w, h;
   Evas_Bool multiline;
   if (!rp) return;
   en = rp->entry_data;
   if ((!en) || (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK) ||
       (rp->part->entry_mode < EDJE_ENTRY_EDIT_MODE_SELECTABLE))
     return;
   multiline = rp->part->entry_mode;
   evas_object_geometry_get(rp->object, &x, &y, &w, &h);
   en->cx = ev->canvas.x - x;
   en->cy = ev->canvas.y - y;
   if (!evas_textblock_cursor_char_coord_set(en->cursor, en->cx, en->cy))
     {
	Evas_Coord lx, ly, lw, lh;
	
	evas_textblock_cursor_line_coord_set(en->cursor, en->cy);
	evas_textblock_cursor_line_geometry_get(en->cursor, &lx, &ly, &lw, &lh);
	if (en->cx <= lx)
	  _curs_lin_start(en->cursor, rp->object, en);
	else
	  _curs_lin_end(en->cursor, rp->object, en);
     }
   evas_textblock_cursor_copy(en->cursor, en->sel_end);
   en->selecting = 0;
   _edje_entry_real_part_configure(rp);
}

static void
_edje_part_mouse_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Edje_Real_Part *rp = data;
   Evas_Event_Mouse_Move *ev = event_info;
   Entry *en;
   Evas_Coord x, y, w, h;
   if (!rp) return;
   Evas_Bool multiline;
   en = rp->entry_data;
   if ((!en) || (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK) ||
       (rp->part->entry_mode < EDJE_ENTRY_EDIT_MODE_SELECTABLE))
     return;
   multiline = rp->part->entry_mode;
   if (!en->selecting) return;
   evas_object_geometry_get(rp->object, &x, &y, &w, &h);
   en->cx = ev->cur.canvas.x - x;
   en->cy = ev->cur.canvas.y - y;
   if (!evas_textblock_cursor_char_coord_set(en->cursor, en->cx, en->cy))
     {
	Evas_Coord lx, ly, lw, lh;
	
	evas_textblock_cursor_line_coord_set(en->cursor, en->cy);
	evas_textblock_cursor_line_geometry_get(en->cursor, &lx, &ly, &lw, &lh);
	if (en->cx <= lx)
	  _curs_lin_start(en->cursor, rp->object, en);
	else
	  _curs_lin_end(en->cursor, rp->object, en);
     }
   _sel_extend(en->cursor, rp->object, en);
   if (evas_textblock_cursor_compare(en->sel_start, en->sel_end) != 0)
     _sel_enable(en->cursor, rp->object, en);
   if (en->have_selection)
     _sel_update(en->cursor, rp->object, en);
   _edje_entry_real_part_configure(rp);
}

/***************************************************************/
void
_edje_entry_init(Edje *ed)
{
   evas_object_event_callback_add(ed->obj, EVAS_CALLBACK_FOCUS_IN, _edje_focus_in_cb, ed);
   evas_object_event_callback_add(ed->obj, EVAS_CALLBACK_FOCUS_OUT, _edje_focus_out_cb, ed);
   evas_object_event_callback_add(ed->obj, EVAS_CALLBACK_KEY_DOWN, _edje_key_down_cb, ed);
   evas_object_event_callback_add(ed->obj, EVAS_CALLBACK_KEY_UP, _edje_key_up_cb, ed);
}

void
_edje_entry_shutdown(Edje *ed)
{
}

void
_edje_entry_real_part_init(Edje_Real_Part *rp)
{
   Entry *en;
   
   en = calloc(1, sizeof(Entry));
   if (!en) return;
   rp->entry_data = en;

   evas_object_event_callback_add(rp->object, EVAS_CALLBACK_MOUSE_DOWN, _edje_part_mouse_down_cb, rp);
   evas_object_event_callback_add(rp->object, EVAS_CALLBACK_MOUSE_UP, _edje_part_mouse_up_cb, rp);
   evas_object_event_callback_add(rp->object, EVAS_CALLBACK_MOUSE_MOVE, _edje_part_mouse_move_cb, rp);

   // FIXME: make cursor object correct
   en->cursor_bg = evas_object_rectangle_add(evas_object_evas_get(rp->object));
   evas_object_smart_member_add(en->cursor_bg, rp->edje->obj);
   evas_object_stack_below(en->cursor_bg, rp->object);
   evas_object_clip_set(en->cursor_bg, evas_object_clip_get(rp->object));
   evas_object_color_set(en->cursor_bg, 255, 20, 20, 150);
   evas_object_pass_events_set(en->cursor_bg, 1);
   if (rp->part->entry_mode >= EDJE_ENTRY_EDIT_MODE_EDITABLE)
     evas_object_show(en->cursor_bg);
   
   en->cursor = evas_object_textblock_cursor_get(rp->object);
}

void
_edje_entry_real_part_shutdown(Edje_Real_Part *rp)
{
   Entry *en = rp->entry_data;
   if (!en) return;
   rp->entry_data = NULL;
   // FIXME: delete cursor objects, sel cursors and en->sel's undo buffer copy/cut buffer
   free(en);
}

void
_edje_entry_real_part_configure(Edje_Real_Part *rp)
{
   Evas_Coord x, y, w, h, xx, yy, ww, hh;
   Entry *en = rp->entry_data;
   if (!en) return;

   _sel_update(en->cursor, rp->object, en);
   x = y = w = h = -1;
   xx = yy = ww = hh = -1;
   evas_object_geometry_get(rp->object, &x, &y, &w, &h);
   evas_textblock_cursor_char_geometry_get(en->cursor, &xx, &yy, &ww, &hh);
   if (ww < 1) ww = 1;
   if (hh < 1) ww = 1;
   if (en->cursor_bg)
     {
	evas_object_move(en->cursor_bg, x + xx, y + yy);
	evas_object_resize(en->cursor_bg, ww, hh);
     }
   if (en->cursor_fg)
     {
	evas_object_move(en->cursor_fg, x + xx, y + yy);
	evas_object_resize(en->cursor_fg, ww, hh);
     }
   // FIXME: move anchor objects based on cursor geometry
}

char *
_edje_entry_selection_get(Edje_Real_Part *rp)
{
   Entry *en = rp->entry_data;
   if (!en) return;
   // get selection - convert to markup
}

const char *
_edje_entry_text_get(Edje_Real_Part *rp)
{
   Entry *en = rp->entry_data;
   if (!en) return;
   // get text - convert to markup
   return evas_object_textblock_text_markup_get(rp->object);
}

void
_edje_entry_text_markup_set(Edje_Real_Part *rp, const char *text)
{
   Entry *en = rp->entry_data;
   if (!en) return;
   // set text as markup
   evas_object_textblock_text_markup_set(rp->object, text);
   // walk textblock nodes
   // in walk find anything of style "a href=XXX" and matching end tag
   // add 2 cursors and save this link in a list
}

void
_edje_entry_set_cursor_start(Edje_Real_Part *rp)
{
   Entry *en = rp->entry_data;
   if (!en) return;
   _curs_start(en->cursor, rp->object, en);
}

void
_edje_entry_set_cursor_end(Edje_Real_Part *rp)
{
   Entry *en = rp->entry_data;
   if (!en) return;
   _curs_end(en->cursor, rp->object, en);
}

// FIXME: apis 's query anchor stuff???
