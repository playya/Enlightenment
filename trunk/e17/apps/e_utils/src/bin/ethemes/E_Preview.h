#ifndef _E_PREVIEW_H
#define _E_PREVIEW_H

#include <Evas.h>

/**
 * E_Preview - This is a smart object to be used to preview Enligthtenment DR17
 * themes. You simply provide the theme name, and let it do all the dirty work.
 */


/**
 * Create a new E_Preview objet.
 * @evas - the evas we want to use.
 */
Evas_Object * e_preview_new(Evas *evas);

/**
 * Set the theme file to use.
 * @object - the E_Preview objet
 * @theme - the name of the theme file, not the full path. i.e: default.edj
 */
void e_preview_theme_set(Evas_Object *object, const char * theme);


#endif

