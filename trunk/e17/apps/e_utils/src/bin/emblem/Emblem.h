#ifndef EMBLEM_H
#define EMBLEM_H

#include <sys/types.h>
#include <dirent.h>

#include <E.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Ecore_Job.h>
#include <Evas.h>
#include <Edje.h>
#include <Esmart/Esmart_Container.h>

#include "config.h"

typedef struct Emblem Emblem;
struct Emblem
{
    char *display;
    char *theme;
   
    struct {
        Ecore_Evas *ee;
        Evas *evas;

        Evas_Object *menu;
        Evas_Object *current;
        Evas_Object *edje;
    } gui;
};

Emblem *emblem_new(void);
void emblem_free(Emblem *em);

int emblem_ui_init(Emblem *em);
void emblem_ui_shutdown(void);

#endif

