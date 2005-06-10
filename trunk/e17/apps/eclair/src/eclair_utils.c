#include "eclair_utils.h"
#include <Ecore_File.h>
#include <string.h>

//Convert the progress rate in seconds to a string depending to the length of the media
void eclair_utils_second_to_string(double position, double length, char *string)
{
   if (length >= 3600)
      sprintf(string, "%d:%.2d:%.2d", (int)position / 3600, ((int)position / 60) % 60, (int)position % 60);
   else
   {
      if (length >= 600)
         sprintf(string, "%.2d:%.2d", ((int)position / 60) % 60, (int)position % 60);
      else
         sprintf(string, "%d:%.2d", ((int)position / 60) % 60, (int)position % 60);
   }
}

//TODO:
/*srand((unsigned int)time(NULL));
return (float)rand() / (float)RAND_MAX * (Max - Min) + Min;
*/

//Remove uri special chars (e.g. "%20" -> ' ')
//The returned string has to be freed
char *eclair_utils_remove_uri_special_chars(const char *uri)
{
   int uri_length;
   char *clean_uri;
   int i, j;
   int hex_to_char;

   if (!uri)
      return NULL;

   uri_length = strlen(uri);
   clean_uri = malloc(strlen(uri) + 1);
   for (i = 0, j = 0; i < uri_length; i++, j++)
   {
      if ((uri[i] == '%') && (i < (uri_length - 2)))
      {
         if (sscanf(&uri[i + 1], "%2x", &hex_to_char) == 1)
         {
            clean_uri[j] = hex_to_char;
            i += 2;
         }
         else
            clean_uri[j] = uri[i];
      }
      else
         clean_uri[j] = uri[i];
   }
   clean_uri[j] = 0;

   return clean_uri;
}

//Add the uri special chars (e.g. ' ' -> "%20")
//The returned string has to be freed
char *eclair_utils_add_uri_special_chars(const char *uri)
{
   int uri_length;
   char *new_uri;
   int i, j;

   if (!uri)
      return NULL;

   uri_length = strlen(uri);
   new_uri = malloc(uri_length * 3 + 1);

   for (i = 0, j = 0; i < uri_length; i++, j++)
   {
      if (uri[i] == ' ' || uri[i] == ';'  || uri[i] == '&'  || uri[i] == '='  ||
         uri[i] == '<'  || uri[i] == '>'  || uri[i] == '?'  || uri[i] == '@'  ||
         uri[i] == '\"' || uri[i] == '['  || uri[i] == '#'  || uri[i] == '\\' ||
         uri[i] == '$'  || uri[i] == ']'  || uri[i] == '%'  || uri[i] == '^'  ||
         uri[i] == '\'' || uri[i] == '�'  || uri[i] == '+'  || uri[i] == '{'  ||
         uri[i] == ','  || uri[i] == '|'  || uri[i] == '/'  || uri[i] == '}'  ||
         uri[i] == ':'  || uri[i] == '~')
      {
         new_uri[j] = '%';
         sprintf(&new_uri[j + 1], "%2x", uri[i]);
         j += 2;
      }
      else
         new_uri[j] = uri[i];
   }
   new_uri[j] = 0;

   return new_uri;
}

//Return the string "artist - title"
//NULL if the media file doesn't have tag
//This string has to be freed
char *eclair_utils_mediafile_to_artist_title_string(const Eclair_Media_File *media_file)
{
   char *string;

   if (!media_file || !media_file->title || strlen(media_file->title) <= 0)
      return NULL;

   if (!media_file->artist || strlen(media_file->artist) <= 0)
      string = strdup(media_file->title);
   else
   {
      string = malloc(strlen(media_file->artist) + strlen(media_file->title) + 4);
      sprintf(string, "%s - %s", media_file->artist, media_file->title);
   }

   return string;
}

//Return the extension of the file
//NULL if failed
char *eclair_utils_file_get_extension(char *file)
{
   char *ext;

   if (!file || !(ext = ecore_file_get_file(file)))
      return NULL;

   if ((ext = strrchr(ext, '.')))
   {
      ext++;
      return ext;
   }
   return NULL;
}

//Return the filename of the file without the extension
//The returned string has to be freed
//NULL if failed
char *eclair_utils_file_get_filename_without_ext(char *file)
{
   char *file_without_ext;
   char *ext_start;

   if (!file || !(file_without_ext = ecore_file_get_file(file)))
      return NULL;

   file_without_ext = strdup(file_without_ext);
   if ((ext_start = strrchr(file_without_ext, '.')))
      *ext_start = 0;

   return file_without_ext;
}

//TODO:
int eclair_utils_get_random_int(int min, int max)
{
   return (int)(((float)rand() / RAND_MAX) * (max - min) + 0.5f) + min;
}
