#include "Enesim.h"
#include "enesim_private.h"
#if 0
static void argb8888_unpre_pt_color_blend(Enesim_Surface_Data *d, Enesim_Surface_Pixel *s, Enesim_Surface_Pixel *color, Enesim_Surface_Data *m)
{
	uint32_t data0;
	argb8888_unpre_from_argb(color, &data0);
	argb8888_unpre_blend(d->argb8888_unpre.plane0, data0);
}
static void argb8888_unpre_pt_pixel_blend_argb8888(Enesim_Surface_Data *d, Enesim_Surface_Pixel *s, Enesim_Surface_Pixel *color, Enesim_Surface_Data *m)
{
	uint32_t data0;
	unsigned int argb;
	argb8888_to_argb(&argb, *(s->argb8888.plane0));
	argb8888_unpre_from_argb(argb, &data0);
	argb8888_unpre_blend(d->argb8888_unpre.plane0, data0);
}
static void argb8888_unpre_pt_pixel_blend_argb8888_unpre(Enesim_Surface_Data *d, Enesim_Surface_Data *s, Enesim_Surface_Pixel *color, Enesim_Surface_Data *m)
{
	uint32_t data0;
	unsigned int argb;
	argb8888_unpre_to_argb(&argb, *(s->argb8888_unpre.plane0));
	argb8888_unpre_from_argb(argb, &data0);
	argb8888_unpre_blend(d->argb8888_unpre.plane0, data0);
}
static void argb8888_unpre_pt_pixel_blend_rgb565_b1a3(Enesim_Surface_Data *d, Enesim_Surface_Data *s, Enesim_Surface_Pixel *color, Enesim_Surface_Data *m)
{
	uint32_t data0;
	unsigned int argb;
	rgb565_b1a3_to_argb(&argb, *(s->rgb565_b1a3.plane0), *(s->rgb565_b1a3.plane1), s->rgb565_b1a3.pixel_plane1);
	argb8888_unpre_from_argb(argb, &data0);
	argb8888_unpre_blend(d->argb8888_unpre.plane0, data0);
}
static void argb8888_unpre_sp_color_blend(Enesim_Surface_Data *d, unsigned int len, Enesim_Surface_Data *s, Enesim_Surface_Pixel *color, Enesim_Surface_Data *m)
{
	Enesim_Surface_Data dtmp, end;

	argb8888_unpre_data_copy(d, &dtmp);
	argb8888_unpre_data_offset(d, &end, len);
	while (dtmp.argb8888_unpre.plane0 < end.argb8888_unpre.plane0)
	{
		argb8888_unpre_pt_color_blend(&dtmp, NULL, color, NULL);
		argb8888_unpre_data_increment(&dtmp, 1);
	}
}
static void argb8888_unpre_sp_pixel_blend_argb8888(Enesim_Surface_Data *d, unsigned int len, Enesim_Surface_Data *s, Enesim_Surface_Pixel *color, Enesim_Surface_Data *m)
{
	Enesim_Surface_Data stmp, dtmp, end;

	argb8888_unpre_data_copy(d, &dtmp);
	argb8888_data_copy(s, &stmp);
	argb8888_unpre_data_offset(d, &end, len);
	while (dtmp.argb8888_unpre.plane0 < end.argb8888_unpre.plane0)
	{
		argb8888_unpre_pt_pixel_blend_argb8888(&dtmp, &stmp, 0, NULL);
		argb8888_data_increment(&stmp, 1);
		argb8888_unpre_data_increment(&dtmp, 1);
	}
}
static void argb8888_unpre_sp_pixel_blend_argb8888_unpre(Enesim_Surface_Data *d, unsigned int len, Enesim_Surface_Data *s, Enesim_Surface_Pixel *color, Enesim_Surface_Data *m)
{
	Enesim_Surface_Data stmp, dtmp, end;

	argb8888_unpre_data_copy(d, &dtmp);
	argb8888_unpre_data_copy(s, &stmp);
	argb8888_unpre_data_offset(d, &end, len);
	while (dtmp.argb8888_unpre.plane0 < end.argb8888_unpre.plane0)
	{
		argb8888_unpre_pt_pixel_blend_argb8888_unpre(&dtmp, &stmp, 0, NULL);
		argb8888_unpre_data_increment(&stmp, 1);
		argb8888_unpre_data_increment(&dtmp, 1);
	}
}
static void argb8888_unpre_sp_pixel_blend_rgb565_b1a3(Enesim_Surface_Data *d, unsigned int len, Enesim_Surface_Data *s, Enesim_Surface_Pixel *color, Enesim_Surface_Data *m)
{
	Enesim_Surface_Data stmp, dtmp, end;

	argb8888_unpre_data_copy(d, &dtmp);
	rgb565_b1a3_data_copy(s, &stmp);
	argb8888_unpre_data_offset(d, &end, len);
	while (dtmp.argb8888_unpre.plane0 < end.argb8888_unpre.plane0)
	{
		argb8888_unpre_pt_pixel_blend_rgb565_b1a3(&dtmp, &stmp, 0, NULL);
		rgb565_b1a3_data_increment(&stmp, 1);
		argb8888_unpre_data_increment(&dtmp, 1);
	}
}
static void argb8888_unpre_pt_color_fill(Enesim_Surface_Data *d, Enesim_Surface_Data *s, Enesim_Surface_Pixel *color, Enesim_Surface_Data *m)
{
	uint32_t data0;
	argb8888_unpre_from_argb(color, &data0);
	argb8888_unpre_fill(d->argb8888_unpre.plane0, data0);
}
static void argb8888_unpre_pt_pixel_fill_argb8888(Enesim_Surface_Data *d, Enesim_Surface_Data *s, Enesim_Surface_Pixel *color, Enesim_Surface_Data *m)
{
	uint32_t data0;
	unsigned int argb;
	argb8888_to_argb(&argb, *(s->argb8888.plane0));
	argb8888_unpre_from_argb(argb, &data0);
	argb8888_unpre_fill(d->argb8888_unpre.plane0, data0);
}
static void argb8888_unpre_pt_pixel_fill_argb8888_unpre(Enesim_Surface_Data *d, Enesim_Surface_Data *s, Enesim_Surface_Pixel *color, Enesim_Surface_Data *m)
{
	uint32_t data0;
	unsigned int argb;
	argb8888_unpre_to_argb(&argb, *(s->argb8888_unpre.plane0));
	argb8888_unpre_from_argb(argb, &data0);
	argb8888_unpre_fill(d->argb8888_unpre.plane0, data0);
}
static void argb8888_unpre_pt_pixel_fill_rgb565_b1a3(Enesim_Surface_Data *d, Enesim_Surface_Data *s, Enesim_Surface_Pixel *color, Enesim_Surface_Data *m)
{
	uint32_t data0;
	unsigned int argb;
	rgb565_b1a3_to_argb(&argb, *(s->rgb565_b1a3.plane0), *(s->rgb565_b1a3.plane1), s->rgb565_b1a3.pixel_plane1);
	argb8888_unpre_from_argb(argb, &data0);
	argb8888_unpre_fill(d->argb8888_unpre.plane0, data0);
}
static void argb8888_unpre_sp_color_fill(Enesim_Surface_Data *d, unsigned int len, Enesim_Surface_Data *s, Enesim_Surface_Pixel *color, Enesim_Surface_Data *m)
{
	Enesim_Surface_Data dtmp, end;

	argb8888_unpre_data_copy(d, &dtmp);
	argb8888_unpre_data_offset(d, &end, len);
	while (dtmp.argb8888_unpre.plane0 < end.argb8888_unpre.plane0)
	{
		argb8888_unpre_pt_color_fill(&dtmp, NULL, color, NULL);
		argb8888_unpre_data_increment(&dtmp, 1);
	}
}
static void argb8888_unpre_sp_pixel_fill_argb8888(Enesim_Surface_Data *d, unsigned int len, Enesim_Surface_Data *s, Enesim_Surface_Pixel *color, Enesim_Surface_Data *m)
{
	Enesim_Surface_Data stmp, dtmp, end;

	argb8888_unpre_data_copy(d, &dtmp);
	argb8888_data_copy(s, &stmp);
	argb8888_unpre_data_offset(d, &end, len);
	while (dtmp.argb8888_unpre.plane0 < end.argb8888_unpre.plane0)
	{
		argb8888_unpre_pt_pixel_fill_argb8888(&dtmp, &stmp, 0, NULL);
		argb8888_data_increment(&stmp, 1);
		argb8888_unpre_data_increment(&dtmp, 1);
	}
}
static void argb8888_unpre_sp_pixel_fill_argb8888_unpre(Enesim_Surface_Data *d, unsigned int len, Enesim_Surface_Data *s, Enesim_Surface_Pixel *color, Enesim_Surface_Data *m)
{
	Enesim_Surface_Data stmp, dtmp, end;

	argb8888_unpre_data_copy(d, &dtmp);
	argb8888_unpre_data_copy(s, &stmp);
	argb8888_unpre_data_offset(d, &end, len);
	while (dtmp.argb8888_unpre.plane0 < end.argb8888_unpre.plane0)
	{
		argb8888_unpre_pt_pixel_fill_argb8888_unpre(&dtmp, &stmp, 0, NULL);
		argb8888_unpre_data_increment(&stmp, 1);
		argb8888_unpre_data_increment(&dtmp, 1);
	}
}
static void argb8888_unpre_sp_pixel_fill_rgb565_b1a3(Enesim_Surface_Data *d, unsigned int len, Enesim_Surface_Data *s, Enesim_Surface_Pixel *color, Enesim_Surface_Data *m)
{
	Enesim_Surface_Data stmp, dtmp, end;

	argb8888_unpre_data_copy(d, &dtmp);
	rgb565_b1a3_data_copy(s, &stmp);
	argb8888_unpre_data_offset(d, &end, len);
	while (dtmp.argb8888_unpre.plane0 < end.argb8888_unpre.plane0)
	{
		argb8888_unpre_pt_pixel_fill_rgb565_b1a3(&dtmp, &stmp, 0, NULL);
		rgb565_b1a3_data_increment(&stmp, 1);
		argb8888_unpre_data_increment(&dtmp, 1);
	}
}
#endif
#ifdef BUILD_SURFACE_ARGB8888_UNPRE

#define argb8888_unpre_sp_color_blend enesim_drawer_sp_color_blend
#define argb8888_unpre_pt_color_blend enesim_drawer_pt_color_blend
#define argb8888_unpre_sp_pixel_blend_argb8888 enesim_drawer_sp_pixel_blend
#define argb8888_unpre_pt_pixel_blend_argb8888 enesim_drawer_pt_pixel_blend
#define argb8888_unpre_sp_pixel_blend_argb8888_unpre enesim_drawer_sp_pixel_blend
#define argb8888_unpre_pt_pixel_blend_argb8888_unpre enesim_drawer_pt_pixel_blend
#define argb8888_unpre_sp_pixel_blend_rgb565_b1a3 enesim_drawer_sp_pixel_blend
#define argb8888_unpre_pt_pixel_blend_rgb565_b1a3 enesim_drawer_pt_pixel_blend

#define argb8888_unpre_sp_color_fill enesim_drawer_sp_color_fill
#define argb8888_unpre_pt_color_fill enesim_drawer_pt_color_fill
#define argb8888_unpre_sp_pixel_fill_argb8888 enesim_drawer_sp_pixel_fill
#define argb8888_unpre_pt_pixel_fill_argb8888 enesim_drawer_pt_pixel_fill
#define argb8888_unpre_sp_pixel_fill_argb8888_unpre enesim_drawer_sp_pixel_fill
#define argb8888_unpre_pt_pixel_fill_argb8888_unpre enesim_drawer_pt_pixel_fill
#define argb8888_unpre_sp_pixel_fill_rgb565_b1a3 enesim_drawer_sp_pixel_fill
#define argb8888_unpre_pt_pixel_fill_rgb565_b1a3 enesim_drawer_pt_pixel_fill

Enesim_Drawer argb8888_unpre_drawer = {
	.sp_color[ENESIM_BLEND] = argb8888_unpre_sp_color_blend,
	.pt_color[ENESIM_BLEND] = argb8888_unpre_pt_color_blend,
	.sp_pixel[ENESIM_BLEND][ENESIM_SURFACE_ARGB8888] = argb8888_unpre_sp_pixel_blend_argb8888,
	.pt_pixel[ENESIM_BLEND][ENESIM_SURFACE_ARGB8888] = argb8888_unpre_pt_pixel_blend_argb8888,
	.sp_pixel[ENESIM_BLEND][ENESIM_SURFACE_ARGB8888_UNPRE] = argb8888_unpre_sp_pixel_blend_argb8888_unpre,
	.pt_pixel[ENESIM_BLEND][ENESIM_SURFACE_ARGB8888_UNPRE] = argb8888_unpre_pt_pixel_blend_argb8888_unpre,
	.sp_pixel[ENESIM_BLEND][ENESIM_SURFACE_RGB565_XA5] = NULL,
	.pt_pixel[ENESIM_BLEND][ENESIM_SURFACE_RGB565_XA5] = NULL,
	.sp_pixel[ENESIM_BLEND][ENESIM_SURFACE_RGB565_B1A3] = argb8888_unpre_sp_pixel_blend_rgb565_b1a3,
	.pt_pixel[ENESIM_BLEND][ENESIM_SURFACE_RGB565_B1A3] = argb8888_unpre_pt_pixel_blend_rgb565_b1a3,
	.sp_color[ENESIM_FILL] = argb8888_unpre_sp_color_fill,
	.pt_color[ENESIM_FILL] = argb8888_unpre_pt_color_fill,
	.sp_pixel[ENESIM_FILL][ENESIM_SURFACE_ARGB8888] = argb8888_unpre_sp_pixel_fill_argb8888,
	.pt_pixel[ENESIM_FILL][ENESIM_SURFACE_ARGB8888] = argb8888_unpre_pt_pixel_fill_argb8888,
	.sp_pixel[ENESIM_FILL][ENESIM_SURFACE_ARGB8888_UNPRE] = argb8888_unpre_sp_pixel_fill_argb8888_unpre,
	.pt_pixel[ENESIM_FILL][ENESIM_SURFACE_ARGB8888_UNPRE] = argb8888_unpre_pt_pixel_fill_argb8888_unpre,
	.sp_pixel[ENESIM_FILL][ENESIM_SURFACE_RGB565_XA5] = NULL,
	.pt_pixel[ENESIM_FILL][ENESIM_SURFACE_RGB565_XA5] = NULL,
	.sp_pixel[ENESIM_FILL][ENESIM_SURFACE_RGB565_B1A3] = argb8888_unpre_sp_pixel_fill_rgb565_b1a3,
	.pt_pixel[ENESIM_FILL][ENESIM_SURFACE_RGB565_B1A3] = argb8888_unpre_pt_pixel_fill_rgb565_b1a3,
};

#endif
