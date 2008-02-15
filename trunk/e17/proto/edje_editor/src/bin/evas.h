#ifndef _EDJE_EDITOR_EVAS_H_
#define _EDJE_EDITOR_EVAS_H_

void ev_redraw(void);
void ev_draw_focus(void);
void ev_draw_part(Engrave_Part* part);
void ev_resize_fake(int w, int h);
void ev_move_fake(int x, int y);
void ev_hide_group(Engrave_Group* group);
void prepare_canvas(void);

#endif
