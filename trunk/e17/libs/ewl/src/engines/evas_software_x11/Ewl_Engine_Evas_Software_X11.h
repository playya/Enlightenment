#ifndef EWL_ENGINE_EVAS_SOFTWARE_X11_H
#define EWL_ENGINE_EVAS_SOFTWARE_X11_H

#include <Ewl.h>
#include <Ecore_X.h>
#include <Evas_Engine_Software_X11.h>

#define EWL_ENGINE_EVAS_SOFTWARE_X11(engine) \
		((Ewl_Engine_Evas_Software_X11 *)engine)

typedef struct Ewl_Engine_Evas_Software_X11 Ewl_Engine_Evas_Software_X11;
struct Ewl_Engine_Evas_Software_X11
{
	Ewl_Engine engine;
};

#endif

