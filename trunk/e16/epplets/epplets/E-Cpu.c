#include "epplet.h"

int                 cpus = 0;
double             *prev_val = NULL;
int                *load_val = NULL;
Window              win;
RGB_buf             buf;
Epplet_gadget       da, b_close, b_config, b_help, pop;
int                *flame = NULL;
int                *vspread, *hspread, *residual;
unsigned char       rm[255], gm[255], bm[255];
ConfigItem          config_defaults[] = { { "color1", "30 90 90" }, { "color2", "50 255 255" }, { "color3", "255 255 255" } };
const int           num_defaults = 3;

static int          colors[] =
{
   30, 90,  90,
   50, 255, 255,
   255, 255, 255,

   255, 0,  0,
   255, 255, 0,
   255, 255, 255,

   0, 255,  0,
   255, 255, 0,
   255, 255, 255,

   0, 0,  255,
   255, 0, 255,
   255, 255, 255,

   0, 0,  200,
   40, 80, 255,
   100, 200, 255,

   80,  90,  140,
   140, 150, 180,
   255, 230, 200,

   20,  40,  180,
   255, 160, 0,
   255, 255, 100

};

static void save_conf(int d1, int d2, int d3, int d4, int d5, int d6, int d7, int d8, int d9);
static void load_conf(void);
static void cb_in(void *data, Window w);
static void cb_out(void *data, Window w);
static void cb_timer(void *data);
static void cb_close(void *data);
static void cb_config(void *data);
static void cb_help(void *data);
static int  count_cpus(void);
static void draw_flame(void);
static void flame_col(int r1, int g1, int b1, int r2, int g2, int b2, int r3, int g3, int b3);

#define VARIANCE 40
#define VARTREND 16

#define HSPREAD  10
#define VSPREAD  160
#define RESIDUAL 75

#define MAX      255

static void 
flame_col(int r1, int g1, int b1, int r2, int g2, int b2, int r3, int g3, int b3)
{
   int i, d, d1, d2, rr, gg, bb;
   
   for (i = 0; i < 25; i++)
     {
	rm[i] = (r1 * i) / 25;
	gm[i] = (g1 * i) / 25;
	bm[i] = (b1 * i) / 25;
     }
   for (i = 0; i < 25; i++)
     {
	rm[25 + i] = ((r2 * i) + (r1 * (25 - i))) / 25;
	gm[25 + i] = ((g2 * i) + (g1 * (25 - i))) / 25;
	bm[25 + i] = ((b2 * i) + (b1 * (25 - i))) / 25;
     }
   for (i = 0; i < 25; i++)
     {
	rm[50 + i] = ((r3 * i) + (r2 * (25 - i))) / 25;
	gm[50 + i] = ((g3 * i) + (g2 * (25 - i))) / 25;
	bm[50 + i] = ((b3 * i) + (b2 * (25 - i))) / 25;
     }

   for (i = 75; i < 255; i++)
     {
	rm[i] = rm[74];
	gm[i] = gm[74];
	bm[i] = bm[74];
     }
}

static void
draw_flame(void)
{
   unsigned char *rgb, *rptr;
   int x, y, *ptr, val1, val2, val3, i, j;
   
   if (!flame)
     {
	vspread = malloc(40 * sizeof(int));
	hspread = malloc(40 * sizeof(int));
	residual = malloc(40 * sizeof(int));
	flame = malloc(sizeof(int) * 40 * 40);
	memset(flame, 0, sizeof(int) * 40 * 40);
     }

   ptr = flame + (39 * 40);
   for (x = 0; x < 40; x++)
     {
	vspread[x] = VSPREAD + (load_val[(x * cpus) / 40] / 50);
	hspread[x] = HSPREAD + (load_val[(x * cpus) / 40] / 50);
	residual[x] = RESIDUAL + (load_val[(x * cpus) / 40] / 50);
	ptr[x] = (rand() % ((load_val[(x * cpus) / 40]) + 155));
	if (ptr[x] > MAX)
	   ptr[x] = 0;
	else if (ptr[x] < 0)
	   ptr[x] = 0;
     }
   
   for (i = 0; i < cpus; i++)
     {
	for (x = (40 / (cpus * 2)) - 1 + (i * (40 / cpus));
	     x <= (40 / (cpus * 2)) + 1 + (i * (40 / cpus));
	     x++)
	  {
	     j = (load_val[i] * 40) / 100;
	     ptr = flame + ((40 - j) * (40)) + (x);
	     for (y = 0; y < j; y++)
	       {
		  ptr[0] += ((y * 64 * load_val[i]) / (j * 100));
		  ptr += 40;
	       }
	  }
     }
   for (y = 39; y >= 2; y--)
     {	
	ptr = flame + (y * 40);
	for (x = 1; x < 39; x++)
	  {
	     val1 = (ptr[x] * vspread[x]) >> 8;
	     val2 = (ptr[x] * hspread[x]) >> 8;
	     val3 = (ptr[x] * residual[x]) >> 8;
	     
	     ptr[x - 1] += val2;
	     if (ptr[x - 1] > MAX)
		ptr[x - 1] = MAX;
	     ptr[x - 40] += val1;
	     if (ptr[x - 40] > MAX)
		ptr[x - 40] = MAX;
	     ptr[x + 1] += val2;
	     if (ptr[x + 1] > MAX)
		ptr[x + 1] = MAX;
	     ptr[x] = val3;
	  }
     }
   for (x = 0; x < 40; x++)
      flame[x] = 0;
   for (x = 0; x < 40; x++)
      flame[40 + x] /= 2;
   for (y = 0; y < 40; y++)
      flame[y * 40] = 0;
   for (y = 0; y < 40; y++)
      flame[(y * 40) + 39] = 0;
   for (y = 0; y < 40; y++)
      flame[(y * 40) + 38] /= 2;
   rgb = Epplet_get_rgb_pointer(buf);
   for (y = 0; y < 40; y++)
     {
	ptr = flame + (y * 40) + 1;
	rptr = rgb + (y * 40 * 3);
	for (x = 0; x < 40; x++)
	  {
	     val1 = ptr[x] & 0xff;
	     rptr[0] = rm[val1];
	     rptr[1] = gm[val1];
	     rptr[2] = bm[val1];
	     rptr += 3;
	  }
     }
}

static void
cb_timer(void *data)
{
   static FILE *f;
   int i;

   f = fopen("/proc/stat", "r");
   if (f)
     {
	char s[256];
	
	if (cpus > 1)
	   fgets(s, 255, f);
	for (i = 0; i < cpus; i++)
	  {
	     char ss[64];
	     double val, val2;
	     
	     fgets(s, 255, f);
	     sscanf(s, "%*s %s %*s %*s %*s", ss);
	     val = atof(ss);
	     val2 = val - prev_val[i];
	     prev_val[i] = val;
	     val2 *= 10;
	     if (val2 > 100)
		val2 = 100;
	     load_val[i] = val2;
	  }
	fclose(f);
	draw_flame();
	Epplet_paste_buf(buf, win, 0, 0);
	Epplet_timer(cb_timer, NULL, 0.1, "TIMER");   
     }
   data = NULL;
}

static void
cb_close(void *data)
{
   Epplet_unremember();
   Esync();
   Epplet_cleanup();
   data = NULL;
   exit(0);
}

static void
cb_color(void *data)
{
   int *d;
   
   d = (int *)data;
   flame_col(d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7], d[8]);
   save_conf(d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7], d[8]);
   Epplet_gadget_hide(pop);
}

static void
cb_config(void *data)
{
   data = NULL;
   Epplet_gadget_show(pop);
}

static void
cb_help(void *data)
{
   data = NULL;
   Epplet_show_about("E-Cpu");
}

static void
cb_in(void *data, Window w)
{
   data = NULL;
   Epplet_gadget_show(b_close);
   Epplet_gadget_show(b_config);
   Epplet_gadget_show(b_help);
}

static void
cb_out(void *data, Window w)
{
   data = NULL;
   Epplet_gadget_hide(b_close);
   Epplet_gadget_hide(b_config);
   Epplet_gadget_hide(b_help);
}

static int
count_cpus(void)
{
   FILE *f;
   char s[256];
   
   f = fopen("/proc/stat", "r");
   if (f)
     {
	int count = 0;
	char ok = 1;
	
	while (ok)
	  {
	     if (!fgets(s, 255, f))
		ok = 0;
	     else
	       {
		  if (strncmp(s, "cpu", 3))
		     ok = 0;
		  else		  
		     count++;
	       }
	  }
	if (count > 1)
	   count--;
	fclose (f);
	return count;
     }
   exit(1);
}

static void
save_conf(int d1, int d2, int d3, int d4, int d5, int d6, int d7, int d8, int d9)
{
   char s[1024];

   sprintf(s, "%d %d %d", d1, d2, d3);
   Epplet_modify_config_data("color1", s);
   sprintf(s, "%d %d %d", d4, d5, d6);
   Epplet_modify_config_data("color2", s);
   sprintf(s, "%d %d %d", d7, d8, d9);
   Epplet_modify_config_data("color3", s);
   Epplet_save_config();
}

static void
load_conf(void)
{
   int d1, d2, d3, d4, d5, d6, d7, d8, d9;
   char *str;

   str = Epplet_query_config_data("color1");
   sscanf(str, "%d %d %d", &d1, &d2, &d3);
   str = Epplet_query_config_data("color2");
   sscanf(str, "%d %d %d", &d4, &d5, &d6);
   str = Epplet_query_config_data("color3");
   sscanf(str, "%d %d %d", &d7, &d8, &d9);
   flame_col(d1, d2,  d3,
             d4, d5, d6,
             d7, d8, d9);
}

int
main(int argc, char **argv)
{
   Epplet_gadget p;
   
   cpus = count_cpus();
   load_val = malloc(sizeof(int) * cpus);
   prev_val = malloc(sizeof(double) * cpus);
   
   Epplet_Init("E-Cpu", "0.1", "Enlightenment CPU Epplet",
	       3, 3, argc, argv, 0);
   Epplet_load_config(config_defaults, num_defaults);
   Epplet_timer(cb_timer, NULL, 0.1, "TIMER");
   Epplet_gadget_show(da = Epplet_create_drawingarea(2, 2, 44, 44));
   win = Epplet_get_drawingarea_window(da);
   buf = Epplet_make_rgb_buf(40, 40);
   b_close = Epplet_create_button(NULL, NULL,
				0, 0, 0, 0, "CLOSE", win, NULL,
				cb_close, NULL);
   b_config = Epplet_create_button(NULL, NULL,
				28, 0, 0, 0, "CONFIGURE", win, NULL,
				cb_config, NULL);
   b_help = Epplet_create_button(NULL, NULL,
				 14, 0, 0, 0, "HELP", win, NULL,
				 cb_help, NULL);
   p = Epplet_create_popup();
   Epplet_add_popup_entry(p, "Turquoise", NULL, cb_color, (void *)(&(colors[0 * 9])));
   Epplet_add_popup_entry(p, "Fire", NULL, cb_color,      (void *)(&(colors[1 * 9])));
   Epplet_add_popup_entry(p, "Copper", NULL, cb_color,    (void *)(&(colors[2 * 9])));
   Epplet_add_popup_entry(p, "Violet", NULL, cb_color,    (void *)(&(colors[3 * 9])));
   Epplet_add_popup_entry(p, "Night", NULL, cb_color,     (void *)(&(colors[4 * 9])));
   Epplet_add_popup_entry(p, "Sunrise", NULL, cb_color,   (void *)(&(colors[5 * 9])));
   Epplet_add_popup_entry(p, "Sunset", NULL, cb_color,    (void *)(&(colors[6 * 9])));
   pop = Epplet_create_popupbutton("Colors", NULL, 6, 24, 36, 12, p);
   Epplet_register_focus_in_handler(cb_in, NULL);
   Epplet_register_focus_out_handler(cb_out, NULL);
   load_conf();
   Epplet_show();
   Epplet_Loop();
   return 0;
}
