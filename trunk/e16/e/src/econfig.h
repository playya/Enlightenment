
/*
 * Copyright (C) 2003-2004 Kim Woelders
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
#ifndef _ECONFIG_H_
#define _ECONFIG_H_

typedef struct
{
   const char         *name;
   void               *ptr;
   char                type;
   long                dflt;
} CfgItem;

typedef enum
{
   ITEM_TYPE_BOOL,
   ITEM_TYPE_INT,
   ITEM_TYPE_FLOAT,
   ITEM_TYPE_STRING
} cfg_item_type_e;

#define CFG_ITEM_BOOL(conf, name, dflt)  { #name, &conf.name, ITEM_TYPE_BOOL, dflt }
#define CFG_ITEM_INT(conf, name, dflt)   { #name, &conf.name, ITEM_TYPE_INT, dflt }
#define CFG_ITEM_STR(conf, name)         { #name, &conf.name, ITEM_TYPE_STRING, 0 }

/* Change to this? */
#define CFR_ITEM_BOOL(conf, name, dflt)  { #name, &conf, ITEM_TYPE_BOOL, dflt }

const CfgItem      *CfgItemFind(const CfgItem * pcl, int ncl, const char *name);
void                CfgItemToString(const CfgItem * ci, char *buf, int len);

int                 CfgItemListNamedItemSet(const CfgItem * pcl, int ncl,
					    const char *item,
					    const char *value);

int                 CfgItemListNamedItemToString(const CfgItem * pcl, int ncl,
						 const char *item, char *buf,
						 int len);

#endif /* _ECONFIG_H_ */
