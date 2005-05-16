/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
/*
 * OLD E hints
 */
#include "config.h"
#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"
#include "Ecore_X_Atoms.h"

/*
 * Convenience macros
 */
#define _ATOM_GET(name) \
     XInternAtom(_ecore_x_disp, name, False)

Ecore_X_Atom        ECORE_X_ATOM_E_FRAME_SIZE = 0;

void
ecore_x_e_init(void)
{
   ECORE_X_ATOM_E_FRAME_SIZE = _ATOM_GET("_E_FRAME_SIZE");
}

void
ecore_x_e_frame_size_set(Ecore_X_Window win, int fl, int fr, int ft, int fb)
{
   unsigned int frames[4];

   frames[0] = fl;
   frames[1] = fr;
   frames[2] = ft;
   frames[3] = fb;
   ecore_x_window_prop_card32_set(win, ECORE_X_ATOM_E_FRAME_SIZE, frames, 4);
}
