/* Timer example.
 */

#include <Ecore.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int size = 0;
int exe_count = 0;
int data_count = 0;
int line_count = 0;
int one_percent = 0;
Ecore_Exe *exe0 = NULL;
Ecore_Exe *exe1 = NULL;
Ecore_Exe *exe2 = NULL;
Ecore_Exe *exe3 = NULL;

static int
exe_data(void *data, int type, void *event)
{
   Ecore_Event_Exe_Data *ev;
   int i;

   ev = event;
   printf("  [*] DATA RET EXE %p - %p [%i bytes]\n", ev->exe, ev->data, ev->size);

   if (ev->lines)
      {
         int i;
	       
	 for (i = 0; ev->lines[i].line != NULL; i++)
	    {
               printf("%d %s\n", ev->lines[i].size, ev->lines[i].line);
	    }
      }
   else
      {
         for (i = 0; i < ev->size; i++)
            putchar(((unsigned char *)ev->data)[i]);
      }
   printf("\n");
   return 1;
}

static int
exe_data_count(void *data, int type, void *event)
{
   Ecore_Event_Exe_Data *ev;
   int i;

   ev = event;

   if (ev->lines)
      {
         int i;
	       
	 for (i = 0; ev->lines[i].line != NULL; i++)
	    line_count++;
         printf("%d ", i);
      }

   for (i = 0; i < ev->size; i++)
      {
         data_count++;
	 if ((data_count % one_percent) == 0)
	    {
               putchar('.');
	       fflush(stdout);
	    }
      }

   if (data_count >= size)
      {
         printf("\n");
         /* Since there does not seem to be anyway to convince /bin/cat to finish... */
	 ecore_exe_terminate(exe0);
      }

   return 1;
}

static int
exe_exit(void *data, int type, void *event)
{
   Ecore_Event_Exe_Exit *ev;

   ev = event;
   printf("  [*] EXE EXIT: %p\n", ev->exe);
   exe_count--;
   if (exe_count <= 0)
      ecore_main_loop_quit();
   return 1;
}

int main(int argc, char **argv) 
{
   double then = 0.0, now = 0.0;

   ecore_init();
   ecore_event_handler_add(ECORE_EVENT_EXE_EXIT, exe_exit, NULL);

   if (argc == 1)
      {
         ecore_event_handler_add(ECORE_EVENT_EXE_DATA, exe_data, NULL);
         exe0 = ecore_exe_run("/bin/uname -a", NULL);
         if (exe0)   exe_count++;

         exe1 = ecore_exe_pipe_run("/bin/sh",
			    ECORE_EXE_PIPE_READ | ECORE_EXE_PIPE_WRITE,
			    NULL);
         if (exe1)
	    {
	       exe_count++;
               ecore_exe_pipe_write(exe1, "ls\n", 3);
               ecore_exe_pipe_write(exe1, "exit\n", 5);
	    }

         exe2 = ecore_exe_pipe_run("/usr/bin/find . -print",
			    ECORE_EXE_PIPE_READ | ECORE_EXE_PIPE_READ_LINE_BUFFERED,
			    NULL);
         if (exe2)   exe_count++;

         exe3 = ecore_exe_pipe_run("/bin/cat",
			    ECORE_EXE_PIPE_WRITE,
			    NULL);
         if (exe3)
	    {
	       exe_count++;
               ecore_exe_pipe_write(exe3, "ls\n", 3);
	    }

         printf("  [*] exe0 = %p (/bin/uname -a)\n", exe0);
         printf("  [*] exe1 = %p (echo \"ls\" | /bin/sh)\n", exe1);
         printf("  [*] exe2 = %p (/usr/bin/find / -print)\n", exe2);
         printf("  [*] exe3 = %p (echo \"ls\" | /bin/cat)\n", exe3);
      }
   else
      {
         int i = 1;

         ecore_event_handler_add(ECORE_EVENT_EXE_DATA, exe_data_count, NULL);
	 printf("FILE : %s\n", argv[i]);
         exe0 = ecore_exe_pipe_run("/bin/cat",
	    ECORE_EXE_PIPE_WRITE | ECORE_EXE_PIPE_READ | ECORE_EXE_PIPE_READ_LINE_BUFFERED,
	    NULL);
         if (exe0)
            {
	       struct stat s;

	       exe_count++;
               if (stat(argv[i], &s) == 0)
		  {
		     int fd;

                     size = s.st_size;
                     one_percent = s.st_size / 100;
		     if (one_percent == 0)
		        one_percent = 1;
		     if ((fd = open(argv[i], O_RDONLY)) != -1)
		        {
		           char buf[1024];
			   int length;
			   while ((length = read(fd, buf, 1024)) > 0)
                              ecore_exe_pipe_write(exe0, buf, length);
		           close(fd);
		        }
	          }
	       /* FIXME: Fuckit, neither of these will actually cause /bin/cat to shut down.  What the fuck does it take? */
               ecore_exe_pipe_write(exe0, "\004", 1);  /* Send an EOF. */
               ecore_exe_pipe_write_close(exe0);  /* /bin/cat should stop when it's stdin closes. */
            }
      }

   if (one_percent)
      then = ecore_time_get() + 0.1;  /* Factor in the exe exit delay at least. */

   if (exe_count > 0)
      ecore_main_loop_begin();

   if (one_percent)
      {
         now = ecore_time_get();
	 printf("Approximate data rate (overhead not accounted for) - %f bytes/second ( %d lines and %d bytes in %f seconds).\n", ((double) data_count) / (now - then), line_count, data_count, now - then);
	 if (data_count != size)
	    printf("Size discrepency of %d bytes.\n", size - data_count);
      }

   ecore_shutdown();
   return 0;
}
