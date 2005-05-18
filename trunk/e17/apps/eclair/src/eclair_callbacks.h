#ifndef _ECLAIR_CALLBACKS_H_
#define _ECLAIR_CALLBACKS_H_

#include "eclair_private.h"

int eclair_exit_cb(void *data, int type, void *event);
void eclair_key_press_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info);

void eclair_video_window_resize_cb(Ecore_Evas *window);
void eclair_video_window_close_cb(Ecore_Evas *window);
void eclair_video_frame_decode_cb(void *data, Evas_Object *obj, void *event_info);
void eclair_video_playback_finished_cb(void *data, Evas_Object *obj, void *event_info);
void eclair_video_audio_level_change_cb(void *data, Evas_Object *obj, void *event_info);

void eclair_gui_open_cb(void *data, Evas_Object *edje_object, const char *emission, const char *source);
void eclair_gui_close_cb(void *data, Evas_Object *edje_object, const char *emission, const char *source);
void eclair_gui_minimize_cb(void *data, Evas_Object *edje_object, const char *emission, const char *source);
void eclair_gui_play_cb(void *data, Evas_Object *edje_object, const char *emission, const char *source);
void eclair_gui_pause_cb(void *data, Evas_Object *edje_object, const char *emission, const char *source);
void eclair_gui_stop_cb(void *data, Evas_Object *edje_object, const char *emission, const char *source);
void eclair_gui_prev_cb(void *data, Evas_Object *edje_object, const char *emission, const char *source);
void eclair_gui_next_cb(void *data, Evas_Object *edje_object, const char *emission, const char *source);
void eclair_gui_entry_down_cb(void *data, Evas *evas, Evas_Object *entry, void *event_info);
void eclair_gui_progress_bar_drag_cb(void *data, Evas_Object *edje_object, const char *emission, const char *source);
void eclair_gui_volume_bar_cb(void *data, Evas_Object *edje_object, const char *emission, const char *source);
void eclair_gui_playlist_scrollbar_button_drag_cb(void *data, Evas_Object *edje_object, const char *emission, const char *source);
void eclair_gui_playlist_container_wheel_cb(void *data, Evas *evas, Evas_Object *playlist_container, void *event_info);
void eclair_gui_playlist_container_scroll_percent_changed(void *data, Evas_Object *obj, void *event_info);
void eclair_gui_playlist_scroll_cb(void *data, Evas_Object *edje_object, const char *emission, const char *source);
int eclair_gui_dnd_position_cb(void *data, int type, void *event);
int eclair_gui_dnd_drop_cb(void *data, int type, void *event);
int eclair_gui_dnd_selection_cb(void *data, int type, void *event);
int eclair_mouse_up_cb(void *data, int type, void *event);
void eclair_gui_message_cb(void *data, Evas_Object *obj, Edje_Message_Type type, int id, void *msg);

#endif
