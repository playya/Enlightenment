#ifndef EKE_GUI_EDJE_ITEM_H_
#define EKE_GUI_EDJE_ITEM_H_

#include <Evas.h>

typedef struct _Eke_Gui_Edje_Item Eke_Gui_Edje_Item;

struct _Eke_Gui_Edje_Item
{
    Evas_Object *obj;
    Evas_Coord x, y, w, h;
};

Evas_Object*
eke_gui_edje_item_new(Evas *e, const char *file, const char *group);
void
eke_gui_edje_item_init(Evas_Object *o, const char *label, const char *txt,
const char *body);

#endif
