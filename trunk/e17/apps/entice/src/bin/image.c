#include "entice.h"

#define DEFAULT_FORMAT "png"

int turntable_rotation = 0;

void
image_add_from_ipc(char *item)
{
   DIR                *d;
   struct dirent      *dent;
   Image              *im;
   char                buf[4096];

   if (item==NULL)
     return;
   printf("adding new image: %s\n", item);
   if (e_file_is_dir(item))
     {
	d = opendir(item);
	while ((dent = readdir(d)) != NULL)
	  {
	     if (!strcmp(dent->d_name, ".") || !strcmp(dent->d_name, "..")
		 || dent->d_name[0] == '.')
		continue;

	     sprintf(buf, "%s/%s", item, dent->d_name);
	     im = e_image_new(buf);
	     im->subst = 1;
	     images = evas_list_append(images, im);
	  }
	closedir(d);
     }
   else
     {
	im = e_image_new(item);
	im->subst = 1;
	images = evas_list_append(images, im);
     }
   need_thumbs = 1;
   // e_display_current_image();
   return;
}

void
image_create_list(int argc, char **argv)
{
   int                 i;
   if (argc==2 && e_file_is_dir(argv[1])) {
     printf("taking argument to be directory name: %s\n", argv[1]);
      image_create_list_dir(argv[1]);
      return;
   }

   for (i = 1; i < argc; i++)
     {
	Image              *im;
	/*
	char                buf[4096];

	buf = e_file_full_name(argv[i]);
	printf("%s\n",buf);
	*/
	im = e_image_new(e_file_full_name(argv[i]));
	images = evas_list_append(images, im);
     }
   current_image = images;
}

void
image_create_list_dir(char *dir)
{
   DIR                *d;
   struct dirent      *dent;
   Image              *im;

   dir = e_file_full_name(dir);

   d = opendir(dir);

   while ((dent = readdir(d)) != NULL)
      // while( readdir_r(d,dent,&dent) )
     {
       char buf[4096];
	/* skip these */
	if (!strcmp(dent->d_name, ".") || !strcmp(dent->d_name, "..")
	    || dent->d_name[0] == '.')
	   continue;

	snprintf(buf, 4096, "%s/%s", dir, dent->d_name);
	im = e_image_new(buf);
	images = evas_list_append(images, im);
     }
   closedir(d);

   current_image = images;
}

void
image_create_thumbnails(void)
{
   Evas_List          *l;
   int                 i;

   i = 1;
   for (l = images; l; l = l->next, i++)
     {
	Image              *im;

	im = l->data;

	if (im->o_thumb) return;
	im->o_thumb = evas_object_image_add(evas);
	evas_object_image_file_set(im->o_thumb, IM "thumb.png", NULL);
	evas_object_event_callback_add(im->o_thumb, EVAS_CALLBACK_MOUSE_MOVE,
				       e_list_item_drag, l);
	evas_object_event_callback_add(im->o_thumb, EVAS_CALLBACK_MOUSE_DOWN,
				       e_list_item_click, l);
	evas_object_event_callback_add(im->o_thumb, EVAS_CALLBACK_MOUSE_UP,
				       e_list_item_select, l);
	evas_object_event_callback_add(im->o_thumb, EVAS_CALLBACK_MOUSE_IN,
				       e_list_item_in, l);
	evas_object_event_callback_add(im->o_thumb, EVAS_CALLBACK_MOUSE_OUT,
				       e_list_item_out, l);
	im->subst = 1;
	evas_object_image_border_set(im->o_thumb, 4, 4, 4, 4);
	evas_object_move(im->o_thumb, 2, 2 + ((48 + 2) * (i - 1)));
	evas_object_resize(im->o_thumb, 48, 48);
	evas_object_image_fill_set(im->o_thumb, 0, 0, 48, 48);
	evas_object_layer_set(im->o_thumb, 249);
	evas_object_show(im->o_thumb);
     }
}

void
image_destroy_list(void)
{
   Evas_List          *l;
   Image              *im;

   for (l = images; l; l = l->next)
     {
	im = l->data;

	evas_object_event_callback_del(im->o_thumb, EVAS_CALLBACK_MOUSE_DOWN,
				       e_list_item_click);
	evas_object_event_callback_del(im->o_thumb, EVAS_CALLBACK_MOUSE_IN,
				       e_list_item_in);
	evas_object_event_callback_del(im->o_thumb, EVAS_CALLBACK_MOUSE_OUT,
				       e_list_item_out);

	evas_object_del(im->o_thumb);

	e_image_free(im);
     }

   images = evas_list_free(images);
}

Image              *
e_image_new(char *file)
{
   Image              *im;

   im = malloc(sizeof(Image));
   im->file = strdup(file);
   im->generator = 0;
   im->thumb = NULL;
   im->o_thumb = NULL;
   im->subst = 0;
   return im;
}

void
e_image_free(Image * im)
{
   if (im->file)
      free(im->file);
   if (im->thumb)
      free(im->thumb);
   free(im);
}

void
image_delete(Image * im)
{
   if (im)
     {
	if (current_image && im == current_image->data) {
	  if (current_image->next)
	    current_image = current_image->next;
	  else if (current_image->prev)
	    current_image = current_image->prev;
	  else
	    current_image = NULL;
	  e_display_current_image();
	}
	if (im->o_thumb)
	   evas_object_del(im->o_thumb);

	images = evas_list_remove(images, (void *)im);
	e_image_free(im);
     }
}

static void
e_flip_object(Evas_Object *obj, int direction)
{
   int w;
   int h;
   DATA32 *image_data;
   Imlib_Image image;

   if (!obj)
       return;

   /* Get image data from Evas */
   evas_object_image_size_get(obj, &w, &h);
   image_data = evas_object_image_data_get(obj, 0);
   if (!image_data)
     {
         evas_object_image_data_set(obj, image_data);
         return;
     }

   /* Set up imlib image */
   image = imlib_create_image_using_copied_data(w, h, image_data);
   evas_object_image_data_set(obj, image_data);
   imlib_context_set_image(image);

   /* Flip image */
   if (direction==1)
         imlib_image_flip_horizontal();
   else
         imlib_image_flip_vertical();

   /* Get image data from Imlib */
   image_data = imlib_image_get_data_for_reading_only();

   /* Set Evas Image Data */
   evas_object_image_size_set(obj, w, h);
   evas_object_image_data_copy_set(obj, image_data);

   /* Free Imlib image */
   imlib_image_put_back_data(image_data);
   imlib_free_image();
}

static void
e_flip_current_image(int direction)
{
   Image *im;

   if (!current_image || !current_image->data)
       return;

   im = (Image *) (current_image->data);

   /* Flip image */
   e_flip_object(o_image, direction);
   e_flip_object(o_mini_image, direction);
   im->modified = 1;
   e_flip_object(im->o_thumb, direction);

   /* Update Display */
   e_turntable_reset();
   e_handle_resize();
   e_fix_icons();
   e_scroll_list(NULL);
   e_fade_scroller_in((void *)1);

}

void
e_flip_h_current_image(void)
{
	e_flip_current_image(1);
}

void
e_flip_v_current_image(void)
{
	e_flip_current_image(2);
}

static void
e_rotate_object(Evas_Object *obj, int rotation)
{
   int w;
   int h;
   DATA32 *image_data;
   Imlib_Image image;

   if (!obj)
       return;

   /* Get image data from Evas */
   evas_object_image_size_get(obj, &w, &h);
   image_data = evas_object_image_data_get(obj, 0);
   if (!image_data)
     {
         evas_object_image_data_set(obj, image_data);
         return;
     }

   /* Set up imlib image */
   image = imlib_create_image_using_copied_data(w, h, image_data);
   evas_object_image_data_set(obj, image_data);
   imlib_context_set_image(image);

   /* Rotate image */
   imlib_image_orientate(rotation);
   w = imlib_image_get_width();
   h = imlib_image_get_height();

   /* Get image data from Imlib */
   image_data = imlib_image_get_data_for_reading_only();

   /* Set Evas Image Data */
   evas_object_image_size_set(obj, w, h);
   evas_object_image_data_copy_set(obj, image_data);

   /* Free Imlib image */
   imlib_image_put_back_data(image_data);
   imlib_free_image();
}

static void
e_rotate_current_image(int rotation)
{
   Image *im;

   if (!current_image || !current_image->data)
       return;

   im = (Image *) (current_image->data);

   /* Rotate image */
   e_rotate_object(o_image, rotation);
   e_rotate_object(o_mini_image, rotation);
   im->modified = 1;
   e_rotate_object(im->o_thumb, rotation);

   /* Update Display */
   e_turntable_reset();
   e_handle_resize();
   e_fix_icons();
   e_scroll_list(NULL);
   e_fade_scroller_in((void *)1);

}

void
e_rotate_r_current_image(void)
{
	e_rotate_current_image(1);
}

void
e_rotate_l_current_image(void)
{
	e_rotate_current_image(3);
}

#define TURNTABLE_COUNT 30
Imlib_Image turntable_image[TURNTABLE_COUNT];
int turntable_image_no = -1;

void
e_turntable_object_init(Evas_Object *obj)
{
   int i;
   int w;
   int h;
   double angle;
   DATA32 *image_data;
   Imlib_Image image;

   if (!obj || turntable_image_no >= 0)
       return;

   	/* Get image data from Evas */
   evas_object_image_size_get(obj, &w, &h);
   image_data = evas_object_image_data_get(obj, 0);
   if (!image_data)
     {
         evas_object_image_data_set(obj, image_data);
         return;
     }

   /* Set up imlib image */
   image = imlib_create_image_using_copied_data(w, h, image_data);
   evas_object_image_data_set(obj, image_data);

   angle = 0;
   for(i = 0 ; i < TURNTABLE_COUNT ; i ++) {
	angle = i * 360 * acos(0) / 90 / TURNTABLE_COUNT;
   	imlib_context_set_image(image);
   	turntable_image[i] = imlib_create_rotated_image(angle);
   	imlib_context_set_image(turntable_image[i]);
   }
	
  /* Free Imlib image */
  imlib_context_set_image(image);
  imlib_free_image();

  turntable_image_no = 0;
}

void
e_turntable_reset()
{
   int i;

   if (turntable_image_no < 0)
       return;

   turntable_image_no = -1;

   for(i = 0 ; i < TURNTABLE_COUNT ; i ++) {
   	imlib_context_set_image(turntable_image[i]);
	imlib_free_image();
   }
}

static void
e_turntable_object_next(Evas_Object *obj)
{
   int w;
   int h;
   DATA32 *image_data;

   if (!obj)
       return;

   e_turntable_object_init(obj);

   if (turntable_rotation == 1) {
      if(++turntable_image_no >= TURNTABLE_COUNT) {
	   turntable_image_no = 0;
      }
   }
   else {
      if(--turntable_image_no < 0) {
	   turntable_image_no = TURNTABLE_COUNT - 1;
      }
   }

   /* Get image data from Imlib */
   imlib_context_set_image(turntable_image[turntable_image_no]);
   image_data = imlib_image_get_data_for_reading_only();
   w = imlib_image_get_width();
   h = imlib_image_get_height();

   /* Set Evas Image Data */
   evas_object_image_size_set(obj, w, h);
   evas_object_image_data_set(obj, image_data);

  imlib_image_put_back_data(image_data);

}

int
e_turntable_object(void *data)
{
   Evas_Object* obj;
   obj = (Evas_Object *) data;
   if (turntable_image_no < 0)
	 return 0;

   e_turntable_object_next(obj);

   /* Update Display */
   e_handle_resize();
   return 1;
}

static void
e_turntable_current_image(int rotation)
{
   int w;
   int h;
   if (!current_image || !current_image->data)
       return;

   evas_object_image_size_get(o_image, &w, &h);
   if (turntable_image_no < 0 && w * h * 4 > 1048576)
      return;

   turntable_rotation = rotation;
   ecore_timer_add((double) 60 / (double) 33 / (double) TURNTABLE_COUNT, e_turntable_object, (void *) o_image);

}

void
e_turntable_r_current_image(void)
{
	e_turntable_current_image(1);
}

void
e_turntable_l_current_image(void)
{
	e_turntable_current_image(2);
}

void
e_zoom_in(int x, int y)
{
  if (!o_image) return;
  scale /= 1.414;
  if (scale < 0.03125)
    scale = 0.03125;
  /*
  scroll_x = ((double)(scroll_x + win_w/2 - x))*1.414;
  scroll_y = ((double)(scroll_y + win_h/2 - y))*1.414;
  */
  if (x == -1)
    scroll_x *= 1.414;
  else
    scroll_x = scroll_x*1.414 + (win_w/2 - x)*(0.414);
  if (y == -1)
    scroll_y *= 1.414;
  else
    scroll_y = scroll_y*1.414 + (win_w/2 - x)*(0.414);
  //printf("scroll_x: %i, scroll_y: %i\n", scroll_x, scroll_y);
  e_handle_resize();
}

void
e_zoom_out(int x, int y)
{
  if (!o_image) return;
  scale *= 1.414;
  /*
  scroll_x = ((double)(scroll_x + win_w/2 - x))/1.414;
  scroll_y = ((double)(scroll_y + win_h/2 - y))/1.414;
  */
  if (x == -1)
    scroll_x /= 1.414;
  else
    scroll_x = scroll_x/1.414 - (win_w/2 - x)*(0.293);
  if (y == -1)
    scroll_y /= 1.414;
  else
    scroll_y = scroll_y/1.414 - (win_w/2 - x)*(0.293);
  //printf("scroll_x: %i, scroll_y: %i\n", scroll_x, scroll_y);
  e_handle_resize();
}

void
e_zoom_normal(void)
{
  if (!o_image) return;
  scale = 1.0;
  e_handle_resize();
}
  
void
e_zoom_full(void)
{
  double              sh, sv;
  int                 w, h;

  if (!o_image) return;
  evas_object_image_size_get(o_image, &w, &h);
  sh = (double)w / (double)win_w;
  sv = (double)h / (double)win_h;
  if (sh > sv)
    scale = sh;
  else
    scale = sv;
  e_handle_resize();
}

void
e_delete_current_image(void)
{
   Evas_List          *l = NULL;

   Image              *im;

   if (!current_image || !current_image->data)
	return;

   im = (Image *) (current_image->data);

   if (im->file)
      unlink(im->file);
   if (im->thumb)
      unlink(im->thumb);
   if (current_image->next)
      l = current_image->next;
   else if (current_image->prev)
      l = current_image->prev;
   else
      l = NULL;

   if (im->o_thumb)
      evas_object_del(im->o_thumb);
   e_image_free((Image *) current_image->data);
   images = evas_list_remove(images, current_image->data);

   current_image = l;

   e_display_current_image();
}

static void
_e_save_image(Evas_Object *obj, const char *path)
{
   int w;
   int h;
   DATA32 *image_data;
   Imlib_Image image;
   int alpha_team; /* Speed Racer! */
   const char *format;

   if (!obj || !path)
       return;

   /* Get image data from Evas */
   evas_object_image_size_get(obj, &w, &h);
   image_data = evas_object_image_data_get(obj, 0);
   if (!image_data)
     {
         evas_object_image_data_set(obj, image_data);
         return;
     }

   /* Set up imlib image */
   image = imlib_create_image_using_copied_data(w, h, image_data);
   evas_object_image_data_set(obj, image_data);
   imlib_context_set_image(image);

   alpha_team = evas_object_image_alpha_get(obj);
   format = strrchr(path, '.') + 1;

   if(!*format)
	   format = DEFAULT_FORMAT;

   /* Save Image */
   imlib_image_set_format(format);
   imlib_image_set_has_alpha(alpha_team);  /* Go Speed, Go */
   imlib_save_image(path);


   /* Free Imlib image */
   imlib_free_image();
}


void
e_save_current_image(void)
{
   Image *im;

   if (!current_image || !current_image->data || !o_image)
       return;

   im = (Image *) (current_image->data);

   _e_save_image(o_image, im->file);
   if(im->modified)
      _e_save_image(im->o_thumb, im->thumb);
   im->modified = 0;
}


static void
e_update_thumb(void)
{
   Image *im;

   if (!current_image)
      return;

   im = (Image *) (current_image->data);

   if(!im->modified)
      return;

   evas_object_image_file_set(im->o_thumb, im->thumb, NULL);
   evas_object_image_reload(im->o_thumb);
   im->modified = 0;
}


void
e_load_next_image(void)
{
   e_update_thumb();
   if (!current_image)
      current_image = images;
   else if (current_image->next)
      current_image = current_image->next;
   e_display_current_image();
}


void
e_load_prev_image(void)
{
   e_update_thumb();
   if (!current_image)
      current_image = images;
   else if (current_image->prev)
      current_image = current_image->prev;
   e_display_current_image();
}


void
e_display_current_image(void)
{
   Imlib_Image im;
   DATA32 *data;
   int mustUseImlib = 0;

   e_turntable_reset();
   if (o_mini_image)
     {
	evas_object_del(o_mini_image);
	o_mini_image = NULL;
     }
   if (current_image)
     {
	char                title[4096];

	if (o_image)
	  {
	    evas_object_del(o_image);
	    o_image = NULL;
	  }
	o_image = evas_object_image_add(evas);
	evas_object_image_file_set(o_image,
				   ((Image *) (current_image->data))->file,
				   NULL);
	evas_object_event_callback_add(o_image, EVAS_CALLBACK_MOUSE_DOWN,
				       next_image, NULL);
	evas_object_event_callback_add(o_image, EVAS_CALLBACK_MOUSE_UP,
				       next_image_up, NULL);
	evas_object_event_callback_add(o_image, EVAS_CALLBACK_MOUSE_MOVE,
				       next_image_move, NULL);
	evas_object_event_callback_add(o_image, EVAS_CALLBACK_MOUSE_WHEEL,
				       next_image_wheel, NULL);
	evas_object_show(o_image);
	// if evas can't load the thing...
	if (evas_object_image_load_error_get(o_image) != EVAS_LOAD_ERROR_NONE)
	  {
	    // try Imlib

	    mustUseImlib = 1;
	    im = imlib_load_image(((Image *)(current_image->data))->file);
	    if (im) {
	      int w, h;
	      enum active_state command = active_out;
	      
	      e_fade_logo(&command);
	      imlib_context_set_image(im);
	      w = imlib_image_get_width();
	      h = imlib_image_get_height();
	      data = imlib_image_get_data_for_reading_only();
	      evas_object_image_size_set(o_image, w, h);
	      evas_object_image_data_set(o_image, data);
	    } else { // we really can't load it
	     enum active_state command = active_in;

	     e_fade_logo(&command);
	     sprintf(txt_info[0], "Error Loading File: %s",
		     ((Image *) (current_image->data))->file);
	     *txt_info[1] = '\0';
	     sprintf(title, "Entice (Error Loading): %s",
		     ((Image *) (current_image->data))->file);
	     ecore_evas_title_set(ecore_evas, title);
	     evas_object_del(o_image);
	     o_image = NULL;
	    }
	  }
	else
	  {
	     int                 w, h;
	     enum active_state command = active_out;

	     e_fade_logo(&command);
	     evas_object_image_size_get(o_image, &w, &h);
	     sprintf(txt_info[0], "File: %s",
		     ((Image *) (current_image->data))->file);
	     sprintf(txt_info[1], "Size: %ix%i", w, h);
	     e_fade_info_in(NULL);

	     sprintf(title, "Entice: %s",
		     ((Image *) (current_image->data))->file);
	     ecore_evas_title_set(ecore_evas, title);
	  }
     }
   else
     {
	ecore_evas_title_set(ecore_evas, "Entice (No Image)");
          {
	     enum active_state command = active_in;
	     evas_object_del(o_image);
	     o_image = NULL;
	     e_fade_logo(&command);
	  }
     }
   if ((o_image) && (current_image))
     {
	o_mini_image = evas_object_image_add(evas);
	evas_object_image_smooth_scale_set(o_mini_image, 0);
	if (mustUseImlib) {
	  evas_object_image_data_set(o_mini_image, data);
	  imlib_image_put_back_data(data);
	} else {
	  evas_object_image_file_set(o_mini_image,
				      ((Image *) (current_image->data))->file,
				      NULL);
	}
	evas_object_show(o_mini_image);

     }
   e_handle_resize();
   e_fix_icons();
   e_scroll_list(NULL);
   e_fade_scroller_in((void *)1);
}

void
next_image(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Evas_Event_Mouse_Down *ev;

   ev = event_info;

   down_x = ev->output.x;
   down_y = ev->output.y;
   down_sx = scroll_x;
   down_sy = scroll_y;
   e_fade_scroller_in(NULL);
}

void
next_image_up(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Evas_Event_Mouse_Up *ev;

   ev = event_info;
   if (((ev->output.x - down_x) * (ev->output.x - down_x)) +
       ((ev->output.y - down_y) * (ev->output.y - down_y)) > 9)
     {
	e_fade_scroller_out(NULL);
	return;
     }
   if ((obj == o_showpanel) && (panel_active))
      return;
   if (!current_image)
     {
        current_image = images;
        e_display_current_image();
     }
   else
     {
	if (ev->button == 1)
	   e_load_next_image();
	else if (ev->button == 3)
	   e_load_prev_image();
     }
}

void
next_image_move(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Evas_Event_Mouse_Move *ev;

   ev = event_info;
   if (ev->buttons != 0 && o_image)
     {
        int w,h;
        evas_object_image_size_get(o_image, &w, &h);
	w = (int)((double)w / scale);
	h = (int)((double)h / scale);
	scroll_x = ev->cur.output.x - down_x + down_sx;
	scroll_y = ev->cur.output.y - down_y + down_sy;
	if (w > win_w)
	  {
	     if (scroll_x > ((w - win_w) / 2))
		scroll_x = ((w - win_w) / 2);
	     if (scroll_x < -((w - win_w + 1) / 2))
		scroll_x = -((w - win_w + 1) / 2);
	  }
	else
	   scroll_x = 0;
	if (h > win_h)
	  {
	     if (scroll_y > ((h - win_h) / 2))
		scroll_y = ((h - win_h) / 2);
	     if (scroll_y < -((h - win_h + 1) / 2))
		scroll_y = -((h - win_h + 1) / 2);
	  }
	else
	   scroll_y = 0;

	/*
	if (scroll_x > w / 2)
	  scroll_x = w / 2;
	else if (scroll_x < -w / 2)
	  scroll_x = -w / 2;
	if (scroll_y > h / 2)
	  scroll_y = h / 2;
	else if (scroll_y < -h / 2)
	  scroll_y = -h / 2;
	*/
	//printf("sx: %i, sy: %i\n", scroll_x, scroll_y);

	e_handle_resize();
     }
}

void
next_image_wheel(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
  Evas_Event_Mouse_Wheel *ev;

  ev = event_info;
  // printf("x: %i, y: %i\n", ev->output.x, ev->output.y);
  if (ev->z > 0) {
    e_zoom_out(ev->output.x, ev->output.y);
  } else {
    e_zoom_in(ev->output.x, ev->output.y);
  }
}
