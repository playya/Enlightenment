#ifndef __EPLAYER_H
#define __EPLAYER_H

#include <Ecore.h>
#include <vorbis/vorbisfile.h>
#include "playlist.h"
#include "output_plugin.h"

typedef enum {
	TIME_DISPLAY_ELAPSED,
	TIME_DISPLAY_LEFT
} TimeDisplay;

typedef struct {
	char evas_engine[255];
	char output_plugin[255];
	TimeDisplay time_display;
} Config;

typedef struct {
	Evas *evas;
	Evas_Object *edje;
	Evas_Object *playlist; /* playlist container */
	Evas_Object *playlist_col[2];
	int playlist_font_size[2]; /* 0 -> title, 1 -> length */
} Gui;

typedef struct {
	PlayList *playlist;
	Ecore_Idler *play_idler;
	Ecore_Timer *time_timer;

	OggVorbis_File current_track;
	OutputPlugin *output;

	Config cfg;
	Gui gui;
} ePlayer;

void eplayer_playback_stop(ePlayer *player, int rewind_track);
void eplayer_playback_start(ePlayer *player, int rewind_track);

#endif

