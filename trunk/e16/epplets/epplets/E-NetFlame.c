#include "epplet.h"

double *prev_val = NULL;
int *load_val = NULL;
Window win;
RGB_buf buf;
Epplet_gadget da, b_close, b_config, b_help, pop, text;
int *flame = NULL;
int *vspread, *hspread, *residual;
unsigned char rm[255], gm[255], bm[255];
double stream_max = 2000;

const int num_defaults = 3;
ConfigItem config_defaults[] =
{
  {"color1", "30 90 90"},
  {"color2", "50 255 255"},
  {"color3", "255 255 255"}};

static int colors[] =
{
  30, 90, 90,
  50, 255, 255,
  255, 255, 255,

  255, 0, 0,
  255, 255, 0,
  255, 255, 255,

  0, 255, 0,
  255, 255, 0,
  255, 255, 255,

  0, 0, 255,
  255, 0, 255,
  255, 255, 255,

  0, 0, 200,
  40, 80, 255,
  100, 200, 255,

  80, 90, 140,
  140, 150, 180,
  255, 230, 200,

  20, 40, 180,
  255, 160, 0,
  255, 255, 100

};

static void save_conf(int d1, int d2, int d3, int d4, int d5,
		      int d6, int d7, int d8, int d9);
static void load_conf(void);
static void epplet_in(void *data, Window w);
static void epplet_out(void *data, Window w);
static void epplet_timer(void *data);
static void epplet_close(void *data);
static void epplet_config(void *data);
static void epplet_help(void *data);
static void draw_flame(void);
static void flame_col(int r1, int g1, int b1,
		      int r2, int g2, int b2,
		      int r3, int g3, int b3);

#define VARIANCE 40
#define VARTREND 16

#define HSPREAD  10
#define VSPREAD  160
#define RESIDUAL 75

#define DIVISIONS 2
#define WIDTH  40
#define HEIGHT 30

#define MAX      255

#define DOWN 0
#define UP   1

/* set the flame color */
static void 
flame_col(int r1, int g1, int b1,
	  int r2, int g2, int b2,
	  int r3, int g3, int b3)
{
  int i;

  for (i = 0; i < 25; i++) {
    rm[i] = (r1 * i) / 25;
    gm[i] = (g1 * i) / 25;
    bm[i] = (b1 * i) / 25;
  }
  for (i = 0; i < 25; i++) {
    rm[25 + i] = ((r2 * i) + (r1 * (25 - i))) / 25;
    gm[25 + i] = ((g2 * i) + (g1 * (25 - i))) / 25;
    bm[25 + i] = ((b2 * i) + (b1 * (25 - i))) / 25;
  }
  for (i = 0; i < 25; i++) {
    rm[50 + i] = ((r3 * i) + (r2 * (25 - i))) / 25;
    gm[50 + i] = ((g3 * i) + (g2 * (25 - i))) / 25;
    bm[50 + i] = ((b3 * i) + (b2 * (25 - i))) / 25;
  }

  for (i = 75; i < 255; i++) {
    rm[i] = rm[74];
    gm[i] = gm[74];
    bm[i] = bm[74];
  }
}

/* draw the flame to display */
static void 
draw_flame(void)
{
  unsigned char *rgb, *rptr;
  int x, y, *ptr, val1, val2, val3, i, j;

  /* initialize the flame if it isn't done already */
  if (!flame) {
    vspread = malloc(40 * sizeof(int));
    hspread = malloc(40 * sizeof(int));
    residual = malloc(40 * sizeof(int));
    flame = malloc(sizeof(int) * WIDTH * HEIGHT);
    memset(flame, 0, sizeof(int) * WIDTH * HEIGHT);
  }
  /* move to the bottom left of the drawing area */
  ptr = flame + ((HEIGHT - 1) * WIDTH);
  /* scan along the bottom row of pixels to start flames */
  for (x = 0; x < WIDTH; x++) {
    /* adjust spreads and residual values to reflect the
     * load values... */
    vspread[x] = VSPREAD + (load_val[(x * DIVISIONS) / HEIGHT] / 50);
    hspread[x] = HSPREAD + (load_val[(x * DIVISIONS) / WIDTH] / 50);
    residual[x] = RESIDUAL + (load_val[(x * DIVISIONS) / HEIGHT] / 50);
    /* assign a random value to the pixel according to the
     * load ... gives randomness to flames */
    ptr[x] = (rand() % ((load_val[(x * DIVISIONS) / 40]) + 155));
    /* bounds checking */
    if (ptr[x] > MAX) {
      ptr[x] = 0;
    } else if (ptr[x] < 0) {
      ptr[x] = 0;
    }
  }

  /* this divides up the chart into multiple sections
   * depending on the cpus value */
  for (i = 0; i < DIVISIONS; i++) {
    for (x = (WIDTH / (DIVISIONS * 2)) - 1 + (i * (WIDTH / DIVISIONS));
	 x <= (WIDTH / (DIVISIONS * 2)) + 1 + (i * (WIDTH / DIVISIONS));
	 x++) {
      j = (load_val[i] * HEIGHT) / 100;
      ptr = flame + ((HEIGHT - j) * (WIDTH)) + (x);
      /* marches down a column increasing the intensity
       * as you travel down */
      for (y = 0; y < j; y++) {
	ptr[0] += ((y * 64 * load_val[i]) / (j * 100));
	ptr += WIDTH;
      }
    }
  }
  /* ----- *
   * --A-- *
   * -BCD- *
   * ----- */
  for (y = HEIGHT - 1; y >= 2; y--) {
    ptr = flame + (y * 40);
    for (x = 1; x < WIDTH - 1; x++) {
      /* val1 = (C * vspread) / 256 */
      val1 = (ptr[x] * vspread[x]) >> 8;
      /* val2 = (C * hspread) / 256 */
      val2 = (ptr[x] * hspread[x]) >> 8;
      /* val3 = (C * residual) / 256 */
      val3 = (ptr[x] * residual[x]) >> 8;

      /* add val2 to B */
      ptr[x - 1] += val2;
      if (ptr[x - 1] > MAX)
	ptr[x - 1] = MAX;
      /* add val1 to A */
      ptr[x - WIDTH] += val1;
      if (ptr[x - WIDTH] > MAX)
	ptr[x - WIDTH] = MAX;
      /* add val2 to D */
      ptr[x + 1] += val2;
      if (ptr[x + 1] > MAX)
	ptr[x + 1] = MAX;
      /* add val3 to C */
      ptr[x] = val3;
    }
  }

  /* blank the top row of pixels */
  for (x = 0; x < WIDTH; x++)
    flame[x] = 0;
  /* half the intensity of the second row */
  for (x = 0; x < WIDTH; x++)
    flame[WIDTH + x] /= 2;
  /* blank out left most column */
  for (y = 0; y < HEIGHT; y++)
    flame[y * WIDTH] = 0;
  /* half the intensity on second left most column */
  for (y = 0; y < HEIGHT; y++)
    flame[(y * WIDTH) + 1] /= 2;
  /* blank out right most column */
  for (y = 0; y < HEIGHT; y++)
    flame[(y * WIDTH) + (WIDTH - 1)] = 0;
  /* half the intensity of the second right most column */
  for (y = 0; y < HEIGHT; y++)
    flame[(y * WIDTH) + (WIDTH - 2)] /= 2;


  rgb = Epplet_get_rgb_pointer(buf);
  for (y = 0; y < HEIGHT; y++) {
    ptr = flame + (y * WIDTH) + 1;
    rptr = rgb + (y * WIDTH * 3);
    for (x = 0; x < WIDTH; x++) {
      val1 = ptr[x] & 0xff;
      rptr[0] = rm[val1];
      rptr[1] = gm[val1];
      rptr[2] = bm[val1];
      rptr += 3;
    }
  }
}

/* handles the timer, check /proc/net/dev to establish
 * up and down load */
static void 
epplet_timer(void *data)
{
  static FILE *f;
  int ok = 1;

  f = fopen("/proc/net/dev", "r");
  if (f) {
    char s[256], ss[32];
    double val, val2, dval, dval2;

    /* eat the top two lines */
    fgets(s, 255, f);
    fgets(s, 255, f);

    while (ok) {
      if (!fgets(s, 255, f)) {
	printf("poping out on read error\n");
	ok = 0;
      } else {
	char *sp, s1[64], s2[64];

	sp = strchr(s, ':');
	if (sp)
	  *sp = ' ';
	val = 0;
	val2 = 0;
	sscanf(s, "%s %s %*s %*s %*s %*s %*s %*s %*s %s", ss, s1, s2);
	val = atof(s1);
	val2 = atof(s2);
	if (!strcmp(ss, "eth0")) {
	  /*printf("%f val2 | %f val\n", val2, val); */
	  dval = val - prev_val[DOWN];
	  dval2 = val2 - prev_val[UP];
	  prev_val[DOWN] = val;
	  prev_val[UP] = val2;
	  load_val[DOWN] = (int) ((dval * 800 * 3) / stream_max);
	  load_val[UP] = (int) ((dval2 * 800 * 3) / stream_max);
	  if (load_val[DOWN] > 100)
	    load_val[DOWN] = 100;
	  if (load_val[UP] > 100)
	    load_val[UP] = 100;
	  ok = 0;
	}
      }
    }
    fclose(f);
    draw_flame();
    Epplet_paste_buf(buf, win, 0, 0);
    Epplet_timer(epplet_timer, NULL, 0.1, "TIMER");
  }
  data = NULL;
}

/* called when you close the epplet */
static void 
epplet_close(void *data)
{
  Epplet_unremember();
  Esync();
  Epplet_cleanup();
  data = NULL;
  exit(0);
}

/* called when you select a color from the epplet color menu */
static void 
epplet_color(void *data)
{
  int *d;

  d = (int *) data;
  flame_col(d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7], d[8]);
  save_conf(d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7], d[8]);
  Epplet_gadget_hide(pop);
}

/* called when the config button is pressed */
static void 
epplet_config(void *data)
{
  data = NULL;
  Epplet_gadget_show(pop);
}

/* called when the help button is pressed */
static void 
epplet_help(void *data)
{
  data = NULL;
  Epplet_show_about("E-NetFlame");
}

/* called when the epplet gets the focus */
static void 
epplet_in(void *data, Window w)
{
  Epplet_gadget_show(b_close);
  Epplet_gadget_show(b_config);
  Epplet_gadget_show(b_help);
  return;
  data = NULL;
  w = (Window) 0;
}

/* called when the epplet loses the focus */
static void 
epplet_out(void *data, Window w)
{
  Epplet_gadget_hide(b_close);
  Epplet_gadget_hide(b_config);
  Epplet_gadget_hide(b_help);
  return;
  data = NULL;
  w = (Window) 0;
}

/* save off the color of the flame in a config file */
static void 
save_conf(int d1, int d2, int d3, int d4, int d5,
	  int d6, int d7, int d8, int d9)
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

/* load the color of the flame from a config file */
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
  flame_col(d1, d2, d3,
	    d4, d5, d6,
	    d7, d8, d9);
}

/* where it all begins... */
int 
main(int argc, char **argv)
{
  Epplet_gadget p;

  load_val = malloc(sizeof(int) * DIVISIONS);
  prev_val = malloc(sizeof(double) * DIVISIONS);

  Epplet_Init("E-NetFlame", "0.2", "E Net-Flame Epplet",
	      3, 3, argc, argv, 0);
  Epplet_load_config(config_defaults, num_defaults);
  Epplet_timer(epplet_timer, NULL, 0.1, "TIMER");
  Epplet_gadget_show(da = Epplet_create_drawingarea(2, 2,
						    (WIDTH + 4), (HEIGHT + 4)));
  win = Epplet_get_drawingarea_window(da);
  buf = Epplet_make_rgb_buf(WIDTH, HEIGHT);
  Epplet_gadget_show(text = Epplet_create_label(8, 36, "IN   OUT", 1));
  b_close = Epplet_create_button(NULL, NULL,
				 3, 3, 0, 0, "CLOSE", 0, NULL,
				 epplet_close, NULL);
  b_help = Epplet_create_button(NULL, NULL,
				17, 3, 0, 0, "HELP", 0, NULL,
				epplet_help, NULL);
  b_config = Epplet_create_button(NULL, NULL,
				  31, 3, 0, 0, "CONFIGURE", 0, NULL,
				  epplet_config, NULL);
  p = Epplet_create_popup();
  Epplet_add_popup_entry(p, "Turquoise", NULL, epplet_color,
			 (void *) (&(colors[0 * 9])));
  Epplet_add_popup_entry(p, "Fire     ", NULL, epplet_color,
			 (void *) (&(colors[1 * 9])));
  Epplet_add_popup_entry(p, "Copper", NULL, epplet_color,
			 (void *) (&(colors[2 * 9])));
  Epplet_add_popup_entry(p, "Violet", NULL, epplet_color,
			 (void *) (&(colors[3 * 9])));
  Epplet_add_popup_entry(p, "Night", NULL, epplet_color,
			 (void *) (&(colors[4 * 9])));
  Epplet_add_popup_entry(p, "Sunrise", NULL, epplet_color,
			 (void *) (&(colors[5 * 9])));
  Epplet_add_popup_entry(p, "Sunset", NULL, epplet_color,
			 (void *) (&(colors[6 * 9])));
  pop = Epplet_create_popupbutton("Colors", NULL, 5, 24, 36, 12, p);
  Epplet_register_focus_in_handler(epplet_in, NULL);
  Epplet_register_focus_out_handler(epplet_out, NULL);
  load_conf();
  Epplet_show();
  Epplet_Loop();
  return 0;
}
