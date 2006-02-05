/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifdef E_TYPEDEFS

typedef enum _E_Gadcon_Orient
{
   /* generic orientations */
   E_GADCON_ORIENT_HORIZ,
     E_GADCON_ORIENT_VERT,
     /* specific oreintations */
     E_GADCON_ORIENT_LEFT,
     E_GADCON_ORIENT_RIGHT,
     E_GADCON_ORIENT_TOP,
     E_GADCON_ORIENT_BOTTOM
} E_Gadcon_Orient;

typedef struct _E_Gadcon              E_Gadcon;
typedef struct _E_Gadcon_Client       E_Gadcon_Client;
typedef struct _E_Gadcon_Client_Class E_Gadcon_Client_Class;

#else
#ifndef E_GADCON_H
#define E_GADCON_H

#define E_GADCON_TYPE 0xE0b01022
#define E_GADCON_CLIENT_TYPE 0xE0b01023

struct _E_Gadcon
{
   E_Object             e_obj_inherit;

   char                *name;
   char                *id;
   
   struct {
      Evas_Object      *o_parent;
      char             *swallow_name;
   } edje;
   
   E_Gadcon_Orient      orient;
   
   Evas                *evas;
   Evas_Object         *o_container;
   Evas_List           *clients;
};

#define GADCON_CLIENT_CLASS_VERSION 1
struct _E_Gadcon_Client_Class
{
   int   version;
   char *name;
   struct {
      E_Gadcon_Client *(*init)     (E_Gadcon *gc, char *name, char *id);
      void             (*shutdown) (E_Gadcon_Client *gcc);
      void             (*orient)   (E_Gadcon_Client *gcc);
   } func;
};

struct _E_Gadcon_Client
{
   E_Object               e_obj_inherit;
   E_Gadcon              *gadcon;
   char                  *name;
   char                  *id;
   Evas_Object           *o_base;
   E_Gadcon_Client_Class  client_class;
   void                  *data;
};

EAPI int              e_gadcon_init(void);
EAPI int              e_gadcon_shutdown(void);
EAPI void             e_gadcon_provider_register(E_Gadcon_Client_Class *cc);
EAPI void             e_gadcon_provider_unregister(E_Gadcon_Client_Class *cc);
EAPI E_Gadcon        *e_gadcon_swallowed_new(char *name, char *id, Evas_Object *obj, char *swallow_name);
EAPI void             e_gadcon_populate(E_Gadcon *gc);
EAPI void             e_gadcon_orient(E_Gadcon *gc, E_Gadcon_Orient orient);
    
EAPI E_Gadcon_Client *e_gadcon_client_new(E_Gadcon *gc, char *name, char *id, Evas_Object *base_obj);
EAPI void             e_gadcon_client_size_request(E_Gadcon_Client *gcc, Evas_Coord w, Evas_Coord h);
EAPI void             e_gadcon_client_min_size_set(E_Gadcon_Client *gcc, Evas_Coord w, Evas_Coord h);
EAPI void             e_gadcon_client_max_size_set(E_Gadcon_Client *gcc, Evas_Coord w, Evas_Coord h);

#endif
#endif

