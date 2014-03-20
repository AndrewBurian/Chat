#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#define SYN 	0x16
#define EOT 	0x04
#define MSG 	0x03
#define NEW_CLINET	0x02
#define SERVER_NAME	0x01
#define CLIENT_LOST	0x04

#define TCP_PORT 43333
#define UDP_PORT 43334

#define MAX_CLIENTS     20
#define MAX_NAME        20
#define MAX_MESSAGE     50
#define SERVER_CALL	4981

typedef int 		SOCKET;
typedef char 		ctl_t;
typedef uint32_t	len_t;
typedef int		roomNo_t;
typedef int		clientNo_t;
