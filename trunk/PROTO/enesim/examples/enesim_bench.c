#include "enesim_bench_common.h"

/**
 * TODO
 * + Make the gradient functions draw pattern boxes
 * + Add a html output mode, for easy viewing of every format/op/time
 * + Split this into differnet benchs instead of only one?
 * + Add a mask and src formats
 */

int opt_width = 256;
int opt_height = 256;
int opt_times = 1;
FILE *opt_file;
int opt_debug = 0;
int opt_rop = ENESIM_FILL;
Enesim_Format opt_fmt = ENESIM_FORMAT_ARGB8888;
char *opt_bench = "compositor";

double get_time(void)
{
	struct timeval timev;

	gettimeofday(&timev, NULL);
	return (double)timev.tv_sec + (((double)timev.tv_usec) / 1000000);
}

char * opacity_get(uint32_t color, Enesim_Format f)
{
	uint8_t alpha;

	enesim_surface_pixel_components_to(color, f, &alpha, NULL, NULL, NULL);
	if (alpha == 0xff)
		return "opaque";
	else
		return "transparent";
}

void surface_save(Enesim_Surface *s, const char *name)
{
	image_save(s, name, 0);
}

void surfaces_create(Enesim_Surface **src, Enesim_Format sfmt,
		Enesim_Surface **dst, Enesim_Format dfmt,
		Enesim_Surface **msk, Enesim_Format mfmt)
{
	if (src)
	{
		if (*src) enesim_surface_delete(*src);
		*src = enesim_surface_new(sfmt, opt_width, opt_height);
		test_gradient1(*src);
		surface_save(*src, "source.png");
	}
	if (dst)
	{
		if (*dst) enesim_surface_delete(*dst);
		*dst = enesim_surface_new(dfmt, opt_width, opt_height);
		test_gradient2(*dst);
		surface_save(*dst, "destination.png");
	}
	if (msk)
	{
		if (*msk) enesim_surface_delete(*msk);
		*msk = enesim_surface_new(mfmt, opt_width, opt_height);
		test_gradient3(*msk);
		surface_save(*msk, "mask.png");
	}
}

void help(void)
{
	printf("Usage: enesim_bench [OPTION]...\n");
	printf("Runs a benchmark test for enesim operations\n");
	printf("Option can be one of the following:\n");
	printf("-h, --help            Print this screen\n");
	printf("-t, --times           Number of times to run the test [10]\n");
	printf("-d, --debug           Dump temporal images [off]\n");
	printf("-f, --file            File to place the statistics [benchmark.txt]\n");
	printf("-r, --rop             Raster operation to use [fill], use help to get a list\n");
	printf("-c, --cpu             What cpu to use\n");
	printf("-m, --fmt             Surface format to use [argb8888_pre], use help to get a list\n");
	printf("-w, --width           Surface width\n");
	printf("-e, --height          Surface height\n");
	printf("-b, --bench           Benchmark to run, use help to get a list\n");
}

const char * rop_name(Enesim_Rop rop)
{
	switch (rop)
	{
		case ENESIM_BLEND:
			return "blend";
		case ENESIM_FILL:
			return "fill";
		default:
			return NULL;
	}
	return NULL;
}

int rop_get(const char *name, int *rop)
{
	int ret = 1;

	if (!strcmp(name, "blend"))
		*rop = ENESIM_BLEND;
	else if (!strcmp(name, "fill"))
		*rop = ENESIM_FILL;
	else
		ret = 0;
	return ret;
}

int bench_get(const char *name)
{
	if (!strcmp(name, "help"))
		return 0;
	else if (!strcmp(name, "renderer"))
	{
		opt_bench = "renderer";
		return 1;
	}
	else if (!strcmp(name, "rasterizer"))
	{
		opt_bench = "rasterizer";
		return 1;
	}
	else if (!strcmp(name, "compositor"))
	{
		opt_bench = "compositor";
		return 1;
	}
	else if (!strcmp(name, "raddist"))
	{
		opt_bench = "raddist";
		return 1;
	}
	else if (!strcmp(name, "dispmap"))
	{
		opt_bench = "dispmap";
		return 1;
	}
	else if (!strcmp(name, "hswitch"))
	{
		opt_bench = "hswitch";
		return 1;
	}
	else if (!strcmp(name, "checker"))
	{
		opt_bench = "checker";
		return 1;
	}
	else if (!strcmp(name, "stripes"))
	{
		opt_bench = "stripes";
		return 1;
	}
	else if (!strcmp(name, "circle"))
	{
		opt_bench = "circle";
		return 1;
	}
	else if (!strcmp(name, "ellipse"))
	{
		opt_bench = "ellipse";
		return 1;
	}
	else if (!strcmp(name, "rectangle"))
	{
		opt_bench = "rectangle";
		return 1;
	}
	else if (!strcmp(name, "perlin"))
	{
		opt_bench = "perlin";
		return 1;
	}
	else if (!strcmp(name, "gradient_linear"))
	{
		opt_bench = "gradient_linear";
		return 1;
	}
	else if (!strcmp(name, "compound"))
	{
		opt_bench = "compound";
		return 1;
	}
	else if (!strcmp(name, "all"))
	{
		opt_bench = "all";
		return 1;
	}
	return 0;
}

int fmt_get(const char *name, Enesim_Format *fmt)
{
	Enesim_Format f = 0;

	while (f < ENESIM_FORMATS)
	{
		if (!strcmp(name, enesim_format_name_get(f)))
		{
			*fmt = f;
			return 1;
		}
		f++;
	}
	return 0;
}


void rop_help(void)
{
	int rop;

	for (rop = 0; rop < ENESIM_ROPS; rop++)
	{
		printf("%s\n", rop_name(rop));
	}
}

void bench_help(void)
{
	printf("dispmap\n");
	printf("compositor\n");
	printf("raddist\n");
	printf("rasterizer\n");
	printf("renderer\n");
	printf("spanner\n");
	printf("hswitch\n");
	printf("stripes\n");
	printf("checker\n");
	printf("circle\n");
	printf("ellipse\n");
	printf("rectangle\n");
	printf("perlin\n");
	printf("gradient_linear\n");
	printf("compound\n");
	printf("all\n");
}

void fmt_help(void)
{
	Enesim_Format f = 0;

	while (f < ENESIM_FORMATS)
	{
		printf("%s\n", enesim_format_name_get(f));
		f++;
	}
}

/*
 * TODO remove the rop as it is only part of the drawer benchmark
 */
void test_finish(const char *name, Enesim_Rop rop, Enesim_Surface *dst,
		Enesim_Surface *src, uint32_t *color, Enesim_Surface *mask)
{
	Enesim_Format sfmt;
	char file[256];
	char tmp[256];

	if (!opt_debug)
		return;

	sfmt = enesim_surface_format_get(dst);
	snprintf(tmp, 256, "%s_%s_%s", name, rop_name(rop), enesim_format_name_get(sfmt));
	if (src)
	{
		char tmp2[256];

		sfmt = enesim_surface_format_get(src);
		snprintf(tmp2, 256, "_%s", enesim_format_name_get(sfmt));
		strncat(tmp, tmp2, 256);
	}
	if (mask)
	{
		char tmp2[256];

		sfmt = enesim_surface_format_get(mask);
		snprintf(tmp2, 256, "_%s", enesim_format_name_get(sfmt));
		strncat(tmp, tmp2, 256);
	}
	/* append the color (transparent/opaque) */
	if (color)
	{
		char tmp2[256];

		snprintf(tmp2, 256, "_%s", opacity_get(*color, sfmt));
		strncat(tmp, tmp2, 256);
	}
	snprintf(file, 256, "%s.png", tmp);
	surface_save(dst, file);
}


int main(int argc, char **argv)
{
	char *short_options = "dhf:t:r:m:w:e:b:c:";
	struct option long_options[] = {
		{"opt_debug", 0, 0, 'd'},
		{"help", 0, 0, 'h'},
		{"times", 1, 0, 't'},
		{"file", 1, 0, 'f'},
		{"rop", 1, 0, 'r'},
		{"fmt", 1, 0, 'm'},
		{"width", 1, 0, 'w'},
		{"height", 1, 0, 'e'},
		{"bench", 1, 0, 'b'},
		{"cpu", 1, 0, 'c'},
		{0, 0, 0, 0}
	};
	int option;
	char c;
	char *file = "benchmark.txt";

	enesim_init();
	/* handle the parameters */
	while ((c = getopt_long(argc, argv, short_options, long_options,
			&option)) != -1)
	{
		/* arm bug ? */
		if (c == 255)
			goto ok;
		switch (c)
		{
			case 'h':
				help();
				return 0;
			case 't':
				opt_times = atoi(optarg);
				break;
			case 'f':
				file = optarg;
				opt_file = fopen(file, "w+");
				if (!opt_file)
				{
					help();
					return 0;
				}
				break;
			case 'd':
				opt_debug = 1;
				break;
			case 'r':
				if (!rop_get(optarg, &opt_rop))
				{
					rop_help();
					return 1;
				}
				break;
			case 'm':

				if (!fmt_get(optarg, &opt_fmt))
				{
					fmt_help();
					return 1;
				}
				break;
			case 'w':
				opt_width = atoi(optarg);
				break;
			case 'e':
				opt_height = atoi(optarg);
				break;
			case 'b':
				if (!bench_get(optarg))
				{
					bench_help();
					return 1;
				}
				break;
			default:
				break;
		}
	}
ok:
	printf("Enesim Bench\n");
	printf("* BENCH = %s\n", opt_bench);
	printf("* SIZE = %dx%d\n", opt_width, opt_height);
	printf("* ROP = %s\n", rop_name(opt_rop));
	printf("* FMT = %s\n", enesim_format_name_get(opt_fmt));
	printf("* TIMES = %d\n", opt_times);

	if (!strcmp(opt_bench, "renderer"))
	{
		//renderer_bench();
	}
	else if (!strcmp(opt_bench, "rasterizer"))
	{
		rasterizer_bench();
	}
	else if (!strcmp(opt_bench, "compositor"))
	{
		compositor_bench();
	}
	else if (!strcmp(opt_bench, "scaler"))
	{
		//scaler_bench();
	}
	else if (!strcmp(opt_bench, "raddist"))
	{
		raddist_bench();
	}
	else if (!strcmp(opt_bench, "dispmap"))
	{
		dispmap_bench();
	}
	else if (!strcmp(opt_bench, "checker"))
	{
		checker_bench();
	}
	else if (!strcmp(opt_bench, "stripes"))
	{
		stripes_bench();
	}
	else if (!strcmp(opt_bench, "hswitch"))
	{
		hswitch_bench();
	}
	else if (!strcmp(opt_bench, "circle"))
	{
		circle_bench();
	}
	else if (!strcmp(opt_bench, "ellipse"))
	{
		ellipse_bench();
	}
	else if (!strcmp(opt_bench, "rectangle"))
	{
		rectangle_bench();
	}
	else if (!strcmp(opt_bench, "perlin"))
	{
		perlin_bench();
	}
	else if (!strcmp(opt_bench, "gradient_linear"))
	{
		gradient_linear_bench();
	}
	else if (!strcmp(opt_bench, "compound"))
	{
		compound_bench();
	}
	else if (!strcmp(opt_bench, "all"))
	{
		compositor_bench();
		//transformer_bench();
		//scaler_bench();
		rasterizer_bench();
		raddist_bench();
		dispmap_bench();
		checker_bench();
		stripes_bench();
		hswitch_bench();
		circle_bench();
		ellipse_bench();
		rectangle_bench();
		perlin_bench();
		gradient_linear_bench();
		compound_bench();
	}
	enesim_shutdown();
	/* this bench should be on test
	 * matrix_bench();
	 */

	return 0;
}
