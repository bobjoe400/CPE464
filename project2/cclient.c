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

#define DEBUG_FLAG 1

void parseHandleList(uint8_t* buf){
	removeFromPollSet(1);
	uint32_t numHandles = get_long(&buf, 1);
	printf("Number of handles: %i", numHandles);
}

void parseMessage(uint8_t* buf, uint8_t cHandleLen){
	uint8_t handleLen = buf++[0];
	char sendHandle[handleLen+1];
	sendHandle[handleLen] = '\0';
	memcpy(sendHandle, buf, handleLen);
	printf("\n%s: ", sendHandle);
	buf+=1+handleLen+1+cHandleLen;
	printf("%s\n", buf);
}

void parseHandle(uint8_t* buf, char error){
	uint8_t handleLen = buf++[0];
	char handle[handleLen+1];
	handle[handleLen] = '\0';
	memcpy(handle, buf, handleLen);
	if(error){
		printf("\nClient with handle %s does not exist\n", handle);
	}else{
		printf("\t\n%s", handle);
	}
}

int parseSetup(uint8_t* buf){
	if(buf[2] == 3){
		return -1;
	}
	return 1;
}

int parseIncoming(int socketNum, uint8_t* outputbuf, uint8_t cHandleLen){
	int pduLen = get_short(&outputbuf, 1);
	uint8_t flag = outputbuf++[0];
	uint8_t rcvbuf[pduLen - C_HDR_SIZE];
	if(flag != 9){
		safeRecv(socketNum, rcvbuf, pduLen-C_HDR_SIZE, MSG_WAITALL);
	}
	switch(flag){
		case 5:
			parseMessage(rcvbuf, cHandleLen);
			break;
		case 7:
			parseHandle(rcvbuf, 1);
			break;
		case 9:
			return -1;
		case 11:
			parseHandleList(rcvbuf);
			break;
		case 12:
			parseHandle(rcvbuf, 0);
			break;
		case 13:
			addToPollSet(1);
			break;
		default:
			printf("Packet parsing error. Flag recieved: %i\n", flag);
			break;
	}
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

void sendMessage(int socketNum, uint8_t* buf, uint8_t* cHandle, uint8_t cHandleLen){
	uint8_t* handlesBuf = buf;
	uint8_t dHandleLen = 0;
	while((char)buf[0] != '\0' && buf[0] != ' ' && dHandleLen <= HANDLEMAX){
		dHandleLen++;
		buf++;
	}
	buf++;

	buildAndSendMessage(socketNum, 1, handlesBuf, dHandleLen+1, buf, cHandle, cHandleLen);
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
		int dHandleLen = 0;
		if(temp[0] == '\0'){
			break;
		}
		while(temp[0] != ' ' && temp[0] != '\0' && dHandleLen <= HANDLEMAX){
			dHandleLen++;
			temp++;
		}

		dHandlesPDULen+=dHandleLen+1;
		handlesRead++;
		buf+=dHandleLen+1;
	}

	if(handlesRead != numHandles){
		printf("Please enter the number of handles you requested. Number requested: %i\n", numHandles);
		return -1;
	}

	buildAndSendMessage(socketNum, handlesRead, handlesBuf, dHandlesPDULen, buf, cHandle, cHandleLen);
	return 1;
}

void sendList(int socketNum){
	uint16_t netPDULen = htons(C_HDR_SIZE);
	uint8_t listPDU[C_HDR_SIZE];
	uint8_t flag = 10;
	memcpy(listPDU, &netPDULen, 2);
	memcpy(listPDU+2, &flag, 1);
	sendToSocket(socketNum, listPDU, (uint16_t) C_HDR_SIZE);
}

int sendExit(int socketNum){
	uint8_t exitPDU[C_HDR_SIZE];
	uint16_t netPDULen = htons((uint16_t) 3);
	uint8_t flag = 8;
	memcpy(exitPDU, &netPDULen, 2);
	memcpy(exitPDU, &flag, 1);
	sendToSocket(socketNum, exitPDU, C_HDR_SIZE);
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
	sendToSocket(socketNum, pdubuf, ntohs(pdulen));
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
			sendMessage(socketNum, inputbuf, cHandle, cHandleLen);
			break;
		case 'M':
			sendMessage(socketNum, inputbuf, cHandle, cHandleLen);
			break;
		case 'b':
			buildAndSendMessage(socketNum, 0, (uint8_t *)NULL, 0, inputbuf, cHandle, cHandleLen);
			break;
		case 'B':
			buildAndSendMessage(socketNum, 0, (uint8_t *)NULL, 0, inputbuf, cHandle, cHandleLen);
			break;
		case 'c':
			return sendMultiCast(socketNum, inputbuf, cHandle, cHandleLen);
		case 'C':
			return sendMultiCast(socketNum, inputbuf, cHandle, cHandleLen);
		case 'l':
			sendList(socketNum);
			break;
		case 'L':
			sendList(socketNum);
			break;
		case 'e':
			return sendExit(socketNum);
		case 'E':
			return sendExit(socketNum);
		default:
			return -1;
	}
	return 1;
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
		printf("\nError: Handle already exits: %s\n", handle);
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
			if(safeRecv(socket, rcvbuf, C_HDR_SIZE, MSG_WAITALL)<=0){
				printf("\nServer Terminated\n");
				break;
			}else{
				if(parseIncoming(socketNum, rcvbuf, cHandleLen) < 0){
					printf("\nExiting...\n");
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