#ifndef _ENNA_IMAGE_H
#define _ENNA_IMAGE_H

#include "enna.h"

Evas_Object *enna_image_add(Evas * evas);
void enna_image_file_set(Evas_Object * obj, const char *file);
const char *enna_image_file_get(Evas_Object * obj);
void enna_image_smooth_scale_set(Evas_Object * obj, int smooth);
int enna_image_smooth_scale_get(Evas_Object * obj);
void enna_image_alpha_set(Evas_Object * obj, int smooth);
int enna_image_alpha_get(Evas_Object * obj);
void enna_image_load_size_set(Evas_Object * obj, int w, int h);
void enna_image_size_get(Evas_Object * obj, int *w, int *h);
int enna_image_fill_inside_get(Evas_Object * obj);
void enna_image_fill_inside_set(Evas_Object * obj, int fill_inside);
void enna_image_data_set(Evas_Object * obj, void *data, int w, int h);
void *enna_image_data_get(Evas_Object * obj, int *w, int *h);
void enna_image_preload(Evas_Object *obj, Eina_Bool cancel);
#endif
