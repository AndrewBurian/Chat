#include "../Chat.h"
#include "Client.h"

int readSock(SOCKET socket, void* buffer, int size);
void clearStream(SOCKET socket);

SOCKET server = 0;

void* Backend(void* params){

    SOCKET ui = ipc[1];
    int isRunning = 1;
    fd_set set;
    SOCKET highSock;

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
    ctl_t protoCtl = SYN;
    char type = 0;
    struct sockaddr_in client;

    client.sin_family = AF_INET;
    client.sin_port = htons(TCP_PORT);
    client.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(server, (struct sockaddr*)&client, sizeof(client)) == -1){
        return 0;
    }

    if(connect(server, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1){
        return 0;
    }

    // Ensure the name is single null terminated
    if(myName[myNameLen] != 0){
        // if there's space, just add the null and increase the size
        if(myNameLen < MAX_NAME){
            myNameLen++;
            myName[myNameLen] = 0;
        }
        // too bad, truncating
        else{
            myName[MAX_NAME] = 0;
        }
    }

    // Receive Server Name Message
    readSock(server, &protoCtl, sizeof(ctl_t));
    if(protoCtl != SYN){
        // joined an non-conforming server, leave
        close(server);
        server = 0;
        return 0;
    }
    readSock(server, &type, sizeof(char));
    readSock(server, &serverNameLen, sizeof(len_t));
    serverName = malloc(serverNameLen);
    readSock(server, serverName, serverNameLen);
    readSock(server, &protoCtl, sizeof(ctl_t));

    if(type != 1 || protoCtl != EOT){
        // joined an non-conforming server, leave
        chatDisconnect();
        return 0;
    }


    // Send client join packet
    protoCtl = SYN;
    send(server, &protoCtl, sizeof(ctl_t), 0);
    type = 2; // Connect;
    send(server, &type, sizeof(char), 0);
    send(server, &myNameLen, sizeof(len_t), 0);
    send(server, myName, myNameLen, 0);
    protoCtl = EOT;
    send(server, &protoCtl, sizeof(ctl_t), 0);

    return 1;

}

int	chatDisconnect(){
    int i;

    ctl_t protoCtl = SYN;
    char type = 4; // Disconnect

    // Sever not connected. Call that a successful disconnect
    if(!server){
        return 1;
    }

    // Send client lost packet
    send(server, &protoCtl, sizeof(ctl_t), 0);
    send(server, &type, sizeof(char), 0);
    send(server, &myNameLen, sizeof(len_t), 0);
    send(server, myName, myNameLen, 0);
    protoCtl = EOT;
    send(server, &protoCtl, sizeof(ctl_t), 0);

    // Disconnect
    close(server);
    server = 0;
    free(serverName);
    serverName = 0;
    serverNameLen = 0;

    for(i = 0; i < MAX_CLIENTS; ++i){
        if(clientNames[i]){
            free(clientNames[i]);
            clientNames[i] = 0;
        }
        clientRooms[i] = 0;
    }

    return 1;
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



int readSock(SOCKET socket, void* buffer, int size){
    int toRead = size;
	int thisRead;
	int totalRead = 0;

	while (toRead > 0) {

		thisRead = read(socket, buffer + totalRead, toRead - totalRead);

		if (thisRead <= 0) {
			continue;
		}

		totalRead += thisRead;
		toRead -= thisRead;
	}

	return totalRead;
}

void clearStream(SOCKET socket){
    char dat;
    do{
        readSock(socket, &dat, sizeof(char));
    }while(dat != EOT);
}
