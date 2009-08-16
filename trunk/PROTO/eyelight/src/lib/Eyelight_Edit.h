
#ifndef  EYELIGHT_EDIT_INC
#define  EYELIGHT_EDIT_INC

#include "Eyelight.h"

/**
 * This file describes a API which allows to work directly in the presentation (set item ...)
 * Currently this API is empty
 */

EAPI const char* eyelight_edit_slide_title_get(Eyelight_Viewer *pres, int id_slide);

EAPI Eyelight_Viewer *eyelight_object_pres_get(Evas_Object *obj);



#endif   /* ----- #ifndef EYELIGHT_EDIT_INC  ----- */

