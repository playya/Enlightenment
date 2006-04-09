/*
 * Copyright (C) 2000-2006 Carsten Haitzler, Geoff Harrison and various contributors
 * Copyright (C) 2004-2006 Kim Woelders
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
#ifndef _DIALOG_H_
#define _DIALOG_H_

/* Dialog items */
#define DITEM_NONE         0
#define DITEM_BUTTON       1
#define DITEM_CHECKBUTTON  2
#define DITEM_TEXT         3
#define DITEM_IMAGE        4
#define DITEM_SEPARATOR    5
#define DITEM_TABLE        6
#define DITEM_RADIOBUTTON  7
#define DITEM_SLIDER       8
#define DITEM_AREA         9

/* Dialog button icons */
#define DLG_BUTTON_NONE    0
#define DLG_BUTTON_OK      1
#define DLG_BUTTON_CANCEL  2
#define DLG_BUTTON_APPLY   3
#define DLG_BUTTON_CLOSE   4

/* Dialog footer flags */
#define DLG_OAC 7
#define DLG_OC  5

typedef struct _dialog Dialog;
typedef struct _ditem DItem;

typedef void        (DialogCallbackFunc) (Dialog * d, int val, void *data);
typedef void        (DialogItemCallbackFunc) (int val, void *data);

/* dialog.c */
Dialog             *DialogCreate(const char *name);
Dialog             *DialogFind(const char *name);
void                DialogBindKey(Dialog * d, const char *key,
				  DialogCallbackFunc * func, int val);
void                DialogSetText(Dialog * d, const char *text);
void                DialogSetTitle(Dialog * d, const char *title);
void                DialogSetExitFunction(Dialog * d, DialogCallbackFunc * func,
					  int val);
void                DialogSetData(Dialog * d, void *data);
void               *DialogGetData(Dialog * d);

void                DialogRedraw(Dialog * d);
void                ShowDialog(Dialog * d);
void                DialogClose(Dialog * d);

void                DialogAddButton(Dialog * d, const char *text,
				    DialogCallbackFunc * func, char doclose,
				    int image);
void                DialogAddHeader(Dialog * d, const char *img,
				    const char *txt);
void                DialogAddFooter(Dialog * d, int flags,
				    DialogCallbackFunc * cb);
DItem              *DialogInitItem(Dialog * d);
DItem              *DialogAddItem(DItem * dii, int type);
void                DialogItemSetCallback(DItem * di, DialogCallbackFunc * func,
					  int val, void *data);
void                DialogItemSetClass(DItem * di, ImageClass * ic,
				       TextClass * tclass);
void                DialogItemSetPadding(DItem * di, int left, int right,
					 int top, int bottom);
void                DialogItemSetFill(DItem * di, char fill_h, char fill_v);
void                DialogItemSetAlign(DItem * di, int align_h, int align_v);
void                DialogItemSetText(DItem * di, const char *text);
void                DialogItemCallCallback(Dialog * d, DItem * di);
void                DialogDrawItems(Dialog * d, DItem * di, int x, int y, int w,
				    int h);
void                DialogItemRadioButtonSetEventFunc(DItem * di,
						      DialogItemCallbackFunc *
						      func);
void                DialogItemCheckButtonSetState(DItem * di, char onoff);
void                DialogItemCheckButtonSetPtr(DItem * di, char *onoff_ptr);
void                DialogItemTableSetOptions(DItem * di, int num_columns,
					      char border, char homogenous_h,
					      char homogenous_v);
void                DialogItemSeparatorSetOrientation(DItem * di,
						      char horizontal);
void                DialogItemImageSetFile(DItem * di, const char *image);
void                DialogItemSetRowSpan(DItem * di, int row_span);
void                DialogItemSetColSpan(DItem * di, int col_span);
void                DialogItemRadioButtonSetFirst(DItem * di, DItem * first);
void                DialogItemRadioButtonGroupSetValPtr(DItem * di,
							int *val_ptr);
void                DialogItemRadioButtonGroupSetVal(DItem * di, int val);

void                DialogItemSliderSetVal(DItem * di, int val);
void                DialogItemSliderSetBounds(DItem * di, int lower, int upper);
void                DialogItemSliderSetUnits(DItem * di, int units);
void                DialogItemSliderSetJump(DItem * di, int jump);
void                DialogItemSliderSetMinLength(DItem * di, int min);
void                DialogItemSliderSetValPtr(DItem * di, int *val_ptr);
void                DialogItemSliderSetOrientation(DItem * di, char horizontal);
int                 DialogItemSliderGetVal(DItem * di);
void                DialogItemSliderGetBounds(DItem * di, int *lower,
					      int *upper);

void                DialogItemAreaSetSize(DItem * di, int w, int h);
void                DialogItemAreaGetSize(DItem * di, int *w, int *h);
Window              DialogItemAreaGetWindow(DItem * di);
void                DialogItemAreaSetEventFunc(DItem * di,
					       DialogItemCallbackFunc * func);

void                DialogCallbackClose(Dialog * d, int val, void *data);

void                DialogsInit(void);

EWin               *FindEwinByDialog(Dialog * d);

#endif /* _DIALOG_H_ */
