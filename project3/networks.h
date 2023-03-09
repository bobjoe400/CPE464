
// 	Writen - HMS April 2017
//  Supports TCP and UDP - both client and server


#ifndef __NETWORKS_H__
#define __NETWORKS_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "gethostbyname.h"

#define LISTEN_BACKLOG 10

typedef struct connection{
    int sk_num;
    struct sockaddr_in6 remote;
    uint32_t len;
}Connection;

int safeRecvfrom(int rcv_sk_num, void * buf, int len, Connection* from);
int safeSendto(void * buf, int len, Connection* to);

// For UDP Server and Client
int safeGetUDPSocket();
int udpServerSetup(int serverPort);
int udpClientSetup(char * hostName, int serverPort, Connection* connection);

#endif
