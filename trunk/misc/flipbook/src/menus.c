#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

#include "menus.h"


GtkWidget *
CreateBarSubMenu(GtkWidget *menu, char *szName) {
	GtkWidget *menuitem;
	GtkWidget *submenu;

	menuitem = gtk_menu_item_new_with_label(szName);
	gtk_menu_bar_append(GTK_MENU_BAR(menu), menuitem);
	gtk_widget_show(menuitem);
	submenu = gtk_menu_new();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem),submenu);

	return submenu;

}

GtkWidget *
CreateMenuItem(GtkWidget *menu, char *szName, char *szAccel, char *szTip,
	   	GtkSignalFunc func, gpointer data)
{
	GtkWidget *menuitem;

	if(szName && strlen (szName)) {
		menuitem = gtk_menu_item_new_with_label(szName);
		gtk_signal_connect(GTK_OBJECT(menuitem),"activate",
				GTK_SIGNAL_FUNC(func),data);
	} else {
		menuitem = gtk_menu_item_new();
	}

	gtk_menu_append(GTK_MENU(menu),menuitem);
	gtk_widget_show(menuitem);
	
	if(szAccel && szAccel[0] == '^') {
		gtk_widget_add_accelerator(menuitem,"activate",accel_group,szAccel[1],
				GDK_CONTROL_MASK,GTK_ACCEL_VISIBLE);
	}

	if(szTip && strlen(szTip)) {
		gtk_tooltips_set_tip(tooltips,menuitem,szTip,NULL);
	}

	return menuitem;

}
