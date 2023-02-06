// 
// Writen by Hugh Smith, Jan. 2023
//
// Put in system calls with error checking.

#ifndef __SAFEUTIL_H__
#define __SAFEUTIL_H__

#include <stdint.h>

#define SHORT_BYTES 2
#define LONG_BYTES 4

#define INPUTMAX 1400
#define HANDLEMAX 100
#define MSGMAXLEN 199
#define C_HDR_SIZE 3
#define RCVMAX C_HDR_SIZE+1+HANDLEMAX+2+HANDLEMAX+MSGMAXLEN+1
#define ASCII_NUM_OFFSET 48

uint16_t get_short(uint8_t** packet, uint8_t conv);
uint32_t get_long(uint8_t** packet, uint8_t conv);

void sendToSocket(int socketNum, uint8_t* pdubuf, uint16_t pdulen);
void buildMessagePDU(uint8_t* buf, uint8_t numHandles, uint8_t* cHandle, uint8_t cHandleLen, uint8_t* message, int messageLen, uint8_t* messagePDU, uint16_t messagePDULen);
void buildAndSendMessage(int socketNum, int numHandles, uint8_t* handlesBuf, int dHandlesLen, uint8_t* messageBuf, uint8_t* cHandle, uint8_t cHandleLen);

int getMessageCount(char* buf, int messageLen);

int safeRecv(int socketNum, uint8_t * buffer, int bufferLen, int flag);
int safeSend(int socketNum, uint8_t * buffer, int bufferLen, int flag);

void * srealloc(void *ptr, size_t size);
void * sCalloc(size_t nmemb, size_t size);


#endif