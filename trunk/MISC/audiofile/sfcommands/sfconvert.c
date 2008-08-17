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
	sfconvert.c

	sfconvert is a program which can convert various parameters of
	sound files.

	The real Irix version has a lot of options.  Mine can only
	convert the file format.  I'm working on expanding the
	capabilities of this command.
*/

#ifdef __USE_SGI_HEADERS__
#include <dmedia/audiofile.h>
#else
#include <audiofile.h>
#endif

#include <stdio.h>
#include <stdlib.h>

void usageerror (void);
void printfileinfo (char *filename);

int main (int argc, char **argv)
{
	int		i = 1;
	char	*infilename, *outfilename;
	int		format = AF_FILE_UNKNOWN;

	AFfilehandle	infile, outfile;
	AFfilesetup		outfilesetup;
	int				sampleFormat, sampleWidth, channelCount;
	char			*buffer;

	AFframecount	frameCount;
	int				frameSize;

	if (argc < 3)
		usageerror();

	infilename = argv[1];
	outfilename = argv[2];

	i = 3;

	while (i < argc)
	{
		if (!strcmp(argv[i], "format"))
		{
			if (i + 1 >= argc)
				usageerror();
			if (!strcmp(argv[i+1], "aiff"))
				format = AF_FILE_AIFF;
			else if (!strcmp(argv[i+1], "aifc"))
				format = AF_FILE_AIFFC;
			else if (!strcmp(argv[i+1], "wave"))
				format = AF_FILE_WAVE;
			else if (!strcmp(argv[i+1], "next"))
				format = AF_FILE_NEXTSND;
			else
			{
				fprintf(stderr, "sfconvert: Unknown format %s.\n", argv[i+1]);
				exit(-1);
			}

			i++;
		}

		i++;
	}

	infile = afOpenFile(infilename, "r", NULL);

	frameCount = afGetFrameCount(infile, AF_DEFAULT_TRACK);
	frameSize = afGetFrameSize(infile, AF_DEFAULT_TRACK, 1);
	channelCount = afGetChannels(infile, AF_DEFAULT_TRACK);
	afGetSampleFormat(infile, AF_DEFAULT_TRACK, &sampleFormat, &sampleWidth);

	buffer = malloc(frameCount * frameSize);
	afReadFrames(infile, AF_DEFAULT_TRACK, buffer, frameCount);

	afCloseFile(infile);

	outfilesetup = afNewFileSetup();
	afInitFileFormat(outfilesetup, format);
	afInitChannels(outfilesetup, AF_DEFAULT_TRACK, channelCount);
	afInitSampleFormat(outfilesetup, AF_DEFAULT_TRACK, sampleFormat,
		sampleWidth);
	outfile = afOpenFile(outfilename, "w", outfilesetup);
	afFreeFileSetup(outfilesetup);

	afWriteFrames(outfile, AF_DEFAULT_TRACK, buffer, frameCount);
	afCloseFile(outfile);

	printfileinfo(infilename);
	putchar('\n');
	printfileinfo(outfilename);

	return 0;
}

void usageerror (void)
{
	fprintf(stderr, "usage: sfconvert infile outfile [ options ... ] [ output keywords ... ]\n");
	exit(-1);
}
