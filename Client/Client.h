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

void* Backend(void* params);

// Backend Calls
int	SendMsg(clientNo_t no, char message[MAX_MESSAGE]);
int	chatConnect();
int	chatDisconnect();
int	chatServerDiscover();


