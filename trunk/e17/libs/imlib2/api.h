#ifndef __IMLIB_API_H
#define __IMLIB_API_H 1

#ifndef DATA64
#define DATA64 unsigned long long
#define DATA32 unsigned int
#define DATA16 unsigned short
#define DATA8  unsigned char
#endif

/* data types - guess what - no transparent datatypes - all hidden */
typedef void * Imlib_Image;
typedef struct _imlib_border Imlib_Border;

struct _imlib_border
{
   int left, right, top, bottom;
};

typedef void (*Imlib_Progress_Function)(Imlib_Image *im, char percent,
					int update_x, int update_y,
					int update_w, int update_h);

/* init and setup functions */
char        imlib_init(void);
int         imlib_get_cache_size(void);
void        imlib_set_cache_size(int bytes);
void        imlib_set_color_usage(int max);

/* image loading functions */
Imlib_Image imlib_load_image(char *file);
Imlib_Image imlib_load_image_with_progress_callback(char *file,
						    Imlib_Progress_Function progress_function,
						    char progress_granulatiy);
Imlib_Image imlib_load_image_immediately(char *file);
Imlib_Image imlib_load_image_without_cache(char *file);
Imlib_Image imlib_load_image_with_progress_callback_without_cache (char *file,
								   Imlib_Progress_Function progress_function,
								   char progress_granulatiy);
Imlib_Image imlib_load_image_immediately_without_cache(char *file);

/* image information retrieval and basic manipulation functions */
int     imlib_get_image_width(Imlib_Image image);
int     imlib_get_image_height(Imlib_Image image);
DATA32 *imlib_get_image_data(Imlib_Image image);
void    imlib_put_back_image_data(Imlib_Image image);
char    imlib_image_has_alpha(Imlib_Image image);
void    imlib_set_image_never_changes_on_disk(Imlib_Image image);
void    imlib_image_get_border(Imlib_Image image, Imlib_Border *border);
void    imlib_image_set_border(Imlib_Image image, Imlib_Border *border);
char   *imlib_image_format(Imlib_Image image);

/* image drawing/rendering functions */
#endif
