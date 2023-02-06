/******************************************************************************
* myServer.c
* 
* Writen by Prof. Smith, updated Jan 2023
* Use at your own risk.  
*
*****************************************************************************/
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdint.h>

#include "networks.h"
#include "safeUtil.h"
#include "pollLib.h"
#include "serverUtil.h"

#define MAXBUF 1400
#define DEBUG_FLAG 1

int checkArgs(int argc, char *argv[])
{
	// Checks args and returns port number
	int portNumber = 0;

	if (argc > 2)
	{
		fprintf(stderr, "Usage %s [optional port number]\n", argv[0]);
		exit(-1);
	}
	
	if (argc == 2)
	{
		portNumber = atoi(argv[1]);
	}
	
	return portNumber;
}

void processHandleList(int socket){

}

void sendBroadcast(int socket, uint8_t* message){
	uint8_t senderHandleLen = message++[0];
	uint8_t senderHandle[senderHandleLen+1];
	senderHandle[senderHandleLen] = '\0';
	memcpy(senderHandle, message, senderHandleLen);
	message+=senderHandleLen;
	LinkedList* clientList = getList();
	ListElem* curr = clientList->head;
	while(curr != NULL){
		if(curr->socket != socket){
			uint8_t dHandleLen = strlen(curr->handle);
			buildAndSendMessage(curr->socket, 1, (uint8_t*) curr->handle, dHandleLen, message, senderHandle, senderHandleLen);
		}
		curr = curr->next;
	}
}

void sendError(int socket, uint8_t* handle, uint8_t currHandleLen){
	uint16_t pduLen = C_HDR_SIZE+1+currHandleLen;
	uint8_t pdu[pduLen];
	uint16_t netPDULen = htons(pduLen);
	uint8_t flag = 7;
	memcpy(pdu, &netPDULen, 2);
	memcpy(pdu+2, &flag, 1);
	memcpy(pdu+C_HDR_SIZE, &currHandleLen, 1);
	memcpy(pdu+C_HDR_SIZE+1, handle, currHandleLen);
	sendToSocket(socket, pdu, pduLen);
}

void sendMessage(int socket, uint8_t* buf, uint16_t pduLen){
	uint8_t sendHandleLen = buf++[0];
	uint8_t sendHandle[sendHandleLen];
	memcpy(sendHandle, buf, sendHandleLen);
	buf+=sendHandleLen;
	uint8_t numHandles = buf++[0];
	LinkedList* handles = (LinkedList*) sCalloc(1,sizeof(LinkedList));
	handles->size = 0;
	int handlesLen = 0;
	for(int i = 0; i< numHandles; i++){
		uint8_t currHandleLen = buf++[0];
		uint8_t currHandle[currHandleLen];
		memcpy(currHandle, buf, currHandleLen);
		ListElem* curr;
		if((curr = findInList_h(currHandle, currHandleLen)) == NULL){
			sendError(socket, currHandle, currHandleLen);
			buf+=currHandleLen;
			continue;
		}else{
			addListElem_l(handles, curr->socket, curr->handle);
		}
		buf+=currHandleLen;
		handlesLen+=currHandleLen+1;
	}
	ListElem* curr = handles->head;
	while(curr != NULL){
		uint8_t* handle = (uint8_t*) strdup(curr->handle);
		socket = curr->socket;
		buildAndSendMessage(socket, 1, handle, handlesLen, buf, sendHandle, sendHandleLen);
		curr = curr->next;
		free(handle);
	}
	curr = handles->head;
	while(curr != NULL){
		ListElem* next = curr->next;
		socket = curr->socket;
		listDelete_l(handles, socket);
		curr = next;
	}
	free(handles);
}

void processExit(int socket){
	removeFromPollSet(socket);
	listDelete(socket);
	uint8_t buf[C_HDR_SIZE];
	uint16_t pduLen = htons(C_HDR_SIZE);
	uint8_t flag = 9;
	memcpy(buf, &pduLen, 2);
	memcpy(buf+2, &flag, 1);
	sendToSocket(socket, buf, C_HDR_SIZE);
}

void parseClientResponse(int socket){
	uint8_t buf[C_HDR_SIZE];
	uint16_t pduLen;

	if(safeRecv(socket, buf, C_HDR_SIZE, MSG_WAITALL)<=0){
		listDelete(socket);
		removeFromPollSet(socket);
		close(socket);
		return;
	}
	memcpy(&pduLen, buf, 2);
	pduLen = ntohs(pduLen);

	uint8_t flag = buf[2];
	if(flag == 8){
		processExit(socket);
		return;
	}

	uint8_t rcvBuf[pduLen-C_HDR_SIZE];
	safeRecv(socket, rcvBuf, pduLen-C_HDR_SIZE, MSG_WAITALL);

	switch(flag){
		case 4:
			sendBroadcast(socket, rcvBuf);
			break;
		case 5:
			sendMessage(socket, rcvBuf, pduLen);
			break;
		case 6:
			sendMessage(socket, rcvBuf, pduLen);
			break;
		case 10:
			processHandleList(socket);
			break;
		default:
			printf("Packet Recieve Error\n");
			break;
	}
}

void sendSetupResponse(int socket, uint8_t* buf, char goodOrBad){
	uint16_t pduLen = htons(C_HDR_SIZE);
	uint8_t flag = 3 - goodOrBad;
	memcpy(buf, &pduLen, 2);
	memcpy(buf+2, &flag, 1);
	sendToSocket(socket, buf, C_HDR_SIZE);
}

int addClient(int clientSocket){
	uint8_t buf[C_HDR_SIZE];
	safeRecv(clientSocket, buf, C_HDR_SIZE, MSG_WAITALL);
	if(buf[2] == 1){
		uint8_t handleLen;
		safeRecv(clientSocket, &handleLen, 1, MSG_WAITALL);
		uint8_t handle[handleLen];
		safeRecv(clientSocket, handle, handleLen, MSG_WAITALL);
		if(findInList_h(handle, handleLen) == NULL){
			addListElem(clientSocket, handle, handleLen);
			addToPollSet(clientSocket);
			sendSetupResponse(clientSocket, buf, 1);
			return 1;
		}
		sendSetupResponse(clientSocket, buf, 0);
		return -1;
	}
	return -1;
}

void serverLoop(int socketNum){
	int socket = 0;

	initLinkedList();

	while(1){
		socket = pollCall(-1);
		if(socket == socketNum){
			int clientSocket = tcpAccept(socket, DEBUG_FLAG);
			if(addClient(clientSocket)<0){
				close(clientSocket);
			}
		}else{
			parseClientResponse(socket);
		}
	}
}

int main(int argc, char *argv[])
{
	int mainServerSocket = 0;   //socket descriptor for the server socket
	int portNumber = 0;
	
	portNumber = checkArgs(argc, argv);
	
	//create the server socket
	mainServerSocket = tcpServerSetup(portNumber);

	setupPollSet();
	addToPollSet(mainServerSocket);

	serverLoop(mainServerSocket);
	
	/* close the sockets */
	close(mainServerSocket);

	
	return 0;
}

