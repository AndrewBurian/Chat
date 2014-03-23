/*
 * GUI.c
 *
 *  Created on: 18 Mar 2014
 *      Author: chris
 */

#include "mainGUI.h"
#include "connectGUI.h"

//char * name = "client name";

// menu items - must be global for callbacks to work;


char ip[20];
int port;
//int logging = 0;

struct mainWindow mainWin;
struct conWindow conWin;

int main(int argc, char *argv[]) {
//GtkWidget *connectWindow;


	gtk_init(&argc, &argv);

	mainWin=getMainStruct();
	conWin = getConnStruct();

	mainWin.window = makeMainWindow();

	gtk_widget_show_all(mainWin.window);
	conWin.connectWindow = makeConnectWindow();

	/*
	 * Callback Functions
	 */

	// when enter is pressed in text input area
	g_signal_connect(G_OBJECT(mainWin.chatInput), "activate", G_CALLBACK(getInput), (gpointer ) mainWin.chatArea);

	// when the program is closed by 'x'
	g_signal_connect(G_OBJECT(mainWin.window), "destroy", G_CALLBACK(cleanup), NULL);
	  g_signal_connect (G_OBJECT(conWin.connectWindow), "destroy", G_CALLBACK(gtk_widget_hide_on_delete), NULL);
	// The menu call to quit
	g_signal_connect(G_OBJECT(mainWin.quit), "activate", G_CALLBACK(cleanup), NULL);

	g_signal_connect_swapped(G_OBJECT(mainWin.connect), "activate", G_CALLBACK(gtk_widget_show_all),(gpointer) conWin.connectWindow);

	g_signal_connect(G_OBJECT(mainWin.recordChat), "activate", G_CALLBACK(logFile), NULL);

	// connect window buttons
	g_signal_connect_swapped(G_OBJECT(conWin.buttonCancel), "clicked", G_CALLBACK(gtk_widget_hide), (gpointer) conWin.connectWindow);
	g_signal_connect(G_OBJECT(conWin.buttonConnect), "clicked", G_CALLBACK(tryConnect), NULL);
	g_signal_connect(G_OBJECT(conWin.buttonDiscover), "clicked", G_CALLBACK(discover), NULL);

	gtk_main();

	StartBackend();

	return 0;
}
