#ifndef _ETK_ENGINE_ECORE_EVAS_H
#define _ETK_ENGINE_ECORE_EVAS_H

#define ETK_ENGINE_ECORE_EVAS_WINDOW_DATA(data) ((Etk_Engine_Ecore_Evas_Window_Data*)data)

/* Engine specific data for Etk_Window */
typedef struct _Etk_Engine_Ecore_Evas_Window_Data Etk_Engine_Ecore_Evas_Window_Data;

struct _Etk_Engine_Ecore_Evas_Window_Data
{
   Ecore_Evas *ecore_evas;
};

#endif
