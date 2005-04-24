#include "eclair_callbacks.h"
#include <Emotion.h>
#include <Esmart/Esmart_Container.h>
#include <gtk/gtk.h>
#include "eclair.h"

//Called when eclair is closed
int eclair_exit_cb(void *data, int type, void *event)
{
   eclair_shutdown((Eclair *)data);

   return 1;
}

//Called when a new frame is decoded
void eclair_video_frame_decode_cb(void *data, Evas_Object *obj, void *event_info)
{
   eclair_update((Eclair *)data);
}

//Called when the current media playback is finished
void eclair_video_playback_finished_cb(void *data, Evas_Object *obj, void *event_info)
{
   eclair_play_next((Eclair *)data);
}

//Called when the audio volume is changed by an external application
void eclair_video_audio_level_change_cb(void *data, Evas_Object *obj, void *event_info)
{
   Eclair *eclair = (Eclair *)data;

   if (!eclair)
      return;
   if (!eclair->gui_object || !eclair->video_object)
      return;

   edje_object_part_drag_value_set(eclair->gui_object, "volume_bar_drag", emotion_object_audio_volume_get(eclair->video_object), 0);
}

//Called when the video window is resized:
//Resize the video object and the black background object
void eclair_video_window_resize_cb(Ecore_Evas *window)
{
   Eclair *eclair;
   Evas_Coord window_width, window_height, video_width, video_height, X, Y;
   double ratio;

   eclair = (Eclair *)ecore_evas_data_get(window, "eclair");
   if (!eclair)
      return;

   evas_output_viewport_get(ecore_evas_get(window), NULL, NULL, &window_width, &window_height);

   if (eclair->video_object)
   {
      ratio = emotion_object_ratio_get(eclair->video_object);
      //FIXME: emotion: ratio is sometimes 0?!
      if (ratio <= -0.01 || ratio >= 0.01)
      {
         if (window_width / ratio > window_height)
         {
            video_height = window_height;
            video_width = video_height * ratio;
         }
         else
         {
            video_width = window_width;
            video_height = video_width / ratio;
         }
      
         X = (window_width - video_width) / 2.0;
         Y = (window_height - video_height) / 2.0;
      }
      else
      {
         video_width = window_width;
         video_height = window_height;
         X = Y = 0;
      }   
      evas_object_move(eclair->video_object, X, Y);
      evas_object_resize(eclair->video_object, video_width, video_height);
   }
   
   if (eclair->black_background)
   {
      evas_object_move(eclair->black_background, 0, 0);
      evas_object_resize(eclair->black_background, window_width, window_height);
   }
}


//Called when a key is pressed
void eclair_key_press_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	Eclair *eclair = (Eclair *)data;
   Evas_Event_Key_Down *ev = (Evas_Event_Key_Down *)event_info;

   if (!eclair)
      return;

   if (strcmp(ev->keyname, "f") == 0)
   {
      if (!eclair->video_window)
         return;

      ecore_evas_fullscreen_set(eclair->video_window, !ecore_evas_fullscreen_get(eclair->video_window));
   }
   else if (strcmp(ev->keyname, "space") == 0)
   {
      if (eclair->state == ECLAIR_PLAYING)
         eclair_pause(eclair);
      else
         eclair_play(eclair);
   }
   else if (strcmp(ev->keyname, "escape") == 0)
      eclair_shutdown(eclair);
   else if (strcmp(ev->keyname, "Left") == 0 || strcmp(ev->keyname, "Right") == 0
      || strcmp(ev->keyname, "Down") == 0 || strcmp(ev->keyname, "Up") == 0)
   {
      if (!eclair->video_object)
         return;

      if (eclair->seek_to_pos < 0.0)
         eclair->seek_to_pos = emotion_object_position_get(eclair->video_object);
   
      if (strcmp(ev->keyname, "Left") == 0)
         eclair->seek_to_pos -= 15.0;
      else if (strcmp(ev->keyname, "Right") == 0)
         eclair->seek_to_pos += 15.0;
      else if (strcmp(ev->keyname, "Down") == 0)
         eclair->seek_to_pos -= 60.0;
      else if (strcmp(ev->keyname, "Up") == 0)
         eclair->seek_to_pos += 60.0;

      eclair->dont_update_progressbar = 1;
      emotion_object_position_set(eclair->video_object, eclair->seek_to_pos);
      if (eclair->seek_to_pos < 0.0)
         eclair->seek_to_pos = 0.0;
      if (eclair->seek_to_pos > emotion_object_play_length_get(eclair->video_object))
         eclair->seek_to_pos = emotion_object_play_length_get(eclair->video_object);
   }
}

//Called when the video window is closed
void eclair_video_window_close_cb(Ecore_Evas *window)
{
   eclair_stop((Eclair *)ecore_evas_data_get(window, "eclair"));
}

//Called when the user clicks on open button
void eclair_gui_open_cb(void *data, Evas_Object *edje_object, const char *emission, const char *source)
{
   Eclair *eclair = (Eclair *)data;

   if (eclair->file_chooser_th_created)
   {
      if (eclair->file_chooser_widget)
      {
         //TODO: make sure that it also raises the file dialog on other WMs than e17
         gtk_window_deiconify(GTK_WINDOW(eclair->file_chooser_widget));
      }
   }
   else
   {
      eclair->file_chooser_th_created = 1;
      pthread_mutex_init(&eclair->file_chooser_mutex, NULL);
      pthread_create(&eclair->file_chooser_thread, NULL, eclair_file_chooser_thread, eclair);
   }
}

//Called when the user clicks on play button
void eclair_gui_play_cb(void *data, Evas_Object *edje_object, const char *emission, const char *source)
{
   eclair_play((Eclair *)data);
}

//Called when the user clicks on prev button
void eclair_gui_prev_cb(void *data, Evas_Object *edje_object, const char *emission, const char *source)
{
   eclair_play_prev((Eclair *)data);
}

//Called when the user clicks on next button
void eclair_gui_next_cb(void *data, Evas_Object *edje_object, const char *emission, const char *source)
{
   eclair_play_next((Eclair *)data);
}

//Called when the user clicks on pause button
void eclair_gui_pause_cb(void *data, Evas_Object *edje_object, const char *emission, const char *source)
{
   eclair_pause((Eclair *)data);
}

//Called when the user clicks on stop button
void eclair_gui_stop_cb(void *data, Evas_Object *edje_object, const char *emission, const char *source)
{
   eclair_stop((Eclair *)data);
}

//Called when the user clicks on close button
void eclair_gui_close_cb(void *data, Evas_Object *edje_object, const char *emission, const char *source)
{
   eclair_shutdown((Eclair *)data);
}

//Called when the user clicks on minimize button
void eclair_gui_minimize_cb(void *data, Evas_Object *edje_object, const char *emission, const char *source)
{
   Eclair *eclair = (Eclair *)data;
   
   if (!eclair)
      return;
   if (!eclair->gui_window)
      return;

   ecore_evas_iconified_set(eclair->gui_window, 1);
}

//Called when the user activates an entry in the playlist
void eclair_gui_play_entry_cb(void *data, Evas_Object *edje_object, const char *emission, const char *source)
{
   Eclair *eclair = (Eclair *)data;
   Eclair_Playlist_Media_File *media_file = evas_object_data_get(edje_object, "media_file");

   if (!eclair)
      return;

   eclair_playlist_current_set(&eclair->playlist, media_file);
   eclair_play_current(eclair);
}

//Called when the user drag the progress bar button
void eclair_gui_progress_bar_drag_cb(void *data, Evas_Object *edje_object, const char *emission, const char *source)
{
   Eclair *eclair = (Eclair *)data;
   double x;

   if (!eclair)
      return;

   edje_object_part_drag_value_get(eclair->gui_object, "progress_bar_drag", &x, NULL);
   eclair_progress_rate_set(eclair, x);
   eclair->dont_update_progressbar = 1;
}

//Called when the user drags the volume bar button
void eclair_gui_volume_bar_cb(void *data, Evas_Object *edje_object, const char *emission, const char *source)
{
   Eclair *eclair = (Eclair *)data;
   double x;

   if (!eclair)
      return;

   edje_object_part_drag_value_get(eclair->gui_object, "volume_bar_drag", &x, NULL);
   emotion_object_audio_volume_set(eclair->video_object, x);
}

//Called when the user drag the scrollbar button of the playlist
void eclair_gui_playlist_scrollbar_button_drag_cb(void *data, Evas_Object *edje_object, const char *emission, const char *source)
{
   Eclair *eclair = (Eclair *)data;
   double y;

   if (!eclair)
      return;  
   if (!eclair->gui_object)
      return;

   edje_object_part_drag_value_get(eclair->gui_object, "playlist_scrollbar_button", NULL, &y);
   eclair_playlist_container_scroll_percent_set(eclair, y);
}

//Called when the user wants to scroll the playlist
void eclair_gui_playlist_scroll_cb(void *data, Evas_Object *edje_object, const char *emission, const char *source)
{
   Eclair *eclair = (Eclair *)data;

   if (!eclair->playlist_container)
      return;

   if (strcmp(emission, "playlist_scroll_down_start") == 0)
      esmart_container_scroll_start(eclair->playlist_container, -1.0);
   else if (strcmp(emission, "playlist_scroll_up_start") == 0)
      esmart_container_scroll_start(eclair->playlist_container, 1.0);
   else
      esmart_container_scroll_stop(eclair->playlist_container);
}

//Called when user uses wheel mouse over playlist container
void eclair_gui_playlist_container_wheel_cb(void *data, Evas *evas, Evas_Object *playlist_container, void *event_info)
{
   Eclair *eclair = (Eclair *)data;
   Evas_Event_Mouse_Wheel *event = (Evas_Event_Mouse_Wheel *)event_info;

   eclair_playlist_container_scroll(eclair, event->z);  
}
