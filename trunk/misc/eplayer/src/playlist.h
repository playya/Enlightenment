#ifndef __PLAYLIST_H
#define __PLAYLIST_H

/*
 * $Id$
 */

#include <Evas.h>
#include "playlist_item.h"

typedef struct {
	int num; /* number of entries */
	int duration;
	Evas_List *items;
	Evas_List *cur_item;

	Evas_List *plugins; /* lists all available plugins */
	Evas *evas;
	Evas_Object *container;
	const char *theme;
} PlayList;

PlayList *playlist_new(Evas *evas, Evas_List *plugins,
                       Evas_Object *container,
                       const char *theme);
void playlist_free(PlayList *pl);

bool playlist_load_file(PlayList *pl, const char *file, bool append);
bool playlist_load_dir(PlayList *pl, const char *dir, bool append);
bool playlist_load_m3u(PlayList *pl, const char *file, bool append);
bool playlist_load_any(PlayList *pl, const char *path, bool append);

void playlist_remove_all(PlayList *pl);
void playlist_remove_item(PlayList *pl, PlayListItem *pli);

PlayListItem *playlist_current_item_get(PlayList *pl);
void playlist_current_item_set(PlayList *pl, PlayListItem *pli);

bool playlist_current_item_prev(PlayList *pl);
bool playlist_current_item_next(PlayList *pl);

bool playlist_current_item_has_prev(PlayList *pl);
bool playlist_current_item_has_next(PlayList *pl);

#endif
