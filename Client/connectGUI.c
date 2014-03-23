/*
 * connectGUI.c
 *
 *  Created on: 23 Mar 2014
 *      Author: chris
 */

#include "mainGUI.h"
#include "connectGUI.h"
#include "Client.h"

struct mainWindow mainWin;
struct conWindow conWin;

GtkWidget* makeConnectWindow() {
	GtkWidget *connectWindow;
	GtkWidget *vbox; //main box
	GtkWidget *hbox; //for buttons
	GtkWidget *entryPort;
	GtkWidget *labelPort;

	GtkWidget *halign;

	GtkWidget *halign2;
	char port[8];

	mainWin = getMainStruct();
	// create new window
	connectWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	/*
	 * Main window
	 */

	// set window parameters
	gtk_window_set_title(GTK_WINDOW(connectWindow), "Connect");
	gtk_window_set_default_size(GTK_WINDOW(connectWindow), 300, 200);
	gtk_window_set_position(GTK_WINDOW(connectWindow), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(connectWindow), FALSE);
	gtk_window_set_icon(GTK_WINDOW(connectWindow), create_pixbuf("sickle.png"));
	gtk_window_set_modal(GTK_WINDOW(connectWindow), TRUE);
	gtk_window_set_transient_for(GTK_WINDOW(connectWindow), GTK_WINDOW(mainWin.window));
	gtk_window_set_destroy_with_parent(GTK_WINDOW(connectWindow), TRUE);
	gtk_container_set_border_width(GTK_CONTAINER(connectWindow), 10);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(connectWindow), vbox);

	halign = gtk_alignment_new(0, 1, 0, 1);
	halign2 = gtk_alignment_new(0, 1, 0, 1);
	conWin.labelIP = gtk_label_new("Host IP:");
	gtk_container_add(GTK_CONTAINER(halign), conWin.labelIP);
	labelPort = gtk_label_new("Port:");
	gtk_container_add(GTK_CONTAINER(halign2), labelPort);

	conWin.entryIP = gtk_entry_new_with_max_length(16);
	entryPort = gtk_entry_new_with_max_length(8);

	sprintf(port, "%d", TCP_PORT);
	gtk_entry_set_text(GTK_ENTRY(entryPort), port);

	gtk_box_pack_start(GTK_BOX(vbox), halign, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(vbox), conWin.entryIP, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(vbox), halign2, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(vbox), entryPort, FALSE, FALSE, 5);

	hbox = gtk_hbox_new(FALSE, 0);

	conWin.buttonCancel = gtk_button_new_with_label("Cancel");
	conWin.buttonConnect = gtk_button_new_with_label("Connect");
	conWin.buttonDiscover = gtk_button_new_with_label("Find Server");

	gtk_box_pack_start(GTK_BOX(hbox), conWin.buttonConnect, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(hbox), conWin.buttonDiscover, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(hbox), conWin.buttonCancel, FALSE, FALSE, 5);

	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);

	return connectWindow;
}

void showConnect(GtkWidget * widget) {
	widget = makeConnectWindow();
	gtk_widget_show_all(widget);
}

void discover() {
	char * IP;
	//attempt discovery of chat server
	if (chatserverDiscover() == 1) {
		//if found, fill in the IP
		struct sockaddr_in serverAddr;
		IP = ntohl(serverAddr.sin_addr.s_addr);
		gtk_entry_set_text(conWin.entryIP, IP);
	} else {
		//if not found, change label.
		gtk_label_set_text(GTK_LABEL(conWin.labelIP), "Server could not be discovered");
	}
}

void tryConnect() {
}
void cancel() {
}
struct conWindow getConnStruct() {
	return conWin;
}
