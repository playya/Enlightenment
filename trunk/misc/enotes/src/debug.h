
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


#ifndef DEBUG_H
#define DEBUG_H 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"


#define DEBUG_MSG "E-Notes: %s.\n"
#define DEBUG_FUNC_IN "E-Notes [%d]: Entered Function %s.\n"
#define DEBUG_FUNC_OUT "E-Notes [%d]: Exited Function %s.\n"

#define debug_func_in(foo) dfi(foo)
#define debug_func_out(foo) dfo(foo)

#define debug_msg(foo) dm(foo)
#define debug_msg_lvl(foo,lvl) dml(foo,lvl)

typedef struct {
	char           *name;
	int             level;
} DebugFuncLst;

extern DebugFuncLst func_list[];


void            debug_msg_lvl(char *msg, int level);
void            debug_msg(char *msg);

/* These are only really used when manually debugging and programming enotes.
 * We don't use this normally because it won't print messages before the configuration
 * and usage has been parsed.. it needs the debug level. */
void            debug_func_in(char *function);
void            debug_func_out(char *function);

#endif
