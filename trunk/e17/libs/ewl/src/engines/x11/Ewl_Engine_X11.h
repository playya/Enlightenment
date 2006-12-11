#ifndef EWL_ENGINE_X11_H
#define EWL_ENGINE_X11_H

#include <Ewl.h>
#include <Ecore_X.h>
#include <Ecore_X_Atoms.h>

#define EWL_ENGINE_X11(engine) ((Ewl_Engine_X11 *)engine)
typedef struct Ewl_Engine_X11 Ewl_Engine_X11;
struct Ewl_Engine_X11
{
	Ewl_Engine engine;
};

#endif

