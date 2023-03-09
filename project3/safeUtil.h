// 
// Writen by Hugh Smith, April 2020
//
// Put in system calls with error checking.

#ifndef __SAFEUTIL_H__
#define __SAFEUTIL_H__

#define LONG_BYTES 32
#define SHORT_BYTES 16
#define HDR_SIZE LONG_BYTES+SHORT_BYTES+1

struct sockaddr;

typedef enum flag FLAG;

enum FLAG{
	DATA=3, RR = 5, SREJ, FILENAME, FNACK;
};

void createHeader(uint8_t flag, int seqNum, uint8_t* hdrbuf);
int buildPacket(uint8_t flag, int seqNum, int dataLen, uint8_t* data, uint8_t* PDUbuf);
int32_t send_buf(uint8_t flag, int seqNum, int dataLen, uint8_t* data, uint8_t* PDUBuf,  Connection* connection);
void * srealloc(void *ptr, size_t size);
void * sCalloc(size_t nmemb, size_t size);


#endif
