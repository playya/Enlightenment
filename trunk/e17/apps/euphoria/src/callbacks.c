/*
 * $Id$
 * vim:noexpandtab:sw=4:sts=4:ts=4
 */

#include <config.h>
#include <Edje.h>
#include <Esmart/container.h>
#include <ewl/Ewl.h>
#include <assert.h>
#include "euphoria.h"
#include "interface.h"
#include "utils.h"
#include "callbacks.h"

#define FFT_BITS 10
#define FFT_LEN (1 << FFT_BITS)

#define XMMS_CB_FINISH(result) { \
	xmmsc_result_t *pants = xmmsc_result_restart((result)); \
\
	if (pants) \
		xmmsc_result_unref(pants); \
\
	xmmsc_result_unref((result)); \
}

typedef enum {
	PLAYBACK_STATE_STOPPED,
	PLAYBACK_STATE_PAUSED,
	PLAYBACK_STATE_PLAYING,
	PLAYBACK_STATE_NUM
} PlaybackState;

static int _euphoria_seek_timer(void *data);

static PlaybackState playback_state = PLAYBACK_STATE_STOPPED;
static Ewl_Widget *_fd_win = NULL;

static void signal_playback_state(Euphoria *e) {
	char *sig[PLAYBACK_STATE_NUM] = {"PLAYBACK_STATE_STOPPED",
	                                 "PLAYBACK_STATE_PAUSED",
	                                 "PLAYBACK_STATE_PLAYING"};

	assert(e);

	edje_object_signal_emit(e->gui.edje, sig[playback_state],
	                        "Euphoria");
}

static void playback_state_set(Euphoria *e, PlaybackState new_state) {
	assert(e);

	if (new_state != playback_state) {
		playback_state = new_state;
		signal_playback_state(e);
	}
}

/**
 * Starts/resumes playback.
 */
EDJE_CB(play) {
	debug(DEBUG_LEVEL_INFO, "Play callback entered\n");

	switch (playback_state) {
		case PLAYBACK_STATE_STOPPED:
		case PLAYBACK_STATE_PAUSED: /* continue playback */
			xmmsc_result_unref(xmmsc_playback_start(e->xmms));
			break;
		case PLAYBACK_STATE_PLAYING: /* restart from beginning */
			xmmsc_result_unref(xmmsc_playback_stop(e->xmms));
			xmmsc_result_unref(xmmsc_playback_start(e->xmms));
			break;
		default:
			assert(false);
			break;
	}
}

/**
 * Stops playback.
 */
EDJE_CB(stop) {
	debug(DEBUG_LEVEL_INFO, "Stop callback entered\n");

	xmmsc_result_unref(xmmsc_playback_stop(e->xmms));
}

/**
 * Pauses/resumes playback.
 */
EDJE_CB(pause) {
	debug(DEBUG_LEVEL_INFO, "Pause callback entered\n");

	switch (playback_state) {
		case PLAYBACK_STATE_STOPPED:
			break;
		case PLAYBACK_STATE_PAUSED:
			xmmsc_result_unref(xmmsc_playback_start(e->xmms));
			break;
		case PLAYBACK_STATE_PLAYING:
			xmmsc_result_unref(xmmsc_playback_pause(e->xmms));
			break;
		default:
			assert(false);
			break;
	}
}

/**
 * Hilight the current playlist item.
 * This automatically un-highlights all other items
 * (see EDJE_CB(playlist_item_selected).
 */
static void hilight_current_track(Euphoria *e) {
	PlayListItem *pli;

	if (!e->playlist || !(pli = e->playlist->current_item))
		return;

	if (pli->edje)
		edje_object_signal_emit(pli->edje, "PLAYLIST_ITEM_SELECTED", "");
}

/**
 * Moves to the next track and plays it, except when we're going
 * back to the beginning of the playlist.
 */
EDJE_CB(track_next) {
	debug(DEBUG_LEVEL_INFO, "Next File Called\n");

	xmmsc_result_unref(xmmsc_playback_next(e->xmms));

	if (playback_state != PLAYBACK_STATE_PLAYING)
		xmmsc_result_unref(xmmsc_playback_start(e->xmms));
}

/**
 * Moves to the previous track and plays it, except when we're
 * at the first track already.
 */
EDJE_CB(track_prev) {
	debug(DEBUG_LEVEL_INFO, "Previous File Called\n");

	/* first item on the list: do nothing */
#if 0
	if (!playlist_current_item_has_prev(e->playlist))
		return;
#endif

	xmmsc_result_unref(xmmsc_playback_prev(e->xmms));

	if (playback_state != PLAYBACK_STATE_PLAYING)
		xmmsc_result_unref(xmmsc_playback_start(e->xmms));
}

EDJE_CB(volume_raise) {
	debug(DEBUG_LEVEL_INFO, "Raising volume\n");

	/* FIXME */
	ui_refresh_volume(e);
}

EDJE_CB(volume_lower) {
	debug(DEBUG_LEVEL_INFO, "Lowering volume\n");

	/* FIXME */
	ui_refresh_volume(e);
}

EDJE_CB(time_display_toggle) {
	e->cfg.time_display = !e->cfg.time_display;
	/* FIXME track_update_time(e); */
}

EDJE_CB(repeat_mode_toggle) {
	e->cfg.repeat = !e->cfg.repeat;
}

EDJE_CB(playlist_scroll_up) {
	int size = (int) evas_object_data_get(e->gui.playlist,
	                                      "PlaylistFontSize");

	/* it's * 3 because we're scrolling 3 elements at once */
	e_container_scroll(e->gui.playlist, size * 3);
}

EDJE_CB(playlist_scroll_down) {
	int size = (int) evas_object_data_get(e->gui.playlist,
	                                      "PlaylistFontSize");

	/* it's * 3 because we're scrolling 3 elements at once */
	e_container_scroll(e->gui.playlist, size * -3);
}

EDJE_CB(playlist_item_play) {
	PlayListItem *pli = evas_object_data_get(obj, "PlayListItem");

	xmmsc_result_unref(xmmsc_playback_jump(e->xmms, pli->id));
	xmmsc_result_unref(xmmsc_playback_start(e->xmms));
}

static void remove_playlist_item(Euphoria *e, PlayListItem *pli) {
	assert(pli);

	xmmsc_result_unref(xmmsc_playlist_remove(e->xmms, pli->id));
}

EDJE_CB(playlist_item_remove) {
	PlayListItem *pli = evas_object_data_get(obj, "PlayListItem");

	if (pli)
		remove_playlist_item(e, pli);
}

EDJE_CB(playlist_item_selected) {
	Evas_List *items = e_container_elements_get(e->gui.playlist);
	Evas_List *l;

	for (l = items; l; l = l->next)
		if (l->data != obj)
			edje_object_signal_emit(l->data,
			                        "PLAYLIST_ITEM_UNSELECTED", "");
}

EDJE_CB(seek_forward) {
#if 0
	/* FIXME */
	PlayListItem *pli = playlist_current_item_get(e->playlist);

	debug(DEBUG_LEVEL_INFO, "Seeking forward\n");

	/* We don't care if you seek past the file, the play loop
	 * will catch EOF and play next file
	 */
	xmmsc_playback_stop(e->xmms);
	pli->plugin->set_current_pos(pli->plugin->get_current_pos() + 5);

	xmmsc_playback_start(e->xmms);
#endif
}

EDJE_CB(seek_backward) {
#if 0
	/* FIXME */
	PlaybackState state = playback_state;
	PlayListItem *pli = playlist_current_item_get(e->playlist);
	/*int cur_time = pli->plugin->get_current_pos();*/

	debug(DEBUG_LEVEL_INFO, "Seeking backward\n",
	      cur_time);

	xmmsc_playback_stop(e);

	if (cur_time >= 6)
		pli->plugin->set_current_pos(cur_time - 5);

	xmmsc_playback_start(e, cur_time < 6);
#endif
}

EDJE_CB(seek_forward_start) {
	debug(DEBUG_LEVEL_INFO, "Start seeking forward\n");
	e->seekerflags.seeking = true;
	e->seekerflags.seek_dir = 1;
	ecore_timer_add(.02, _euphoria_seek_timer, e);
}

EDJE_CB(seek_forward_stop) {
	debug(DEBUG_LEVEL_INFO, "Stop seeking forward\n");
	e->seekerflags.seeking = false;
}

EDJE_CB(seek_backward_start) {
	debug(DEBUG_LEVEL_INFO, "Start seeking backward\n");
	e->seekerflags.seeking = true;
	e->seekerflags.seek_dir = -1;
	ecore_timer_add(.02, _euphoria_seek_timer, e);
}

EDJE_CB(seek_backward_stop) {
	debug(DEBUG_LEVEL_INFO, "Stop seeking backward\n");
	e->seekerflags.seeking = false;
}

EDJE_CB(euphoria_quit) {
	xmmsc_result_unref(xmmsc_playback_stop(e->xmms));
	xmmsc_result_unref(xmmsc_quit(e->xmms));
	ecore_main_loop_quit();
}

EDJE_CB(euphoria_close) {
	ecore_main_loop_quit();
}

EDJE_CB(euphoria_raise) {
	ecore_evas_raise(e->gui.ee);
}

EDJE_CB(switch_group) {
	ui_shutdown_edje(e);
	ui_init_edje(e, src);

	xmmsc_result_unref(xmmsc_playback_current_id(e->xmms));

	playlist_container_set(e->playlist, e->gui.playlist);
	signal_playback_state(e);
}

EDJE_CB(update_seeker) {
	Evas_Coord x, y, w, h;
	int type, ex = 0, ey = 0;
	double pos;

	if (!strcmp(emission, "SEEKER_START"))
		e->seekerflags.seeker_seeking = true;
	else if (!strcmp(emission, "SEEKER_STOP"))
		e->seekerflags.seeker_seeking = false;

	if (!e->seekerflags.seeker_seeking)
		return;

	type = ecore_event_current_type_get();

	if (type == ECORE_X_EVENT_MOUSE_MOVE) {
		Ecore_X_Event_Mouse_Move *event;

		event = ecore_event_current_event_get();
		ex = event->x;
		ey = event->y;
	} else if (type == ECORE_X_EVENT_MOUSE_BUTTON_DOWN) {
		Ecore_X_Event_Mouse_Button_Down *event;

		event = ecore_event_current_event_get();
		ex = event->x;
		ey = event->y;
	} else
		assert(false);

	edje_object_part_geometry_get(e->gui.edje, "seeker_grabber",
	                              &x, &y, &w, &h);

	if (e->seekerflags.is_vertical)
		pos = ((double) (ey - y)) / ((double) h);
	else
		pos = ((double) (ex - x)) / ((double) w);

	if (pos < 0) pos = 0;
	if (pos > 1) pos = 1;

	if (e->seekerflags.invert_dir)
		pos = 1 - pos;

	pos *= playlist_item_duration_get(e->playlist->current_item) * 1000;

	xmmsc_result_unref(xmmsc_playback_seek_ms(e->xmms, (int) pos));
}

static int _euphoria_seek_timer(void *data) {
	Euphoria *e = data;
	int new_pos;

	new_pos = e->track_current_pos + e->seekerflags.seek_dir;

	if (new_pos <= 0)
		new_pos = 0;

	if (new_pos > playlist_item_duration_get(e->playlist->current_item))
		new_pos = playlist_item_duration_get(e->playlist->current_item);

	xmmsc_result_unref(xmmsc_playback_seek_ms(e->xmms, new_pos * 1000));

	return !!e->seekerflags.seeking;
}

/* Handle Key Bindings via EVAS Event Callback */
void cb_key_press(void *data, Evas *evas, Evas_Object *obj,
                  void *event_info) {
	Euphoria *e = data;

        Evas_Event_Key_Down *ev;
        ev = (Evas_Event_Key_Down *)event_info;

        //printf("DEBUG: You hit key: %s\n", ev->keyname);

	if      (!strcmp(ev->keyname, "Return"))
	   edje_object_signal_emit(e->gui.edje, "PLAY_NEXT", "*");
        if      (!strcmp(ev->keyname, "space"))
           edje_object_signal_emit(e->gui.edje, "PAUSE", "*");
	else if (!strcmp(ev->keyname, "Escape"))
	   edje_object_signal_emit(e->gui.edje, "QUIT", "*");
        else if (!strcmp(ev->keyname, "q"))
           edje_object_signal_emit(e->gui.edje, "QUIT", "*");
        else if (!strcmp(ev->keyname, "Down"))
           edje_object_signal_emit(e->gui.edje, "PLAY_NEXT", "*");
        else if (!strcmp(ev->keyname, "Up"))
           edje_object_signal_emit(e->gui.edje, "PLAY_PREVIOUS", "*");
        else if (!strcmp(ev->keyname, "Right"))
           edje_object_signal_emit(e->gui.edje, "SEEK_FORWARD", "*");
        else if (!strcmp(ev->keyname, "Left"))
           edje_object_signal_emit(e->gui.edje, "SEEK_BACK", "*");
        else if (!strcmp(ev->keyname, "KP_Add"))
           edje_object_signal_emit(e->gui.edje, "VOL_INCR", "*");
        else if (!strcmp(ev->keyname, "equal"))
           edje_object_signal_emit(e->gui.edje, "VOL_INCR", "*");
        else if (!strcmp(ev->keyname, "KP_Subtract"))
           edje_object_signal_emit(e->gui.edje, "VOL_DECR", "*");
        else if (!strcmp(ev->keyname, "minus"))
           edje_object_signal_emit(e->gui.edje, "VOL_DECR", "*");


	/********* See notes on cb_key_release().
        else if (!strcmp(ev->keyname, "Right"))
           edje_object_signal_emit(e->gui.edje, "SEEK_FORWARD_START", "*");
        else if (!strcmp(ev->keyname, "Left"))
           edje_object_signal_emit(e->gui.edje, "SEEK_BACK_START", "*");
	************/

}

/* Handle Key Bindings via EVAS Event Callback */
/* - This only works IF you turn off X key repeat, which raster */
/*   pointed out is global... so for most of us, this won't work. */
/* This function is, thus, currently unused, but left in for testing. */
void cb_key_release(void *data, Evas *evas, Evas_Object *obj,
                    void *event_info) {
	Euphoria *e = data;
	Evas_Event_Key_Down *ev;

        ev = (Evas_Event_Key_Down *)event_info;

        //printf("DEBUG: You released key: %s\n", ev->keyname);

        if      (!strcmp(ev->keyname, "Right"))
           edje_object_signal_emit(e->gui.edje, "SEEK_FORWARD_STOP", "*");
        else if (!strcmp(ev->keyname, "Left"))
           edje_object_signal_emit(e->gui.edje, "SEEK_BACK_STOP", "*");

}

/* Callback to to close the filedialog window */
static void destroy_ewl_filedialog(Ewl_Widget * w, void *ev_data,
                            void *user_data) {
	ewl_widget_destroy(w);
	_fd_win = NULL;
}

static void cb_file_dialog_value_changed(Ewl_Widget *w, void *ev_data,
                                         void *udata) {
	Euphoria *e = udata;

	if (ev_data) {
		char buf[PATH_MAX];

		snprintf(buf, PATH_MAX, "file://%s", (char*)ev_data);
		xmmsc_result_unref(xmmsc_playlist_add(e->xmms, buf));
	}

	ewl_widget_hide(_fd_win);
}

/* File Dialog to add files, thanx to EWL */
EDJE_CB(playlist_add) {
    Ewl_Widget *fd_win = NULL;
    Ewl_Widget *fd = NULL;
    Ewl_Widget *vbox = NULL;

    if (_fd_win) {
		ewl_widget_show(_fd_win);
		return;
	}

	fd_win = ewl_window_new();
	ewl_window_set_title(EWL_WINDOW(fd_win), "Euphoria Add File...");
	ewl_window_set_name(EWL_WINDOW(fd_win), "Euphoria Add File...");
	ewl_object_request_size(EWL_OBJECT(fd_win), 500, 400);
	ewl_object_set_fill_policy(EWL_OBJECT(fd_win), EWL_FLAG_FILL_FILL |
	                           EWL_FLAG_FILL_SHRINK);

	ewl_callback_append(fd_win, EWL_CALLBACK_DELETE_WINDOW,
	                    destroy_ewl_filedialog, NULL);

	vbox = ewl_vbox_new();
	ewl_object_set_fill_policy(EWL_OBJECT(vbox), EWL_FLAG_FILL_FILL |
	                           EWL_FLAG_FILL_SHRINK);
	ewl_container_append_child(EWL_CONTAINER(fd_win), vbox);
	ewl_widget_show(vbox);

	fd = ewl_filedialog_new(EWL_FILEDIALOG_TYPE_OPEN);
	ewl_object_set_fill_policy(EWL_OBJECT(fd), EWL_FLAG_FILL_FILL |
	                           EWL_FLAG_FILL_SHRINK);

	ewl_callback_append(fd, EWL_CALLBACK_VALUE_CHANGED,
	                    cb_file_dialog_value_changed, e);

	ewl_container_append_child(EWL_CONTAINER(vbox), fd);
	ewl_widget_show(fd);

	_fd_win = fd_win;
    ewl_widget_show(_fd_win);
}

EDJE_CB(playlist_del) {
	if (e->playlist->current_item)
		remove_playlist_item(e, e->playlist->current_item);
}

EDJE_CB(playlist_shuffle) {
	assert(e->xmms);
	xmmsc_result_unref(xmmsc_playlist_shuffle(e->xmms));
}

EDJE_CB(playlist_clear) {
	assert(e->xmms);
	xmmsc_result_unref(xmmsc_playback_stop(e->xmms));
	xmmsc_result_unref(xmmsc_playlist_clear(e->xmms));
}

XMMS_CB(playback_status) {
	PlaybackState state;
	unsigned int xstate = 0;

	xmmsc_result_get_uint (res, &xstate);

	switch (xstate) {
		case XMMSC_PLAYBACK_PLAY:
			state = PLAYBACK_STATE_PLAYING;
			break;
		case XMMSC_PLAYBACK_STOP:
			state = PLAYBACK_STATE_STOPPED;
			break;
		case XMMSC_PLAYBACK_PAUSE:
			state = PLAYBACK_STATE_PAUSED;
			break;
		default:
			assert(false);
			break;
	}

	playback_state_set(e, state);
	XMMS_CB_FINISH(res);
}

XMMS_CB(playback_playtime) {
	unsigned int duration = 0;
	double pos;

	if (!e->playlist->current_item) {
		XMMS_CB_FINISH(res);
		return;
	}

	xmmsc_result_get_uint (res, &duration);

	e->track_current_pos = duration / 1000; /* time is in msecs */
	duration = playlist_item_duration_get(e->playlist->current_item);

	ui_refresh_time(e, e->track_current_pos);

	pos = ((double) e->track_current_pos / duration);

	if (e->seekerflags.invert_dir)
		pos = 1 - pos;

	ui_refresh_seeker(e, pos);
	XMMS_CB_FINISH(res);
}

static void handle_current_id(struct _Euphoria *e,
                              xmmsc_result_t *res) {
	unsigned int id = 0;

	assert(e);
	assert(res);

	if (!xmmsc_result_get_uint (res, &id))
		return;

	if (playlist_set_current(e->playlist, id))
		hilight_current_track(e);
}

XMMS_CB(playback_current_id) {
	handle_current_id(e, res);
	XMMS_CB_FINISH(res);
}

XMMS_CB(playlist_mediainfo) {
	PlayListItem *pli;
	x_hash_t *hash = NULL;
	unsigned int id;

	xmmsc_result_get_mediainfo(res, &hash);
	id = (unsigned int) x_hash_lookup(hash, "id");

	if (!(pli = playlist_item_find_by_id(e->playlist, id))) {
		xmmsc_result_unref(res);
		return;
	}

	playlist_item_properties_set(pli, hash);

	/* we need to call this here, too, since the Edje might have been
	 * created after the playback_currentid callback has been called
	 */
	if (pli == e->playlist->current_item) {
		hilight_current_track(e);
		ui_fill_track_info(e, pli);
	}

	xmmsc_result_unref(res);
}

XMMS_CB(playlist_entry_changed) {
	xmmsc_result_t *res2;
	unsigned int id = 0;

	xmmsc_result_get_uint (res, &id);

	res2 = xmmsc_playlist_get_mediainfo(e->xmms, id);
	xmmsc_result_notifier_set(res2,
	                          (XmmsCb) on_xmms_playlist_mediainfo, e);
	xmmsc_result_unref(res2);
	XMMS_CB_FINISH(res);
}

XMMS_CB(playlist_list) {
	xmmsc_result_t *res2;
	x_list_t *list = NULL, *l;
	unsigned int id;
	int r;

	if ((!(r = xmmsc_result_get_uintlist(res, &list)))) {
		xmmsc_result_unref(res);
		return;
	}

	for (l = list; l; l = l->next) {
		id = (unsigned int) l->data;
		playlist_item_add(e->playlist, id);

		res2 = xmmsc_playlist_get_mediainfo(e->xmms, id);
		xmmsc_result_notifier_set(res2,
		                          (XmmsCb) on_xmms_playlist_mediainfo,
		                          e);
		xmmsc_result_unref(res2);
	}

	xmmsc_result_unref(res);

	/* hack: get the current id */
	res2 = xmmsc_playback_current_id(e->xmms);
	xmmsc_result_wait(res2);
	handle_current_id(e, res2);
	xmmsc_result_unref(res2);
}

static void playlist_refill(struct _Euphoria *e) {
	PlayListItem *pli;
	x_list_t *list, *l;
	unsigned int id;

	if (!(list = xmmscs_playlist_list(e->xmms)))
		return;

	for (l = list; l; l = l->next) {
		id = (unsigned int) l->data;

		if ((pli = playlist_item_find_by_id(e->playlist, id))) {
			e_container_element_remove(pli->container, pli->edje);
			e_container_element_append(pli->container, pli->edje);
		} else
			fprintf(stderr, "Unable to find id %d\n", id);
	}
}

XMMS_CB(playlist_changed) {
	PlayListItem *pli;
	unsigned type = 0, id = 0, arg = 0;

	if (!xmmsc_result_get_playlist_change(res, &type, &id, &arg)) {
		XMMS_CB_FINISH(res);
		return;
	}

	switch (type) {
		case XMMSC_PLAYLIST_ADD:
			playlist_item_add(e->playlist, id);
			break;
		case XMMSC_PLAYLIST_REMOVE:
			if (xmmscs_playback_current_id(e->xmms) == id)
				xmmsc_playback_stop(e->xmms);

			if ((pli = playlist_item_find_by_id(e->playlist, id)))
				playlist_item_remove (e->playlist, pli);

			break;
		case XMMSC_PLAYLIST_CLEAR:
			playlist_remove_all(e->playlist);
			ui_zero_track_info(e);
			break;
		case XMMSC_PLAYLIST_SHUFFLE:
		case XMMSC_PLAYLIST_SORT:
			playlist_refill(e);
			break;
		default:
			break;
	}

	XMMS_CB_FINISH(res);
}

#if 0
XMMS_CB(visdata) {
	x_list_t *list, *l;
	double spec[FFT_LEN / 2], sum, peak = 0.0;
	int r, i, j;

	r = xmmsc_result_get_doublelist (res, &list);
	assert (r);

	for (i = 0, l = list; l; l = l->next, i++)
		spec[i] = *(double *) l->data;

	for (i = 0; i < FFT_LEN / 32 / 2; i++) {
		for (sum = 0.0, j = 0; j < 32; j++)
			sum += spec[i * 16 + j];

		if (sum)
			sum = log(sum / 32);

		sum = MIN(255, sum * 64);
		peak = MAX(peak, sum);
	}

	peak = 1 - (peak / 255);

	edje_object_part_drag_value_set(e->gui.edje, "peak_analyzer", 0, peak);
	XMMS_CB_FINISH(res);
}
#endif
