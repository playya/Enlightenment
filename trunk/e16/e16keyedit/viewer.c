#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "viewer.h"
#include "menus.h"
#include "ipc.h"

extern GtkTooltips *tooltips;
extern GtkAccelGroup *accel_group;

GtkWidget *clist;
GtkWidget *act_key;
GtkWidget *act_params;
GtkWidget *act_mod;
GtkWidget *act_clist;

static void receive_ipc_msg(gchar * msg);
static gchar *wait_for_ipc_msg(void);

gchar *e_ipc_msg = NULL;
GList *keys = NULL;
char dont_update=0;


typedef struct _keybind {
	char *key;
	gint modifier;
	gint id;
	char *params;
	gint action_id;
} Keybind;

typedef struct _actionopt {
	gchar *text;
	gint id;
	gchar param_tpe;
	gchar *params;
} ActionOpt;

gchar *mod_str[] = {
	    "",
		"CTRL",
		"ALT",
		"SHIFT",
		"CTRL+ALT",
		"CTRL+SHIFT",
		"ALT+SHIFT",
		"CTRL+ALT+SHIFT",
		"WIN",
		"MOD3",
		"MOD4",
		"MOD5",
		"WIN+SHIFT",
		"WIN+CTRL",
		"WIN+ALT",
		"MOD4+SHIFT",
		"MOD4+CTRL",
		"MOD4+ALT",
		"MOD4+CTRL+SHIFT",
		"MOD5+SHIFT",
		"MOD5+CTRL"
};


static ActionOpt actions[] = {
	{"Run command", 1, 1, NULL},

	{"Restart Enlightenment", 7, 0, "restart"},
	{"Exit Enlightenment", 7, 0, NULL},

	{"Goto Next Desktop", 15, 0, NULL},
	{"Goto Previous Deskop", 16, 0, NULL},
	{"Goto Desktop", 42, 2, NULL},
	{"Raise Desktop", 17, 0, NULL},
	{"Lower Desktop", 18, 0, NULL},
	{"Reset Desktop In Place", 21, 0, NULL},

	{"Toggle Deskrays", 43, 0, NULL},

	{"Cleanup Windows", 8, 0, NULL},
	{"Scroll Windows to left", 48, 0, "-16 0"},
	{"Scroll Windows to right", 48, 0, "16 0"},
	{"Scroll Windows up", 48, 0, "0 -16"},
	{"Scroll Windows down", 48, 0, "0 16"},
	{"Scroll Windows by [X Y] pixels", 48, 3, NULL},

	{"Move mouse pointer to left", 66, 0, "-1 0"},
	{"Move mouse pointer to right", 66, 0, "1 0"},
	{"Move mouse pointer up", 66, 0, "0 -1"},
	{"Move mouse pointer down", 66, 0, "0 1"},
	{"Move mouse pointer by [X Y]", 66, 3, NULL},

	{"Goto Desktop area [X Y]", 62, 3, NULL},
	{"Move to Desktop area on the left", 63, 0, "-1 0"},
	{"Move to Desktop area on the right", 63, 0, "1 0"},
	{"Move to Desktop area above", 63, 0, "0 -1"},
	{"Move to Desktop area below", 63, 0, "0 1"},

	{"Raise Window", 5, 0, NULL},
	{"Lower Window", 6, 0, NULL},
	{"Close Window", 13, 0, NULL},
	{"Annihilate Window", 14, 0, NULL},
	{"Stick / Unstick Window", 20, 0, NULL},
	{"Iconify Window", 46, 0, NULL},
	{"Shade / Unshade Window", 49, 0, NULL},
	{"Maximise Height of Window", 50, 0, "conservative"},
	{"Maximise Height of Window to whole screen", 50, 0, NULL},
	{"Maximise Height of Window toavailable space", 50, 0, "available"},
	{"Maximise Width of Window", 51, 0, "conservative"},
	{"Maximise Width of Window to whole screen", 51, 0, NULL},
	{"Maximise Width of Window toavailable space", 51, 0, "available"},
	{"Maximise Size of Window", 52, 0, "conservative"},
	{"Maximise Size of Window to whole screen", 52, 0, NULL},
	{"Maximise Size of Window toavailable space", 52, 0, "available"},
	{"Send window to next desktop", 53, 0, NULL},
	{"Send window to previous desktop", 54, 0, NULL},
	{"Switch focus to next window", 58, 0, NULL},
	{"Switch focus to previous window", 59, 0, NULL},
	{"Glue / Unglue Window to Desktop screen", 64, 0, NULL},
	{"Set Window layer to On Top", 65, 0, "20"},
	{"Set Window layer to Above", 65, 0, "6"},
	{"Set Window layer to Normal", 65, 0, "4"},
	{"Set Window layer to Below", 65, 0, "2"},
	{"Set Window layer", 65, 2, NULL},
	{"Move Window to area on left", 0, 0, "-1 0"},
	{"Move Window to area on right", 0, 0, "1 0"},
	{"Move Window to area above", 0, 0, "0 -1"},
	{"Move Window to area below", 0, 0, "0 1"},
	{"Move Window by area [X Y]", 0, 3, NULL},

	{"Set Window border style to the Default", 69, 0, "DEFAULT"},
	{"Set Window border style to the Borderless", 69, 0, "BORDERLESS"},

	{"Forget everything about Window", 55, 0, "none"},
	{"Remember all Window settings", 55, 0, NULL},
	{"Remember Window Border", 55, 0, "border"},
	{"Remember Window Desktop", 55, 0, "desktop"},
	{"Remember Window Desktop Area", 55, 0, "area"},
	{"Remember Window Size", 55, 0, "size"},
	{"Remember Window Location", 55, 0, "location"},
	{"Remember Window Layer", 55, 0, "layer"},
	{"Remember Window Stickyness", 55, 0, "sticky"},
	{"Remember Window Shadedness", 55, 0, "shade"},

	{"Show Root Menu", 9, 0, "ROOT_2"},
	{"Show Winops Menu", 9, 0, "WINOPS_MENU"},
	{"Show Named Menu", 9, 1, NULL},

	{"Goto Linear Area", 70, 2, NULL},
	{"Previous Linear Area", 71, 0, "-1"},
	{"Next Linear Area", 71, 0, "1"},
	{NULL, 0, 0, NULL}
};


void e_cb_modifier(GtkWidget * widget, gpointer data) {

	return;

}

static gchar *wait_for_ipc_msg(void)
{
	gtk_main();
	return e_ipc_msg;
}

char               *
atword(char *s, int num)
{
	int                 cnt, i;

	if (!s)
		return NULL;
	cnt = 0;
	i = 0;

	while (s[i])
	{
		if ((s[i] != ' ') && (s[i] != '\t'))
		{
			if (i == 0)
				cnt++;
			else if ((s[i - 1] == ' ') || (s[i - 1] == '\t'))
				cnt++;
			if (cnt == num)
				return &s[i];
		}
		i++;
	}
	return NULL;
}

void
selection_made(GtkWidget *clist, gint row, gint column, GdkEventButton *event,
		gpointer data)
{

	gchar *modstring;
	gchar *keyused;
	gchar *actperform;
	gchar *paramsused;
	int i;

	if (data) {
		event = NULL;
		clist = NULL;
		column = 0;
	}

	dont_update = 1;
	gtk_clist_get_text(GTK_CLIST(clist), row, 0, &modstring);
	gtk_option_menu_set_history(GTK_OPTION_MENU(act_mod),0);
	for(i=1;i<20;i++) {
		if(!strcmp(mod_str[i],modstring)) {
			gtk_option_menu_set_history(GTK_OPTION_MENU(act_mod),i);
		}
	}
	gtk_clist_get_text(GTK_CLIST(clist), row, 1, &keyused);
	gtk_entry_set_text(GTK_ENTRY(act_key),keyused);
	gtk_clist_get_text(GTK_CLIST(clist), row, 2, &actperform);
	for (i = 0; (actions[i].text); i++) {
		if(!strcmp(actperform,actions[i].text)) {
			gtk_clist_select_row(GTK_CLIST(act_clist),i,0);
		}
	}
	gtk_clist_get_text(GTK_CLIST(clist), row, 3, &paramsused);
	gtk_entry_set_text(GTK_ENTRY(act_params),paramsused);

	printf("%s\n%s\n%s\n%s\n",modstring,keyused,actperform,paramsused);

	dont_update=0;


	return;
}

static gchar *get_line(gchar * str, int num);

static gchar *get_line(gchar * str, int num)
{
	gchar *s1, *s2, *s;
	gint i, count, l;

	i = 0;
	count = 0;
	s1 = str;
	if (*str == '\n')
		i = 1;
	s2 = NULL;
	for (i = 0;; i++) {
		if ((str[i] == '\n') || (str[i] == 0)) {
			s2 = &(str[i]);
			if ((count == num) && (s2 > s1)) {
				l = s2 - s1;
				s = g_malloc(l + 1);
				strncpy(s, s1, l);
				s[l] = 0;
				return s;
			}
			count++;
			if (str[i] == 0)
				return NULL;
			s1 = s2 + 1;
		}
	}
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
	GtkWidget *frames;
	GtkWidget *alignment;
	GtkWidget *frame_vbox;
	GtkWidget *table;
	GtkWidget *label;
	GtkWidget *entry;
	GtkWidget *button;
	GtkWidget *hbox;
	GtkWidget *m,*mi,*om;


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
			GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_paned_pack1(GTK_PANED(panes), scrollybit, TRUE, FALSE);

	clist = gtk_clist_new(4);
	gtk_widget_show(clist);
	gtk_container_add(GTK_CONTAINER(scrollybit), clist);

	gtk_clist_set_column_title(GTK_CLIST(clist), 0, "Modifier");
	gtk_clist_set_column_title(GTK_CLIST(clist), 1, "Key Used");
	gtk_clist_set_column_title(GTK_CLIST(clist), 2, "Action to Perform");
	gtk_clist_set_column_title(GTK_CLIST(clist), 3, "Parameters Used");
	gtk_clist_column_titles_show(GTK_CLIST(clist));
	gtk_signal_connect(GTK_OBJECT(clist), "select_row",
			GTK_SIGNAL_FUNC(selection_made), NULL);
	gtk_signal_connect(GTK_OBJECT(clist), "click_column",
			GTK_SIGNAL_FUNC(on_resort_columns), NULL);

	{
		gchar *msg;
		gint i, j, k;
		gchar *buf;
		gchar cmd[4096];

		CommsSend("get_keybindings");
		msg = wait_for_ipc_msg();
		i = 0;
		while ((buf = get_line(msg, i++))) {
			/* stuff[0] = modifier */
			/* stuff[1] = key */
			/* stuff[2] = action */
			/* stuff[3] = params */

			char *stuff[4];

			stuff[0] = malloc(1024);
			stuff[1] = malloc(1024);
			stuff[2] = malloc(1024);
			stuff[3] = malloc(1024);
			if (strlen(buf) < 1)
				break;
			sscanf(buf, "%1000s", cmd);
			sprintf(stuff[1],"%s",cmd);
			sscanf(buf, "%*s %i", &j);
			sprintf(stuff[0],"%s",mod_str[j]);
			sscanf(buf, "%*s %*s %i", &j);
			strcpy(stuff[2],"");
			/*sprintf(stuff[2],"%s",actions[j].text); */
			if (atword(buf, 4))
				sprintf(stuff[3],"%s",atword(buf, 4));
			else
				strcpy(stuff[3],"");
			for (k = 0; (actions[k].text); k++) {
				if (j == actions[k].id) {
					if (strcmp(stuff[3],"")) {
						if ((actions[k].param_tpe == 0)&&(actions[k].params)) {
							if (!strcmp(stuff[3], actions[k].params)) {
								sprintf(stuff[2],"%s",actions[k].text);
							}
						} else {
							sprintf(stuff[2],"%s",actions[k].text);
						}
					} else if (!actions[k].params) {
						sprintf(stuff[2],"%s",actions[k].text);
					}
				}
			}
			if(strcmp(stuff[2],""))
				gtk_clist_append(GTK_CLIST(clist), stuff);
			free(stuff[0]);
			free(stuff[1]);
			free(stuff[2]);
			free(stuff[3]);
			g_free(buf);
		}
		g_free(msg);

	}

	vbox = gtk_vbox_new(FALSE,0);
	gtk_widget_show(vbox);
	frames = gtk_frame_new("Edit Keybinding Properties");
	gtk_container_set_border_width(GTK_CONTAINER(frames),2);
	gtk_widget_show(frames);
	gtk_paned_pack2(GTK_PANED(panes),vbox,FALSE,TRUE);
	gtk_box_pack_start(GTK_BOX(vbox), frames, TRUE, TRUE, 0);


	frame_vbox = gtk_vbox_new(FALSE,3);
	gtk_widget_show(frame_vbox);

	gtk_container_set_border_width(GTK_CONTAINER(frame_vbox),4);
	gtk_container_add(GTK_CONTAINER(frames), frame_vbox);

	table = gtk_table_new(3, 3, FALSE);
	gtk_widget_show(table);
	gtk_table_set_row_spacings(GTK_TABLE(table),3);
	gtk_table_set_col_spacings(GTK_TABLE(table),3);
	gtk_box_pack_start(GTK_BOX(frame_vbox), table, FALSE, FALSE, 2);

	alignment = gtk_alignment_new(1.0,0.5,0,0);
	label = gtk_label_new("Key:");
	gtk_container_add(GTK_CONTAINER(alignment),label);
	gtk_widget_show(alignment);
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_RIGHT);
	gtk_widget_show(label);
	gtk_table_attach(GTK_TABLE(table), alignment, 0, 1, 0, 1,
			GTK_FILL, (GtkAttachOptions) (0), 0, 0);

	alignment = gtk_alignment_new(1.0,0.5,0,0);
	label = gtk_label_new("Modifier:");
	gtk_container_add(GTK_CONTAINER(alignment),label);
	gtk_widget_show(alignment);
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_RIGHT);
	gtk_widget_show(label);
	gtk_table_attach(GTK_TABLE(table), alignment, 0, 1, 1, 2,
			GTK_FILL, (GtkAttachOptions) (0), 0, 0);

	alignment = gtk_alignment_new(1.0,0.5,0,0);
	label = gtk_label_new("Parameters:");
	gtk_container_add(GTK_CONTAINER(alignment),label);
	gtk_widget_show(alignment);
	gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_RIGHT);
	gtk_widget_show(label);
	gtk_table_attach(GTK_TABLE(table), alignment, 0, 1, 2, 3,
			GTK_FILL, (GtkAttachOptions) (0), 0, 0);

	act_key = entry = gtk_entry_new_with_max_length(4096);
	gtk_widget_show(entry);
	gtk_widget_set_sensitive(entry, FALSE);
	/* gtk_widget_set_usize(entry, 24, -1); */
	gtk_table_attach(GTK_TABLE(table), entry, 1, 2, 0, 1,
			GTK_EXPAND | GTK_FILL, (GtkAttachOptions) (0), 0, 0);

	button = gtk_button_new_with_label("Change");
	gtk_widget_show(button);
	gtk_table_attach(GTK_TABLE(table), button, 2, 3, 0, 1,
			GTK_EXPAND | GTK_FILL, (GtkAttachOptions) (0), 0, 0);



	m = gtk_menu_new();
	gtk_widget_show(m);

	mi = gtk_menu_item_new_with_label("NONE");
	gtk_widget_show(mi);
	gtk_signal_connect(GTK_OBJECT(mi), "activate",
			GTK_SIGNAL_FUNC(e_cb_modifier), (gpointer) 0);
	gtk_menu_append(GTK_MENU(m), mi);
	mi = gtk_menu_item_new_with_label("CTRL");
	gtk_widget_show(mi);
	gtk_signal_connect(GTK_OBJECT(mi), "activate",
			GTK_SIGNAL_FUNC(e_cb_modifier), (gpointer) 1);
	gtk_menu_append(GTK_MENU(m), mi);
	mi = gtk_menu_item_new_with_label("ALT");
	gtk_widget_show(mi);
	gtk_signal_connect(GTK_OBJECT(mi), "activate",
			GTK_SIGNAL_FUNC(e_cb_modifier), (gpointer) 2);
	gtk_menu_append(GTK_MENU(m), mi);
	mi = gtk_menu_item_new_with_label("SHIFT");
	gtk_widget_show(mi);
	gtk_signal_connect(GTK_OBJECT(mi), "activate",
			GTK_SIGNAL_FUNC(e_cb_modifier), (gpointer) 3);
	gtk_menu_append(GTK_MENU(m), mi);
	mi = gtk_menu_item_new_with_label("CTRL & ALT");
	gtk_widget_show(mi);
	gtk_signal_connect(GTK_OBJECT(mi), "activate",
			GTK_SIGNAL_FUNC(e_cb_modifier), (gpointer) 4);
	gtk_menu_append(GTK_MENU(m), mi);
	mi = gtk_menu_item_new_with_label("CTRL & SHIFT");
	gtk_widget_show(mi);
	gtk_signal_connect(GTK_OBJECT(mi), "activate",
			GTK_SIGNAL_FUNC(e_cb_modifier), (gpointer) 5);
	gtk_menu_append(GTK_MENU(m), mi);
	mi = gtk_menu_item_new_with_label("ALT & SHIFT");
	gtk_widget_show(mi);
	gtk_signal_connect(GTK_OBJECT(mi), "activate",
			GTK_SIGNAL_FUNC(e_cb_modifier), (gpointer) 6);
	gtk_menu_append(GTK_MENU(m), mi);
	mi = gtk_menu_item_new_with_label("CTRL & ALT & SHIFT");
	gtk_widget_show(mi);
	gtk_signal_connect(GTK_OBJECT(mi), "activate",
			GTK_SIGNAL_FUNC(e_cb_modifier), (gpointer) 7);
	gtk_menu_append(GTK_MENU(m), mi);
	mi = gtk_menu_item_new_with_label("WIN");
	gtk_widget_show(mi);
	gtk_signal_connect(GTK_OBJECT(mi), "activate",
			GTK_SIGNAL_FUNC(e_cb_modifier), (gpointer) 8);
	gtk_menu_append(GTK_MENU(m), mi);
	mi = gtk_menu_item_new_with_label("MOD3");
	gtk_widget_show(mi);
	gtk_signal_connect(GTK_OBJECT(mi), "activate",
			GTK_SIGNAL_FUNC(e_cb_modifier), (gpointer) 9);
	gtk_menu_append(GTK_MENU(m), mi);
	mi = gtk_menu_item_new_with_label("MOD4");
	gtk_widget_show(mi);
	gtk_signal_connect(GTK_OBJECT(mi), "activate",
			GTK_SIGNAL_FUNC(e_cb_modifier), (gpointer) 10);
	gtk_menu_append(GTK_MENU(m), mi);
	mi = gtk_menu_item_new_with_label("MOD5");
	gtk_widget_show(mi);
	gtk_signal_connect(GTK_OBJECT(mi), "activate",
			GTK_SIGNAL_FUNC(e_cb_modifier), (gpointer) 11);
	gtk_menu_append(GTK_MENU(m), mi);
	mi = gtk_menu_item_new_with_label("WIN & SHIFT");
	gtk_widget_show(mi);
	gtk_signal_connect(GTK_OBJECT(mi), "activate",
			GTK_SIGNAL_FUNC(e_cb_modifier), (gpointer) 12);
	gtk_menu_append(GTK_MENU(m), mi);
	mi = gtk_menu_item_new_with_label("WIN & CTRL");
	gtk_widget_show(mi);
	gtk_signal_connect(GTK_OBJECT(mi), "activate",
			GTK_SIGNAL_FUNC(e_cb_modifier), (gpointer) 13);
	gtk_menu_append(GTK_MENU(m), mi);
	mi = gtk_menu_item_new_with_label("WIN & ALT");
	gtk_widget_show(mi);
	gtk_signal_connect(GTK_OBJECT(mi), "activate",
			GTK_SIGNAL_FUNC(e_cb_modifier), (gpointer) 14);
	gtk_menu_append(GTK_MENU(m), mi);
	mi = gtk_menu_item_new_with_label("MOD4 & SHIFT");
	gtk_widget_show(mi);
	gtk_signal_connect(GTK_OBJECT(mi), "activate",
			GTK_SIGNAL_FUNC(e_cb_modifier), (gpointer) 15);
	gtk_menu_append(GTK_MENU(m), mi);
	mi = gtk_menu_item_new_with_label("MOD4 & CTRL");
	gtk_widget_show(mi);
	gtk_signal_connect(GTK_OBJECT(mi), "activate",
			GTK_SIGNAL_FUNC(e_cb_modifier), (gpointer) 16);
	gtk_menu_append(GTK_MENU(m), mi);
	mi = gtk_menu_item_new_with_label("MOD4 & CTRL & SHIFT");
	gtk_widget_show(mi);
	gtk_signal_connect(GTK_OBJECT(mi), "activate",
			GTK_SIGNAL_FUNC(e_cb_modifier), (gpointer) 17);
	gtk_menu_append(GTK_MENU(m), mi);
	mi = gtk_menu_item_new_with_label("MOD5 & SHIFT");
	gtk_widget_show(mi);
	gtk_signal_connect(GTK_OBJECT(mi), "activate",
			GTK_SIGNAL_FUNC(e_cb_modifier), (gpointer) 18);
	gtk_menu_append(GTK_MENU(m), mi);
	mi = gtk_menu_item_new_with_label("MOD5 & CTRL");
	gtk_widget_show(mi);
	gtk_signal_connect(GTK_OBJECT(mi), "activate",
			GTK_SIGNAL_FUNC(e_cb_modifier), (gpointer) 19);
	gtk_menu_append(GTK_MENU(m), mi);

	act_mod = om = gtk_option_menu_new();
	gtk_widget_show(om);
	gtk_option_menu_set_menu(GTK_OPTION_MENU(om), m);
	gtk_option_menu_set_history(GTK_OPTION_MENU(om), 0);
	gtk_table_attach(GTK_TABLE(table),om, 1, 3, 1, 2,
			GTK_EXPAND | GTK_FILL, (GtkAttachOptions) (0), 0, 0);

	act_params = entry = gtk_entry_new_with_max_length(4096);
	gtk_widget_show(entry);
	gtk_widget_set_sensitive(entry,FALSE);
	gtk_table_attach(GTK_TABLE(table),entry, 1, 3, 2, 3,
			GTK_EXPAND | GTK_FILL, (GtkAttachOptions) (0), 0, 0);


	scrollybit = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_show(scrollybit);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollybit),
			GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

	act_clist = gtk_clist_new(1);
	gtk_widget_show(act_clist);
	gtk_box_pack_start(GTK_BOX(frame_vbox), scrollybit, TRUE, TRUE, 0);
	gtk_clist_set_column_title(GTK_CLIST(act_clist), 0, "Action Used:");
	gtk_clist_column_titles_show(GTK_CLIST(act_clist));
	gtk_container_add(GTK_CONTAINER(scrollybit), act_clist);

	{
		char *stuff[1];
		int k;
		for(k=0; (actions[k].text); k++) {
			stuff[0] = malloc(1024);
			strcpy(stuff[0],actions[k].text);
			gtk_clist_append(GTK_CLIST(act_clist),stuff);
			free(stuff[0]);
		}
	}


	hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);

	button = gtk_button_new_with_label(" New Keybinding ");
	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, FALSE, 5);

	button = gtk_button_new_with_label(" Delete Current Row ");
	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, FALSE, 5);

	button = gtk_button_new_with_label(" Save ");
	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, FALSE, 5);

	button = gtk_button_new_with_label(" Quit ");
	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, FALSE, 5);

	gtk_clist_select_row(GTK_CLIST(clist),0,0);

	return list_window;

}

static void
receive_ipc_msg(gchar * msg)
{
	gdk_flush();
	e_ipc_msg = g_strdup(msg);

	gtk_main_quit();

}

int
main(int argc, char *argv[])
{
	GtkWidget *lister;

	gtk_set_locale();
	gtk_init(&argc, &argv);

	tooltips = gtk_tooltips_new();
	accel_group = gtk_accel_group_new();

	if(!CommsInit(receive_ipc_msg)) {
		GtkWidget *win, *label, *align, *frame, *button, *vbox;

		win = gtk_window_new(GTK_WINDOW_POPUP);
		gtk_window_set_policy(GTK_WINDOW(win), 0, 0, 1);
		gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
		frame = gtk_frame_new(NULL);
		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
		align = gtk_alignment_new(0.0, 0.0, 0.0, 0.0);
		gtk_container_set_border_width(GTK_CONTAINER(align), 32);
		vbox = gtk_vbox_new(FALSE, 5);
		button = gtk_button_new_with_label("Quit");
		gtk_signal_connect(GTK_OBJECT(button), "clicked",
				GTK_SIGNAL_FUNC(on_exit_application), NULL);
		label = gtk_label_new("You are not running Enlightenment\n"
					"\n"
					"This window manager has to be running in order\n"
					"to configure it.\n" "\n");
		gtk_container_add(GTK_CONTAINER(win), frame);
		gtk_container_add(GTK_CONTAINER(frame), align);
		gtk_container_add(GTK_CONTAINER(align), vbox);
		gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
		gtk_widget_show_all(win);
		gtk_main();
		exit(1);
	}
	CommsSend("set clientname Enlightenment Configuration Utility");
	CommsSend("set version 0.1.0");
	CommsSend("set author Mandrake (Geoff Harrison)");
	CommsSend("set email mandrake@mandrake.net");
	CommsSend("set web http://mandrake.net/");
	CommsSend("set address C/O VA Linux Systems, USA");
	CommsSend("set info "
			"This is the Enlightenemnt KeyBindings Configuration Utility\n"
			"that uses Enlightenment's IPC mechanism to configure\n"
			"it remotely.");

	lister = create_list_window();

	gtk_clist_set_column_auto_resize(GTK_CLIST(clist), 0, TRUE);
	gtk_clist_set_column_auto_resize(GTK_CLIST(clist), 1, TRUE);

	gtk_widget_show(lister);
	gtk_signal_connect(GTK_OBJECT(lister), "destroy",
			GTK_SIGNAL_FUNC(on_exit_application), NULL);
	gtk_signal_connect(GTK_OBJECT(lister), "delete_event",
			GTK_SIGNAL_FUNC(on_exit_application), NULL);

	gtk_main();

	return 0;
}
