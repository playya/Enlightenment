#include "e.h"

#include "layout.h"
#include "background.h"
#include "volume.h"
#include "status.h"
#include "menu.h"
#include "video.h"
#include "mini.h"
#include "sha1.h"

extern Evas *evas;
extern char *theme;
extern char *config;

#define NONE  0
#define MENU  1
#define VIDEO 2

void main_mode_push(int mode);
void main_mode_pop(void);
