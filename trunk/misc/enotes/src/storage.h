
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


#ifndef STORAGE_H
#define STORAGE_H 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <Evas.h>		// For the list structures.

#include "note.h"
#include "debug.h"

#define DEF_VALUE_SEPERATION "|"
#define MAX_VALUE 2000
#define NOTE_LIMIT 9999

typedef struct {
	char           *content;
	int             x;
	int             y;
	int             width;
	int             height;
} NoteStor;

/* Freeing */
NoteStor       *alloc_note_stor();
void            free_note_stor(NoteStor * p);

/* One Shot Functions. :-) */
int             append_note_stor(NoteStor * p);
void            append_autosave_note_stor(NoteStor * p);
void            remove_note_stor(NoteStor * p);
void            process_note_storage_locations();


/* Autosave Functions */
void            note_load(char *target);
void            autoload(void);
void            autosave(void);

/* Internal Functions */
char           *make_storage_fn(void);
char           *make_autosave_fn(void);

NoteStor       *get_notestor_from_value(char *e);
char           *get_value_from_notestor(NoteStor * p);

#endif
