#include "enesim_bench_common.h"

void perlin_bench(void)
{
	Enesim_Renderer *r;
	Enesim_Surface *dst = NULL;

	printf("**************\n");
	printf("*   Perlin   *\n");
	printf("**************\n");

	surfaces_create(NULL, 0, NULL, 0, &dst, opt_fmt);

	/* 4, 0.1, 0.8 */
	/* 6 0.01 1.2 triggers the error */
	r = enesim_renderer_perlin_new();
	enesim_renderer_perlin_octaves_set(r, 6);
	enesim_renderer_perlin_frequency_set(r, 0.01);
	enesim_renderer_perlin_persistence_set(r, 1.3);
	renderer_run(r, dst, "Perlin", "perlin");
}
