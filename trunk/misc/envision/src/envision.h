/*
 * $Id$
 * vim:expandtab:ts=3:sts=3:sw=3
 */

#ifndef __ENVISION_H
#define __ENVISION_H

#include <Ecore_Evas.h>
#include <Ecore.h>
#include <Emotion.h>
#include <Edje.h>

typedef struct
{
  char *engine;
  int width;
  int height;
} Config;

typedef struct
{
  Ecore_Evas *ee;
  Evas *evas;
  Evas_Object *edje;
  Evas_Object *emotion;
} Gui;

typedef struct
{
  Evas_List *playlist;
  Gui gui;
  Config config;
} Envision;

// envision.c
Envision *envision_new ();
void envision_delete (Envision * e);

#endif /* __ENVISION_H */
