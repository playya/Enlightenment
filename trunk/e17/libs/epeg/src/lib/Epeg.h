#ifndef _EPEG_H
#define _EPEG_H

#ifdef __cplusplus
extern "C" {
#endif

   typedef struct _Epeg_Image          Epeg_Image;
   typedef struct _Epeg_Thumbnail_Info Epeg_Thumbnail_Info;
   
   struct _Epeg_Thumbnail_Info
     {
	char                   *uri;
	unsigned long long int  mtime;
	int                     w, h;
	char                   *mimetype;
     };
   
   Epeg_Image   *epeg_file_open                 (const char *file);
   Epeg_Image   *epeg_memory_open               (unsigned char *data, int size);
   void          epeg_size_get                  (Epeg_Image *im, int *w, int *h);
   void          epeg_decode_size_set           (Epeg_Image *im, int w, int h);
   const char   *epeg_comment_get               (Epeg_Image *im);
   void          epeg_thumbnail_comments_get    (Epeg_Image *im, Epeg_Thumbnail_Info *info);
   void          epeg_comment_set               (Epeg_Image *im, const char *comment);
   void          epeg_quality_set               (Epeg_Image *im, int quality);
   void          epeg_thumbnail_comments_enable (Epeg_Image *im, int onoff);
   void          epeg_file_output_set           (Epeg_Image *im, const char *file);
   void          epeg_memory_output_set         (Epeg_Image *im, unsigned char **data, int *size);
   void          epeg_encode                    (Epeg_Image *im);
   void          epeg_close                     (Epeg_Image *im);
   
#ifdef __cplusplus
}
#endif

#endif
