/* Example of how to set and retrieve program arguments.
 */
#include <Ecore.h>
 
int timer_once(void *data)
{
  int argc;
  char **argv;
  int i;

  ecore_app_args_get(&argc, &argv);
  for (i = 0; i < argc; i++) printf("ARG %i: %s\n", i, argv[i]);
  ecore_main_loop_quit();
  return 1;
}
 
int main(int argc, char **argv)
{
  ecore_init();
  ecore_app_args_set(argc, argv);
  ecore_timer_add(2.0, timer_once, NULL);
  ecore_main_loop_begin();
  ecore_shutdown();
  return 0;
}
