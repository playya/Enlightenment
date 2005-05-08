#include "eclair_playlist.h"
#include "../config.h"
#include <string.h>
#include <stdio.h>
#include <Esmart/Esmart_Container.h>
#include <Evas.h>
#include <Ecore_File.h>
#include <Edje.h>
#include "eclair.h"
#include "eclair_cover.h"
#include "eclair_media_file.h"
#include "eclair_meta_tag.h"
#include "eclair_callbacks.h"
#include "eclair_utils.h"

#define MAX_PATH_LEN 1024

//Initialize the playlist
void eclair_playlist_init(Eclair_Playlist *playlist, Eclair *eclair)
{
   if (!playlist)
      return;   

   playlist->playlist = NULL;
   playlist->current = NULL;
   playlist->shuffle = 0;
   playlist->repeat = 0;
   playlist->eclair = eclair;
}

//Empty the playlist and destroy all the media_file
void eclair_playlist_empty(Eclair_Playlist *playlist)
{
   Evas_List *l;

   if (!playlist)
      return;

   for (l = playlist->playlist; l; )
      l = eclair_playlist_remove_media_file_list(playlist, l);

   evas_list_free(playlist->playlist);
   playlist->playlist = NULL;
   playlist->current = NULL;
}

//Return the active media file
Eclair_Media_File *eclair_playlist_current_media_file(Eclair_Playlist *playlist)
{
   if (!playlist)
      return NULL;

   return evas_list_data(playlist->current);
}

//Return the media file just before the active media file
Eclair_Media_File *eclair_playlist_prev_media_file(Eclair_Playlist *playlist)
{
   if (!playlist)
      return NULL;

   return (Eclair_Media_File *)evas_list_data(evas_list_prev(playlist->current));
}

//Return the media file just after the active media file
Eclair_Media_File *eclair_playlist_next_media_file(Eclair_Playlist *playlist)
{
   if (!playlist)
      return NULL;

   return (Eclair_Media_File *)evas_list_data(evas_list_next(playlist->current));
}

//Add recursively a directory
Evas_Bool eclair_playlist_add_dir(Eclair_Playlist *playlist, char *dir)
{
   Ecore_List *files;
   Ecore_List_Node *l;
   char *filename, *filepath;

   if (!playlist || !dir || !ecore_file_is_dir(dir))
      return 0;

   if ((files = ecore_file_ls(dir)))
   {
      for (l = files->first; l; l = l->next)
      {
         if (!(filename = (char *)l->data))
            continue;
         filepath = (char *)malloc(strlen(dir) + strlen(filename) + 2);
         sprintf(filepath, "%s/%s", dir, filename);
         eclair_playlist_add_uri(playlist, filepath);
         free(filepath);
      }
      ecore_list_destroy(files);
   }
   return 1;
}

//Add files stored in the m3u file
Evas_Bool eclair_playlist_add_m3u(Eclair_Playlist *playlist, char *m3u_path)
{
   FILE *m3u_file;
   char line[MAX_PATH_LEN], *path, *c, *m3u_dir;

   if (!playlist || !m3u_path || !(m3u_file = fopen(m3u_path, "rt")))
      return 0;

   m3u_dir = ecore_file_get_dir(m3u_path);

   while (fgets(line, MAX_PATH_LEN, m3u_file))
   {
      if (line[0] == '#')
         continue;

      for (c = strpbrk(line, "\r\n"); c; c = strpbrk(c, "\r\n"))
         *c = 0;
   
      if (line[0] == '/')
         eclair_playlist_add_uri(playlist, line);
      else if (m3u_dir)
      {
         path = (char *)malloc(strlen(m3u_dir) + strlen(line) + 2);
         sprintf(path, "%s/%s", m3u_dir, line);
         eclair_playlist_add_uri(playlist, path);
         free(path);
      }
   }

   free(m3u_dir);
   fclose(m3u_file);
   return 1;
}

//Add the media file located at the uri
Evas_Bool eclair_playlist_add_uri(Eclair_Playlist *playlist, char *uri)
{
   Eclair_Media_File *new_media_file;
   Evas_Coord min_height;
   Eclair *eclair;
   char *clean_uri, *new_path;

   if (!playlist || !uri)
      return 0;

   if (strstr(uri, "://"))
   {
      if (!(clean_uri = eclair_utils_remove_uri_special_chars(uri)))
         return 0;

      if (strlen(clean_uri) <= 7 || strncmp(clean_uri, "file://", 7) != 0)
         new_path = clean_uri;
      else
      {
         new_path = strdup(&clean_uri[7]);
         free(clean_uri);
      }
   }
   else
      new_path = strdup(uri);

   if (!strstr(new_path, "://"))
   {
      if (eclair_playlist_add_dir(playlist, new_path))
      {
         free(new_path);
         return 1;
      }
      if (strcmp(eclair_utils_file_get_extension(new_path), "m3u") == 0)
      {
         eclair_playlist_add_m3u(playlist, new_path);
         free(new_path);
         return 1;  
      }
   }
   
   new_media_file = eclair_media_file_new();
   new_media_file->path = new_path;

   if ((eclair = playlist->eclair))
   {
      if (eclair->playlist_container)
      {
         new_media_file->playlist_entry = edje_object_add(evas_object_evas_get(eclair->playlist_container));
         edje_object_file_set(new_media_file->playlist_entry, eclair->gui_theme_file, "eclair_playlist_entry");
         evas_object_data_set(new_media_file->playlist_entry, "media_file", new_media_file);
         if (eclair->playlist_entry_height <= 0)
         {
            edje_object_size_min_get(new_media_file->playlist_entry, NULL, &min_height);
            eclair->playlist_entry_height = (int)min_height;
         }
         evas_object_resize(new_media_file->playlist_entry, 1, eclair->playlist_entry_height);
         edje_object_part_text_set(new_media_file->playlist_entry, "playlist_entry_name", ecore_file_get_file(new_media_file->path));
         edje_object_part_text_set(new_media_file->playlist_entry, "playlist_entry_length", "");
         edje_object_signal_callback_add(new_media_file->playlist_entry, "eclair_play_entry", "*", eclair_gui_play_entry_cb, eclair);
         esmart_container_element_append(eclair->playlist_container, new_media_file->playlist_entry);
         evas_object_show(new_media_file->playlist_entry);
      }
      
      if (!strstr(new_media_file->path, "://"))
         eclair_meta_tag_add_file_to_scan(&eclair->meta_tag_manager, new_media_file);
   }

   playlist->playlist = evas_list_append(playlist->playlist, new_media_file);
   if (!playlist->current)
      eclair_playlist_current_set_list(playlist, playlist->playlist);

   return 1;   
}

//Remove the media file from the playlist
void eclair_playlist_remove_media_file(Eclair_Playlist *playlist, Eclair_Media_File *media_file)
{
   if (!playlist || !media_file)
      return;

   eclair_playlist_remove_media_file_list(playlist, evas_list_find_list(playlist->playlist, media_file));
}

//Remove the media file pointed by the list from the playlist
//Return the next media file
Evas_List *eclair_playlist_remove_media_file_list(Eclair_Playlist *playlist, Evas_List *list)
{
   Eclair_Media_File *remove_media_file;
   Evas_List *next;

   if (!playlist || !list)
      return NULL;
/*
   if (playlist->current == list)
   {
      if (playlist->current->next)
         eclair_playlist_next_as_current(playlist);
      else
         eclair_playlist_prev_as_current(playlist);            
   }*/

   if ((remove_media_file = evas_list_data(list)))
   {
      if (remove_media_file->playlist_entry && playlist->eclair)
         esmart_container_element_destroy(playlist->eclair->playlist_container, remove_media_file->playlist_entry);
      eclair_media_file_free(remove_media_file);
   }

   next = list->next;
   playlist->playlist = evas_list_remove_list(playlist->playlist, list);

   return next;
}

//Set the media file as the active media file  
void eclair_playlist_current_set(Eclair_Playlist *playlist, Eclair_Media_File *media_file)
{
   if (!playlist)
      return;

   eclair_playlist_current_set_list(playlist, evas_list_find_list(playlist->playlist, media_file));
}

//Set the media file stored in the list as the active media file  
void eclair_playlist_current_set_list(Eclair_Playlist *playlist, Evas_List *list)
{
   Eclair_Media_File *media_file;

   if (!playlist)
      return;

   if ((media_file = eclair_playlist_current_media_file(playlist)) && media_file->playlist_entry)
      edje_object_signal_emit(media_file->playlist_entry, "signal_unset_current", "eclair_bin");

   if ((media_file = evas_list_data(list)) && media_file->playlist_entry)
   {
      edje_object_signal_emit(media_file->playlist_entry, "signal_set_current", "eclair_bin");
      if (playlist->eclair)
      {
         //TODO: doesn't work?
         if (playlist->eclair->playlist_container)
            esmart_container_scroll_to(playlist->eclair->playlist_container, media_file->playlist_entry);
      }
   }
   
   playlist->current = list;
   eclair_update_current_file_info(playlist->eclair, 0);
} 

//Set the media file which is just before the active media file as the active media file 
void eclair_playlist_prev_as_current(Eclair_Playlist *playlist)
{
   if (!playlist)
      return;
   if (!playlist->current)
      return;

   eclair_playlist_current_set_list(playlist,  playlist->current->prev);
}

//Set the media file which is just after the active media file as the active media file 
void eclair_playlist_next_as_current(Eclair_Playlist *playlist)
{
   if (!playlist)
      return;
   if (!playlist->current)
      return;

   eclair_playlist_current_set_list(playlist,  playlist->current->next);
}
