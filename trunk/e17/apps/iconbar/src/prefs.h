#ifndef ICONBAR_PREFS_H_
#define ICONBAR_PREFS_H_

#include<Evas.h>

/* start/stop */
void iconbar_config_init(void);
void iconbar_config_free(void);

/* modify */
void iconbar_config_home_set(char *home);
void iconbar_config_time_format_set(char *str);
void iconbar_config_font_path_append(char *str);
void iconbar_config_geometry_set(int x, int y, int w, int h);
void iconbar_config_icons_set(Evas_List *list);
void iconbar_config_theme_set(const char *theme);

/* query */
const char *iconbar_config_theme_get(void);
const char *iconbar_config_home_get(void);
const char *iconbar_config_time_format_get(void);
Evas_List *iconbar_config_font_path_get(void);
Evas_List *iconbar_config_icons_get(void);
void iconbar_config_geometry_get(int *x, int *y, int *w, int *h);
#endif
