#include <config.h>
#include <Edje.h>
#include <Esmart/container.h>
#include <assert.h>
#include "eplayer.h"
#include "track.h"
#include "interface.h"
#include "utils.h"

static int paused = 0;

/**
 * Starts/resumes playback.
 *
 * @param player
 * @param e
 * @param o
 * @param event
 */
void cb_play(ePlayer *player, Evas *e, Evas_Object *o, void *event) {
	debug(DEBUG_LEVEL_INFO, "Play callback entered\n");

	if (!paused) { /* restart from beginning */
		eplayer_playback_stop(player);
		eplayer_playback_start(player, 1);
	} else { /* continue playback */
		eplayer_playback_start(player, 0);
		paused = 0;
	}
}

/**
 * Pauses/resumes playback.
 *
 * @param player
 * @param e
 * @param o
 * @param event
 */
void cb_pause(ePlayer *player, Evas *e, Evas_Object *o, void *event) {
	debug(DEBUG_LEVEL_INFO, "Pause callback entered\n");
	
	if (paused)
		eplayer_playback_start(player, 0);
	else
		eplayer_playback_stop(player);

	paused = !paused;
}

/**
 * Moves to the next track and plays it, except when we're going
 * back to the beginning of the playlist.
 *
 * @param player
 * @param e
 * @param o
 * @param event
 */
void cb_track_next(ePlayer *player, Evas *e, Evas_Object *o,
                   void *event) {
	int play = 1;
	
	debug(DEBUG_LEVEL_INFO, "Next File Called\n");

	eplayer_playback_stop(player);

	/* check whether we moved to the beginning of the list */
	if (playlist_current_item_next(player->playlist))
		play = player->cfg.repeat;

	if (play) {
		eplayer_playback_start(player, 1);
		paused = 0;
	} else /* refresh track info parts, but don't start playing yet */
		track_open(player);
}

/**
 * Moves to the previous track and plays it, except when we're
 * at the first track already.
 *
 * @param player
 * @param e
 * @param o
 * @param event
 */
void cb_track_prev(ePlayer *player, Evas *e, Evas_Object *o,
                   void *event) {
	debug(DEBUG_LEVEL_INFO, "Previous File Called\n");

	/* first item on the list: do nothing */
	if (!playlist_current_item_has_prev(player->playlist))
		return;

	eplayer_playback_stop(player);
	
	/* Get the previous list item */
	playlist_current_item_prev(player->playlist);

	eplayer_playback_start(player, 1);
	paused = 0;
}

void cb_volume_raise(ePlayer *player, Evas_Object *obj,
                     const char *emission, const char *src) {
	int left = 0, right = 0;
	
	debug(DEBUG_LEVEL_INFO, "Raising volume\n");

	if (!player->output->volume_get(&left, &right))
		return;
	
	player->output->volume_set(left + 5, right + 5);
	ui_refresh_volume(player);
}

void cb_volume_lower(ePlayer *player, Evas_Object *obj,
                     const char *emission, const char *src) {
	int left = 0, right = 0;
	
	debug(DEBUG_LEVEL_INFO, "Lowering volume\n");
	
	if (!player->output->volume_get(&left, &right))
		return;
	
	player->output->volume_set(left - 5, right - 5);
	ui_refresh_volume(player);
}

void cb_time_display_toggle(ePlayer *player, Evas_Object *obj,
                            const char *emission, const char *src) {
	player->cfg.time_display = !player->cfg.time_display;
	track_update_time(player);
}

void cb_repeat_mode_toggle(ePlayer *player, Evas_Object *obj,
                           const char *emission, const char *src) {
	player->cfg.repeat = !player->cfg.repeat;
}

void cb_playlist_scroll_up(void *udata, Evas_Object *obj,
                           const char *emission, const char *src) {
	ePlayer *player = udata;

	/* it's * 3 because we're scrolling 3 elements at once */
	e_container_scroll(player->gui.playlist,
	                   player->gui.playlist_font_size * 3);
}

void cb_playlist_scroll_down(void *udata, Evas_Object *obj,
                             const char *emission, const char *src) {
	ePlayer *player = udata;

	/* it's * 3 because we're scrolling 3 elements at once */
	e_container_scroll(player->gui.playlist,
	                   player->gui.playlist_font_size * -3);
}

void cb_playlist_item_play(void *udata, Evas_Object *obj,
                           const char *emission, const char *src) {
	ePlayer *player = udata;
	PlayListItem *pli = evas_object_data_get(obj, "PlayListItem");

	eplayer_playback_stop(player);

	playlist_current_item_set(player->playlist, pli);
	eplayer_playback_start(player, 1);
	paused = 0;
}

void cb_playlist_item_selected(void *udata, Evas_Object *obj,
                               const char *emission, const char *src) {
	ePlayer *player = udata;
	Evas_List *items = e_container_elements_get(player->gui.playlist);
	Evas_List *l;

	for (l = items; l; l = l->next)
		if (l->data != obj)
			edje_object_signal_emit(l->data,
			                        "PLAYLIST_ITEM_UNSELECTED", "");
}

void cb_seek_forward(void *udata, Evas_Object *obj,
                     const char *emission, const char *src) {
	ePlayer *player = udata;
	PlayListItem *pli = playlist_current_item_get(player->playlist);

	debug(DEBUG_LEVEL_INFO, "Seeking forward\n");

	/* We don't care if you seek past the file, the play loop
	 * will catch EOF and play next file
	 */
	eplayer_playback_stop(player);
	pli->plugin->set_current_pos(pli->plugin->get_current_pos() + 5);
	eplayer_playback_start(player, 0);
	paused = 0;
}

void cb_seek_backward(void *udata, Evas_Object *obj,
                      const char *emission, const char *src) {
	ePlayer *player = udata;
	PlayListItem *pli = playlist_current_item_get(player->playlist);
	int cur_time  = pli->plugin->get_current_pos();
	
	debug(DEBUG_LEVEL_INFO, "Seeking backward - Current Pos: %i\n",
	      cur_time);

	eplayer_playback_stop(player);

	if (cur_time < 6) /* restart from the beginning */
		eplayer_playback_start(player, 1);
	else {
		pli->plugin->set_current_pos(cur_time - 5);
		eplayer_playback_start(player, 0);
	}
	
	paused = 0;
}

void cb_eplayer_quit(void *udata, Evas_Object *obj,
                     const char *emission, const char *src) {
	ecore_main_loop_quit();
}

void cb_eplayer_raise(void *udata, Evas_Object *obj,
                      const char *emission, const char *src) {
	ePlayer *player = udata;

	ecore_evas_raise(player->gui.ee);
}

void cb_switch_group(void *udata, Evas_Object *obj,
                     const char *emission, const char *src) {
	ePlayer *player = udata;
	
	evas_object_del(player->gui.edje);
	ui_init_edje(player, src);
}
