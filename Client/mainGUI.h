/*
 * chatGUI.h
 *
 *  Created on: 23 Mar 2014
 *      Author: chris
 */

#ifndef CHATGUI_H_
#define CHATGUI_H_

#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <glib/gstdio.h>

#include "Client.h"
#include "../Chat.h"

#define PORT 29768
#define MAX_MESSAGE     50
#define MAX_ROOMS 4
#define MAX_CHAT_LINE (MAX_MESSAGE + MAX_NAME + 6)

GtkTextBuffer *tBuff[MAX_ROOMS];
GtkWidget *chatRoom[MAX_ROOMS];

typedef struct mainWindow {
	GtkWidget *chat;
	GtkWidget *connect;
	GtkWidget *disconnect;
	GtkWidget *recordChat;
	GtkWidget *quit;
	GtkWidget *help;
	GtkWidget *about;
	GtkWidget *window;
	GtkWidget *clientList;

	GtkWidget *chatArea;

	GtkWidget *chatInput;

	GtkWidget *openDialog;

	FILE * logfile;
} WinStruct;

#endif /* CHATGUI_H_ */

GdkPixbuf *create_pixbuf(const gchar * filename);
void getInput(GtkEntry * widget, GtkWidget * notebook);
GtkWidget* makeClientList();
GtkWidget* makeTabbedChat(int rooms);
GtkWidget * makeMenu();
GtkWidget * makeInput();
void connectDialog();
void closeLogFile();
void openLogFile();
void logFile();
void cleanup();
GtkWidget* makeMainWindow();
struct mainWindow getMainStruct();

