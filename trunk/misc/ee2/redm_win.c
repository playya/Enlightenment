/*
	red color modifiers and other fun stuff
*/

#include "ee2.h"

void
redm_init(void)
{
	GtkWidget *nfr1, *nlbl, *vbox1, *sep, *btn;
	GtkWidget *b1;
	GtkWidget *r1, *r2, *r3;
	GtkAdjustment *adj, *adj2, *adj3;

	DATA8 rmap[256];

	nlbl = gtk_label_new("Red Modifiers");
	nfr1 = gtk_frame_new("Red Modifiers");
	gtk_container_set_border_width(GTK_CONTAINER(nfr1), 3);
	gtk_notebook_insert_page(GTK_NOTEBOOK(ModMdi), nfr1, nlbl, 1);
	gtk_widget_show(nlbl);
	gtk_widget_show(nfr1);

	vbox1 = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(nfr1), vbox1);

	adj  = gtk_adjustment_new(0, 0, 256, 1, 1, 0);
	adj2 = gtk_adjustment_new(0, 0, 256, 1, 1, 0);
	adj3 = gtk_adjustment_new(0, 0, 256, 1, 1, 0);

	r1 = gtk_hscale_new(adj);
	gtk_widget_show(r1);
	gtk_box_pack_start(GTK_BOX(vbox1), r1, TRUE, TRUE, 0);

	r2 = gtk_hscale_new(adj2);
	gtk_widget_show(r2);
	gtk_box_pack_start(GTK_BOX(vbox1), r2, TRUE, TRUE, 0);

	r3 = gtk_hscale_new(adj3);
	gtk_widget_show(r3);
	gtk_box_pack_start(GTK_BOX(vbox1), r3, TRUE, TRUE, 0);

	gtk_widget_show(vbox1);
}
