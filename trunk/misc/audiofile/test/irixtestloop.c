/*
	Audio File Library

	Copyright 1998, Michael Pruett <michael@68k.org>

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
	testloop.c

	This file reads the loop points from a file (presumably AIFF) and
	loops that part of the file several times.  Audio output is routed
	to Irix's default audio output device.  This program will not
	compile on any platform other than Irix.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dmedia/audio.h>
#include <dmedia/audiofile.h>

main (int ac, char **av)
{
	AFfilehandle	file;
	AFframecount	frameCount;
	int				sampleFormat, sampleWidth, channelCount;
	char			*buffer;
	int				*loopids, *markids;
	int				i, loopCount, markCount;
	int				startmarkid, endmarkid;
	AFframecount	startloop, endloop;

	ALport			outport;
	ALconfig		outportconfig;

	file = afOpenFile(av[1], "r", NULL);
	frameCount = afGetFrameCount(file, AF_DEFAULT_TRACK);
	channelCount = afGetChannels(file, AF_DEFAULT_TRACK);
	printf("frame count: %d\n", frameCount);
	afGetSampleFormat(file, AF_DEFAULT_TRACK, &sampleFormat, &sampleWidth);
	buffer = (char *) malloc(frameCount * sampleWidth);
	afReadFrames(file, AF_DEFAULT_TRACK, buffer, frameCount);

	loopCount = afGetLoopIDs(file, AF_DEFAULT_INST, NULL);
	loopids = malloc(sizeof (int) * loopCount);
	afGetLoopIDs(file, AF_DEFAULT_INST, loopids);

	markCount = afGetMarkIDs(file, AF_DEFAULT_TRACK, NULL);
	markids = malloc(sizeof (int) * markCount);
	afGetMarkIDs(file, AF_DEFAULT_TRACK, markids);

	printf("loop ids:");
	for (i=0; i<loopCount; i++)
		printf(" %d", loopids[i]);
	printf("\n");

	printf("mark ids:");
	for (i=0; i<markCount; i++)
		printf(" %d", markids[i]);
	printf("\n");

	startmarkid = afGetLoopStart(file, AF_DEFAULT_INST, 1);
	endmarkid = afGetLoopEnd(file, AF_DEFAULT_INST, 1);
	startloop = afGetMarkPosition(file, AF_DEFAULT_TRACK, startmarkid);
	endloop = afGetMarkPosition(file, AF_DEFAULT_TRACK, endmarkid);

	afCloseFile(file);

	outportconfig = alNewConfig();
	alSetWidth(outportconfig, AL_SAMPLE_16);
	alSetChannels(outportconfig, 2);

	outport = alOpenPort("dick", "w", outportconfig);
	alWriteFrames(outport, buffer, startloop);
	for (i=0; i<3; i++)
	{
		printf("starting iteration %d: %d, %d, %d\n", i, endloop, startloop, endloop - startloop);
		alWriteFrames(outport, buffer + startloop * channelCount * (sampleWidth / 8), endloop - startloop);
	}

	alClosePort(outport);
	alFreeConfig(outportconfig);
}
