/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "edje_viewer.h"

int main(int argc, char **argv)
{
   if (!etk_init())
     {
	fprintf(stderr, "Could not init etk. Exiting...\n");
	return 0;
     };

   if (argc == 2) main_window_show(argv[1]);
   else main_window_show(NULL);

   etk_main();
   etk_shutdown();

   return 0;
}
