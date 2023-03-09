
// 
// Writen by Hugh Smith, April 2020, Feb. 2021
//
// Put in system calls with error checking
// keep the function paramaters same as system call

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#include "safeUtil.h"
#include "checksum.h"
#include "networks.h"

#ifdef __LIBCPE464_
#include "cpe464.h"
#endif

void createHeader(uint8_t flag, int seqNum, uint8_t* hdrbuf){
	uint32_t netSeqNum = htonl(seqNum);
	memcpy(hdrbuf, &netSeqNum, LONG_BYTES);
	memset(hdrbuf+LONG_BYTES, SHORT_BYTES, 0);
	memcpy(hdrbuf+LONG_BYTES+SHORT_BYTES, &flag, sizeof(uint8_t));
}

int buildPacket(uint8_t flag, int seqNum, int dataLen, uint8_t* data, uint8_t* PDUBuf){
	uint8_t hdrbuf[HDR_SIZE];
	createHeader(flag, seqNum, hdrbuf);
	memcpy(PDUBuf, hdrbuf, HDR_SIZE);
	if(dataLen>0){
		memcpy(PDUBuf+HDR_SIZE, data, dataLen);
	}
	uint16_t chksum = in_cksum(PDUBuf, HDR_SIZE+dataLen);
	memcpy(PDUBuf+LONG_BYTES, chksum, SHORT_BYTES);
	return HDR_SIZE+dataLen;
}

int32_t send_buf(uint8_t flag, int seqNum, int dataLen, uint8_t* data, uint8_t* PDUBuf,  Connection* connection){
	int sendingLen = buildPacket(flag, seqNum, dataLen, data, PDUBuf);
	return safeSendto(PDUBuf, sendingLen, connection);
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

