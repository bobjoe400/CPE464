/******************************************************************************
* myClient.c
*
* Writen by Prof. Smith, updated Jan 2023
* Use at your own risk.  
*
*****************************************************************************/

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

#define INPUTMAX 1400
#define HANDLEMAX 100
#define MSGMAXLEN 199
#define C_HDR_SIZE 3
#define DEBUG_FLAG 1
#define RCVMAX C_HDR_SIZE+1+HANDLEMAX+2+HANDLEMAX+MSGMAXLEN
#define ASCII_NUM_OFFSET 48

int parseIncoming(uint8_t* outputbuf){
	return 1;
}

int readFromStdin(uint8_t * buffer)
{
	char aChar = 0;
	int inputLen = 0;        
	
	// Important you don't input more characters than you have space 
	buffer[0] = '\0';
	while (inputLen < (INPUTMAX) && aChar != '\n')
	{
		aChar = getchar();
		if(aChar != '\n'){
			buffer[inputLen] = aChar;
			inputLen++;
		}
	}
	
	// Null terminate the string
	buffer[inputLen] = '\0';
	inputLen++;
	
	return inputLen;
}

void sendToServer(int socketNum, uint8_t* pdubuf, uint16_t pdulen){
	int sent = 0;
	sent =  safeSend(socketNum, pdubuf, pdulen, 0);
	if (sent < 0)
	{
		perror("send call");
		exit(-1);
	}

	printf("Amount of data sent is: %d\n", sent);
}

void buildMessagePDU(uint8_t* dHandle, uint8_t dHandleLen, uint8_t* cHandle, uint8_t cHandleLen, uint8_t* message, uint8_t messageLen, uint8_t* msgPDU, uint16_t pduLen){
	uint16_t netPDULen = htons(pduLen);
	uint8_t flag = 5;
	uint8_t handleNum = 1;
	memcpy(msgPDU, &netPDULen, 2);
	memcpy(msgPDU+2, &flag, 1);
	memcpy(msgPDU+C_HDR_SIZE, &cHandleLen, 1);
	memcpy(msgPDU+C_HDR_SIZE+1, cHandle, cHandleLen);
	memcpy(msgPDU+C_HDR_SIZE+1+cHandleLen, &handleNum, 1);
	memcpy(msgPDU+C_HDR_SIZE+1+cHandleLen+1, &dHandleLen, 1);
	memcpy(msgPDU+C_HDR_SIZE+1+cHandleLen+1+1, dHandle, dHandleLen);
	memcpy(msgPDU+C_HDR_SIZE+1+cHandleLen+1+1+dHandleLen, message, messageLen+1);
} 

int sendMessage(int socketNum, uint8_t* buf, uint8_t* cHandle, uint8_t cHandleLen){
	uint8_t* message = buf;
	uint8_t dHandleLen = 0;
	while((char)buf[0] != '\0' && buf[0] != ' '){
		dHandleLen++;
		buf++;
	}
	uint8_t dHandle[dHandleLen];
	memcpy(dHandle, message, dHandleLen);
	message+=dHandleLen+1;
	int messageLen = strlen((char*) message);
	int messageCount = messageLen/MSGMAXLEN;
	if(messageLen%MSGMAXLEN > 0){
		messageCount++;
	}else if(messageLen == 0){
		messageCount++;
	}
	for(int i = 0; i < messageCount; i++){
		uint8_t pduMessageLen = (messageLen>199) ? 199 : messageLen;
		uint8_t pduMessage[pduMessageLen+1];

		uint16_t pduLen = C_HDR_SIZE+1+cHandleLen+1+1+dHandleLen+pduMessageLen+1;
		uint8_t msgPDU[pduLen];
		
		pduMessage[pduMessageLen] = '\0';
		memcpy(pduMessage, message, pduMessageLen);
		message+=pduMessageLen;
		messageLen-=pduMessageLen;

		buildMessagePDU(dHandle, dHandleLen, cHandle, cHandleLen, pduMessage, pduMessageLen, msgPDU, pduLen);

		sendToServer(socketNum, msgPDU, pduLen);
	}
	return 1;
}

void buildMultiCastPDU(uint8_t* handlesBuf, int dHandlesPDULen, uint8_t numHandles, uint8_t* cHandle, uint8_t cHandleLen, uint8_t* message, uint8_t messageLen,  uint8_t* multiPDU, uint16_t multiPDULen){
	uint16_t netPDULen = htons(multiPDULen);
	uint8_t flag = 6;
	
	memcpy(multiPDU, &netPDULen, 2);
	memcpy(multiPDU+2, &flag, 1);
	memcpy(multiPDU+C_HDR_SIZE, &cHandleLen, 1);
	memcpy(multiPDU+C_HDR_SIZE+1, cHandle, cHandleLen);
	memcpy(multiPDU+C_HDR_SIZE+1+cHandleLen, &numHandles, 1);

	uint8_t* handles = multiPDU+C_HDR_SIZE+1+cHandleLen+1;

	for(int i = 0; i<numHandles; i++){
		
		uint8_t* temp = handlesBuf;
		uint8_t count = 0;
		while(temp[0] != ' ' && temp[0] != '\0'){
			count++;
			temp++;
		}

		memcpy(handles++, &count, 1);
		memcpy(handles, handlesBuf, count);

		handles+=count;
		handlesBuf+=count+1;
	}

	memcpy(multiPDU+C_HDR_SIZE+1+cHandleLen+1+dHandlesPDULen, message, messageLen+1);
}

int sendMultiCast(int socketNum, uint8_t* buf, uint8_t* cHandle, uint8_t cHandleLen){
	uint8_t numHandles = 0;
	memcpy(&numHandles, buf++, 1);
	numHandles -= ASCII_NUM_OFFSET;
	if(numHandles<2 || numHandles>9){
		printf("Please enter a number of handles from 2 to 9\n");
		return -1;
	}
	if(buf++[0] != ' '){
		return -1;
	}

	uint8_t* handlesBuf = buf;

	int handlesRead = 0;
	int dHandlesPDULen = 0;
	for(int i = 0; i<numHandles; i++){
		
		uint8_t* temp = buf;
		int count = 0;
		if(temp[0] == '\0'){
			break;
		}
		while(temp[0] != ' ' && temp[0] != '\0'){
			count++;
			temp++;
		}

		dHandlesPDULen+=count+1;
		handlesRead++;
		buf+=count+1;
	}

	if(handlesRead != numHandles){
		printf("Please enter the number of handles you requested. Number requested: %i\n", numHandles);
		return -1;
	}

	int messageLen = strlen((char*) buf);
	int messageCount = messageLen/MSGMAXLEN;
	if(messageLen%MSGMAXLEN > 0){
		messageCount++;
	}else if(messageLen == 0){
		messageCount++;
	}

	for(int i = 0; i < messageCount; i++){
		uint8_t pduMessageLen = (messageLen>199) ? 199 : messageLen;
		uint8_t pduMessage[pduMessageLen+1];

		uint16_t multiPDULen = C_HDR_SIZE+1+cHandleLen+1+dHandlesPDULen+pduMessageLen+1;
		
		uint8_t multiPDU[multiPDULen];
		pduMessage[pduMessageLen] = '\0';
		memcpy(pduMessage, buf, pduMessageLen);
		buf+=pduMessageLen;
		messageLen-=pduMessageLen;

		buildMultiCastPDU(handlesBuf, dHandlesPDULen, numHandles, cHandle, cHandleLen, pduMessage, pduMessageLen, multiPDU, multiPDULen);

		sendToServer(socketNum, multiPDU, multiPDULen);
	}
	return 1;
}

int sendExit(int socketNum){
	uint8_t exitPDU[C_HDR_SIZE];
	uint16_t netPDULen = htons((uint16_t) 3);
	uint8_t flag = 8;
	memcpy(exitPDU, &netPDULen, 2);
	memcpy(exitPDU, &flag, 1);
	sendToServer(socketNum, exitPDU, C_HDR_SIZE);
	return 1;
}

void sendSetup(int socketNum, char* handle, uint8_t cHandleLen){
	uint8_t pdubuf[4+cHandleLen];
	uint16_t pdulen = htons(4+(uint16_t)cHandleLen);
	uint8_t flag = 1;
	memcpy(pdubuf, &pdulen, 2);
	memcpy(pdubuf+2, &flag, 1);
	memcpy(pdubuf+C_HDR_SIZE, &cHandleLen, 1);
	memcpy(pdubuf+C_HDR_SIZE+1, handle, cHandleLen);
	sendToServer(socketNum, pdubuf, ntohs(pdulen));
}

int parseSetup(uint8_t* outputbuf){
	if(outputbuf[2] == 3){
		return -1;
	}
	return 1;
}

int parseInput(int socketNum, uint8_t* inputbuf, uint8_t* cHandle, int cHandleLen){
	if(strlen((char*) inputbuf) < 2){
		return -1;
	}
	if(inputbuf++[0] != '%'){
		return -1;
	}

	uint8_t command = 0;
	memcpy(&command, inputbuf++, 1);

	if(inputbuf[0] != ' ' && inputbuf[0] != '\0'){
		return -1;
	}

	inputbuf++;
	switch(command){
		case 'm':
			return sendMessage(socketNum, inputbuf, cHandle, cHandleLen);
		case 'M':
			return sendMessage(socketNum, inputbuf, cHandle, cHandleLen);
		case 'b':
			break;
		case 'B':
			break;
		case 'c':
			return sendMultiCast(socketNum, inputbuf, cHandle, cHandleLen);
		case 'C':
			return sendMultiCast(socketNum, inputbuf, cHandle, cHandleLen);
		case 'l':
			break;
		case 'L':
			break;
		case 'e':
			return sendExit(socketNum);
		case 'E':
			return sendExit(socketNum);
		default:
			return -1;
	}
	return -1;
}

void clientLoop(int socketNum, char* handle){
	uint8_t cHandleLen = (uint8_t) strlen(handle);
	uint8_t inputbuf[INPUTMAX];   //data buffer
	uint8_t rcvbuf[RCVMAX];
	
	int socket = 0;

	sendSetup(socketNum, handle, cHandleLen);
	socket = pollCall(-1);
	safeRecv(socketNum, rcvbuf, RCVMAX, MSG_DONTWAIT);
	if(parseSetup(rcvbuf) < 0){
		printf("\nError: Handle already exits\n");
		return;
	}

	addToPollSet(1);
	while(1){
		printf("$:");
		fflush(stdout);
		socket = pollCall(-1);
		if(socket == 1){
			memset(inputbuf, 0, INPUTMAX);
			int amtRead = readFromStdin(inputbuf);
			printf("read: %s string len: %d (including null)\n", inputbuf, amtRead);
			int retNum = 0;
			if((retNum = parseInput(socketNum, inputbuf, (uint8_t*) handle, cHandleLen)) < 0){
				printf("Please enter a valid command\n");
			}
		}else{
			memset(rcvbuf, 0, RCVMAX);
			if(safeRecv(socket, rcvbuf, RCVMAX, MSG_DONTWAIT)<=0){
				printf("\nLost Connect to Server\n");
				break;
			}else{
				if(parseIncoming(rcvbuf) < 0){
					printf("\nExiting...");
					break;
				}
			}
		}
	}

}

void checkArgs(int argc, char * argv[])
{
	/* check command line arguments  */
	if (argc != 4)
	{
		printf("usage: %s handle host-name port-number\n", argv[0]);
		exit(1);
	}
	if((strlen(argv[1]))>100){
		printf("client handle must not be longer than 100 characters\n");
		exit(1);
	}
}


int main(int argc, char * argv[])
{
	int socketNum = 0;         //socket descriptor
	
	checkArgs(argc, argv);

	/* set up the TCP Client socket  */
	socketNum = tcpClientSetup(argv[2], argv[3], DEBUG_FLAG);

	setupPollSet();
	addToPollSet(socketNum);

	clientLoop(socketNum, argv[1]);
	
	close(socketNum);
	
	return 0;
}