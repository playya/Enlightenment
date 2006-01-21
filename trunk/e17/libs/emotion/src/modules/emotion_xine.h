#ifndef EMOTION_XINE_H
#define EMOTION_XINE_H

#include <xine.h>
#include <xine/xine_plugin.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

typedef struct _Emotion_Xine_Video       Emotion_Xine_Video;
typedef struct _Emotion_Xine_Video_Frame Emotion_Xine_Video_Frame;
typedef struct _Emotion_Xine_Event Emotion_Xine_Event;

struct _Emotion_Xine_Video
{
   xine_t                   *decoder;
   xine_video_port_t        *video;
   xine_audio_port_t        *audio;
   xine_stream_t            *stream;
   xine_event_queue_t       *queue;
   int                       fd;
   volatile double           len;
   volatile double           pos;
   double                    fps;
   double                    ratio;
   int                       w, h;
   Evas_Object              *obj;
   Emotion_Xine_Video_Frame *cur_frame;
   volatile int              seek_to;
   volatile int              get_poslen;
   volatile double           seek_to_pos;
   Ecore_Timer              *timer;
   int                       fd_read;
   int                       fd_write;
   Ecore_Fd_Handler         *fd_handler;
   int                       fd_ev_read;
   int                       fd_ev_write;
   Ecore_Fd_Handler         *fd_ev_handler;
   unsigned char             play : 1;
   unsigned char             just_loaded : 1;
   unsigned char             video_mute : 1;
   unsigned char             audio_mute : 1;
   unsigned char             spu_mute : 1;
   volatile unsigned char    delete_me : 1;
   volatile unsigned char    no_time : 1;
   
   pthread_t                 seek_th;
   pthread_t                 get_pos_len_th;
   pthread_cond_t            seek_cond;
   pthread_cond_t            get_pos_len_cond;
   pthread_mutex_t           seek_mutex;
   pthread_mutex_t           get_pos_len_mutex;
   unsigned char             seek_thread_deleted : 1;
   unsigned char             get_pos_thread_deleted : 1;
};

struct _Emotion_Xine_Video_Frame
{
   int            w, h;
   double         ratio;
   Emotion_Format format;
   unsigned char *y, *u, *v;
   unsigned char *bgra_data;
   int            y_stride, u_stride, v_stride;
   Evas_Object   *obj;
   double         timestamp;
   void         (*done_func)(void *data);
   void          *done_data;
   void          *frame;
};

struct _Emotion_Xine_Event
{
   int type;
   char *xine_event;
};

unsigned char         module_open(Evas_Object *obj, Emotion_Video_Module **module, void **video);
void                  module_close(Emotion_Video_Module *module, void *video);

#endif
