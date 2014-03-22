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
#include <signal.h>
#include <stdint.h>


// Shared data
char clientNames[MAX_CLIENTS][MAX_NAME];
int clientRooms[MAX_CLIENTS];
struct sockaddr_in serverAddr;
char myName[MAX_NAME];
len_t myNameLen;

SOCKET ipc[2];



void* Backend(void* params);

// Backend Calls
int	SendMsg(char *message, len_t length);
int	chatConnect();
int	chatDisconnect();
int	chatServerDiscover();


