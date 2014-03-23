/*
 * chatGUI.c
 *
 *  Created on: 23 Mar 2014
 *      Author: chris
 */

#include "mainGUI.h"

struct mainWindow mainWin;

int logging;
char name[MAX_NAME] = "Client0";
/*
 * This function sets up the program icon
 */

GdkPixbuf *create_pixbuf(const gchar * filename) {
	GdkPixbuf *pixbuf;
	GError *error = NULL;
	pixbuf = gdk_pixbuf_new_from_file(filename, &error);
	if (!pixbuf) {
		fprintf(stderr, "%s\n", error->message);
		g_error_free(error);
	}

	return pixbuf;
}

/*
 * This function prints the text from the input box to the chat window for the current room
 */

void getInput(GtkEntry * widget, GtkWidget * notebook) {

	GtkTextBuffer *tbuffer;
	GtkTextIter tIter;
	GtkTextTag* redTag;
	char message[MAX_CHAT_LINE];
	const char * input = "";

	int i = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
	tbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chatRoom[i]));
	gtk_text_buffer_get_end_iter(tbuffer, &tIter);
	redTag = gtk_text_buffer_create_tag(tbuffer, NULL, "foreground", "#FF0000", NULL);

	input = gtk_entry_get_text(widget);
	sprintf(message, "[%s] - %s\n", name, input);
	gtk_text_buffer_insert_with_tags(tbuffer, &tIter, message, -1, redTag, NULL);
	if (logging == 1) {
		fprintf(mainWin.logfile, "%s", message);
	}
	gtk_entry_set_text(widget, "");
	gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(chatRoom[i]), &tIter, 0, FALSE, 1.0, 1.0);

	//send room update
	//update client list
	//send message(input)
}

/*
 * The client list window
 */
GtkWidget* makeClientList() {
	GtkWidget * scrollbarClients;

	scrollbarClients = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollbarClients), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	mainWin.clientList = gtk_text_view_new();
	gtk_container_add(GTK_CONTAINER(scrollbarClients), mainWin.clientList);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(mainWin.clientList), FALSE);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(mainWin.clientList), FALSE);
	gtk_widget_set_size_request(mainWin.clientList, 250, 200);
	return scrollbarClients;
}

/*
 * This method creates the tabbed rooms
 */
GtkWidget* makeTabbedChat(int rooms) {
	int i;
	GtkWidget* child;
	GtkWidget* scroll;
	gchar label[7];
	GtkWidget *roomLabel[MAX_ROOMS];
	GtkWidget *note = gtk_notebook_new();
	gtk_notebook_set_scrollable(GTK_NOTEBOOK(note), TRUE);
	for (i = 0; i < rooms; ++i) {

		//create child windows
		child = gtk_table_new(2, 1, FALSE);

		chatRoom[i] = gtk_text_view_new_with_buffer(tBuff[i]);

		scroll = gtk_scrolled_window_new(0, NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
		gtk_container_add(GTK_CONTAINER(scroll), chatRoom[i]);

		gtk_text_view_set_editable(GTK_TEXT_VIEW(chatRoom[i]), FALSE);
		gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(chatRoom[i]), FALSE);

		gtk_table_attach(GTK_TABLE(child), scroll, 1, 2, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 0, 0);

		gtk_widget_set_size_request(chatRoom[i], 550, 400);

		//create label for tab
		g_sprintf(label, "Room %d", i + 1);
		roomLabel[i] = gtk_label_new(label);

		gtk_notebook_append_page(GTK_NOTEBOOK(note), child, roomLabel[i]);

	}
	return note;
}

/*
 * This method makes the menu bar
 */
GtkWidget * makeMenu() {
	GtkWidget *menubar;

	// Items on menu bar
	GtkWidget *helpmenu;
	GtkWidget *chatmenu;
	GtkWidget *helpMenu;

	menubar = gtk_menu_bar_new();
	chatmenu = gtk_menu_new();
	helpmenu = gtk_menu_new();

	// menu titles and items
	mainWin.chat = gtk_menu_item_new_with_label("Chat");
	mainWin.connect = gtk_menu_item_new_with_label("Connect");
	mainWin.disconnect = gtk_menu_item_new_with_label("Disconnect");
	mainWin.recordChat = gtk_check_menu_item_new_with_label("Save chat");

	mainWin.quit = gtk_menu_item_new_with_label("Quit");
	mainWin.help = gtk_menu_item_new_with_label("Help");
	helpMenu = gtk_menu_item_new_with_label("Help");
	mainWin.about = gtk_menu_item_new_with_label("About");

	// put menu together - first items and then menu bar option
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(mainWin.chat), chatmenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(chatmenu), mainWin.connect);
	gtk_menu_shell_append(GTK_MENU_SHELL(chatmenu), mainWin.disconnect);
	gtk_menu_shell_append(GTK_MENU_SHELL(chatmenu), mainWin.recordChat);
	gtk_menu_shell_append(GTK_MENU_SHELL(chatmenu), mainWin.quit);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), mainWin.chat);

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(helpMenu), helpmenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(helpmenu), mainWin.about);
	gtk_menu_shell_append(GTK_MENU_SHELL(helpmenu), mainWin.help);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), helpMenu);
	return menubar;
}

/*
 * This method makes the input area
 */
GtkWidget * makeInput() {
	GtkWidget *chatInputTitle;
	GtkWidget *halign3;
	GtkWidget *tableInput;

	tableInput = gtk_table_new(2, 6, TRUE);

	chatInputTitle = gtk_label_new("Chat input");
	halign3 = gtk_alignment_new(0, 1, 0, 1);
	gtk_container_add(GTK_CONTAINER(halign3), chatInputTitle);
	gtk_table_attach(GTK_TABLE(tableInput), halign3, 0, 1, 0, 1, GTK_FILL, GTK_FILL, 0, 0);

	mainWin.chatInput = gtk_entry_new_with_max_length(MAX_MESSAGE);
	gtk_widget_set_size_request(mainWin.chatInput, 100, 30);

	gtk_table_attach(GTK_TABLE(tableInput), mainWin.chatInput, 0, 5, 1, 2, GTK_FILL | GTK_EXPAND, 0, 0, 0);

	return tableInput;
}

/*
 * This method creates the connect dialog on command
 */
void connectDialog() {
	GtkWidget *connectBox;
	GtkWidget *entryIP;
	GtkWidget *entryPort;
	GtkWidget *labelPort;
	GtkWidget *labelIP;
	GtkWidget *content_area;

	gint discovered;
	int kill = 1;

	connectBox = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(connectBox), "Connect");
	gtk_dialog_add_button(GTK_DIALOG(connectBox), "Discover", 1);
	gtk_dialog_add_button(GTK_DIALOG(connectBox), "Connect", 2);
	gtk_dialog_add_button(GTK_DIALOG(connectBox), "Cancel", 6);
	gtk_window_set_modal(GTK_WINDOW(connectBox), TRUE);
	gtk_window_set_destroy_with_parent(GTK_WINDOW(connectBox), TRUE);
	gtk_widget_set_size_request(connectBox, 350, 250);

	labelIP = gtk_label_new("Host IP:");
	labelPort = gtk_label_new("Port:");

	entryIP = gtk_entry_new_with_max_length(16);
	entryPort = gtk_entry_new_with_max_length(8);

	gtk_entry_set_text(GTK_ENTRY(entryPort), "29768");

	content_area = gtk_dialog_get_content_area(GTK_DIALOG(connectBox));
	gtk_box_pack_start(GTK_BOX(content_area), labelIP, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(content_area), entryIP, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(content_area), labelPort, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(content_area), entryPort, FALSE, FALSE, 5);

	gtk_widget_show_all(connectBox);

	gint result = gtk_dialog_run(GTK_DIALOG(connectBox));
	switch (result) {
	case 1:
		//discover

		// this is where we try to get the ip from the server
		discovered = 1;
		if (discovered == 1) {
			gtk_entry_set_text(GTK_ENTRY(entryIP), "This IP");
		} else {
			gtk_entry_set_text(GTK_ENTRY(entryIP), "No server found");
		}
		kill = 0;
		break;
	case 2:
		//connect
		//call connect with the ip and port given
		break;
	case 6:
		kill = 1;
		break;
	default:

		break;
	}
	if (kill == 1) {
		gtk_widget_destroy(connectBox);
	}
}

/*
 * this method closes the log file
 */
void closeLogFile() {
	fclose(mainWin.logfile);
	logging = 0;
}

/*
 * This method brings up the file chooser dialog and opens the log file for writing
 */
void openLogFile() {
	mainWin.openDialog = gtk_file_chooser_dialog_new("Choose log file", GTK_WINDOW(mainWin.window),
			GTK_FILE_CHOOSER_ACTION_SAVE,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
			NULL);

	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(mainWin.openDialog), "Untitled document");

	if (gtk_dialog_run(GTK_DIALOG(mainWin.openDialog)) == GTK_RESPONSE_ACCEPT) {
		if (logging == 1) {
			closeLogFile();
		}

		char *filename;

		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(mainWin.openDialog));
		mainWin.logfile = g_fopen(filename, "a+");
		g_free(filename);
		logging = 1;
	}
	gtk_widget_destroy(mainWin.openDialog);
}

/*
 * Helper method for the log file menu item
 */
void logFile() {
	if (logging == 0) {
		openLogFile();
	} else {
		closeLogFile();
	}
	if (logging == 0) {
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mainWin.recordChat), FALSE);
	}
}

/*
 *  method cleans up on exit
 */
void cleanup() {
	if (logging == 1) {
		closeLogFile();
	}
	gtk_main_quit();
}

/*
 * this method adds incoming messages to the appropriate buffers and also saves to file if user is logging and in the same room
 */
void incomingMessage(int client, char incoming[MAX_MESSAGE]) {
	GtkTextBuffer *tbuffer;
	GtkTextIter tIter;
	GtkTextTag* blueTag;
	char message[MAX_CHAT_LINE];
	char name[MAX_NAME];
	int room = 1;

	if (room >= MAX_ROOMS) {
		return;
	}

	if (client >= MAX_CLIENTS) {
		return;
	}
	/*
	 * name = client name somehow......
	 */

	//get the current page
	int i = gtk_notebook_get_current_page(GTK_NOTEBOOK(mainWin.chat));

	//get the buffer for the room the message is going to
	tbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chatRoom[room]));
	// set the iterator to the end of that buffer
	gtk_text_buffer_get_end_iter(tbuffer, &tIter);
	// set the text to be blue
	blueTag = gtk_text_buffer_create_tag(tbuffer, NULL, "foreground", "#0000FF", NULL);
	// format the message with the sender's name
	sprintf(message, "[%s] - %s\n", name, incoming);
	//add the formatted message to the buffer
	gtk_text_buffer_insert_with_tags(tbuffer, &tIter, message, -1, blueTag, NULL);
	// make the buffer 'scroll' to the new end of the message
	gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(chatRoom[i]), &tIter, 0, FALSE, 1.0, 1.0);
	// if logging is enabled and the user is currently in this room, add message to log file
	if (logging == 1 && room == i) {
		fprintf(mainWin.logfile, "%s", message);
	}
}

GtkWidget* makeMainWindow() {
	GtkWidget *mainWindow;

	GtkWidget *vbox;

	GtkWidget *menubar;

	GtkWidget *table;

// the title labels
	GtkWidget *chatWinTitle;
	GtkWidget *clientListTitle;

// and somewhere to put them
	GtkWidget *halign;
	GtkWidget *halign2;

	GtkWidget *halign4;

	GtkWidget *clientList;

	GtkWidget *inputArea;

	// create new window
	mainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	/*
	 * Main window
	 */

	// set window parameters
	gtk_window_set_title(GTK_WINDOW(mainWindow), "CommieChat");
	gtk_window_set_default_size(GTK_WINDOW(mainWindow), 800, 500);
	gtk_window_set_position(GTK_WINDOW(mainWindow), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(mainWindow), FALSE);
	gtk_window_set_icon(GTK_WINDOW(mainWindow), create_pixbuf("sickle.png"));

	// the layout table
	table = gtk_table_new(8, 6, FALSE);
	//gtk_table_set_row_spacings(GTK_TABLE(table), 10);
	//gtk_table_set_col_spacings(GTK_TABLE(table), 5);
	gtk_container_add(GTK_CONTAINER(mainWindow), table);

	/*
	 *  The menu bar
	 */

	// the menu bar container
	halign2 = gtk_alignment_new(0, 1, 0, 1);
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(halign2), vbox);
	gtk_table_attach(GTK_TABLE(table), halign2, 0, 4, 0, 1, GTK_FILL, GTK_FILL, 0, 0);

	// the menu bar
	menubar = makeMenu();
	gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 3);

	/*
	 *  Chat display
	 */

	// the chat title
	chatWinTitle = gtk_label_new("Chat window");
	halign = gtk_alignment_new(0, 1, 0, 1);
	gtk_container_add(GTK_CONTAINER(halign), chatWinTitle);
	gtk_table_attach(GTK_TABLE(table), halign, 0, 1, 1, 2, GTK_FILL, GTK_FILL, 20, 0);

	// the window displaying the chat
	mainWin.chatArea = makeTabbedChat(MAX_ROOMS);
	gtk_table_attach(GTK_TABLE(table), mainWin.chatArea, 0, 4, 2, 6, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 20,
			0);

	/*
	 * Message input
	 */

	// the text input area
	inputArea = makeInput();
	gtk_table_attach(GTK_TABLE(table), inputArea, 0, 4, 6, 7, GTK_FILL, GTK_FILL, 20, 20);

	/*
	 *  Client list
	 */

	// the title
	clientListTitle = gtk_label_new("Client list");
	halign4 = gtk_alignment_new(0, 1, 0, 1);
	gtk_container_add(GTK_CONTAINER(halign4), clientListTitle);
	gtk_table_attach(GTK_TABLE(table), halign4, 4, 6, 1, 2, GTK_FILL, GTK_FILL, 20, 0);

	// the client list
	clientList = makeClientList();
	gtk_table_attach(GTK_TABLE(table), clientList, 4, 6, 2, 6, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 20, 0);

	return mainWindow;
}

void updateClientList(int room) {
	GtkTextBuffer *tbuffer;
	GtkTextIter tIter;
	GtkTextTag* tag;

	char client[MAX_NAME + 1];
	int j, k;

	//get the client list buffer, clean it out
	tbuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(mainWin.clientList));
	gtk_text_buffer_get_end_iter(tbuffer, &tIter);
	gtk_text_buffer_set_text(tbuffer, "", -1);

	// populate the list by room, giving a room heading and then the list of clients in that room
	for (j = 0; j < MAX_ROOMS; ++j) {
		if (j == room) {
			//same room is green, other rooms are blue
			tag = gtk_text_buffer_create_tag(tbuffer, NULL, "foreground", "#00FF00", NULL);
		} else {
			tag = gtk_text_buffer_create_tag(tbuffer, NULL, "foreground", "#0000FF", NULL);
		}
		sprintf(client, "----Room %d----\n", j + 1);
		gtk_text_buffer_insert_with_tags(tbuffer, &tIter, client, -1, tag, NULL);

		for (k = 0; k < MAX_CLIENTS; ++k) {
			if (clientRooms[k] == j) {
				sprintf(client, "%s\n", clientNames[j]);
				gtk_text_buffer_insert_with_tags(tbuffer, &tIter, client, -1, tag, NULL);
			}
		}
	}
}

struct mainWindow getMainStruct() {
	return mainWin;
}
