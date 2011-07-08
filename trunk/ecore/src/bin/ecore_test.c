#include <Ecore.h>
#include <assert.h>
#include <unistd.h>

const char *called = NULL;

static const char *idler_str = "idler";
static const char *idle_enterer_str = "idler_enterer";
static const char *idle_exiter_str = "idler_exiter";
static const char *timer_str = "timer";
static const char *pipe_read_str = "pipe read";

int count;

Eina_Bool idle_enterer_one(void *data)
{
   fprintf(stderr, "idle enterer!\n");
   if (count)
     assert(called == timer_str);
   else
     assert(called == NULL);
   called = idle_enterer_str;
   return EINA_TRUE;
}

Eina_Bool idler_one(void *data)
{
   fprintf(stderr, "idler!\n");
   assert(called == idle_enterer_str);
   called = idler_str;
   return EINA_TRUE;
}

Eina_Bool idle_exiter_one(void *data)
{
   fprintf(stderr, "idle exiter!\n");
   assert(called == idler_str);
   called = idle_exiter_str;
   return EINA_TRUE;
}

Eina_Bool timer_one(void *data)
{
   fprintf(stderr, "timer\n");
   assert(called == pipe_read_str);
   called = timer_str;

   count++;
   if (count == 10)
     {
       ecore_main_loop_quit();
       return EINA_FALSE;
     }

   return EINA_TRUE;
}

Eina_Bool pipe_read(void *data, Ecore_Fd_Handler *fd_handler)
{
   fprintf(stderr, "pipe read\n");
   assert(called == idle_exiter_str);
   called = pipe_read_str;

   return EINA_TRUE;
}

int main(int argc, char **argv)
{
   int fds[2];

   assert(0 == pipe(fds));

   assert(1 == write(fds[1], "x", 1));

   ecore_init();

   ecore_timer_add(0.0, timer_one, NULL);
   ecore_main_fd_handler_add(fds[0], ECORE_FD_READ, pipe_read, NULL, NULL, NULL);

   ecore_idle_enterer_add(&idle_enterer_one, NULL);
   ecore_idler_add(&idler_one, NULL);
   ecore_idle_exiter_add(&idle_exiter_one, NULL);

   ecore_main_loop_begin();

   /* FIXME?: glib main loop exits on an idle enterer */
   //assert(called == idle_enterer_str);

   ecore_shutdown();
   return 0;
}
