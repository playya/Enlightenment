/**********************************************************************
* lvs-gui.c
* Copyright (C) 1999 Carsten Haitzler and Simon Horman
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
**********************************************************************/

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

#include "options.h"
#include "config_file.h"

#include "va.xpm"
const char *readme_txt =
#include "readme.h"
;

#define CONNECT(ob, sig, func, dat) \
gtk_signal_connect(GTK_OBJECT(ob), sig, GTK_SIGNAL_FUNC(func), dat);

#define METHOD_WLC 0
#define METHOD_WRR 1
#define METHOD_RR  2
#define METHOD_LC  3

#define PROTO_TCP          0
#define PROTO_TCP_PERSIST  1
#define PROTO_UDP          2

typedef struct _tcp TCP;
typedef struct _tcp2 TCP2;

struct _tcp
{
   gchar *address;
   gchar *port;
   GList *servers;
   gint   method;
   gint   protocol;
};

struct _tcp2
{
   gchar *address;
   gchar *port;
   gint   weight;
};

void del_tcp(TCP *t);
void del_tcp2(TCP *t, TCP2 *t2);
TCP *add_tcp(char *addr, char *port, int method, int protocol);
void add_tcp2(TCP *t, char *addr, char *port, int weight);
void fill_tcp_list2(TCP *t);
void update_tcp(TCP *t);
void update_tcp2(TCP *t, TCP2 *t2);
void load_config(char *file);
void save_transparent_proxy(char *host, GList *hosts);
void save_config(char *file);
void start_lvs(char *machine);
/* void stop_lvs(char *machine); */
void start_transparent_proxy(char *machine);
/*
 void stop_transparent_proxy(char *machine);
 */
void remote_cp(char *machine1, char *file1, char *machine2, char *file2);
void gui_save_config(void);

/* globals */
GtkWidget *tcp_list,   *tcp_list2,  *tcp_frame,  *tcp_addr,   *tcp_port, 
 *tcp_radio1, *tcp_radio2, *tcp_radio3, *tcp_radio4, *tcp_addr2,
 *tcp_port2,  *tcp_weight, *tcp_radio5, *tcp_radio6, *tcp_radio7,
 *tcp_add,    *tcp_del,    *tcp_add2,   *tcp_del2,   *tcp_ok,
 *tcp_apply,  *tcp_cancel, *tcp_frame2, *tcp_frame3, *cfg_reload,
 *cfg_config;

GList     *tcps         = NULL;
TCP       *current_tcp  = NULL;
TCP2      *current_tcp2 = NULL;

extern options_t opt;
   

/* callbacks */
void
cb_close(GtkWidget * w, void *data)
{
   gtk_widget_destroy(GTK_WIDGET(w));
}

void 
cb_addr_change(GtkEntry *entry, gpointer user_data)
{
   if (current_tcp)
     {
	if (current_tcp->address)
	   g_free(current_tcp->address);
	current_tcp->address = g_strdup(gtk_entry_get_text(entry));
	update_tcp(current_tcp);
     }
}

void 
cb_port_change(GtkEntry *entry, gpointer user_data)
{
   if (current_tcp)
     {
	if (current_tcp->port)
	   g_free(current_tcp->port);
	current_tcp->port = g_strdup(gtk_entry_get_text(entry));
	update_tcp(current_tcp);
     }
}

void 
cb_addr2_change(GtkEntry *entry, gpointer user_data)
{
   if (current_tcp2)
     {
	if (current_tcp2->address)
	   g_free(current_tcp2->address);
	current_tcp2->address = g_strdup(gtk_entry_get_text(entry));
	if (current_tcp)
	   update_tcp2(current_tcp, current_tcp2);
     }
}

void 
cb_port2_change(GtkEntry *entry, gpointer user_data)
{
   if (current_tcp2)
     {
	if (current_tcp2->port)
	   g_free(current_tcp2->port);
	current_tcp2->port = g_strdup(gtk_entry_get_text(entry));
	if (current_tcp)
	   update_tcp2(current_tcp, current_tcp2);
     }
}

void 
cb_weight_change(GtkEntry *entry, gpointer user_data)
{
   if (current_tcp2)
     {
	current_tcp2->weight = atoi(gtk_entry_get_text(entry));
	if (current_tcp)
	   update_tcp2(current_tcp, current_tcp2);
     }
}

void
cb_radio1_toggle(GtkRadioButton *radiobutton, gpointer user_data)
{
   if (current_tcp)
     {
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton)))
	   current_tcp->method = METHOD_WLC;
	update_tcp(current_tcp);
     }
}

void
cb_radio2_toggle(GtkRadioButton *radiobutton, gpointer user_data)
{
   if (current_tcp)
     {
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton)))
	   current_tcp->method = METHOD_WRR;
	update_tcp(current_tcp);
     }
}

void
cb_radio3_toggle(GtkRadioButton *radiobutton, gpointer user_data)
{
   if (current_tcp)
     {
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton)))
	   current_tcp->method = METHOD_RR;
	update_tcp(current_tcp);
     }
}

void
cb_radio4_toggle(GtkRadioButton *radiobutton, gpointer user_data)
{
   if (current_tcp)
     {
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton)))
	   current_tcp->method = METHOD_LC;
	update_tcp(current_tcp);
     }
}

void
cb_radio5_toggle(GtkRadioButton *radiobutton, gpointer user_data)
{
   if (current_tcp)
     {
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton)))
	   current_tcp->protocol = PROTO_TCP;
	update_tcp(current_tcp);
     }
}

void
cb_radio6_toggle(GtkRadioButton *radiobutton, gpointer user_data)
{
   if (current_tcp)
     {
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton)))
	   current_tcp->protocol = PROTO_TCP_PERSIST;
	update_tcp(current_tcp);
     }
}

void
cb_radio7_toggle(GtkRadioButton *radiobutton, gpointer user_data)
{
   if (current_tcp)
     {
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radiobutton)))
	   current_tcp->protocol = PROTO_UDP;
	update_tcp(current_tcp);
     }
}

void 
cb_tcp_sel(GtkCList *clist, gint row, gint column, GdkEventButton *event,
	   gpointer user_data)
{
   GList *l;
   gint   i;
   
   for (l = tcps, i = 0; l; l = l->next, i++)
     {
	if (i == row)
	  {
	     current_tcp = (TCP *)l->data;
	     gtk_entry_set_text(GTK_ENTRY(tcp_addr), current_tcp->address);
	     gtk_entry_set_text(GTK_ENTRY(tcp_port), current_tcp->port);
	     if (current_tcp->method == METHOD_WLC)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tcp_radio1), TRUE);
	     else if (current_tcp->method == METHOD_WRR)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tcp_radio2), TRUE);
	     else if (current_tcp->method == METHOD_RR)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tcp_radio3), TRUE);
	     else if (current_tcp->method == METHOD_LC)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tcp_radio4), TRUE);
	     if (current_tcp->protocol == PROTO_TCP)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tcp_radio5), TRUE);
	     else if (current_tcp->protocol == PROTO_TCP_PERSIST)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tcp_radio6), TRUE);
	     else if (current_tcp->protocol == PROTO_UDP)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tcp_radio7), TRUE);
	     gtk_widget_set_sensitive(tcp_frame, 1);
	     gtk_widget_set_sensitive(tcp_frame2, 1);
	  }
     }
   fill_tcp_list2(current_tcp);
}

void 
cb_tcp_unsel(GtkCList *clist, gint row, gint column, GdkEventButton *event,
	     gpointer user_data)
{
   current_tcp = NULL;
   current_tcp2 = NULL;
   gtk_widget_set_sensitive(tcp_frame, 0);
   gtk_widget_set_sensitive(tcp_frame2, 0);
   gtk_widget_set_sensitive(tcp_frame3, 0);
   gtk_entry_set_text(GTK_ENTRY(tcp_addr), "");   
   gtk_entry_set_text(GTK_ENTRY(tcp_port), "");   
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tcp_radio1), TRUE);
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tcp_radio5), TRUE);
   gtk_entry_set_text(GTK_ENTRY(tcp_addr2), "");   
   gtk_entry_set_text(GTK_ENTRY(tcp_port2), "");   
   gtk_entry_set_text(GTK_ENTRY(tcp_weight), "");   
   fill_tcp_list2(current_tcp);
}

void 
cb_tcp2_sel(GtkCList *clist, gint row, gint column, GdkEventButton *event,
	    gpointer user_data)
{
   GList *l;
   int i;
   
   if (current_tcp)
     {
	for (l = current_tcp->servers, i = 0; l; l = l->next, i++)
	  {
	     if (i == row)
	       {
		  gchar s[32];
		  
		  current_tcp2 = (TCP2 *)l->data;		  
		  gtk_entry_set_text(GTK_ENTRY(tcp_addr2), current_tcp2->address);
		  gtk_entry_set_text(GTK_ENTRY(tcp_port2), current_tcp2->port);
		  g_snprintf(s, sizeof(s), "%i", current_tcp2->weight);
		  gtk_entry_set_text(GTK_ENTRY(tcp_weight), s);
		  gtk_widget_set_sensitive(tcp_frame3, 1);
	       }
	  }
     }
}

void 
cb_tcp2_unsel(GtkCList *clist, gint row, gint column, GdkEventButton *event,
	      gpointer user_data)
{
   current_tcp2 = NULL;
   gtk_entry_set_text(GTK_ENTRY(tcp_addr2), "");   
   gtk_entry_set_text(GTK_ENTRY(tcp_port2), "");   
   gtk_entry_set_text(GTK_ENTRY(tcp_weight), "");   
   gtk_widget_set_sensitive(tcp_frame3, 0);
}

void
cb_add(GtkButton *button, gpointer user_data)
{
   add_tcp("127.0.0.1", "80", METHOD_WLC, PROTO_TCP);
}

void
cb_del(GtkButton *button, gpointer user_data)
{
   if (current_tcp)
      del_tcp(current_tcp);
}

void
cb_add2(GtkButton *button, gpointer user_data)
{
   if (current_tcp)
      add_tcp2(current_tcp, "127.0.0.2", "80", 1);
}

void
cb_del2(GtkButton *button, gpointer user_data)
{
   if ((current_tcp) && (current_tcp2))
      del_tcp2(current_tcp, current_tcp2);
}

void
cb_ok(GtkButton *button, gpointer user_data)
{
   gui_save_config();
   save_config("ipvs");
   exit(0);
}

void
cb_apply(GtkButton *button, gpointer user_data)
{
   gui_save_config();
   save_config("ipvs");
}

void
cb_cancel(GtkButton *button, gpointer user_data)
{
   gui_save_config();
   exit(0);
}

void
cb_reload(GtkButton *button, gpointer user_data)
{
   gui_save_config();
   remote_cp(opt.master_host, opt.ipvs_config_file, NULL, "ipvs");
   load_config("ipvs");
}

void
cb_config_change(GtkEntry *entry, gpointer user_data)
{
   char **txt;
   
   txt = user_data;
   if (*txt)
      g_free(*txt);
   *txt = NULL;
   *txt = g_strdup(gtk_entry_get_text(entry));
}

void
cb_config_close(GtkButton *button, gpointer user_data)
{
   gtk_widget_destroy(GTK_WIDGET(user_data));
}

void
cb_about(GtkButton *button, gpointer user_data)
{
   GtkWidget *win, *vbox, *sw, *pix, *label;
   GdkPixmap *pm, *mm;
   
   win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
   CONNECT(win, "delete_event", cb_close, win);
   gtk_window_set_wmclass(GTK_WINDOW(win), "LVS-Gui-About", "LVS");
   gtk_window_set_title(GTK_WINDOW(win), "LVS About");
   gtk_container_border_width(GTK_CONTAINER(win), 2);
   gtk_window_set_policy(GTK_WINDOW(win), 0, 0, 1);
   gtk_widget_set_usize(win, 512, 400);
   
   sw = gtk_scrolled_window_new(NULL, NULL);
   gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);
   gtk_container_add(GTK_CONTAINER(win), sw);
   
   vbox = gtk_vbox_new(FALSE, 2);

   gtk_widget_realize(win);
   pm = gdk_pixmap_create_from_xpm_d(win->window, &mm, NULL, va_xpm);
   pix = gtk_pixmap_new(pm, mm);
   gdk_pixmap_unref(pm);
   gdk_pixmap_unref(mm);
   gtk_box_pack_start(GTK_BOX(vbox), pix, FALSE, FALSE, 2);
   
   label = gtk_label_new(readme_txt);
   gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
   
   gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 2);

   gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(sw), vbox);
   
   gtk_widget_show_all(win);
}

void
cb_config(GtkButton *button, gpointer user_data)
{
   GtkWidget *win, *label, *entry, *table, *align, *hbox, *close;
   GtkWidget *e1, *e2, *e3, *e4, *e5, *e6, *e7, *e8;
   
   win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
   CONNECT(win, "delete_event", cb_close, win);
   gtk_window_set_wmclass(GTK_WINDOW(win), "LVS-Gui-Config", "LVS");
   gtk_window_set_title(GTK_WINDOW(win), "LVS Gui Configuration Options");
   gtk_container_border_width(GTK_CONTAINER(win), 2);
   gtk_window_set_policy(GTK_WINDOW(win), 0, 0, 1);
   
   table = gtk_table_new(2, 2, FALSE);
   gtk_container_add(GTK_CONTAINER(win), table);
   gtk_table_set_row_spacings(GTK_TABLE(table), 4);
   gtk_table_set_col_spacings(GTK_TABLE(table), 4);

   label = gtk_label_new("LVS configuration editor options");
   gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 2, 0, 1);

   align = gtk_alignment_new(0.0, 0.5, 0.0, 1.0);
   gtk_table_attach_defaults(GTK_TABLE(table), align, 0, 1, 1, 2);
   label = gtk_label_new("Master host");
   gtk_container_add(GTK_CONTAINER(align), label);
   e1 = entry = gtk_entry_new();
   gtk_table_attach_defaults(GTK_TABLE(table), entry, 1, 2, 1, 2);

   align = gtk_alignment_new(0.0, 0.5, 0.0, 1.0);
   gtk_table_attach_defaults(GTK_TABLE(table), align, 0, 1, 2, 3);
   label = gtk_label_new("Remote LVS config file");
   gtk_container_add(GTK_CONTAINER(align), label);
   e2 = entry = gtk_entry_new();
   gtk_table_attach_defaults(GTK_TABLE(table), entry, 1, 2, 2, 3);

   align = gtk_alignment_new(0.0, 0.5, 0.0, 1.0);
   gtk_table_attach_defaults(GTK_TABLE(table), align, 0, 1, 3, 4);
   label = gtk_label_new("Remote LVS init script");
   gtk_container_add(GTK_CONTAINER(align), label);
   e3 = entry = gtk_entry_new();
   gtk_table_attach_defaults(GTK_TABLE(table), entry, 1, 2, 3, 4);

   align = gtk_alignment_new(0.0, 0.5, 0.0, 1.0);
   gtk_table_attach_defaults(GTK_TABLE(table), align, 0, 1, 4, 5);
   label = gtk_label_new("Remote LVS transparent proxy config file");
   gtk_container_add(GTK_CONTAINER(align), label);
   e4 = entry = gtk_entry_new();
   gtk_table_attach_defaults(GTK_TABLE(table), entry, 1, 2, 4, 5);

   align = gtk_alignment_new(0.0, 0.5, 0.0, 1.0);
   gtk_table_attach_defaults(GTK_TABLE(table), align, 0, 1, 5, 6);
   label = gtk_label_new("Remote LVS transparent proxy init script");
   gtk_container_add(GTK_CONTAINER(align), label);
   e5 = entry = gtk_entry_new();
   gtk_table_attach_defaults(GTK_TABLE(table), entry, 1, 2, 5, 6);

   align = gtk_alignment_new(0.0, 0.5, 0.0, 1.0);
   gtk_table_attach_defaults(GTK_TABLE(table), align, 0, 1, 6, 7);
   label = gtk_label_new("Remote execution command");
   gtk_container_add(GTK_CONTAINER(align), label);
   e6 = entry = gtk_entry_new();
   gtk_table_attach_defaults(GTK_TABLE(table), entry, 1, 2, 6, 7);

   align = gtk_alignment_new(0.0, 0.5, 0.0, 1.0);
   gtk_table_attach_defaults(GTK_TABLE(table), align, 0, 1, 7, 8);
   label = gtk_label_new("Remote copy command");
   gtk_container_add(GTK_CONTAINER(align), label);
   e7 = entry = gtk_entry_new();
   gtk_table_attach_defaults(GTK_TABLE(table), entry, 1, 2, 7, 8);

   align = gtk_alignment_new(0.0, 0.5, 0.0, 1.0);
   gtk_table_attach_defaults(GTK_TABLE(table), align, 0, 1, 8, 9);
   label = gtk_label_new("Remote LVS user");
   gtk_container_add(GTK_CONTAINER(align), label);
   e8 = entry = gtk_entry_new();
   gtk_table_attach_defaults(GTK_TABLE(table), entry, 1, 2, 8, 9);

   align = gtk_alignment_new(1.0, 0.5, 0.0, 0.0);
   gtk_table_attach_defaults(GTK_TABLE(table), align, 0, 2, 9, 10);
   
   hbox = gtk_hbox_new(TRUE, 2);
   gtk_container_add(GTK_CONTAINER(align), hbox);
   
   close = gtk_button_new_with_label("Close");
   gtk_box_pack_start(GTK_BOX(hbox), close, TRUE, TRUE, 2);   
   
   gtk_entry_set_text(GTK_ENTRY(e1), opt.master_host);
   gtk_entry_set_text(GTK_ENTRY(e2), opt.ipvs_config_file);
   gtk_entry_set_text(GTK_ENTRY(e3), opt.ipvs_init_script);
   gtk_entry_set_text(GTK_ENTRY(e4), opt.transparent_proxy_config_file);
   gtk_entry_set_text(GTK_ENTRY(e5), opt.transparent_proxy_init_script);
   gtk_entry_set_text(GTK_ENTRY(e6), opt.rsh_command);
   gtk_entry_set_text(GTK_ENTRY(e7), opt.rcp_command);
   gtk_entry_set_text(GTK_ENTRY(e8), opt.user);
   
   CONNECT(close, "clicked", cb_config_close, win);
   CONNECT(e1, "changed", cb_config_change, &(opt.master_host));
   CONNECT(e2, "changed", cb_config_change, &(opt.ipvs_config_file));
   CONNECT(e3, "changed", cb_config_change, &(opt.ipvs_init_script));
   CONNECT(e4, "changed", cb_config_change, &(opt.transparent_proxy_config_file));
   CONNECT(e5, "changed", cb_config_change, &(opt.transparent_proxy_init_script));
   CONNECT(e6, "changed", cb_config_change, &(opt.rsh_command));
   CONNECT(e7, "changed", cb_config_change, &(opt.rcp_command));
   CONNECT(e8, "changed", cb_config_change, &(opt.user));
   gtk_widget_show_all(win);
}

/* gui setup */
void
gui_tcp(GtkWidget *win)
{
   GtkWidget *hbox, *hbox2, *vbox, *vport, *list, *frame, *align, *button;
   GtkWidget *hbox3, *entry, *radio, *vbox2;
   gchar *titles1[] = {"External address", "Port", "Scheduling Method", "Protocol"};
   gchar *titles2[] = {"Internal address", "Port", "Weight"};
   
   hbox = gtk_vbox_new(FALSE, 2);
   gtk_container_add(GTK_CONTAINER(win), hbox);
   
   frame = gtk_frame_new("Externally advertised services");
   gtk_container_border_width(GTK_CONTAINER(frame), 2);
   gtk_box_pack_start(GTK_BOX(hbox), frame, TRUE, TRUE, 2);
   
   hbox2 = gtk_hbox_new(FALSE, 2);
   gtk_container_border_width(GTK_CONTAINER(hbox2), 4);
   gtk_container_add(GTK_CONTAINER(frame), hbox2);
   
   align = gtk_alignment_new(0.0, 0.0, 0.0, 0.0);
   gtk_box_pack_start(GTK_BOX(hbox2), align, FALSE, FALSE, 0);
   vbox = gtk_vbox_new(FALSE, 0);
   gtk_container_add(GTK_CONTAINER(align), vbox);
   
   tcp_add = button = gtk_button_new_with_label("Add");
   gtk_box_pack_start(GTK_BOX(vbox), button, TRUE, TRUE, 0);
   
   tcp_del = button = gtk_button_new_with_label("Del");
   gtk_box_pack_start(GTK_BOX(vbox), button, TRUE, TRUE, 0);
   
   vbox = gtk_vbox_new(FALSE, 0);
   gtk_box_pack_start(GTK_BOX(hbox2), vbox, TRUE, TRUE, 0);
   
   vport = gtk_scrolled_window_new(NULL, NULL);
   gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(vport), 
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);
   gtk_widget_set_usize(vport, 400, 64);
   gtk_box_pack_start(GTK_BOX(vbox), vport, TRUE, TRUE, 0);
   
   tcp_frame2 = hbox3 = gtk_hbox_new(FALSE, 2);
   gtk_box_pack_start(GTK_BOX(vbox), hbox3, FALSE, FALSE, 2);
   
   align = gtk_alignment_new(0.0, 0.0, 1.0, 0.0);
   gtk_box_pack_start(GTK_BOX(hbox3), align, TRUE, TRUE, 2);
   frame = gtk_frame_new("External address");
   gtk_container_border_width(GTK_CONTAINER(frame), 2);
   gtk_container_add(GTK_CONTAINER(align), frame);
   tcp_addr = entry = gtk_entry_new();
   gtk_widget_set_usize(entry, 80, -1);
   align = gtk_alignment_new(0.0, 0.0, 1.0, 0.0);
   gtk_container_add(GTK_CONTAINER(align), entry);
   gtk_container_add(GTK_CONTAINER(frame), align);
   
   align = gtk_alignment_new(0.0, 0.0, 1.0, 0.0);
   gtk_box_pack_start(GTK_BOX(hbox3), align, FALSE, FALSE, 2);
   frame = gtk_frame_new("Port");
   gtk_container_border_width(GTK_CONTAINER(frame), 2);
   gtk_container_add(GTK_CONTAINER(align), frame);
   tcp_port = entry = gtk_entry_new();
   gtk_widget_set_usize(entry, 32, -1);
   align = gtk_alignment_new(0.0, 0.0, 1.0, 0.0);
   gtk_container_add(GTK_CONTAINER(align), entry);
   gtk_container_add(GTK_CONTAINER(frame), align);
   
   align = gtk_alignment_new(0.0, 0.0, 1.0, 1.0);
   gtk_box_pack_start(GTK_BOX(hbox3), align, FALSE, FALSE, 2);
   frame = gtk_frame_new("Connection scheduling method");
   gtk_container_border_width(GTK_CONTAINER(frame), 2);
   gtk_container_add(GTK_CONTAINER(align), frame);
   vbox2 = gtk_vbox_new(FALSE, 0);
   gtk_container_add(GTK_CONTAINER(frame), vbox2);
   
   tcp_radio1 = radio = gtk_radio_button_new_with_label_from_widget(NULL, "Weighted least connection");
   gtk_box_pack_start(GTK_BOX(vbox2), radio, FALSE, FALSE, 0);
   
   tcp_radio2 = radio = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio), "Weighted round robin");
   gtk_box_pack_start(GTK_BOX(vbox2), radio, FALSE, FALSE, 0);
   
   tcp_radio3 = radio = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio), "Round robin");
   gtk_box_pack_start(GTK_BOX(vbox2), radio, FALSE, FALSE, 0);
   
   tcp_radio4 = radio = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio), "Least connection");
   gtk_box_pack_start(GTK_BOX(vbox2), radio, FALSE, FALSE, 0);
   
   align = gtk_alignment_new(0.0, 0.0, 1.0, 1.0);
   gtk_box_pack_start(GTK_BOX(hbox3), align, FALSE, FALSE, 2);
   frame = gtk_frame_new("Protocol");
   gtk_container_border_width(GTK_CONTAINER(frame), 2);
   gtk_container_add(GTK_CONTAINER(align), frame);
   vbox2 = gtk_vbox_new(FALSE, 0);
   gtk_container_add(GTK_CONTAINER(frame), vbox2);
   
   tcp_radio5 = radio = gtk_radio_button_new_with_label_from_widget(NULL, "TCP");
   gtk_box_pack_start(GTK_BOX(vbox2), radio, FALSE, FALSE, 0);
   
   tcp_radio6 = radio = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio), "Persistant TCP");
   gtk_box_pack_start(GTK_BOX(vbox2), radio, FALSE, FALSE, 0);
   
   tcp_radio7 = radio = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio), "UDP");
   gtk_box_pack_start(GTK_BOX(vbox2), radio, FALSE, FALSE, 0);
   
   tcp_list = list = gtk_clist_new_with_titles(4, titles1);
   gtk_clist_set_column_resizeable(GTK_CLIST(list), 0, TRUE);
   gtk_clist_set_column_resizeable(GTK_CLIST(list), 1, FALSE);
   gtk_clist_set_column_resizeable(GTK_CLIST(list), 2, FALSE);
   gtk_clist_set_column_resizeable(GTK_CLIST(list), 3, FALSE);
   gtk_clist_set_column_auto_resize(GTK_CLIST(list), 0, TRUE);
   gtk_clist_set_column_auto_resize(GTK_CLIST(list), 1, TRUE);
   gtk_clist_set_column_auto_resize(GTK_CLIST(list), 2, TRUE);
   gtk_clist_set_column_auto_resize(GTK_CLIST(list), 3, TRUE);
   gtk_clist_set_selection_mode(GTK_CLIST(list), GTK_SELECTION_BROWSE);
   
   gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(vport), list);
   
   tcp_frame = frame = gtk_frame_new("Internal servers for selected external service");
   gtk_container_border_width(GTK_CONTAINER(frame), 2);
   gtk_box_pack_start(GTK_BOX(hbox), frame, TRUE, TRUE, 2);
   
   hbox2 = gtk_hbox_new(FALSE, 2);
   gtk_container_border_width(GTK_CONTAINER(hbox2), 4);
   gtk_container_add(GTK_CONTAINER(frame), hbox2);
   
   align = gtk_alignment_new(0.0, 0.0, 0.0, 0.0);
   gtk_box_pack_start(GTK_BOX(hbox2), align, FALSE, FALSE, 0);
   vbox = gtk_vbox_new(FALSE, 0);
   gtk_container_add(GTK_CONTAINER(align), vbox);
   
   tcp_add2 = button = gtk_button_new_with_label("Add");
   gtk_box_pack_start(GTK_BOX(vbox), button, TRUE, TRUE, 0);
   
   tcp_del2 = button = gtk_button_new_with_label("Del");
   gtk_box_pack_start(GTK_BOX(vbox), button, TRUE, TRUE, 0);
   
   vbox = gtk_vbox_new(FALSE, 0);
   gtk_box_pack_start(GTK_BOX(hbox2), vbox, TRUE, TRUE, 0);
   
   vport = gtk_scrolled_window_new(NULL, NULL);
   gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(vport), 
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);
   gtk_widget_set_usize(vport, 400, 64);
   gtk_box_pack_start(GTK_BOX(vbox), vport, TRUE, TRUE, 0);
   
   tcp_frame3 = hbox3 = gtk_hbox_new(FALSE, 2);
   gtk_box_pack_start(GTK_BOX(vbox), hbox3, FALSE, FALSE, 2);
   
   align = gtk_alignment_new(0.0, 0.0, 1.0, 0.0);
   gtk_box_pack_start(GTK_BOX(hbox3), align, TRUE, TRUE, 2);
   frame = gtk_frame_new("Internal address");
   gtk_container_border_width(GTK_CONTAINER(frame), 2);
   gtk_container_add(GTK_CONTAINER(align), frame);
   tcp_addr2 = entry = gtk_entry_new();
   gtk_widget_set_usize(entry, 80, -1);
   align = gtk_alignment_new(0.0, 0.0, 1.0, 0.0);
   gtk_container_add(GTK_CONTAINER(align), entry);
   gtk_container_add(GTK_CONTAINER(frame), align);
   
   align = gtk_alignment_new(0.0, 0.0, 1.0, 0.0);
   gtk_box_pack_start(GTK_BOX(hbox3), align, FALSE, FALSE, 2);
   frame = gtk_frame_new("Port");
   gtk_container_border_width(GTK_CONTAINER(frame), 2);
   gtk_container_add(GTK_CONTAINER(align), frame);
   tcp_port2 = entry = gtk_entry_new();
   gtk_widget_set_usize(entry, 32, -1);
   align = gtk_alignment_new(0.0, 0.0, 1.0, 0.0);
   gtk_container_add(GTK_CONTAINER(align), entry);
   gtk_container_add(GTK_CONTAINER(frame), align);
   
   align = gtk_alignment_new(0.0, 0.0, 1.0, 0.0);
   gtk_box_pack_start(GTK_BOX(hbox3), align, FALSE, FALSE, 2);
   frame = gtk_frame_new("Weight");
   gtk_container_border_width(GTK_CONTAINER(frame), 2);
   gtk_container_add(GTK_CONTAINER(align), frame);
   tcp_weight = entry = gtk_entry_new();
   gtk_widget_set_usize(entry, 32, -1);
   align = gtk_alignment_new(0.0, 0.0, 1.0, 0.0);
   gtk_container_add(GTK_CONTAINER(align), entry);
   gtk_container_add(GTK_CONTAINER(frame), align);
   
   tcp_list2 = list = gtk_clist_new_with_titles(3, titles2);
   gtk_clist_set_column_resizeable(GTK_CLIST(list), 0, TRUE);
   gtk_clist_set_column_resizeable(GTK_CLIST(list), 1, FALSE);
   gtk_clist_set_column_resizeable(GTK_CLIST(list), 2, FALSE);
   gtk_clist_set_column_auto_resize(GTK_CLIST(list), 0, TRUE);
   gtk_clist_set_column_auto_resize(GTK_CLIST(list), 1, TRUE);
   gtk_clist_set_column_auto_resize(GTK_CLIST(list), 2, TRUE);
   gtk_clist_set_selection_mode(GTK_CLIST(list), GTK_SELECTION_BROWSE);
   
   gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(vport), list);
   
   gtk_widget_set_sensitive(tcp_frame, 0);
   gtk_widget_set_sensitive(tcp_frame2, 0);
   gtk_widget_set_sensitive(tcp_frame3, 0);      
   
   CONNECT(tcp_addr,   "changed",      cb_addr_change,   NULL);
   CONNECT(tcp_port,   "changed",      cb_port_change,   NULL); 
   CONNECT(tcp_addr2,  "changed",      cb_addr2_change,  NULL);
   CONNECT(tcp_port2,  "changed",      cb_port2_change,  NULL);
   CONNECT(tcp_weight, "changed",      cb_weight_change, NULL);
   CONNECT(tcp_list,   "select-row",   cb_tcp_sel,       NULL);
   CONNECT(tcp_list,   "unselect-row", cb_tcp_unsel,     NULL);
   CONNECT(tcp_list2,  "select-row",   cb_tcp2_sel,      NULL);
   CONNECT(tcp_list2,  "unselect-row", cb_tcp2_unsel,    NULL);
   CONNECT(tcp_radio1, "toggled",      cb_radio1_toggle, NULL);
   CONNECT(tcp_radio2, "toggled",      cb_radio2_toggle, NULL);
   CONNECT(tcp_radio3, "toggled",      cb_radio3_toggle, NULL);
   CONNECT(tcp_radio4, "toggled",      cb_radio4_toggle, NULL);
   CONNECT(tcp_radio5, "toggled",      cb_radio5_toggle, NULL);
   CONNECT(tcp_radio6, "toggled",      cb_radio6_toggle, NULL);
   CONNECT(tcp_radio7, "toggled",      cb_radio7_toggle, NULL);
   CONNECT(tcp_add,    "clicked",      cb_add,           NULL);
   CONNECT(tcp_del,    "clicked",      cb_del,           NULL);
   CONNECT(tcp_add2,   "clicked",      cb_add2,          NULL);
   CONNECT(tcp_del2,   "clicked",      cb_del2,          NULL);
   
}

void
gui(void)
{
   GtkWidget *win, *align, *label, *vbox, *hbox, *button, *hbox2;
   GtkWidget *pix;
   GdkPixmap *pm, *mm;
   
   win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
   CONNECT(win, "delete_event", cb_close, win);
   gtk_window_set_wmclass(GTK_WINDOW(win), "LVS-Gui", "LVS");
   gtk_window_set_title(GTK_WINDOW(win), "LVS Configuration");
   gtk_container_border_width(GTK_CONTAINER(win), 2);
   gtk_window_set_policy(GTK_WINDOW(win), 1, 1, 1);
   
   vbox = gtk_vbox_new(FALSE, 0);
   gtk_container_add(GTK_CONTAINER(win), vbox);
   
   hbox = gtk_hbox_new(FALSE, 2);
   gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 2);
   
   label = gtk_label_new("Connection forwarding and balancing\n"
			 "configuration options");
   gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 2);
   
   gtk_widget_realize(win);
   pm = gdk_pixmap_create_from_xpm_d(win->window, &mm, NULL, va_xpm);
   pix = gtk_pixmap_new(pm, mm);
   gdk_pixmap_unref(pm);
   gdk_pixmap_unref(mm);
   
   button = gtk_button_new();
   gtk_container_add(GTK_CONTAINER(button), pix);
   gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 2);

   CONNECT(button, "clicked", cb_about, NULL);
   
   align = gtk_alignment_new(0.5, 0.5, 1.0, 1.0);
   gtk_box_pack_start(GTK_BOX(vbox), align, TRUE, TRUE, 0);
   
   gui_tcp(align);
   
   align = gtk_alignment_new(1.0, 0.5, 0.0, 0.0);
   gtk_box_pack_start(GTK_BOX(vbox), align, FALSE, FALSE, 0);
   
   hbox2 = gtk_hbox_new(FALSE, 2);
   gtk_container_add(GTK_CONTAINER(align), hbox2);
   
   hbox = gtk_hbox_new(FALSE, 2);
   gtk_box_pack_start(GTK_BOX(hbox2), hbox, TRUE, TRUE, 2);
   
   cfg_reload = button = gtk_button_new_with_label("Reload Configuration");
   gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 2);
   
   cfg_config = button = gtk_button_new_with_label("Change Configuration...");
   gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 2);
   
   hbox = gtk_hbox_new(TRUE, 2);
   gtk_box_pack_start(GTK_BOX(hbox2), hbox, FALSE, FALSE, 2);
   
   tcp_ok = button = gtk_button_new_with_label("OK");
   gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 2);
   
   tcp_apply = button = gtk_button_new_with_label("Apply");
   gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 2);
   
   tcp_cancel = button = gtk_button_new_with_label("Cancel");
   gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 2);
   
   CONNECT(tcp_ok,     "clicked", cb_ok,     NULL);
   CONNECT(tcp_apply,  "clicked", cb_apply,  NULL);
   CONNECT(tcp_cancel, "clicked", cb_cancel, NULL);
   CONNECT(cfg_reload, "clicked", cb_reload, NULL);
   CONNECT(cfg_config, "clicked", cb_config, NULL);
   
   gtk_widget_show_all(win);
}

/* utility functions */

void
del_tcp(TCP *t)
{
   GList *l;
   gint i;
   
   for (l = t->servers; l; l = l->next)
     {
	TCP2 *t2;
	
	t2 = l->data;
	if (t2->address)
	   g_free(t2->address);
	if (t2->port)
	   g_free(t2->port);
	g_free(t2);
     }
   if (t->servers)
      g_list_free(t->servers);
   if (t->address)
      g_free(t->address);
   if (t->port)
      g_free(t->port);
   g_free(t);
   for (l = tcps, i = 0; l; l = l->next, i++)
     {
	if (l->data == t)
	  {
	     gtk_clist_select_row(GTK_CLIST(tcp_list), i + 1, 0);
	     gtk_clist_remove(GTK_CLIST(tcp_list), i);
	     break;
	  }
     }
   tcps = g_list_remove(tcps, t);
}

void
del_tcp2(TCP *t, TCP2 *t2)
{
   GList *l;
   gint i;
   
   if (t2->address)
      g_free(t2->address);
   if (t2->port)
      g_free(t2->port);
   g_free(t2);
   for (i = 0, l = t->servers; l; l = l->next, i++)
     {
	if (l->data == t2)
	  {
	     gtk_clist_select_row(GTK_CLIST(tcp_list2), i + 1, 0);
	     gtk_clist_remove(GTK_CLIST(tcp_list2), i);
	     break;
	  }
     }
   t->servers = g_list_remove(t->servers, t2);
}

TCP *
add_tcp(char *addr, char *port, int method, int protocol)
{
   TCP *t;
   gchar *s_text[4];
   gchar *s_methods[] =
     {
	"Weighted least connection",
	"Weighted round robin",
	"Round robin",
	"Least connection"
     };	   
   gchar *s_protocols[] =
     {
	"TCP",
	"TCP Persistant",
	"UDP"
     };	   
   
   t = g_malloc(sizeof(TCP));
   
   t->address  = g_strdup(addr);
   t->port     = g_strdup(port);
   t->servers  = NULL;
   t->method   = method;
   t->protocol = protocol;
   
   tcps = g_list_append(tcps, t);
   
   s_text[0] = t->address;
   s_text[1] = t->port;
   s_text[2] = s_methods[t->method];
   s_text[3] = s_protocols[t->protocol];
   gtk_clist_append(GTK_CLIST(tcp_list), s_text);
   
   return t;
}

void
add_tcp2(TCP *t, char *addr, char *port, int weight)
{
   TCP2 *t2;
   gchar s_weight[32], *s_text[3];
   
   t2 = g_malloc(sizeof(TCP2));
   
   t2->address = g_strdup(addr);
   t2->port    = g_strdup(port);
   t2->weight  = weight;
   
   t->servers = g_list_append(t->servers, t2);
   
   g_snprintf(s_weight, sizeof(s_weight), "%i", t2->weight);
   
   s_text[0] = t2->address;
   s_text[1] = t2->port;
   s_text[2] = s_weight;
   gtk_clist_append(GTK_CLIST(tcp_list2), s_text);
}

void
fill_tcp_list2(TCP *t)
{
   gtk_clist_freeze(GTK_CLIST(tcp_list2));
   gtk_clist_clear(GTK_CLIST(tcp_list2));
   if ((t) && (t->servers))
     {
	GList *l;
	
	for (l = t->servers; l; l = l->next)
	  {
	     TCP2 *t2;
	     gchar s_weight[32], *s_text[3];
	     
	     t2 = l->data;
	     g_snprintf(s_weight, sizeof(s_weight), "%i", t2->weight);
	     
	     s_text[0] = t2->address;
	     s_text[1] = t2->port;
	     s_text[2] = s_weight;
	     gtk_clist_append(GTK_CLIST(tcp_list2), s_text);	     
	  }
     }
   gtk_clist_thaw(GTK_CLIST(tcp_list2));
}

void
update_tcp(TCP *t)
{
   GList *l;
   gint i;
   gchar *s_text[4];
   gchar *s_methods[] =
     {
	"Weighted least connection",
	"Weighted round robin",
	"Round robin",
	"Least connection"
     };	   
   gchar *s_protocols[] =
     {
	"TCP",
	"TCP Persistant",
	"UDP"
     };	   
   
   for (l = tcps, i = 0; l; l = l->next, i++)
     {
	if (l->data == t)
	  {
	     s_text[0] = t->address;
	     s_text[1] = t->port;
	     s_text[2] = s_methods[t->method];
	     s_text[3] = s_protocols[t->protocol];
	     gtk_clist_freeze(GTK_CLIST(tcp_list));
	     gtk_clist_set_text(GTK_CLIST(tcp_list), i, 0, s_text[0]);
	     gtk_clist_set_text(GTK_CLIST(tcp_list), i, 1, s_text[1]);
	     gtk_clist_set_text(GTK_CLIST(tcp_list), i, 2, s_text[2]);
	     gtk_clist_set_text(GTK_CLIST(tcp_list), i, 3, s_text[3]);
	     gtk_clist_thaw(GTK_CLIST(tcp_list));
	  }
     }
}

void
update_tcp2(TCP *t, TCP2 *t2)
{
   GList *l;
   gint i;
   gchar *s_text[3];
   
   for (l = t->servers, i = 0; l; l = l->next, i++)
     {
	if (l->data == t2)
	  {
	     gchar s_weight[32];
	     
	     g_snprintf(s_weight, sizeof(s_weight), "%i", t2->weight);
	     s_text[0] = t2->address;
	     s_text[1] = t2->port;
	     s_text[2] = s_weight;
	     gtk_clist_freeze(GTK_CLIST(tcp_list2));
	     gtk_clist_set_text(GTK_CLIST(tcp_list2), i, 0, s_text[0]);
	     gtk_clist_set_text(GTK_CLIST(tcp_list2), i, 1, s_text[1]);
	     gtk_clist_set_text(GTK_CLIST(tcp_list2), i, 2, s_text[2]);
	     gtk_clist_thaw(GTK_CLIST(tcp_list2));
	  }
     }
}

/* config load/save */
void
load_config(char *file)
{
   TCP *t = NULL;
   FILE *f;
   
   /* delete old stuff */
   while (tcps)
      del_tcp(tcps->data);
   
   /* -s wlc = weighted lest connection */
   /* -s wrr = weighted round-robnin */
   /* -s rr =  round robin */
   /* -s lc =  least connection */
   
   /* -w NUM = weight of server */
   
   /* IP.ADDRESS.COM.BLAH:PORT */
   f = fopen(file,  "r");
   if (f)
     {
	char buf[1024];
	
	while (fgets(buf, 1023, f))
	  {
	     gchar opt1[16], opt2[16], addr[512], port[32];
	     gchar meth[32], opt3[16], wgt[12];
	     
	     if (buf[0] != '#')
	       {
		  opt1[0] = 0;
		  opt3[0] = 0;
		  sscanf(buf, 
			 "%*s %15s %15s %511[0-9a-zA-Z.-]:%31s %*s %31s %15s ",
			 opt1, opt2, addr, port, meth, opt3);
		  if (!strcmp(opt1, "-A"))
		    {
		       int method = METHOD_WLC, protocol = PROTO_TCP;
		       
		       if (!strcmp(meth, "wlc"))
			  method = METHOD_WLC;
		       else if (!strcmp(meth, "wrr"))
			  method = METHOD_WRR;
		       else if (!strcmp(meth, "rr"))
			  method = METHOD_RR;
		       else if (!strcmp(meth, "lc"))
			  method = METHOD_LC;
		       if (!strcmp(opt2, "-u"))
			  protocol = PROTO_UDP;
		       else if (!strcmp(opt3, "-p"))
			  protocol = PROTO_TCP_PERSIST;
		       t = add_tcp(addr, port, method, protocol);
		    }
		  else if ((!strcmp(opt1, "-a")) && (t))
		    {
		       sscanf(buf, 
			      "%*s %*s %*s %*s %*s %511[0-9a-zA-Z.-]:%31s %*s %*s %15s ",
			      addr, port, wgt);
		       add_tcp2(t, addr, port, atoi(wgt));
		    }
	       }
	  }
	fclose(f);
     }
   gtk_clist_select_row(GTK_CLIST(tcp_list), 1, 0);
   gtk_clist_select_row(GTK_CLIST(tcp_list), 0, 0);
}

void
save_transparent_proxy(char *host, GList *hosts)
{
   FILE *f;
   gchar tprox[4096];
   
   g_snprintf(tprox, sizeof(tprox), "transparent_proxy");
   f = fopen(tprox, "w");
   if (f)
     {
	GList *l;
	
	fprintf(f,
		"#########################################################\n"
		"# WARNING WARNING WARING WARNING WARNING WARING WARNING #\n"
		"#########################################################\n"
		"# Do NOT Edit this file by hand. This is automatically  #\n"
		"# generated and its format is assumed by the config     #\n"
		"# tools that generate this file.                        #\n"
		"#########################################################\n"
		"\n"
		"#########################################################\n"
		"# Config flie for IPVS transparent back end servers     #\n"
		"# settings for transparent proxy                        #\n"
		"# Generated by a GUI config tool                        #\n"
		"#########################################################\n"
		"\n"
		"#########################################################\n"
		"# Config follows ...                                    #\n"
		"#########################################################\n"
		"\n\n\n\n"
		);
	fprintf(f, "TRANSPARENT_ADDRESSES=\"%s", host);
	if (hosts)
	  {
	     for (l = hosts; l; l = l->next)
		fprintf(f, " %s", (char *)l->data);
	  }
	fprintf(f, "\"\n");
	fprintf(f, 
		"\n\n\n\n"
		"#########################################################\n"
		"# End config ...                                        #\n"
		"#########################################################\n"
		"\n"
		"#########################################################\n"
		"# WARNING WARNING WARING WARNING WARNING WARING WARNING #\n"
		"#########################################################\n"
		"# Do NOT Edit this file by hand. This is automatically  #\n"
		"# generated and its format is assumed by the config     #\n"
		"# tools that generate this file.                        #\n"
		"#########################################################\n"
		);
	fclose(f);
     }
}

void
save_config(char *file)
{
   FILE *f;
   
   /* write ipvs config */
   f = fopen(file, "w");
   if (f)
     {
	TCP   *t;
	TCP2  *t2;
	GList *l, *ll;
	gchar *methods[] = {"wlc", "wrr", "rr", "lc"};
	
	fprintf(f, 
		"#########################################################\n"
		"# WARNING WARNING WARING WARNING WARNING WARING WARNING #\n"
		"#########################################################\n"
		"# Do NOT Edit this file by hand. This is automatically  #\n"
		"# generated and its format is assumed by the config     #\n"
		"# tools that generate this file.                        #\n"
		"#########################################################\n"
		"\n"
		"#########################################################\n"
		"# Config flie for IPVS                                  #\n"
		"# Commands to configure and IPVS server, to be          #\n"
		"# run by /etc/rc.d/init.d/ipvs                          #\n"
		"# Generated by a GUI config tool                        #\n"
		"#########################################################\n"
		"\n"
		"#########################################################\n"
		"# Config follows ...                                    #\n"
		"#########################################################\n"
		"\n\n\n\n"
		);
	for (l = tcps; l; l = l->next)
	  {
	     t = l->data;
	     
	     if (t->protocol == PROTO_TCP)
		fprintf(f, "ipvsadm -A -t %s:%s -s %s\n", 
			t->address, t->port, methods[t->method]);
	     else if (t->protocol == PROTO_TCP_PERSIST)
		fprintf(f, "ipvsadm -A -t %s:%s -s %s -p 300 -M 255.255.255.255\n",
			t->address, t->port, methods[t->method]);
	     else if (t->protocol == PROTO_UDP)
		fprintf(f, "ipvsadm -A -u %s:%s -s %s\n", 
			t->address, t->port, methods[t->method]);
	     for (ll = t->servers; ll; ll = ll->next)
	       {
		  t2 = ll->data;
		  
		  if (t->protocol == PROTO_UDP)
		     fprintf(f, "ipvsadm -a -u %s:%s -r %s:%s -g -w %i\n",
			     t->address, t->port, t2->address, t2->port, t2->weight);
		  else
		     fprintf(f, "ipvsadm -a -t %s:%s -r %s:%s -g -w %i\n",
			     t->address, t->port, t2->address, t2->port, t2->weight);
	       }
	  }
	fprintf(f, 
		"\n\n\n\n"
		"#########################################################\n"
		"# End config ...                                        #\n"
		"#########################################################\n"
		"\n"
		"#########################################################\n"
		"# WARNING WARNING WARING WARNING WARNING WARING WARNING #\n"
		"#########################################################\n"
		"# Do NOT Edit this file by hand. This is automatically  #\n"
		"# generated and its format is assumed by the config     #\n"
		"# tools that generate this file.                        #\n"
		"#########################################################\n"
		);
	fclose(f);
     }
   /* write transparent proxy config per machine */
     {
	gchar *host;
	GList *host_names = NULL;
	GList *hosts = NULL;
        TCP   *t;
	TCP2  *t2;
	GList *l, *ll;	
	
	/* build list of unique backend serevr hosts */
        for (l = tcps; l; l = l->next)
	  {
	     t = l->data;
	     
             for (ll = t->servers; ll; ll = ll->next)
	       {
		  GList *l2;
		  int    ok = 1;
		  
		  t2 = ll->data;		  
		  for (l2 = host_names; l2; l2 = l2->next)
		    {
		       if (!strcmp(l2->data, t2->address))
			  ok = 0;
		    }
		  if (ok)
		     host_names = g_list_append(host_names, t2->address);
	       }
	  }
	/* go thru backend host list and build proxy host list */
	for (l = host_names; l; l = l->next)
	  {
	     /* go thru all frontend servers */
	     for (ll = tcps; ll; ll = ll->next)
	       {
		  GList *l2;
		  int    ok = 0;
		  
		  t = ll->data;
		  
		  /* find server in bacnekdn server list */
		  for (l2 = t->servers; l2; l2 = l2->next)
		    {
		       t2 = l2->data;
		       if (!strcmp(l->data, t2->address))
			  ok = 1;
		    }
		  /* if front end server contains backend in list - add */
		  if (ok)
		    {
		       /* if the serevsr is alreayd inthe list - dont add */
		       for (l2 = hosts; l2; l2 = l2->next)
			 {
			    if (!strcmp(l2->data, t->address))
			       ok = 0;
			 }
		       if (ok)
			  hosts = g_list_append(hosts, t->address);
		    }
	       }
	     host = l->data;
	     save_transparent_proxy(host, hosts);
	     remote_cp(NULL, "transparent_proxy", 
		       host, opt.transparent_proxy_config_file);
	     start_transparent_proxy(host);
	     if (hosts)
	       {
		  g_list_free(hosts);
		  hosts = NULL;
	       }
	  }
	if (host_names)
	   g_list_free(host_names);
     }
   remote_cp(NULL, "ipvs", opt.master_host, opt.ipvs_config_file);
   start_lvs(opt.master_host);
}

/* activation */

void
start_lvs(char *machine)
{
   gchar s[4096];
   
   g_snprintf(s, sizeof(s), "%s %s@%s %s start", 
	      opt.rsh_command, opt.user, machine, opt.ipvs_init_script);
   system(s);
}

/*
 void
 stop_lvs(char *machine)
 {
    gchar s[4096];
    
    g_snprintf(s, sizeof(s), "%s %s@%s %s stop", 
	       opt.rsh_command, opt.user, machine,  opt.ipvs_init_script);
    system(s);
 }
 */

void
start_transparent_proxy(char *machine)
{
   gchar s[4096];
   
   g_snprintf(s, sizeof(s), 
	      "%s %s@%s %s start", opt.rsh_command,
	      opt.user, machine, opt.transparent_proxy_init_script);
   system(s);
}

/*
 void
 stop_transparent_proxy(char *machine)
 {
    gchar s[4096];
    
    g_snprintf(s, sizeof(s), 
	       "%s %s@%s %s stop", opt.rsh_command,
	       opt.user, machine, opt.transparent_proxy_init_script);
    system(s);
 }
 */

/* copying files around */
void
remote_cp(char *machine1, char *file1, char *machine2, char *file2)
{
   gchar s[4096];
   
   if ((machine1) && (machine2))
      g_snprintf(s, sizeof(s), "%s %s@%s:%s %s@%s:%s", opt.rcp_command,
		 opt.user, machine1, file1, opt.user, machine2, file2);
   else if ((!machine1) && (machine2))
      g_snprintf(s, sizeof(s), "%s %s %s@%s:%s",  opt.rcp_command,
		 file1, opt.user, machine2, file2);
   else if ((machine1) && (!machine2))
      g_snprintf(s, sizeof(s), "%s %s@%s:%s %s",  opt.rcp_command,
		 opt.user, machine1, file1, file2);
   else
      g_snprintf(s, sizeof(s), "cp %s %s", file1, file2);
   system(s);
}


/* Put GTK values into opt struct and write to disk*/

void gui_save_config(void){
   extern options_t opt;
   
   config_file_write(opt.rc_file);
}


/* main */

int
main(int argc, char **argv)
{
   options(argc, argv, OPT_FIRST_CALL);
   config_file_to_opt(opt.rc_file);
   config_file_write(opt.rc_file);
   
   gtk_init(&argc, &argv);   
   gui();
   
   remote_cp(opt.master_host, opt.ipvs_config_file, NULL, "ipvs");
   load_config("ipvs");
   gtk_main();
   return(0);
}
