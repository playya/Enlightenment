#include "evas_test_main.h"

// test writing to image objects for video playback
//#define VID_TEST
// actualyl fill the video buffer (not fair a test as cpu spends time filling)
//#define VID_WRITE

#define EVAS_PI	(3.141592654)

#ifndef _WIN32_WCE

# define IM "data/"
# define FN "data/"

#else

# define IM "data\\"
# define FN "data\\"
wchar_t buf[1024];

#endif


Evas               *evas = NULL;

#ifdef VID_TEST
int                 win_w = 720;
int                 win_h = 480;
#else
int                 win_w = 240;
int                 win_h = 320;
#endif

int                 mode = 0;
int                 loop_count = 0;
int                 orig_loop_count = 0;

double              start_time = 0.0;
double              orig_start_time = 0.0;

Evas_Object        *backdrop, *panel, *panel_top, *panel_shadow, *panel_clip;
Evas_Object        *evas_logo, *e_logo;

Evas_Object        *title = NULL, *comment;

Evas_Object        *t1 = NULL, *t2 = NULL;
Evas_Object        *test_pattern = NULL;
Evas_Object        *c1 = NULL, *c2 = NULL;
Evas_Object        *cv1 = NULL, *cv2 = NULL;

Evas_Object        *scroll[16];

#ifndef _WIN32_WCE
double
get_time(void)
{

   struct timeval      timev;

   gettimeofday(&timev, NULL);
   return (double)timev.tv_sec + (((double)timev.tv_usec) / 1000000);

}

#else
double
get_time(void)
{

	return (double)GetTickCount()/1000.0;
}
#endif

static int did_setup = 0;
static double time_start = 0.0;
static int frames = 0;

static Evas_Object *p_s[16];
static Evas_Object *p_i[2];
static Evas_Object *p_r[8];
static Evas_Object *p_t[16];

static unsigned int grad_map[128] = { 
0xfe597170, 0xfe566e6e, 0xfe556e6e, 0xfe516b6b, 0xfe4e6767, 0xfe4d6565, 0xfe4c6464, 0xfe4c6564, 
0xfe4c6564, 0xfe4c6564, 0xfe4d6566, 0xfe4c6465, 0xfe4a6262, 0xfe496060, 0xfe475f5f, 0xfe475e5f,
0xfe445c5c, 0xfe435b5c, 0xfe435b5c, 0xfe445c5e, 0xfe486062, 0xfe4f6868, 0xfe536b6d, 0xfe536b6d, 
0xfe516b6d, 0xfe4a6266, 0xfe445c60, 0xfe41595d, 0xfe3e565a, 0xfe3b5357, 0xfe395155, 0xfe375053, 
0xfe354e51, 0xfe334d50, 0xfe324c4f, 0xfe314b4e, 0xfe304a4e, 0xfe324b4f, 0xfe334c50, 0xfe334d50, 
0xfe344d50, 0xfe364f53, 0xfe384f53, 0xfe3b5359, 0xfe3b5359, 0xfe395257, 0xfe385257, 0xfe385256, 
0xfe385157, 0xfe385257, 0xfe395359, 0xfe3b5459, 0xfe3b5459, 0xfe395358, 0xfe395358, 0xfe3a5359, 
0xfe3c555b, 0xfe3c565b, 0xfe3b555a, 0xfe3c555a, 0xfe3b555a, 0xfe3a5359, 0xfe385257, 0xfe385257, 
0xfe395357, 0xfe3b5459, 0xfe3f595d, 0xfe415a5f, 0xfe415a5d, 0xfe415b5f, 0xfe445c5f, 0xfe475f62, 
0xfe4a6265, 0xfe4d6566, 0xfe4e6868, 0xfe536b6b, 0xfe586f6d, 0xfe5e7471, 0xfe657a77, 0xfe667c77, 
0xfe788a82, 0xfe8e9f94, 0xfe8e9f95, 0xfeabbaac, 0xfeb1c0b2, 0xfeafbeb0, 0xfea5b4a6, 0xfe8c9d91, 
0xfe899a91, 0xfe87988e, 0xfe91a197, 0xfe99a99e, 0xfe8b9d93, 0xfe7c9088, 0xfe768a83, 0xfe7a8d85, 
0xfe7c8e85, 0xfe71867f, 0xfe677d78, 0xfe667b76, 0xfe627873, 0xfe5c726e, 0xfe566e6b, 0xfe566e6b, 
0xfe59706c, 0xfe566e6b, 0xfe617772, 0xfe6b8079, 0xfe6d827c, 0xfe6a7f7b, 0xfe657c77, 0xfe5d7571, 
0xfe57706d, 0xfe566e6b, 0xfe536b68, 0xfe516a67, 0xfe4d6865, 0xfe4b6562, 0xfe4a6262, 0xfe496262, 
0xfe4a6462, 0xfe4b6563, 0xfe4a6362, 0xfe4c6565, 0xfe506a68, 0xfe506a69, 0xfe546e6c, 0xfe5c7472
};

void
loop(void)
{
   double              t;

   loop_count++;
   orig_loop_count++;
   t = get_time() - start_time;

   if (mode == 1)
     {
	int iw, ih;
	int i;

	evas_object_image_fill_set(scroll[0], 0, loop_count, 240, 320);
	for (i = 1; i < 16; i++)
	  {
	     evas_object_image_size_get(scroll[i], &iw, &ih);
	     evas_object_image_fill_set(scroll[i], 0, loop_count * (i + 1), iw, ih);
	  }
	if ((loop_count % 500) == 499)
	  {
	     printf("FPS: %3.3f\n", (double)loop_count/t);
	  }
	return;
     }

#ifdef VID_TEST
   if (t <= 5.0)
     {
	if (!test_pattern)
	  {
	     evas_object_hide(backdrop);
	     evas_object_hide(panel);
	     evas_object_hide(panel_top);
	     evas_object_hide(panel_shadow);
	     evas_object_hide(panel_clip);
	     evas_object_hide(evas_logo);
	     evas_object_hide(e_logo);
	     test_pattern = evas_object_image_add(evas);
	     evas_object_move(test_pattern, 0, 0);
	     evas_object_resize(test_pattern, 720, 480);
	     evas_object_layer_set(test_pattern, 99999);
	     evas_object_image_size_set(test_pattern, 720, 480);
	     evas_object_image_fill_set(test_pattern, 0, 0, 720, 480);
	     evas_object_show(test_pattern);
             frames = 0;
	     time_start = t;
	  }
	  {
	     int                 iw, ih;
	     int                 x, y;
	     int                *data;

	     evas_object_image_size_get(test_pattern, &iw, &ih);
	     evas_object_image_alpha_set(test_pattern, 0);
	     data = evas_object_image_data_get(test_pattern, 1);
	     if (data)
	       {
		  int tt;

#ifdef VID_WRITE
		  tt = t * 1000;
		  for (y = 0; y < ih; y++)
		    {
		       for (x = 0; x < iw; x++)
			 data[(y * iw) + x] =
			 (((x * y / 10) + tt)) | 0xff000000;
		    }
#endif
		  evas_object_image_data_update_add(test_pattern, 0, 0, iw, ih);
		  evas_object_image_data_set(test_pattern, data);
	       }
	  }
	frames++;
     }
   else if (t > 5.0)
     {
//	printf("# FRAME COUNT: %i frames\n", frames);
//	printf("# TIME:        %3.3f seconds\n", (t - time_start));
//	printf("# AVERAGE FPS: %3.3f fps\n", (double)frames / (t - time_start));
	printf("# EVAS BENCH:  %3.3f\n", ((double)frames / (t - time_start)) / 60.0);
	exit(0);
     }
   else
#endif
   if (t <= 2.0)
     {
	Evas_Coord              y;

	y = sin((t / 2) * (EVAS_PI / 2)) * 240;
	evas_object_move(panel, 0, win_h - y);
	evas_object_move(panel_clip, 0, win_h - y);
	evas_object_move(panel_top, 0, win_h - y - 10);
	evas_object_move(panel_shadow, 0, win_h - y);
	evas_object_resize(backdrop, win_w, win_h - y);
     }
   else if (t <= (6.0))
     {
	int a = (int)(255 * (((6.0) - t) / 4.0));

	evas_object_move(panel, 0, win_h - 240);
	evas_object_move(panel_clip, 0, win_h - 240);
	evas_object_move(panel_top, 0, win_h - 240 - 10);
	evas_object_move(panel_shadow, 0, win_h - 240);
	evas_object_color_set(evas_logo, a, a, a, a);
     }
   else if (t <= 26.0)
     {
	int i;

	if (!did_setup)
	  {
	     frames = 0;
	     time_start = t;
	     for (i = 0; i < 16; i++)
	       {
		  Evas_Object        *ob;

		  ob = evas_object_image_add(evas);
		  if (i & 0x1)
		    {
		       evas_object_image_file_set(ob, IM "t1.png", NULL);
		       evas_object_image_smooth_scale_set(ob, 1);
		    }
		  else
		    {
		       evas_object_image_file_set(ob, IM "t2.png", NULL);
		       evas_object_image_smooth_scale_set(ob, 0);
		    }
		  evas_object_resize(ob, 48, 48);
		  evas_object_image_fill_set(ob, 0, 0, 48, 48);
		  evas_object_layer_set(ob, 5);
		  evas_object_clip_set(ob, panel_clip);
		  evas_object_show(ob);
		  p_s[i] = ob;
	       }
	     for (i = 0; i < 2; i++)
	       {
		  Evas_Object        *ob;

		  ob = evas_object_image_add(evas);
		  evas_object_image_file_set(ob, IM "test_pattern.png", NULL);
		  evas_object_layer_set(ob, 5);
		  evas_object_clip_set(ob, panel_clip);
		  evas_object_show(ob);
		  p_i[i] = ob;
	       }
	     evas_object_image_smooth_scale_set(p_i[1], 0);
	     for (i = 0; i < 16; i++)
	       {
		  Evas_Object        *ob;

		  ob = evas_object_text_add(evas);
		  evas_object_text_font_set(ob, "Vera", (i * 2) + 4);
/*		  evas_object_text_font_set(ob, "-*-Timmons-bold-r-*-*-*-*-*-*-*-*-*-*", (i * 2) + 4); */
		  evas_object_text_text_set(ob, "A Test String");
		  evas_object_layer_set(ob, 5);
		  evas_object_clip_set(ob, panel_clip);
		  evas_object_color_set(ob, 0, 0, 0, 255);
		  evas_object_show(ob);
		  p_t[i] = ob;
	       }
	     for (i = 0; i < 8; i++)
	       {
		  Evas_Object        *ob;
		  int                a = i * 32;

		  ob = evas_object_rectangle_add(evas);
		  evas_object_color_set(ob, (a * (100 + (i * 16))) / 255 , (a * (255 - (i * 16))) / 255, (a * i * 64) / 255, a);
		  evas_object_layer_set(ob, 5);
		  evas_object_clip_set(ob, panel_clip);
		  evas_object_show(ob);
		  p_r[i] = ob;
	       }
	     did_setup = 1;
	  }
	for (i = 0; i < 16; i++)
	  {
	     Evas_Coord              x, y;
	     double              v;

	     v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	     x = cos(v * (double)i / 3) * ((240 - 48) / 2);
	     y = sin(v * (double)i / 3) * ((240 - 48) / 2);
	     evas_object_move(p_s[i], 120 + x - 24, win_h - 120 + y - 24);
	     if (i & 0x1)
	       {
		  v = (((t - 6.0) / 35.0) * EVAS_PI * 2) * 2;
		  x = (cos(v * (double)i / 3) * 48) + 50;
		  y = (sin(v * (double)i / 3) * 48) + 50;
		  evas_object_resize(p_s[i], x, y);
		  evas_object_image_fill_set(p_s[i], 0, 0, x, y);
	       }
	  }
	for (i = 0; i < 2; i++)
	  {
	     int iw, ih;
	     Evas_Coord              x, y;
	     double              v;

	     evas_object_image_size_get(p_i[i], &iw, &ih);
	     v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
#define MULTI 1.0
	     x = cos(v * MULTI * (double)(i + 1) * 3) * ((240 - iw) / 2);
	     y = sin(v * MULTI * (double)(i + 1) * 2) * ((240 - ih) / 2);
	     evas_object_move(p_i[i], 120 + x - 80, win_h - 120 + y - 80);
	     v = (((t - 6.0) / 35.0) * EVAS_PI * 2) * 2;
	     x = (cos(v * MULTI * (double)(i + 1)  * 7) * iw) + iw;
	     y = (sin(v * MULTI * (double)(i + 1)  * 6) * ih) + ih;
	     evas_object_resize(p_i[i], x, y);
	     evas_object_image_fill_set(p_i[i], 0, 0, x, y);
//	     evas_object_hide(p_i[1]);
	  }
	for (i = 0; i < 16; i++)
	  {
	     Evas_Coord              x, y;
	     double              v;

	     v = (((t - 6.0) / 24.0) * EVAS_PI * 2) * 2;
	     x = cos(v * (double)i / 3) * ((240 - 48) / 2);
	     y = sin(v * (double)i / 2) * ((240 - 48) / 2);
	     evas_object_move(p_t[i], 120 + x - 80, win_h - 120 + y - 24);
	  }
	for (i = 0; i < 8; i++)
	  {
	     Evas_Coord              x, y;
	     double              v;

	     v = (((t - 6.0) / 29.0) * EVAS_PI * 2) * 2;
	     x = cos(v * (double)i / 2) * ((240 - 48) / 2);
	     y = sin(v * (double)i / 3) * ((240 - 48) / 2);
	     evas_object_move(p_r[i], 120 + x - 24, win_h - 120 + y - 24);
	     if (i & 0x1)
	       {
		  v = (((t - 6.0) / 31.0) * EVAS_PI * 2) * 2;
		  x = (cos(v * (double)i * 2) * 48) + 50;
		  y = (sin(v * (double)i *5) * 48) + 50;
		  evas_object_resize(p_r[i], x, y);
	       }
	  }
	frames++;
     }
   else if (t <= 36.0)
     {
	Evas_Coord              tw, th;
	static int          did_fps = 0;

	if (!did_fps)
	  {
	     int i;

	     did_fps = 1;
	     printf("####################################################\n");
	     printf("# Performance Test. Your system scores...\n");
	     printf("####################################################\n");
//	     printf("# FRAME COUNT: %i frames\n", frames);
//	     printf("# TIME:        %3.3f seconds\n", (t - time_start));
//	     printf("# AVERAGE FPS: %3.3f fps\n", (double)frames / (t - time_start));
//	     printf("####################################################\n");
	     printf("# Your system Evas Benchmark:\n");
	     printf("#\n");
	     printf("# EVAS BENCH: %3.3f\n", ((double)frames / (t - time_start)) / 60.0);
	     printf("#\n");
	     printf("####################################################\n");
//	     exit(0);
	     for (i = 0; i < 16; i++) evas_object_del(p_s[i]);
	     for (i = 0; i < 2; i++) evas_object_del(p_i[i]);
	     for (i = 0; i < 16; i++) evas_object_del(p_t[i]);
	     for (i = 0; i < 8; i++) evas_object_del(p_r[i]);
	  }

	if (evas_logo)
	  {
	     Evas_Object        *ob;

	     evas_object_del(evas_logo);
	     evas_logo = NULL;

	     ob = evas_object_image_add(evas);
	     evas_object_image_file_set(ob, IM "t1.png", NULL);
	     evas_object_move(ob, (win_w - 48) / 2,
			      (win_h - 240) + ((240 - 48) / 2));
	     evas_object_resize(ob, 48, 48);
	     evas_object_image_fill_set(ob, 0, 0, 48, 48);
	     evas_object_layer_set(ob, 5);
	     evas_object_clip_set(ob, panel_clip);
	     evas_object_image_smooth_scale_set(ob, 1);
	     evas_object_show(ob);
	     t1 = ob;
	     ob = evas_object_image_add(evas);
	     evas_object_image_file_set(ob, IM "t2.png", NULL);
	     evas_object_move(ob, (win_w - 48) / 2,
			      (win_h - 240) + ((240 - 48) / 2));
	     evas_object_resize(ob, 48, 48);
	     evas_object_image_fill_set(ob, 0, 0, 48, 48);
	     evas_object_layer_set(ob, 5);
	     evas_object_clip_set(ob, panel_clip);
	     evas_object_image_smooth_scale_set(ob, 0);
	     evas_object_show(ob);
	     t2 = ob;
	     ob = evas_object_text_add(evas);
	     evas_object_text_font_set(ob, "Vera", 8);
	     evas_object_geometry_get(ob, NULL, NULL, &tw, &th);
	     evas_object_move(ob, (win_w - tw) / 2, win_h - th);
	     evas_object_color_set(ob, 0, 0, 0, 255);
	     evas_object_layer_set(ob, 9);
	     evas_object_clip_set(ob, panel_clip);
	     evas_object_show(ob);
	     title = ob;
	     ob = evas_object_text_add(evas);
	     evas_object_text_font_set(ob, "Vera", 8);
	     evas_object_geometry_get(ob, NULL, NULL, &tw, &th);
	     evas_object_move(ob, 0, win_h - 240);
	     evas_object_color_set(ob, 0, 0, 0, 255);
	     evas_object_layer_set(ob, 9);
	     evas_object_clip_set(ob, panel_clip);
	     evas_object_show(ob);
	     comment = ob;
	  }

	/* Japanese: "日本語]" */
	/* Korean: "한국" */
	/* Chinese: "汉语" */
	evas_object_text_text_set(comment, "Image objects can be moved");
	evas_object_text_text_set(title, "Test 1: Move 2 Images ");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	{
	   Evas_Coord              x, y;
	   double              v;

	   v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = cos(v * 1.7) * ((240 - 48) / 2);
	   y = sin(v * 1.9) * ((240 - 48) / 2);
	   evas_object_move(t1, 120 + x - 24, win_h - 120 + y - 24);
	   x = cos(1.0 + (v * 2.3)) * ((240 - 48) / 2);
	   y = sin(1.0 + (v * 1.45)) * ((240 - 48) / 2);
	   evas_object_move(t2, 120 + x - 24, win_h - 120 + y - 24);
	}
     }
   else if (t <= 46.0)
     {
	Evas_Coord              tw, th;

	evas_object_text_text_set(comment, "Image objects can be resized");
	evas_object_text_text_set(title, "Test 2: Resize 2 Images");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	{
	   Evas_Coord              x, y;
	   double              v;

	   v = (((t - 16.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = (cos(v * 1.6) + 1.0) * 64;
	   y = (sin(v * 1.2) + 1.0) * 64;
	   evas_object_resize(t1, x, y);
	   x = (cos(v * 1.2) + 1.0) * 64;
	   y = (sin(v * 2.3) + 1.0) * 64;
	   evas_object_resize(t2, x, y);
	}
     }

   else if (t <= 56.0)
     {
	Evas_Coord              tw, th;

	evas_object_text_text_set(comment,
				  "Image objects are filled and scaled differently");
	evas_object_text_text_set(title, "Test 3: Resize & Fill 2 Images");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	{
	   Evas_Coord              x, y;
	   double              v;

	   v = (((t - 26.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = (cos(v * 1.6) + 1.0) * 64;
	   y = (sin(v * 1.2) + 1.0) * 64;
	   evas_object_resize(t1, x, y);
	   evas_object_image_fill_set(t1, 0, 0, x, y);
	   x = (cos(v * 1.2) + 1.0) * 64;
	   y = (sin(v * 2.3) + 1.0) * 64;
	   evas_object_resize(t2, x, y);
	   evas_object_image_fill_set(t2, 0, 0, x, y);
	}
     }
   else if (t <= 66.0)
     {
	Evas_Coord              tw, th;

	evas_object_text_text_set(comment, "Image objects are offset filled");
	evas_object_text_text_set(title, "Test 4: Fill Offset 2 Images");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);

	evas_object_resize(t1, 100, 100);
	evas_object_image_fill_set(t1, -25, -25, 100, 100);
	{
	   Evas_Coord              x, y;
	   double              v;

	   v = (((t - 36.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = (cos(v * 1.6) + 1.0) * 64;
	   y = (sin(v * 1.2) + 1.0) * 64;
	   evas_object_resize(t1, x, y);
	   evas_object_image_fill_set(t1, -x / 4, -y / 4, x / 2, y / 2);
	   x = (cos(v * 1.2) + 1.0) * 64;
	   y = (sin(v * 2.3) + 1.0) * 64;
	   evas_object_resize(t2, x, y);
	   evas_object_image_fill_set(t2, -x / 4, -y / 4, x / 2, y / 2);
	}
     }

   else if (t <= 76.0)
     {
	Evas_Coord              tw, th;

	evas_object_text_text_set(comment,
				  "Edges of images can be scaled differently");
	evas_object_text_text_set(title, "Test 5: Border Fill 2 Images");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	evas_object_image_border_set(t1, 8, 8, 8, 8);
	evas_object_image_border_set(t2, 8, 8, 8, 8);
	{
	   Evas_Coord              x, y;
	   double              v;

	   v = (((t - 46.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = (cos(v * 1.6) + 1.0) * 64;
	   y = (sin(v * 1.2) + 1.0) * 64;
	   evas_object_resize(t1, x, y);
	   evas_object_image_fill_set(t1, 0, 0, x, y);
	   x = (cos(v * 1.2) + 1.0) * 64;
	   y = (sin(v * 2.3) + 1.0) * 64;
	   evas_object_resize(t2, x, y);
	   evas_object_image_fill_set(t2, 0, 0, x, y);
	}
     }
   else if (t <= 96.0)
     {
	Evas_Coord              tw, th;
	int                 iw, ih;

	evas_object_text_text_set(title, "Test 7: Image Quality");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	if (!test_pattern)
	  {
	     Evas_Object        *ob;

	     evas_object_hide(t1);
	     evas_object_hide(t2);
	     ob = evas_object_image_add(evas);
	     evas_object_image_file_set(ob, IM "test_pattern.png", NULL);
	     evas_object_image_size_get(ob, &iw, &ih);
	     evas_object_move(ob, (win_w - iw) / 2,
			      (win_h - 240) + ((240 - ih) / 2));
	     evas_object_resize(ob, iw, ih);
	     evas_object_image_fill_set(ob, 0, 0, iw, ih);
	     evas_object_layer_set(ob, 5);
	     evas_object_clip_set(ob, panel_clip);
	     evas_object_show(ob);
	     test_pattern = ob;
	  }
	{
	   double              v;
	   Evas_Coord              x, y;

	   evas_object_image_size_get(test_pattern, &iw, &ih);
	   v = 2.0 - ((t - 76.0) / 10.0);
	   if ((v >= 0.4) && (v <= 1.4))
	     {
		evas_object_text_text_set(comment,
					  "Look and see if it looks right");
		v = 1.4;
	     }
	   else if (v <= 0.4)
	     {
		evas_object_text_text_set(comment, "Scaling test pattern");
		v += 1.0;
	     }
	   else
	     {
		evas_object_text_text_set(comment, "Scaling test pattern");
	     }
	   v -= 1.0;
	   x = iw * v;
	   y = ih * v;
	   evas_object_move(test_pattern, (win_w - x) / 2,
			    (win_h - 240) + ((240 - y) / 2));
	   evas_object_resize(test_pattern, x, y);
	   evas_object_image_fill_set(test_pattern, 0, 0, x, y);
	}
     }
   else if (t <= 106.0)
     {
	Evas_Coord              tw, th;

	evas_object_text_text_set(title, "Test 8: Raise & Lower");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	if (test_pattern)
	  {
	     evas_object_del(test_pattern);
	     test_pattern = NULL;
	     evas_object_image_border_set(t1, 0, 0, 0, 0);
	     evas_object_image_border_set(t2, 0, 0, 0, 0);
	     evas_object_resize(t1, 48, 48);
	     evas_object_resize(t2, 48, 48);
	     evas_object_move(t1, 40, win_h - 240 + 40);
	     evas_object_move(t2, 60, win_h - 240 + 60);
	     evas_object_image_fill_set(t1, 0, 0, 48, 48);
	     evas_object_image_fill_set(t2, 0, 0, 48, 48);
	     evas_object_show(t1);
	     evas_object_show(t2);
	  }
	if (((int)(t)) & 0x1)
	  {
	     evas_object_text_text_set(comment, "2 On Top");
	     evas_object_raise(t2);
	  }
	else
	  {
	     evas_object_text_text_set(comment, "1 On Top");
	     evas_object_raise(t1);
	  }
     }
   else if (t <= 116.0)
     {
	Evas_Coord              tw, th;

	evas_object_text_text_set(title, "Test 9: Layer Setting");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	if (test_pattern)
	  {
	     evas_object_del(test_pattern);
	     test_pattern = NULL;
	     evas_object_image_border_set(t1, 0, 0, 0, 0);
	     evas_object_image_border_set(t2, 0, 0, 0, 0);
	     evas_object_resize(t1, 48, 48);
	     evas_object_resize(t2, 48, 48);
	     evas_object_move(t1, 40, win_h - 240 + 40);
	     evas_object_move(t2, 60, win_h - 240 + 60);
	     evas_object_image_fill_set(t1, 0, 0, 48, 48);
	     evas_object_image_fill_set(t2, 0, 0, 48, 48);
	     evas_object_show(t1);
	     evas_object_show(t2);
	  }
	if (((int)(t)) & 0x1)
	  {
	     evas_object_text_text_set(comment, "2 On Top");
	     evas_object_layer_set(t1, 5);
	     evas_object_layer_set(t2, 6);
	  }
	else
	  {
	     evas_object_text_text_set(comment, "1 On Top");
	     evas_object_layer_set(t1, 6);
	     evas_object_layer_set(t2, 5);
	  }
     }
   else if (t <= 126.0)
     {
	Evas_Coord              tw, th;

	evas_object_text_text_set(title, "Test 10: Show & Hide");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	if (test_pattern)
	  {
	     evas_object_del(test_pattern);
	     test_pattern = NULL;
	     evas_object_image_border_set(t1, 0, 0, 0, 0);
	     evas_object_image_border_set(t2, 0, 0, 0, 0);
	     evas_object_resize(t1, 48, 48);
	     evas_object_resize(t2, 48, 48);
	     evas_object_move(t1, 40, win_h - 240 + 40);
	     evas_object_move(t2, 60, win_h - 240 + 60);
	     evas_object_image_fill_set(t1, 0, 0, 48, 48);
	     evas_object_image_fill_set(t2, 0, 0, 48, 48);
	     evas_object_show(t1);
	     evas_object_show(t2);
	  }
	if (((int)(t)) & 0x1)
	  {
	     evas_object_text_text_set(comment, "1 Show");
	     evas_object_show(t1);
	  }
	else
	  {
	     evas_object_text_text_set(comment, "1 Hide");
	     evas_object_hide(t1);
	  }
     }
   else if (t <= 156.0)
     {
	Evas_Coord              tw, th;
	int                 iw, ih;

	evas_object_text_text_set(title, "Test 11: Image Data Access");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	if (!test_pattern)

	  {
	     Evas_Object        *ob;

	     evas_object_hide(t1);
	     evas_object_hide(t2);
	     ob = evas_object_image_add(evas);
	     evas_object_image_file_set(ob, IM "test_pattern.png", NULL);
	     evas_object_image_size_get(ob, &iw, &ih);
	     ih = ((win_w - 8) * ih) / iw;
	     iw = (win_w - 8);
	     evas_object_resize(ob, iw, ih);
	     evas_object_image_fill_set(ob, 0, 0, iw, ih);
	     evas_object_move(ob, 4, (win_h - 240) + ((240 - ih) / 2));
	     evas_object_layer_set(ob, 5);
	     evas_object_clip_set(ob, panel_clip);
	     evas_object_show(ob);
	     test_pattern = ob;
	  }
	if (t <= 136.0)
	  {
	     int                 iw, ih;
	     int                 x, y;
	     int                *data;

	     evas_object_text_text_set(comment, "Play with all the image data");
	     evas_object_image_size_get(test_pattern, &iw, &ih);
	     evas_object_image_alpha_set(test_pattern, 0);
	     data = evas_object_image_data_get(test_pattern, 1);
	     if (data)
	       {
		  for (y = 0; y < ih; y++)
		    {
		       for (x = 0; x < iw; x++)
			 {
			    data[(y * iw) + x] =
			       (((x * y / 10) + (int)(t * 1000))) | 0xff000000;
			 }
		    }
		  evas_object_image_data_update_add(test_pattern, 0, 0, iw, ih);
		  evas_object_image_data_set(test_pattern, data);
	       }
	  }
	else if (t <= 146.0)
	  {
	     int                 iw, ih;
	     int                 x, y;
	     int                *data;

	     evas_object_text_text_set(comment,
				       "Play with part of the image data");
	     evas_object_image_size_get(test_pattern, &iw, &ih);
	     evas_object_image_alpha_set(test_pattern, 0);
	     data = evas_object_image_data_get(test_pattern, 1);
	     if (data)
	       {
		  for (y = ih / 4; y < ((3 * ih) / 4); y++)
		    {
		       for (x = iw / 4; x < ((3 * iw) / 4); x++)
			 {
			    data[(y * iw) + x] =
			       (((x * y / 10) + (int)(t * 1000))) | 0xff000000;
			 }
		    }
		  evas_object_image_data_update_add(test_pattern, iw / 4,
						    ih / 4, iw / 2, ih / 2);
		  evas_object_image_data_set(test_pattern, data);
	       }
	  }
	else
	  {
	     int                 iw, ih;

	     evas_object_text_text_set(comment, "Play with image size");
	     evas_object_image_size_get(test_pattern, &iw, &ih);
	     if ((iw > 1) && (ih > 1))
	       {
		  evas_object_image_size_set(test_pattern, iw - 1, ih - 1);
	       }
	  }
     }
   else if (t <= 166.0)
     {
	Evas_Coord              tw, th;

	evas_object_text_text_set(title, "Test 12: Image Destroy & Create");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	if (test_pattern)
	  {
	     evas_object_del(test_pattern);
	     test_pattern = NULL;
	     if (t1)
		evas_object_del(t1);
	     t1 = NULL;
	     if (t2)
		evas_object_del(t2);
	     t2 = NULL;

	  }
	if (((int)(t * 10.0)) & 0x1)
	  {
	     if (t2)
		evas_object_del(t2);
	     t2 = NULL;
	     if (!t1)
	       {
		  Evas_Object        *ob;

		  evas_object_text_text_set(comment,
					    "Destroy image 2, create 1");
		  ob = evas_object_image_add(evas);
		  evas_object_image_file_set(ob, IM "t1.png", NULL);
		  evas_object_move(ob, (win_w - 48) / 2,
				   (win_h - 240) + ((240 - 48) / 2));
		  evas_object_resize(ob, 48, 48);
		  evas_object_image_fill_set(ob, 0, 0, 48, 48);
		  evas_object_layer_set(ob, 5);
		  evas_object_clip_set(ob, panel_clip);
		  evas_object_image_smooth_scale_set(ob, 1);
		  evas_object_show(ob);
		  t1 = ob;
	       }
	  }
	else
	  {
	     if (t1)
		evas_object_del(t1);
	     t1 = NULL;
	     if (!t2)
	       {
		  Evas_Object        *ob;

		  evas_object_text_text_set(comment,
					    "Destroy image 1, create 2");
		  ob = evas_object_image_add(evas);
		  evas_object_image_file_set(ob, IM "t2.png", NULL);
		  evas_object_move(ob, (win_w - 48) / 2,
				   (win_h - 240) + ((240 - 48) / 2));
		  evas_object_resize(ob, 48, 48);
		  evas_object_image_fill_set(ob, 0, 0, 48, 48);
		  evas_object_layer_set(ob, 5);
		  evas_object_clip_set(ob, panel_clip);
		  evas_object_image_smooth_scale_set(ob, 0);
		  evas_object_show(ob);
		  t2 = ob;
	       }
	  }
     }
   else if (t <= 176.0)
     {
	Evas_Coord              tw, th;

	evas_object_text_text_set(title, "Test 13: Line Destroy & Create");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	if (((int)(t * 10.0)) & 0x1)
	  {
	     if (t2)
		evas_object_del(t2);
	     t2 = NULL;
	     if (!t1)
	       {
		  Evas_Object        *ob;

		  evas_object_text_text_set(comment,
					    "Destroy line blue, create red");
		  ob = evas_object_line_add(evas);
		  evas_object_line_xy_set(ob, 10, win_h - 240 + 10, win_w - 10,
					  win_h - 10);
		  evas_object_layer_set(ob, 5);
		  evas_object_clip_set(ob, panel_clip);
		  evas_object_color_set(ob, 200, 0, 0, 200);
		  evas_object_show(ob);
		  t1 = ob;
	       }
	  }
	else
	  {
	     if (t1)
		evas_object_del(t1);
	     t1 = NULL;
	     if (!t2)
	       {
		  Evas_Object        *ob;

		  evas_object_text_text_set(comment,
					    "Destroy line red, create blue");
		  ob = evas_object_line_add(evas);
		  evas_object_line_xy_set(ob, win_w - 10, win_h - 240 + 10, 10,
					  win_h - 10);
		  evas_object_layer_set(ob, 5);
		  evas_object_clip_set(ob, panel_clip);
		  evas_object_color_set(ob, 0, 0, 200, 200);
		  evas_object_show(ob);
		  t2 = ob;
	       }
	  }
     }
   else if (t <= 186.0)
     {
	Evas_Coord              tw, th;

	evas_object_text_text_set(title, "Test 14: Rectangle Destroy & Create");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	if (((int)(t * 10.0)) & 0x1)
	  {
	     if (t2)
		evas_object_del(t2);
	     t2 = NULL;
	     if (!t1)
	       {
		  Evas_Object        *ob;

		  evas_object_text_text_set(comment,
					    "Destroy rectangle blue, create red");
		  ob = evas_object_rectangle_add(evas);
		  evas_object_move(ob, (win_w - 48) / 2,
				   (win_h - 240) + ((240 - 48) / 2));
		  evas_object_resize(ob, 48, 48);
		  evas_object_layer_set(ob, 5);
		  evas_object_clip_set(ob, panel_clip);
		  evas_object_color_set(ob, 200, 0, 0, 200);
		  evas_object_show(ob);
		  t1 = ob;
	       }
	  }
	else
	  {
	     if (t1)
		evas_object_del(t1);
	     t1 = NULL;
	     if (!t2)
	       {
		  Evas_Object        *ob;

		  evas_object_text_text_set(comment,
					    "Destroy rectangle red, create blue");
		  ob = evas_object_rectangle_add(evas);
		  evas_object_move(ob, (win_w - 48) / 2,
				   (win_h - 240) + ((240 - 48) / 2));
		  evas_object_resize(ob, 48, 48);
		  evas_object_layer_set(ob, 5);
		  evas_object_clip_set(ob, panel_clip);
		  evas_object_color_set(ob, 0, 0, 200, 200);
		  evas_object_show(ob);
		  t2 = ob;
	       }
	  }
     }
   else if (t <= 196.0)
     {
	Evas_Coord              tw, th;

	evas_object_text_text_set(title, "Test 15: Gradient Destroy & Create");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	if (((int)(t * 10.0)) & 0x1)
	  {
	     if (t2)
		evas_object_del(t2);
	     t2 = NULL;
	     if (!t1)
	       {
		  Evas_Object        *ob;
		  float               d = 128 * sqrt(2);

		  evas_object_text_text_set(comment,
					    "Destroy gradient blue, create red");
		  ob = evas_object_gradient_add(evas);
		  evas_object_color_set(ob, 255, 0, 0, 255);
		  evas_object_gradient_alpha_stop_add(ob, 255, 1);
		  evas_object_gradient_alpha_stop_add(ob, 0, 1);
		  evas_object_gradient_fill_angle_set(ob, 45.0);
		  evas_object_gradient_fill_spread_set(ob, EVAS_TEXTURE_REFLECT);
		  evas_object_move(ob, (win_w - 128) / 2,
				   (win_h - 240) + ((240 - 128) / 2));
		  evas_object_resize(ob, 128, 128);
		  evas_object_gradient_fill_set(ob, 0, 127, d, d);
		  evas_object_layer_set(ob, 5);
		  evas_object_clip_set(ob, panel_clip);
		  evas_object_show(ob);
		  t1 = ob;
	       }
	  }
	else
	  {
	     if (t1)
		evas_object_del(t1);
	     t1 = NULL;
	     if (!t2)
	       {
		  Evas_Object        *ob;
		  float               d = 128 * sqrt(2);

		  evas_object_text_text_set(comment,
					    "Destroy gradient red, create blue");
		  ob = evas_object_gradient_add(evas);
		  evas_object_color_set(ob, 0, 0, 255, 255);
		  evas_object_gradient_alpha_stop_add(ob, 255, 1);
		  evas_object_gradient_alpha_stop_add(ob, 0, 1);
		  evas_object_gradient_fill_angle_set(ob, 45.0);
		  evas_object_gradient_fill_spread_set(ob, EVAS_TEXTURE_REFLECT);
		  evas_object_move(ob, (win_w - 128) / 2,
				   (win_h - 240) + ((240 - 128) / 2));
		  evas_object_resize(ob, 128, 128);
		  evas_object_gradient_fill_set(ob, 127, 0, d, d);
		  evas_object_layer_set(ob, 5);
		  evas_object_clip_set(ob, panel_clip);
		  evas_object_show(ob);
		  t2 = ob;
	       }
	  }
     }
   else if (t <= 206.0)
     {
	Evas_Coord              tw, th;

	evas_object_text_text_set(title, "Test 16: Polygon Destroy & Create");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	if (((int)(t * 10.0)) & 0x1)
	  {
	     if (t2)
		evas_object_del(t2);
	     t2 = NULL;
	     if (!t1)
	       {
		  Evas_Object        *ob;

		  evas_object_text_text_set(comment,
					    "Destroy polygon blue, create red");
		  ob = evas_object_polygon_add(evas);
		  evas_object_polygon_point_add(ob, 10, win_h - 240 + 10);
		  evas_object_polygon_point_add(ob, win_w - 10,
						win_h - 240 + 10);
		  evas_object_polygon_point_add(ob, 10, win_h - 10);
		  evas_object_layer_set(ob, 5);
		  evas_object_clip_set(ob, panel_clip);
		  evas_object_color_set(ob, 255, 0, 0, 200);
		  evas_object_show(ob);
		  t1 = ob;
	       }
	  }
	else
	  {
	     if (t1)
		evas_object_del(t1);
	     t1 = NULL;
	     if (!t2)
	       {
		  Evas_Object        *ob;

		  evas_object_text_text_set(comment,
					    "Destroy polygon red, create blue");
		  ob = evas_object_polygon_add(evas);
		  evas_object_polygon_point_add(ob, 10, win_h - 10);
		  evas_object_polygon_point_add(ob, win_w - 10, win_h - 10);
		  evas_object_polygon_point_add(ob, win_w - 10,
						win_h - 240 + 10);
		  evas_object_layer_set(ob, 5);
		  evas_object_clip_set(ob, panel_clip);
		  evas_object_color_set(ob, 0, 0, 255, 200);
		  evas_object_show(ob);
		  t2 = ob;
	       }
	  }
     }
   else if (t <= 216.0)
     {
	Evas_Coord              tw, th;

	evas_object_text_text_set(title, "Test 17: Text Destroy & Create");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	if (((int)(t * 10.0)) & 0x1)
	  {
	     if (t2)
		evas_object_del(t2);
	     t2 = NULL;
	     if (!t1)
	       {
		  Evas_Object        *ob;

		  evas_object_text_text_set(comment,
					    "Destroy text blue, create red");
		  ob = evas_object_text_add(evas);
		  evas_object_text_font_set(ob, "Vera", 12);
		  evas_object_text_text_set(ob, "Red test string");
		  evas_object_move(ob, (win_w / 2) - 50, win_h - 120);
		  evas_object_layer_set(ob, 5);
		  evas_object_clip_set(ob, panel_clip);
		  evas_object_color_set(ob, 255, 0, 0, 200);
		  evas_object_show(ob);
		  t1 = ob;
	       }
	  }
	else
	  {
	     if (t1)
		evas_object_del(t1);
	     t1 = NULL;
	     if (!t2)
	       {
		  Evas_Object        *ob;

		  evas_object_text_text_set(comment,
					    "Destroy text red, create blue");
		  ob = evas_object_text_add(evas);
		  evas_object_text_font_set(ob, "Vera", 12);
		  evas_object_text_text_set(ob, "Blue test string");
		  evas_object_move(ob, (win_w / 2) - 50, win_h - 120);
		  evas_object_layer_set(ob, 5);
		  evas_object_clip_set(ob, panel_clip);
		  evas_object_color_set(ob, 0, 0, 255, 200);
		  evas_object_show(ob);
		  t2 = ob;
	       }
	  }
     }
   else if (t <= 217.0)
     {
	if (t1)
	   evas_object_del(t1);
	t1 = NULL;
	if (t2)
	   evas_object_del(t2);
	t2 = NULL;
     }
   else if (t <= 227.0)
     {
	Evas_Coord              tw, th;

	evas_object_text_text_set(comment,
				  "Image objects can be clipped by rectangles");
	evas_object_text_text_set(title, "Test 18: Clip Objects");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	if (!t1)
	  {
	     Evas_Object        *ob;

	     ob = evas_object_image_add(evas);
	     evas_object_image_file_set(ob, IM "t1.png", NULL);
	     evas_object_move(ob, 5, win_h - 240 + 5);
	     evas_object_resize(ob, 48, 48);
	     evas_object_image_fill_set(ob, 0, 0, 48, 48);
	     evas_object_layer_set(ob, 5);
	     evas_object_clip_set(ob, panel_clip);
	     evas_object_image_smooth_scale_set(ob, 1);
	     evas_object_show(ob);
	     t1 = ob;
	     ob = evas_object_image_add(evas);
	     evas_object_image_file_set(ob, IM "t2.png", NULL);
	     evas_object_move(ob, 40, win_h - 240 + 40);
	     evas_object_resize(ob, 48, 48);
	     evas_object_image_fill_set(ob, 0, 0, 48, 48);
	     evas_object_layer_set(ob, 5);
	     evas_object_clip_set(ob, panel_clip);
	     evas_object_image_smooth_scale_set(ob, 0);
	     evas_object_show(ob);
	     t2 = ob;
	     ob = evas_object_rectangle_add(evas);
	     evas_object_move(ob, 10, win_h - 240 + 10);
	     evas_object_resize(ob, 180, 180);
	     evas_object_color_set(ob, 255, 255, 128, 255);
	     evas_object_clip_set(ob, panel_clip);
	     evas_object_show(ob);
	     c1 = ob;
	     ob = evas_object_rectangle_add(evas);
	     evas_object_move(ob, 10, win_h - 240 + 10);
	     evas_object_resize(ob, 180, 180);
	     evas_object_layer_set(ob, 5);
	     evas_object_color_set(ob, 100, 100, 50, 100);
	     evas_object_clip_set(ob, panel_clip);
	     evas_object_show(ob);
	     cv1 = ob;
	     ob = evas_object_rectangle_add(evas);
	     evas_object_move(ob, win_w - 10 - 180, win_h - 10 - 180);
	     evas_object_resize(ob, 180, 180);
	     evas_object_color_set(ob, 128, 255, 255, 255);
	     evas_object_clip_set(ob, panel_clip);
	     evas_object_show(ob);
	     c2 = ob;
	     ob = evas_object_rectangle_add(evas);
	     evas_object_move(ob, win_w - 10 - 180, win_h - 10 - 180);
	     evas_object_resize(ob, 180, 180);
	     evas_object_layer_set(ob, 5);
	     evas_object_color_set(ob, 50, 100, 100, 100);
	     evas_object_clip_set(ob, panel_clip);
	     evas_object_show(ob);
	     cv2 = ob;
	     evas_object_clip_set(t1, c1);
	     evas_object_clip_set(t2, c2);
	  }
	{
	   Evas_Coord              x, y;
	   double              v;

	   v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = cos(v * 1.7) * ((240 - 48) / 2);
	   y = sin(v * 1.9) * ((240 - 48) / 2);
	   evas_object_move(t1, 120 + x - 24, win_h - 120 + y - 24);
	   x = cos(1.0 + (v * 2.3)) * ((240 - 48) / 2);
	   y = sin(1.0 + (v * 1.45)) * ((240 - 48) / 2);
	   evas_object_move(t2, 120 + x - 24, win_h - 120 + y - 24);
	}
     }
   else if (t <= 237.0)
     {
	Evas_Coord              tw, th;

	evas_object_text_text_set(comment,
				  "Image objects can be clipped by recursively");
	evas_object_text_text_set(title, "Test 19: Recusrively Clip Objects");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	evas_object_clip_set(c2, c1);
	{
	   Evas_Coord              x, y;
	   double              v;

	   v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = cos(v * 1.7) * ((240 - 48) / 2);
	   y = sin(v * 1.9) * ((240 - 48) / 2);
	   evas_object_move(t1, 120 + x - 24, win_h - 120 + y - 24);
	   x = cos(1.0 + (v * 2.3)) * ((240 - 48) / 2);
	   y = sin(1.0 + (v * 1.45)) * ((240 - 48) / 2);
	   evas_object_move(t2, 120 + x - 24, win_h - 120 + y - 24);
	}
     }
   else if (t <= 247.0)
     {
	Evas_Coord              tw, th;

	evas_object_text_text_set(comment,
				  "Clip objects can move around too...");
	evas_object_text_text_set(title, "Test 19: Recusrively Clip Objects");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	{
	   Evas_Coord              x, y;
	   double              v;

	   v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = cos(v * 1.7) * ((240 - 180) / 2);
	   y = sin(v * 1.9) * ((240 - 180) / 2);
	   evas_object_move(c1, 120 + x - 90, win_h - 120 + y - 80);
	   evas_object_move(cv1, 120 + x - 90, win_h - 120 + y - 80);
	   x = cos(1.0 + (v * 2.3)) * ((240 - 180) / 2);
	   y = sin(1.0 + (v * 1.45)) * ((240 - 180) / 2);
	   evas_object_move(c2, 120 + x - 90, win_h - 120 + y - 90);
	   evas_object_move(cv2, 120 + x - 90, win_h - 120 + y - 90);
	}
     }
   else if (t <= 257.0)
     {
	Evas_Coord              tw, th;

	evas_object_text_text_set(comment,
				  "Clip objects can resize around too...");
	evas_object_text_text_set(title, "Test 20: Recusrively Clip Objects");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	{
	   Evas_Coord              x, y;
	   double              v;

	   evas_object_move(t1, 40, win_h - 240 + 60);
	   evas_object_move(t2, 60, win_h - 240 + 100);
	   v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = cos(v * 1.7) * ((180) / 2);
	   y = sin(v * 1.9) * ((180) / 2);
	   evas_object_resize(c1, 90 + x, 90 + y);
	   evas_object_resize(cv1, 90 + x, 90 + y);
	   x = cos(1.0 + (v * 2.3)) * ((180) / 2);
	   y = sin(1.0 + (v * 1.45)) * ((180) / 2);
	   evas_object_resize(c2, 90 + x, 90 + y);
	   evas_object_resize(cv2, 90 + x, 90 + y);
	}
     }
   else if (t <= 267.0)
     {
	Evas_Coord              tw, th;

	evas_object_text_text_set(comment, "Clip objects can hide and show...");
	evas_object_text_text_set(title, "Test 21: Show and Hide Clip Objects");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	evas_object_move(c1, 10, win_h - 240 + 10);
	evas_object_resize(c1, 180, 180);
	evas_object_move(cv1, 10, win_h - 240 + 10);
	evas_object_resize(cv1, 180, 180);
	evas_object_move(c2, win_w - 10 - 180, win_h - 10 - 180);
	evas_object_resize(c2, 180, 180);
	evas_object_move(cv2, win_w - 10 - 180, win_h - 10 - 180);
	evas_object_resize(cv2, 180, 180);
	if ((((int)(t)) & 0x3) == 0)
	  {
	     evas_object_text_text_set(comment, "Show yellow");
	     evas_object_show(c1);
	     evas_object_show(cv1);
	     evas_object_hide(c2);
	     evas_object_hide(cv2);
	  }
	else if ((((int)(t)) & 0x3) == 1)
	  {
	     evas_object_text_text_set(comment, "Show blue");
	     evas_object_hide(c1);
	     evas_object_hide(cv1);
	     evas_object_show(c2);
	     evas_object_show(cv2);
	  }
	else if ((((int)(t)) & 0x3) == 2)
	  {
	     evas_object_text_text_set(comment, "Show yellow, blue");
	     evas_object_show(c1);
	     evas_object_show(cv1);
	     evas_object_show(c2);
	     evas_object_show(cv2);
	  }
	else if ((((int)(t)) & 0x3) == 3)
	  {
	     evas_object_text_text_set(comment, "Hide both");
	     evas_object_hide(c1);
	     evas_object_hide(cv1);
	     evas_object_hide(c2);
	     evas_object_hide(cv2);
	  }
     }
   else if (t <= 277.0)
     {
	Evas_Coord              tw, th;

	evas_object_text_text_set(comment,
				  "Clip objects can clip and unclip...");
	evas_object_text_text_set(title, "Test 22: Clip and Unclip Objects");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	evas_object_move(t1, 20, win_h - 240 + 20);
	evas_object_move(t2, 160, win_h - 240 + 40);
	evas_object_move(c1, 10, win_h - 240 + 10);
	evas_object_resize(c1, 180, 180);
	evas_object_move(cv1, 10, win_h - 240 + 10);
	evas_object_resize(cv1, 180, 180);
	evas_object_move(c2, win_w - 10 - 180, win_h - 10 - 180);
	evas_object_resize(c2, 180, 180);
	evas_object_move(cv2, win_w - 10 - 180, win_h - 10 - 180);
	evas_object_resize(cv2, 180, 180);
	if ((((int)(t)) & 0x1) == 0)
	  {
	     evas_object_text_text_set(comment, "Unlip 2 from blue");
	     evas_object_clip_unset(t2);
	  }
	else if ((((int)(t)) & 0x1) == 1)
	  {
	     evas_object_text_text_set(comment, "Clip 2 to blue");
	     evas_object_clip_set(t2, c2);
	  }
     }
   else if (t <= 287.0)
     {
	Evas_Coord              tw, th;

	evas_object_text_text_set(comment, "Clip objects can be destroyed...");
	evas_object_text_text_set(title, "Test 23: Destroy Clip Objects");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	evas_object_move(t1, 20, win_h - 240 + 20);
	evas_object_move(t2, 160, win_h - 240 + 40);
	if (c1)
	  {
	     evas_object_move(c1, 10, win_h - 240 + 10);
	     evas_object_resize(c1, 180, 180);
	  }
	if (cv1)
	  {
	     evas_object_move(cv1, 10, win_h - 240 + 10);
	     evas_object_resize(cv1, 180, 180);
	  }
	if (c2)
	  {
	     evas_object_move(c2, win_w - 10 - 180, win_h - 10 - 180);
	     evas_object_resize(c2, 180, 180);
	  }
	if (cv2)
	  {
	     evas_object_move(cv2, win_w - 10 - 180, win_h - 10 - 180);
	     evas_object_resize(cv2, 180, 180);
	  }
	if (c2)
	  {
	     evas_object_clip_set(t2, c2);
	  }
	if (t < 282.0)
	  {
	     evas_object_text_text_set(comment, "Destroy yellow");
	     if (c1)
		evas_object_del(c1);
	     if (cv1)
		evas_object_del(cv1);
	     c1 = NULL;
	     cv1 = NULL;
	  }
	else
	  {
	     evas_object_text_text_set(comment, "Destroy blue");
	     if (c2)
		evas_object_del(c2);
	     if (cv2)
		evas_object_del(cv2);
	     c2 = NULL;
	     cv2 = NULL;
	  }
     }
   else if (t <= 288.0)
     {
	if (t1)
	   evas_object_del(t1);
	t1 = NULL;
	if (t2)
	   evas_object_del(t2);
	t2 = NULL;
     }
   else if (t <= 298.0)
     {
	Evas_Coord              tw, th;

	evas_object_text_text_set(comment,
				  "We can change the text of a text object");
	evas_object_text_text_set(title, "Test 24: Set Text Object Text");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	if (!t1)
	  {
	     Evas_Object        *ob;

	     ob = evas_object_text_add(evas);
	     evas_object_move(ob, 10, win_h - 240 + 60);
	     evas_object_color_set(ob, 0, 0, 0, 255);
	     evas_object_text_font_set(ob, "Vera", 8);
	     evas_object_text_text_set(ob, "Hello World [\xe2\x88\x9e]");
	     evas_object_clip_set(ob, panel_clip);
	     evas_object_layer_set(ob, 5);
	     evas_object_show(ob);
	     t1 = ob;
	  }
	if ((((int)(t * 4)) & 0x3) == 0)
	  {
	     evas_object_text_text_set(t1, "Hello World [ \xe2\x88\x9e ]");
	  }
	else if ((((int)(t * 4)) & 0x3) == 1)
	  {
	     evas_object_text_text_set(t1, "Heizölrückstoßabdämpfung");
	  }
	else if ((((int)(t * 4)) & 0x3) == 2)
	  {
	     evas_object_text_text_set(t1,
				       "В чащах юга жил бы цитрус? Да, но фальшивый экземпляр!");
	  }
	else if ((((int)(t * 4)) & 0x3) == 3)
	  {
	     evas_object_text_text_set(t1,
				       "All Evas text objects use UTF-8 Unicode");
	  }
     }
   else if (t <= 308.0)
     {
	Evas_Coord              tw, th;

	evas_object_text_text_set(comment, "We can change the font size");
	evas_object_text_text_set(title, "Test 25: Set Text Object Size");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	evas_object_text_font_set(t1, "Vera", (t - 278.0 + 0.5) * 8.0);
     }
   else if (t <= 318.0)
     {
	Evas_Coord              tw, th;

	evas_object_text_text_set(comment, "We can move text");
	evas_object_text_text_set(title, "Test 26: Move Text Object");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	evas_object_text_font_set(t1, "Vera", 24);
	evas_object_text_text_set(t1, "Evas");
	{
	   Evas_Coord              x, y;
	   double              v;

	   evas_object_geometry_get(t1, NULL, NULL, &tw, &th);
	   v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = cos(v * 1.7) * ((240 - tw) / 2);
	   y = sin(v * 1.9) * ((240 - th) / 2);
	   evas_object_move(t1, 120 + x - (tw / 2), win_h - 120 + y - (th / 2));
	}
     }
   else if (t <= 328.0)
     {
	Evas_Coord              tw, th;

	evas_object_text_text_set(comment, "We can change the font");
	evas_object_text_text_set(title, "Test 27: Set Text Object Font");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	if ((((int)(t * 4)) & 0x3) == 0)
	  {
	     evas_object_text_font_set(t1, "Vera", 48);
	  }
	else if ((((int)(t * 4)) & 0x3) == 1)
	  {
	     evas_object_text_font_set(t1, "Vera-Serif", 48);
	  }
	else if ((((int)(t * 4)) & 0x3) == 2)
	  {
	     evas_object_text_font_set(t1, "Vera-Italic", 48);
	  }
	else if ((((int)(t * 4)) & 0x3) == 3)
	  {
	     evas_object_text_font_set(t1, "Vera-Serif-Bold", 48);
	  }
	evas_object_text_text_set(t1, "Evas");
	evas_object_geometry_get(t1, NULL, NULL, &tw, &th);
	evas_object_move(t1, 120 - (tw / 2), win_h - 120 - (th / 2));
     }
   else if (t <= 338.0)
     {
	Evas_Coord              tw, th;
	int                     a = ((int)(t * 128)) & 0xff;

	if (!t2)
	  {
	     Evas_Object        *ob;

	     ob = evas_object_text_add(evas);
	     evas_object_clip_set(ob, panel_clip);
	     evas_object_layer_set(ob, 5);
	     evas_object_show(ob);
	     evas_object_text_font_set(ob, "Vera-Serif-Bold", 44);
	     t2 = ob;
	  }
	evas_object_text_text_set(comment, "We can change the color");
	evas_object_text_text_set(title, "Test 28: Set Text Object Color");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	evas_object_text_font_set(t1, "Vera-Serif-Bold", 44);
	if (t <= 333.0)
	{
	evas_object_render_op_set(t1, EVAS_RENDER_COPY);
	evas_object_text_text_set(t1, "Copy");
	}
	else if (t <= 338.0)
	{
	evas_object_render_op_set(t1, EVAS_RENDER_MUL);
	evas_object_text_text_set(t1, "Multiply");
	}
	evas_object_geometry_get(t1, NULL, NULL, &tw, &th);
	evas_object_move(t1, 120 - (tw / 2), win_h - 80 - (th / 2));
	evas_object_color_set(t1,
			      (a*(((int)(t * 255)) & 0xff))/255,
			      (a*(((int)(t * 200)) & 0xff))/255,
			      (a*(((int)(t * 133)) & 0xff))/255, a);

	evas_object_geometry_get(t2, NULL, NULL, &tw, &th);
	evas_object_move(t2, 120 - (tw / 2), win_h - 160 - (th / 2));
	evas_object_color_set(t2,
			      (a*(((int)(t * 255)) & 0xff))/255,
			      (a*(((int)(t * 200)) & 0xff))/255,
			      (a*(((int)(t * 133)) & 0xff))/255, a);
	evas_object_text_text_set(t2, "Blend");
     }
   else if (t <= 339.0)
     {
	if (t1)
	   evas_object_del(t1);
	t1 = NULL;
	if (t2)
	   evas_object_del(t2);
	t2 = NULL;
     }
   else if (t <= 349.0)
     {
	Evas_Coord              tw, th;

	evas_object_text_text_set(comment, "We can move gradients around");
	evas_object_text_text_set(title, "Test 29: Move Gradient Object");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	if (!t1)
	  {
	     Evas_Object        *ob;
	     int                 d = (120 / 2) * sqrt(2);
	     
	     ob = evas_object_gradient_add(evas);
	     evas_object_gradient_color_stop_add(ob, 255, 255, 255, 255, 1);
	     evas_object_gradient_color_stop_add(ob, 250, 192, 64, 255, 1);
	     evas_object_gradient_color_stop_add(ob, 220, 128, 0, 255, 1);
	     evas_object_gradient_color_stop_add(ob, 200, 64, 0, 255, 1);
	     evas_object_gradient_fill_angle_set(ob, 45.0);
	     evas_object_gradient_fill_spread_set(ob, EVAS_TEXTURE_REFLECT);
	     evas_object_move(ob, 60, win_h - 240 + 60);
	     evas_object_resize(ob, 120, 120);
	     evas_object_gradient_fill_set(ob, 0, 0, 120, d);
	     evas_object_clip_set(ob, panel_clip);
	     evas_object_layer_set(ob, 5);
	     evas_object_show(ob);
	     t1 = ob;
	  }
	  {
	     Evas_Coord              x, y;
	     double              v;
	     
	     evas_object_geometry_get(t1, NULL, NULL, &tw, &th);
	     v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	     x = cos(v * 1.7) * ((240 - tw) / 2);
	     y = sin(v * 1.9) * ((240 - th) / 2);
	     evas_object_move(t1, 120 + x - (tw / 2), win_h - 120 + y - (th / 2));
	  }
     }
   else if (t <= 359.0)
     {
	Evas_Coord              tw, th;

	evas_object_text_text_set(comment, "We can resize gradients");
	evas_object_text_text_set(title, "Test 30: Resize Gradient Object");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	  {
	     Evas_Coord              x, y;
	     double              v;
	     
	     v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	     x = cos(v * 1.7) * ((220) / 2);
	     y = sin(v * 1.9) * ((220) / 2);
	     evas_object_move(t1, 10, win_h - 240 + 10);
	     evas_object_resize(t1, x + 110, y + 110);
	     evas_object_gradient_color_data_set(t1, grad_map, 128, 1);
	     evas_object_gradient_fill_set(t1, 0, 0, 110, 110);
	}
     }
   else if (t <= 369.0)
     {
	Evas_Coord              tw, th;

	evas_object_text_text_set(comment,
				  "We can change gradient fill-angle & resize");
	evas_object_text_text_set(title, "Test 30: Gradient Fill-Angle");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	  {
	     Evas_Coord              x, y;
	     double              v;
	     
	     v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	     x = cos(v * 1.7) * ((220) / 2);
	     y = sin(v * 1.9) * ((220) / 2);
	     evas_object_resize(t1, x + 110, y + 110);
	     evas_object_gradient_fill_set(t1, 0, 0, 110, 110);
	     evas_object_gradient_fill_angle_set(t1, t * 60);
	     if (t >= 365.0)
	       {
	        evas_object_gradient_clear(t1);
	        evas_object_gradient_color_stop_add(t1, 255, 255, 255, 255, 1);
	        evas_object_gradient_color_stop_add(t1, 250, 192, 64, 255, 1);
	        evas_object_gradient_color_stop_add(t1, 220, 128, 0, 255, 1);
	        evas_object_gradient_color_stop_add(t1, 200, 64, 0, 255, 1);
	       }
	  }
     }
   else if (t <= 379.0)
     {
	Evas_Coord              tw, th;

	evas_object_text_text_set(comment,
				  "We can change gradient colors, angle & resize");
	evas_object_text_text_set(title, "Test 31: Gradient Stops");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	  {
	     Evas_Coord              x, y;
	     double              v;
	     
	     v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	     x = cos(v * 1.7) * ((220) / 2);
	     y = sin(v * 1.9) * ((220) / 2);
	     evas_object_resize(t1, x + 110, y + 110);
	     evas_object_gradient_fill_set(t1, 0, 0, 110, 110);
	     evas_object_gradient_fill_angle_set(t1, t * 60);
	     evas_object_gradient_clear(t1);
	     evas_object_gradient_color_stop_add(t1, 255, 255, 255, 255, 1);
	     evas_object_gradient_color_stop_add(t1,
						 ((int)(t * 250)) & 0xff,
						 ((int)(t * 200)) & 0xff,
						 ((int)(t * 150)) & 0xff,
						 255, 1);
	     evas_object_gradient_alpha_stop_add(t1,
						 ((int)(t * 350)) & 0xff, 1);
	     evas_object_gradient_color_stop_add(t1,
						 ((int)(t * 150)) & 0xff,
						 ((int)(t * 250)) & 0xff,
						 ((int)(t * 350)) & 0xff,
						 255, 1);
	     evas_object_gradient_alpha_stop_add(t1,
						 ((int)(t * 100)) & 0xff, 1);
	     evas_object_gradient_color_stop_add(t1,
						 ((int)(t * 120)) & 0xff,
						 ((int)(t * 260)) & 0xff,
						 ((int)(t * 490)) & 0xff,
						 255, 1);
	     evas_object_gradient_alpha_stop_add(t1,
						 ((int)(t * 230)) & 0xff, 1);
	     evas_object_gradient_color_stop_add(t1, 0, 0, 0, 0, 1);
	  }
     }
   else if (t <= 389.0)
     {
	Evas_Coord              tw, th;

	evas_object_text_text_set(comment,
				  "We can change the color of a gradient object");
	evas_object_text_text_set(title, "Test 32: Gradient Object Color");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	  {
	     Evas_Coord              x, y;
	     double              v;
	     int                 a;
	     
	     v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	     x = cos(v * 1.7) * ((220) / 2);
	     y = sin(v * 1.9) * ((220) / 2);
	     evas_object_resize(t1, x + 110, y + 110);
	     evas_object_gradient_fill_set(t1, 0, 0, 110, 110);
	     evas_object_gradient_fill_angle_set(t1, t * 60);
	     evas_object_gradient_color_data_set(t1, grad_map, 128, 1);
	     if (t >= 384.0)
	       {
	         evas_object_gradient_clear(t1);
	         evas_object_gradient_color_stop_add(t1, 255, 255, 255, 255, 1);
	         evas_object_gradient_color_stop_add(t1, 0, 0, 0, 255, 1);
	       }
	     a = ((int)(t * 230)) & 0xff;
	     evas_object_color_set(t1,
				   (a * (((int)(t * 120)) & 0xff)) / 255,
				   (a * (((int)(t * 260)) & 0xff)) / 255,
				   (a * (((int)(t * 490)) & 0xff)) / 255,
				   a);
	  }
     }
   else if (t <= 409.0)
     {
	Evas_Coord              tw, th;

	evas_object_text_text_set(comment,
				  "We can change the fill and spread of a gradient");
	evas_object_text_text_set(title, "Test 33: Gradient Fill Size and Spread");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	  {
	     Evas_Coord              y;
	     double                  v, d = (160 * sqrt(2)) / 2;
	     
	     v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	     y = sin(v * 0.5) * d;
	     evas_object_color_set(t1, 255, 255, 255, 255);
	     evas_object_resize(t1, 160, 160);
	     evas_object_move(t1, (win_w - 160) / 2, (win_h - 240) + ((240 - 160) / 2));
	     evas_object_gradient_fill_set(t1, 0, 0, 1, y);
	     evas_object_gradient_fill_angle_set(t1, 45.0);
	     evas_object_gradient_clear(t1);
	     evas_object_gradient_color_stop_add(t1, 255, 255, 255, 255, 2);
	     evas_object_gradient_color_stop_add(t1, 250, 192, 64, 255, 1);
	     evas_object_gradient_color_stop_add(t1, 220, 128, 0, 255, 1);
	     evas_object_gradient_color_stop_add(t1, 200, 64, 0, 255, 1);
	     if (t <= 394.0)
	       evas_object_gradient_fill_spread_set(t1, EVAS_TEXTURE_REFLECT);
	     else if (t <= 399.0)
	       evas_object_gradient_fill_spread_set(t1, EVAS_TEXTURE_REPEAT);
	     else if (t <= 404.0)
	       evas_object_gradient_fill_spread_set(t1, EVAS_TEXTURE_RESTRICT);
	     else
	       {
		  evas_object_gradient_offset_set(t1, (2.0 * (t - 405)) / 5.0);
		  evas_object_gradient_fill_set(t1, 0, -80, 1, d);
		  evas_object_gradient_fill_spread_set(t1, EVAS_TEXTURE_RESTRICT_REFLECT);
	       }
	}
     }
   else if (t <= 419.0)
     {
	Evas_Coord              tw, th;

	evas_object_text_text_set(comment,
				  "We can change the geometry of a gradient");
	evas_object_text_text_set(title, "Test 34: Gradient Type Radial");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	{
	   Evas_Coord              y;
	   double              v;

	   v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	   y = sin(v * 0.5) * ((110) / 2);
	   if (y < 0) y = -y;
	   evas_object_resize(t1, 110, 110);
	   evas_object_move(t1, (win_w - 110) / 2, (win_h - 240) + ((240 - 110) / 2));
	   evas_object_gradient_color_data_set(t1, grad_map, 128, 1);
	   if (t <= 414.0)
	     {
		evas_object_gradient_offset_set(t1, (2.0 * (t - 410)) / 5.0);
		evas_object_gradient_type_set(t1, "radial", NULL);
		evas_object_gradient_fill_set(t1, 55, 55, 55, 55);
		evas_object_gradient_fill_angle_set(t1, 0);
		evas_object_gradient_fill_spread_set(t1, EVAS_TEXTURE_RESTRICT_REPEAT);
	     }
	   else
	     {
		char                s[1024];
		
		snprintf(s, sizeof(s), "inner_radius = %3.3f;", 0.7 * (1.0 - (y / 55.0)));
		evas_object_gradient_type_set(t1, "radial", s);
		evas_object_gradient_fill_set(t1, 55, 55, 55, y);
		evas_object_gradient_fill_angle_set(t1, t * 60);
		evas_object_gradient_fill_spread_set(t1, EVAS_TEXTURE_RESTRICT);
		evas_object_render_op_set(t1, EVAS_RENDER_MUL);
	     }
	}
     }
   else if (t <= 439.0)
     {
	Evas_Coord              tw, th;
	
	evas_object_text_text_set(title, "Test 34: Gradient Type Angular");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	  {
	     Evas_Coord              y;
	     double              v;
	     
	     v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	     y = sin(v * 0.5) * ((110) / 2);
	     if (y < 0) y = -y;
	     evas_object_render_op_set(t1, EVAS_RENDER_BLEND);
	     evas_object_gradient_fill_angle_set(t1, t * 60);
	     evas_object_gradient_fill_spread_set(t1, EVAS_TEXTURE_REFLECT);
	     evas_object_gradient_offset_set(t1, 0.0);
	     evas_object_resize(t1, 110, 110);
	     evas_object_move(t1, (win_w - 110) / 2, (win_h - 240) + ((240 - 110) / 2));
	     if (t <= 429.0)
	       {
		  evas_object_gradient_fill_set(t1, 55, 55, 55, 55);
		  evas_object_gradient_type_set(t1, "angular", NULL);
	       }
	     else
	       {
		  char                s[1024];
		  
		  evas_object_gradient_clear(t1);
		  evas_object_gradient_color_stop_add(t1, 255, 255, 255, 255, 2);
		  evas_object_gradient_color_stop_add(t1, 250, 192, 64, 255, 1);
		  evas_object_gradient_color_stop_add(t1, 220, 128, 0, 255, 1);
		  evas_object_gradient_color_stop_add(t1, 200, 64, 0, 255, 1);
		  evas_object_gradient_fill_set(t1, 55, 55, 55, y);
		  snprintf(s, sizeof(s), "annulus = %3.3f; wrap = 0.25;", 0.7 * (1.0 - (y / 55.0)));
		  evas_object_gradient_type_set(t1, "angular", s);
	       }
	  }
     }
   else if (t <= 454.0)
     {
	Evas_Coord              tw, th;

	evas_object_text_text_set(title, "Test 34: Gradient Type Sinusoidal");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	  {
	     Evas_Coord              y;
	     double              v;
	     
	     evas_object_gradient_fill_angle_set(t1, t * 60);
	     v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	     y = sin(v * 0.5) * (110 / 2);
	     evas_object_gradient_fill_set(t1, 55, 55, 55, y);
	     evas_object_gradient_type_set(t1, "sinusoidal", NULL);
	     if (t <= 444.0)
	       {
		  evas_object_gradient_fill_spread_set(t1, EVAS_TEXTURE_REFLECT);
	       }
	     else if (t <= 449.0)
	       {
		  unsigned  char amap[2] = {255, 0};

		  evas_object_color_set(t1, 0, 155, 255, 255);
		  evas_object_gradient_alpha_data_set(t1, amap, 2);
		  evas_object_gradient_fill_spread_set(t1, EVAS_TEXTURE_REPEAT);
	       }
	     else
	       {
		  unsigned  char amap[2] = {255, 0};

		  evas_object_color_set(t1, 255, 255, 255, 255);
		  evas_object_gradient_color_data_set(t1, grad_map, 128, 1);
		  evas_object_gradient_alpha_data_set(t1, amap, 2);
		  evas_object_gradient_fill_spread_set(t1, EVAS_TEXTURE_RESTRICT);
	       }
	  }
     }
   else if (t <= 455.0)
     {
	if (t1)
	   evas_object_del(t1);
	t1 = NULL;
	if (t2)
	   evas_object_del(t2);
	t2 = NULL;
     }
   else if (t <= 464.0)
     {
	Evas_Coord              tw, th;

	evas_object_text_text_set(comment,
				  "We can move, resize and recolor rectangles");
	evas_object_text_text_set(title, "Test 35: Rectangles");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	if (!t1)
	  {
	     Evas_Object        *ob;

	     ob = evas_object_rectangle_add(evas);
	     evas_object_move(ob, 60, win_h - 240 + 60);
	     evas_object_resize(ob, 120, 120);
	     evas_object_clip_set(ob, panel_clip);
	     evas_object_layer_set(ob, 5);
	     evas_object_show(ob);
	     t1 = ob;
	  }
	{
	   Evas_Coord              x, y;
	   double              v;
	   int                 a = ((int)(t * 230)) & 0xff;

	   v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = cos(v * 1.3) * ((220) / 2);
	   y = sin(v * 1.1) * ((220) / 2);
	   evas_object_resize(t1, 110 + x, 110 + y);
	   evas_object_geometry_get(t1, NULL, NULL, &tw, &th);
	   v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = cos(v * 1.7) * ((240 - tw) / 2);
	   y = sin(v * 1.9) * ((240 - th) / 2);
	   evas_object_move(t1, 120 + x - (tw / 2), win_h - 120 + y - (th / 2));
	   evas_object_color_set(t1,
				 (a * (((int)(t * 120)) & 0xff)) / 255,
				 (a * (((int)(t * 260)) & 0xff)) / 255,
				 (a * (((int)(t * 490)) & 0xff)) / 255,
				 a);
	}
     }
   else if (t <= 465.0)
     {
	if (t1)
	   evas_object_del(t1);
	t1 = NULL;
	if (t2)
	   evas_object_del(t2);
	t2 = NULL;
     }
   else if (t <= 474.0)
     {
	Evas_Coord              tw, th;

	evas_object_text_text_set(comment,
				  "We can modify line begin & end coords");
	evas_object_text_text_set(title, "Test 36: Lines");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	if (!t1)
	  {
	     Evas_Object        *ob;

	     ob = evas_object_line_add(evas);
	     evas_object_line_xy_set(ob,
				     10, win_h - 240 + 10,
				     win_w - 10, win_h - 10);
	     evas_object_color_set(ob, 0, 0, 0, 255);
	     evas_object_clip_set(ob, panel_clip);
	     evas_object_layer_set(ob, 5);
	     evas_object_show(ob);
	     t1 = ob;
	  }
	{
	   Evas_Coord              x, y, xx, yy;
	   double              v;
	   int                 a = ((int)(t * 230)) & 0xff;

	   v = (((t - 2.0) / 12.0) * EVAS_PI * 2) * 2;
	   x = cos(v * 1.3) * ((240 - 20) / 2);
	   y = sin(v * 1.8) * ((240 - 20) / 2);
	   v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	   xx = cos(v * 1.7) * ((240 - 20) / 2);
	   yy = sin(v * 1.9) * ((240 - 20) / 2);
	   evas_object_line_xy_set(t1,
				   (win_w / 2) + x, (win_h - 120) + y,
				   (win_w / 2) + xx, (win_h - 120) + yy);
	   evas_object_color_set(t1,
				 (a * (((int)(t * 120)) & 0xff)) / 255,
				 (a * (((int)(t * 260)) & 0xff)) / 255,
				 (a * (((int)(t * 490)) & 0xff)) / 255,
				 a);
	}
     }
   else if (t <= 483.0)
     {
	Evas_Coord              tw, th;

	evas_object_text_text_set(comment,
				  "We can move, resize and recolor lines");
	evas_object_text_text_set(title, "Test 37: Line Move & Resize");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	{
	   Evas_Coord              x, y;
	   double              v;
	   int                 a = ((int)(t * 230)) & 0xff;

	   v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = cos(v * 1.3) * ((220) / 2);
	   y = sin(v * 1.1) * ((220) / 2);
	   evas_object_resize(t1, 112 + x, 112 + y);
	   evas_object_geometry_get(t1, NULL, NULL, &tw, &th);
	   v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = cos(v * 1.7) * ((240 - tw) / 2);
	   y = sin(v * 1.9) * ((240 - th) / 2);
	   evas_object_move(t1, 120 + x - (tw / 2), win_h - 120 + y - (th / 2));
	   evas_object_color_set(t1,
				 (a * (((int)(t * 120)) & 0xff)) / 255,
				 (a * (((int)(t * 260)) & 0xff)) / 255,
				 (a * (((int)(t * 490)) & 0xff)) / 255,
				 a);
	}
     }
   else if (t <= 484.0)
     {
	if (t1)
	   evas_object_del(t1);
	t1 = NULL;
	if (t2)
	   evas_object_del(t2);
	t2 = NULL;
     }
   else if (t <= 494.0)
     {
	Evas_Coord              tw, th;

	evas_object_text_text_set(comment, "We can modify polygon points");
	evas_object_text_text_set(title, "Test 38: Polygons");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	if (!t1)
	  {
	     Evas_Object        *ob;

	     ob = evas_object_polygon_add(evas);
	     evas_object_polygon_point_add(ob, 10, 10);
	     evas_object_polygon_point_add(ob, 80, 20);
	     evas_object_polygon_point_add(ob, 30, 100);
	     evas_object_color_set(ob, 0, 0, 0, 255);
	     evas_object_clip_set(ob, panel_clip);
	     evas_object_layer_set(ob, 5);
	     evas_object_show(ob);
	     t1 = ob;
	  }
	{
	   Evas_Coord              x, y;
	   double              v;
	   int                 a = ((int)(t * 230)) & 0xff;

	   evas_object_polygon_points_clear(t1);
	   v = (((t - 2.0) / 12.0) * EVAS_PI * 2) * 2;
	   x = cos(v * 1.3) * ((240 - 20) / 2);
	   y = sin(v * 1.8) * ((240 - 20) / 2);
	   evas_object_polygon_point_add(t1, (win_w / 2) + x,
					 (win_h - 120) + y);
	   v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = cos(v * 1.7) * ((240 - 20) / 2);
	   y = sin(v * 1.9) * ((240 - 20) / 2);
	   evas_object_polygon_point_add(t1, (win_w / 2) + x,
					 (win_h - 120) + y);
	   v = (((t - 3.0) / 11.0) * EVAS_PI * 2) * 3;
	   x = cos(v * 1.2) * ((240 - 20) / 2);
	   y = sin(v * 1.1) * ((240 - 20) / 2);
	   evas_object_polygon_point_add(t1, (win_w / 2) + x,
					 (win_h - 120) + y);
	   v = (((t - 8.0) / 13.0) * EVAS_PI * 2) * 1;
	   x = cos(v * 1.7) * ((240 - 20) / 2);
	   y = sin(v * 1.4) * ((240 - 20) / 2);
	   evas_object_polygon_point_add(t1, (win_w / 2) + x,
					 (win_h - 120) + y);
	   evas_object_color_set(t1,
				 (a * (((int)(t * 120)) & 0xff)) / 255,
				 (a * (((int)(t * 260)) & 0xff)) / 255,
				 (a * (((int)(t * 490)) & 0xff)) / 255,
				 a);
	}
     }
   else if (t <= 503.0)
     {
	Evas_Coord              tw, th;

	evas_object_text_text_set(comment,
				  "We can move, resize and recolor polygons");
	evas_object_text_text_set(title, "Test 39: Polygon Move & Resize");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
	{
	   Evas_Coord              x, y;
	   double              v;
	   int                 a = ((int)(t * 230)) & 0xff;

	   v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = cos(v * 1.3) * ((220) / 2);
	   y = sin(v * 1.1) * ((220) / 2);
	   evas_object_resize(t1, 110 + x, 110 + y);
	   evas_object_geometry_get(t1, NULL, NULL, &tw, &th);
	   v = (((t - 6.0) / 10.0) * EVAS_PI * 2) * 2;
	   x = cos(v * 1.7) * ((240 - tw) / 2);
	   y = sin(v * 1.9) * ((240 - th) / 2);
	   evas_object_move(t1, 120 + x - (tw / 2), win_h - 120 + y - (th / 2));
	   evas_object_color_set(t1,
				 (a * (((int)(t * 120)) & 0xff)) / 255,
				 (a * (((int)(t * 260)) & 0xff)) / 255,
				 (a * (((int)(t * 490)) & 0xff)) / 255,
				 a);
	}
     }
   else if (t <= 504.0)
     {
	if (t1)
	   evas_object_del(t1);
	t1 = NULL;
	if (t2)
	   evas_object_del(t2);
	t2 = NULL;
     }
   else if (t <= 509.0)
     {
	Evas_Coord              tw, th;

	evas_object_text_text_set(comment,
				  "We are now done with testing. Goodbye!");
	evas_object_text_text_set(title, "The End.");
	evas_object_geometry_get(title, NULL, NULL, &tw, &th);
	evas_object_move(title, (win_w - tw) / 2, win_h - th);
     }
   else if (t <= 514.0)
     {
	printf("################ evas free\n");
	evas_free(evas);
	printf("evas freed. DONE\n");
	exit(0);
     }
   /* need to test:
    *
    * callbacks & events...
    *
    */
}

void
cb_mouse_down(void *data, Evas *e, Evas_Object *obj,
	      void *event)
{
   Evas_Event_Mouse_Down *ev;

   ev = event;

   if (mode == 0)
     {
	mode = 1;
	setdown();
	scroll_setup();
	orig_start_time = start_time = get_time();
	loop_count = orig_loop_count = 0;
     }
   else
     {
	mode = 0;
	scroll_setdown();
	setup();
	orig_start_time = start_time = get_time();
	loop_count = orig_loop_count = 0;
     }
   printf("cb_mouse_down() [%i], %4i,%4i | %4i,%4i\n", ev->button,
	  ev->output.x, ev->output.y, ev->canvas.x, ev->canvas.y);
}

void
cb_mouse_up(void *data, Evas * e, Evas_Object * obj, Evas_Event_Mouse_Up * ev)
{
   printf("cb_mouse_up()   [%i], %4i,%4i | %4i,%4i\n", ev->button,
	  ev->output.x, ev->output.y, ev->canvas.x, ev->canvas.y);
}

void
cb_mouse_in(void *data, Evas * e, Evas_Object * obj, Evas_Event_Mouse_In * ev)
{
   printf("cb_mouse_in()   [%08x], %4i,%4i | %4i,%4i\n", ev->buttons,
	  ev->output.x, ev->output.y, ev->canvas.x, ev->canvas.y);
}

void
cb_mouse_out(void *data, Evas * e, Evas_Object * obj, Evas_Event_Mouse_Out * ev)
{
   printf("cb_mouse_out()  [%08x], %4i,%4i | %4i,%4i\n", ev->buttons,
	  ev->output.x, ev->output.y, ev->canvas.x, ev->canvas.y);
}

void
cb_mouse_move(void *data, Evas * e, Evas_Object * obj,
	      Evas_Event_Mouse_Move * ev)
{
   printf
      ("cb_mouse_move()  [%08x], %4i,%4i | %4i,%4i (%4i,%4i | %4i,%4i)\n",
       ev->buttons, ev->cur.output.x, ev->cur.output.y, ev->cur.canvas.x,
       ev->cur.canvas.y, ev->prev.output.x, ev->prev.output.y,
       ev->prev.canvas.x, ev->prev.canvas.y);
}

void
scroll_setdown(void)
{
   int i;

   evas_object_del(scroll[0]);
   for (i = 1; i < 16; i++)
     evas_object_del(scroll[i]);
}

void
scroll_setup(void)
{
   Evas_Object        *ob;
   int                 iw, ih;
   int                 i;

   ob = evas_object_image_add(evas);
   evas_object_image_file_set(ob, IM "backdrop.png", NULL);
   evas_object_move(ob, 0, 0);
   evas_object_resize(ob, 240, 320);
   evas_object_image_fill_set(ob, 0, 0, 240, 320);
   evas_object_layer_set(ob, 0);
   evas_object_show(ob);
   scroll[0] = ob;

   for (i = 1; i < 16; i++)
     {
	ob = evas_object_image_add(evas);
	evas_object_image_file_set(ob, IM "e_logo.png", NULL);
	evas_object_move(ob, 0, 0);
	evas_object_resize(ob, 240, 320);
	evas_object_image_size_get(ob, &iw, &ih);
	evas_object_image_fill_set(ob, 0, 0, iw, ih);
	evas_object_layer_set(ob, 1);
	evas_object_show(ob);
	evas_object_event_callback_add(ob, EVAS_CALLBACK_MOUSE_DOWN, cb_mouse_down, NULL);
	scroll[i] = ob;
     }
}

void
setdown(void)
{
   evas_object_del(backdrop);
   evas_object_del(e_logo);
   evas_object_del(panel);
   evas_object_del(panel_top);
   evas_object_del(panel_shadow);
   evas_object_del(panel_clip);
   evas_object_del(evas_logo);
}

void
setup(void)
{
   Evas_Object        *ob;
   int                 iw, ih;

   evas_font_path_prepend(evas, FN);
   evas_image_cache_set(evas, 2048 * 1024);
   evas_font_cache_set(evas, 1024 * 1024);
//   evas_image_cache_set(evas, 0);
//   evas_font_cache_set(evas, 0);

   ob = evas_object_image_add(evas);
   evas_object_image_file_set(ob, IM "backdrop.png", NULL);
   evas_object_move(ob, 0, 0);
   evas_object_resize(ob, 240, 320);
   evas_object_image_fill_set(ob, 0, 0, 240, 320);
   evas_object_layer_set(ob, 0);
   evas_object_show(ob);
   backdrop = ob;

   ob = evas_object_image_add(evas);
   evas_object_image_file_set(ob, IM "e_logo.png", NULL);
   
//   evas_object_image_save(ob, "t.png", NULL, "compress=9");
//   evas_object_image_save(ob, "t.jpg", NULL, "quality=20 compress=7");
//   evas_object_image_save(ob, "t.eet", "key", "quality=50");
   
   evas_object_image_size_get(ob, &iw, &ih);
   evas_object_move(ob, (win_w - iw) / 2, 0);
   evas_object_resize(ob, iw, ih);
   evas_object_image_fill_set(ob, 0, 0, iw, ih);
   evas_object_layer_set(ob, 1);
   evas_object_show(ob);
   evas_object_event_callback_add(ob, EVAS_CALLBACK_MOUSE_DOWN, cb_mouse_down, NULL);
   e_logo = ob;

   ob = evas_object_image_add(evas);
   evas_object_image_file_set(ob, IM "panel.png", NULL);
   evas_object_move(ob, 0, win_h - 240);
   evas_object_resize(ob, win_w, 240);
   evas_object_image_fill_set(ob, 0, 0, win_w, 240);
   evas_object_layer_set(ob, 3);
   evas_object_show(ob);
   panel = ob;

   ob = evas_object_image_add(evas);
   evas_object_image_file_set(ob, IM "panel_top.png", NULL);
   evas_object_image_size_get(ob, &iw, &ih);
   evas_object_move(ob, 0, win_h - 240 - ih);
   evas_object_resize(ob, win_w, ih);
   evas_object_image_fill_set(ob, 0, 0, win_w, ih);
   evas_object_layer_set(ob, 3);
   evas_object_show(ob);
   panel_top = ob;

   ob = evas_object_image_add(evas);
   evas_object_image_file_set(ob, IM "panel_shadow.png", NULL);
   evas_object_image_size_get(ob, &iw, &ih);
   evas_object_move(ob, 0, win_h - 240);
   evas_object_resize(ob, win_w, ih);
   evas_object_image_fill_set(ob, 0, 0, win_w, ih);
   evas_object_layer_set(ob, 10);
   evas_object_show(ob);
   panel_shadow = ob;

   ob = evas_object_rectangle_add(evas);
   evas_object_move(ob, 0, win_h - 240);
   evas_object_resize(ob, win_w, 240);
   evas_object_color_set(ob, 255, 255, 255, 255);
   evas_object_show(ob);
   panel_clip = ob;

   ob = evas_object_image_add(evas);
   evas_object_image_file_set(ob, IM "evas_logo.png", NULL);
   evas_object_image_size_get(ob, &iw, &ih);
   evas_object_move(ob, (win_w - iw) / 2, (win_h - 240) + ((240 - ih) / 2));
   evas_object_resize(ob, iw, ih);
   evas_object_image_fill_set(ob, 0, 0, iw, ih);
   evas_object_layer_set(ob, 5);
   evas_object_clip_set(ob, panel_clip);
   evas_object_show(ob);
   evas_logo = ob;
}

#if 0
evas_object_event_callback_add(ob, EVAS_CALLBACK_MOUSE_DOWN, cb_mouse_down,
			       NULL);
evas_object_event_callback_add(ob, EVAS_CALLBACK_MOUSE_UP, cb_mouse_up, NULL);
evas_object_event_callback_add(ob, EVAS_CALLBACK_MOUSE_IN, cb_mouse_in, NULL);
evas_object_event_callback_add(ob, EVAS_CALLBACK_MOUSE_OUT, cb_mouse_out, NULL);
evas_object_event_callback_add(ob, EVAS_CALLBACK_MOUSE_MOVE, cb_mouse_move,
			       NULL);

ob = evas_object_rectangle_add(evas);
evas_object_move(ob, 40, 10);
evas_object_resize(ob, 60, 40);
evas_object_color_set(ob, 0, 0, 0, 155);
evas_object_show(ob);
r1 = ob;
ob = evas_object_rectangle_add(evas);
evas_object_move(ob, 60, 20);
evas_object_resize(ob, 80, 50);
evas_object_color_set(ob, (200*100)/255, (200*200)/255, (200*30)/255, 200);
evas_object_show(ob);
r2 = ob;
ob = evas_object_rectangle_add(evas);
evas_object_move(ob, 90, 45);
evas_object_resize(ob, 100, 170);
evas_object_color_set(ob, (120*200)/255, (120*120)/255, (120*80)/255, 120);
evas_object_show(ob);
r3 = ob;
ob = evas_object_gradient_add(evas);
evas_object_move(ob, 20, 120);
evas_object_resize(ob, 60, 80);
evas_object_color_set(ob, 255, 255, 255, 255);
evas_object_gradient_color_stop_add(ob, 255, 255, 255, 255, 10);
evas_object_gradient_color_stop_add(ob, (180*250)/255, (180*240)/255, (180*50)/255, 180, 10);
evas_object_gradient_color_stop_add(ob, (120*220)/255, (120*60)/255, 0, 120, 10);
evas_object_gradient_color_stop_add(ob, (80*200)/255, 0, 0, 80, 10);
evas_object_gradient_color_stop_add(ob, 0, 0, 0, 0, 10);
evas_object_gradient_angle_set(ob, 45.0);
evas_object_show(ob);
g1 = ob;
ob = evas_object_gradient_add(evas);
evas_object_move(ob, 40, 110);
evas_object_resize(ob, 64, 48);
evas_object_color_set(ob, 255, 255, 255, 255);
evas_object_gradient_color_stop_add(ob, 255, 255, 255, 255, 10);
evas_object_gradient_color_stop_add(ob, 50, 240, 250, 255, 10);
evas_object_gradient_color_stop_add(ob, 0, 60, 220, 255, 10);
evas_object_gradient_color_stop_add(ob, 0, 0, 200, 255, 10);
evas_object_gradient_color_stop_add(ob, 0, 0, 0, 255, 10);
evas_object_gradient_angle_set(ob, 45.0);
evas_object_show(ob);
g2 = ob;
ob = evas_object_line_add(evas);
evas_object_color_set(ob, 0, 0, 0, 255);
evas_object_line_xy_set(ob, 20, 40, 80, 130);
evas_object_show(ob);
l1 = ob;
ob = evas_object_polygon_add(evas);
evas_object_color_set(ob, (100*10)/255, (100*80)/255, (100*200)/255, 100);
evas_object_polygon_point_add(ob, 10, 10);
evas_object_polygon_point_add(ob, 80, 20);
evas_object_polygon_point_add(ob, 30, 100);
evas_object_show(ob);
p1 = ob;
ob = evas_object_image_add(evas);
evas_object_image_file_set(ob, PACKAGE_SOURCE_DIR "/data/image.png", NULL);
evas_object_move(ob, 40, 30);
evas_object_resize(ob, 120, 120);
evas_object_image_fill_set(ob, 0, 0, 120, 120);
evas_object_show(ob);
i2 = ob;
ob = evas_object_image_add(evas);
evas_object_image_file_set(ob, PACKAGE_SOURCE_DIR "/data/test_32.png", NULL);
evas_object_move(ob, 2, 2);
evas_object_image_border_set(ob, 5, 5, 5, 5);
evas_object_image_smooth_scale_set(ob, 1);
evas_object_resize(ob, 128, 128);
evas_object_image_fill_set(ob, 0, 0, 64, 64);
evas_object_show(ob);
i3 = ob;
   /*
    * Danish:
    * "Quizdeltagerne spiste jordbær med fløde, mens cirkusklovnen"
    * german:
    * "Heizölrückstoßabdämpfung"
    * Spanish:
    * "El pingüino Wenceslao hizo kilómetros bajo exhaustiva lluvia y"
    * French:
    * "Le cœur déçu mais l'âme plutôt naïve, Louÿs rêva de crapaüter en"
    * Irish Gaelic:
    * "D'fhuascail Íosa, Úrmhac na hÓighe Beannaithe, pór Éava agus Ádhaimh"
    * Hungarian:
    * "Árvíztűrő tükörfúrógép"
    * Icelandic:
    * "Kæmi ný öxi hér ykist þjófum nú bæði víl og ádrepa"
    * Japanese (hiragana):
    * "いろはにほへとちりぬるを"
    * Japanese (katakana):
    * "イロハニホヘト チリヌルヲ ワカヨタレソ ツネナラム"
    * Hebrew:
    * "? דג סקרן שט בים מאוכזב ולפתע מצא לו חברה איך הקליטה"
    * Polish:
    * "Pchnąć w tę łódź jeża lub ośm skrzyń fig"
    * Russian:
    * "В чащах юга жил бы цитрус? Да, но фальшивый экземпляр!"
    * IPA:
    * "ˈjunɪˌkoːd"
    * American Dictionary:
    * "Ūnĭcōde̽"
    * Anglo-saxon:
    * "ᛡᚢᚾᛁᚳᚩᛞ"
    * Arabic:
    * "يونِكود"
    * Armenian:
    * "Յունիկօդ"
    * Bengali:
    * "য়ূনিকোড"
    * Bopomofo:
    * "ㄊㄨㄥ˅ ㄧˋ ㄇㄚ˅"
    * Canadian Syllabics:
    * "ᔫᗂᑰᑦ"
    * Cherokee:
    * "ᏳᏂᎪᏛ"
    * Chinese:
    * "萬國碼"
    * Ethiopic:
    * "ዩኒኮድ"
    * Georgian:
    * "უნიკოდი"
    * Greek:
    * "Γιούνικοντ"
    * Gujarati:
    * "યૂનિકોડ"
    * Gurmukhi:
    * "ਯੂਨਿਕੋਡ"
    * Hindi:
    * "यूनिकोड"
    * Kannada:
    * "ಯೂನಿಕೋಡ್"
    * Khmer:
    * "យូនីគោដ"
    * Korean:
    * "유니코드"
    * Malayalam:
    * "യൂനികോഡ്"
    * Ogham:
    * "ᚔᚒᚅᚔᚉᚑᚇ"
    * Oriya:
    * "ୟୂନିକୋଡ"
    * Persian:
    * "یونی‌کُد"
    * Sinhala:
    * "යණනිකෞද්"
    * Syriac:
    * "ܝܘܢܝܩܘܕ"
    * Tamil:
    * "யூனிகோட்"
    * Telugu:
    * "యూనికోడ్"
    * Thai:
    * "ยูนืโคด"
    * Tibetan:
    * "ཨུ་ནི་ཀོཌྲ།"
    * Yiddish:
    * "יוניקאָד"
    *
    */
ob = evas_object_text_add(evas);
evas_object_move(ob, 10, 10);
evas_object_color_set(ob, 0, 0, 0, 255);
evas_object_text_font_set(ob, "Vera", 14);
evas_object_text_text_set(ob, "Hello World [\xe2\x88\x9e]");
/* the 2 following commented out sections dont display japanese */
/* and i'm not sure why they don't. something deep down in the font rendering */
/* code.. but i don't know what */
/*   evas_object_text_font_set(ob, "cyberbit", 14);*/
/*   evas_object_text_text_set(ob, "Hello World - いろはにほへとちりぬるを");*/
evas_object_show(ob);
t1 = ob;

ob = evas_object_rectangle_add(evas);
evas_object_move(ob, 10, 10);
evas_object_resize(ob, 220, 300);
evas_object_color_set(ob, 255, 255, 255, 255);
evas_object_show(ob);
c1 = ob;

evas_object_clip_set(r1, c1);
evas_object_clip_set(r2, c1);
evas_object_clip_set(r3, c1);
evas_object_clip_set(g1, c1);
#endif
