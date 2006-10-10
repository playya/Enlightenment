#ifndef EVAS_ENGINE_H
#define EVAS_ENGINE_H

#include <sys/ipc.h>
#include <sys/shm.h>

#include <xcb/xcb.h>
#include <xcb/xcb_image.h>

typedef struct _Outbuf                Outbuf;
typedef struct _Outbuf_Perf           Outbuf_Perf;
typedef struct _Outbuf_Region         Outbuf_Region;
typedef struct _Xcb_Output_Buffer     Xcb_Output_Buffer;

typedef enum   _Outbuf_Depth          Outbuf_Depth;

enum _Outbuf_Depth
{
   OUTBUF_DEPTH_NONE,
   OUTBUF_DEPTH_INHERIT,
   OUTBUF_DEPTH_RGB_16BPP_565_565_DITHERED,
   OUTBUF_DEPTH_RGB_16BPP_555_555_DITHERED,
   OUTBUF_DEPTH_RGB_16BPP_444_444_DITHERED,
   OUTBUF_DEPTH_RGB_16BPP_565_444_DITHERED,
   OUTBUF_DEPTH_RGB_32BPP_888_8888,
   OUTBUF_DEPTH_LAST
};

struct _Outbuf
{
   Outbuf_Depth    depth;
   int             w, h;
   int             rot;
   Outbuf_Perf    *perf;

   struct {
      Convert_Pal *pal;
      struct {
         xcb_connection_t  *conn;
	 xcb_drawable_t     win;
	 xcb_drawable_t     mask;
	 xcb_visualtype_t  *vis;
	 xcb_colormap_t     cmap;
	 int                depth;
	 int                shm;
	 xcb_gcontext_t     gc;
	 xcb_gcontext_t     gcm;
	 int                swap : 1;
	 unsigned char      bit_swap : 1;
      } x;
      struct {
	 DATA32    r, g, b;
      } mask;
      /* lets not do back buf for now */
      /* RGBA_Image  *back_buf; */

      /* a list of pending regions to write to the target */
      Evas_List   *pending_writes;

      int          mask_dither : 1;
      int          destination_alpha : 1;

      int          debug : 1;
   } priv;
};

struct _Outbuf_Perf
{
   struct {
      xcb_connection_t  *conn;
      xcb_drawable_t     root;

      char *display;
      char *vendor;
      int   version;
      int   revision;
      int   release;
      int   w, h;
      int   screen_count;
      int   depth;
      int   screen_num;
   } x;
   struct{
      char *name;
      char *version;
      char *machine;
   } os;
   struct {
      char *info;
   } cpu;

   int   min_shm_image_pixel_count;
};

struct _Outbuf_Region
{
   Xcb_Output_Buffer *xcbob, *mxcbob;
   int x, y, w, h;
};

struct _Xcb_Output_Buffer
{
   xcb_connection_t       *connection;
   xcb_image_t            *image;
   xcb_shm_segment_info_t *shm_info;
   void                   *data;
};


/****/
/* main */
void               evas_software_xcb_x_init                    (void);

/* buffer */
void               evas_software_xcb_x_write_mask_line         (Outbuf            *buf,
                                                                Xcb_Output_Buffer *xcbob,
								DATA32            *src,
								int                w,
								int                y);
int                evas_software_xcb_x_can_do_shm              (xcb_connection_t *c);
Xcb_Output_Buffer *evas_software_xcb_x_output_buffer_new       (xcb_connection_t *c,
								int            depth,
								int            w,
								int            h,
								int            try_shm,
								void          *data);
void               evas_software_xcb_x_output_buffer_free      (Xcb_Output_Buffer *xcbob,
								int                sync);
void               evas_software_xcb_x_output_buffer_paste     (Xcb_Output_Buffer *xcbob,
								xcb_drawable_t        d,
								xcb_gcontext_t        gc,
								int                x,
								int                y,
								int                sync);
DATA8             *evas_software_xcb_x_output_buffer_data      (Xcb_Output_Buffer *xcbob,
								int               *bytes_per_line_ret);
int                evas_software_xcb_x_output_buffer_depth     (Xcb_Output_Buffer *xcbob);
int                evas_software_xcb_x_output_buffer_byte_order(Xcb_Output_Buffer *xcbob);
int                evas_software_xcb_x_output_buffer_bit_order (Xcb_Output_Buffer *xcbob);


/* color */
void         evas_software_xcb_x_color_init       (void);
Convert_Pal *evas_software_xcb_x_color_allocate   (xcb_connection_t   *conn,
						   xcb_colormap_t      cmap,
						   xcb_visualtype_t   *vis,
						   Convert_Pal_Mode    colors);
void         evas_software_xcb_x_color_deallocate (xcb_connection_t *conn,
						   xcb_colormap_t    cmap,
						   xcb_visualtype_t *vis,
						   Convert_Pal      *pal);

/* outbuf */
void         evas_software_xcb_outbuf_init                   (void);
void         evas_software_xcb_outbuf_free                   (Outbuf *buf);
Outbuf      *evas_software_xcb_outbuf_setup_x                (int               w,
							      int               h,
							      int               rot,
							      Outbuf_Depth      depth,
							      xcb_connection_t *conn,
							      xcb_drawable_t    draw,
							      xcb_visualtype_t *vis,
							      xcb_colormap_t    cmap,
							      int               x_depth,
							      Outbuf_Perf      *perf,
							      int               grayscale,
							      int               max_colors,
							      xcb_drawable_t    mask,
							      int               shape_dither,
							      int               destination_alpha);

char        *evas_software_xcb_outbuf_perf_serialize_x       (Outbuf_Perf *perf);
void         evas_software_xcb_outbuf_perf_deserialize_x     (Outbuf_Perf *perf,
							      const char *data);
Outbuf_Perf *evas_software_xcb_outbuf_perf_new_x             (xcb_connection_t *conn,
							      xcb_drawable_t    draw,
                                                              xcb_visualtype_t *vis,
							      xcb_colormap_t    cmap,
							      int               x_depth);

char        *evas_software_xcb_outbuf_perf_serialize_info_x  (Outbuf_Perf *perf);
void         evas_software_xcb_outbuf_perf_store_x           (Outbuf_Perf *perf);
Outbuf_Perf *evas_software_xcb_outbuf_perf_restore_x         (xcb_connection_t *conn,
							      xcb_drawable_t    draw,
							      xcb_visualtype_t *vis,
							      xcb_colormap_t    cmap,
							      int               x_depth);
void         evas_software_xcb_outbuf_perf_free              (Outbuf_Perf *perf);
Outbuf_Perf *evas_software_xcb_outbuf_perf_x                 (xcb_connection_t *conn,
							      xcb_drawable_t    draw,
							      xcb_visualtype_t *vis,
							      xcb_colormap_t    cmap,
							      int               x_depth);

void         evas_software_xcb_outbuf_blit                   (Outbuf *buf,
							      int     src_x,
							      int     src_y,
							      int     w,
							      int     h,
							      int     dst_x,
							      int     dst_y);
void         evas_software_xcb_outbuf_update                 (Outbuf *buf,
							      int     x,
							      int     y,
							      int     w,
							      int     h);
RGBA_Image  *evas_software_xcb_outbuf_new_region_for_update  (Outbuf *buf,
							      int     x,
							      int     y,
							      int     w,
							      int     h,
							      int    *cx,
							      int    *cy,
							      int    *cw,
							      int    *ch);
void         evas_software_xcb_outbuf_free_region_for_update (Outbuf    *buf,
							      RGBA_Image *update);
void         evas_software_xcb_outbuf_flush                  (Outbuf *buf);
void         evas_software_xcb_outbuf_push_updated_region    (Outbuf     *buf,
							      RGBA_Image *update,
							      int         x,
							      int         y,
							      int         w,
							      int         h);
void         evas_software_xcb_outbuf_reconfigure            (Outbuf      *buf,
							      int          w,
							      int          h,
							      int          rot,
							      Outbuf_Depth depth);
int          evas_software_xcb_outbuf_get_width              (Outbuf *buf);
int          evas_software_xcb_outbuf_get_height             (Outbuf *buf);
Outbuf_Depth evas_software_xcb_outbuf_get_depth              (Outbuf *buf);
int          evas_software_xcb_outbuf_get_rot                (Outbuf *buf);
int          evas_software_xcb_outbuf_get_have_backbuf       (Outbuf *buf);
void         evas_software_xcb_outbuf_set_have_backbuf       (Outbuf *buf, int have_backbuf);
void         evas_software_xcb_outbuf_drawable_set           (Outbuf *buf, xcb_drawable_t draw);
void         evas_software_xcb_outbuf_mask_set               (Outbuf *buf, xcb_drawable_t mask);
void         evas_software_xcb_outbuf_rotation_set           (Outbuf *buf, int rot);

void         evas_software_xcb_outbuf_debug_set              (Outbuf *buf, int debug);
void         evas_software_xcb_outbuf_debug_show             (Outbuf        *buf,
							      xcb_drawable_t draw,
							      int            x,
							      int            y,
							      int            w,
							      int            h);

#endif /* EVAS_ENGINE_H */
