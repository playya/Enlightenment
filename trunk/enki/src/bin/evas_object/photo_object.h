// vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2

#ifndef  PHOTO_OBJECT_INC
#define  PHOTO_OBJECT_INC

#include <Evas.h>
#include <Edje.h>
#include <Ecore_Evas.h>
#include <stdio.h>
#include <Elementary.h>

Evas_Object *photo_object_add(Evas_Object *obj);
void photo_object_theme_file_set(Evas_Object *obj, const char *theme, const char* theme_group);
void photo_object_file_set(Evas_Object *obj, const char *image, const char *photo_group);
void photo_object_size_set(Evas_Object *obj, Evas_Coord w, Evas_Coord h);
void photo_object_progressbar_set(Evas_Object *obj, Eina_Bool b);
void photo_object_radio_set(Evas_Object *obj, Eina_Bool b);
void photo_object_camera_set(Evas_Object *obj, Eina_Bool b);
void photo_object_text_set(Evas_Object *obj, const char *s);
void photo_object_done_set(Evas_Object *obj, Eina_Bool b);

Evas_Object *photo_object_flickr_state_set(Evas_Object *obj, const char* state);

#endif   /* ----- #ifndef PHOTO_OBJECT_INC  ----- */

