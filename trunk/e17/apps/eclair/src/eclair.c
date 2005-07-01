#include "eclair.h"
#include "../config.h"
#include <Ecore.h>
#include <Ecore_X.h>
#include <Ecore_File.h>
#include <Ecore_Evas.h>
#include <Edje.h>
#include <Emotion.h>
#include <Esmart/Esmart_Draggies.h>
#include <pthread.h>
#include "eclair_playlist.h"
#include "eclair_playlist_container.h"
#include "eclair_media_file.h"
#include "eclair_callbacks.h"
#include "eclair_subtitles.h"
#include "eclair_meta_tag.h"
#include "eclair_cover.h"
#include "eclair_utils.h"
#include "eclair_config.h"
#include "eclair_args.h"
#include "eclair_dialogs.h"
#include "eclair_window.h"
#include "eclair_database.h"

static void *_eclair_create_video_object_thread(void *param);
static Evas_Bool _eclair_create_gui_window(Eclair *eclair);
static Evas_Bool _eclair_create_playlist_window(Eclair *eclair);
static Evas_Bool _eclair_create_equalizer_window(Eclair *eclair);
static Evas_Bool _eclair_create_cover_window(Eclair *eclair);
static Evas_Bool _eclair_create_playlist_container_object(Eclair *eclair, Eclair_Window *window);
static Evas_Bool _eclair_create_cover_object(Eclair *eclair, Eclair_Window *window);
static void _eclair_add_inter_windows_callbacks(Eclair *eclair);
static void _eclair_create_video_window(Eclair *eclair);
static void _eclair_sig_pregest();
static void _eclair_on_segv(int num);

//Initialize eclair
Evas_Bool eclair_init(Eclair *eclair, int *argc, char ***argv)
{
   Evas_List *filenames, *l;
   filenames = NULL;

   if (!eclair)
      return 0;

   ecore_init();
   ecore_file_init();
   ecore_evas_init();
   edje_init();

   eclair->argc = argc;
   eclair->argv = argv;
   eclair->video_window = NULL;
   eclair->video_object = NULL;
   eclair->black_background = NULL;
   eclair->subtitles_object = NULL;
   eclair->gui_window = NULL;
   eclair->cover = NULL;
   eclair->previous_cover = NULL;
   eclair->equalizer_window = NULL;
   eclair->playlist_window = NULL;
   eclair->playlist_container = NULL;
   eclair->playlist_container_owner = NULL;
   eclair->playlist_entry_height = -1;
   eclair->cover_window = NULL;
   eclair->cover_owner = NULL;   
   eclair->state = ECLAIR_STOP;
   eclair->seek_to_pos = -1.0;
   eclair->use_progress_bar_drag_for_time = 0;
   eclair->dont_update_progressbar = 0;
   eclair->gui_engine = ECLAIR_SOFTWARE;
   eclair->gui_theme_file = strdup(PACKAGE_DATA_DIR "/themes/default.edj");
   eclair->drop_object = ECLAIR_DROP_NONE;
   eclair->drop_window = NULL;
   eclair->video_engine = ECLAIR_SOFTWARE;
   eclair->start_playing = 0;

   if (!eclair_args_parse(eclair, &filenames))
      return 0;

   eclair_config_init(&eclair->config);
   eclair_utils_init();
   if (!_eclair_create_gui_window(eclair))
   {
      eclair_config_shutdown(&eclair->config);
      fprintf(stderr, "Error: Unable to create the gui window\n");
      return 0;
   }
   _eclair_create_playlist_window(eclair);
   _eclair_create_equalizer_window(eclair);
   _eclair_create_cover_window(eclair);
   _eclair_create_video_window(eclair);
   _eclair_add_inter_windows_callbacks(eclair);
   eclair_database_init(&eclair->database, eclair);
   eclair_dialogs_init(&eclair->dialogs_manager, eclair);
   eclair_playlist_init(&eclair->playlist, eclair);
   eclair_playlist_container_set_media_list(eclair->playlist_container, &eclair->playlist.playlist);
   eclair_subtitles_init(&eclair->subtitles);
   eclair_meta_tag_init(&eclair->meta_tag_manager, eclair);
   eclair_cover_init(&eclair->cover_manager, eclair);
   eclair_update_current_file_info(eclair, 0);

   if ((l = filenames))
   {
      for (; l; l = l->next)
         eclair_playlist_add_uri(&eclair->playlist, l->data, 0, 1);
      evas_list_free(filenames);
      eclair_playlist_container_update(eclair->playlist_container);
   }
   else
      eclair_playlist_add_uri(&eclair->playlist, eclair->config.default_playlist_path, 1, 0);

   ecore_event_handler_add(ECORE_X_EVENT_XDND_POSITION, eclair_dnd_position_cb, eclair);
   ecore_event_handler_add(ECORE_X_EVENT_XDND_DROP, eclair_dnd_drop_cb, eclair);
   ecore_event_handler_add(ECORE_X_EVENT_SELECTION_NOTIFY, eclair_dnd_selection_cb, eclair);
   ecore_event_handler_add(ECORE_X_EVENT_MOUSE_BUTTON_UP, eclair_mouse_up_cb, eclair);
   edje_object_part_drag_value_set(eclair->gui_window->edje_object, "volume_bar_drag", 1.0, 0.0);

   return 1;
}

//Shutdown eclair
void eclair_shutdown(Eclair *eclair)
{
   if (eclair)
   {
      eclair_window_del(eclair->gui_window);
      eclair_window_del(eclair->playlist_window);
      eclair_window_del(eclair->equalizer_window);
      eclair_window_del(eclair->cover_window);

      fprintf(stderr, "Eclair: Debug: Destroying create video object thread\n");
      pthread_join(eclair->video_create_thread, NULL); 
      fprintf(stderr, "Eclair: Debug: Create video object thread destroyed\n");
      eclair_dialogs_shutdown(&eclair->dialogs_manager);
      eclair_subtitles_free(&eclair->subtitles);
      eclair_meta_tag_shutdown(&eclair->meta_tag_manager);
      eclair_cover_shutdown(&eclair->cover_manager);
      eclair_playlist_save(&eclair->playlist, eclair->config.default_playlist_path);
      eclair_playlist_shutdown(&eclair->playlist);
      eclair_config_shutdown(&eclair->config);
      free(eclair->gui_theme_file);
   }

   ecore_main_loop_quit();
}

//Update text objects, progress bar...
//Called when an new frame is decoded
void eclair_update(Eclair *eclair)
{
   char time_elapsed[10];
   double position, length;

   if (!eclair)
      return;

   position = eclair_position_get(eclair);

   //Display subtitles
   eclair_subtitles_display_current_subtitle(&eclair->subtitles, eclair_position_get(eclair), eclair->subtitles_object);

   if (!eclair->video_object || !eclair->gui_window)
      return;

   //Update time text
   length = emotion_object_play_length_get(eclair->video_object);
   if (eclair->use_progress_bar_drag_for_time)
   {
      edje_object_part_drag_value_get(eclair->gui_window->edje_object, "progress_bar_drag", &position, NULL);
      position *= length;
   }
   eclair_utils_second_to_string(position, length, time_elapsed);
   eclair_all_windows_text_set(eclair, "time_elapsed", time_elapsed);

   //Update progress bar
   if (eclair->dont_update_progressbar)
      eclair->dont_update_progressbar = 0;
   else if (eclair->seek_to_pos >= 0.0)
      eclair->seek_to_pos = -1.0;
   edje_object_part_drag_value_set(eclair->gui_window->edje_object, "progress_bar_drag", position / length, 0.0);
}

//Update the gui infos about the current media file
void eclair_update_current_file_info(Eclair *eclair, Evas_Bool force_cover_update)
{   
   char *window_title;
   char *artist_title_string;
   const char *filename;
   Eclair_Media_File *current_file;
   char string[10];

   if (!eclair)
      return;

   current_file = eclair_playlist_current_media_file(&eclair->playlist);

   //Update the name of the current file only if video is created 
   if (eclair->video_object)
   {
      if (current_file)
      {
         if ((artist_title_string = eclair_utils_mediafile_to_artist_title_string(current_file)))
         {
            eclair_all_windows_text_set(eclair, "current_media_name", artist_title_string);
            free(artist_title_string);
         }
         else if (current_file->path && strstr(current_file->path, "://"))
            eclair_all_windows_text_set(eclair, "current_media_name", current_file->path);
         else if (current_file->path && (filename = ecore_file_get_file(current_file->path)))
            eclair_all_windows_text_set(eclair, "current_media_name", filename);
         else
            eclair_all_windows_text_set(eclair, "current_media_name", "No media opened");
      }
      else
         eclair_all_windows_text_set(eclair, "current_media_name", "No media opened");
   }
   //Update current media file data
   if (current_file)
   {
      if (current_file->samplerate > 0)
      {
         snprintf(string, 10, "%d", current_file->samplerate / 1000);
         eclair_all_windows_text_set(eclair, "samplerate", string);
      }
      else
         eclair_all_windows_text_set(eclair, "samplerate", "-");

      if (current_file->bitrate > 0)
      {
         snprintf(string, 10, "%d", current_file->bitrate);
         eclair_all_windows_text_set(eclair, "bitrate", string);
      }
      else
         eclair_all_windows_text_set(eclair, "bitrate", "-");
   }
   else
   {
      eclair_all_windows_text_set(eclair, "samplerate", "-");
      eclair_all_windows_text_set(eclair, "bitrate", "-");
   }
   

   //Update the title of the video window
   if (eclair->video_window)
   {
      if (current_file && current_file->path)
      {
         window_title = malloc(strlen(current_file->path) + strlen("eclair: ") + 1);
         sprintf(window_title, "eclair: %s", current_file->path);
         ecore_evas_title_set(eclair->video_window, window_title);
         free(window_title);
      }
      else
         ecore_evas_title_set(eclair->video_window, "eclair");
   }

   //Update the current cover
   if (current_file)
   {
      if (current_file->cover_path || (current_file->cover_path = eclair_cover_file_get_from_local(&eclair->cover_manager, current_file->artist, current_file->album, current_file->path)))
         eclair_gui_cover_set(eclair, current_file->cover_path, force_cover_update);
      else
         eclair_gui_cover_set(eclair, NULL, force_cover_update);
   }
   else
      eclair_gui_cover_set(eclair, NULL, force_cover_update);
}

//Set the cover displayed on the GUI
//Remove it if cover_path == NULL
void eclair_gui_cover_set(Eclair *eclair, const char *cover_path, Evas_Bool force_cover_update)
{
   char *current_path;

   if (!eclair || !eclair->cover_owner || !eclair->cover)
      return;

   current_path = NULL;
   evas_object_image_file_get(eclair->cover, &current_path, NULL);
   if (!current_path && !cover_path)
      return;
   if (!force_cover_update && current_path && cover_path && (strcmp(current_path, cover_path) == 0))
      return;

   if (eclair->previous_cover)
   {
      Evas_Object *tmp;

      edje_object_part_unswallow(eclair->cover_owner->edje_object, eclair->cover);
      edje_object_part_unswallow(eclair->cover_owner->edje_object, eclair->previous_cover);
      tmp = eclair->previous_cover;
      eclair->previous_cover = eclair->cover;
      eclair->cover = tmp;
      edje_object_part_swallow(eclair->cover_owner->edje_object, "cover", eclair->cover);
      edje_object_part_swallow(eclair->cover_owner->edje_object, "previous_cover", eclair->previous_cover);
   }

   evas_object_image_file_set(eclair->cover, cover_path, NULL);
   //TODO: evas_object_image_reload bug? need to do two reloads to really reload the image?!
   if (current_path && cover_path && (strcmp(current_path, cover_path) == 0))
   {
      evas_object_image_reload(eclair->cover);
      evas_object_image_reload(eclair->cover);
   }
   if (cover_path)
   {
      edje_object_signal_emit(eclair->cover_owner->edje_object, "signal_cover_set", "eclair_bin");
      evas_object_show(eclair->cover);
   }
   else
   {
      edje_object_signal_emit(eclair->cover_owner->edje_object, "signal_cover_unset", "eclair_bin");
      evas_object_hide(eclair->cover);
   }
}

//Send a signal to the edje object of all the windows
void eclair_send_signal_to_all_windows(Eclair *eclair, const char *signal)
{
   if (!eclair || !signal)
      return;

   if (eclair->gui_window)
      edje_object_signal_emit(eclair->gui_window->edje_object, signal, "eclair_bin");
   if (eclair->playlist_window)
      edje_object_signal_emit(eclair->playlist_window->edje_object, signal, "eclair_bin");
   if (eclair->cover_window)
      edje_object_signal_emit(eclair->cover_window->edje_object, signal, "eclair_bin");
}

//Set the text value of a field for all the windows
void eclair_all_windows_text_set(Eclair *eclair, const char *field_name, const char *text)
{
   if (!eclair || !field_name || !text)
      return;

   if (eclair->gui_window)
      edje_object_part_text_set(eclair->gui_window->edje_object, field_name, text);
   if (eclair->playlist_window)
      edje_object_part_text_set(eclair->playlist_window->edje_object, field_name, text);
   if (eclair->cover_window)
      edje_object_part_text_set(eclair->cover_window->edje_object, field_name, text);
}

//TODO: drag_get and and drag_set all windows

//Play a new file
void eclair_play_file(Eclair *eclair, const char *path)
{
   int video_width, video_height;

   if (!eclair)
      return;

   if (!eclair->video_object)
   {
      eclair->start_playing = 1;
      return;
   }

   if (path)
   {
      emotion_object_file_set(eclair->video_object, path);
      emotion_object_play_set(eclair->video_object, 0);
      eclair_progress_rate_set(eclair, 0.0);
      eclair->state = ECLAIR_PAUSE;
      eclair_play(eclair);
   
      eclair_subtitles_load_from_media_file(&eclair->subtitles, path);
      
      if (eclair->video_window)
      {
         if (emotion_object_video_handled_get(eclair->video_object))
         {
            ecore_evas_show(eclair->video_window);
            emotion_object_size_get(eclair->video_object, &video_width, &video_height);
            ecore_evas_resize(eclair->video_window, video_width, video_height);
            eclair_video_window_resize_cb(eclair->video_window);
         }
         else
            ecore_evas_hide(eclair->video_window);
      }
   }
   else
      eclair_stop(eclair);
}

//Play the active file from the playlist
void eclair_play_current(Eclair *eclair)
{
   Eclair_Media_File *current_media_file;

   if (!eclair)
      return;

   if ((current_media_file = eclair_playlist_current_media_file(&eclair->playlist)))
      eclair_play_file(eclair, current_media_file->path);
   else
      eclair_stop(eclair);
}

//Play the file which is before the active file 
void eclair_play_prev(Eclair *eclair)
{
   if (!eclair)
      return;
   
   eclair_playlist_prev_as_current(&eclair->playlist);
   eclair_play_current(eclair);
}

//Play the file which is after the active file
void eclair_play_next(Eclair *eclair)
{
   if (!eclair)
      return;
   
   eclair_playlist_next_as_current(&eclair->playlist);
   eclair_play_current(eclair);
}

//Pause the playback
void eclair_pause(Eclair *eclair)
{
   if (!eclair || (eclair->state != ECLAIR_PLAYING))
      return;

   emotion_object_play_set(eclair->video_object, 0);
   eclair_send_signal_to_all_windows(eclair, "signal_pause");
   eclair->state = ECLAIR_PAUSE;
}

//TODO: redundant with play_current?
//Play the current file if state is STOP or resume if state is PAUSE
void eclair_play(Eclair *eclair)
{
   if (!eclair)
      return;

   if (!eclair->video_object)
   {
      eclair->start_playing = 1;
      return;
   }

   if (eclair->state == ECLAIR_PAUSE)
   {
      emotion_object_play_set(eclair->video_object, 1);
      eclair_send_signal_to_all_windows(eclair, "signal_play");
      eclair->state = ECLAIR_PLAYING;
   }
   else if (eclair->state == ECLAIR_STOP)
      eclair_play_current(eclair);
}

//Stop the playback and hide the video window
void eclair_stop(Eclair *eclair)
{
   if (!eclair)
      return;

   if (eclair->video_object)
   {
      emotion_object_play_set(eclair->video_object, 0);
      eclair_progress_rate_set(eclair, 0.0);
   }
   
   eclair_all_windows_text_set(eclair, "time_elapsed", "0:00");
   eclair_send_signal_to_all_windows(eclair, "signal_stop");
   
   if (eclair->video_window)
      ecore_evas_hide(eclair->video_window);

   eclair->state = ECLAIR_STOP;
}

//Set the audio level
void eclair_audio_level_set(Eclair *eclair, double audio_level)
{
   if (!eclair)
      return;

      if (audio_level < 0.0)
         audio_level = 0.0;
      else if (audio_level > 1.0)
         audio_level = 1.0;

   if (eclair->video_object)
      emotion_object_audio_volume_set(eclair->video_object, audio_level);
   if (eclair->gui_window)
      edje_object_part_drag_value_set(eclair->gui_window->edje_object, "volume_bar_drag", audio_level, 0);
}

//Get the media progress rate
double eclair_progress_rate_get(Eclair *eclair)
{
   if (!eclair || !eclair->video_object)
      return 0.0;
   
   return (eclair_position_get(eclair) / emotion_object_play_length_get(eclair->video_object));
}

//Set the media progress rate
void eclair_progress_rate_set(Eclair *eclair, double progress_rate)
{
   if (!eclair || !eclair->video_object)
      return;
   
   eclair_position_set(eclair, progress_rate * emotion_object_play_length_get(eclair->video_object));
}

//Get the media position in seconds
double eclair_position_get(Eclair *eclair)
{
   if (!eclair || !eclair->video_object)
      return 0.0;

   if (eclair->seek_to_pos < 0.0)
      return emotion_object_position_get(eclair->video_object);
   else
      return eclair->seek_to_pos;
}

//Set the media position in seconds
void eclair_position_set(Eclair *eclair, double position)
{
   double media_length;

   if (!eclair || !eclair->video_object)
      return;

   media_length = emotion_object_play_length_get(eclair->video_object);
   if (position < 0.0)
      position = 0.0;
   else if (position > media_length)
      position = media_length;
   eclair->dont_update_progressbar = 1;
   eclair->seek_to_pos = position;
   emotion_object_position_set(eclair->video_object, eclair->seek_to_pos);
}

static void testff(void *data, Evas_Object *obj, const char *part)
{
   Evas_Coord x, y, w, h;
   edje_object_part_geometry_get(obj, part, &x, &y, &w, &h);
   printf("%s %p %s -> %d %d %d %d\n", part, obj, edje_object_part_text_get(obj, part), x, y, w, h);
}

//Create the gui window and load the edje theme
static Evas_Bool _eclair_create_gui_window(Eclair *eclair)
{
   if (!eclair)
      return 0;

   if ((eclair->gui_window = eclair_window_create(eclair->gui_theme_file, "main", "eclair", eclair->gui_engine, eclair, 1)))
   {
      _eclair_create_playlist_container_object(eclair, eclair->gui_window);
      _eclair_create_cover_object(eclair, eclair->gui_window);
      eclair_window_add_default_callbacks(eclair->gui_window, eclair);
      edje_object_text_change_cb_set(eclair->gui_window->edje_object, testff, NULL);
      printf("%p\n", eclair->gui_window->edje_object);
      return 1;
   }

   //TODO: try to load default theme
   fprintf(stderr, "Error: Unable to load the theme \"%s\"\n", eclair->gui_theme_file);
   return 0;
}

//Create the playlist window and load the edje theme for the playlist
static Evas_Bool _eclair_create_playlist_window(Eclair *eclair)
{
   if (!eclair)
      return 0;

   if ((eclair->playlist_window = eclair_window_create(eclair->gui_theme_file, "playlist", "eclair: playlist", eclair->gui_engine, eclair, 0)))
   {
      _eclair_create_playlist_container_object(eclair, eclair->playlist_window);
      _eclair_create_cover_object(eclair, eclair->playlist_window);      
      eclair_window_add_default_callbacks(eclair->playlist_window, eclair);
      return 1;
   }

   return 0;
}

//Create the equalizer window and load the edje theme for the equalizer
static Evas_Bool _eclair_create_equalizer_window(Eclair *eclair)
{
   if (!eclair)
      return 0;

   if ((eclair->equalizer_window = eclair_window_create(eclair->gui_theme_file, "equalizer", "eclair: equalizer", eclair->gui_engine, eclair, 0)))
   {
      _eclair_create_playlist_container_object(eclair, eclair->equalizer_window);
      _eclair_create_cover_object(eclair, eclair->equalizer_window);      
      eclair_window_add_default_callbacks(eclair->equalizer_window, eclair);
      return 1;
   }

   return 0;
}

//Create the cover window
static Evas_Bool _eclair_create_cover_window(Eclair *eclair)
{
   if (!eclair)
      return 0;

   if ((eclair->cover_window = eclair_window_create(eclair->gui_theme_file, "cover", "eclair: cover", eclair->gui_engine, eclair, 0)))
   {
      _eclair_create_playlist_container_object(eclair, eclair->cover_window);
      _eclair_create_cover_object(eclair, eclair->cover_window);
      eclair_window_add_default_callbacks(eclair->cover_window, eclair);
      return 1;
   }

   return 0;
}

//Create the video window and object
static void _eclair_create_video_window(Eclair *eclair)
{
   Evas *evas;

   if (!eclair)
      return;

   if (eclair->video_engine == ECLAIR_GL)
      eclair->video_window = ecore_evas_gl_x11_new(NULL, 0, 0, 0, 0, 0);
   else
      eclair->video_window = ecore_evas_software_x11_new(NULL, 0, 0, 0, 0, 0);
   ecore_evas_title_set(eclair->video_window, "eclair");
   ecore_evas_name_class_set(eclair->video_window, "eclair", "eclair");
   ecore_evas_data_set(eclair->video_window, "eclair", eclair);
   ecore_evas_callback_resize_set(eclair->video_window, eclair_video_window_resize_cb);
   ecore_evas_callback_delete_request_set(eclair->video_window, eclair_video_window_close_cb);
   ecore_evas_hide(eclair->video_window);

   evas = ecore_evas_get(eclair->video_window);   
   eclair->black_background = evas_object_rectangle_add(evas);
   evas_object_color_set(eclair->black_background, 0, 0, 0, 255);
   evas_object_show(eclair->black_background);
   evas_object_layer_set(eclair->black_background, 0);

   eclair->subtitles_object = evas_object_textblock_add(evas);
   evas_font_path_append(evas, PACKAGE_DATA_DIR "/fonts/");
   evas_object_layer_set(eclair->subtitles_object, 2);
   evas_object_show(eclair->subtitles_object);

   pthread_create(&eclair->video_create_thread, NULL, _eclair_create_video_object_thread, eclair);
}

//Initialize the video object
static void *_eclair_create_video_object_thread(void *param)
{
   Eclair *eclair;
   Evas *evas;
   Evas_Object *new_video_object;

   if (!(eclair = param))
      return NULL;

   if (eclair->gui_window)
      edje_object_part_text_set(eclair->gui_window->edje_object, "current_media_name", "Initializing - Please wait...");

   evas = ecore_evas_get(eclair->video_window);
   new_video_object = emotion_object_add(evas);
   evas_object_show(new_video_object);
   evas_object_layer_set(new_video_object, 1);
   emotion_object_init(new_video_object);

   evas_object_focus_set(new_video_object, 1);
   evas_object_event_callback_add(new_video_object, EVAS_CALLBACK_KEY_DOWN, eclair_key_press_cb, eclair);
   evas_object_smart_callback_add(new_video_object, "frame_decode", eclair_video_frame_decode_cb, eclair);
   evas_object_smart_callback_add(new_video_object, "playback_finished", eclair_video_playback_finished_cb, eclair);
   evas_object_smart_callback_add(new_video_object, "audio_level_change", eclair_video_audio_level_change_cb, eclair);

   eclair_audio_level_set(eclair, emotion_object_audio_volume_get(new_video_object));
   eclair->video_object = new_video_object;
   eclair_update_current_file_info(eclair, 0);

   if (eclair->start_playing)
      eclair_play_current(eclair);

   return NULL; 
}

//Create the playlist container and swallow it into the window
//Return 0 if failed
static Evas_Bool _eclair_create_playlist_container_object(Eclair *eclair, Eclair_Window *window)
{
   if (!eclair || !window)
      return 0;

   if (edje_object_part_exists(window->edje_object, "playlist_container"))
   {
      if (eclair->playlist_container)
         evas_object_del(eclair->playlist_container);
      eclair->playlist_container = eclair_playlist_container_object_add(window->evas, eclair);
      eclair_playlist_container_set_entry_theme_path(eclair->playlist_container, eclair->gui_theme_file);
      edje_object_part_swallow(window->edje_object, "playlist_container", eclair->playlist_container);
      evas_object_event_callback_add(eclair->playlist_container, EVAS_CALLBACK_MOUSE_WHEEL, eclair_playlist_container_wheel_cb, eclair);
      evas_object_smart_callback_add(eclair->playlist_container, "eclair_playlist_container_scroll_percent_changed", eclair_playlist_container_scroll_percent_changed, eclair);
      evas_object_show(eclair->playlist_container);
      eclair->playlist_container_owner = window;
      return 1;
   }
   return 0;
}

//Create the cover object and swallow it into the window
//Return 0 if failed
static Evas_Bool _eclair_create_cover_object(Eclair *eclair, Eclair_Window *window)
{
   Evas_Coord cover_width, cover_height;

   if (!eclair || !window)
      return 0;

   if (edje_object_part_exists(window->edje_object, "cover"))
   {
      if (eclair->cover)
         evas_object_del(eclair->cover);
      if (eclair->previous_cover)
      {
         evas_object_del(eclair->previous_cover);
         eclair->previous_cover = NULL;
      }
      eclair->cover = evas_object_image_add(window->evas);
      evas_object_repeat_events_set(eclair->cover, 1);
      edje_object_part_swallow(window->edje_object, "cover", eclair->cover);
      edje_object_part_geometry_get(window->edje_object, "cover", NULL, NULL, &cover_width, &cover_height);
      evas_object_image_fill_set(eclair->cover, 0, 0, cover_width, cover_height);
      evas_object_hide(eclair->cover);
      if (edje_object_part_exists(window->edje_object, "previous_cover"))
      {
         eclair->previous_cover = evas_object_image_add(window->evas);
         evas_object_repeat_events_set(eclair->previous_cover, 1);
         edje_object_part_swallow(window->edje_object, "previous_cover", eclair->previous_cover);
         edje_object_part_geometry_get(window->edje_object, "previous_cover", NULL, NULL, &cover_width, &cover_height);
         evas_object_image_fill_set(eclair->previous_cover, 0, 0, cover_width, cover_height);
         evas_object_show(eclair->previous_cover);
      }
      eclair->cover_owner = window;
      return 1;
   }
   return 0;
}

//Add the callbacks used to control interaction between the windows
//(the equalizer window should be able to open the playlist window for example)
static void _eclair_add_inter_windows_callbacks(Eclair *eclair)
{
   if (!eclair)
      return;

   if (eclair->gui_window)
   {
      edje_object_signal_callback_add(eclair->gui_window->edje_object, "eclair_playlist_open", "*", eclair_window_open_cb, eclair->playlist_window);
      edje_object_signal_callback_add(eclair->gui_window->edje_object, "eclair_playlist_close", "*", eclair_window_close_cb, eclair->playlist_window);
      edje_object_signal_callback_add(eclair->gui_window->edje_object, "eclair_equalizer_open", "*", eclair_window_open_cb, eclair->equalizer_window);
      edje_object_signal_callback_add(eclair->gui_window->edje_object, "eclair_equalizer_close", "*", eclair_window_close_cb, eclair->equalizer_window);
      edje_object_signal_callback_add(eclair->gui_window->edje_object, "eclair_cover_open", "*", eclair_window_open_cb, eclair->cover_window);
      edje_object_signal_callback_add(eclair->gui_window->edje_object, "eclair_cover_close", "*", eclair_window_close_cb, eclair->cover_window);
   }
   if (eclair->playlist_window)
   {
      edje_object_signal_callback_add(eclair->playlist_window->edje_object, "eclair_main_open", "*", eclair_window_open_cb, eclair->gui_window);
      edje_object_signal_callback_add(eclair->playlist_window->edje_object, "eclair_main_close", "*", eclair_window_close_cb, eclair->gui_window);
      edje_object_signal_callback_add(eclair->playlist_window->edje_object, "eclair_equalizer_open", "*", eclair_window_open_cb, eclair->equalizer_window);
      edje_object_signal_callback_add(eclair->playlist_window->edje_object, "eclair_equalizer_close", "*", eclair_window_close_cb, eclair->equalizer_window);
      edje_object_signal_callback_add(eclair->playlist_window->edje_object, "eclair_cover_open", "*", eclair_window_open_cb, eclair->cover_window);
      edje_object_signal_callback_add(eclair->playlist_window->edje_object, "eclair_cover_close", "*", eclair_window_close_cb, eclair->cover_window);
   }
   if (eclair->equalizer_window)
   {
      edje_object_signal_callback_add(eclair->equalizer_window->edje_object, "eclair_main_open", "*", eclair_window_open_cb, eclair->gui_window);
      edje_object_signal_callback_add(eclair->equalizer_window->edje_object, "eclair_main_close", "*", eclair_window_close_cb, eclair->gui_window);
      edje_object_signal_callback_add(eclair->equalizer_window->edje_object, "eclair_playlist_open", "*", eclair_window_open_cb, eclair->playlist_window);
      edje_object_signal_callback_add(eclair->equalizer_window->edje_object, "eclair_playlist_close", "*", eclair_window_close_cb, eclair->playlist_window);
      edje_object_signal_callback_add(eclair->equalizer_window->edje_object, "eclair_cover_open", "*", eclair_window_open_cb, eclair->cover_window);
      edje_object_signal_callback_add(eclair->equalizer_window->edje_object, "eclair_cover_close", "*", eclair_window_close_cb, eclair->cover_window);
   }
   if (eclair->cover_window)
   {
      edje_object_signal_callback_add(eclair->cover_window->edje_object, "eclair_main_open", "*", eclair_window_open_cb, eclair->gui_window);
      edje_object_signal_callback_add(eclair->cover_window->edje_object, "eclair_main_close", "*", eclair_window_close_cb, eclair->gui_window);
      edje_object_signal_callback_add(eclair->cover_window->edje_object, "eclair_playlist_open", "*", eclair_window_open_cb, eclair->playlist_window);
      edje_object_signal_callback_add(eclair->cover_window->edje_object, "eclair_playlist_close", "*", eclair_window_close_cb, eclair->playlist_window);
      edje_object_signal_callback_add(eclair->cover_window->edje_object, "eclair_equalizer_open", "*", eclair_window_open_cb, eclair->equalizer_window);
      edje_object_signal_callback_add(eclair->cover_window->edje_object, "eclair_equalizer_close", "*", eclair_window_close_cb, eclair->equalizer_window);
   }
}

//Handle segvs
static void _eclair_sig_pregest()
{
   struct sigaction sa;

   sa.sa_handler = _eclair_on_segv;
   sigaction(SIGSEGV, &sa, (struct sigaction *)0);
}

//Display a message on segvs
static void _eclair_on_segv(int num)
{
   fprintf(stderr, "\n\n");   
   fprintf(stderr, "Oops, eclair has crashed (SIG: %d) :(\n", num);
   fprintf(stderr, "\n");
   fprintf(stderr, "Have you compiled the latest version of eclair, emotion, evas, and all eclair dependencies ?\n");
   fprintf(stderr, "If it failed again, please report bugs to Mo0m (simon.treny@free.fr)\n");
   fprintf(stderr, "Describe how bugs happened, gdb traces, and so on ;)\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "With that, devs will be able to correct bugs faster and easier\n");
   fprintf(stderr, "If you correct the bug, or see in which code part it can provide, include it too\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "Thanks :)\n");
   
   exit(128 + num);
}

int main(int argc, char *argv[])
{
   Eclair eclair;

   _eclair_sig_pregest();

   if (!eclair_init(&eclair, &argc, &argv))
      return 1;
   
   ecore_main_loop_begin();

   edje_shutdown();
   ecore_file_shutdown();
   ecore_evas_shutdown();
   ecore_shutdown();

   return 0;
}
