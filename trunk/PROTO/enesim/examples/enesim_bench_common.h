#ifndef ENESIM_BENCH_COMMON_H_
#define ENESIM_BENCH_COMMON_H_

#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <getopt.h>
#include <math.h>

#include "Eina.h"
#include "Enesim.h"

#include "image.h"

extern int opt_width;
extern int opt_height;
extern int opt_times;
extern FILE *opt_file;
extern int opt_debug;
extern int opt_rop;
extern Enesim_Format opt_fmt;
extern Enesim_Cpu *opt_cpu;

double get_time(void);
void test_finish(const char *name, Enesim_Rop rop, Enesim_Surface *dst,
		Enesim_Surface *src, uint32_t *color, Enesim_Surface *mask);
Enesim_Surface * test_pattern1(int w);
Enesim_Surface * test_pattern2(int w);
void test_gradient2(Enesim_Surface *s);
void test_gradient1(Enesim_Surface *s);
void test_gradient3(Enesim_Surface *s);
void renderer_bench(void);
void rasterizer_bench(void);
void drawer_bench(void);
void cpu_bench(void);
void transformer_bench(void);
void spanner_bench(void);
void scaler_bench(void);
void raddist_bench(void);
char * opacity_get(uint32_t color, Enesim_Format f);
void surfaces_create(Enesim_Surface **src, Enesim_Format sfmt,
		Enesim_Surface **dst, Enesim_Format dfmt,
		Enesim_Surface **msk, Enesim_Format mfmt);
void surface_save(Enesim_Surface *s, const char *name);

#endif /*ENESIM_BENCH_COMMON_H_*/
