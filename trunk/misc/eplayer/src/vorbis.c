#include <config.h>
#include <Edje.h>
#include <vorbis/codec.h>
#include <sys/ioctl.h>
#include <assert.h>
#include "eplayer.h"
#include "vorbis.h"
#include "interface.h"

/**
 * Plays a chunk of the current track.
 *
 * @param udata Pointer to an ePlayer struct.
 */
int track_play_chunk(void *udata) {
	ePlayer *player = udata;
	long bytes_read;
	int big_endian = 0;
	static char pcmout[4096];

#ifdef WORDS_BIGENDIAN
	big_endian = 1;
#endif

	/* read the data ... */
	bytes_read = ov_read(&player->current_track, pcmout, sizeof(pcmout),
	                     big_endian, 2, 1, NULL);

    if (bytes_read) /* ... and play it */
		ao_play(player->ao_dev, pcmout, bytes_read);
	else /* EOF -> move to the next track */
		edje_object_signal_emit(player->gui.edje,
	                            "PLAY_NEXT", "next_button");

	/* the edje callback will re-add the idler, so we can remove it here,
	 * in case ov_read() failed
	 */
	return !!bytes_read;
}

int update_time(void *udata) {
	ePlayer *player = udata;
	static int old_time = -1;
	int cur_time;

	cur_time = ov_time_tell(&player->current_track);
	
	if (player->time_display == TIME_DISPLAY_LEFT)
		cur_time = ov_time_total(&player->current_track, -1) - cur_time;

	if (cur_time == old_time) /* value didn't change, so don't update */
		return 1;

	old_time = cur_time;
	refresh_time(player, cur_time);

	return 1;
}

static int setup_ao(ePlayer *player) {
	PlayListItem *current = player->playlist->cur_item->data;
	ao_sample_format format = {0};
	
	format.bits = 16;
	format.channels = current->channels;
	format.rate = current->rate;
	format.byte_format = AO_FMT_NATIVE;

#ifdef DEBUG
	printf("AO DEBUG: %d Channels at %d Hz, in %d bit words\n",
	       format.channels, format.rate, format.bits);
#endif
	
	player->ao_dev = ao_open_live(ao_default_driver_id(), &format, NULL);

	if (!player->ao_dev)
		fprintf(stderr, "Error opening device.\n");

	return !!player->ao_dev;
}

/**
 * Closes the current track.
 *
 * @param player
 */
void track_close(ePlayer *player) {
	ao_close(player->ao_dev);
	ov_clear(&player->current_track);
}

/**
 * Opens the current track and prepares libao for playing.
 *
 * @param player
 */
void track_open(ePlayer *player) {
	PlayListItem *pli;
	FILE *fp;

	assert(player->playlist->cur_item);
	pli = player->playlist->cur_item->data;

	if (!(fp = fopen (pli->file, "rb"))
	    || ov_open(fp, &player->current_track, NULL, 0)) {
		fprintf (stderr, "ERROR: Can't open file '%s'\n", pli->file);
		return;
	}

	edje_object_part_text_set(player->gui.edje, "song_name", pli->title);
	edje_object_part_text_set(player->gui.edje, "artist_name", pli->artist);
	edje_object_part_text_set(player->gui.edje, "album_name", pli->album);

	setup_ao(player);
}

void cb_seek_forward(void *udata, Evas_Object *obj,
                     const char *emission, const char *src) {
	ePlayer *player = udata;

#ifdef DEBUG
	printf("DEBUG: Seeking forward\n");
#endif

	/* We don't care if you seek past the file, the play loop
	 * will catch EOF and play next file
	 */
	ov_time_seek(&player->current_track,
	             ov_time_tell(&player->current_track) + 5);
}

void cb_seek_backward(void *udata, Evas_Object *obj,
                      const char *emission, const char *src) {
	ePlayer *player = udata;
	double cur_time = ov_time_tell(&player->current_track);
	
	/* Lets not seek backward if there isn't a point */
	if (cur_time < 6) {
		printf("You arent even 5 secs into the stream!\n");
		return;
	} 

#ifdef DEBUG
	printf("DEBUG: Seeking backward - Current Pos: %lf\n", cur_time);
#endif
	
	ov_time_seek(&player->current_track, cur_time - 5);
}

