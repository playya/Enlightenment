/*
 * Copyright (C) 2004-2005 Jaron Omega
 * Copyright (C) 2004-2005 Kim Woelders
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
#include "E.h"

#ifdef ENABLE_THEME_TRANSPARENCY
/*
 * Theme transparency settings
 */
static Dialog      *tr_sel_dialog;

static int          tmp_theme_transparency;

static int          tmp_st_border;
static int          tmp_st_widget;
static int          tmp_st_dialog;
static int          tmp_st_menu;
static int          tmp_st_tooltip;
static int          tmp_st_hilight;

static void
CB_ConfigureTrans(Dialog * d __UNUSED__, int val, void *data __UNUSED__)
{
   if (val < 2)
     {
	Conf.trans.border = tmp_st_border;
	Conf.trans.widget = tmp_st_widget;
	Conf.trans.dialog = tmp_st_dialog;
	Conf.trans.menu = tmp_st_menu;
	Conf.trans.tooltip = tmp_st_tooltip;

	if (tmp_st_hilight == ICLASS_ATTR_GLASS)
	  {
	     Conf.trans.hilight = tmp_st_hilight;
	     Conf.trans.menu_item = ICLASS_ATTR_GLASS;

	  }
	else if (tmp_st_hilight == ICLASS_ATTR_BG)
	  {
	     Conf.trans.hilight = tmp_st_hilight;
	     Conf.trans.menu_item = ICLASS_ATTR_BG;

	  }
	else if (tmp_st_hilight == ICLASS_ATTR_OPAQUE)
	  {
	     Conf.trans.hilight = tmp_st_hilight;
	     Conf.trans.menu_item = ICLASS_ATTR_OPAQUE;

	  }

	Conf.trans.pager = ICLASS_ATTR_BG;
	Conf.trans.iconbox = ICLASS_ATTR_BG;
	Conf.trans.warplist = ICLASS_ATTR_BG;

	TransparencySet(tmp_theme_transparency);
     }
   autosave();
}

static void
CB_ThemeTransparency(Dialog * d __UNUSED__, int val __UNUSED__, void *data)
{
   DItem              *di;
   char                s[256];

   di = (DItem *) data;
   Esnprintf(s, sizeof(s), _("Theme transparency: %2d"),
	     tmp_theme_transparency);
   DialogItemTextSetText(di, s);
   DialogDrawItems(tr_sel_dialog, di, 0, 0, 99999, 99999);

   /* FIXME - We may not want to do this unless things are speeded up */
   TransparencySet(tmp_theme_transparency);
}

static void
SettingsTransparency(void)
{
   Dialog             *d;
   DItem              *table, *di, *label;
   DItem              *radio_border, *radio_widget, *radio_menu,
      *radio_dialog, *radio_tooltip, *radio_hilight;
   char                s[256];

   if ((d = FindItem("CONFIGURE_TRANS", 0, LIST_FINDBY_NAME, LIST_TYPE_DIALOG)))
     {
	SoundPlay("SOUND_SETTINGS_ACTIVE");
	ShowDialog(d);
	return;
     }
   SoundPlay("SOUND_SETTINGS_TRANS");

   tmp_st_border = Conf.trans.border;
   tmp_st_widget = Conf.trans.widget;
   tmp_st_dialog = Conf.trans.dialog;
   tmp_st_menu = Conf.trans.menu;
   tmp_st_tooltip = Conf.trans.tooltip;
   tmp_st_hilight = Conf.trans.hilight;

   tmp_theme_transparency = Conf.trans.alpha;

   d = tr_sel_dialog = DialogCreate("CONFIGURE_TRANS");
   DialogSetTitle(d, _("Selective Transparency Settings"));

   table = DialogInitItem(d);
   DialogItemTableSetOptions(table, 7, 0, 0, 0);

   if (Conf.dialogs.headers)
     {
	di = DialogAddItem(table, DITEM_TEXT);
	DialogItemSetColSpan(di, 7);
	DialogItemSetPadding(di, 2, 2, 2, 2);
	DialogItemSetFill(di, 1, 0);
	DialogItemTextSetText(di,
			      _("Enlightenment Selective Transparency\n"
				"Settings Dialog\n"));

	di = DialogAddItem(table, DITEM_SEPARATOR);
	DialogItemSetColSpan(di, 7);
	DialogItemSetPadding(di, 2, 2, 2, 2);
	DialogItemSetFill(di, 1, 0);
	DialogItemSeparatorSetOrientation(di, 0);
     }

   di = DialogAddItem(table, DITEM_TEXT);
   DialogItemSetColSpan(di, 7);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetAlign(di, 0, 512);
   DialogItemTextSetText(di, _("Changes Might Require Restart:"));

   di = DialogAddItem(table, DITEM_SEPARATOR);
   DialogItemSetColSpan(di, 7);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSeparatorSetOrientation(di, 0);

   di = DialogAddItem(table, DITEM_NONE);
   DialogItemSetColSpan(di, 1);
   DialogItemSetPadding(di, 2, 10, 2, 2);
   DialogItemSetFill(di, 1, 0);

   di = DialogAddItem(table, DITEM_TEXT);
   DialogItemSetColSpan(di, 1);
   DialogItemSetPadding(di, 2, 15, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetAlign(di, 0, 512);
   DialogItemTextSetText(di, _("Borders:"));

   di = DialogAddItem(table, DITEM_TEXT);
   DialogItemSetColSpan(di, 1);
   DialogItemSetPadding(di, 2, 15, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetAlign(di, 0, 512);
   DialogItemTextSetText(di, _("Menus:"));

   di = DialogAddItem(table, DITEM_TEXT);
   DialogItemSetColSpan(di, 1);
   DialogItemSetPadding(di, 2, 15, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetAlign(di, 0, 512);
   DialogItemTextSetText(di, _("Hilights:"));

   di = DialogAddItem(table, DITEM_TEXT);
   DialogItemSetColSpan(di, 1);
   DialogItemSetPadding(di, 2, 15, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetAlign(di, 0, 512);
   DialogItemTextSetText(di, _("E Widgets:"));

   di = DialogAddItem(table, DITEM_TEXT);
   DialogItemSetColSpan(di, 1);
   DialogItemSetPadding(di, 2, 15, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetAlign(di, 0, 512);
   DialogItemTextSetText(di, _("E Dialogs:"));

   di = DialogAddItem(table, DITEM_TEXT);
   DialogItemSetColSpan(di, 1);
   DialogItemSetPadding(di, 2, 15, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetAlign(di, 0, 512);
   DialogItemTextSetText(di, _("Tooltips:"));

   di = DialogAddItem(table, DITEM_TEXT);
   DialogItemSetColSpan(di, 1);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetAlign(di, 0, 1024);
   DialogItemTextSetText(di, _("Opaque"));

   radio_border = di = DialogAddItem(table, DITEM_RADIOBUTTON);
   DialogItemSetPadding(di, 2, 15, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemRadioButtonSetFirst(di, radio_border);
   DialogItemRadioButtonGroupSetVal(di, 0);

   radio_menu = di = DialogAddItem(table, DITEM_RADIOBUTTON);
   DialogItemSetPadding(di, 2, 15, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemRadioButtonSetFirst(di, radio_menu);
   DialogItemRadioButtonGroupSetVal(di, 0);

   radio_hilight = di = DialogAddItem(table, DITEM_RADIOBUTTON);
   DialogItemSetPadding(di, 2, 15, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemRadioButtonSetFirst(di, radio_hilight);
   DialogItemRadioButtonGroupSetVal(di, 0);

   radio_widget = di = DialogAddItem(table, DITEM_RADIOBUTTON);
   DialogItemSetPadding(di, 2, 15, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemRadioButtonSetFirst(di, radio_widget);
   DialogItemRadioButtonGroupSetVal(di, 0);

   radio_dialog = di = DialogAddItem(table, DITEM_RADIOBUTTON);
   DialogItemSetPadding(di, 2, 15, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemRadioButtonSetFirst(di, radio_dialog);
   DialogItemRadioButtonGroupSetVal(di, 0);

   radio_tooltip = di = DialogAddItem(table, DITEM_RADIOBUTTON);
   DialogItemSetPadding(di, 2, 15, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemRadioButtonSetFirst(di, radio_tooltip);
   DialogItemRadioButtonGroupSetVal(di, 0);

   di = DialogAddItem(table, DITEM_TEXT);
   DialogItemSetColSpan(di, 1);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetAlign(di, 0, 1024);
   DialogItemTextSetText(di, _("Background"));

   di = DialogAddItem(table, DITEM_RADIOBUTTON);
   DialogItemSetPadding(di, 2, 15, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemRadioButtonSetFirst(di, radio_border);
   DialogItemRadioButtonGroupSetVal(di, 1);
   DialogItemRadioButtonGroupSetValPtr(radio_border, &tmp_st_border);

   di = DialogAddItem(table, DITEM_RADIOBUTTON);
   DialogItemSetPadding(di, 2, 15, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemRadioButtonSetFirst(di, radio_menu);
   DialogItemRadioButtonGroupSetVal(di, 1);

   di = DialogAddItem(table, DITEM_RADIOBUTTON);
   DialogItemSetPadding(di, 2, 15, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemRadioButtonSetFirst(di, radio_hilight);
   DialogItemRadioButtonGroupSetVal(di, 1);

   di = DialogAddItem(table, DITEM_RADIOBUTTON);
   DialogItemSetPadding(di, 2, 15, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemRadioButtonSetFirst(di, radio_widget);
   DialogItemRadioButtonGroupSetVal(di, 1);
   DialogItemRadioButtonGroupSetValPtr(radio_widget, &tmp_st_widget);

   di = DialogAddItem(table, DITEM_RADIOBUTTON);
   DialogItemSetPadding(di, 2, 15, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemRadioButtonSetFirst(di, radio_dialog);
   DialogItemRadioButtonGroupSetVal(di, 1);
   DialogItemRadioButtonGroupSetValPtr(radio_dialog, &tmp_st_dialog);

   di = DialogAddItem(table, DITEM_RADIOBUTTON);
   DialogItemSetPadding(di, 2, 15, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemRadioButtonSetFirst(di, radio_tooltip);
   DialogItemRadioButtonGroupSetVal(di, 1);

   di = DialogAddItem(table, DITEM_TEXT);
   DialogItemSetColSpan(di, 1);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetAlign(di, 0, 1024);
   DialogItemTextSetText(di, _("Glass"));

   di = DialogAddItem(table, DITEM_NONE);
   DialogItemSetPadding(di, 2, 15, 2, 2);
   DialogItemSetFill(di, 1, 0);

   di = DialogAddItem(table, DITEM_RADIOBUTTON);
   DialogItemSetPadding(di, 2, 15, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemRadioButtonSetFirst(di, radio_menu);
   DialogItemRadioButtonGroupSetVal(di, 2);
   DialogItemRadioButtonGroupSetValPtr(radio_menu, &tmp_st_menu);

   di = DialogAddItem(table, DITEM_RADIOBUTTON);
   DialogItemSetPadding(di, 2, 15, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemRadioButtonSetFirst(di, radio_hilight);
   DialogItemRadioButtonGroupSetVal(di, 2);
   DialogItemRadioButtonGroupSetValPtr(radio_hilight, &tmp_st_hilight);

   di = DialogAddItem(table, DITEM_NONE);
   DialogItemSetPadding(di, 2, 15, 2, 2);
   DialogItemSetFill(di, 1, 0);

   di = DialogAddItem(table, DITEM_NONE);
   DialogItemSetPadding(di, 2, 15, 2, 2);
   DialogItemSetFill(di, 1, 0);

   di = DialogAddItem(table, DITEM_RADIOBUTTON);
   DialogItemSetPadding(di, 2, 15, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemRadioButtonSetFirst(di, radio_tooltip);
   DialogItemRadioButtonGroupSetVal(di, 2);
   DialogItemRadioButtonGroupSetValPtr(radio_tooltip, &tmp_st_tooltip);

   di = DialogAddItem(table, DITEM_SEPARATOR);
   DialogItemSetColSpan(di, 7);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSeparatorSetOrientation(di, 0);

   di = label = DialogAddItem(table, DITEM_TEXT);
   DialogItemSetColSpan(di, 7);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSetAlign(di, 512, 512);
   Esnprintf(s, sizeof(s), _("Theme transparency: %2d"),
	     tmp_theme_transparency);
   DialogItemTextSetText(di, s);

   di = DialogAddItem(table, DITEM_SLIDER);
   DialogItemSetColSpan(di, 7);
   DialogItemSliderSetMinLength(di, 10);
   DialogItemSetPadding(di, 2, 2, 2, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSliderSetBounds(di, 0, 255);
   DialogItemSliderSetUnits(di, 1);
   DialogItemSliderSetJump(di, 16);
   DialogItemSliderSetVal(di, tmp_theme_transparency);
   DialogItemSliderSetValPtr(di, &tmp_theme_transparency);
   DialogItemSetCallback(di, CB_ThemeTransparency, 0, (void *)label);

   di = DialogAddItem(table, DITEM_SEPARATOR);
   DialogItemSetColSpan(di, 7);
   DialogItemSetPadding(di, 2, 2, 4, 2);
   DialogItemSetFill(di, 1, 0);
   DialogItemSeparatorSetOrientation(di, 0);

   DialogAddButton(d, _("OK"), CB_ConfigureTrans, 1, DIALOG_BUTTON_OK);
   DialogAddButton(d, _("Apply"), CB_ConfigureTrans, 0, DIALOG_BUTTON_APPLY);
   DialogAddButton(d, _("Close"), CB_ConfigureTrans, 1, DIALOG_BUTTON_CLOSE);
   DialogSetExitFunction(d, CB_ConfigureTrans, 2);
   DialogBindKey(d, "Escape", DialogCallbackClose, 0);
   DialogBindKey(d, "Return", CB_ConfigureTrans, 0);

   ShowDialog(d);
}

static void
TransparencySighan(int sig, void *prm __UNUSED__)
{
   switch (sig)
     {
     case ESIGNAL_CONFIGURE:
	TransparencySet(Conf.trans.alpha);
	break;
     }
}

static void
TransparencyIpc(const char *params, Client * c __UNUSED__)
{
   if (params && !strncmp(params, "cfg", 3))
     {
	SettingsTransparency();
     }
}

IpcItem             TransIpcArray[] = {
   {
    TransparencyIpc,
    "trans", "tr",
    "Transparency functions",
    "  trans cfg            Configure transparency\n"}
};
#define N_IPC_FUNCS (sizeof(TransIpcArray)/sizeof(IpcItem))

static const CfgItem TransCfgItems[] = {
   CFG_ITEM_INT(Conf.trans, alpha, 0),
   CFG_ITEM_INT(Conf.trans, menu, ICLASS_ATTR_BG),
   CFG_ITEM_INT(Conf.trans, menu_item, ICLASS_ATTR_BG),
   CFG_ITEM_INT(Conf.trans, tooltip, ICLASS_ATTR_GLASS),
   CFG_ITEM_INT(Conf.trans, widget, ICLASS_ATTR_BG),
   CFG_ITEM_INT(Conf.trans, hilight, ICLASS_ATTR_OPAQUE),
   CFG_ITEM_INT(Conf.trans, border, ICLASS_ATTR_BG),
   CFG_ITEM_INT(Conf.trans, iconbox, ICLASS_ATTR_BG),
   CFG_ITEM_INT(Conf.trans, dialog, ICLASS_ATTR_BG),
   CFG_ITEM_INT(Conf.trans, pager, ICLASS_ATTR_BG),
   CFG_ITEM_INT(Conf.trans, warplist, ICLASS_ATTR_BG),
};
#define N_CFG_ITEMS (sizeof(TransCfgItems)/sizeof(CfgItem))

/*
 * Module descriptor
 */
EModule             ModTransparency = {
   "transparency", "tr",
   TransparencySighan,
   {N_IPC_FUNCS, TransIpcArray},
   {N_CFG_ITEMS, TransCfgItems}
};

#endif /* ENABLE_THEME_TRANSPARENCY */
