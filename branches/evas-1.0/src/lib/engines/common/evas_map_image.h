#ifndef _EVAS_MAP_H
#define _EVAS_MAP_H

EAPI void
evas_common_map4_rgba(RGBA_Image *src, RGBA_Image *dst,
                      RGBA_Draw_Context *dc,
                      RGBA_Map_Point *points,
                      int smooth, int level);

#endif /* _EVAS_MAP_H */
