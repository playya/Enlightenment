/* vim: set sw=8 ts=8 sts=8 noexpandtab: */
#ifndef EWL_ENGINE_EVAS_H
#define EWL_ENGINE_EVAS_H

#include <Ewl.h>

#define EWL_ENGINE_EVAS(engine) ((Ewl_Engine_Evas *)engine)

typedef struct Ewl_Engine_Evas Ewl_Engine_Evas;
struct Ewl_Engine_Evas
{
	Ewl_Engine engine;
};

#endif

