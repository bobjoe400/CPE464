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

int checkArgs(int argc, char * argv[])
{
    int portNumber = 0;
	
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

	portNumber = atoi(argv[8]);
		
	return portNumber;
}

int main (int argc, char *argv[])
 {
	int socketNum = 0;				
	struct sockaddr_in6 server;		// Supports 4 and 6 but requires IPv6 struct
	int portNumber = 0;
	
	portNumber = checkArgs(argc, argv);
	
	socketNum = setupUdpClientToServer(&server, argv[1], portNumber);
	
	close(socketNum);

	return 0;
}