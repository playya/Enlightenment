#include <evfs.h>
#include <string.h>

static int mon_current = 0;     /*A demo of stopping monitoring, after 10 events */
evfs_file_uri_path *dir_path;
evfs_connection *con;

void
callback(evfs_event * data, void *obj)
{
   if (data->type == EVFS_EV_FILE_MONITOR)
     {
        printf("DEMO: Received a file monitor notification\n");
        printf("DEMO: For file: '%s'\n", data->file_monitor.filename);
        mon_current++;
     }
   else if (data->type == EVFS_EV_STAT)
     {
        printf("Received stat event for file '%s'!\n",
               data->resp_command.file_command.files[0]->path);
        printf("File size: %llu\n", data->stat.stat_obj.st_size);
        //printf("File inode: %ld\n", data->stat.stat_obj.st_ino);
        printf("File uid: %d\n", data->stat.stat_obj.st_uid);
        printf("File gid: %d\n", data->stat.stat_obj.st_gid);
        printf("Last access: %d\n", data->stat.stat_obj.ist_atime);
        printf("Last modify : %d\n", data->stat.stat_obj.ist_mtime);
     }
   else if (data->type == EVFS_EV_DIR_LIST)
     {
        evfs_filereference *ref;

        printf("Received a directory listing..\nFiles:\n\n");

        ecore_list_first_goto(data->file_list.list);
        while ((ref = ecore_list_next(data->file_list.list)))
          {
             printf("(%s) Received file type for file: %d\n", ref->path,
                    ref->file_type);
          }

     } else if (data->type == EVFS_EV_METADATA)  {
	     printf("Received metadata:\n");

	     printf("Artist: '%s'\n", (char *)ecore_hash_get(data->meta->meta_hash, "artist"));
	     printf("Title: '%s'\n", (char *)ecore_hash_get(data->meta->meta_hash, "title"));
	     printf("Length: '%s'\n", (char *)ecore_hash_get(data->meta->meta_hash, "length"));
	     
			     
	     
     }

   /*if (mon_current == 2) {
    * static char str_data[1024];
    * snprintf(str_data,1024,"file://%s/newfile", getenv("HOME"));
    * 
    * evfs_file_uri_path* path = evfs_parse_uri(str_data);
    * printf("Removing monitor...\n");
    * evfs_monitor_remove(con, dir_path->files[0]);
    * 
    * printf("DEMO: Removing HOME/newfile\n");
    * evfs_client_file_remove(con, path->files[0]);
    * 
    * 
    * } */

   exit(0);
}

int
main(int argc, char **argv)
{

   char pathi[1024];
   char *patharg = NULL;
   char *cmd = NULL;
   int i;

   for (i = 1; i < argc; i++)
     {
        if (!strcmp(argv[i], "-u"))
          {
             if (++i < argc)
               {
                  patharg = strdup(argv[i]);
               }
             else
               {
                  printf("The option \"-u\" requires a valid URI\n");
                  return 1;
               }
          }
        else
          {
             if (!cmd)
               {
                  cmd = strdup(argv[i]);
               }
             else
               {
                  printf("Error: Enter only one command.\n");
                  return 1;
               }
          }

     }

   printf("EVFS Demo system..\n");

   /*Check if the user entered a command.  TODO: Add command functionality. */
   if (!cmd)
     {
        printf("You did not enter a command. Defaulting to DIR.\n");
        cmd = strdup("DIR");
     }

   if (!patharg)
     {
        snprintf(pathi, 1024, "file://%s", getenv("HOME"));
     }
   else
     {
        snprintf(pathi, 1024, "%s", patharg);
     }

   con = evfs_connect(&callback, NULL);

   //path = evfs_parse_uri("file:///dev/ttyS0");

   printf("Listing dir: %s\n", pathi);
   dir_path = evfs_parse_uri(pathi);

   printf("Plugin uri is '%s', for path '%s'\n\n",
          dir_path->files[0]->plugin_uri, dir_path->files[0]->path);

   /*evfs_monitor_add(con, dir_path->files[0]);
    * evfs_client_file_copy(con, dir_path->files[0], NULL); */

   if (!strcmp(cmd, "DIR")) {
	   evfs_client_dir_list(con, dir_path->files[0]);
   } else if (!strcmp(cmd, "STAT")) {
	   evfs_client_file_stat(con, dir_path->files[0]);
   } else if (!strcmp(cmd, "META")) {
	   evfs_client_metadata_retrieve(con, dir_path->files[0]);
   } else if (!strcmp(cmd, "METASET")) {
	   evfs_client_metadata_string_file_set(con, dir_path->files[0], "entropy_folder_preference", "icon");
   } else if (!strcmp(cmd, "METAGET")) {
           evfs_client_metadata_string_file_get(con, dir_path->files[0], "entropy_folder_preference");
   }
   ecore_main_loop_begin();
   evfs_disconnect(con);
}
