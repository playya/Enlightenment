#ifdef E_TYPEDEFS

typedef enum _E_Gadman_Policy
{
   /* type */
   E_GADMAN_POLICY_ANYWHERE = 0, 
   E_GADMAN_POLICY_EDGES = 1, 
   E_GADMAN_POLICY_LEFT_EDGE = 2, 
   E_GADMAN_POLICY_RIGHT_EDGE = 3, 
   E_GADMAN_POLICY_TOP_EDGE = 4, 
   E_GADMAN_POLICY_BOTTOM_EDGE = 5, 
   /* extra flags */
   E_GADMAN_POLICY_FIXED_ZONE = 1 << 8,
   E_GADMAN_POLICY_HSIZE = 1 << 9,
   E_GADMAN_POLICY_VSIZE = 1 << 10,
   E_GADMAN_POLICY_HMOVE = 1 << 11,
   E_GADMAN_POLICY_VMOVE = 1 << 12
} E_Gadman_Policy;

typedef enum _E_Gadman_Change
{
   E_GADMAN_CHANGE_MOVE,
   E_GADMAN_CHANGE_RESIZE
} E_Gadman_Change;

typedef enum _E_Gadman_Mode
{
   E_GADMAN_MODE_NORMAL,
   E_GADMAN_MODE_EDIT
} E_Gadman_Mode;

typedef struct _E_Gadman        E_Gadman;
typedef struct _E_Gadman_Client E_Gadman_Client;

#else
#ifndef E_GADMAN_H
#define E_GADMAN_H

struct _E_Gadman
{
   E_Object             e_obj_inherit;
   E_Container         *container;
   Evas_List           *clients;
   E_Gadman_Mode        mode;
};

struct _E_Gadman_Client
{
   E_Object             e_obj_inherit;
   E_Gadman            *gadman;
   
   Evas_Object         *control_object;
   Evas_Object         *event_object;
   E_Menu              *menu;
   Evas_Coord           down_x, down_y;
   Evas_Coord           down_store_x, down_store_y, down_store_w, down_store_h;
   unsigned char        moving : 1;
   unsigned char        resizing_l : 1;
   unsigned char        resizing_r : 1;
   unsigned char        resizing_u : 1;
   unsigned char        resizing_d : 1;
   char                *domain;
   E_Zone              *zone;
   int                  instance;
   E_Gadman_Policy      policy;
   Evas_Coord           x, y, w, h;
   Evas_Coord           minw, minh, maxw, maxh;
   double               ax, ay;
   double               mina, maxa;
   void               (*func) (void *data, E_Gadman_Client *gmc, E_Gadman_Change change);
   void                *data;
};

EAPI int              e_gadman_init(void);
EAPI int              e_gadman_shutdown(void);
EAPI void             e_gadman_mode_set(E_Gadman *gm, E_Gadman_Mode mode);
EAPI E_Gadman_Mode    e_gadman_mode_get(E_Gadman *gm);
EAPI E_Gadman        *e_gadman_new(E_Container *con);
EAPI E_Gadman_Client *e_gadman_client_new(E_Gadman *gm);
EAPI void             e_gadman_client_save(E_Gadman_Client *gmc);
EAPI void             e_gadman_client_load(E_Gadman_Client *gmc);
EAPI void             e_gadman_client_domain_set(E_Gadman_Client *gmc, char *domain, int instance);
EAPI void             e_gadman_client_zone_set(E_Gadman_Client *gmc, E_Zone *zone);
EAPI void             e_gadman_client_policy_set(E_Gadman_Client *gmc, E_Gadman_Policy pol);
EAPI void             e_gadman_client_min_size_set(E_Gadman_Client *gmc, Evas_Coord minw, Evas_Coord minh);
EAPI void             e_gadman_client_max_size_set(E_Gadman_Client *gmc, Evas_Coord maxw, Evas_Coord maxh);
EAPI void             e_gadman_client_align_set(E_Gadman_Client *gmc, double xalign, double yalign);
EAPI void             e_gadman_client_aspect_set(E_Gadman_Client *gmc, double mina, double maxa);
EAPI void             e_gadman_client_geometry_get(E_Gadman_Client *gmc, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);
EAPI void             e_gadman_client_change_func_set(E_Gadman_Client *gmc, void (*func) (void *data, E_Gadman_Client *gmc, E_Gadman_Change change), void *data);
EAPI E_Menu          *e_gadman_client_menu_new(E_Gadman_Client *gmc);

#endif
#endif
