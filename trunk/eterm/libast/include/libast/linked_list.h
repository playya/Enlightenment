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

#ifndef _LIBAST_LINKED_LIST_H_
#define _LIBAST_LINKED_LIST_H_

/*
 * interface goop
 */

/* Standard typecast macros.... */
#define SPIF_LINKED_LIST_ITEM(obj)                 (SPIF_CAST(linked_list_item) (obj))
#define SPIF_LINKED_LIST(obj)                      (SPIF_CAST(linked_list) (obj))

#define SPIF_LINKED_LIST_ITEM_ISNULL(o)            (SPIF_LINKED_LIST_ITEM(o) == SPIF_NULL_TYPE(linked_list_item))
#define SPIF_OBJ_IS_LINKED_LIST_ITEM(o)            (SPIF_OBJ_IS_TYPE((o), linked_list_item))

typedef struct spif_linked_list_item_t_struct *spif_linked_list_item_t;
typedef struct spif_linked_list_item_t_struct spif_const_linked_list_item_t;

struct spif_linked_list_item_t_struct {
    spif_nullobj_t parent;
    spif_obj_t data;
    spif_linked_list_item_t next;
};

typedef struct spif_linked_list_t_struct *spif_linked_list_t;
typedef struct spif_linked_list_t_struct spif_const_linked_list_t;

struct spif_linked_list_t_struct {
  spif_obj_t parent;
  size_t len;
  spif_linked_list_item_t head;
};

extern spif_listclass_t SPIF_CLASS_VAR(linked_list);
#endif /* _LIBAST_LINKED_LIST_H_ */
