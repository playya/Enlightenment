#ifndef __IMLIB_API_H
# define __IMLIB_API_H 1

# ifndef DATA64
#  define DATA64 unsigned long long
#  define DATA32 unsigned int
#  define DATA16 unsigned short
#  define DATA8  unsigned char
# endif

/* data types - guess what - no transparent datatypes - all hidden */
typedef void * Imlib_Image;
typedef void * Imlib_Color_Modifier;
typedef void * Imlib_Updates;
typedef void * Imlib_Font;
typedef void * Imlib_Color_Range;
typedef struct _imlib_border Imlib_Border;
typedef struct _imlib_color Imlib_Color;
typedef struct _imlib_rectangle Imlib_Rectangle;

enum _imlib_operation
{
   IMLIB_OP_COPY,
   IMLIB_OP_ADD,
   IMLIB_OP_SUBTRACT,
   IMLIB_OP_RESHADE
};

enum _imlib_text_direction
{
   IMLIB_TEXT_TO_RIGHT = 0,
   IMLIB_TEXT_TO_LEFT = 1,
   IMLIB_TEXT_TO_DOWN = 2,
   IMLIB_TEXT_TO_UP = 3
};

enum _imlib_load_error
{
   IMLIB_LOAD_ERROR_NONE,
   IMLIB_LOAD_ERROR_FILE_DOES_NOT_EXIST,
   IMLIB_LOAD_ERROR_FILE_IS_DIRECTORY,
   IMLIB_LOAD_ERROR_PERMISSION_DENIED_TO_READ,
   IMLIB_LOAD_ERROR_NO_LOADER_FOR_FILE_FORMAT,
   IMLIB_LOAD_ERROR_PATH_TOO_LONG,
   IMLIB_LOAD_ERROR_PATH_COMPONENT_NON_EXISTANT,
   IMLIB_LOAD_ERROR_PATH_COMPONENT_NOT_DIRECTORY,
   IMLIB_LOAD_ERROR_PATH_POINTS_OUTSIDE_ADDRESS_SPACE,
   IMLIB_LOAD_ERROR_TOO_MANY_SYMBOLIC_LINKS,
   IMLIB_LOAD_ERROR_OUT_OF_MEMORY,
   IMLIB_LOAD_ERROR_OUT_OF_FILE_DESCRIPTORS,
   IMLIB_LOAD_ERROR_PERMISSION_DENIED_TO_WRITE,
   IMLIB_LOAD_ERROR_OUT_OF_DISK_SPACE,
   IMLIB_LOAD_ERROR_UNKNOWN
};

typedef enum _imlib_operation Imlib_Operation;
typedef enum _imlib_load_error Imlib_Load_Error;
typedef enum _imlib_text_direction Imlib_Text_Direction;

struct _imlib_border
{
   int left, right, top, bottom;
};

struct _imlib_color
{
   int alpha, red, green, blue;
};

struct _imlib_rectangle
{
   int x, y, width, height;
};

typedef void (*Imlib_Progress_Function)(Imlib_Image im, char percent,
					int update_x, int update_y,
					int update_w, int update_h);
typedef void (*Imlib_Data_Destructor_Function)(Imlib_Image im, void *data);

void imlib_context_set_display(Display *display);
void imlib_context_set_visual(Visual *visual);
void imlib_context_set_colormap(Colormap colormap);
void imlib_context_set_drawable(Drawable drawable);
void imlib_context_set_anti_alias(char anti_alias);
void imlib_context_set_dither(char dither);
void imlib_context_set_blend(char blend);
void imlib_context_set_color_modifier(Imlib_Color_Modifier color_modifier);
void imlib_context_set_operation(Imlib_Operation operation);
void imlib_context_set_font(Imlib_Font font);
void imlib_context_set_direction(Imlib_Text_Direction direction);
void imlib_context_set_color(int red, int green, int blue, int alpha);
void imlib_context_set_color_range(Imlib_Color_Range color_range);
void imlib_context_set_progress_function(Imlib_Progress_Function progress_function);
void imlib_context_set_progress_granularity(char progress_granularity);
void imlib_context_set_image(Imlib_Image image);

int     imlib_get_cache_size(void);
void    imlib_set_cache_size(int bytes);
int     imlib_get_color_usage(void);
void    imlib_set_color_usage(int max);
void    imlib_flush_loaders(void);
int     imlib_get_visual_depth(Display *display, Visual *visual);
Visual *imlib_get_best_visual(Display *display, int screen, int *depth_return);

Imlib_Image imlib_load_image(char *file);
Imlib_Image imlib_load_image_immediately(char *file);
Imlib_Image imlib_load_image_without_cache(char *file);
Imlib_Image imlib_load_image_immediately_without_cache(char *file);
Imlib_Image imlib_load_image_with_error_return(char *file, Imlib_Load_Error *error_return);
void        imlib_free_image(void);
void        imlib_free_image_and_decache(void);

int     imlib_image_get_width(void);
int     imlib_image_get_height(void);
DATA32 *imlib_image_get_data(void);
DATA32 *imlib_image_get_data_for_reading_only(void);
void    imlib_image_put_back_data(DATA32 *data);
char    imlib_image_has_alpha(void);
void    imlib_image_set_never_changes_on_disk(Imlib_Image image);
void    imlib_image_get_border(Imlib_Border *border);
void    imlib_image_set_border(Imlib_Border *border);
void    imlib_image_set_format(char *format);
void    imlib_image_set_irrelevant_format(char irrelevant);
void    imlib_image_set_irrelevant_border(char irrelevant);
void    imlib_image_set_irrelevant_alpha(char irrelevant);
char   *imlib_image_format(void);
void    imlib_image_set_has_alpha(char has_alpha);

void        imlib_render_pixmaps_for_whole_image(Pixmap *pixmap_return, Pixmap *mask_return, char create_dithered_mask);
void        imlib_render_pixmaps_for_whole_image_at_size(Pixmap *pixmap_return, Pixmap *mask_return, char create_dithered_mask, int width, int height);
void        imlib_render_image_on_drawable(int x, int y);
void        imlib_render_image_on_drawable_at_size(int x, int y, int width, int height);
void        imlib_render_image_part_on_drawable_at_size(int source_x, int source_y, int source_width, int source_height, int x, int y, int width, int height);
void        imlib_blend_image_onto_image(Imlib_Image source_image, char merge_alpha, int source_x, int source_y, int source_width, int source_height, int destination_x, int destination_y, int destination_width, int destination_height);
Imlib_Image imlib_create_image(int width, int height);
Imlib_Image imlib_create_image_using_data(int width, int height, DATA32 *data);
Imlib_Image imlib_create_image_using_copied_data(int width, int height, DATA32 *data);
Imlib_Image imlib_create_image_from_drawable(Pixmap mask, int x, int y, int width, int height, char need_to_grab_x);
Imlib_Image imlib_create_scaled_image_from_drawable(Pixmap mask, int source_x, int source_y, int source_width, int source_height, int destination_width, int destination_height, char need_to_grab_x, char get_mask_from_shape);
char        imlib_copy_drawable_to_image(Pixmap mask, int x, int y, int width, int height, int destination_x, int destination_y, char need_to_grab_x);
Imlib_Image imlib_clone_image(void);
Imlib_Image imlib_create_cropped_image(int x, int y, int width, int height);
Imlib_Image imlib_create_cropped_scaled_image(int source_x, int source_y, int source_width, int source_height, int destination_width, int destination_height);

Imlib_Updates imlib_updates_clone(Imlib_Updates updates);
Imlib_Updates imlib_update_append_rect(Imlib_Updates updates, int x, int y, int w, int h);
Imlib_Updates imlib_updates_merge(Imlib_Updates updates, int w, int h);
Imlib_Updates imlib_updates_merge_for_rendering(Imlib_Updates updates, int w, int h);
void          imlib_updates_free(Imlib_Updates updates);
Imlib_Updates imlib_updates_get_next(Imlib_Updates updates);
void          imlib_updates_get_coordinates(Imlib_Updates updates, int *x_return, int *y_return, int *width_return, int *height_return);
void          imlib_render_image_updates_on_drawable(Imlib_Updates updates, int x, int y);
Imlib_Updates imlib_updates_init(void);
Imlib_Updates imlib_updates_append_updates(Imlib_Updates updates, Imlib_Updates appended_updates);

void imlib_image_flip_horizontal(void);
void imlib_image_flip_vertical(void);
void imlib_image_flip_diagonal(void);
void imlib_image_blur(int radius);
void imlib_image_sharpen(int radius);
void imlib_image_tile_horizontal(void);
void imlib_image_tile_vertical(void);
void imlib_image_tile(void);

Imlib_Font imlib_load_font(char *font_name);
void       imlib_free_font(void);
void       imlib_text_draw(int x, int y, char *text);
void       imlib_text_draw_with_return_metrics(int x, int y, char *text, int *width_return, int *height_return, int *horizontal_advance_return, int *vertical_advance_return);
void       imlib_get_text_size(char *text, int *width_return, int *height_return);
void       imlib_add_path_to_font_path(char *path);
void       imlib_remove_path_from_font_path(char *path);
char     **imlib_list_font_path(int *number_return);
int        imlib_text_get_index_and_location(char *text, int x, int y, int *char_x_return, int *char_y_return, int *char_width_return, int *char_height_return);
char     **imlib_list_fonts(int *number_return);
void       imlib_free_font_list(char **font_list, int number);
int        imlib_get_font_cache_size(void);
void       imlib_set_font_cache_size(int bytes);
void       imlib_flush_font_cache(void);
int        imlib_get_font_ascent(void);
int        imlib_get_font_descent(void);
int        imlib_get_maximum_font_ascent(Imlib_Font font);
int        imlib_get_maximum_font_descent(Imlib_Font font);

Imlib_Color_Modifier imlib_create_color_modifier(void);
void                 imlib_free_color_modifier(void);
void                 imlib_modify_color_modifier_gamma(double gamma_value);
void                 imlib_modify_color_modifier_brightness(double brightness_value);
void                 imlib_modify_color_modifier_contrast(double contrast_value);
void                 imlib_set_color_modifier_tables(DATA8 *red_table, DATA8 *green_table, DATA8 *blue_table, DATA8 *alpha_table);
void                 imlib_get_color_modifier_tables(DATA8 *red_table, DATA8 *green_table, DATA8 *blue_table, DATA8 *alpha_table);
void                 imlib_reset_color_modifier(void);
void                 imlib_apply_color_modifier(void);
void                 imlib_apply_color_modifier_to_rectangle(int x, int y, int width, int height);

Imlib_Updates imlib_image_draw_line(int x1, int y1, int x2, int y2, char make_updates);
void imlib_image_draw_rectangle(int x, int y, int width, int height);
void imlib_image_fill_rectangle(int x, int y, int width, int height);
void imlib_image_copy_alpha_to_image(Imlib_Image image_source, int x, int y);
void imlib_image_copy_alpha_rectangle_to_image(Imlib_Image image_source, int x, int y, int width, int height, int destination_x, int destination_y);
void imlib_image_scroll_rect(int x, int y, int width, int height, int delta_x, int delta_y);
void imlib_image_copy_rect(int x, int y, int width, int height, int new_x,int new_y);

Imlib_Color_Range imlib_create_color_range(void);
void imlib_free_color_range(Imlib_Color_Range color_range);
void imlib_add_color_to_color_range(int distance_away);
void imlib_image_fill_color_range_rectangle(int x, int y, int width, int height, double angle);

void imlib_image_query_pixel(int x, int y, Imlib_Color *color_return);

void imlib_image_attach_data_value(char *key, void *data, int value, Imlib_Data_Destructor_Function destructor_function);
void *imlib_image_get_attached_data(char *key);
int imlib_image_get_attached_value(char *key);
void imlib_image_remove_attached_data_value(char *key);
void imlib_image_remove_and_free_attached_data_value(char *key);
void imlib_save_image(char *filename);
void imlib_save_image_with_error_return(char *filename, Imlib_Load_Error *error_return);

/* FIXME: */
/* need to add arbitary rotation routines */
/* need to add polygon fill code */

#endif
