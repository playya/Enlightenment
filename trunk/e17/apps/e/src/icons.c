#include "e.h"

void
e_icon_free(E_Icon *icon)
{
   FREE(icon);
}

E_Icon *
e_icon_new(void)
{
   E_Icon *icon;

   icon = NEW(E_Icon, 1);
   ZERO(icon, E_Icon, 1);
   OBJ_INIT(icon, e_icon_free);
   return icon;
}

void
e_icon_calulcate_geometry(E_Icon *icon)
{
}
