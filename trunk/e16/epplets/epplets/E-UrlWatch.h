#if 0
#define D(a) { \
    printf("%s +%u : ",__FILE__,__LINE__); \
            printf a; \
            fflush(stdout); \
}
#else
#define D(a) { \
    /* No debug */; \
}
#endif


Epplet_gadget btn_conf, btn_close, btn_help, btn_ctimer, btn_www, btn_ftp, btn_wget;
Epplet_gadget btn_col, lbl_url;
Epplet_gadget p, col_p, ctimer_p, stimer_p;
Window win;
RGB_buf buf;
Epplet_gadget da;
int cloaked = 0;
extern int load_val;
extern int colors[];
static void cb_in (void *data, Window w);

int cloak_anims[] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17
};
int cloak_delays[] = {
  1, 2, 3, 4, 5, 10, 15, 20, 30, 60, 120
};
int rand_delays[] = {
  0, 30, 60, 90, 120, 180, 240, 300, 600, 900, 1200
};

struct
{
    char *str;
    unsigned int len;
    unsigned int pos;
}dtext;

struct
{
  int quality;
  int win;
  int beep;
  int cloak_anim;
  int frame;
  int do_cloak;
  int rand_cloak;
  int run_script;
  int save_urls;
  double delay;
  double cloak_delay;
  double rand_delay;
  double draw_interval;
  char *dir;
  char *file_prefix;
  char *file_stamp;
  char *file_type;
  char *script;
  char *url_save_file;
  char *www_command;
  char *ftp_command;
  char *get_command;
}
opt;
