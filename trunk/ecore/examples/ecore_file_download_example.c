#include <stdio.h>
#include <Ecore.h>
#include <Ecore_File.h>

/* 
 * ecore_file_download() example
 *
 * compile with:
 * gcc ecore_file_download_example.c `pkg-config --libs --cflags ecore-file` \
 *     -o ecore_file_download_example
 *
 */

#define URL "http://cdimage.ubuntu.com/releases/10.10/release/ubuntu-10.10-dvd-i386.iso.zsync"
#define DST "ubuntu.zsync"


void
completion_cb(void *data, const char *file, int status)
{
   printf("Done (status: %d)\n", status);
   ecore_main_loop_quit();
}

int
progress_cb(void *data, const char *file,
            long int dltotal, long int dlnow,
            long int ultotal, long int ulnow)
{
   printf("Progress: %ld/%ld\n", dlnow, dltotal);
   return 0; // 0 to continue the download, or 1 to abort
}


int main(void)
{
   double start;

   eina_init();
   ecore_init();
   ecore_file_init();

   if (ecore_file_exists(DST))
     ecore_file_unlink(DST);

   start = ecore_time_get();

   if (ecore_file_download(URL, DST, completion_cb, progress_cb, NULL, NULL))
   {
      printf("Download started successfully:\n  URL: %s\n  DEST: %s\n", URL, DST);
      ecore_main_loop_begin();
      printf("\nTime elapsed: %f seconds\n", ecore_time_get() - start);
      printf("Downloaded %lld bytes\n", ecore_file_size(DST));
   }
   else
   {
       printf("Error, can't start download\n");
       return 1;
   }

   ecore_file_shutdown();
   ecore_shutdown();
   eina_shutdown();
   return 0;
}
