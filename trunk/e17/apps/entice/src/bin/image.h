#ifndef __IMAGE_H__
#define __IMAGE_H__

void                image_add_from_ipc(char *item);

void                image_create_list(int argc, char **argv);
void                image_create_list_dir(char *dir);
void                image_destroy_list(void);

void                image_create_thumbnails(void);

Image              *e_image_new(char *file);
void                e_image_free(Image * im);
void                image_delete(Image * im);

void                e_rotate_r_current_image(void);
void                e_rotate_l_current_image(void);

void                e_turntable_l_current_image(void);
void                e_turntable_r_current_image(void);
void		    e_turntable_reset(void);

void		    e_zoom_in(int x, int y);
void		    e_zoom_out(int x, int y);
void		    e_zoom_normal(void);
void		    e_zoom_full(void);
void                e_flip_h_current_image(void);
void                e_flip_v_current_image(void);

void                e_delete_current_image(void);
void                e_load_prev_image(void);
void                e_load_next_image(void);
void                e_save_current_image(void);
void                e_display_current_image(void);

void                next_image(void *data, Evas * e, Evas_Object * obj,
			       void *event_info);

void                next_image_up(void *data, Evas * e, Evas_Object * obj,
				  void *event_info);
void                next_image_move(void *data, Evas * e, Evas_Object * obj,
				    void *event_info);
void                next_image_wheel(void *data, Evas * e, Evas_Object * obj,
				    void *event_info);

#endif /* __IMAGE_H__ */
