#ifndef ELICIT_UTIL_H
#define ELICIT_UTIL_H

void elicit_util_colors_set_from_hsv(Elicit *el);
void elicit_util_colors_set_from_rgb(Elicit *el);

int elicit_glob_match(const char *str, const char *glob);

void elicit_util_color_at_pointer_get(int *r, int *g, int *b);
void elicit_util_shoot(Evas_Object *shot, int w, int h);

char *elicit_theme_find(const char *name);

#endif
