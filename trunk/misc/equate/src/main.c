#include "Equate.h"

void
print_usage(void)
{
   printf("Equate - a calculator for Enlightenment\n");
   printf("Version 0.0.1 (Dec 3 2003)\n");
   printf("(c)2003 by HandyAndE.\n");
   printf("Usage: equate [options]\n\n");
   printf("Supported Options:\n");
   printf("-h, --help              Print this help text\n");
   printf("-e, --exec        <str> Execute an equation and exit\n");
   printf("-b, --basic             Use Equate in basic mode (default)\n");
   printf("-s, --scientific        Use Equate in scientific mode\n");
   exit(0);
}

void
exec(char *exe)
{
   if (!exe) {
      fprintf(stderr, "Error: --exec needs an addtional argument\n");
      exit(1);
   }

   equate_append(exe);
   printf("%.10g\n", equate_eval());
   exit(0);
}

void
equate_init(Equate * equate)
{
   math_init();
}

void
equate_quit(void)
{
   ewl_main_quit();
}

int
main(int argc, char *argv[], char *env[])
{
   Equate          equate;
   int             nextarg = 1;
   char           *arg;

   while (nextarg < argc) {
      arg = argv[nextarg];
      if (!strcmp(arg, "--scientific") || !strcmp(arg, "-s"))
         equate.conf.mode = SCI;
      else if (!strcmp(arg, "--basic") || !strcmp(arg, "-b"))
         equate.conf.mode = BASIC;
      else if (!strcmp(arg, "--exec") || !strcmp(arg, "-e")) {
         exec(argv[++nextarg]);
      }


      else if (!strcmp(arg, "--help") || !strcmp(arg, "-h"))
         print_usage();
      else {
         printf("Unrecognised option \"%s\"\n\n", arg);
         print_usage();
      }
      nextarg++;
   }

   equate_init(&equate);

   ewl_init(&argc, argv);

   init_gui(&equate);

   return 0;
}
