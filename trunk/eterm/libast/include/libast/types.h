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

/* Generic pointer */
typedef void *spif_ptr_t;
/* Generic function pointer */
typedef void (*spif_fptr_t)(spif_ptr_t, ...);
/* An object instance */
typedef struct spif_obj_t_struct spif_obj_t;
/* An object class definition */
typedef struct spif_class_t_struct spif_class_t;
/* Pointer to a generic object member function */
typedef void (*spif_ofptr_t)(spif_obj_t *, ...);

/* This type defines an object class data structure.  It holds pointers to the
   member functions for a particular class, as well as its parent class and a
   set of flags denoting properties of the class. */
struct spif_class_t_struct {
  spif_class_t *parent;
  spif_uint32_t flags;
};

/* This type is used for an actual class instance.  There may be many objects
   declared as spif_obj_t's, but each class has exactly one spif_class_t which
   is used by all instances of that object type. */
struct spif_obj_t_struct {
  spif_obj_t *parent;
  spif_class_t *class;
};

#endif /* _LIBAST_TYPES_H_ */
