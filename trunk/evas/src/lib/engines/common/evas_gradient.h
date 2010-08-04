#ifndef _EVAS_GRADIENT_H
#define _EVAS_GRADIENT_H


EAPI void           evas_common_gradient_init              (void);

EAPI RGBA_Gradient *evas_common_gradient_new               (void);
EAPI void           evas_common_gradient_free              (RGBA_Gradient *gr);
EAPI void           evas_common_gradient_clear             (RGBA_Gradient *gr);
EAPI void           evas_common_gradient_color_stop_add    (RGBA_Gradient *gr, int r, int g, int b, int a, int dist);
EAPI void           evas_common_gradient_alpha_stop_add    (RGBA_Gradient *gr, int a, int dist);
EAPI void           evas_common_gradient_color_data_set    (RGBA_Gradient *gr, DATA32 *data, int len, int alpha_flags);
EAPI void           evas_common_gradient_alpha_data_set    (RGBA_Gradient *gr, DATA8 *adata, int len);
EAPI void           evas_common_gradient_type_set          (RGBA_Gradient *gr, const char *name, char *params);
EAPI void           evas_common_gradient_fill_set          (RGBA_Gradient *gr, int x, int y, int w, int h);
EAPI void           evas_common_gradient_fill_angle_set    (RGBA_Gradient *gr, float angle);
EAPI void           evas_common_gradient_fill_spread_set   (RGBA_Gradient *gr, int spread);
EAPI void           evas_common_gradient_map_angle_set     (RGBA_Gradient *gr, float angle);
EAPI void           evas_common_gradient_map_offset_set    (RGBA_Gradient *gr, float offset);
EAPI void           evas_common_gradient_map_direction_set (RGBA_Gradient *gr, int direction);
EAPI void           evas_common_gradient_map               (RGBA_Draw_Context *dc, RGBA_Gradient *gr, int len);
EAPI void           evas_common_gradient_draw              (RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h, RGBA_Gradient *gr);

EAPI RGBA_Gradient_Type *evas_common_gradient_geometer_get (const char *name);



EAPI void           evas_common_gradient2_free             (RGBA_Gradient2 *gr);
EAPI RGBA_Gradient2 *evas_common_gradient2_linear_new              (void);
EAPI void           evas_common_gradient2_linear_fill_set (RGBA_Gradient2 *gr, float x0, float y0, float x1, float y1);
EAPI RGBA_Gradient2 *evas_common_gradient2_radial_new              (void);
EAPI void           evas_common_gradient2_radial_fill_set (RGBA_Gradient2 *gr, float cx, float cy, float rx, float ry);
EAPI void           evas_common_gradient2_clear            (RGBA_Gradient2 *gr);
EAPI void           evas_common_gradient2_color_np_stop_insert   (RGBA_Gradient2 *gr, int r, int g, int b, int a, float pos);
EAPI void           evas_common_gradient2_fill_spread_set  (RGBA_Gradient2 *gr, int spread);
EAPI void           evas_common_gradient2_fill_transform_set (RGBA_Gradient2 *gr, Evas_Common_Transform *t);
EAPI void           evas_common_gradient2_map              (RGBA_Draw_Context *dc, RGBA_Gradient2 *gr, int len);
EAPI void           evas_common_gradient2_draw             (RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h, RGBA_Gradient2 *gr);

EAPI RGBA_Gradient2_Type *evas_common_gradient2_type_linear_get      (void);
EAPI RGBA_Gradient2_Type *evas_common_gradient2_type_radial_get      (void);
//EAPI RGBA_Gradient2_Type *evas_common_gradient2_type_angular_get     (void);
//EAPI RGBA_Gradient2_Type *evas_common_gradient2_type_rectangular_get (void);
//EAPI RGBA_Gradient2_Type *evas_common_gradient2_type_sinusoidal_get  (void);

#endif /* _EVAS_GRADIENT_H */
