#include "evas_common.h"
#include <math.h>


static void _get_word(char *in, char *key);
static void evas_common_gradient_map_argb(RGBA_Draw_Context *dc, RGBA_Gradient *gr,
					  int spread);
static void evas_common_gradient_map_ahsv(RGBA_Draw_Context *dc, RGBA_Gradient *gr,
					  int spread);

static void
_get_word(char *in, char *key)
{
   char  *p, *pp;
   int    l;

   if (!key) return;
   *key = 0;
   if (!in || !*in) return;

   p = in;
   while (*p && isspace(*p))
	p++;
   if (!*p) return;
   pp = p;
   while (*pp && !isspace(*pp))
	pp++;
   l = pp - p;
   if (l >= 255) return;
   *(p + l) = 0;
   strncpy(key, p, l + 1);
}

char *
evas_common_gradient_get_key_fval(char *in, char *key, float *val)
{
   char   *p, *pp, sval[256];

   if (!key || !val) return NULL;
   *key = 0;
   if (!in || !*in) return NULL;
   p = strchr(in, '=');
   if (!p || !*p)  return NULL;
   *p = 0;  p++;
   if (!*p)  return NULL;
   pp = strchr(p, ';');
   if (!pp || !*pp)  return NULL;
   _get_word(in, key);
   if (!*key)  return NULL;
   *pp = 0;
   _get_word(p, sval);
   if (!sval[0])  return NULL;
   *val = atof(sval);
   return (pp + 1);
}

void
evas_common_gradient_init(void)
{
}

void
evas_common_gradient_shutdown(void)
{
}

RGBA_Gradient *
evas_common_gradient_new(void)
{
   RGBA_Gradient *gr;

   gr = calloc(1, sizeof(RGBA_Gradient));
   return gr;
}

void
evas_common_gradient_free(RGBA_Gradient *gr)
{
   if (!gr) return;
   evas_common_gradient_colors_clear(gr);
   if (gr->type.name) free(gr->type.name);
   if (gr->type.params) free(gr->type.params);
   if (gr->map.data) free(gr->map.data);
   free(gr);
}

void
evas_common_gradient_colors_clear(RGBA_Gradient *gr)
{
   if (!gr) return;
   if (gr->colors)
     {
	Evas_Object_List *l;

	while (gr->colors)
	  {
	   l = gr->colors;
	   gr->colors = evas_object_list_remove(gr->colors, gr->colors);
	   free(l);
	  }
	gr->colors = NULL;
	gr->ncolors = 0;
	gr->len = 0;
     }
}

void
evas_common_gradient_color_add(RGBA_Gradient *gr, int r, int g, int b, int a, int dist)
{
   RGBA_Gradient_Color *gc, *gcm, *gc_last;

   if (!gr) return;
   gc = malloc(sizeof(RGBA_Gradient_Color));
   if (!gc) return;
   if (dist < 1) dist = 1;
   if (r < 0) r = 0;  if (r > 255) r = 255;
   if (g < 0) g = 0;  if (g > 255) g = 255;
   if (b < 0) b = 0;  if (b > 255) b = 255;
   if (a < 0) a = 0;  if (a > 255) a = 255;
   gc->r = r;
   gc->g = g;
   gc->b = b;
   gc->a = a;
   gc->dist = dist;
   
   if (!gr->colors)
     {
	gr->colors = evas_object_list_append(gr->colors, gc);
	gr->ncolors = 1;
	gr->len = 1;
	if (a < 255)
	   gr->has_alpha = 1;
	return;
     }
   gcm = malloc(sizeof(RGBA_Gradient_Color));
   if (!gcm) { free(gc); return; }
   gc_last = (RGBA_Gradient_Color *)(gr->colors->last);
   gcm->r = (gc_last->r + r) / 2;
   gcm->g = (gc_last->g + g) / 2;
   gcm->b = (gc_last->b + b) / 2;
   gcm->a = (gc_last->a + a) / 2;
   gcm->dist = dist;
   gr->colors = evas_object_list_append(gr->colors, gcm);   
   gr->len += gc_last->dist;
   gr->colors = evas_object_list_append(gr->colors, gc);
   gr->len += dist;
   gr->ncolors += 2;
   if (a < 255)
	gr->has_alpha = 1;
}

void
evas_common_gradient_type_set(RGBA_Gradient *gr, char *name)
{
   if (!gr) return;
   if (!name || !*name)
	name = "linear";
   gr->type.geometer = evas_common_gradient_geometer_get(name);
   if (gr->type.name) free(gr->type.name);
   gr->type.name = strdup(gr->type.geometer->name);
}

void
evas_common_gradient_type_params_set(RGBA_Gradient *gr, char *params)
{
   if (!gr) return;
   if (params && !*params) 
	params = NULL;
   if (gr->type.params) free(gr->type.params);
   gr->type.params = NULL;
   if (params) gr->type.params = strdup(params);
}

void
evas_common_gradient_fill_set(RGBA_Gradient *gr, int x, int y, int w, int h)
{
   if (!gr) return;
   gr->fill.x = x;
   gr->fill.y = y;
   gr->fill.w = w;
   gr->fill.h = h;
}

RGBA_Gradient *
evas_common_gradient_geometry_init(RGBA_Gradient *gr, int spread)
{
   if (!gr) return NULL;
   gr->type.geometer = evas_common_gradient_geometer_get(gr->type.name);
   if (!gr->type.geometer)
     {
	evas_common_gradient_free(gr);
	return NULL;
     }
     gr->type.geometer->setup_geom(gr, spread);
     return gr;
}

int 
evas_common_gradient_has_alpha(RGBA_Gradient *gr, int spread)
{
   if (!gr || !gr->type.geometer) return 0;
   return gr->type.geometer->has_alpha(gr, spread);
}

RGBA_Gradient_Type  *
evas_common_gradient_geometer_get(char *name)
{
   RGBA_Gradient_Type  *geom = NULL;

   if (!name || !*name)
	name = "linear";
   if (!strcmp(name,"linear"))
	geom = evas_common_gradient_linear_get();
   else if (!strcmp(name,"radial"))
	geom = evas_common_gradient_radial_get();
   else if (!strcmp(name,"angular"))
	geom = evas_common_gradient_angular_get();
   else if (!strcmp(name,"sinusoidal"))
	geom = evas_common_gradient_sinusoidal_get();
   else if (!strcmp(name,"rectangular"))
	geom = evas_common_gradient_rectangular_get();
   if (!geom)
	geom = evas_common_gradient_linear_get();
   return geom;
}


void
evas_common_gradient_draw(RGBA_Image *dst, RGBA_Draw_Context *dc,
			  int x, int y, int w, int h, RGBA_Gradient *gr,
			  double angle, int spread)
{
   Gfx_Func_Gradient_Span   sfunc;
   Gfx_Func_Blend_Src_Dst   bfunc;
   int             len;
   int             xin, yin, xoff, yoff;
   int             axx, axy, ayx, ayy;
   DATA32          *pdst, *dst_end, *buf, *map;
   RGBA_Image      *line_buf;
   void            *gdata;
   unsigned char   buf_a = 0;

   if (!dst || !dc || !gr)
	return;
   if (!gr->map.data || !gr->type.geometer)
	return;
   if ((gr->fill.w < 1) || (gr->fill.h < 1))
	return;
   if ((w < 1) || (h < 1))
	return;
   if (!(RECTS_INTERSECT(x, y, w, h, 0, 0, dst->image->w, dst->image->h)))
	return;

   xin = x;  yin = y;
   if (x < 0)
     {
	w += x;
	x = 0;
     }
   if (y < 0)
     {
	h += y;
	y = 0;
     }
   if ((w <= 0) || (h <= 0)) return;
   if ((x + w) > dst->image->w) w = dst->image->w - x;
   if ((y + h) > dst->image->h) h = dst->image->h - y;
   if ((w <= 0) || (h <= 0)) return;

   if (dc->clip.use)
     {
	if (dc->clip.x > x)
	  {
	     w += x - dc->clip.x;
	     x = dc->clip.x;
	  }
	if (dc->clip.y > y)
	  {
	     h += y - dc->clip.y;
	     y = dc->clip.y;
	  }
	if ((w <= 0) || (h <= 0)) return;
	if ((dc->clip.x + dc->clip.w) < (x + w))
	  w = dc->clip.x + dc->clip.w - x;
	if ((dc->clip.y + dc->clip.h) < (y + h))
	  h = dc->clip.y + dc->clip.h - y;
     }
   if ((w <= 0) || (h <= 0)) return;

   xoff = (xin - x) - gr->fill.x;
   yoff = (yin - y) - gr->fill.y;

   line_buf = evas_common_image_line_buffer_obtain(w);
   if (!line_buf)
     {
	evas_common_image_line_buffer_release();
	return;
     }
   
   if ((spread < 0) || (spread > 2)) spread = 0;
   buf_a = gr->type.geometer->has_alpha(gr, spread);
   if (buf_a)
   	line_buf->flags |= RGBA_IMAGE_HAS_ALPHA;
   else
   	line_buf->flags &= ~RGBA_IMAGE_HAS_ALPHA;
   
   gdata = gr->type.geometer->geom_data;

   sfunc = gr->type.geometer->get_span_func(gr, spread, dc->anti_alias);
   bfunc = evas_common_draw_func_blend_get(line_buf, dst, w);

   angle = (angle * M_PI) / 180.0;
   axx = (cos(angle) * 65536.0);
   ayy = axx;
   axy = (sin(angle) * 65536.0);
   ayx = -axy;

   map = gr->map.data;
   len = gr->map.len;
   buf = line_buf->image->data;
   pdst = dst->image->data + (y * dst->image->w) + x;
   dst_end = pdst + (h * dst->image->w);
   
   while (pdst < dst_end)
     {
	evas_common_cpu_end_opt();
	sfunc(map, len, buf, w, xoff, yoff, axx, axy, ayx, ayy, gdata);
	bfunc(buf, pdst, w);
	pdst += dst->image->w;
	yoff++;
     }
   
   evas_common_image_line_buffer_release();
}


static void
evas_common_gradient_map_argb(RGBA_Draw_Context *dc, RGBA_Gradient *gr, int spread)
{
   Evas_Object_List    *lc;
   DATA32              *pmap, *map_end;
   RGBA_Gradient_Color *gc, *gc_next;
   int   d, n;
   int   i, prev_i, ii, dii;
   int   r, g, b, a;
   int   next_r, next_g, next_b, next_a;
   int   rr, drr, gg, dgg, bb, dbb, aa, daa;
   int   mr = 256, mg = 256, mb = 256, ma = 256;

   if (!gr || !gr->colors || !dc)
	return;
   if ((gr->fill.w < 1) || (gr->fill.h < 1))
	return;
   if (!gr->type.geometer)
	return;
   d = gr->type.geometer->get_map_len(gr, spread);
   if (d < 1)
     {
	if (gr->map.data)
	   free(gr->map.data);
	gr->map.data = NULL;
	gr->map.len = 0;
	return;
     }
   if ((d != gr->map.len) || (!gr->map.data))
	gr->map.data = realloc(gr->map.data, d * sizeof(DATA32));
   if (!gr->map.data)
     { gr->map.len = 0; return; }
   gr->map.len = d;
   gr->map.has_alpha = gr->has_alpha;

   pmap = gr->map.data;
   map_end = pmap + gr->map.len;

   gc = (RGBA_Gradient_Color *)gr->colors;
   r = gc->r;  g = gc->g;  b = gc->b;  a = gc->a;
   if (dc->mul.use)
     {
	mr = 1 + R_VAL(&dc->mul.col);  mg = 1 + G_VAL(&dc->mul.col);
	mb = 1 + B_VAL(&dc->mul.col);  ma = 1 + A_VAL(&dc->mul.col);
	if (ma < 256) gr->map.has_alpha = 1;
	r = (r * mr) >> 8;  g = (g * mg) >> 8;
	b = (b * mb) >> 8;  a = (a * ma) >> 8;
     }

   /* two 'trivial' cases: */
   
   /* just one color, or the grad map is of length 1 */
   if (!gr->colors->next || (gr->map.len == 1))
     {
	DATA32 c = ARGB_JOIN(a,r,g,b);

	while (pmap < map_end)
	   *pmap++ = c;
	return;
     }

   /* do simple gradient mapping if just two equally weighted colors */
   if ((gr->ncolors == 3) && ((2 * gc->dist) == (gr->len - 1)))
     {
	gc_next = (RGBA_Gradient_Color *)(gr->colors->last);
	next_r = gc_next->r;  next_g = gc_next->g;
	next_b = gc_next->b;  next_a = gc_next->a;
	if (dc->mul.use)
        {
	   next_r = (next_r * mr) >> 8;  next_g = (next_g * mg) >> 8;
	   next_b = (next_b * mb) >> 8;  next_a = (next_a * ma) >> 8;
        }
	d = (gr->map.len - 1);
	rr = r << 16;  drr = ((next_r - r) << 16) / d;
	gg = g << 16;  dgg = ((next_g - g) << 16) / d;
	bb = b << 16;  dbb = ((next_b - b) << 16) / d;
	aa = a << 16;  daa = ((next_a - a) << 16) / d;

	while (pmap < map_end)
	  {
	   r = rr >> 16;  r += (rr - (r << 16)) >> 15;
	   g = gg >> 16;  g += (gg - (g << 16)) >> 15;
	   b = bb >> 16;  b += (bb - (b << 16)) >> 15;
	   a = aa >> 16;  a += (aa - (a << 16)) >> 15;
	   *pmap++ = ARGB_JOIN(a,r,g,b);
	   rr += drr;  gg += dgg;  bb += dbb;  aa += daa;
	  }
	return;
     }

   /* general case, gr->map.len > 1 */
   lc = gr->colors->next;
   gc_next = (RGBA_Gradient_Color *)lc;
   next_r = gc_next->r;  next_g = gc_next->g;
   next_b = gc_next->b;  next_a = gc_next->a;
   if (dc->mul.use)
     {
	next_r = (next_r * mr) >> 8;  next_g = (next_g * mg) >> 8;
	next_b = (next_b * mb) >> 8;  next_a = (next_a * ma) >> 8;
     }

   n = (gr->len - 1) << 16;
   d = gc->dist * (gr->map.len - 1);

   i = 1;
   prev_i = i;
   ii = i << 16;
   dii = n / d;

   rr = r << 16;  drr = ((next_r - r) * dii);
   gg = g << 16;  dgg = ((next_g - g) * dii);
   bb = b << 16;  dbb = ((next_b - b) * dii);
   aa = a << 16;  daa = ((next_a - a) * dii);

   while (pmap < map_end)
     {
	i = ii >> 16;
	if ((prev_i != i) && (i < gr->ncolors))
	  {
	    while (prev_i < i)
		{
		 prev_i++;
		 gc = gc_next;  lc = lc->next;
		 gc_next = (RGBA_Gradient_Color *)lc;
		}
	    d = gc->dist * (gr->map.len - 1);
	    dii = n / d;
	    r = gc->r;  g = gc->g;  b = gc->b;  a = gc->a;
	    next_r = gc_next->r;  next_g = gc_next->g;
	    next_b = gc_next->b;  next_a = gc_next->a;
	    if (dc->mul.use)
		{
		 r = (r * mr) >> 8;  g = (g * mg) >> 8;
		 b = (b * mb) >> 8;  a = (a * ma) >> 8;
		 next_r = (next_r * mr) >> 8;  next_g = (next_g * mg) >> 8;
		 next_b = (next_b * mb) >> 8;  next_a = (next_a * ma) >> 8;
		}
	    drr = ((next_r - r) * dii);
	    dgg = ((next_g - g) * dii);
	    dbb = ((next_b - b) * dii);
	    daa = ((next_a - a) * dii);
	  }
	r = rr >> 16;  r += (rr - (r << 16)) >> 15;
	g = gg >> 16;  g += (gg - (g << 16)) >> 15;
	b = bb >> 16;  b += (bb - (b << 16)) >> 15;
	a = aa >> 16;  a += (aa - (a << 16)) >> 15;
	if (r > 255) r = 255;  if (r < 0) r = 0;
	if (g > 255) g = 255;  if (g < 0) g = 0;
	if (b > 255) b = 255;  if (b < 0) b = 0;
	if (a > 255) a = 255;  if (a < 0) a = 0;
	*pmap++ = ARGB_JOIN(a,r,g,b);
	rr += drr;  gg += dgg;  bb += dbb;  aa += daa;
	ii += dii;
     }
}

static void
evas_common_gradient_map_ahsv(RGBA_Draw_Context *dc, RGBA_Gradient *gr, int spread)
{
   Evas_Object_List    *lc;
   DATA32              *pmap, *map_end;
   RGBA_Gradient_Color *gc, *gc_next;
   int   d, n;
   int   i, prev_i, ii, dii;
   int   h, s, v, a;
   int   next_h, next_s, next_v, next_a;
   int   hh, dhh, ss, dss, vv, dvv, aa, daa;
   int   r, g, b;
   int   mr = 256, mg = 256, mb = 256, ma = 256;

   if (!gr || !gr->colors || !dc)
	return;
   if ((gr->fill.w < 1) || (gr->fill.h < 1))
	return;
   if (!gr->type.geometer)
	return;
   d = gr->type.geometer->get_map_len(gr, spread);
   if (d < 1)
     {
	if (gr->map.data)
	   free(gr->map.data);
	gr->map.data = NULL;
	gr->map.len = 0;
	return;
     }
   if ((d != gr->map.len) || (!gr->map.data))
	gr->map.data = realloc(gr->map.data, d * sizeof(DATA32));
   if (!gr->map.data)
     { gr->map.len = 0; return; }
   gr->map.len = d;
   gr->map.has_alpha = gr->has_alpha;

   pmap = gr->map.data;
   map_end = pmap + gr->map.len;
   
   if (dc->mul.use)
     {
	mr = 1 + R_VAL(&dc->mul.col);  mg = 1 + G_VAL(&dc->mul.col);
	mb = 1 + B_VAL(&dc->mul.col);  ma = 1 + A_VAL(&dc->mul.col);
	if (ma < 256) gr->map.has_alpha = 1;
     }

   gc = (RGBA_Gradient_Color *)gr->colors;
   r = gc->r;  g = gc->g;  b = gc->b;  a = gc->a;

   /* two 'trivial' cases: */
   
   /* just one color, or the grad map is of length 1 */
   if (!gr->colors->next || (gr->map.len == 1))
     {
	DATA32 c;

	if (dc->mul.use)
	  {
	   r = (r * mr) >> 8;  g = (g * mg) >> 8;
	   b = (b * mb) >> 8;  a = (a * ma) >> 8;
	  }
	c = ARGB_JOIN(a,r,g,b);
	while (pmap < map_end)
	   *pmap++ = c;
	return;
     }

   evas_common_convert_rgb_to_hsv_int(r, g, b, &h, &s, &v);

   /* do simple gradient mapping if just two equally weighted colors */
   if ((gr->ncolors == 3) && ((2 * gc->dist) == (gr->len - 1)))
     {
	gc_next = (RGBA_Gradient_Color *)(gr->colors->last);
	next_a = gc_next->a;
	evas_common_convert_rgb_to_hsv_int(gc_next->r, gc_next->g, gc_next->b,
	                                   &next_h, &next_s, &next_v);
	d = (gr->map.len - 1);
	hh = h << 16;  dhh = ((next_h - h) << 16) / d;
	ss = s << 16;  dss = ((next_s - s) << 16) / d;
	vv = v << 16;  dvv = ((next_v - v) << 16) / d;
	aa = a << 16;  daa = ((next_a - a) << 16) / d;

	while (pmap < map_end)
	  {
	   h = hh >> 16;  h += (hh - (h << 16)) >> 15;
	   s = ss >> 16;  s += (ss - (s << 16)) >> 15;
	   v = vv >> 16;  v += (vv - (v << 16)) >> 15;
	   a = aa >> 16;  a += (aa - (a << 16)) >> 15;
	   evas_common_convert_hsv_to_rgb_int(h, s, v, &r, &g, &b);
	   if (dc->mul.use)
	     {
		r = (r * mr) >> 8;  g = (g * mg) >> 8;
		b = (b * mb) >> 8;  a = (a * ma) >> 8;
	     }
	   *pmap++ = ARGB_JOIN(a,r,g,b);
	   hh += dhh;  ss += dss;  vv += dvv;  aa += daa;
	  }
	return;
     }

   /* general case, gr->map.len > 1 */
   lc = gr->colors->next;
   gc_next = (RGBA_Gradient_Color *)lc;
   next_a = gc_next->a;
   evas_common_convert_rgb_to_hsv_int(gc_next->r, gc_next->g, gc_next->b,
                                      &next_h, &next_s, &next_v);

   n = (gr->len - 1) << 16;
   d = gc->dist * (gr->map.len - 1);

   i = 1;
   prev_i = i;
   ii = i << 16;
   dii = n / d;

   hh = h << 16;  dhh = ((next_h - h) * dii);
   ss = s << 16;  dss = ((next_s - s) * dii);
   vv = v << 16;  dvv = ((next_v - v) * dii);
   aa = a << 16;  daa = ((next_a - a) * dii);

   while (pmap < map_end)
     {
	i = ii >> 16;
	if ((prev_i != i) && (i < gr->ncolors))
	  {
	    while (prev_i < i)
		{
		 prev_i++;
		 gc = gc_next;  lc = lc->next;
		 gc_next = (RGBA_Gradient_Color *)lc;
		}
	    d = gc->dist * (gr->map.len - 1);
	    dii = n / d;
	    a = gc->a;
	    evas_common_convert_rgb_to_hsv_int(gc->r, gc->g, gc->b,
	                                       &h, &s, &v);
	    next_a = gc_next->a;
	    evas_common_convert_rgb_to_hsv_int(gc_next->r, gc_next->g, gc_next->b,
	                                       &next_h, &next_s, &next_v);

	    dhh = ((next_h - h) * dii);
	    dss = ((next_s - s) * dii);
	    dvv = ((next_v - v) * dii);
	    daa = ((next_a - a) * dii);
	  }
	h = hh >> 16;  h += (hh - (h << 16)) >> 15;
	s = ss >> 16;  s += (ss - (s << 16)) >> 15;
	v = vv >> 16;  v += (vv - (v << 16)) >> 15;
	a = aa >> 16;  a += (aa - (a << 16)) >> 15;
	if (h > 1530) h = 1530;  if (h < 0) h = 0;
	if (s > 255) s = 255;  if (s < 0) s = 0;
	if (v > 255) v = 255;  if (v < 0) v = 0;
	if (a > 255) a = 255;  if (a < 0) a = 0;
	evas_common_convert_hsv_to_rgb_int(h, s, v, &r, &g, &b);
	if (dc->mul.use)
	  {
	   r = (r * mr) >> 8;  g = (g * mg) >> 8;
	   b = (b * mb) >> 8;  a = (a * ma) >> 8;
	  }
	*pmap++ = ARGB_JOIN(a,r,g,b);
	hh += dhh;  ss += dss;  vv += dvv;  aa += daa;
	ii += dii;
     }
}

void
evas_common_gradient_map(RGBA_Draw_Context *dc, RGBA_Gradient *gr, int spread)
{
   if (!gr || !dc) return;
   if (dc->interpolation.color_space == _EVAS_COLOR_SPACE_AHSV)
     {
	evas_common_gradient_map_ahsv(dc, gr, spread);
	return;
     }
   evas_common_gradient_map_argb(dc, gr, spread);
}

/* from imlib2 code */
void
evas_common_convert_hsv_to_rgb(float h, float s, float v, int *r, int *g, int *b)
{
   int   i;
   float f, p, q, t;

   v = 255 * v;   
   if (s == 0)
     {
	if (r) *r = v;
	if (g) *g = v;
	if (b) *b = v;
	return;
     }
   h /= 60;
   i = h;
   f = h - i;
   
   s = v * s;
   f = s * f;
   p = v - s;
   q = v - f;
   t = p + f;
   switch (i)
     {
	case 0:
	  if (r) *r = v;
	  if (g) *g = t;
	  if (b) *b = p;
	  return;
	case 1:
	  if (r) *r = q;
	  if (g) *g = v;
	  if (b) *b = p;
	  return;
	case 2:
	  if (r) *r = p;
	  if (g) *g = v;
	  if (b) *b = t;
	  return;
	case 3:
	  if (r) *r = p;
	  if (g) *g = q;
	  if (b) *b = v;
	  return;
	case 4:
	  if (r) *r = t;
	  if (g) *g = p;
	  if (b) *b = v;
	  return;
	default:
	  if (r) *r = v;
	  if (g) *g = p;
	  if (b) *b = q;
     }
}

void
evas_common_convert_rgb_to_hsv(int r, int g, int b, float *h, float *s, float *v)
{
   float  min, max, del;
   
   min = MIN(r,g);  min = MIN(min,b);
   max = MAX(r,g);  max = MAX(max,b);
   del = max - min;
   
   if (v) *v = (max / 255);
   if ((max == 0) || (del == 0))
     {
	if (s) *s = 0;
	if (h) *h = 0;
	return; 
     }

   if (s) *s = (del / max);
   
   if (!h) return;
   if (r == max)
	*h = ((g - b) / del);
   else if (g == max)
	*h = 2 + ((b - r) / del);
   else if (b == max)
	*h = 4 + ((r - g) / del);

   *h *= 60;
   if (*h < 0) *h += 360;
}

void
evas_common_convert_hsv_to_rgb_int(int h, int s, int v, int *r, int *g, int *b)
{
   int   i, f, p, q, t;

   if (s == 0)
     {
	*r = *g = *b = v;
	return;
     }
     
   i = h / 255;
   f = h - (i * 255);
   s = (v * s) / 255;
   f = (s * f) / 255;
   p = v - s;
   q = v - f;
   t = p + f;
   switch (i)
     {
	case 0:
	  *r = v; *g = t; *b = p;
	  return;
	case 1:
	  *r = q; *g = v; *b = p;
	  return;
	case 2:
	  *r = p; *g = v; *b = t;
	  return;
	case 3:
	  *r = p; *g = q; *b = v;
	  return;
	case 4:
	  *r = t; *g = p; *b = v;
	  return;
	default:
	  *r = v; *g = p; *b = q;
     }
}

void
evas_common_convert_rgb_to_hsv_int(int r, int g, int b, int *h, int *s, int *v)
{
   int  min, max, del;
   
   min = MIN(r,g);  min = MIN(min,b);
   max = MAX(r,g);  max = MAX(max,b);
   del = max - min;
   
   *v = max;
   if ((max == 0) || (del == 0))
     {
	*s = *h = 0;
	return; 
     }

   *s = ((del * 255) / max);
   
   if (r == max)
	*h = (((g - b) * 255) / del);
   else if (g == max)
	*h = 510 + (((b - r) * 255) / del);
   else if (b == max)
	*h = 1020 + (((r - g) * 255) / del);

   if (*h < 0) *h += 1530;

}

