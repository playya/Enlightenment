#include "eclair_subtitles.h"
#include <stdio.h>
#include <string.h>
#include <Evas.h>

int _eclair_subtitles_sort_cb(void *sub1, void *sub2);

//Init subtitles module
void eclair_subtitles_init(Eclair_Subtitles *subtitles)
{
   if (!subtitles)
      return;

   subtitles->subtitles = NULL;
   subtitles->enable = 0;
}

//Free the loaded subtitles
void eclair_subtitles_free(Eclair_Subtitles *subtitles)
{
   Evas_List *l;
   Eclair_Subtitle *sub;

   if (!subtitles)
      return;

   for (l = subtitles->subtitles; l; l = l->next)
   {
      sub = (Eclair_Subtitle *)l->data;
      if (sub)
         free(sub->subtitle);
   }
   evas_list_free(subtitles->subtitles);
   subtitles->subtitles = NULL;
}

//Return current subtitle (may be NULL)
char *eclair_subtitles_get_current_subtitle(Eclair_Subtitles *subtitles, double current_time)
{
   Evas_List *l;
   Eclair_Subtitle *subtitle;

   if (!subtitles)
      return NULL;
   if (!subtitles->subtitles)
      return NULL;

   for (l = subtitles->subtitles; l; l = l->next)
   {
      if (!(subtitle = (Eclair_Subtitle *)evas_list_data(l)))
         continue;

      if (subtitle->start <= current_time && subtitle->end >= current_time)
         return subtitle->subtitle;
   }

   return NULL;
}

//Display subtitles corresponding to the current time
void eclair_subtitles_display_current_subtitle(Eclair_Subtitles *subtitles, double current_time, Evas_Object *subtitles_object)
{
   char *current_subtitle, *sub, *new_line, *end_of_line;
   Evas_Coord subtitle_height, evas_width, evas_height;

   if (!subtitles || !subtitles_object)
      return;

   if (!subtitles->enable)
      return;

   evas_object_textblock_clear(subtitles_object);

   if (!(current_subtitle = eclair_subtitles_get_current_subtitle(subtitles, current_time)))
      return;

   evas_object_textblock_format_insert(subtitles_object, "color=#ffffffff outline_color=#000000ff style=outline font='Vera-Bold' size=24 wrap=word align=center valign=bottom");
   sub = strdup(current_subtitle);
   new_line = sub;
   while (new_line)
   {
      end_of_line = strpbrk(new_line, "\r\n");
      if (end_of_line)
         *end_of_line = 0;
      evas_object_textblock_text_insert(subtitles_object, new_line);
      if (end_of_line)
      {
         new_line = end_of_line + 1;
         if (*new_line == '\n' || *new_line == '\r')
            new_line++;
         evas_object_textblock_format_insert(subtitles_object, "\n");
      }
      else
         new_line = NULL;
   }
   free(sub);

   evas_output_size_get(evas_object_evas_get(subtitles_object), &evas_width, &evas_height);
	evas_object_textblock_format_size_get(subtitles_object, NULL, &subtitle_height);
   evas_object_move(subtitles_object, 0, evas_height - subtitle_height);
   evas_object_resize(subtitles_object, evas_width, subtitle_height);
}

//Return 0 if the two subtitles starts at the same time
//-1 if data1 starts before data2
//1 if data1 starts after data2
int _eclair_subtitles_sort_cb(void *data1, void *data2)
{
   Eclair_Subtitle *sub1, *sub2;

   if (!data1)
      return 1;
   if (!data2)
      return -1;

   sub1 = (Eclair_Subtitle *)data1;
   sub2 = (Eclair_Subtitle *)data2;

   if (sub1->start < sub2->start)
      return 1;
   if (sub1->start == sub2->start)
      return 0;
   return -1;
}

//Sort subtitles by start time
void eclair_subtitles_sort(Eclair_Subtitles *subtitles)
{
   if (!subtitles)
      return;

   if (!subtitles->subtitles)
      return;

   subtitles->subtitles = evas_list_sort(subtitles->subtitles, evas_list_count(subtitles->subtitles), _eclair_subtitles_sort_cb);
}

//Try to load subtitles from the media filename
Evas_Bool eclair_subtitles_load_from_media_file(Eclair_Subtitles *subtitles, const char *media_file)
{
   char *subtitles_filename;
   char *extension_start;
   int extension_start_id;

   if (!subtitles || !media_file)
      return 0;

   if (!(extension_start = rindex(media_file, '.')))
      return 0;
   
   //ugly... :(
   extension_start_id = extension_start - media_file + 1;

   subtitles_filename = (char *)malloc(strlen(media_file) + 10);
   strcpy(subtitles_filename, media_file);

   //Try to load srt subtitles
   subtitles_filename[extension_start_id] = 's';
   subtitles_filename[extension_start_id + 1] = 'r';
   subtitles_filename[extension_start_id + 2] = 't';
   subtitles_filename[extension_start_id + 3] = 0;
   subtitles->enable = eclair_subtitles_load_srt(subtitles, subtitles_filename);

   free(subtitles_filename);

   return subtitles->enable;
}

//Load subtitles from a srt file
Evas_Bool eclair_subtitles_load_srt(Eclair_Subtitles *subtitles, const char *path)
{
   FILE *subtitles_file;
   char line[4096];
   int tmp;
   Evas_Bool have_num, have_time, have_text;
   int start[4], end[4];
   Eclair_Subtitle *new_subtitle;

   have_num = have_time = have_text = 0;
   new_subtitle = NULL;

   if (!subtitles || !path)
      return 0;

   eclair_subtitles_free(subtitles);

   if (!(subtitles_file = fopen(path, "rt")))
      return 0;

   while (fgets(line, 4096, subtitles_file))
   {
      if (strlen(line) <= 2)
         continue;

      if (!have_num || have_text)
      {
         if (sscanf(line, "%d\n", &tmp) == 1)
         {
            if (new_subtitle)
            {
               if (new_subtitle->subtitle[strlen(new_subtitle->subtitle) - 1] == '\n' ||
                  new_subtitle->subtitle[strlen(new_subtitle->subtitle) - 1] == '\r')
                  new_subtitle->subtitle[strlen(new_subtitle->subtitle) - 1] = 0;
               if (new_subtitle->subtitle[strlen(new_subtitle->subtitle) - 1] == '\n' ||
                  new_subtitle->subtitle[strlen(new_subtitle->subtitle) - 1] == '\r')
                  new_subtitle->subtitle[strlen(new_subtitle->subtitle) - 1] = 0;
               subtitles->subtitles = evas_list_append(subtitles->subtitles, new_subtitle);
            }
            new_subtitle = (Eclair_Subtitle *)calloc(1, sizeof(Eclair_Subtitle));
            have_time = have_text = 0;
            have_num = 1;
            continue;
         }
      }
      if (!have_num)
         continue;
      
      if (!have_time)
      {
         if (sscanf(line, "%d:%d:%d,%d --> %d:%d:%d,%d", &start[0], &start[1], 
            &start[2], &start[3], &end[0],  &end[1],  &end[2],  &end[3]) == 8)
         {
            have_time = 1;
            new_subtitle->start = start[0] * 3600 + start[1] * 60 + start[2] + start[3] * 0.001;
            new_subtitle->end = end[0] * 3600 + end[1] * 60 + end[2] + end[3] * 0.001;
            continue;
         }
      }
      if (!have_time)
         continue;

      if (new_subtitle->subtitle)
      {
         new_subtitle->subtitle = (char *)realloc(new_subtitle->subtitle, strlen(new_subtitle->subtitle) + strlen(line) + 1);
         strcat(new_subtitle->subtitle, line);
      }
      else
         new_subtitle->subtitle = strdup(line);
      have_text = 1;
   }

   if (new_subtitle && have_text)
   {
      if (new_subtitle->subtitle[strlen(new_subtitle->subtitle) - 1] == '\n' ||
         new_subtitle->subtitle[strlen(new_subtitle->subtitle) - 1] == '\r')
         new_subtitle->subtitle[strlen(new_subtitle->subtitle) - 1] = 0;
      if (new_subtitle->subtitle[strlen(new_subtitle->subtitle) - 1] == '\n' ||
         new_subtitle->subtitle[strlen(new_subtitle->subtitle) - 1] == '\r')
         new_subtitle->subtitle[strlen(new_subtitle->subtitle) - 1] = 0;
      subtitles->subtitles = evas_list_append(subtitles->subtitles, new_subtitle);
   }

   fclose(subtitles_file);

   eclair_subtitles_sort(subtitles);

   return 1;
}
