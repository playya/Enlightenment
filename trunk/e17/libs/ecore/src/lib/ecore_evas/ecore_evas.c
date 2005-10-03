#include "config.h"
#include "Ecore.h"
#include "ecore_private.h"
#include "ecore_evas_private.h"
#include "Ecore_Evas.h"

static int _ecore_evas_init_count = 0;

static int _ecore_evas_idle_enter_delete(void *data);

/**
 * Query if a particular renginering engine target has support
 * @param  engine The engine to check support for
 * @return 1 if the particualr engine is supported, 0 if it is not
 * 
 * Query if engine @param engine is supported by ecore_evas. 1 is returned if
 * it is, and 0 is returned if it is not supported.
 */
int
ecore_evas_engine_type_supported_get(Ecore_Evas_Engine_Type engine)
{
   switch (engine)
     {
      case ECORE_EVAS_ENGINE_SOFTWARE_X11:
#ifdef BUILD_ECORE_X
	return 1;
#else
	return 0;
#endif	
	break;
      case ECORE_EVAS_ENGINE_SOFTWARE_FB:
#ifdef BUILD_ECORE_EVAS_FB
	return 1;
#else
	return 0;
#endif	
	break;
      case ECORE_EVAS_ENGINE_GL_X11:
#ifdef BUILD_ECORE_EVAS_GL
	return 1;
#else
	return 0;
#endif	
	break;
      case ECORE_EVAS_ENGINE_XRENDER_X11:
#ifdef BUILD_ECORE_EVAS_XRENDER
	return 1;
#else
	return 0;
#endif	
	break;
      case ECORE_EVAS_ENGINE_SOFTWARE_BUFFER:
#ifdef BUILD_ECORE_EVAS_BUFFER
	return 1;
#else
	return 0;
#endif	
	break;
      default:
	return 0;
	break;
     };
   return 0;
}

/**
 * Init the Evas system.
 * @return greater than 0 on success, 0 on failure
 * 
 * Set up the Evas wrapper system.
 */
int
ecore_evas_init(void)
{
   if (_ecore_evas_init_count == 0)
     evas_init ();
   return ++_ecore_evas_init_count;
}

/**
 * Shut down the Evas system.
 * @return 0 if ecore evas is fully shut down, or > 0 if it still needs to be shut down
 * 
 * This closes the Evas system down.
 */
int
ecore_evas_shutdown(void)
{
   _ecore_evas_init_count--;
   if (_ecore_evas_init_count == 0)
     {
#ifdef BUILD_ECORE_X
	while (_ecore_evas_x_shutdown());
#endif
#ifdef BUILD_ECORE_EVAS_FB
	while (_ecore_evas_fb_shutdown());
#endif
#ifdef BUILD_ECORE_EVAS_BUFFER
	while (_ecore_evas_buffer_shutdown());
#endif
	evas_shutdown(); 
     }
   if (_ecore_evas_init_count < 0) _ecore_evas_init_count = 0;
   return _ecore_evas_init_count;
}

/**
 * Free an Ecore_Evas
 * @param ee The Ecore_Evas to free
 *
 * This frees up any memory used by the Ecore_Evas. 
 */
void
ecore_evas_free(Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_free");
	return;
     }
   if (ee->delete_idle_enterer) return;
   ee->delete_idle_enterer = 
     ecore_idle_enterer_add(_ecore_evas_idle_enter_delete, ee);
   return;
}

void *
ecore_evas_data_get(Ecore_Evas *ee, const char *key)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_data_get");
	return NULL;
     }

   if (!key) return NULL;

   return evas_hash_find(ee->data, key);
}

void
ecore_evas_data_set(Ecore_Evas *ee, const char *key, const void *data)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_data_set");
	return;
     }

   if (!key) return;

   ee->data = evas_hash_del(ee->data, key, NULL);
   ee->data = evas_hash_add(ee->data, key, data);
}

#define IFC(_ee, _fn)  if (_ee->engine.func->_fn) {_ee->engine.func->_fn
#define IFE            return;}

/**
 * Set a callback for Ecore_Evas resize events. 
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call
 
 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever @p ee is resized.
 */
void
ecore_evas_callback_resize_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee))
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_callback_resize_set");
	return;
     }
   IFC(ee, fn_callback_resize_set) (ee, func);
   IFE;
   ee->func.fn_resize = func;
}

/**
 * Set a callback for Ecore_Evas move events. 
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call
 
 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever @p ee is moved.
 */
void
ecore_evas_callback_move_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee))
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_callback_move_set");
	return;
     }
   IFC(ee, fn_callback_move_set) (ee, func);
   IFE;
   ee->func.fn_move = func;
}

/**
 * Set a callback for Ecore_Evas show events. 
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call
 
 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever @p ee is shown.
 */
void
ecore_evas_callback_show_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee))
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_callback_show_set");
	return;
     }
   IFC(ee, fn_callback_show_set) (ee, func);
   IFE;
   ee->func.fn_show = func;
}

/**
 * Set a callback for Ecore_Evas hide events. 
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call
 
 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever @p ee is hidden.
 */
void
ecore_evas_callback_hide_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee))
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_callback_hide_set");
	return;
     }
   IFC(ee, fn_callback_hide_set) (ee, func);
   IFE;
   ee->func.fn_hide = func;
}

/**
 * Set a callback for Ecore_Evas delete request events. 
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call
 
 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever @p ee gets a delete request.
 */
void
ecore_evas_callback_delete_request_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee))
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_callback_delete_request_set");
	return;
     }
   IFC(ee, fn_callback_delete_request_set) (ee, func);
   IFE;
   ee->func.fn_delete_request = func;
}

/**
 * Set a callback for Ecore_Evas destroy events. 
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call
 
 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever @p ee is destroyed.
 */
void
ecore_evas_callback_destroy_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee))
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_callback_destroy_set");
	return;
     }
   IFC(ee, fn_callback_destroy_set) (ee, func);
   IFE;
   ee->func.fn_destroy = func;
}

/**
 * Set a callback for Ecore_Evas focus in events. 
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call
 
 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever @p ee gets focus.
 */
void
ecore_evas_callback_focus_in_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee))
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_callback_focus_in_set");
	return;
     }
   IFC(ee, fn_callback_focus_in_set) (ee, func);
   IFE;
   ee->func.fn_focus_in = func;
}

/**
 * Set a callback for Ecore_Evas focus out events. 
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call
 
 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever @p ee loses focus.
 */
void
ecore_evas_callback_focus_out_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee))
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_callback_focus_out_set");
	return;
     }
   IFC(ee, fn_callback_focus_out_set) (ee, func);
   IFE;
   ee->func.fn_focus_out = func;
}

/**
 * Set a callback for Ecore_Evas mouse in events. 
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call
 
 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever the mouse enters @p ee.
 */
void
ecore_evas_callback_mouse_in_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee))
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_callback_mouse_in_set");
	return;
     }
   IFC(ee, fn_callback_mouse_in_set) (ee, func);
   IFE;
   ee->func.fn_mouse_in = func;
}

/**
 * Set a callback for Ecore_Evas mouse out events. 
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call
 
 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called whenever the mouse leaves @p ee.
 */
void
ecore_evas_callback_mouse_out_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee))
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_callback_mouse_out_set");
	return;
     }
   IFC(ee, fn_callback_mouse_out_set) (ee, func);
   IFE;
   ee->func.fn_mouse_out = func;
}

/**
 * Set a callback for Ecore_Evas mouse pre render events. 
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call
 
 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called just before the evas in @p ee is rendered.
 */
void
ecore_evas_callback_pre_render_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee))
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_callback_pre_render_set");
	return;
     }
   IFC(ee, fn_callback_pre_render_set) (ee, func);
   IFE;
   ee->func.fn_pre_render = func;
}

/**
 * Set a callback for Ecore_Evas mouse post render events. 
 * @param ee The Ecore_Evas to set callbacks on
 * @param func The function to call
 
 * A call to this function will set a callback on an Ecore_Evas, causing
 * @p func to be called just after the evas in @p ee is rendered.
 */
void
ecore_evas_callback_post_render_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee))
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_callback_post_render_set");
	return;
     }
   IFC(ee, fn_callback_post_render_set) (ee, func);
   IFE;
   ee->func.fn_post_render = func;
}

/**
 * Get an Ecore_Evas's Evas 
 * @param ee The Ecore_Evas whose Evas you wish to get
 * @return The Evas wrapped by @p ee
 * 
 * This function returns the Evas contained within @p ee.
 */
Evas *
ecore_evas_get(Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_get");
	return NULL;
     }
   return ee->evas;
}

/**
 * Move an Ecore_Evas
 * @param ee The Ecore_Evas to move
 * @param x The x coordinate to move to
 * @param y The y coordinate to move to
 *
 * This moves @p ee to the screen coordinates (@p x, @p y)
 */
void
ecore_evas_move(Ecore_Evas *ee, int x, int y)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_move");
	return;
     }
   IFC(ee, fn_move) (ee, x, y);
   IFE;
}

/**
 * Resize an Ecore_Evas
 * @param ee The Ecore_Evas to move
 * @param w The w coordinate to resize to
 * @param h The h coordinate to resize to
 *
 * This resizes @p ee to @p w x @p h
 */
void
ecore_evas_resize(Ecore_Evas *ee, int w, int h)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_resize");
	return;
     }
   if (w < 1) w = 1;
   if (h < 1) h = 1;
   if ((ee->rotation == 90) || (ee->rotation == 270))
     {
	IFC(ee, fn_resize) (ee, h, w);
	IFE;
     }
   else
     {
	IFC(ee, fn_resize) (ee, w, h);
	IFE;
     }
}

/**
 * Resize an Ecore_Evas
 * @param ee The Ecore_Evas to move
 * @param x The x coordinate to move to
 * @param y The y coordinate to move to
 * @param w The w coordinate to resize to
 * @param h The h coordinate to resize to
 *
 * This moves @p ee to the screen coordinates (@p x, @p y) and  resizes
 * it to @p w x @p h.
 *
 */
void
ecore_evas_move_resize(Ecore_Evas *ee, int x, int y, int w, int h)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_move_resize");
	return;
     }
   if (w < 1) w = 1;
   if (h < 1) h = 1;
   if ((ee->rotation == 90) || (ee->rotation == 270))
     {
	IFC(ee, fn_move_resize) (ee, x, y, h, w);
	IFE;
     }
   else
     {
	IFC(ee, fn_move_resize) (ee, x, y, w, h);
	IFE;
     }
}

/**
 * Get the geometry of an Ecore_Evas
 * @param ee The Ecore_Evas whose geometry y
 * @param x A pointer to an int to place the x coordinate in
 * @param y A pointer to an int to place the y coordinate in
 * @param w A pointer to an int to place the w size in
 * @param h A pointer to an int to place the h size in
 *
 * This function takes four pointers to (already allocated) ints, and places
 * the geometry of @p ee in them.
 *
 * @code
 * int x, y, w, h;
 * ecore_evas_geometry_get(ee, &x, &y, &w, &h);
 * @endcode
 *
 */
void
ecore_evas_geometry_get(Ecore_Evas *ee, int *x, int *y, int *w, int *h)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_geometry_get");
	return;
     }
   if ((ee->rotation == 90) || (ee->rotation == 270))
     {
	if (x) *x = ee->x;
	if (y) *y = ee->y;
	if (w) *w = ee->h;
	if (h) *h = ee->w;
     }
   else
     {
	if (x) *x = ee->x;
	if (y) *y = ee->y;
	if (w) *w = ee->w;
	if (h) *h = ee->h;
     }
}

/**
 * Set the rotation of an Ecore_Evas' window
 *
 * @param ee The Ecore_Evas
 * @param rot the angle (in degrees) of rotation.
 *
 * The allowed values of @p rot depend on the engine being used. Most only
 * allow multiples of 90.
 */
void
ecore_evas_rotation_set(Ecore_Evas *ee, int rot)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_rotation_set");
	return;
     }
   rot = rot % 360;
   while (rot < 0) rot += 360;
   while (rot >= 360) rot -= 360;
   IFC(ee, fn_rotation_set) (ee, rot);
   IFE;
}

/**
 * Set the rotation of an Ecore_Evas' window
 *
 * @param ee The Ecore_Evas
 * @return the angle (in degrees) of rotation.
 *
 */
int
ecore_evas_rotation_get(Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_rotation_get");
	return 0;
     }
   return ee->rotation;
}

/**
 * Set whether an Ecore_Evas is shaped or not. 
 * @param ee The Ecore_Evas to shape
 * @param shaped 1 to shape, 0 to not
 *
 * This function allows one to make an Ecore_Evas shaped to the contents of the
 * evas. If @p shaped is 1, @p ee will be transparent in parts of the evas that
 * contain no objects. If @p shaped is 0, then @p ee will be rectangular, and
 * and parts with no data will show random framebuffer artifacting. For
 * non-shaped Ecore_Evases, it is recommend to cover the entire evas with a
 * background object.
 */
void
ecore_evas_shaped_set(Ecore_Evas *ee, int shaped)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_shaped_set");
	return;
     }
   IFC(ee, fn_shaped_set) (ee, shaped);
   IFE;
}

/**
 * Query whether an Ecore_Evas is shaped or not.
 * @param ee The Ecore_Evas to query.
 * @return 1 if shaped, 0 if not.
 *
 * This function returns 1 if @p ee is shaped, and 0 if not.
 */
int
ecore_evas_shaped_get(Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_shaped_get");
	return 0;
     }
   return ee->shaped ? 1:0;
}

/**
 * Show an Ecore_Evas' window 
 * @param ee The Ecore_Evas to show.
 *
 * This function makes @p ee visible.
 */
void
ecore_evas_show(Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_show");
	return;
     }
   IFC(ee, fn_show) (ee);
   IFE;
}

/**
 * Hide an Ecore_Evas' window 
 * @param ee The Ecore_Evas to show.
 *
 * This function makes @p ee hidden.
 */
void
ecore_evas_hide(Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_hide");
	return;
     }
   IFC(ee, fn_hide) (ee);
   IFE;
}

/**
 * Query whether an Ecore_Evas' window is visible or not.
 * @param ee The Ecore_Evas to query.
 * @return 1 if visible, 0 if not.
 *
 * This function queries @p ee and returns 1 if it is visible, and 0 if not.
 */
int
ecore_evas_visibility_get(Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_visibility_get");
	return 0;
     }
   return ee->visible ? 1:0;
}

/**
 * Raise and Ecore_Evas' window.
 * @param ee The Ecore_Evas to raise.
 *
 * This functions raises the Ecore_Evas to the front.
 */
void
ecore_evas_raise(Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_raise");
	return;
     }
   IFC(ee, fn_raise) (ee);
   IFE;
}

/**
 * Lower an Ecore_Evas' window.
 * @param ee The Ecore_Evas to raise.
 *
 * This functions lowers the Ecore_Evas to the back.
 */
void
ecore_evas_lower(Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_lower");
	return;
     }
   IFC(ee, fn_lower) (ee);
   IFE;
}

/**
 * Set the title of an Ecore_Evas' window
 * @param ee The Ecore_Evas whose title you wish to set.
 * @param t The title
 * 
 * This function sets the title of @p ee to @p t.
 */
void
ecore_evas_title_set(Ecore_Evas *ee, const char *t)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_title_set");
	return;
     }
   IFC(ee, fn_title_set) (ee, t);
   IFE;
}

/**
 * Get the title of an Ecore_Evas' window
 * @param ee The Ecore_Evas whose title you wish to get.
 * @return The title of @p ee.
 *
 * This function returns the title of @p ee.
 */
const char *
ecore_evas_title_get(Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_title_get");
	return NULL;
     }
   return ee->prop.title;
}

/**
 * Set the name and class of an Ecore_Evas' window
 * @param ee the Ecore_Evas
 * @param n the name
 * @param c the class
 *
 * This function sets the name of @p ee to @p n, and its class to @p c.
 */
void
ecore_evas_name_class_set(Ecore_Evas *ee, const char *n, const char *c)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_name_class_set");
	return;
     }
   IFC(ee, fn_name_class_set) (ee, n, c);
   IFE;
}

/**
 * Get the name and class of an Ecore_Evas' window
 * @p ee The Ecore_Evas to query
 * @p n A pointer to a string to place the name in.
 * @p c A pointer to a string to place the class in.
 *
 * This function gets puts the name of @p ee into @p n, and its class into
 * @p c.
 */
void
ecore_evas_name_class_get(Ecore_Evas *ee, const char **n, const char **c)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_name_class_get");
	return;
     }
   if (n) *n = ee->prop.name;
   if (c) *c = ee->prop.clas;
}

/**
 * Set the min size of an Ecore_Evas' window
 * @param ee The Ecore_Evas to set
 * @param w The minimum width
 * @param h The minimum height
 *
 * This function sets the minimum size of @p ee to @p w x @p h.
 */
void
ecore_evas_size_min_set(Ecore_Evas *ee, int w, int h)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_size_min_set");
	return;
     }
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   if ((ee->rotation == 90) || (ee->rotation == 270))
     {
	IFC(ee, fn_size_min_set) (ee, h, w);
	IFE;
     }
   else
     {
	IFC(ee, fn_size_min_set) (ee, w, h);
	IFE;
     }
}

/**
 * Get the min size of an Ecore_Evas' window
 * @param ee The Ecore_Evas to set
 * @param w A pointer to an int to place the min width in.
 * @param h A pointer to an int to place the min height in.
 *
 * This function puts the minimum size of @p ee into @p w and @p h.
 */
void
ecore_evas_size_min_get(Ecore_Evas *ee, int *w, int *h)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_size_min_get");
	return;
     }
   if ((ee->rotation == 90) || (ee->rotation == 270))
     {
	if (w) *w = ee->prop.min.h;
	if (h) *h = ee->prop.min.w;
     }
   else
     {
	if (w) *w = ee->prop.min.w;
	if (h) *h = ee->prop.min.h;
     }
}

/**
 * Set the max size of an Ecore_Evas' window
 * @param ee The Ecore_Evas to set
 * @param w The maximum width
 * @param h The maximum height
 *
 * This function sets the maximum size of @p ee to @p w x @p h.
 */
void
ecore_evas_size_max_set(Ecore_Evas *ee, int w, int h)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_size_max_set");
	return;
     }
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   if ((ee->rotation == 90) || (ee->rotation == 270))
     {
	IFC(ee, fn_size_max_set) (ee, h, w);
	IFE;
     }
   else
     {
	IFC(ee, fn_size_max_set) (ee, w, h);
	IFE;
     }
}

/**
 * Get the max size of an Ecore_Evas' window
 * @param ee The Ecore_Evas to set
 * @param w A pointer to an int to place the max width in.
 * @param h A pointer to an int to place the max height in.
 *
 * This function puts the maximum size of @p ee into @p w and @p h.
 */
void
ecore_evas_size_max_get(Ecore_Evas *ee, int *w, int *h)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_size_max_get");
	return;
     }
   if ((ee->rotation == 90) || (ee->rotation == 270))
     {
	if (w) *w = ee->prop.max.h;
	if (h) *h = ee->prop.max.w;
     }
   else
     {
	if (w) *w = ee->prop.max.w;
	if (h) *h = ee->prop.max.h;
     }
}

/**
 * Set the base size of an Ecore_Evas' window
 * @param ee The Ecore_Evas to set
 * @param w The base width
 * @param h The base height
 *
 * This function sets the base size of @p ee to @p w x @p h.
 */
void
ecore_evas_size_base_set(Ecore_Evas *ee, int w, int h)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_size_base_set");
	return;
     }
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   if ((ee->rotation == 90) || (ee->rotation == 270))
     {
	IFC(ee, fn_size_base_set) (ee, h, w);
	IFE;
     }
   else
     {
	IFC(ee, fn_size_base_set) (ee, w, h);
	IFE;
     }
}

/**
 * Get the base size of an Ecore_Evas' window
 * @param ee The Ecore_Evas to set
 * @param w A pointer to an int to place the base width in.
 * @param h A pointer to an int to place the base height in.
 *
 * This function puts the base size of @p ee into @p w and @p h.
 */
void
ecore_evas_size_base_get(Ecore_Evas *ee, int *w, int *h)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_size_base_get");
	return;
     }
   if ((ee->rotation == 90) || (ee->rotation == 270))
     {
	if (w) *w = ee->prop.base.h;
	if (h) *h = ee->prop.base.w;
     }
   else
     {
	if (w) *w = ee->prop.base.w;
	if (h) *h = ee->prop.base.h;
     }
}

/**
 * Set the step size of an Ecore_Evas
 * @param ee The Ecore_Evas to set
 * @param w The step width
 * @param h The step height
 *
 * This function sets the step size of @p ee to @p w x @p h. This limits the
 * size of an Ecore_Evas to always being an integer multiple of the step size.
 */
void
ecore_evas_size_step_set(Ecore_Evas *ee, int w, int h)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_size_step_set");
	return;
     }
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   if ((ee->rotation == 90) || (ee->rotation == 270))
     {
	IFC(ee, fn_size_step_set) (ee, h, w);
	IFE;
     }
   else
     {
	IFC(ee, fn_size_step_set) (ee, w, h);
	IFE;
     }
}

/**
 * Get the step size of an Ecore_Evas' window
 * @param ee The Ecore_Evas to set
 * @param w A pointer to an int to place the step width in.
 * @param h A pointer to an int to place the step height in.
 *
 * This function puts the step size of @p ee into @p w and @p h.
 */
void
ecore_evas_size_step_get(Ecore_Evas *ee, int *w, int *h)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_size_step_get");
	return;
     }
   if ((ee->rotation == 90) || (ee->rotation == 270))
     {
	if (w) *w = ee->prop.step.h;
	if (h) *h = ee->prop.step.w;
     }
   else
     {
	if (w) *w = ee->prop.step.w;
	if (h) *h = ee->prop.step.h;
     }
}

/**
 * Set the cursor of an Ecore_Evas
 * @param ee The Ecore_Evas
 * @param file  The path to an image file for the cursor
 * @param layer
 * @param hot_x The x coordinate of the cursor's hot spot
 * @param hot_y The y coordinate of the cursor's hot spot
 * 
 * This function makes the mouse cursor over @p ee be the image specified by
 * @p file. The actual point within the image that the mouse is at is specified
 * by @p hot_x and @p hot_y, which are coordinates with respect to the top left
 * corner of the cursor image.
 */
void
ecore_evas_cursor_set(Ecore_Evas *ee, const char *file, int layer, int hot_x, int hot_y)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_cursor_set");
	return;
     }
   IFC(ee, fn_cursor_set) (ee, file, layer, hot_x, hot_y);
   IFE;
}

/**
 * Get information about an Ecore_Evas' cursor
 * @param ee The Ecore_Evas to set
 * @param file A pointer to a string to place the cursor file name in.
 * @param layer A pointer to an int to place the cursor's layer in..
 * @param hot_x A pointer to an int to place the cursor's hot_x coordinate in.
 * @param hot_y A pointer to an int to place the cursor's hot_y coordinate in.
 *
 * This function queries information about an Ecore_Evas' cursor.
 */
void
ecore_evas_cursor_get(Ecore_Evas *ee, char **file, int *layer, int *hot_x, int *hot_y)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_cursor_get");
	return;
     }
   if (file) *file = ee->prop.cursor.file;
   if (layer) *layer = ee->prop.cursor.layer;
   if (hot_x) *hot_x = ee->prop.cursor.hot.x;
   if (hot_y) *hot_y = ee->prop.cursor.hot.y;
}

/**
 * Set the layer of an Ecore_Evas' window
 * @param ee The Ecore_Evas
 * @param layer The layer to put @p ee on.
 *
 * This function moves @p ee to the layer @p layer.
 */
void
ecore_evas_layer_set(Ecore_Evas *ee, int layer)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_layer_set");
	return;
     }
   IFC(ee, fn_layer_set) (ee, layer);
   IFE;
}

/**
 * Get the layer of an Ecore_Evas' window
 * @param ee The Ecore_Evas to set
 * @return the layer @p ee's window is on.
 *
 */
int
ecore_evas_layer_get(Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_layer_get");
	return 0;
     }
   return ee->prop.layer;
}

/**
 * Set the focus of an Ecore_Evas' window
 * @param ee The Ecore_Evas
 * @param on 1 for focus, 0 to defocus.
 *
 * This function focuses @p ee if @p on is 1, or defocuses @p ee if @p on is 0.
 */
void
ecore_evas_focus_set(Ecore_Evas *ee, int on)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_focus_set");
	return;
     }
   IFC(ee, fn_focus_set) (ee, on);
   IFE;
}

/**
 * Query whether an Ecore_Evas' window is focused or not
 * @param ee The Ecore_Evas to set
 * @return 1 if @p ee if focused, 0 if not.
 *
 */
int
ecore_evas_focus_get(Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_focus_get");
	return 0;
     }
   return ee->prop.focused ? 1:0;
}

/**
 * Iconify or uniconify an Ecore_Evas' window
 * @param ee The Ecore_Evas
 * @param on 1 to iconify, 0 to uniconify.
 *
 * This function iconifies @p ee if @p on is 1, or uniconifies @p ee if @p on
 * is 0.
 */
void
ecore_evas_iconified_set(Ecore_Evas *ee, int on)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_iconified_set");
	return;
     }
   IFC(ee, fn_iconified_set) (ee, on);
   IFE;
}

/**
 * Query whether an Ecore_Evas' window is iconified or not
 * @param ee The Ecore_Evas to set
 * @return 1 if @p ee is iconified, 0 if not.
 *
 */
int
ecore_evas_iconified_get(Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_iconified_get");
	return 0;
     }
   return ee->prop.iconified ? 1:0;
}

/**
 * Set whether an Ecore_Evas' window is borderless or not
 * @param ee The Ecore_Evas
 * @param on 1 for borderless, 0 for bordered.
 *
 * This function makes @p ee borderless if @p on is 1, or bordered if @p on
 * is 0.
 */
void
ecore_evas_borderless_set(Ecore_Evas *ee, int on)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_borderless_set");
	return;
     }
   IFC(ee, fn_borderless_set) (ee, on);
   IFE;
}

/**
 * Query whether an Ecore_Evas' window is borderless or not
 * @param ee The Ecore_Evas to set
 * @return 1 if @p ee is borderless, 0 if not.
 *
 */
int
ecore_evas_borderless_get(Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_borderless_get");
	return 0;
     }
   return ee->prop.borderless ? 1:0;
}

/**
 * Tell the WM whether or not to ignore an Ecore_Evas' window
 * @param ee The Ecore_Evas
 * @param on 1 to ignore, 0 to not.
 *
 * This function causes the window manager to ignore @p ee if @p on is 1,
 * or not ignore @p ee if @p on is 0.
 */
void
ecore_evas_override_set(Ecore_Evas *ee, int on)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_override_set");
	return;
     }
   IFC(ee, fn_override_set) (ee, on);
   IFE;
}

/**
 * Query whether an Ecore_Evas' window is overridden or not
 * @param ee The Ecore_Evas to set
 * @return 1 if @p ee is overridden, 0 if not.
 *
 */
int
ecore_evas_override_get(Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_override_get");
	return 0;
     }
   return ee->prop.override ? 1:0;
}

/**
 * Maximize (or unmaximize) an Ecore_Evas' window
 * @param ee The Ecore_Evas
 * @param on 1 to maximize, 0 to unmaximize.
 *
 * This function maximizes @p ee if @p on is 1, or unmaximizes @p ee
 * if @p on is 0.
 */
void
ecore_evas_maximized_set(Ecore_Evas *ee, int on)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_maximized_set");
	return;
     }
   IFC(ee, fn_maximized_set) (ee, on);
   IFE;
}

/**
 * Query whether an Ecore_Evas' window is maximized or not
 * @param ee The Ecore_Evas to set
 * @return 1 if @p ee is maximized, 0 if not.
 *
 */
int
ecore_evas_maximized_get(Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_maximized_get");
	return 0;
     }
   return ee->prop.maximized ? 1:0;
}

/**
 * Set whether or not an Ecore_Evas' window is fullscreen
 * @param ee The Ecore_Evas
 * @param on 1 fullscreen, 0 not.
 *
 * This function causes @p ee to be fullscreen if @p on is 1,
 * or not if @p on is 0.
 */
void
ecore_evas_fullscreen_set(Ecore_Evas *ee, int on)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_fullscreen_set");
	return;
     }
   IFC(ee, fn_fullscreen_set) (ee, on);
   IFE;
}

/**
 * Query whether an Ecore_Evas' window is fullscreen or not
 * @param ee The Ecore_Evas to set
 * @return 1 if @p ee is fullscreen, 0 if not.
 *
 */
int
ecore_evas_fullscreen_get(Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_fullscreen_get");
	return 0;
     }
   return ee->prop.fullscreen ? 1:0;
}

/** 
 * Set whether or not an Ecore_Evas' window should avoid damage  
 *
 * @param ee The Ecore_Evas
 * @param on 1 to avoid damage, 0 to not
 * 
 * This function causes @p ee to be drawn to a pixmap to avoid recalculations.
 * On expose events it will copy from the pixmap to the window.
 */
void
ecore_evas_avoid_damage_set(Ecore_Evas *ee, int on)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_avoid_damage_set");
	return;
     }
   IFC(ee, fn_avoid_damage_set) (ee, on);
   IFE;
}

/**
 * Query whether an Ecore_Evas' window avoids damage or not
 * @param ee The Ecore_Evas to set
 * @return 1 if @p ee avoids damage, 0 if not.
 *
 */
int
ecore_evas_avoid_damage_get(Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
	ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
			 "ecore_evas_avoid_damage_get");
	return 0;
     }
   return ee->prop.avoid_damage ? 1:0;
}

/**
 * Set the withdrawn state of an Ecore_Evas' window.
 * @param ee The Ecore_Evas whose window's withdrawn state is set.
 * @param withdrawn The Ecore_Evas window's new withdrawn state.
 *
 */
void
ecore_evas_withdrawn_set(Ecore_Evas *ee, int withdrawn)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
   {
      ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
         "ecore_evas_withdrawn_set");
      return;
   }
   
   IFC(ee, fn_withdrawn_set) (ee, withdrawn);
   IFE;
}

/**
 * Returns the withdrawn state of an Ecore_Evas' window.
 * @param ee The Ecore_Evas whose window's withdrawn state is returned.
 * @return The Ecore_Evas window's withdrawn state.
 *
 */
int
ecore_evas_withdrawn_get(Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
   {
      ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
         "ecore_evas_withdrawn_get");
      return 0;
   } else
      return ee->prop.withdrawn ? 1:0;
}

/**
 * Set the sticky state of an Ecore_Evas window.
 *
 * @param ee The Ecore_Evas whose window's sticky state is set.
 * @param sticky The Ecore_Evas window's new sticky state.
 *
 */
void
ecore_evas_sticky_set(Ecore_Evas *ee, int sticky)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
   {
      ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
         "ecore_evas_sticky_set");
      return;
   }
   
   IFC(ee, fn_sticky_set) (ee, sticky);
   IFE;
}

/**
 * Returns the sticky state of an Ecore_Evas' window.
 * 
 * @param ee The Ecore_Evas whose window's sticky state is returned.
 * @return The Ecore_Evas window's sticky state.
 *
 */
int
ecore_evas_sticky_get(Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
   {
      ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
         "ecore_evas_sticky_get");
      return 0;
   } else
      return ee->prop.sticky ? 1:0;
}

#ifndef WIN32
/* fps debug calls - for debugging how much time your app actually spends */
/* rendering graphics... :) */

static int _ecore_evas_fps_debug_init_count = 0;
static int _ecore_evas_fps_debug_fd = -1;
unsigned int *_ecore_evas_fps_rendertime_mmap = NULL;

void
_ecore_evas_fps_debug_init(void)
{
   char buf[4096];
   
   _ecore_evas_fps_debug_init_count++;
   if (_ecore_evas_fps_debug_init_count > 1) return;
   snprintf(buf, sizeof(buf), "/tmp/.ecore_evas_fps_debug-%i", (int)getpid());
   _ecore_evas_fps_debug_fd = open(buf, O_CREAT | O_TRUNC | O_RDWR);
   if (_ecore_evas_fps_debug_fd < 0)
     {
	unlink(buf);
	_ecore_evas_fps_debug_fd = open(buf, O_CREAT | O_TRUNC | O_RDWR);
     }
   if (_ecore_evas_fps_debug_fd >= 0)
     {
	unsigned int zero = 0;
	
	write(_ecore_evas_fps_debug_fd, &zero, sizeof(unsigned int));
	_ecore_evas_fps_rendertime_mmap = mmap(NULL, sizeof(unsigned int),
					       PROT_READ | PROT_WRITE,
					       MAP_SHARED,
					       _ecore_evas_fps_debug_fd, 0);
     }
}

void
_ecore_evas_fps_debug_shutdown(void)
{
   _ecore_evas_fps_debug_init_count--;
   if (_ecore_evas_fps_debug_init_count > 0) return;
   if (_ecore_evas_fps_debug_fd >= 0)
     {
	char buf[4096];
	
	snprintf(buf, sizeof(buf), "/tmp/.ecore_evas_fps_debug-%i", (int)getpid());
	unlink(buf);
	if (_ecore_evas_fps_rendertime_mmap)
	  {
	     munmap(_ecore_evas_fps_rendertime_mmap, sizeof(int));
	     _ecore_evas_fps_rendertime_mmap = NULL;
	  }
	close(_ecore_evas_fps_debug_fd);
	_ecore_evas_fps_debug_fd = -1;
     }
}

void
_ecore_evas_fps_debug_rendertime_add(double t)
{
   if ((_ecore_evas_fps_debug_fd >= 0) && 
       (_ecore_evas_fps_rendertime_mmap))
     {
	unsigned int tm;
	
	tm = (unsigned int)(t * 1000000.0);
	/* i know its not 100% theoretically guaranteed, but i'd say a write */
	/* of an int could be considered atomic for all practical purposes */
	/* oh and since this is cumulative, 1 second = 1,000,000 ticks, so */
	/* this can run for about 4294 seconds becore looping. if you are */
	/* doing performance testing in one run for over an hour... well */
	/* time to restart or handle a loop condition :) */
	*(_ecore_evas_fps_rendertime_mmap) += tm;
     }
}
#endif

void
_ecore_evas_free(Ecore_Evas *ee)
{
   ECORE_MAGIC_SET(ee, ECORE_MAGIC_NONE);
   if (ee->delete_idle_enterer)
     {
	ecore_idle_enterer_del(ee->delete_idle_enterer);
	ee->delete_idle_enterer = NULL;
     }
   while (ee->sub_ecore_evas)
     {
	_ecore_evas_free(ee->sub_ecore_evas->data);
     }
   if (ee->data) evas_hash_free(ee->data);
   if (ee->driver) free(ee->driver);
   if (ee->name) free(ee->name);
   if (ee->prop.title) free(ee->prop.title);
   if (ee->prop.name) free(ee->prop.name);
   if (ee->prop.clas) free(ee->prop.clas);
   if (ee->prop.cursor.file) free(ee->prop.cursor.file);
   if (ee->prop.cursor.object) evas_object_del(ee->prop.cursor.object);
   if (ee->evas) evas_free(ee->evas);
   ee->data = NULL;
   ee->driver = NULL;
   ee->name = NULL;
   ee->prop.title = NULL;
   ee->prop.name = NULL;
   ee->prop.clas = NULL;
   ee->prop.cursor.file = NULL;
   ee->prop.cursor.object = NULL;
   ee->evas = NULL;
   if (ee->engine.func->fn_free) ee->engine.func->fn_free(ee);
   free(ee);
}

static int
_ecore_evas_idle_enter_delete(void *data)
{
   Ecore_Evas *ee;
   
   ee = (Ecore_Evas *)data;
   _ecore_evas_free(ee);
   return 0;
}
