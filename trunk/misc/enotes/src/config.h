
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


#ifndef CONFIG_H
#define CONFIG_H 1

#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <Ecore.h>
#include <Ecore_Config.h>

#include "debug.h"
#include "../config.h"
#include "xml.h"

typedef struct {
	char           *render_method;
	char           *theme;
	int             controlcentre;
	int             debug;
	int             autosave;
	int             welcome;
	int             ontop;
	int             sticky;
} MainConfig;

extern MainConfig *main_config;

MainConfig     *mainconfig_new(void);
void            mainconfig_free(MainConfig * p);

int             read_configuration(MainConfig * p);

#endif
