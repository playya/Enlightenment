#include "epplet.h"

int                prev_bat_val = 110;
int                bat_val = 0;
int                time_val = 0;
int                prev_up[16] = 
{0, 0, 0, 0, 0, 0, 0, 0, 
   0, 0, 0, 0, 0, 0, 0, 0};
int                prev_count = 0;
Epplet_gadget      b_close, b_help, image, label;

static void cb_timer(void *data);
static void cb_close(void *data);
static void cb_in(void *data, Window w);
static void cb_out(void *data, Window w);
static void cb_help(void *data);

static void
cb_timer(void *data)
{
   static FILE *f;

   f = fopen("/proc/apm", "r");
   if (f)
     {
	char s[256], s1[32], s2[32], s3[32];
	int apm_flags, ac_stat, bat_stat, bat_flags;
	int  i, hours, minutes, up, up2;
	char *s_ptr;
	
	fgets(s, 255, f);
	sscanf(s, "%*s %*s %x %x %x %x %s %s %s", &apm_flags, &ac_stat, &bat_stat, &bat_flags, s1, s2, s3);
	s1[strlen(s1) - 1] = 0;
	bat_val=atoi(s1);
	if (!strcmp(s3, "sec"))
	   time_val = atoi(s2);
	else if (!strcmp(s3, "min"))
	   time_val = atoi(s2) * 60;
	fclose(f);
	
	up = bat_val - prev_bat_val;
	up2 = up;
	for (i = 0; i < 16; i++)
	   up2 = + prev_up[i];
	up2 = (up2 * 60) / 17;
	
	prev_up[prev_count] = up;
	
	prev_count++;
	if (prev_count >= 16)
	   prev_count = 0;
	
        s_ptr = s;

        if( bat_flags != 0xff
	    && bat_flags & 0x80 )
	  {
	    s_ptr += Esnprintf( s_ptr, sizeof(s_ptr), "no battery" );
	  }
	else
	  {
	    if( bat_val > 0 )
	      s_ptr += Esnprintf( s_ptr, sizeof(s_ptr), "%i%%", bat_val );

	    switch( bat_stat )
	      {
		case 0:
	          s_ptr += Esnprintf( s_ptr, sizeof(s_ptr), ", high" );
		  break;
		case 1:
	          s_ptr += Esnprintf( s_ptr, sizeof(s_ptr), ", low" );
		  break;
		case 2:
	          s_ptr += Esnprintf( s_ptr, sizeof(s_ptr), ", crit." );
		  break;
		case 3:
	          s_ptr += Esnprintf( s_ptr, sizeof(s_ptr), ", charge" );
		  break;
	      }
	  }
	s_ptr += Esnprintf( s_ptr, sizeof(s_ptr), "\n" );


	if( ac_stat == 1 )
          {
	    s_ptr += Esnprintf(s_ptr, sizeof(s_ptr), "AC on-line" );
          }
        else
          {
	    hours = time_val / 3600;
            minutes = (time_val / 60) % 60;
	    if (up2 > 0)
	       s_ptr += Esnprintf(s_ptr, sizeof(s_ptr), "(%i:%02i)\n%i:%02i", 
		       		(((100 - bat_val) * 2 * 60) / up2) / 60, 
		       		(((100 - bat_val) * 2 * 60) / up2) % 60, 
		       		hours, minutes);
	    else
	       s_ptr += Esnprintf(s_ptr, sizeof(s_ptr), "%i:%02i", hours, minutes);
          }
	Epplet_change_label(label, s);

	Esnprintf(s, sizeof(s), EROOT"/epplet_icons/E-Power-Bat-%i.png", 
		((bat_val + 5) / 10) * 10);
	Epplet_change_image(image, 44, 24, s);
	Epplet_timer(cb_timer, NULL, 30.0, "TIMER");   

	prev_bat_val = bat_val;
     }
   data = NULL;
}

static void
cb_close(void *data)
{
   Epplet_unremember();
   Esync();
   data = NULL;
   exit(0);
}

static void
cb_in(void *data, Window w)
{
   Epplet_gadget_show(b_close);
   Epplet_gadget_show(b_help);
   return;
   data = NULL;
   w = (Window) 0;
}

static void
cb_out(void *data, Window w)
{
   Epplet_gadget_hide(b_close);
   Epplet_gadget_hide(b_help);
   return;
   data = NULL;
   w = (Window) 0;
}

static void
cb_help(void *data)
{
   Epplet_show_about("E-Power");
   return;
   data = NULL;
}

int
main(int argc, char **argv)
{
   atexit(Epplet_cleanup);
   Epplet_Init("E-Power", "0.1", "Enlightenment Laptop Power Epplet",
	       3, 3, argc, argv, 0);
   Epplet_timer(cb_timer, NULL, 30.0, "TIMER");
   b_close = Epplet_create_button(NULL, NULL,
				  2, 2, 0, 0, "CLOSE", 0, NULL,
				  cb_close, NULL);
   b_help = Epplet_create_button(NULL, NULL,
				 34, 2, 0, 0, "HELP", 0, NULL,
				 cb_help, NULL);
   Epplet_gadget_show(image = 
		      Epplet_create_image
		      (2, 2, 44, 24,
		       EROOT"/epplet_icons/E-Power-Bat-100.png"));
   Epplet_gadget_show(label = 
		      Epplet_create_label
		      (2, 28, "APM not\nin Kernel", 1));
   Epplet_register_focus_in_handler(cb_in, NULL);
   Epplet_register_focus_out_handler(cb_out, NULL);
   cb_timer(NULL);
   Epplet_show();
   Epplet_Loop();
   return 0;
}
