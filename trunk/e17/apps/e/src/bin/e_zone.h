#ifdef E_TYPEDEFS

typedef struct _E_Zone     E_Zone;

#else
#ifndef E_ZONE_H
#define E_ZONE_H

struct _E_Zone
{
   E_Object             e_obj_inherit;

   int                  x, y, w, h;
   char                *name;
   int                  num;
   E_Container         *container;

   Evas_Object         *bg_object;
   Evas_Object         *bg_event_object;
   
   int                  desk_x_count, desk_y_count;
   int                  desk_x_current, desk_y_current;
   E_Desk             **desks;
   Evas_List           *clients;

};

EAPI int        e_zone_init(void);
EAPI int        e_zone_shutdown(void);
EAPI E_Zone    *e_zone_new(E_Container *con, int x, int y, int w, int h);
EAPI void       e_zone_move(E_Zone *zone, int x, int y);
EAPI void       e_zone_resize(E_Zone *zone, int w, int h);
EAPI void       e_zone_move_resize(E_Zone *zone, int x, int y, int w, int h);
EAPI E_Zone    *e_zone_current_get(E_Container *con);
EAPI void       e_zone_bg_reconfigure(E_Zone *zone);
EAPI Evas_List *e_zone_clients_list_get(E_Zone *zone);
EAPI void       e_zone_desk_count_set(E_Zone *zone, int x_count, int y_count);
EAPI void       e_zone_desk_count_get(E_Zone *zone, int *x_count, int *y_count);

#endif
#endif
