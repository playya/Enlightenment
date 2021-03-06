/* Evolve
 * Copyright (C) 2007-2008 Hisham Mardam-Bey 
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. 
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _EVOLVE_PRIVATE_H
#define _EVOLVE_PRIVATE_H

#include <Eet.h>
#include <Ecore_File.h>
#include <Evas.h>

#include "config.h"

#include "Evolve.h"

typedef struct _Evolve_Widget Evolve_Widget;
typedef struct _Evolve_Widget_Signal Evolve_Widget_Signal;
typedef struct _Evolve_Widget_Property Evolve_Widget_Property;
typedef struct _Evolve_Signal Evolve_Signal;
typedef struct _Evolve_Signal_Callback Evolve_Signal_Callback;
typedef Etk_Widget *(*Evolve_Ctor)(void);

#include "evolve_util.h"
#include "evolve_parse.h"
#include "evolve_property.h"
#include "evolve_widget.h"
#include "evolve_constructor.h"
#include "evolve_signal.h"
#include "evolve_gui.h"

struct _Evolve
{
   /* fields to save */
   Eina_List *widgets;
   
   /* fields we dont want to save */
   char *eet_filename;
   Eina_Hash *emissions;
   Eina_Hash *callbacks;
   Eina_Hash *parents;
};

typedef enum Evolve_Widget_Property_Type {
   EVOLVE_WIDGET_PROP_INT,
   EVOLVE_WIDGET_PROP_DOUBLE,
   EVOLVE_WIDGET_PROP_STR
} Evolve_Widget_Property_Type;

struct _Evolve_Widget_Property
{
   char *name;
   char *value;
   int type;
};

struct _Evolve_Widget
{
   /* fields we want to save */
   char *type;   
   char *name;   
   char *parent;
   Eina_Hash *props;
   Eina_Hash *packing_props;
   Eina_List *signals;
   
   /* fields we dont want to save */
   Etk_Widget *widget;
};

struct _Evolve_Widget_Signal
{
   char *name;
   char *callback;
   char *emit;
   int swapped;
   char *data_type;
   char *data_name;
};

struct _Evolve_Signal
{
   int event_type;
   char *name;
   char *emit;
   char *data_type;
   char *data_name;
   Evolve *evolve;
};  

struct _Evolve_Signal_Callback
{
   void (*func)(char *emission, void *data);
   void *data;
};

#endif
