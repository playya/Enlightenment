/* EINA - EFL data type library
 * Copyright (C) 2002-2009 Rafael Antognolli
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
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EINA_TILER_INLINE_H_
#define EINA_TILER_INLINE_H_

#include "eina_safety_checks.h"

/**
 * This struct should not be accessed directly, it is used by
 * eina_tile_grid_slicer functions to maintain context and fill "info"
 * member with correct values for given iteration.
 */
struct _Eina_Tile_Grid_Slicer
{
   unsigned long col1, col2, row1, row2; // initial and final col,row
   int tile_w, tile_h; // tile width, height
   int x_rel, y_rel; // starting x,y coordinates of the first col,row
   int w1_rel, h1_rel; // width,height of the first col,row
   int w2_rel, h2_rel; // width,height of the last col,row
   struct Eina_Tile_Grid_Info info; // info about the current tile
   Eina_Bool first;
};

/**
 * @brief Iterates over the tiles set by eina_tile_grid_slicer_setup().
 *
 * @param   slc Pointer to an Eina_Tile_Grid_Slicer struct.
 * @param   rect Pointer to a struct Eina_Tile_Grid_Info *.
 * @return  @c EINA_TRUE if the current rect is valid.
 *          @c EINA_FALSE if there is no more rects to iterate over (and
 *	       thus the current one isn't valid).
 *
 * This functions iterates over each Eina_Tile_Grid_Info *rect of the grid.
 * eina_tile_grid_slicer_setup() must be called first, and *rect is only valid
 * if this function returns EINA_TRUE. Its content shouldn't be modified.
 */
static inline Eina_Bool
eina_tile_grid_slicer_next(Eina_Tile_Grid_Slicer *slc, const Eina_Tile_Grid_Info **rect)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(slc, 0);

   if (slc->first)
     {
	slc->first = 0;
	*rect = &slc->info;
	return EINA_TRUE;
     }

   slc->info.col++;

   if (slc->info.col > slc->col2)
     {
	slc->info.row++;
	if (slc->info.row > slc->row2)
	  return EINA_FALSE;
	else if (slc->info.row < slc->row2)
	  slc->info.rect.h = slc->tile_h;
	else
	  slc->info.rect.h = slc->h2_rel;
	slc->info.rect.y = 0;
	slc->info.col = slc->col1;
	slc->info.rect.x = slc->x_rel;
	slc->info.rect.w = slc->w1_rel;
     }
   else
     {
	slc->info.rect.x = 0;
	if (slc->info.col < slc->col2)
	  slc->info.rect.w = slc->tile_w;
	else
	  slc->info.rect.w = slc->w2_rel;
     }

   if (slc->info.rect.w == slc->tile_w && slc->info.rect.h == slc->tile_h)
     slc->info.full = EINA_TRUE;
   else
     slc->info.full = EINA_FALSE;

   *rect = &slc->info;

   return EINA_TRUE;
}

/**
 * @brief Setup an Eina_Tile_Grid_Slicer struct.
 *
 * @param   slc Pointer to an Eina_Tile_Grid_Slicer struct.
 * @param   x X axis coordinate.
 * @param   y Y axis coordinate.
 * @param   w width.
 * @param   h height.
 * @param   tile_w tile width.
 * @param   tile_h tile height.
 * @return  A pointer to the Eina_Iterator.
 *          @c NULL on failure.
 *
 * This function splits the rectangle given as argument into tiles of size
 * tile_w X tile_h, and returns an iterator to them. The geometry of each
 * tile can be accessed with eina_tile_grid_slicer_next, where rect
 * will be a pointer to a struct Eina_Tile_Grid_Info.
 */
static inline Eina_Bool
eina_tile_grid_slicer_setup(Eina_Tile_Grid_Slicer *slc, int x, int y, int w, int h, int tile_w, int tile_h)
{
   int x1, x2, y1, y2;

   EINA_SAFETY_ON_NULL_RETURN_VAL(slc, 0);

   x1 = x;
   y1 = y;
   x2 = x + w - 1;
   y2 = y + h - 1;

   if (x < 0 || y < 0 || w <= 0 || h <= 0 || tile_w <= 0 || tile_h <= 0)
     {
	slc->first = 0;
	slc->col1 = slc->row1 = 0;
	slc->col2 = slc->row2 = 0;
	slc->info.col = slc->col1;
	slc->info.row = slc->row1;
	return EINA_TRUE;
     }

   slc->col1 = x1 / tile_w;
   slc->row1 = y1 / tile_h;
   slc->col2 = (x2 - 0) / tile_w;
   slc->row2 = (y2 - 0) / tile_h;
   slc->x_rel = x1 % tile_w;
   slc->y_rel = y1 % tile_h;
   slc->w1_rel = tile_w - slc->x_rel;
   slc->h1_rel = tile_h - slc->y_rel;
   slc->w2_rel = x2 % tile_w + 1;
   slc->h2_rel = y2 % tile_h + 1;

   slc->tile_w = tile_w;
   slc->tile_h = tile_h;

   slc->first = 1;
   slc->info.col = slc->col1;
   slc->info.row = slc->row1;
   slc->info.rect.x = slc->x_rel;
   slc->info.rect.y = slc->y_rel;

   if (slc->info.col == slc->col2)
     slc->w1_rel = slc->w2_rel - slc->x_rel;

   if (slc->info.row == slc->row2)
     slc->h1_rel = slc->h2_rel - slc->y_rel;

   slc->info.rect.w = slc->w1_rel;
   slc->info.rect.h = slc->h1_rel;

   if (slc->info.rect.w == slc->tile_w && slc->info.rect.h == slc->tile_h)
     slc->info.full = EINA_TRUE;
   else
     slc->info.full = EINA_FALSE;

   return EINA_TRUE;
}

#endif
