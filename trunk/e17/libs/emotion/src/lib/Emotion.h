#ifndef EMOTION_H
#define EMOTION_H

#include <Evas.h>
#include <Ecore.h>

enum _Emotion_Event
{
   EMOTION_EVENT_MENU1, // Escape Menu
   EMOTION_EVENT_MENU2, // Title Menu
   EMOTION_EVENT_MENU3, // Root Menu
   EMOTION_EVENT_MENU4, // Subpicture Menu
   EMOTION_EVENT_MENU5, // Audio Menu
   EMOTION_EVENT_MENU6, // Angle Menu
   EMOTION_EVENT_MENU7, // Part Menu
   EMOTION_EVENT_UP,
   EMOTION_EVENT_DOWN,
   EMOTION_EVENT_LEFT,
   EMOTION_EVENT_RIGHT,
   EMOTION_EVENT_SELECT,
   EMOTION_EVENT_NEXT,
   EMOTION_EVENT_PREV,
   EMOTION_EVENT_ANGLE_NEXT,
   EMOTION_EVENT_ANGLE_PREV,
   EMOTION_EVENT_FORCE,
   EMOTION_EVENT_0,
   EMOTION_EVENT_1,
   EMOTION_EVENT_2,
   EMOTION_EVENT_3,
   EMOTION_EVENT_4,
   EMOTION_EVENT_5,
   EMOTION_EVENT_6,
   EMOTION_EVENT_7,
   EMOTION_EVENT_8,
   EMOTION_EVENT_9,
   EMOTION_EVENT_10
};

typedef enum _Emotion_Event Emotion_Event;

#define EMOTION_CHANNEL_AUTO -1
#define EMOTION_CHANNEL_DEFAULT 0

/* api calls available */
Evas_Object *emotion_object_add                   (Evas *evas);
void         emotion_object_file_set              (Evas_Object *obj, const char *file);
const char  *emotion_object_file_get              (Evas_Object *obj);
void         emotion_object_play_set              (Evas_Object *obj, Evas_Bool play);
Evas_Bool    emotion_object_play_get              (Evas_Object *obj);
void         emotion_object_position_set          (Evas_Object *obj, double sec);
double       emotion_object_position_get          (Evas_Object *obj);
Evas_Bool    emotion_object_seekable_get          (Evas_Object *obj);
double       emotion_object_play_length_get       (Evas_Object *obj);
void         emotion_object_size_get              (Evas_Object *obj, int *iw, int *ih);
void         emotion_object_smooth_scale_set      (Evas_Object *obj, Evas_Bool smooth);
Evas_Bool    emotion_object_smooth_scale_get      (Evas_Object *obj);
double       emotion_object_ratio_get             (Evas_Object *obj);
void         emotion_object_event_simple_send     (Evas_Object *obj, Emotion_Event ev);
void         emotion_object_audio_volume_set      (Evas_Object *obj, double vol);
double       emotion_object_audio_volume_get      (Evas_Object *obj);
void         emotion_object_audio_mute_set        (Evas_Object *obj, Evas_Bool mute);
Evas_Bool    emotion_object_audio_mute_get        (Evas_Object *obj);
int          emotion_object_audio_channel_count   (Evas_Object *obj);
const char  *emotion_object_audio_channel_name_get(Evas_Object *obj, int channel);
void         emotion_object_audio_channel_set     (Evas_Object *obj, int channel);
int          emotion_object_audio_channel_get     (Evas_Object *obj);
void         emotion_object_video_mute_set        (Evas_Object *obj, Evas_Bool mute);
Evas_Bool    emotion_object_video_mute_get        (Evas_Object *obj);
int          emotion_object_video_channel_count   (Evas_Object *obj);
const char  *emotion_object_video_channel_name_get(Evas_Object *obj, int channel);
void         emotion_object_video_channel_set     (Evas_Object *obj, int channel);
int          emotion_object_video_channel_get     (Evas_Object *obj);
void         emotion_object_spu_mute_set          (Evas_Object *obj, Evas_Bool mute);
Evas_Bool    emotion_object_spu_mute_get          (Evas_Object *obj);
int          emotion_object_spu_channel_count     (Evas_Object *obj);
const char  *emotion_object_spu_channel_name_get  (Evas_Object *obj, int channel);
void         emotion_object_spu_channel_set       (Evas_Object *obj, int channel);
int          emotion_object_spu_channel_get       (Evas_Object *obj);
int          emotion_object_chapter_count         (Evas_Object *obj);
void         emotion_object_chapter_set           (Evas_Object *obj, int chapter);
int          emotion_object_chapter_get           (Evas_Object *obj);
const char  *emotion_object_chapter_name_get      (Evas_Object *obj, int chapter);
void         emotion_object_play_speed_set        (Evas_Object *obj, double speed);
double       emotion_object_play_speed_get        (Evas_Object *obj);
void         emotion_object_eject                 (Evas_Object *obj);
const char  *emotion_object_title_get             (Evas_Object *obj);
const char  *emotion_object_progress_info_get     (Evas_Object *obj);
double       emotion_object_progress_status_get   (Evas_Object *obj);
const char  *emotion_object_ref_file_get          (Evas_Object *obj);
int          emotion_object_ref_num_get           (Evas_Object *obj);
int          emotion_object_spu_button_count_get  (Evas_Object *obj);
int          emotion_object_spu_button_get        (Evas_Object *obj);
    
#endif
