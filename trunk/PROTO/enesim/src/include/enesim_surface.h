/* ENESIM - Direct Rendering Library
 * Copyright (C) 2007-2008 Jorge Luis Zapata
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
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef ENESIM_SURFACE_H_
#define ENESIM_SURFACE_H_

/**
 * @defgroup Enesim_Surface_Group Surface
 * @{
 * 
 * TODO
 * Add a pitch, this is good for different planes, still the width and height
 * are good things to have so the pitch
 */
typedef struct _Enesim_Surface 	Enesim_Surface; /**< Surface Handler */
/**
 * 
 */
typedef enum
{
	ENESIM_SURFACE_ARGB8888, /**< */
	ENESIM_SURFACE_ARGB8888_UNPRE, /**< */
	ENESIM_SURFACE_RGB565_XA5, /**< */
	ENESIM_SURFACE_RGB565_B1A3, /**< */
	ENESIM_SURFACE_RGB888_A8, /**< */
	ENESIM_SURFACE_A8, /**< */
	ENESIM_SURFACE_b1A3, /**< */
	ENESIM_SURFACE_FORMATS,
} Enesim_Surface_Format;
/* TODO add the concept of colorspace? Buffer format (texture, linear?)? */
/**
 * 
 */
typedef struct _Argb8888_Data
{
	uint32_t *plane0; /* a8r8g8b8 plane */
} Argb8888_Unpre_Data, Argb8888_Data;

typedef struct _Argb8888_Pixel
{
	uint32_t plane0;
} Argb8888_Unpre_Pixel, Argb8888_Pixel;

/**
 * 
 */
typedef struct _Rgb565_Xa5_Data
{
	uint16_t *plane0; /* r5g6b5 plane */
	uint8_t *plane1; /* a5 plane */
} Rgb565_Xa5_Data;
/**
 * 
 * 
 */
typedef struct _Rgb565_B1a3_Data
{
	uint16_t *plane0; /* r5g6b5 plane */
	uint8_t *plane1; /* b1a3 plane */
	uint8_t pixel_plane1; /* which of the pixel of plane1 to access */
} Rgb565_B1a3_Data;
/**
 * 
 */
typedef struct _Rgb888_Data
{
	uint8_t *plane0; /* r8g8b8 plane */
	uint8_t *plane1; /* a8 plane */
} Rgb888_Data;
/**
 * +---------------+----------------+
 * |     Alpha     |      Alpha     |
 * +---------------+----------------+
 *         8                8
 * <------P0------>.<------P1------>.
 * TODO how to handle this??
 */
typedef struct _A8_Data
{
	uint8_t 	*plane0; /* a8 plane */
} A8_Data;
/**
 * +-------+-------+--------+-------+
 * | Blink | Alpha |  Blink | Alpha |
 * +-------+-------+--------+-------+
 *     1       3        1        3
 * <------P0------>.<------P1------>.
 */
typedef struct _B1a3_Data
{
	uint8_t pixel_plane0; /* which of the pixel to access */
	uint8_t *plane0; /* b1A3 plane */
} B1a3_Data;
/**
 * 
 */
typedef struct _Enesim_Surface_Data
{
	union {
		Argb8888_Data argb8888;
		Argb8888_Unpre_Data argb8888_unpre;
		A8_Data a8;
		Rgb565_Xa5_Data rgb565_xa5;
		Rgb565_B1a3_Data rgb565_b1a3;
		Rgb888_Data rgb888;
	} data;
	Enesim_Surface_Format format;
} Enesim_Surface_Data;

/**
 * 
 */
typedef struct _Enesim_Surface_Pixel
{
	union {
		Argb8888_Unpre_Pixel argb8888_unpre;
		Argb8888_Pixel argb8888;
	} pixel;
	Enesim_Surface_Format format;
} Enesim_Surface_Pixel;


EAPI Enesim_Surface * enesim_surface_new_data_from(int w, int h, Enesim_Surface_Data *sdata);
EAPI Enesim_Surface * enesim_surface_new(Enesim_Surface_Format f, int w, int h);
EAPI void enesim_surface_size_get(const Enesim_Surface *s, int *w, int *h);
EAPI void enesim_surface_size_set(Enesim_Surface *s, int w, int h);
EAPI Enesim_Surface_Format enesim_surface_format_get(const Enesim_Surface *s);
EAPI void enesim_surface_data_get(const Enesim_Surface *s, Enesim_Surface_Data *sdata);
EAPI void enesim_surface_data_set(Enesim_Surface *s, const Enesim_Surface_Data *sdata);
EAPI uint32_t enesim_surface_data_argb_to(Enesim_Surface_Data *sdata);
EAPI void enesim_surface_data_argb_from(Enesim_Surface_Data *sdata, uint32_t);
EAPI void enesim_surface_data_increment(Enesim_Surface_Data *sdata, unsigned int len);
EAPI void enesim_surface_delete(Enesim_Surface *s);
EAPI const char * enesim_surface_format_name_get(Enesim_Surface_Format f);
EAPI uint32_t enesim_surface_pixel_argb_to(Enesim_Surface_Pixel *sp);
EAPI void enesim_surface_pixel_argb_from(Enesim_Surface_Pixel *dp, Enesim_Surface_Format df, uint32_t argb);
EAPI void enesim_surface_pixel_convert(Enesim_Surface_Pixel *sp, Enesim_Surface_Pixel *dp, Enesim_Surface_Format df);
EAPI void enesim_surface_pixel_components_from(Enesim_Surface_Pixel *color,
		Enesim_Surface_Format f, uint8_t a, uint8_t r, uint8_t g, uint8_t b);
EAPI void enesim_surface_pixel_components_to(Enesim_Surface_Pixel *color,
		uint8_t *a, uint8_t *r, uint8_t *g, uint8_t *b);
/** @} */ //End of Enesim_Surface_Group


#endif /*ENESIM_SURFACE_H_*/
