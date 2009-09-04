#include "enesim_bench_common.h"

void test_gradient1(Enesim_Surface *s)
{
	uint32_t *sdata;
	Enesim_Format sfmt;
	uint32_t color;
	Enesim_Compositor_Span span;
	int i;
	int skip = 0;

	sdata = enesim_surface_data_get(s);
	sfmt = enesim_surface_format_get(s);
	/* create a simple gradient */
	enesim_surface_pixel_components_from(&color, sfmt, 0xff, 0xff, 0xff, 0xff);
	if (!(span = enesim_compositor_span_get(ENESIM_FILL, &sfmt, ENESIM_FORMAT_NONE, color, ENESIM_FORMAT_NONE)))
		return;
	for (i = 0; i < opt_height; i++)
	{
		int j;
		uint32_t *sdata_tmp;
		uint8_t col = ((opt_height - 1) - i) * 255 / opt_height;

		sdata_tmp = sdata;
		//enesim_surface_pixel_components_from(&color, sfmt, col, col/2, 0, col);
		enesim_surface_pixel_components_from(&color, sfmt, 0xff, col/2, 0, col);

		if (!((i + 1) % 16))
			skip = (skip + 1) % 2;
		if (skip)
			sdata_tmp += 16;
		for (j = 0; j < opt_width; j += 32)
		{
			span(sdata_tmp, 16, NULL, color, NULL);
			sdata_tmp += 32;
		}
		sdata += opt_width;
	}
}

void test_gradient2(Enesim_Surface *s)
{
	uint32_t *sdata;
	Enesim_Format sfmt;
	uint32_t color;
	Enesim_Compositor_Span span;
	int i;

	sdata = enesim_surface_data_get(s);
	sfmt = enesim_surface_format_get(s);
	/* create a simple gradient */
	enesim_surface_pixel_components_from(&color, sfmt, 0xaa, 0xff, 0xff, 0xff);
	if (!(span = enesim_compositor_span_get(ENESIM_FILL, &sfmt, ENESIM_FORMAT_NONE, color, ENESIM_FORMAT_NONE)))
		return;
	for (i = 0; i < opt_height; i++)
	{
		uint8_t col = (i * 255)/opt_height;

		enesim_surface_pixel_components_from(&color, sfmt, col, col, col, 0);
		span(sdata, opt_width, NULL, color, NULL);
		sdata += opt_width;
	}
}

void test_gradient3(Enesim_Surface *s)
{
	uint32_t *sdata;
	Enesim_Format sfmt;
	uint32_t color;
	Enesim_Compositor_Span span;
	int i;

	sdata = enesim_surface_data_get(s);
	sfmt = enesim_surface_format_get(s);
	/* create a simple gradient */
	enesim_surface_pixel_components_from(&color, sfmt, 0xaa, 0xff, 0xff, 0xff);
	if (!(span = enesim_compositor_span_get(ENESIM_FILL, &sfmt, ENESIM_FORMAT_NONE, color, ENESIM_FORMAT_NONE)))
		return;
	for (i = 0; i < opt_height/2; i++)
	{
		uint8_t col = (i * 255)/(opt_height/2);

		enesim_surface_pixel_components_from(&color, sfmt, col, 0, col / 2, col);
		span(sdata, opt_width, NULL, color, NULL);
		sdata += opt_width;
	}
	for (i = 0; i < opt_height/2; i++)
	{
		uint8_t col = 255 - (i * 255)/(opt_height/2);

		enesim_surface_pixel_components_from(&color, sfmt, col, 0, col / 2, col);
		span(sdata, opt_width, NULL, color, NULL);
		sdata += opt_width;
	}
}

Enesim_Surface * test_pattern1(int w)
{
	Enesim_Surface *s;
	uint32_t color;
	uint32_t *sdata;
	Enesim_Compositor_Span span;
	Enesim_Format sfmt = opt_fmt;

	int i;
	int spaces = w / 2;

	enesim_surface_pixel_components_from(&color, opt_fmt, 0xaa, 0xaa, 0xff, 0);
	if (!(span = enesim_compositor_span_get(ENESIM_FILL, &sfmt, ENESIM_FORMAT_NONE, color, ENESIM_FORMAT_NONE)))
		return NULL;
	s = enesim_surface_new(opt_fmt, w, w);

	/* draw the pattern */
	sdata = enesim_surface_data_get(s);
	sdata += spaces;
	for (i = 0; i < w / 2; i++)
	{
		int len = i * 2 +1;
		int nspaces = spaces - 1;

		span(sdata, len, NULL, color, NULL);
		sdata += len + spaces + nspaces;
		spaces--;
	}
	span(sdata, w, NULL, color, NULL);
	sdata += w + 1;
	spaces = 1;
	for (i = 0; i < w / 2; i++)
	{
		int len = (w - 1) - (i * 2 +1);
		int nspaces = spaces + 1;

		span(sdata, len, NULL, color, NULL);
		sdata += len + spaces + nspaces;
		spaces++;
	}
	surface_save(s, "pattern.png");
	return s;
}

Enesim_Surface * test_pattern2(int w)
{
	Enesim_Surface *s;
	uint32_t color;
	uint32_t *sdata;
	Enesim_Compositor_Span span;
	Enesim_Format sfmt = opt_fmt;

	int i;
	int spaces = w / 2;

	enesim_surface_pixel_components_from(&color, opt_fmt, 0xaa, 0xaa, 0xff, 0);
	if (!(span = enesim_compositor_span_get(ENESIM_FILL, &sfmt, ENESIM_FORMAT_NONE, color, ENESIM_FORMAT_NONE)))
		return NULL;
	s = enesim_surface_new(opt_fmt, w, w);

	/* draw the pattern */
	sdata = enesim_surface_data_get(s);
	sdata += spaces;
	for (i = 0; i < w / 2; i++)
	{
		int len = i * 2 +1;
		int nspaces = spaces - 1;

		enesim_surface_pixel_components_from(&color, opt_fmt, i * (255 / w), 0xff, 0, 0xff);
		span(sdata, len, NULL, color, NULL);
		sdata += len + spaces + nspaces;
		spaces--;
	}
	span(sdata, w, NULL, color, NULL);
	sdata += w + 1;
	spaces = 1;
	for (i = 0; i < w / 2; i++)
	{
		int len = (w - 1) - (i * 2 +1);
		int nspaces = spaces + 1;

		enesim_surface_pixel_components_from(&color, opt_fmt, (w / 2 - i) * (255 / w), 0xff, 0, 0xff);
		span(sdata, len, NULL, color, NULL);
		sdata += len + spaces + nspaces;
		spaces++;
	}
	surface_save(s, "pattern2.png");
	return s;
}
