#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include "utility.h"

void print_mac(const unsigned char** packet){
    for(int i = 0; i < (MAC_ADDR_SIZE-1); i++){
        printf("%x:", (*packet)++[0]);
    }
    printf("%x\n", (*packet)++[0]);
}

void print_ip(const unsigned char** packet){
    for(int i = 0; i < (IP_ADDR_SIZE-1); i++){
        printf("%i.", (*packet)++[0]);
    }
    printf("%i\n", (*packet)++[0]);
}

uint16_t get_short(const unsigned char** packet, char conv){
    uint16_t t_short;
    memcpy(&t_short, *packet, SHORT_BYTES);
    *packet = *packet+SHORT_BYTES;
    if(conv) return ntohs(t_short);
    return t_short;
}

uint32_t get_long(const unsigned char** packet, char conv){
    uint32_t t_long;
    memcpy(&t_long, *packet, LONG_BYTES);
    *packet = *packet+LONG_BYTES;
    if(conv) return ntohl(t_long);
    return t_long;
}

void print_tcp_udp_port(const unsigned char** packet){
    *packet = *packet + SHORT_BYTES;
    printf("\n");
    return;
}