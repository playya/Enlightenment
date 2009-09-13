/* ENESIM - Direct Rendering Library
 * Copyright (C) 2007-2008 Jorge Luis Zapata
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef ENESIM_RENDERER_H_
#define ENESIM_RENDERER_H_

/**
 * @defgroup Enesim_Renderer_Group Renderer
 *
 * @{
 */
typedef struct _Enesim_Renderer Enesim_Renderer; /**< Renderer Handler */
EAPI void enesim_renderer_transform_set(Enesim_Renderer *r, Enesim_Matrix *m);
EAPI void enesim_renderer_delete(Enesim_Renderer *r);
EAPI Eina_Bool enesim_renderer_state_setup(Enesim_Renderer *r);
EAPI void enesim_renderer_state_cleanup(Enesim_Renderer *r);
EAPI void enesim_renderer_span_fill(Enesim_Renderer *r, int x, int y,
	unsigned int len, uint32_t *dst);
EAPI void enesim_renderer_origin_set(Enesim_Renderer *r, int x, int y);

typedef enum _Enesim_Shape_Draw_Mode
{
	ENESIM_SHAPE_DRAW_MODE_FILL = 0,
	ENESIM_SHAPE_DRAW_MODE_STROKE = 1,
	ENESIM_SHAPE_DRAW_MODE_STROKE_FILL = 2,
} Enesim_Shape_Draw_Mode;

EAPI Enesim_Renderer * enesim_renderer_hswitch_new(void);
EAPI void enesim_renderer_hswitch_w_set(Enesim_Renderer *r, int w);
EAPI void enesim_renderer_hswitch_h_set(Enesim_Renderer *r, int h);
EAPI void enesim_renderer_hswitch_left_set(Enesim_Renderer *r,
		Enesim_Renderer *left);
EAPI void enesim_renderer_hswitch_right_set(Enesim_Renderer *r,
		Enesim_Renderer *right);
EAPI void enesim_renderer_hswitch_step_set(Enesim_Renderer *r, float step);

EAPI Enesim_Renderer * enesim_renderer_surface_new(void);
EAPI void enesim_renderer_surface_x_set(Enesim_Renderer *r, int x);
EAPI void enesim_renderer_surface_y_set(Enesim_Renderer *r, int y);
EAPI void enesim_renderer_surface_w_set(Enesim_Renderer *r, int w);
EAPI void enesim_renderer_surface_h_set(Enesim_Renderer *r, int h);
EAPI void enesim_renderer_surface_src_set(Enesim_Renderer *r, Enesim_Surface *src);


EAPI Enesim_Renderer * enesim_renderer_rectangle_new(void);
EAPI void enesim_renderer_rectangle_size_set(Enesim_Renderer *p, int w, int h);
EAPI void enesim_renderer_rectangle_corner_radius_set(Enesim_Renderer *p, float radius);
EAPI void enesim_renderer_rectangle_corners_set(Enesim_Renderer *p, int tl, int tr, int bl, int br);

EAPI Enesim_Renderer * enesim_renderer_circle_new(void);
EAPI void enesim_renderer_circle_center_set(Enesim_Renderer *r, float x, float y);
EAPI void enesim_renderer_circle_radius_set(Enesim_Renderer *r, float radius);

EAPI Enesim_Renderer * enesim_renderer_ellipse_new(void);
EAPI void enesim_renderer_ellipse_center_set(Enesim_Renderer *p, float x, float y);
EAPI void enesim_renderer_ellipse_radii_set(Enesim_Renderer *p, float radius_x, float radius_y);

EAPI void enesim_renderer_shape_outline_weight_set(Enesim_Renderer *r, float weight);
EAPI void enesim_renderer_shape_outline_color_set(Enesim_Renderer *r, Enesim_Color stroke_color);
EAPI void enesim_renderer_shape_outline_renderer_set(Enesim_Renderer *r, Enesim_Renderer *o);
EAPI void enesim_renderer_shape_fill_color_set(Enesim_Renderer *r, Enesim_Color fill_color);
EAPI void enesim_renderer_shape_fill_renderer_set(Enesim_Renderer *r, Enesim_Renderer *f);
EAPI void enesim_renderer_shape_draw_mode_set(Enesim_Renderer *r, Enesim_Shape_Draw_Mode draw_mode);

EAPI Enesim_Renderer * enesim_renderer_checker_new(void);
EAPI void enesim_renderer_checker_color1_set(Enesim_Renderer *r, uint32_t color);
EAPI void enesim_renderer_checker_color2_set(Enesim_Renderer *r, uint32_t color);
EAPI void enesim_renderer_checker_size_set(Enesim_Renderer *r, int w, int h);

EAPI Enesim_Renderer * enesim_renderer_transition_new(void);
EAPI void enesim_renderer_transition_value_set(Enesim_Renderer *r, float interp_value);
EAPI void enesim_renderer_transition_source_set(Enesim_Renderer *r, Enesim_Renderer *r0);
EAPI void enesim_renderer_transition_target_set(Enesim_Renderer *r, Enesim_Renderer *r1);
EAPI void enesim_renderer_transition_offset_set(Enesim_Renderer *r, int x, int y);

EAPI Enesim_Renderer * enesim_renderer_stripes_new(void);
EAPI void enesim_renderer_stripes_color_set(Enesim_Renderer *p,
		Enesim_Color c0, Enesim_Color c1);
EAPI void enesim_renderer_stripes_thickness_set(Enesim_Renderer *p,
		float thickness_0, float thickness_1);

EAPI Enesim_Renderer * enesim_renderer_dispmap_new(void);
EAPI void enesim_renderer_dispmap_map_set(Enesim_Renderer *r, Enesim_Surface *map);
EAPI void enesim_renderer_dispmap_src_set(Enesim_Renderer *r, Enesim_Surface *src);
EAPI void enesim_renderer_dispmap_scale_set(Enesim_Renderer *r, float scale);

EAPI Enesim_Renderer * enesim_renderer_raddist_new(void);
EAPI void enesim_renderer_raddist_radius_set(Enesim_Renderer *r, float radius);
EAPI void enesim_renderer_raddist_scale_set(Enesim_Renderer *r, float scale);
EAPI void enesim_renderer_raddist_src_set(Enesim_Renderer *r, Enesim_Surface *src);
EAPI void enesim_renderer_raddist_center_set(Enesim_Renderer *r, int ox, int oy);

EAPI Enesim_Renderer * enesim_renderer_importer_new(void);
EAPI void enesim_renderer_importer_angle_set(Enesim_Renderer *r, Enesim_Angle angle);
EAPI void enesim_renderer_importer_data_set(Enesim_Renderer *r, Enesim_Converter_Data *cdata);
EAPI void enesim_renderer_importer_format_set(Enesim_Renderer *r, Enesim_Converter_Format fmt);

EAPI Enesim_Renderer * enesim_renderer_perlin_new(void);
EAPI void enesim_renderer_perlin_octaves_set(Enesim_Renderer *r, unsigned int octaves);
EAPI void enesim_renderer_perlin_persistence_set(Enesim_Renderer *r, float persistence);
EAPI void enesim_renderer_perlin_xfrequency_set(Enesim_Renderer *r, float freq);
EAPI void enesim_renderer_perlin_yfrequency_set(Enesim_Renderer *r, float freq);

EAPI Enesim_Renderer * enesim_renderer_gradient_linear_new(void);
EAPI void enesim_renderer_gradient_linear_pos_set(Enesim_Renderer *r, float x0,
		float y0, float x1, float y1);


EAPI void enesim_renderer_gradient_stop_add(Enesim_Renderer *r, Enesim_Color c,
		float pos);
EAPI void enesim_renderer_gradient_clear(Enesim_Renderer *r);


EAPI Enesim_Renderer * enesim_renderer_compound_new(void);
EAPI void enesim_renderer_compound_layer_add(Enesim_Renderer *r,
		Enesim_Renderer *rend, Enesim_Rop rop);
EAPI void enesim_renderer_compound_clear(Enesim_Renderer *r);

/**
 * @}
 */

#endif /*ENESIM_RENDERER_H_*/
