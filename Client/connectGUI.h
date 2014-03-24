/*
 * connectGUI.h
 *
 *  Created on: 23 Mar 2014
 *      Author: chris
 */

typedef struct conWindow{
	GtkWidget *labelIP;
	GtkWidget *entryIP;
	GtkWidget *buttonCancel;
		GtkWidget *buttonConnect;
		GtkWidget *buttonDiscover;
		GtkWidget *connectWindow;

}conWind;


GtkWidget* makeConnectWindow();
void showConnect(GtkWidget * widget);
void discover();
void tryConnect();
void cancel();
struct conWindow getConnStruct();

