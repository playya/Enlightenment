#ifndef __PANEL_H__
#define __PANEL_H__

void e_slide_panel_in(int v, void *data);
void e_slide_panel_out(int v, void *data);
void show_panel(void *_data, Evas *_e, Evas_Object *_o, void *event_info);
void hide_panel(void *_data, Evas *_e, Evas_Object *_o, void *event_info);
void setup_panel(Evas *_e);

Panel_Button *panel_button(Evas *_e, char *_label, Evas_List *tests);
void panel_button_free(Panel_Button * pbutton);

#endif				/* __PANEL_H__ */
