#include <stdio.h>
#include <stdlib.h>
#include "../Chat.h"

#define BUFFSIZE 2048

int readSock(SOCKET socket, void* buffer, int size);
int serverDiscover(struct sockaddr_in* serverAddr);

int main(int argc, char* argv[])
{

    SOCKET server;
    struct sockaddr_in serverAddr, clientAddr;
    int enable = 1;
    uint32_t myNameLen;
    char syn = SYN;
    char eot = EOT;
    char type;
    char* recvName;
    uint32_t recvNameLen;
    char* recvMsg;
    uint32_t recvMsgLen;
    int typeSize = sizeof(char);
    int nameSize = sizeof(uint32_t);
    fd_set set;
    char* outBuff;
    size_t no = 0;
    uint32_t outLen = 0;


    if(argc < 2){
        printf("Usage: %s <name>\n", argv[0]);
        exit(0);
    }

    myNameLen = strlen(argv[1]) + 1;

    printf("Searching for server...\n");
    if(!serverDiscover(&serverAddr)){
        printf("Failed to find server\n");
        exit(0);
    }
    printf("Server found\n");

    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = 0;
    clientAddr.sin_addr.s_addr = 0;

    if((server = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("Could not create socket");
        exit(2);
    }

    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));

    if(bind(server, (struct sockaddr*)&clientAddr, sizeof(clientAddr)) == -1){
        perror("Could not bind socket");
        exit(3);
    }

    if(connect(server, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1){
        perror("Could not connect");
        exit(4);
    }

    // Syn
    readSock(server, &type, typeSize);
    //type
    readSock(server, &type, typeSize);
    // server name len
    readSock(server, &recvNameLen, nameSize);
    recvName = malloc(recvNameLen);
    // server name
    readSock(server, recvName, recvNameLen);
    recvName[recvNameLen-1] = 0;
    // eot
    readSock(server, &type, typeSize);


    printf("Connecting to %s\n", recvName);
    free(recvName);

    type = 2;
    send(server, &syn, typeSize, 0);
    send(server, &type, typeSize, 0);
    send(server, &myNameLen, nameSize, 0);
    send(server, argv[1], myNameLen, 0);
    send(server, &eot, typeSize, 0);

    printf("Connected\n");

    while(1){

        FD_ZERO(&set);
        FD_SET(0, &set);
        FD_SET(server, &set);
        select(server + 1, &set, NULL, NULL, NULL);
        if(FD_ISSET(0, &set)){
            if(getline(&outBuff, &no, stdin)){
                outLen = strlen(outBuff) + 1;
                outBuff[outLen - 2] = 0;
                type = 3;
                send(server, &syn, typeSize, 0);
                send(server, &type, typeSize, 0);
                send(server, &myNameLen, nameSize, 0);
                send(server, argv[1], myNameLen, 0);
                send(server, &outLen, nameSize, 0);
                send(server, outBuff, outLen, 0);
                send(server, &eot, typeSize, 0);
                bzero(outBuff, BUFFSIZE);
            }
        }
        if(FD_ISSET(server, &set)){
            readSock(server, &type, typeSize);
            readSock(server, &type, typeSize);
            switch(type){
                case 3:
                    readSock(server, &recvNameLen, nameSize);
                    recvName = malloc(recvNameLen);
                    readSock(server, recvName, recvNameLen);
                    readSock(server, &recvMsgLen, nameSize);
                    recvMsg = malloc(recvMsgLen);
                    readSock(server, recvMsg, recvMsgLen);
                    readSock(server, &type, typeSize);
                    recvName[recvNameLen-1] = 0;
                    recvMsg[recvMsgLen-1] = 0;
                    printf("%s: \"%s\"\n", recvName, recvMsg);
                    free(recvName);
                    free(recvMsg);
                    break;
                case 4:
                    printf("Lost ");
                case 2:
                    readSock(server, &recvNameLen, nameSize);
                    recvName = malloc(recvNameLen);
                    readSock(server, recvName, recvNameLen);
                    readSock(server, &type, typeSize);
                    recvName[recvNameLen-1] = 0;
                    printf("Client: %s\n", recvName);
                    free(recvName);
                    break;
                default:
                    do{
                        readSock(server, &type, typeSize);
                    }while(type != EOT);
            }
        }
    }
    return 0;
}

int serverDiscover(struct sockaddr_in* serverAddr){
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

    serverAddr->sin_family = AF_INET;

    do{
        ++tries;
        sendto(udpOut, &serverCall, sizeof(int), 0, (struct sockaddr*)&broadcast, sizeof(broadcast));
        // receive own broadcast packet
        recvfrom(udpOut, &echo, sizeof(echo), 0, (struct sockaddr*)serverAddr, &serverLen);
        recvfrom(udpIn, &echo, sizeof(echo), 0, (struct sockaddr*)serverAddr, &serverLen);

    } while((received = recvfrom(udpIn, serverName, MAX_NAME, 0, (struct sockaddr*)serverAddr, &serverLen)) == -1 && tries < 10);


    if(tries == 10){
        ret = 0;
    }
    else{
        ret = 1;

    }

    serverAddr->sin_port = htons(TCP_PORT);

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
			return -1;
		}

		totalRead += thisRead;
		toRead -= thisRead;
	}

	return totalRead;
}
