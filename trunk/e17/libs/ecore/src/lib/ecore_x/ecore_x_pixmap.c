#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
Ecore_X_Pixmap
ecore_x_pixmap_new(Ecore_X_Window win, int w, int h, int dep)
{
   if (win == 0) win = DefaultRootWindow(_ecore_x_disp);
   if (dep == 0) dep = DefaultDepth(_ecore_x_disp, DefaultScreen(_ecore_x_disp));
   return XCreatePixmap(_ecore_x_disp, win, w, h, dep);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_x_pixmap_del(Ecore_X_Pixmap pmap)
{
   XFreePixmap(_ecore_x_disp, pmap);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_x_pixmap_paste(Ecore_X_Pixmap pmap, Ecore_X_Drawable dest, 
		     Ecore_X_GC gc, int sx, int sy, 
		     int w, int h, int dx, int dy)
{
   XCopyArea(_ecore_x_disp, pmap, dest, gc, sx, sy, w, h, dx, dy);
}
