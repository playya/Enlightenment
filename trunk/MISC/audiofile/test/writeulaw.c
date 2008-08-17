/*
	Audio File Library

	Copyright 1999, Michael Pruett <michael@68k.org>

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation; either version 2 of
	the License, or (at your option) any later version.

	This program is distributed in the hope that it will be
	useful, but WITHOUT ANY WARRANTY; without even the implied
	warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
	PURPOSE.  See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public
	License along with this program; if not, write to the Free
	Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
	MA 02111-1307, USA.
*/

/*
	writeulaw.c

	The writeulaw program performs sanity testing on the Audio File
	Library's G.711 u-law compression by writing and then reading
	back known data to a file to make sure the two sets of data agree.

	This program writes a set of data which is invariant under G.711
	u-law compression to a file and then reads that set of data back.

	The data read from that file should match the data written
	exactly.

	If this test fails, something in the Audio File Library is broken.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef __USE_SGI_HEADERS__
#include <dmedia/audiofile.h>
#else
#include <audiofile.h>
#endif

#include <stdio.h>
#include <unistd.h>

#define TEST_FILE "/tmp/fuck.au"

void cleanup (void)
{
	unlink(TEST_FILE);
}

void ensure (int condition, const char *message)
{
	if (!condition)
	{
		printf("%s.\n", message);
		cleanup();
		exit(-1);
	}
}

int main (int argc, char **argv)
{
	AFfilehandle	file;
	AFfilesetup		setup;
	u_int16_t		frames[] = {8, 16, 80, 120, 180, 780, 924, 988,
						1116, 1436, 1884, 8828, 9852, 15996, 19836, 32124};
	u_int16_t		readframes[16];
	int				i;

	setup = afNewFileSetup();

	afInitCompression(setup, AF_DEFAULT_TRACK, AF_COMPRESSION_G711_ULAW);
	afInitFileFormat(setup, AF_FILE_NEXTSND);
	afInitChannels(setup, AF_DEFAULT_TRACK, 1);

	file = afOpenFile(TEST_FILE, "w", setup);
	afFreeFileSetup(setup);

	ensure(afGetCompression(file, AF_DEFAULT_TRACK) ==
		AF_COMPRESSION_G711_ULAW,
		"test file not created with G.711 u-law compression");

	ensure(file != AF_NULL_FILEHANDLE, "unable to open file for writing");

	ensure(afWriteFrames(file, AF_DEFAULT_TRACK, frames, 16) == 16,
		"number of frames requested does not match number of frames written");
	afCloseFile(file);

	/* Open the file for reading and verify the data. */
	file = afOpenFile(TEST_FILE, "r", NULL);
	ensure(file != AF_NULL_FILEHANDLE, "unable to open file for reading");

	ensure(afGetFileFormat(file, NULL) == AF_FILE_NEXTSND,
		"test file not created as NeXT/Sun .snd/.au");

	ensure(afGetCompression(file, AF_DEFAULT_TRACK) ==
		AF_COMPRESSION_G711_ULAW,
		"test file not opened with G.711 u-law compression");

	ensure(afReadFrames(file, AF_DEFAULT_TRACK, readframes, 16) == 16,
		"number of frames read does not match number of frames requested");

#ifdef DEBUG
	for (i=0; i<16; i++)
		printf("readframes[%d]: %d\n", i, readframes[i]);
	for (i=0; i<16; i++)
		printf("frames[%d]: %d\n", i, frames[i]);
#endif

	for (i=0; i<16; i++)
	{
		ensure(frames[i] == readframes[i],
			"data written does not match data read");
	}

	ensure(afGetTrackBytes(file, AF_DEFAULT_TRACK) == 16,
		"track byte count is incorrect");

	ensure(afGetFrameCount(file, AF_DEFAULT_TRACK) == 16,
		"frame count is incorrect");

	ensure(afGetChannels(file, AF_DEFAULT_TRACK) == 1,
		"channel count is incorrect");

	ensure(afCloseFile(file) == 0, "error closing file");

	cleanup();

	printf("writeulaw test passed.\n");

	exit(0);
}
