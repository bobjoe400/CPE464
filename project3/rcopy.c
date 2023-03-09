// Client side - UDP Code				    
// By Hugh Smith	4/1/2017		

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
#include <math.h>

#include "gethostbyname.h"
#include "networks.h"
#include "safeUtil.h"
#include "slideWindow.h"
#include "cpe464.h"

typedef enum State STATE;

enum State{
	START, DONE, FILENAME, SEND_DATA, WAIT_ON_ACK, TIMEOUT_ON_ACK, WAIT_ON_EOF_ACK, TIMEOUT_ON_EOF_ACK
};

void  checkArgs(int argc, char * argv[])
{
	
        /* check command line arguments  */
	if (argc != 8)
	{
		printf("usage: %s from-filename to-filename window-size buffer-size error-percent remote-machine remote-port \n", argv[0]);
		exit(1);
	}
	if(atoi(argv[3]) > (int)pow(2, 30)){
		printf("usage: %s window-size must be less than or equal to 2^30\n", argv[0]);
		exit(1);
	}
	if(atoi(argv[4]) > 1400 || atoi(argv[4]) < 1){
		printf("usage: %s buffer-size must be from 1 to 1400\n", argv[0]);
		exit(1);
	}
	if(strlen(argv[2])> 100){
		printf("usage: %s maximum destination filename is 100 characters\n", argv[0]);
		exit(1);
	}
}

STATE start_state(char** argv, Connection* server, int* clientSeqNum){
	int fileNameLen = strlen(argv[2]);
	uint8_t packet[HDR_SIZE+fileNameLen+SHORT_BYTES+LONG_BYTES];
	int dataLen = fileNameLen+SHORT_BYTES+LONG_BYTES;
	uint8_t data[dataLen];
	
	STATE returnValue = SEND_DATA;
	uint16_t buffersize = 0;
	uint32_t windowSize = 0;
	
	if(server->sk_num>0){
		close(server->sk_num);
	}

	if(udpClientSetup(argv[6], argv[7], server) < 0){
		returnValue = DONE;
	}
	else{
		buffersize = htons(atoi(argv[4]));
		windowSize = hotnl(atoi(argv[3]));
		memcpy(data, argv[2], fileNameLen);
		memcpy(&data[fileNameLen], buffersize, SHORT_BYTES);
		memcpy(&data[fileNameLen+SHORT_BYTES], windowSize, LONG_BYTES);
		send_buf(FILENAME, *clientSeqNum, dataLen, data, packet, server);
		(*clientSeqNum)++;
		
		returnValue = FILENAME;
	}
	
	return returnValue;
}

STATE filename(Connection* server){

}

void processClient(char* argv[]){
	
	int readfilefd;
	if(readfilefd = open(argv[1], O_RDONLY) < 0){
		perror("File does not exist\n");
		exit(1);
	}

	STATE state = START;
	int clientSeqNum = 0;
	Connection* server = sCalloc(1, sizeof(Connection));

	while(state != DONE){
		switch(state){
			case START:
				state = start_state(argv, server, &clientSeqNum);
				break;
			case FILENAME:
				state = filename(server);
			case SEND_DATA:
				break;
			case WAIT_ON_ACK:
				break;
			case WAIT_ON_EOF_ACK:
				break;
			case TIMEOUT_ON_ACK:
				break;
			case TIMEOUT_ON_EOF_ACK:
				break;
			case DONE:
				break;
		}
	}
}

int main (int argc, char *argv[])
 {
	checkArgs(argc, argv);

	//sendtoErr_init(atof(argv[4]), DROP_OFF, FLIP_OFF, DEBUG_ON, RSEED_OFF);

	processClient(argv);

	return 0;
}