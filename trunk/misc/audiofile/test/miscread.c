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

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#ifdef __USE_SGI_HEADERS__
#include <dmedia/audiofile.h>
#else
#include "audiofile.h"
#endif

char copyright[] = "1998 Michael Pruett";
char name[] = "Michael Pruett's home-brew methamphetamines";

/* Two frames of 16-bit stereo samples. */
u_int16_t data[] = {0, 1, 2, 3};

int main (int argc, char **argv)
{
	AFfilehandle	file;
	int				*miscids;
	int				i, misccount;

	if (argc < 2)
	{
		fprintf(stderr, "usage: %s <audio file>\n", argv[0]);
		exit(0);
	}

	file = afOpenFile(argv[1], "r", NULL);
	misccount = afGetMiscIDs(file, NULL);
	miscids = malloc(sizeof (int) * misccount);
	afGetMiscIDs(file, miscids);

	for (i=0; i<misccount; i++)
	{
		char	*data;
		int		datasize;

		datasize = afGetMiscSize(file, miscids[i]);
		printf("Miscellaneous %d, %d bytes:\n", afGetMiscType(file, miscids[i]),
				datasize);
		data = malloc(datasize);
		afReadMisc(file, miscids[i], data, datasize);
		puts(data);
		free(data);
	}

	afCloseFile(file);
}
