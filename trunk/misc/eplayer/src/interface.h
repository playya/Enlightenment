#ifndef __INTERFACE_H
#define __INTERFACE_H

#include "eplayer.h"

void show_playlist_item(PlayListItem *pli, void *data);
int setup_gui(ePlayer *player);
int setup_edje(ePlayer *player, const char *name);

int refresh_volume(void *udata);
int refresh_time(ePlayer *player, int time);

#endif

