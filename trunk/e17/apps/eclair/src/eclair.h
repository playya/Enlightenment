#ifndef _ECLAIR_H_
#define _ECLAIR_H_

#include "eclair_private.h"
#include "eclair_playlist.h"

//core functions
Evas_Bool eclair_init(Eclair *eclair, int *argc, char *argv[]);
void eclair_shutdown(Eclair *eclair);
void eclair_update(Eclair *eclair);
void *eclair_file_chooser_thread(void *param);
void eclair_current_file_set(Eclair *eclair, const Eclair_Playlist_Media_File *file);
void eclair_progress_rate_set(Eclair *eclair, double progress_rate);
void eclair_playlist_container_scroll(Eclair *eclair, int num_entries);
void eclair_playlist_container_scroll_percent_set(Eclair *eclair, double percent);

//navigation functions
void eclair_play_file(Eclair *eclair, const char *path);
void eclair_play_current(Eclair *eclair);
void eclair_play_prev(Eclair *eclair);
void eclair_play_next(Eclair *eclair);
void eclair_pause(Eclair *eclair);
void eclair_play(Eclair *eclair);
void eclair_stop(Eclair *eclair);

#endif
