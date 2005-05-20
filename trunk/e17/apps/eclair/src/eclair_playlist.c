#include "eclair_playlist.h"
#include "../config.h"
#include <string.h>
#include <stdio.h>
#include <Evas.h>
#include <Ecore_File.h>
#include <Edje.h>
#include "eclair.h"
#include "eclair_playlist_container.h"
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

//Shutdown the playlist
void eclair_playlist_shutdown(Eclair_Playlist *playlist)
{
   eclair_playlist_empty(playlist);
}

//Save the playlist
//0 if failed
Evas_Bool eclair_playlist_save(Eclair_Playlist *playlist, const char *path)
{
   Evas_List *l;
   FILE *playlist_file;
   Eclair_Media_File *media_file;

   if (!playlist || !path)
      return 0;
   
   if (!(playlist_file = fopen(path, "wt")))
      return 0;

   for (l = playlist->playlist; l; l = l->next)
   {
      if (!(media_file = (Eclair_Media_File *)l->data) || !media_file->path || strlen(media_file->path) <= 0)
         continue;
      fprintf(playlist_file, "%s\n", media_file->path);
   }
   fclose(playlist_file);
   return 1;
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
Evas_Bool eclair_playlist_add_dir(Eclair_Playlist *playlist, char *dir, Evas_Bool update_container)
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
         eclair_playlist_add_uri(playlist, filepath, 0);
         free(filepath);
      }
      ecore_list_destroy(files);
   }

   if (playlist->eclair && update_container)
      eclair_playlist_container_update(playlist->eclair->playlist_container);

   return 1;
}

//Add files stored in the m3u file
Evas_Bool eclair_playlist_add_m3u(Eclair_Playlist *playlist, char *m3u_path, Evas_Bool update_container)
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
         eclair_playlist_add_uri(playlist, line, 0);
      else if (m3u_dir)
      {
         path = (char *)malloc(strlen(m3u_dir) + strlen(line) + 2);
         sprintf(path, "%s/%s", m3u_dir, line);
         eclair_playlist_add_uri(playlist, path, 0);
         free(path);
      }
   }

   free(m3u_dir);
   fclose(m3u_file);

   if (playlist->eclair && update_container)
      eclair_playlist_container_update(playlist->eclair->playlist_container);

   return 1;
}

//Add the media file located at the uri
Evas_Bool eclair_playlist_add_uri(Eclair_Playlist *playlist, char *uri, Evas_Bool update_container)
{
   Eclair_Media_File *new_media_file;
   Eclair *eclair;
   char *clean_uri, *new_path, *ext;

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
      if (eclair_playlist_add_dir(playlist, new_path, 0))
      {
         free(new_path);
         return 1;
      }
      if ((ext = eclair_utils_file_get_extension(new_path)) && strcmp(ext, "m3u") == 0)
      {
         eclair_playlist_add_m3u(playlist, new_path, 0);
         free(new_path);
         return 1;  
      }
   }
   
   new_media_file = eclair_media_file_new();
   new_media_file->path = new_path;
   playlist->playlist = evas_list_append(playlist->playlist, new_media_file);
   if (!playlist->current)
      eclair_playlist_current_set_list(playlist, playlist->playlist);

   if ((eclair = playlist->eclair))
   {
      if (update_container)
         eclair_playlist_container_update(eclair->playlist_container);
      if (!strstr(new_media_file->path, "://"))
         eclair_meta_tag_add_file_to_scan(&eclair->meta_tag_manager, new_media_file);
   }
   return 1;   
}

//Remove the media file from the playlist
void eclair_playlist_remove_media_file(Eclair_Playlist *playlist, Eclair_Media_File *media_file, Evas_Bool update_container)
{
   if (!playlist || !media_file)
      return;

   eclair_playlist_remove_media_file_list(playlist, evas_list_find_list(playlist->playlist, media_file), update_container);
}

//Remove the media file pointed by the list from the playlist
//Return the next media file
Evas_List *eclair_playlist_remove_media_file_list(Eclair_Playlist *playlist, Evas_List *list, Evas_Bool update_container)
{
   Eclair_Media_File *remove_media_file;
   Evas_List *next;

   if (!playlist || !list)
      return NULL;

   if (playlist->current == list)
      eclair_playlist_current_set_list(playlist, NULL);

   if ((remove_media_file = evas_list_data(list)))
      eclair_media_file_free(remove_media_file);

   next = list->next;
   playlist->playlist = evas_list_remove_list(playlist->playlist, list);

   if (update_container && playlist->eclair && playlist->eclair->playlist_container)
      eclair_playlist_container_update(playlist->eclair->playlist_container);

   return next;
}

//Remove the selected media files
void eclair_playlist_remove_selected_media_files(Eclair_Playlist *playlist)
{
   Evas_List *l;
   Eclair_Media_File *media_file;

   if (!playlist)
      return;

   for (l = playlist->playlist; l; )
   {
      if ((media_file = l->data) && media_file->selected)
         l = eclair_playlist_remove_media_file_list(playlist, l, 0);
      else
         l = l->next;
   }

   if (playlist->eclair && playlist->eclair->playlist_container)
      eclair_playlist_container_update(playlist->eclair->playlist_container);
}

//Remove the unselected media files
void eclair_playlist_remove_unselected_media_files(Eclair_Playlist *playlist)
{
   Evas_List *l;
   Eclair_Media_File *media_file;

   if (!playlist)
      return;

   for (l = playlist->playlist; l; )
   {
      if ((media_file = l->data) && !media_file->selected)
         l = eclair_playlist_remove_media_file_list(playlist, l, 0);
      else
         l = l->next;
   }

   if (playlist->eclair && playlist->eclair->playlist_container)
      eclair_playlist_container_update(playlist->eclair->playlist_container);
}

//Empty the playlist and destroy all the media_file
void eclair_playlist_empty(Eclair_Playlist *playlist)
{
   Evas_List *l;

   if (!playlist)
      return;

   for (l = playlist->playlist; l; l = eclair_playlist_remove_media_file_list(playlist, l, 0));
   playlist->playlist = evas_list_free(playlist->playlist);
   playlist->current = NULL;

   if (playlist->eclair && playlist->eclair->playlist_container)
      eclair_playlist_container_update(playlist->eclair->playlist_container);
}

//Set the media file as the active media file  
void eclair_playlist_current_set(Eclair_Playlist *playlist, Eclair_Media_File *media_file)
{
   if (!playlist)
      return;

   if (media_file)
      eclair_playlist_current_set_list(playlist, evas_list_find_list(playlist->playlist, media_file));
   else
      eclair_playlist_current_set_list(playlist, evas_list_find_list(playlist->playlist, NULL));
}

//Set the media file stored in the list as the active media file  
void eclair_playlist_current_set_list(Eclair_Playlist *playlist, Evas_List *list)
{
   Eclair_Media_File *previous_media_file;

   if (!playlist)
      return;

   previous_media_file = evas_list_data(playlist->current);
   playlist->current = list;
   eclair_media_file_update(playlist->eclair, previous_media_file);
   eclair_media_file_update(playlist->eclair, evas_list_data(playlist->current));

   //TODO: eclair_playlist_container scroll_to
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
