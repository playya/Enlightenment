#ifndef E_PATH_H
#define E_PATH_H

typedef struct _E_Path E_Path;
   
struct _E_Path
{
   E_Object   e_obj_inherit;
   
   Evas_Hash *hash;
   
   Evas_List *dir_list;
};

EAPI E_Path     *e_path_new(void);
EAPI void        e_path_path_append(E_Path *ep, const char *path);
EAPI void        e_path_path_prepend(E_Path *ep, const char *path);
EAPI void        e_path_path_remove(E_Path *ep, const char *path);
EAPI const char *e_path_find(E_Path *ep, const char *file);

EAPI void        e_path_evas_append(E_Path *ep, Evas *evas);
#endif
