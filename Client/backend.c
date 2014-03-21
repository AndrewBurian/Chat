#include "../Chat.h"
#include "Client.h"

void* Backend(void* params){

    SOCKET ui = ipc[1];
    int isRunning = 1;
    fd_set set;
    SOCKET highSock;
    SOCKET server = 0;


    while(isRunning){

        FD_ZERO(&set);

        FD_SET(ui, &set);
        highSock = ui;

        if(server != 0){
            FD_SET(server, &set);
            highSock = (highSock > server) ? highSock : server;
        }

    }

    return NULL;
}

int	SendMsg(char *message, len_t length){
    return -99;
}

int	chatConnect(){
    return -99;
}
int	chatDisconnect(){
    return -99;
}

int	chatserverDiscover(){
    // Search for the server via broadcast
    SOCKET udpOut;
    SOCKET udpIn;

    int tries = 0;
    int serverCall = SERVER_CALL;
    int echo = 0;
    int received = 0;
    char serverName[MAX_NAME];
    int ret = 0;
    int enable = 1;

    struct sockaddr_in broadcast;
    struct sockaddr_in udpClient;
    struct sockaddr_in server;

    struct timeval rcvTimeout;
    socklen_t serverLen = 0;

    broadcast.sin_family = AF_INET;
    broadcast.sin_port = htons(UDP_PORT);
    broadcast.sin_addr.s_addr = htonl(INADDR_BROADCAST);

    udpClient.sin_family = AF_INET;
    udpClient.sin_port = htons(UDP_PORT);
    udpClient.sin_addr.s_addr = htonl(INADDR_ANY);

    rcvTimeout.tv_sec = 5;
    rcvTimeout.tv_usec = 0;

    udpOut = socket(AF_INET, SOCK_DGRAM, 0);
    udpIn = socket(AF_INET, SOCK_DGRAM, 0);


    // Set all the options and bind like crazy
    if(setsockopt(udpOut, SOL_SOCKET, SO_BROADCAST, &enable, sizeof(enable)) == -1){
        perror("Broadcast Enable Failed");
        return 0;
    }

    if(setsockopt(udpIn, SOL_SOCKET, SO_RCVTIMEO, &rcvTimeout, sizeof(rcvTimeout)) == -1){
        perror("Timeout Set Failed");
        return 0;
    }

    if(setsockopt(udpOut, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) == -1){
        perror("Reuse Addr Failed");
        return 0;
    }

    if(setsockopt(udpIn, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) == -1){
        perror("Reuse Addr Failed");
        return 0;
    }

    if(bind(udpOut, (struct sockaddr*)&udpClient, sizeof(udpClient)) == -1){
        perror("UDP bind failed");
        return 0;
    }

    if(bind(udpIn, (struct sockaddr*)&udpClient, sizeof(udpClient)) == -1){
        perror("UDP Second bind failed");
        return 0;
    }



    do{
        ++tries;
        sendto(udpOut, &serverCall, sizeof(int), 0, (struct sockaddr*)&broadcast, sizeof(broadcast));
        // receive own broadcast packet
        recvfrom(udpOut, &echo, sizeof(echo), 0, (struct sockaddr*)&server, &serverLen);
        recvfrom(udpIn, &echo, sizeof(echo), 0, (struct sockaddr*)&server, &serverLen);

    } while((received = recvfrom(udpIn, serverName, MAX_NAME, 0, (struct sockaddr*)&server, &serverLen)) == -1 && tries < 10);


    if(tries == 10){
        ret = 0;
    }
    else{
        ret = 1;
        memcpy(&serverAddr, &server, serverLen);

    }

    close(udpIn);
    close(udpOut);

    return ret;
}
