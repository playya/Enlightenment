/*
 * Copyright (C) 1997-2002, Michael Jennings
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies of the Software, its documentation and marketing & publicity
 * materials, and acknowledgment shall be given in the documentation, materials
 * and software packages that this Software was used.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _LIBAST_TYPES_H_
#define _LIBAST_TYPES_H_

/* Integer types that are guaranteed to be there and to work...more or less. :) */
typedef signed   char  spif_int8_t;
typedef unsigned char  spif_uint8_t;
typedef signed   short spif_int16_t;
typedef unsigned short spif_uint16_t;
typedef signed   int spif_int32_t;
typedef unsigned int spif_uint32_t;
typedef signed   long long spif_int64_t;
typedef unsigned long long spif_uint64_t;

typedef signed char spif_char_t;
typedef signed short spif_short_t;
typedef signed int spif_int_t;
typedef signed long spif_long_t;
typedef unsigned char spif_uchar_t;
typedef unsigned short spif_ushort_t;
typedef unsigned int spif_uint_t;
typedef unsigned long spif_ulong_t;

/* Char pointer that enforces signedness of char type */
typedef spif_char_t *spif_charptr_t;

#undef false
#undef False
#undef FALSE
#undef true
#undef True
#undef TRUE

typedef enum {
  false = 0,
  False = 0,
  FALSE = 0,
  true = 1,
  True = 1,
  TRUE = 1
} spif_bool_t;

/* Generic pointer */
typedef void *spif_ptr_t;
/* Generic function pointer */
typedef void (*spif_fptr_t)(spif_ptr_t, ...);
#endif /* _LIBAST_TYPES_H_ */
