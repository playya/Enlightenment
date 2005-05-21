#include "eclair_callbacks.h"
#include <string.h>
#include <Emotion.h>
#include <Edje.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Esmart/Esmart_Container.h>
#include <gtk/gtk.h>
#include <pthread.h>
#include "eclair.h"
#include "eclair_playlist.h"
#include "eclair_playlist_container.h"
#include "eclair_cover.h"
#include "eclair_dialogs.h"

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

   if (!eclair || !eclair->gui_object || !eclair->video_object)
      return;

   edje_object_part_drag_value_set(eclair->gui_object, "volume_bar_drag", emotion_object_audio_volume_get(eclair->video_object), 0);
}

//Called when the video window is resized:
//Resize the video object and the black background object
void eclair_video_window_resize_cb(Ecore_Evas *window)
{
   Eclair *eclair = (Eclair *)ecore_evas_data_get(window, "eclair");
   Evas_Coord window_width, window_height, video_width, video_height, X, Y;
   double ratio;

   if (!eclair)
      return;

   evas_output_viewport_get(ecore_evas_get(window), NULL, NULL, &window_width, &window_height);

   if (eclair->video_object)
   {
      ratio = emotion_object_ratio_get(eclair->video_object);
      //TODO: emotion bug? ratio is sometimes 0?!
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

   if (strcmp(ev->key, "f") == 0)
   {
      if (eclair->video_window)
         ecore_evas_fullscreen_set(eclair->video_window, !ecore_evas_fullscreen_get(eclair->video_window));
   }
   else if (strcmp(ev->key, "space") == 0 || strcmp(ev->key, "p") == 0)
   {
      if (eclair->state == ECLAIR_PLAYING)
         eclair_pause(eclair);
      else
         eclair_play(eclair);
   }
   else if (strcmp(ev->key, "escape") == 0 || strcmp(ev->key, "q") == 0)
      eclair_shutdown(eclair);
   else if (strcmp(ev->key, "Left") == 0 || strcmp(ev->key, "Right") == 0
      || strcmp(ev->key, "Down") == 0 || strcmp(ev->key, "Up") == 0
      || strcmp(ev->key, "Prior") == 0 || strcmp(ev->key, "Next") == 0
      || strcmp(ev->key, "Home") == 0)
   {
      double media_position;

      if (!eclair->video_object)
         return;

      media_position = eclair_position_get(eclair);   
      if (strcmp(ev->key, "Left") == 0)
         media_position -= 15.0;
      else if (strcmp(ev->key, "Right") == 0)
         media_position += 15.0;
      else if (strcmp(ev->key, "Down") == 0)
         media_position -= 60.0;
      else if (strcmp(ev->key, "Up") == 0)
         media_position += 60.0;
      else if (strcmp(ev->key, "Next") == 0)
         media_position -= 600.0;
      else if (strcmp(ev->key, "Prior") == 0)
         media_position += 600.0;
      else if (strcmp(ev->key, "Home") == 0)
         media_position = 0.0;
      eclair_position_set(eclair, media_position);
   }
   else if (strcmp(ev->key, "KP_Divide") == 0 || strcmp(ev->key, "slash") == 0 
      || strcmp(ev->key, "KP_Multiply") == 0 || strcmp(ev->key, "asterisk") == 0)
   {
      double volume;

      if (!eclair->gui_object)
         return;

      edje_object_part_drag_value_get(eclair->gui_object, "volume_bar_drag", &volume, 0);
      if (strcmp(ev->key, "KP_Divide") == 0 || strcmp(ev->key, "slash") == 0)
         volume -= (1.0 / 100);
      else if (strcmp(ev->key, "KP_Multiply") == 0 || strcmp (ev->key, "asterisk") == 0)
         volume += (1.0 / 100);

      if (volume < 0.0)
         volume = 0.0;
      else if (volume > 1.0)
         volume = 1.0;
      eclair_audio_level_set(eclair, volume);
   }
   else if (strcmp(ev->key, "Delete") == 0)
   {
      eclair_playlist_remove_selected_media_files(&eclair->playlist);
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

   if (!eclair)
      return;

   eclair_dialogs_add_files_file_chooser(&eclair->dialogs_manager);
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
   
   if (!eclair || !eclair->gui_window)
      return;

   ecore_evas_iconified_set(eclair->gui_window, 1);
}

//Called when the user clicks on an entry in the playlist 
void eclair_gui_entry_down_cb(void *data, Evas *evas, Evas_Object *entry, void *event_info)
{
   Eclair *eclair = (Eclair *)data;
   Eclair_Media_File *media_file = evas_object_data_get(entry, "media_file");
   Evas_Event_Mouse_Down *event = (Evas_Event_Mouse_Down *)event_info;

   if (!eclair || !media_file || event->button != 1)
      return;

   if (event->flags == EVAS_BUTTON_NONE)
      eclair_playlist_container_select_file(eclair->playlist_container, media_file, event->modifiers);
   else if (event->flags == EVAS_BUTTON_DOUBLE_CLICK)
   {
      eclair_playlist_current_set(&eclair->playlist, media_file);
      eclair_play_current(eclair);
   }
}

//Called when the user drag the progress bar button
void eclair_gui_progress_bar_drag_cb(void *data, Evas_Object *edje_object, const char *emission, const char *source)
{
   Eclair *eclair = (Eclair *)data;
   double progress_rate;

   if (!eclair)
      return;

   if (strcmp(emission, "drag,start") == 0)
      eclair->use_progress_bar_drag_for_time = 1;      
   else if (strcmp(emission, "drag,stop") == 0)
   {
      eclair->use_progress_bar_drag_for_time = 0;
      edje_object_part_drag_value_get(eclair->gui_object, "progress_bar_drag", &progress_rate, NULL);
      eclair_progress_rate_set(eclair, progress_rate);
   }
}

//Called when the user drags the volume bar button
void eclair_gui_volume_bar_cb(void *data, Evas_Object *edje_object, const char *emission, const char *source)
{
   Eclair *eclair = (Eclair *)data;
   double volume;

   if (!eclair)
      return;

   edje_object_part_drag_value_get(eclair->gui_object, "volume_bar_drag", &volume, NULL);
   eclair_audio_level_set(eclair, volume);
}

//Called when the user drag the scrollbar button of the playlist
void eclair_gui_playlist_scrollbar_button_drag_cb(void *data, Evas_Object *edje_object, const char *emission, const char *source)
{
   Eclair *eclair = (Eclair *)data;
   double y;

   if (!eclair || !eclair->gui_object)
      return;

   edje_object_part_drag_value_get(eclair->gui_object, "playlist_scrollbar_button", NULL, &y);
   eclair_playlist_container_scroll_percent_set(eclair->playlist_container, y);
}

//Called when the scroll percent of the playlist container is changed
void eclair_gui_playlist_container_scroll_percent_changed(void *data, Evas_Object *obj, void *event_info)
{
   Eclair *eclair = (Eclair *)data;

   if (!eclair || !eclair->gui_object)
      return;

   edje_object_part_drag_value_set(eclair->gui_object, "playlist_scrollbar_button", 0, eclair_playlist_container_scroll_percent_get(obj));
}

//Called when the user wants to scroll the playlist
void eclair_gui_playlist_scroll_cb(void *data, Evas_Object *edje_object, const char *emission, const char *source)
{
   Eclair *eclair = (Eclair *)data;

   if (!eclair || !eclair->playlist_container)
      return;

   if (strcmp(emission, "playlist_scroll_down_start") == 0)
      eclair_playlist_container_scroll_start(eclair->playlist_container, 2.0);
   else if (strcmp(emission, "playlist_scroll_up_start") == 0)
      eclair_playlist_container_scroll_start(eclair->playlist_container, -2.0);
   else
      eclair_playlist_container_scroll_stop(eclair->playlist_container);
}

//Called when user uses wheel mouse over playlist container
void eclair_gui_playlist_container_wheel_cb(void *data, Evas *evas, Evas_Object *playlist_container, void *event_info)
{
   Eclair *eclair = (Eclair *)data;
   Evas_Event_Mouse_Wheel *event = (Evas_Event_Mouse_Wheel *)event_info;

   eclair_playlist_container_scroll(eclair->playlist_container, event->z);
}

//Called when an object is dragged over the gui
int eclair_gui_dnd_position_cb(void *data, int type, void *event)
{
   Evas *evas;
   Evas_Coord gui_window_x, gui_window_y, x, y;
   Ecore_X_Rectangle rect;
   Ecore_X_Event_Xdnd_Position *ev = (Ecore_X_Event_Xdnd_Position *)event;
   Eclair *eclair = (Eclair *)data;

   if (!eclair || !eclair->gui_window)
      return 1;

   evas = ecore_evas_get(eclair->gui_window);
   ecore_evas_geometry_get(eclair->gui_window, &gui_window_x, &gui_window_y, NULL, NULL);
   x = ev->position.x - gui_window_x;
   y = ev->position.y - gui_window_y;

   if (eclair->playlist_container && evas_object_visible_get(eclair->playlist_container))
   {
      evas_object_geometry_get(eclair->playlist_container, &rect.x, &rect.y, &rect.width, &rect.height);
      if (x >= rect.x && x <= (rect.x + rect.width) && y >= rect.y && y <= (rect.y + rect.height))
      {
         rect.x += gui_window_x;
         rect.y += gui_window_y;
         ecore_x_dnd_send_status(1, 1, rect, ECORE_X_DND_ACTION_PRIVATE);
         eclair->gui_drop_object = ECLAIR_DROP_PLAYLIST;
         return 1;
      }
   }
   if (eclair->gui_cover && evas_object_visible_get(eclair->gui_cover))
   {
      evas_object_geometry_get(eclair->gui_cover, &rect.x, &rect.y, &rect.width, &rect.height);
      if (x >= rect.x && x <= (rect.x + rect.width) && y >= rect.y && y <= (rect.y + rect.height))
      {
         rect.x += gui_window_x;
         rect.y += gui_window_y;
         ecore_x_dnd_send_status(1, 1, rect, ECORE_X_DND_ACTION_PRIVATE);
         eclair->gui_drop_object = ECLAIR_DROP_COVER;
         return 1;
      }
   }  

   ecore_evas_geometry_get(eclair->gui_window, &rect.x, &rect.y, &rect.width, &rect.height);
   rect.x += gui_window_x;
   rect.y += gui_window_y;
   ecore_x_dnd_send_status(0, 1, rect, ECORE_X_DND_ACTION_PRIVATE);
   eclair->gui_drop_object = ECLAIR_DROP_NONE;
   return 1;
}

//Called when an object is dropped on the gui
int eclair_gui_dnd_drop_cb(void *data, int type, void *event)
{
   Eclair *eclair = (Eclair *)data;

   if (!eclair)
      return 1;

   ecore_x_selection_xdnd_request(eclair->gui_x_window, "text/uri-list");
   return 1;
}

//Treat the files dropped
int eclair_gui_dnd_selection_cb(void *data, int type, void *event)
{
   Ecore_X_Event_Selection_Notify *ev = (Ecore_X_Event_Selection_Notify *)event;
   Eclair *eclair = (Eclair *)data;
   Ecore_X_Selection_Data_Files *files;
   int i;

   if (!eclair || !eclair->gui_window || eclair->gui_drop_object == ECLAIR_DROP_NONE
      || ev->selection != ECORE_X_SELECTION_XDND || !(files = ev->data) || files->num_files <= 0)
   {
      ecore_x_dnd_send_finished();
      return 1;
   }

   if (eclair->gui_drop_object == ECLAIR_DROP_PLAYLIST)
   {
      for (i = 0; i < files->num_files; i++)
         eclair_playlist_add_uri(&eclair->playlist, files->files[i], 0, 1);
      eclair_playlist_container_update(eclair->playlist_container);
   }
   else if (eclair->gui_drop_object == ECLAIR_DROP_COVER)
      eclair_cover_current_set(&eclair->cover_manager, files->files[0]);

   ecore_x_dnd_send_finished();

   return 1;
}

//Called when the user clicks on on of the ecore windows
int eclair_mouse_up_cb(void *data, int type, void *event)
{
   Eclair *eclair = (Eclair *)data;
   Ecore_X_Event_Mouse_Button_Up *mouse_event = (Ecore_X_Event_Mouse_Button_Up *)event;

   if (!eclair || !mouse_event)
      return 1;

   //TODO:Ecore event bug? mouse_event win and video x window doesn't match but they should have
   if (mouse_event->button == 3/* && (mouse_event->event_win == eclair->gui_x_window || mouse_event->win == eclair->video_x_window)*/)
      eclair_popup_menu(&eclair->dialogs_manager);

   return 1;
}

//Called when the gui send a message
void eclair_gui_message_cb(void *data, Evas_Object *obj, Edje_Message_Type type, int id, void *msg)
{
   Eclair *eclair = (Eclair *)data;
   
   if (!eclair)
      return;

   switch (id)
   {
      default:
         break;
   }
}
