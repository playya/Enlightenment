#include <evfs.h>
#include <string.h>

evfs_connection *con;

void
callback (evfs_event * data, void *obj)
{


  switch (data->type) {
  case EVFS_EV_FILE_OPEN:
    //printf("File open event..fd is %d, reading..\n", data->resp_command.file_command.files[0]->fd);

    evfs_client_file_read (con, data->resp_command.file_command.files[0],
			   4096);

    break;

  case EVFS_EV_FILE_READ:
    //printf("File read event, size is %ld\n", data->data.size);
    if (data->data.size > 0) {
      fwrite (data->data.bytes, data->data.size, 1, stdout);
      evfs_client_file_read (con,
			     data->resp_command.file_command.files[0], 4096);
    }
    else {
      exit (0);
    }



    break;
  default:
    printf ("Unknown event\n");
    break;
  }
}

int
main (int argc, char **argv)
{

  evfs_file_uri_path *path;
  char pathi[1024];

  if (argc < 2) {
    return 0;
  }

  if ((con = evfs_connect (&callback, NULL))) {
    path = evfs_parse_uri (argv[1]);

    evfs_client_file_open (con, path->files[0]);
  }
  else {
    printf ("evfscat: failure to connect to evfs server\n");
  }

  ecore_main_loop_begin ();
  evfs_disconnect (con);

  return 0;
}
