#ifndef ESMART_TEXTAREA_H
#define ESMART_TEXTAREA_H

#include <stdio.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_X.h>
#include <Ecore_Evas.h>
#include <Edje.h>

enum Esmart_Textarea_Key_Modifiers           /* several key modifiers */
{
     ESMART_TEXTAREA_KEY_MODIFIER_SHIFT = 0x1,
     ESMART_TEXTAREA_KEY_MODIFIER_CTRL = 0x2,
     ESMART_TEXTAREA_KEY_MODIFIER_ALT = 0x4,
     ESMART_TEXTAREA_KEY_MODIFIER_MOD = 0x8,
     ESMART_TEXTAREA_KEY_MODIFIER_WIN = 0x10,
};

enum Esmart_Textarea_Mouse_Modifiers        /* several mouse modifiers */
{
     ESMART_TEXTAREA_MOUSE_MODIFIER_LEFT = 0x1,
     ESMART_TEXTAREA_MOUSE_MODIFIER_MIDDLE = 0x2,
     ESMART_TEXTAREA_MOUSE_MODIFIER_RIGHT = 0x4,
};

struct _Esmart_Text_Area {                  /* our typical text area */   
   Evas_Object  *text;
   Evas_Object  *bg;
   Evas_Object  *cursor;      
   unsigned int  key_modifiers;
   unsigned int  in_selection;
   unsigned int  mouse_modifiers;
};

typedef struct _Esmart_Text_Area Esmart_Text_Area;

/* text area public api */
Evas_Object    *esmart_textarea_add(Evas *evas);
void            esmart_textarea_cursor_goto_cursor(Evas_Object *o);
void            esmart_textarea_cursor_move_left(Evas_Object *o);
void            esmart_textarea_cursor_move_right(Evas_Object *o);
void            esmart_textarea_cursor_move_down(Evas_Object *o);
void            esmart_textarea_cursor_move_up(Evas_Object *o);
void            esmart_textarea_cursor_move_home(Evas_Object *o);
void            esmart_textarea_cursor_move_end(Evas_Object *o);
void            esmart_textarea_cursor_delete_right(Evas_Object *o);
void            esmart_textarea_cursor_delete_left(Evas_Object *o);
void            esmart_textarea_focus_set(Evas_Object *o, Evas_Bool focus);  
void            esmart_textarea_bg_set(Evas_Object *o, Evas_Object *bg);
void            esmart_textarea_text_insert(Evas_Object *o, const char *text);
void            esmart_textarea_cursor_set(Evas_Object *o, Evas_Object *c);
void            esmart_textarea_clear(Evas_Object *o);
void            esmart_textarea_cursor_pos_set(Evas_Object *o, int pos);
int             esmart_textarea_cursor_pos_get(Evas_Object *o);
int             esmart_textarea_length_get(Evas_Object *o);
int             esmart_textarea_cursor_line_get(Evas_Object *o);
int             esmart_textarea_lines_get(Evas_Object *o);
int             esmart_textarea_line_start_pos_get(Evas_Object *o);
int             esmart_textarea_line_end_pos_get(Evas_Object *o);
Evas_Bool       esmart_textarea_line_get(Evas_Object *o, int line, Evas_Coord *lx, Evas_Coord *ly, Evas_Coord *lw, Evas_Coord *lh);
Evas_Bool       esmart_textarea_char_pos_get(Evas_Object *o, int pos, Evas_Coord *lx, Evas_Coord *ly, Evas_Coord *lw, Evas_Coord *lh);
int             esmart_textarea_char_coords_get(Evas_Object *o, Evas_Coord x, Evas_Coord y, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch);
void            esmart_textarea_text_insert(Evas_Object *o, const char *text);
char           *esmart_textarea_text_get(Evas_Object *o, int len);
void            esmart_textarea_text_del(Evas_Object *o, int len);
void            esmart_textarea_format_insert(Evas_Object *o, const char *format);
int             esmart_textarea_format_next_pos_get(Evas_Object *o);
int             esmart_textarea_format_next_count_get(Evas_Object *o);
const char     *esmart_textarea_format_next_get(Evas_Object *o, int n);
void            esmart_textarea_format_next_del(Evas_Object *o, int n);
int             esmart_textarea_format_prev_pos_get(Evas_Object *o);
int             esmart_textarea_format_prev_count_get(Evas_Object *o);
const char     *esmart_textarea_format_prev_get(Evas_Object *o, int n);
void            esmart_textarea_format_prev_del(Evas_Object *o, int n);
char           *esmart_textarea_format_current_get(Evas_Object *o);
void            esmart_textarea_format_size_get(Evas_Object *o, Evas_Coord *w, Evas_Coord *h);
void            esmart_textarea_native_size_get(Evas_Object *o, Evas_Coord *w, Evas_Coord *h);
int             esmart_textarea_native_lines_get(Evas_Object *o);

#endif
