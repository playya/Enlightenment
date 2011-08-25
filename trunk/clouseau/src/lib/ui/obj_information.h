#ifndef _OBJ_INFORMATION_H
#define _OBJ_INFORMATION_H

#include "libclouseau.h"

typedef struct _Obj_Information Obj_Information;
struct _Obj_Information
{
   struct {
        const char *name;
        short layer;
        Evas_Coord x, y, w, h;
        double scale;
        Evas_Coord min_w, min_h, max_w, max_h, req_w, req_h;
        double align_x, align_y;
        double weight_x, weight_y;
        int r, g, b, a;
        Eina_Bool is_clipper : 1;
        Eina_Bool is_visible : 1;
   } evas_props;

   enum {
        CLOUSEAU_OBJ_TYPE_OTHER,
        CLOUSEAU_OBJ_TYPE_ELM,
        CLOUSEAU_OBJ_TYPE_TEXT,
        CLOUSEAU_OBJ_TYPE_IMAGE,
        CLOUSEAU_OBJ_TYPE_EDJE,
   } obj_type;

   union {
        struct {
             const char *type;
             const char *style;
             double scale;
             Eina_Bool is_disabled : 1;
             Eina_Bool is_mirrored : 1;
             Eina_Bool is_mirrored_automatic : 1;
        } elm;
        struct {
             const char *font;
             int size;
             const char *source;
        } text;
        struct {
             const char *file, *key;
             void *source;
             const char *load_err;
        } image;
        struct {
             const char *file, *group;
             const char *load_err;
        } edje;
   } extra_props;
};

Evas_Object *clouseau_obj_information_list_add(Evas_Object *parent);
void clouseau_obj_information_list_populate(Tree_Item *treeit);
void clouseau_obj_information_list_clear();

#endif
