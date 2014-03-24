#include <stdio.h>
#include <stdlib.h>
#include "../Chat.h"

#define BUFFSIZE 2048

int readSock(SOCKET socket, void* buffer, int size);

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


    if(argc < 4){
        printf("Usage: %s <name> <address> <port>", argv[0]);
        exit(1);
    }

    myNameLen = strlen(argv[1]) + 1;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(argv[3]));
    serverAddr.sin_addr.s_addr = inet_addr(argv[2]);

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
