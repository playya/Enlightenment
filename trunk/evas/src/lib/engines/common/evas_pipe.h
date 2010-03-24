#ifndef _EVAS_PIPE_H
#define _EVAS_PIPE_H


/* image rendering pipelines... new optional system - non-immediate and
 * threadable
 */

EAPI void evas_common_pipe_free(RGBA_Image *im);
EAPI void evas_common_pipe_rectangle_draw(RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h);
EAPI void evas_common_pipe_line_draw(RGBA_Image *dst, RGBA_Draw_Context *dc, int x0, int y0, int x1, int y1);
EAPI void evas_common_pipe_poly_draw(RGBA_Image *dst, RGBA_Draw_Context *dc, RGBA_Polygon_Point *points, int x, int y);
EAPI void evas_common_pipe_grad_draw(RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h, RGBA_Gradient *gr);
EAPI void evas_common_pipe_grad2_draw(RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h, RGBA_Gradient2 *gr);
EAPI void evas_common_pipe_text_draw(RGBA_Image *dst, RGBA_Draw_Context *dc, RGBA_Font *fn, int x, int y, const char *text);
EAPI void evas_common_pipe_image_load(RGBA_Image *im);
EAPI void evas_common_pipe_image_draw(RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, int smooth, int src_region_x, int src_region_y, int src_region_w, int src_region_h, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h);
EAPI void evas_common_pipe_map4_draw(RGBA_Image *src, RGBA_Image *dst,
				     RGBA_Draw_Context *dc, RGBA_Map_Point *p,
				     int smooth, int level);

#endif /* _EVAS_PIPE_H */
