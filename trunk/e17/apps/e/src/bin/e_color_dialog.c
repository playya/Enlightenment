/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "e.h"

static void _e_color_dialog_button1_click(void *data, E_Dialog *edia);
static void _e_color_dialog_button2_click(void *data, E_Dialog *edia);
static void _e_color_dialog_free(E_Color_Dialog *dia);
static void _e_color_dialog_dia_del(E_Object *obj);

/**
 * Create a color selector dialog.
 *
 * @param con container to display on
 * @param color color to initialize to (or NULL for black). 
 */
E_Color_Dialog  *
e_color_dialog_new (E_Container *con, const E_Color *color) 
{
   E_Color_Dialog *dia;
   Evas_Object *o;
   Evas_Coord mw, mh;

   dia = E_OBJECT_ALLOC(E_File_Dialog, E_COLOR_DIALOG_TYPE, _e_color_dialog_free);
   if(!dia) return NULL;
   dia->dia = e_dialog_new(con, "E", "_test");
   e_dialog_title_set(dia->dia, "Color Selector");

   dia->color = calloc(1, sizeof(E_Color));

   if (color)
     e_color_copy(color, dia->color);
   else
      dia->color->a = 255;

   o = e_widget_csel_add(dia->dia->win->evas, dia->color);
   evas_object_show(o);
   e_widget_min_size_get(o, &mw, &mh);
   e_dialog_content_set(dia->dia, o, 460, 260);

   /* buttons at the bottom */
   e_dialog_button_add(dia->dia, "OK", NULL,  _e_color_dialog_button1_click, dia);
   e_dialog_button_add(dia->dia, "Cancel", NULL,  _e_color_dialog_button2_click, dia);
   e_dialog_resizable_set(dia->dia, 1);
   e_win_centered_set(dia->dia->win, 1);

   dia->dia->data = dia;
   e_object_del_attach_func_set(E_OBJECT(dia->dia), _e_color_dialog_dia_del);

   return dia;
}

void
e_color_dialog_show (E_Color_Dialog *dia)
{
   e_dialog_show(dia->dia);
}

void
e_color_dialog_title_set (E_Color_Dialog *dia, const char *title)
{
   e_dialog_title_set(dia->dia, title);
}

void
e_color_dialog_select_callback_add(E_Color_Dialog *dia, void (*func)(E_Color_Dialog *dia, E_Color *color, void *data), void *data)
{
   dia->select_func = func;
   dia->select_data = data;
}

void
e_color_dialog_cancel_callback_add(E_Color_Dialog *dia, void (*func)(E_Color_Dialog *dia, E_Color *color, void *data), void *data)
{
   dia->cancel_func = func;
   dia->cancel_data = data;
}

static void
_e_color_dialog_button1_click(void *data, E_Dialog *edia)
{
   E_Color_Dialog *dia;
   
   dia = data;
   if(dia->select_func && dia->color)
     dia->select_func(dia, dia->color, dia->select_data);
   _e_color_dialog_free(dia);
}

static void
_e_color_dialog_button2_click(void *data, E_Dialog *edia)
{
   E_Color_Dialog *dia;

   dia = data;
   if(dia->cancel_func && dia->color)
     dia->cancel_func(dia, dia->color, dia->cancel_data);
   _e_color_dialog_free(data);     	
}

static void
_e_color_dialog_free(E_Color_Dialog *dia)
{
   e_object_unref(E_OBJECT(dia->dia));
   E_FREE(dia->color);
   E_FREE(dia);
}

static void
_e_color_dialog_dia_del(E_Object *obj)
{
   E_Dialog *dia = obj;
   E_Color_Dialog *cdia = dia->data;
   _e_color_dialog_button2_click(cdia, dia);
}
