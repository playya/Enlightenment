#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "viewer.h"
#include "menus.h"

extern GtkTooltips *tooltips;
extern GtkAccelGroup *accel_group;

GtkWidget *clist;

void
selection_made(GtkWidget *clist, gint row, gint column, GdkEventButton *event,
		                    gpointer data)
{

	return;
}

void
on_resort_columns(GtkWidget *widget, gint column, gpointer user_data)
{
	static int order=0;
	static int last_col=0;

	if(user_data) {
		widget = NULL;
	}
	gtk_clist_set_sort_column(GTK_CLIST(clist),column);
	if(last_col == column) {
		if(order) {
			order=0;
			gtk_clist_set_sort_type(GTK_CLIST(clist),GTK_SORT_DESCENDING);
		} else {
			order=1;
			gtk_clist_set_sort_type(GTK_CLIST(clist),GTK_SORT_ASCENDING);
		}
	} else {
		order=1;
		gtk_clist_set_sort_type(GTK_CLIST(clist),GTK_SORT_ASCENDING);
		last_col = column;
	}

	gtk_clist_sort(GTK_CLIST(clist));

	return;

}


void
on_exit_application(GtkWidget * widget, gpointer user_data)
{

	if (user_data) {
		widget = NULL;
	}
	gtk_exit(0);

}

GtkWidget *
create_list_window(void)
{

	GtkWidget *list_window;
	GtkWidget *bigvbox;
	GtkWidget *menubar;
	GtkWidget *panes;
	GtkWidget *scrollybit;
	GtkWidget *vbox;


	list_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_object_set_data(GTK_OBJECT(list_window), "key_editor", list_window);
	GTK_WIDGET_SET_FLAGS(list_window, GTK_CAN_FOCUS);
	GTK_WIDGET_SET_FLAGS(list_window, GTK_CAN_DEFAULT);
	gtk_window_set_title(GTK_WINDOW(list_window), "E Keys Editor");

	bigvbox = gtk_vbox_new(FALSE,0);
	gtk_widget_show(bigvbox);
	gtk_container_add(GTK_CONTAINER(list_window), bigvbox);

	menubar = gtk_menu_bar_new();
	gtk_widget_show(menubar);
	gtk_box_pack_start(GTK_BOX(bigvbox), menubar, FALSE, FALSE, 0);

	{
		GtkWidget *menu;
		GtkWidget *menuitem;

		menu = CreateBarSubMenu(menubar,"File");
		menuitem = CreateMenuItem(menu,"Save","","Save Current Data",NULL,
				"save data");
		menuitem = CreateMenuItem(menu,"Save & Quit","",
				"Save Current Data & Quit Application",NULL, "save quit");
		menuitem = CreateMenuItem(menu,"Quit","","Quit Without Saving",NULL,
				"quit program");

	}

	{
		GtkWidget *menu;
		GtkWidget *menuitem;

		menu = CreateRightAlignBarSubMenu(menubar,"Help");
		menuitem = CreateMenuItem(menu,"About","","About E Keybinding Editor",
				NULL, "about");
		menuitem = CreateMenuItem(menu,"Documentation","",
				"Read the Keybinding Editor Documentation",NULL, "read docs");

	}

	panes = gtk_hpaned_new();
	gtk_widget_show(panes);
	gtk_paned_set_gutter_size(GTK_PANED(panes), 10);
	gtk_box_pack_start(GTK_BOX(bigvbox), panes, TRUE, TRUE, 0);

	scrollybit = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_show(scrollybit);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollybit),
			GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
	gtk_paned_pack1(GTK_PANED(panes), scrollybit, TRUE, FALSE);

	clist = gtk_clist_new(4);
	gtk_widget_show(clist);
	gtk_container_add(GTK_CONTAINER(scrollybit), clist);

	gtk_clist_set_column_title(GTK_CLIST(clist), 0, "Modifier");
	gtk_clist_set_column_title(GTK_CLIST(clist), 1, "Key");
	gtk_clist_set_column_title(GTK_CLIST(clist), 2, "Action to perform");
	gtk_clist_set_column_title(GTK_CLIST(clist), 3, "Optional Parameters");
	gtk_clist_column_titles_show(GTK_CLIST(clist));
	gtk_signal_connect(GTK_OBJECT(clist), "select_row",
			GTK_SIGNAL_FUNC(selection_made), NULL);
	gtk_signal_connect(GTK_OBJECT(clist), "click_column",
			GTK_SIGNAL_FUNC(on_resort_columns), NULL);


	return list_window;

}

int main(int argc, char *argv[])
{
	GtkWidget *lister;

	gtk_set_locale();
	gtk_init(&argc, &argv);

	tooltips = gtk_tooltips_new();
	accel_group = gtk_accel_group_new();

	lister = create_list_window();

	gtk_widget_show(lister);
	gtk_signal_connect(GTK_OBJECT(lister), "destroy",
			GTK_SIGNAL_FUNC(on_exit_application), NULL);
	gtk_signal_connect(GTK_OBJECT(lister), "delete_event",
			GTK_SIGNAL_FUNC(on_exit_application), NULL);

	gtk_main();

	return 0;
}
