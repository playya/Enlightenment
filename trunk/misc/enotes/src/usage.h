
/**************************************************
 **               E  -  N O T E S                **
 **                                              **
 **  The contents of this file are released to   **
 **  the public under the General Public Licence **
 **  Version 2.                                  **
 **                                              **
 **  By  Thomas Fletcher (www.fletch.vze.com)    **
 **                                              **
 **************************************************/


#ifndef USAGE_H
#define USAGE_H 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "debug.h"
#include "config.h"
#include "../config.h"
#include "ipc.h"

#define USAGE "\
\
E-Notes\n\
By Thomas Fletcher\n\n\
Command Line Arguments:\n\
\n\n\
POSIX  |  GNU             | TYPE  | DESCRITION\n\
----------------------------------------------------------------\n\
-?     |  --help          | N/A   | Display the Usage.\n\
-v     |  --version       | N/A   | Display the Version.\n\
       |                  |       |\n\
-R     |  --remote        | STR   | Send a remote message to\n\
       |                  |       | a running E-Notes.\n\
       |                  |       |\n\
-c     |  --config-file   | STR   | Configuration File.\n\
-d     |  --debug         | INT   | Set the debugging level [0-2].\n\
       |                  |       |\n\
-r     |  --render-method | STR   | Render Method\n\
-t     |  --theme         | STR   | Theme\n\
-C     |  --control-centre| INT   | Enable/Disable the Control\n\
       |                  |       | Centre.\n\
-A     |  --auto-save     | INT   | Enable the autosaving and\n\
       |                  |       | loading of notes.\n\
-s     |  --sticky        | INT   | Make the notes sticky?\n\
-o     |  --ontop         | INT   | Keep the note ontop?\n\n\
-w     |  --welcome       | INT   | Welcome You?\n\
\
\n"

#define USAGE_VERSION "E-Notes Version:\n%s\n"

#define OPTSTR "v?hc:r:t:R:d:A:w:C:s:o:"

static struct option long_options[] = {
	{"help", 0, 0, '?'},
	{"version", 0, 0, 'v'},
	{"remote", 1, 0, 'R'},
	{"config-file", 1, 0, 'c'},
	{"render-method", 1, 0, 'r'},
	{"theme", 1, 0, 't'},
	{"control-centre", 1, 0, 'C'},
	{"debug", 1, 0, 'd'},
	{"auto-save", 1, 0, 'A'},
	{"welcome", 1, 0, 'w'},
	{"sticky", 1, 0, 's'},
	{"ontop", 1, 0, 'o'},
	{NULL, 0, 0, 0}
};

extern char    *optarg;

/* Reading the Usage */
void            read_usage_configuration(MainConfig * p, int argc,
					 char *argv[]);
char           *read_usage_for_configuration_fn(int argc, char *argv[]);

/* Printing the Usage */
void            print_usage(void);

/* External Variables */
extern int      dispusage;

#endif
