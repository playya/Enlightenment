#include "evas_common.h"
#include "evas_private.h"
#include <math.h>

static void
_evas_map_calc_geom_change(Evas_Object *obj)
{
   int is, was = 0, pass = 0;

   evas_object_change(obj);
   evas_object_clip_dirty(obj);
   if (obj->layer->evas->events_frozen <= 0)
     {
	evas_object_recalc_clippees(obj);
	if (!pass)
	  {
	     if (!obj->smart.smart)
	       {
		  is = evas_object_is_in_output_rect(obj,
						     obj->layer->evas->pointer.x,
						     obj->layer->evas->pointer.y, 1, 1);
		  if ((is ^ was) && obj->cur.visible)
		    evas_event_feed_mouse_move(obj->layer->evas,
					       obj->layer->evas->pointer.x,
					       obj->layer->evas->pointer.y,
					       obj->layer->evas->last_timestamp,
					       NULL);
	       }
	  }
     }
   evas_object_inform_call_move(obj);
   evas_object_inform_call_resize(obj);
}

static void
_evas_map_calc_map_geometry(Evas_Object *obj)
{
   Evas_Coord x1, x2, y1, y2;
   const Evas_Map_Point *p, *p_end;

   if (!obj->cur.map) return;
   p = obj->cur.map->points;
   p_end = p + 4;
   x1 = p->x;
   x2 = p->x;
   y1 = p->y;
   y2 = p->y;
   p++;
   for (; p < p_end; p++)
     {
        if (p->x < x1) x1 = p->x;
        if (p->x > x2) x2 = p->x;
        if (p->y < y1) y1 = p->y;
        if (p->y > y2) y2 = p->y;
     }
   obj->cur.geometry.x = x1;
   obj->cur.geometry.y = y1;
   obj->cur.geometry.w = (x2 - x1) + 1;
   obj->cur.geometry.h = (y2 - y1) + 1;
   _evas_map_calc_geom_change(obj);
}

static inline Evas_Map *
_evas_map_new(int count)
{
   int i;
   
   Evas_Map *m = calloc(1, sizeof(Evas_Map) + count * sizeof(Evas_Map_Point));
   if (!m) return NULL;
   m->count = count;
   m->alpha = 1;
   m->smooth = 1;
   for (i = 0; i < count; i++)
     {
        m->points[i].r = 255;
        m->points[i].g = 255;
        m->points[i].b = 255;
        m->points[i].a = 255;
     }
   return m;
}

static inline Eina_Bool
_evas_map_copy(Evas_Map *dst, const Evas_Map *src)
{
   if (dst->count != src->count)
     {
	ERR("cannot copy map of different sizes: dst=%i, src=%i", dst->count, src->count);
	return EINA_FALSE;
     }
   memcpy(dst->points, src->points, src->count * sizeof(Evas_Map_Point));
   dst->smooth = src->smooth;
   dst->alpha = src->alpha;
   return EINA_TRUE;
}

static inline Evas_Map *
_evas_map_dup(const Evas_Map *orig)
{
   Evas_Map *copy = _evas_map_new(orig->count);
   if (!copy) return NULL;
   memcpy(copy->points, orig->points, orig->count * sizeof(Evas_Map_Point));
   copy->smooth = orig->smooth;
   copy->alpha = orig->alpha;
   return copy;
}

static inline void
_evas_map_free(Evas_Map *m)
{
   free(m);
}

/**
 * Enable or disable the map that is set
 * 
 * This enables the map that is set or disables it. On enable, the object
 * geometry will be saved, and the new geometry will change (position and
 * size) to reflect the map geometry set. If none is set yet, this may be
 * an undefined geometry, unless you have already set the map with
 * evas_object_map_set(). It is suggested you first set a map with
 * evas_object_map_set() with valid useful coordinatesm then enable and
 * disable the map with evas_object_map_enable_set() as needed.
 * 
 * @param obj object to enable the map on
 * @param enbled enabled state
 */
EAPI void
evas_object_map_enable_set(Evas_Object *obj, Eina_Bool enabled)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (obj->cur.usemap == !!enabled) return;
   obj->cur.usemap = enabled;
   if (enabled)
     {
        if (!obj->cur.map)
          obj->cur.map = _evas_map_new(4);
        obj->cur.map->normal_geometry = obj->cur.geometry;
     }
   else
     {
        if (obj->cur.map)
          {
             obj->cur.geometry = obj->cur.map->normal_geometry;
             _evas_map_calc_geom_change(obj);
          }
     }
   if (obj->cur.usemap) _evas_map_calc_map_geometry(obj);
}

/**
 * Get the map enabled state
 * 
 * This returns the currently enabled state of the map on the object indicated.
 * The default map enable state is off. You can enable and disable it with
 * evas_object_map_enable_set().
 * 
 * @param obj object to get the map enabled state from
 * @return the map enabled state
 */
EAPI Eina_Bool
evas_object_map_enable_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   return obj->cur.usemap;
}

/**
 * Set current object transformation map.
 * 
 * This sets the map on a given object. It is copied from the @p map pointer,
 * so there is no need to keep the @p map object if you don't need it anymore.
 * 
 * A map is a set of 4 points which have canvas x, y coordinates per point,
 * with an optional z point value as a hint for perspective correction, if it
 * is available. As well each point has u and v coordinates. These are like
 * "texture coordinates" in OpenGL in that they define a point in the source
 * image that is mapped to that map vertex/point. The u corresponds to the x
 * coordinate of this mapped point and v, the y coordinate. Note that these
 * coordinates describe a bounding region to sample. If you have a 200x100
 * source image and wannt to display it at 200x100 with proper pixel
 * precision, then do:
 * 
 * @code
 * Evas_Map *m = evas_map_new(4);
 * evas_map_point_coord_set(m, 0,   0,   0, 0);
 * evas_map_point_coord_set(m, 1, 200,   0, 0);
 * evas_map_point_coord_set(m, 2, 200, 100, 0);
 * evas_map_point_coord_set(m, 3,   0, 100, 0);
 * evas_map_point_image_uv_set(m, 0,   0,   0);
 * evas_map_point_image_uv_set(m, 1, 200,   0);
 * evas_map_point_image_uv_set(m, 2, 200, 100);
 * evas_map_point_image_uv_set(m, 3,   0, 100);
 * evas_object_map_set(obj, m);
 * evas_map_free(m);
 * @endcode
 * 
 * Note that the map points a uv coordinates match the image geometry. If
 * the @p map parameter is NULL, the sotred map will be freed and geometry
 * prior to enabling/setting a map will be restored.
 *
 * @param obj object to change transformation map
 * @param map new map to use
 *
 * @see evas_map_new()
 */
EAPI void
evas_object_map_set(Evas_Object *obj, const Evas_Map *map)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   if (!map)
     {
        if (obj->cur.map)
          {
             if (obj->cur.map->surface)
               {
                  obj->layer->evas->engine.func->image_map_surface_free
                    (obj->layer->evas->engine.data.output,
                     obj->cur.map->surface);
                  obj->cur.map->surface = NULL;
               }
             obj->cur.geometry = obj->cur.map->normal_geometry;
             if (!obj->prev.map)
               {
		  _evas_map_free(obj->cur.map);
                  obj->cur.map = NULL;
                  return;
               }
             obj->cur.map = NULL;
             if (!obj->cur.usemap) _evas_map_calc_geom_change(obj);
             else _evas_map_calc_map_geometry(obj);
          }
        return;
     }
   if (!obj->cur.map)
     {
        obj->cur.map = _evas_map_dup(map);
        obj->prev.map = NULL;
     }
   else
     {
	_evas_map_copy(obj->cur.map, map);
        obj->prev.map = NULL;
     }
   if (obj->cur.usemap) _evas_map_calc_map_geometry(obj);
}

/**
 * Get current object transformation map.
 * 
 * This returns the current internal map set on the indicated object. It is
 * intended for read-only acces and is only valid as long as the object is
 * not deleted or the map on the object is not changed. If you wish to modify
 * the map and set it back do the following:
 * 
 * @code
 * const Evas_Map *m = evas_object_map_get(obj);
 * Evas_Map *m2 = evas_map_dup(m);
 * evas_map_util_rotate(m2, 30.0, 0, 0);
 * evas_object_map_set(obj);
 * evas_map_free(m2);
 * @endcode
 *
 * @param obj object to query transformation map.
 * @return map reference to map in use. This is an internal data structure, so
 * do not modify it.
 *
 * @see evas_object_map_set()
 */
EAPI const Evas_Map *
evas_object_map_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return NULL;
   MAGIC_CHECK_END();
   if (obj->cur.map) return obj->cur.map;
   return NULL;
}

/**
 * Create map of transformation points to be later used with an evas object.
 *
 * This creates a set of points (currently only 4 is supported. no other
 * number for @p count will work). That is empty and ready to be modified
 * with evas_map calls.
 * 
 * @param count number of points in the map. *
 * @return a newly allocated map or NULL on errors.
 *
 * @see evas_map_free()
 * @see evas_map_dup()
 * @see evas_map_point_coord_set()
 * @see evas_map_point_image_uv_set()
 *
 * @see evas_object_map_set()
 */
EAPI Evas_Map *
evas_map_new(int count)
{
   if (count != 4)
     {
	ERR("num (%i) != 4 is unsupported!", count);
	return NULL;
     }
   return _evas_map_new(count);
}

/**
 * Set the smoothing for map rendering
 * 
 * This sets smoothing for map rendering. If the object is a type that has
 * its own smoothing settings, then both the smooth settings for this object
 * and the map must be turned off. By default smooth maps are enabled.
 * 
 * @param m map to modify. Must not be NULL.
 * @param enabled enable or disable smooth map rendering
 */
EAPI void
evas_map_smooth_set(Evas_Map *m, Eina_Bool enabled)
{
   if (!m) return;
   m->smooth = enabled;
}

/**
 * get the smoothing for map rendering
 * 
 * This gets smoothing for map rendering.
 * 
 * @param m map to get the smooth from. Must not be NULL.
 */
EAPI Eina_Bool
evas_map_smooth_get(const Evas_Map *m)
{
   if (!m) return 0;
   return m->smooth;
}

/**
 * Set the alpha flag for map rendering
 * 
 * This sets alpha flag for map rendering. If the object is a type that has
 * its own alpha settings, then this will take precedence. Only image objects
 * have this currently. Fits stops alpha blending of the map area, and is
 * useful if you know the object and/or all sub-objects is 100% solid.
 * 
 * @param m map to modify. Must not be NULL.
 * @param enabled enable or disable alpha map rendering
 */
EAPI void
evas_map_alpha_set(Evas_Map *m, Eina_Bool enabled)
{
   if (!m) return;
   m->alpha = enabled;
}

/**
 * get the alpha flag for map rendering
 * 
 * This gets the alph flag for map rendering.
 * 
 * @param m map to get the alpha from. Must not be NULL.
 */
EAPI Eina_Bool
evas_map_alpha_get(const Evas_Map *m)
{
   if (!m) return 0;
   return m->alpha;
}

/**
 * Copy a previously allocated map.
 * 
 * This makes a duplicate of the @p m object and returns it.
 *
 * @param m map to copy. Must not be NULL.
 * @return newly allocated map with the same count and contents as @p m.
 */
EAPI Evas_Map *
evas_map_dup(const Evas_Map *m)
{
   if (!m) return NULL;
   return _evas_map_dup(m);
}

/**
 * Free a previously allocated map.
 *
 * This frees a givem map @p m and all memory associated with it. You must NOT
 * free a map returned by evas_object_map_get() as this is internal.
 * 
 * @param m map to free.
 */
EAPI void
evas_map_free(Evas_Map *m)
{
   if (!m) return;
   _evas_map_free(m);
}

/**
 * Change the map point's coordinate.
 * 
 * This sets the fixen point's coordinate in the map. Note that points
 * describe the outline of a quadrangle and are ordered either clockwise
 * or anit-clock-wise. It is suggested to keep your quadrangles concave and
 * non-complex, though these polygon modes may work, they may not render
 * a desired set of output. The quadrangle will use points 0 and 1 , 1 and 2,
 * 2 and 3, and 3 and 0 to describe the edges of the quandrangle.
 * 
 * The X and Y and Z coordinates are in canvas units. Z is optional and may
 * or may not be honored in drawing. Z is a hint and does not affect the
 * X and Y rendered coordinates. It may be used for calculating fills with
 * perspective correct rendering.
 * 
 * Remember all coordinates are canvas global ones like with move and reize
 * in evas.
 *
 * @param m map to change point. Must not be @c NULL.
 * @param idx index of point to change. Must be smaller than map size.
 * @param x Point X Coordinate
 * @param y Point Y Coordinate
 * @param z Point Z Coordinate hint (pre-perspective transform)
 *
 * @see evas_map_util_rotate()
 * @see evas_map_util_zoom()
 */
EAPI void
evas_map_point_coord_set(Evas_Map *m, int idx, Evas_Coord x, Evas_Coord y, Evas_Coord z)
{
   Evas_Map_Point *p;
   if (!m) return;
   if (idx >= m->count) return;
   p = m->points + idx;
   p->x = x;
   p->y = y;
   p->z = z;
}

/**
 * Get the map point's coordinate.
 * 
 * This returns the coordinates of the given point in the map.
 *
 * @param m map to query point.
 * @param idx index of point to query. Must be smaller than map size.
 * @param x where to return the X coordinate.
 * @param y where to return the Y coordinate.
 * @param z where to return the Z coordinate.
 */
EAPI void
evas_map_point_coord_get(const Evas_Map *m, int idx, Evas_Coord *x, Evas_Coord *y, Evas_Coord *z)
{
   const Evas_Map_Point *p;
   
   if (!m) goto error;
   if (idx >= m->count) goto error;
   p = m->points + idx;
   if (x) *x = p->x;
   if (y) *y = p->y;
   if (z) *z = p->z;
   return;

 error:
   if (x) *x = 0;
   if (y) *y = 0;
   if (z) *z = 0;
}

/**
 * Change the map point's U and V texture source point
 *
 * This sets the U and V coordinates for the point. This determines which
 * coordinate in the source image is mapped to the given point, much like
 * OpenGL and textures. Notes that these points do select the pixel, but
 * are double floating point values to allow for accuracy and sub-pixel
 * selection.
 * 
 * @param m map to change the point of.
 * @param idx index of point to change. Must be smaller than map size.
 * @param u the X coordinate within the image/texture source
 * @param v the Y coordinate within the image/texture source
 * 
 * @see evas_map_point_coord_set()
 * @see evas_object_map_set()
 */
EAPI void
evas_map_point_image_uv_set(Evas_Map *m, int idx, double u, double v)
{
   Evas_Map_Point *p;
   if (!m) return;
   if (idx >= m->count) return;
   p = m->points + idx;
   p->u = u;
   p->v = v;
}

/**
 * Get the map point's U and V texture source points
 *
 * This returns the texture points set by evas_map_point_image_uv_set().
 * 
 * @param m map to query point.
 * @param idx index of point to query. Must be smaller than map size.
 * @param u where to write the X coordinate within the image/texture source
 * @param v where to write the Y coordinate within the image/texture source
 */
EAPI void
evas_map_point_image_uv_get(const Evas_Map *m, int idx, double *u, double *v)
{
   const Evas_Map_Point *p;
   if (!m) goto error;
   if (idx >= m->count) goto error;
   p = m->points + idx;
   if (u) *u = p->u;
   if (v) *v = p->v;
   return;

 error:
   if (u) *u = 0.0;
   if (v) *v = 0.0;
}

/**
 * Set the color of a vertex in the map
 *
 * This sets the color of the vertex in the map. Colors will be linearly
 * interpolated between vertex points through the map. Color will multiply
 * the "texture" pixels (like GL_MODULATE in OpenGL). The default color of
 * a vertex in a map is white solid (255, 255, 255, 255) which means it will
 * have no affect on modifying the texture pixels.
 * 
 * @param m map to change the color of.
 * @param idx index of point to change. Must be smaller than map size.
 * @param r red (0 - 255)
 * @param g green (0 - 255)
 * @param b blue (0 - 255)
 * @param a alpha (0 - 255)
 * 
 * @see evas_map_point_coord_set()
 * @see evas_object_map_set()
 */
EAPI void
evas_map_point_color_set(Evas_Map *m, int idx, int r, int g, int b, int a)
{
   Evas_Map_Point *p;
   if (!m) return;
   if (idx >= m->count) return;
   p = m->points + idx;
   p->r = r;
   p->g = g;
   p->b = b;
   p->a = a;
}

/**
 * Get the color set on a vertex in the map
 *
 * This gets the color set by evas_map_point_color_set() on the given vertex
 * of the map.
 * 
 * @param m map to get the color of the vertex from.
 * @param idx index of point get. Must be smaller than map size.
 * @param r pointer to red return
 * @param g pointer to green return
 * @param b pointer to blue return
 * @param a pointer to alpha return (0 - 255)
 * 
 * @see evas_map_point_coord_set()
 * @see evas_object_map_set()
 */
EAPI void
evas_map_point_color_get(const Evas_Map *m, int idx, int *r, int *g, int *b, int *a)
{
   const Evas_Map_Point *p;
   if (!m) return;
   if (idx >= m->count) return;
   p = m->points + idx;
   if (r) *r = p->r;
   if (g) *g = p->g;
   if (b) *b = p->b;
   if (a) *a = p->a;
}

/****************************************************************************/
/* util functions for manipulating maps, so you don't need to know the math */
/****************************************************************************/

/**
 * Change the map to apply the given rotation.
 * 
 * This rotates the indicated map's coordinates around the center coordinate
 * given by @p cx and @p cy as the rotation center. The points will have their
 * X and Y coordinates rotated clockwise by @p degrees degress (360.0 is a
 * full rotation). Negative values for degrees will rotate counter-clockwise
 * by that amount. All coordinates are canvas global coordinates.
 *
 * @param m map to change.
 * @param degrees amount of degrees from 0.0 to 360.0 to rotate.
 * @param cx rotation's center horizontal positon.
 * @param cy rotation's center vertical positon.
 *
 * @see evas_map_point_coord_set()
 * @see evas_map_util_zoom()
 */
EAPI void
evas_map_util_rotate(Evas_Map *m, double degrees, Evas_Coord cx, Evas_Coord cy)
{
   double r = (degrees * M_PI) / 180.0;
   Evas_Map_Point *p, *p_end;

   if (!m) return;
   p = m->points;
   p_end = p + m->count;

   for (; p < p_end; p++)
     {
        Evas_Coord x, y, xx, yy;

        xx = x = p->x - cx;
        yy = y = p->y - cy;

        xx = (x * cos(r));
        yy = (x * sin(r));
        x = xx + (y * cos(r + (M_PI / 2.0)));
        y = yy + (y * sin(r + (M_PI / 2.0)));

        p->x = x + cx;
        p->y = y + cy;
     }
}

/**
 * Change the map to apply the given zooming.
 *
 * Like evas_map_util_rotate(), this zooms the points of the map from a center
 * point. That center is defined by @p cx and @p cy. The @p zoomx and @p zoomy
 * parameters specific how much to zoom in the X and Y direction respectively.
 * A value of 1.0 means "don't zoom". 2.0 means "dobule the size". 0.5 is
 * "half the size" etc. All coordinates are canvas global coordinates.
 * 
 * @param m map to change.
 * @param zoomx horizontal zoom to use.
 * @param zoomy vertical zoom to use.
 * @param cx zooming center horizontal positon.
 * @param cy zooming center vertical positon.
 *
 * @see evas_map_point_coord_set()
 * @see evas_map_util_rotate()
 */
EAPI void
evas_map_util_zoom(Evas_Map *m, double zoomx, double zoomy, Evas_Coord cx, Evas_Coord cy)
{
   Evas_Map_Point *p, *p_end;

   if (!m) return;
   p = m->points;
   p_end = p + m->count;

   for (; p < p_end; p++)
     {
        Evas_Coord x, y;

        x = p->x - cx;
        y = p->y - cy;

        x = (((double)x) * zoomx);
        y = (((double)y) * zoomy);

        p->x = x + cx;
        p->y = y + cy;
     }
}

/**
 * XXX
 * 
 * xxx
 *
 * @param m map to change.
 * @param dx amount of degrees from 0.0 to 360.0 to rotate arount X axis.
 * @param dy amount of degrees from 0.0 to 360.0 to rotate arount Y axis.
 * @param dz amount of degrees from 0.0 to 360.0 to rotate arount Z axis.
 * @param cx rotation's center horizontal positon.
 * @param cy rotation's center vertical positon.
 * @param cz rotation's center vertical positon.
 *
 * @see evas_map_point_coord_set()
 * @see evas_map_util_zoom()
 */
EAPI void
evas_map_util_3d_rotate(Evas_Map *m, double dx, double dy, double dz, 
                        Evas_Coord cx, Evas_Coord cy, Evas_Coord cz)
{
   double rz = (dz * M_PI) / 180.0;
   double rx = (dx * M_PI) / 180.0;
   double ry = (dy * M_PI) / 180.0;
   Evas_Map_Point *p, *p_end;

   if (!m) return;
   p = m->points;
   p_end = p + m->count;

   for (; p < p_end; p++)
     {
        double x, y, z, xx, yy, zz;

        x = p->x - cx;
        y = p->y - cy;
        z = p->z - cz;
        
        if (rz != 0.0)
          {
             xx = x * cos(rz);
             yy = x * sin(rz);
             x = xx + (y * cos(rz + M_PI_2));
             y = yy + (y * sin(rz + M_PI_2));
          }

        if (ry != 0.0)
          {
             xx = x * cos(ry);
             zz = x * sin(ry);
             x = xx + (z * cos(ry + M_PI_2));
             z = zz + (z * sin(ry + M_PI_2));
          }
        
        if (rx != 0.0)
          {
             zz = z * cos(rx);
             yy = z * sin(rx);
             z = zz + (y * cos(rx + M_PI_2));
             y = yy + (y * sin(rx + M_PI_2));
          }
        
        p->x = x + cx;
        p->y = y + cy;
        p->z = z + cz;
     }
}

/**
 * XXX
 * 
 * xxx
 *
 * @param m map to change.
 * @param lx X coordinate in space of light point
 * @param ly Y coordinate in space of light point
 * @param lz Z coordinate in space of light point
 */
EAPI void
evas_map_util_3d_lighting(Evas_Map *m, 
                          Evas_Coord lx, Evas_Coord ly, Evas_Coord lz,
                          int lr, int lg, int lb, int ar, int ab, int ag)
{
   int i;
   
   if (!m) return;
   
   for (i = 0; i < m->count; i++)
     {
        double x, y, z;
        double nx, ny, nz, x1, y1, z1, x2, y2, z2, ln, br;
        int h, j, mr, mg, mb;
        
        x = m->points[i].x;
        y = m->points[i].y;
        z = m->points[i].z;
        
        // calc normal
        h = (i + m->count - 1) % m->count; // prev point
        j = (i + 1) % m->count; // next point

        x1 = m->points[h].x - x;
        y1 = m->points[h].y - y;
        z1 = m->points[h].z - z;
        
        x2 = m->points[j].x - x;
        y2 = m->points[j].y - y;
        z2 = m->points[j].z - z;
        
        nx = (y1 * z2) - (z1 * y2);
        ny = (z1 * x2) - (x1 * z2);
        nz = (x1 * y2) - (y1 * x2);
        
        ln = (nx * nx) + (ny * ny) + (nz * nz);
        ln = sqrt(ln);
        
        if (ln != 0.0)
          {
             nx /= ln;
             ny /= ln;
             nz /= ln;
          }
        
        // calc point -> light vector
        x = lx - x;
        y = ly - y;
        z = lz - z;
        
        ln = (x * x) + (y * y) + (z * z);
        ln = sqrt(ln);
        
        if (ln != 0.0)
          {
             x /= ln;
             y /= ln;
             z /= ln;
          }
        
        // brightness - tan (0.0 -> 1.0 brightness really)
        br = (nx * x) + (ny * y) + (nz * z);
        if (br < 0.0) br = 0.0;
        
        mr = ar + ((lr - ar) * br);
        mg = ag + ((lg - ag) * br);
        mb = ab + ((lb - ab) * br);
        m->points[i].r = (m->points[i].r * mr) / 255;
        m->points[i].g = (m->points[i].g * mg) / 255;
        m->points[i].b = (m->points[i].b * mb) / 255;
     }
}

/**
 * XXX
 * 
 * xxx
 *
 * @param m map to change.
 */
EAPI void
evas_map_util_3d_perspective(Evas_Map *m,
                             Evas_Coord px, Evas_Coord py,
                             Evas_Coord z0, Evas_Coord foc)
{
   Evas_Map_Point *p, *p_end;

   if (!m) return;
   p = m->points;
   p_end = p + m->count;

   for (; p < p_end; p++)
     {
        Evas_Coord x, y, zz;

        if (foc > 0)
          {
             x = p->x - px;
             y = p->y - py;
             
             zz = ((p->z - z0) + foc);
             
             if (zz > 0)
               {
                  x = (x * foc) / zz;
                  y = (y * foc) / zz;
               }
             
             p->x = px + x;
             p->y = py + y;
          }
     }
}

/**
 * XXX
 * 
 * xxx
 *
 * @param m map to query.
 * @return 1 if clockwise, 0 otherwise
 */
EAPI Eina_Bool
evas_map_util_clockwise_get(Evas_Map *m)
{
   int i, j, k, count;
   long long c;
   
   if (!m) return 0;
   if (m->count < 3) return 0;
   
   count = 0;
   for (i = 0; i < m->count; i++)
     {
        j = (i + 1) % m->count; 
        k = (i + 2) % m->count;
        c = 
          ((m->points[j].x - m->points[i].x) *
           (m->points[k].y - m->points[j].y))
          -
          ((m->points[j].y - m->points[i].y) *
           (m->points[k].x - m->points[j].x));
        if (c < 0) count--;
        else if (c > 0) count++;
     }
   if (count > 0) return 1;
   return 0;
}
