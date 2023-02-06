
// 
// Writen by Hugh Smith, April 2020
//
// Put in system calls with error checking
// and and an s to the name: srealloc()
// keep the function paramaters same as system call

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
#include <errno.h>

#include "networks.h"
#include "safeUtil.h"

uint16_t get_short(uint8_t** packet, uint8_t conv){
    uint16_t t_short;
    memcpy(&t_short, *packet, SHORT_BYTES);
    *packet = *packet+SHORT_BYTES;
    if(conv) return ntohs(t_short);
    return t_short;
}

uint32_t get_long(uint8_t** packet, uint8_t conv){
    uint32_t t_long;
    memcpy(&t_long, *packet, LONG_BYTES);
    *packet = *packet+LONG_BYTES;
    if(conv) return ntohl(t_long);
    return t_long;
}

int getMessageCount(char* buf, int messageLen){
    int messageCount = messageLen/MSGMAXLEN;
    if(messageLen%MSGMAXLEN > 0){
        messageCount++;
    }else if(messageLen == 0){
        messageCount++;
    }
    return messageCount;
}

void sendToSocket(int socketNum, uint8_t* pdubuf, uint16_t pdulen){
	int sent = 0;
	sent =  safeSend(socketNum, pdubuf, pdulen, 0);
	if (sent < 0)
	{
		perror("send call");
		exit(-1);
	}

	printf("Amount of data sent is: %d\n", sent);
}

void buildMessagePDU(uint8_t* handlesBuf, uint8_t numHandles, uint8_t* cHandle, uint8_t cHandleLen, uint8_t* message, int messageLen, uint8_t* messagePDU, uint16_t messagePDULen){
	uint16_t netPDULen = htons(messagePDULen);
	uint8_t flag = 6;
	
	memcpy(messagePDU, &netPDULen, 2);
	memcpy(messagePDU+2, &flag, 1);
	memcpy(messagePDU+C_HDR_SIZE, &cHandleLen, 1);
	memcpy(messagePDU+C_HDR_SIZE+1, cHandle, cHandleLen);
	memcpy(messagePDU+C_HDR_SIZE+1+cHandleLen, &numHandles, 1);

	uint8_t* handles = messagePDU+C_HDR_SIZE+1+cHandleLen+1;

    if(numHandles == 0){
        memcpy(messagePDU+C_HDR_SIZE+1+cHandleLen, message, messageLen+1);
        return;
    }

	int dHandlesPDULen = 0;
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
		dHandlesPDULen+=count+1;
	}

	memcpy(messagePDU+C_HDR_SIZE+1+cHandleLen+1+dHandlesPDULen, message, messageLen+1);
}

void buildAndSendMessage(int socketNum, int numHandles, uint8_t* handlesBuf, int dHandlesLen, uint8_t* messageBuf, uint8_t* cHandle, uint8_t cHandleLen){
    int messageLen = strlen((char*) messageBuf);
	int messageCount = getMessageCount((char*) messageBuf, messageLen);

	for(int i = 0; i < messageCount; i++){
		uint8_t pduMessageLen = (messageLen>199) ? 199 : messageLen;
		uint8_t pduMessage[pduMessageLen+1];

        uint16_t pduLen;
        
        pduLen = C_HDR_SIZE+1+cHandleLen+1+dHandlesLen+pduMessageLen+1;
		
		uint8_t msgPDU[pduLen];
		
		pduMessage[pduMessageLen] = '\0';
		memcpy(pduMessage, messageBuf, pduMessageLen);
		messageBuf+=pduMessageLen;
		messageLen-=pduMessageLen;

        buildMessagePDU(handlesBuf, numHandles, cHandle, cHandleLen, pduMessage, pduMessageLen, msgPDU, pduLen);

		sendToSocket(socketNum, msgPDU, pduLen);
	}
}

int safeRecv(int socketNum, uint8_t * buffer, int bufferLen, int flag)
{
    int bytesReceived = recv(socketNum, buffer, bufferLen, flag);
    if (bytesReceived < 0)
    {
        if (errno == ECONNRESET)
        {
            bytesReceived = 0;
        }
        else
        {
            perror("recv call");
            exit(-1);
        }
    }
    return bytesReceived ;
}

int safeSend(int socketNum, uint8_t * buffer, int bufferLen, int flag)
{
	int bytesSent = 0;
	if ((bytesSent = send(socketNum, buffer, bufferLen, flag)) < 0)
	{
        perror("recv call");
        exit(-1);
    }
	 
    return bytesSent;
}


void * srealloc(void *ptr, size_t size)
{
	void * returnValue = NULL;
	
	if ((returnValue = realloc(ptr, size)) == NULL)
	{
		printf("Error on realloc (tried for size: %d\n", (int) size);
		exit(-1);
	}
	
	return returnValue;
} 

void * sCalloc(size_t nmemb, size_t size)
{
	void * returnValue = NULL;
	if ((returnValue = calloc(nmemb, size)) == NULL)
	{
		perror("calloc");
		exit(-1);
	}
	return returnValue;
}

