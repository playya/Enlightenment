/** @file emphasis_mpc.h */
#ifndef EMPHASIS_MPC_H_
#define EMPHASIS_MPC_H_

/**
 * @defgroup mpc
 * @{
 */

/**
 * @var mo
 * @brief It's the global object using the connexion to the mpd daemon
 */
MpdObj *mo;

Ecore_Timer *mpc_init(const char *hostname, int port, const char *password);
void mpc_signal_connect_status_changed(void *data);
int mpc_update(void *data);
void status_changed_callback(MpdObj *mo, ChangedStatusType what, void *data);
void mpc_connection_changed(MpdObj *mo, int connect, void *data);

int mpc_assert_status(MpdState status);

Evas_List *mpc_mlib_artist_get(void);
Evas_List *mpc_mlib_album_get(char *artist);
Evas_List *mpc_mlib_track_get(char *album, char *artist);

void mpc_playlist_add(Evas_List *list);
void mpc_playlist_delete(Evas_List *list);
void mpc_playlist_clear(void);

void mpc_play_id(int id);
void mpc_play(void);
void mpc_pause(void);
void mpc_stop(void);
void mpc_prev(void);
void mpc_next(void);
void mpc_seek(double percent);

void mpc_play_if_stopped(void);
void mpc_toggle_play_pause(void);
void mpc_toggle_random(void);
void mpc_toggle_repeat(void);

int mpc_get_vol(void);
void mpc_change_vol(int value);
void mpc_database_update(void);
void mpc_disconnect(void);

Evas_List *mpc_list_playlists(void);
Evas_List *mpc_get_playlist_content(char *playlist_name);
void mpc_save_playlist(char *playlist_name);
void mpc_load_playlist(char *playlist_name);
void mpc_delete_playlist(char *playlist_name);

#if defined(LIBMPD_0_12_4)
#include <libmpd/libmpd-internal.h>
MpdData *mpd_database_list_playlist(MpdObj *mi);
#endif
/** @} */

#endif /* EMPHASIS_MPC_H_ */
