#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdint.h>

#include "mainGUI.h"
#include "../Chat.h"

// Shared data
char* clientNames[MAX_CLIENTS];
int clientRooms[MAX_CLIENTS];

struct sockaddr_in serverAddr;
char* serverName;
len_t serverNameLen;

char* myName;
len_t myNameLen;

#define UI 0 // The UI end of the sock set
#define BE 1 // The Back End end of the sock set
SOCKET ipc[2];

void* Backend(void* params);

// Backend Calls
int StartBackend();
int StopBackend();
int SendMsg(char *message, len_t length);
int chatConnect();
int chatDisconnect();
int chatserverDiscover();
int changeRoom(roomNo_t room);

void incomingMessage(int client, char incoming[50]);
void updateClientList(int room);

