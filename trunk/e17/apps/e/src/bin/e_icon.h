/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifdef E_TYPEDEFS
#else
#ifndef E_ICON_H
#define E_ICON_H

EAPI Evas_Object *e_icon_add              (Evas *evas);
EAPI void         e_icon_file_set         (Evas_Object *obj, const char *file);
EAPI const char  *e_icon_file_get         (Evas_Object *obj);
EAPI void         e_icon_smooth_scale_set (Evas_Object *obj, int smooth);
EAPI int          e_icon_smooth_scale_get (Evas_Object *obj);
EAPI void         e_icon_size_get         (Evas_Object *obj, int *w, int *h);
EAPI int          e_icon_fill_inside_get  (Evas_Object *obj);
EAPI void         e_icon_fill_inside_set  (Evas_Object *obj, int fill_inside);

#endif
#endif
