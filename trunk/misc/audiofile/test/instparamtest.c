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

#include <dmedia/audiofile.h>
#include <stdio.h>

main (int ac, char **av)
{
	AFfilehandle	file;
	long			result;
	int				instids;

	file = afOpenFile(av[1], "r", NULL);
	result = afGetInstIDs(file, &instids);
	printf("%ld\n", result);
	result = afGetInstParamLong(file, AF_DEFAULT_INST, AF_INST_MIDI_BASENOTE);
	printf("%ld\n", result);
	result = afGetInstParamLong(file, AF_DEFAULT_INST, AF_INST_SUSLOOPID);
	printf("%ld\n", result);
	result = afGetInstParamLong(file, AF_DEFAULT_INST, AF_INST_RELLOOPID);
	printf("%ld\n", result);
	result = afGetInstParamLong(file, 0, AF_INST_MIDI_BASENOTE);
	printf("%ld\n", result);
	afCloseFile(file);
}
