#ifndef THIMBNAIL_H
#define THIMBNAIL_H


#include "feh.h"
#include "filelist.h"
#include "winwidget.h"

#define FEH_THUMB(l) ((feh_thumbnail *) l)

typedef struct thumbnail {
	int x;
	int y;
	int w;
	int h;
	feh_file *file;
    unsigned char exists;
	struct feh_thumbnail *next;
} feh_thumbnail;


feh_thumbnail *feh_thumbnail_new(feh_file *fil, int x, int y, int w, int h);
feh_file* feh_thumbnail_get_file_from_coords(int x, int y);
feh_thumbnail* feh_thumbnail_get_thumbnail_from_coords(int x, int y);
feh_thumbnail *feh_thumbnail_get_from_file(feh_file *file);
void feh_thumbnail_mark_removed(feh_file * file, int deleted);

#endif

