#ifndef UTILITY_H
#define UTILITY_H

#include <stdint.h>
#include <arpa/inet.h>

#define MAC_ADDR_SIZE 6
#define IP_ADDR_SIZE 4
#define SHORT_BYTES 2
#define LONG_BYTES 4

void print_mac(const unsigned char**);
void print_ip(const unsigned char**);
uint16_t get_short(const unsigned char**, char);
uint32_t get_long(const unsigned char**, char);
void print_tcp_udp_port(const unsigned char**);

#endif