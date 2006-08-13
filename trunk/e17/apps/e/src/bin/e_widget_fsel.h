/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifdef E_TYPEDEFS
#else
#ifndef E_WIDGET_FSEL_H
#define E_WIDGET_FSEL_H

EAPI Evas_Object *e_widget_fsel_add(Evas *evas, char *dev, char *path, char *selected, char *filter,
				    void (*sel_func) (void *data, Evas_Object *obj), void *sel_data,
				    void (*chg_func) (void *data, Evas_Object *obj), void *chg_data, int preview);
EAPI const char *e_widget_fsel_selection_path_get(Evas_Object *obj);
    
#endif
#endif
