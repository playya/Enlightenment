#ifndef EAPS_H
#define EAPS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <Eet.h>
#include <Engrave.h>

void create_dir_eap(char *path, char *cat);
void write_icon(char *file, G_Eap *eap);

#endif
