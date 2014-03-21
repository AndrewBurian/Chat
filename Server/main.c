
#include "../Chat.h"

void acceptNew(SOCKET* listenSock);
void removeClient(int pos);
void declareServer(SOCKET* sock);
int handleMessage(int pos);
void chat(int pos);
void clearStream(int pos);
void removeClient(int pos);
int readSock(SOCKET socket, void* buffer, int size);

int currentClients;
SOCKET clients[MAX_CLIENTS];
char* serverName;
uint32_t myNameLen;


int main(int argc, char* argv[]){

    SOCKET udp;
    SOCKET tcp;

    struct sockaddr_in server;

    fd_set fdset;
	int numLiveSockets;
	SOCKET highSocket;

	int i;

    currentClients = 0;
    memset(clients, 0, sizeof(SOCKET) * MAX_CLIENTS);

    serverName = argv[1];
    myNameLen = strlen(serverName);

    server.sin_family = AF_INET;
    server.sin_port = htons(TCP_PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    tcp = socket(AF_INET, SOCK_STREAM, 0);
    udp = socket(AF_INET, SOCK_DGRAM, 0);

    if(bind(tcp, (struct sockaddr*)&server, sizeof(server)) == -1){
        perror("TCP bind");
        exit(1);
    }

    server.sin_port = htons(UDP_PORT);

    if(bind(udp, (struct sockaddr*)&server, sizeof(server)) == -1){
        perror("UDP bind");
        exit(1);
    }

    listen(tcp, 5);

    printf("Server name: %s\nOnline.\n", serverName);

    while(1){

        FD_ZERO(&fdset);

        FD_SET(tcp, &fdset);
        FD_SET(udp, &fdset);
        highSocket = (tcp > udp) ? tcp : udp;

        for(i = 0; i < MAX_CLIENTS; ++i){
            if(clients[i] != 0){
                FD_SET(clients[i], &fdset);
                highSocket = (clients[i] > highSocket) ? clients[i] : highSocket;
            }
        }

        numLiveSockets = select(highSocket + 1, &fdset, NULL, NULL, NULL);

        if(numLiveSockets == -1){
            perror("Select Failed");
            exit(2);
        }


        if(FD_ISSET(tcp, &fdset)){
            if(currentClients < MAX_CLIENTS){
                acceptNew(&tcp);
            }
        }

        if(FD_ISSET(udp, &fdset)){
            declareServer(&udp);
        }

        for(i = 0; i < MAX_CLIENTS; ++i){
            if(clients[i] != 0){
                if(FD_ISSET(clients[i], &fdset)){
                    handleMessage(i);
                }
            }
        }

    }


    return 0;
}

void acceptNew(SOCKET* listenSock){
    int pos;
    int i;
    SOCKET newClient;
    char protoCtl;
    char type;
    void* name;
    uint32_t namelen;

    newClient = accept(*listenSock, NULL, NULL);

    for(i = 0; i < MAX_CLIENTS; ++i){
        if(clients[i] != 0){
            break;
        }
    }
    pos = i;
    clients[pos] = newClient;

    // Send Server name message
    protoCtl = SYN;
    type = 1;
    send(newClient, &protoCtl, sizeof(char), 0);
    send(newClient, &type, sizeof(char), 0);
    send(newClient, &myNameLen, sizeof(uint32_t), 0);
    send(newClient, serverName, myNameLen, 0);
    protoCtl = EOT;
    send(newClient, &protoCtl, sizeof(char), 0);

    // Get Client name message
    readSock(newClient, &protoCtl, sizeof(uint32_t));
    if(protoCtl != SYN){
        fprintf(stderr, "Malformed protocol stream new client start\n");
        fprintf(stderr, "Client is non-confroming. Rejecting.\n");
        close(newClient);
        return;
    }

    readSock(newClient, &type, sizeof(char));
    if(type != 1){
        fprintf(stderr, "Out of sequence data received in new client\n");
        fprintf(stderr, "Client is non-confroming. Rejecting.\n");
        close(newClient);
        return;
    }

    readSock(newClient, &namelen, sizeof(uint32_t));
    name = malloc(namelen);
    readSock(newClient, name, namelen);
    readSock(newClient, &protoCtl, sizeof(char));

    if(protoCtl != EOT){
        fprintf(stderr, "Malformed protocol stream new client termination\n");
    }


    printf("Added new client: %s\n", (char*)name);
    ++currentClients;


    // Send new client messages
    type = 2;
    for(i = 0; i < MAX_CLIENTS; ++i){
        if(clients[i] && i != pos){
            protoCtl = SYN;
            send(clients[i], &protoCtl, sizeof(char), 0);
            send(clients[i], &type, sizeof(char), 0);
            send(clients[i], &namelen, sizeof(uint32_t), 0);
            send(clients[i], name, namelen, 0);
            protoCtl = EOT;
            send(clients[i], &protoCtl, sizeof(char), 0);
        }
    }
    free(name);
}

void declareServer(SOCKET* sock){
    struct sockaddr_in client;
    socklen_t len;
    int call;

    recvfrom(*sock, &call, sizeof(int), 0, (struct sockaddr*)&client, &len);

    if(call != SERVER_CALL){
        return;
    }

    sendto(*sock, serverName, MAX_NAME, 0, (struct sockaddr*)&client, len);

}

int handleMessage(int pos){

    char protocolCtl = 0;
    char type = 0;

    readSock(clients[pos], &protocolCtl, sizeof(char));

    if(protocolCtl != SYN){
        fprintf(stderr, "Malformed protocol stream start\n");
        return -1;
    }

    readSock(clients[pos], &type, sizeof(char));

    switch(type){
        case 3:
            chat(pos);
            break;
        case 4:
            removeClient(pos);
            currentClients--;
            break;
        default:
            clearStream(pos);
            printf("Unknown Packet type %d discarded\n", (int)type);
            break;
    }

    return 0;
}

void removeClient(int pos){
    char type = 4;
    char protoCtl = 0;
    close(clients[pos]);
    clients[pos] = 0;
    int i;

    uint32_t namelen = 0;
    void* name;

    readSock(clients[pos], &namelen, sizeof(uint32_t));
    name = malloc(namelen);
    readSock(clients[pos], name, namelen);
    readSock(clients[pos], &protoCtl, sizeof(char));
    if(protoCtl != EOT){
        fprintf(stderr, "Malformed protocol stream termination\n");
    }

    for(i = 0; i < MAX_CLIENTS; ++i){
        if(clients[i] != 0 && i != pos){
            protoCtl = SYN;
            send(clients[i], &protoCtl, sizeof(char), 0);
            send(clients[i], &type, sizeof(char), 0);
            send(clients[i], &namelen, sizeof(uint32_t), 0);
            send(clients[i], name, namelen, 0);
            protoCtl = EOT;
            send(clients[i], &protoCtl, sizeof(char), 0);
        }
    }

    free(name);
}

void chat(int pos){

    char protocolCtl = 0;
    uint32_t msglen = 0;
    uint32_t namelen = 0;
    void* name;
    void* message;
    int i;
    char type = 3;


    readSock(clients[pos], &namelen, sizeof(uint32_t));

    name = malloc(namelen);

    readSock(clients[pos], name, namelen);

    readSock(clients[pos], &msglen, sizeof(uint32_t));

    message = malloc(msglen);

    readSock(clients[pos], message, msglen);

    readSock(clients[pos], &protocolCtl, sizeof(char));

    if(protocolCtl != EOT){
        fprintf(stderr, "Malformed protocol stream termination\n");
    }

    for(i = 0; i < MAX_CLIENTS; ++i){
        if(clients[i] != 0 && i != pos){
            protocolCtl = SYN;
            send(clients[i], &protocolCtl, sizeof(char), 0);
            send(clients[i], &type, sizeof(char), 0);
            send(clients[i], &namelen, sizeof(uint32_t), 0);
            send(clients[i], name, namelen, 0);
            send(clients[i], &msglen, sizeof(uint32_t), 0);
            send(clients[i], message, msglen, 0);
            protocolCtl = EOT;
            send(clients[i], &protocolCtl, sizeof(char), 0);
        }
    }

    free(message);
    free(name);

    return;
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

void clearStream(int pos){
    char dat;
    do{
        readSock(clients[pos], &dat, sizeof(char));
    }while(dat != EOT);
}
