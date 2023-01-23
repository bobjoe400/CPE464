#include <stdio.h>
#include <pcap.h>
#include "checksum.h"
#include "utility.h"
#include <arpa/inet.h>
#include <sys/types.h>
#include <string.h>

#define MAC_SIZE 6

void process_eth_hdr(const unsigned char* packet){
    printf("\tEthernet Header\n");
    printf("\t\tDest MAC: ");
    int i = 0;
    for(; i < (MAC_SIZE-1); i++){
        printf("%x:", packet[i]);
    }
    printf("%x\n", packet[i++]);
    printf("\t\tSource MAC: ");
    for(; i<(2*MAC_SIZE-1); i++){
        printf("%x:", packet[i]);
    }
    printf("%x\n", packet[i++]);
    printf("\t\tType: ");
    uint16_t type;
    memcpy(&type, packet+(2*MAC_SIZE), 2);
    type = ntohs(type);
    if(type == 0x0800){
        printf("IP\n");
    }else if(type == 0x0806){
        printf("ARP\n");   
    }else{
        printf("Unknown\n");
    }
    printf("\n");
    return;
}

void process_ip_hdr(const unsigned char* packet){
    printf("\tIP Header\n");

    printf("\n");
    return;
}

int main(int argc, char* argv[]){
    if(argc != 2){
        fprintf(stderr, "Usage trace <File Name>\n");
        return -1;
    }
    char* fileName = argv[1];
    
    pcap_t* file;
    char p_o_errbuf[256];
    
    if((file = pcap_open_offline(fileName, p_o_errbuf)) == NULL){
        fprintf(stderr, "%s", p_o_errbuf);
        return -1;
    }

    struct pcap_pkthdr* pkthdr;
    const unsigned char* packet;
    int i = 1;

    while(pcap_next_ex(file, &pkthdr, &packet) > 0){
        printf("Packet number: %i  Frame Len: %i\n\n", i, pkthdr->len);
        process_eth_hdr(packet);
        process_ip_hdr(packet);
        i++;
    }

}