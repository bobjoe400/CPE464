#include <stdio.h>
#include <pcap.h>
#include "checksum.h"
#include "utility.h"
#include <sys/types.h>
#include <string.h>

void process_icmp_hdr(const unsigned char* packet, const unsigned char* ip_st){
    printf("\tICMP Header\n");
    printf("\t\tType: ");
    char type = packet++[0];
    if(type == 8){
        printf("Request\n");
    }else if(type == 0){
        printf("Reply\n");
    }else{
        printf("%i\n", type);
    }
    printf("\n");
    return;
}

void process_udp_hdr(const unsigned char* packet,const unsigned char* ip_st){
    
}

void process_tcp_hdr(const unsigned char* packet,const unsigned char* ip_st, uint16_t tcp_pdu_len, char ip_ptcl){
    unsigned char* tcp_begin = packet;
    printf("\tTCP Header\n");
    printf("\t\tSource Port: :");
    print_tcp_udp_port(&packet);
    printf("\t\tDest Port: : ");
    print_tcp_udp_port(&packet);
    printf("\t\tSequence Number: %u\n", get_long(&packet,1));
    printf("\t\tACK Number: ");
    uint32_t ack_n = get_long(&packet,0);
    packet++;
    char ack_f = packet[0] & 0x10;
    if(ack_f){
        printf("%u\n", ack_n);
    }else{
        printf("<not valid>\n");
    }
    char rst_f = packet[0] & 0x4;
    char syn_f = packet[0] & 0x2;
    char fin_f = packet[0] & 0x1;
    printf("\t\tACK Flag: %s\n", (ack_f)? "Yes" : "No");
    printf("\t\tSYN Flag: %s\n", (syn_f)? "Yes" : "No");
    printf("\t\tRST Flag: %s\n", (rst_f)? "Yes" : "No");
    printf("\t\tFIN Flag: %s\n", (fin_f)? "Yes" : "No");
    packet++;
    printf("\t\tWindow Size: %i\n", get_short(&packet, 1));
    uint8_t p_hdr[12] = {0};
    tcp_pdu_len = htons(tcp_pdu_len);
    memcpy(p_hdr, ip_st, 2*IP_ADDR_SIZE);
    memcpy(p_hdr+2*IP_ADDR_SIZE+1, &ip_ptcl, 1);
    memcpy(p_hdr+2*IP_ADDR_SIZE+2, &tcp_pdu_len, 2);
    tcp_pdu_len = ntohs(tcp_pdu_len);
    uint8_t tcp_chksm[12+tcp_pdu_len];
    memcpy(tcp_chksm, p_hdr, 12);
    memcpy(tcp_chksm+12, tcp_begin, tcp_pdu_len);
    printf("\t\tChecksum: ");
    if(in_cksum((unsigned short*)tcp_chksm, 12+tcp_pdu_len) == 0){
        printf("Correct ");
    }else{
        printf("Incorrect ");
    }
    printf("(0x%x)\n", get_short(&packet, 0));
}

void process_ip_hdr(const unsigned char* packet){
    const unsigned char* ip_begin = packet;
    printf("\tIP Header\n");

    uint16_t hdr_len = (packet++[0]&0xf)*4;

    printf("\t\tHeader Len: %i (bytes)\n", hdr_len);
    printf("\t\tTOS: 0x%x\n", packet++[0]);

    uint16_t pdu_len = get_short(&packet, 1);
    packet = packet+LONG_BYTES;

    printf("\t\tTTL: %i\n", packet++[0]);
    printf("\t\tIP PDU Len: %i (bytes)\n", pdu_len);
    printf("\t\tProtocol: ");

    char protocol = packet++[0];
    switch(protocol){
        case 1:
            printf("ICMP\n");
            break;
        case 6:
            printf("TCP\n");
            break;
        case 17:
            printf("UDP\n");
            break;
        default:
            printf("Unknown\n");
            break;
    }

    printf("\t\tChecksum: ");
    if(in_cksum((unsigned short*)ip_begin, hdr_len)==0){
        printf("Correct ");
    }else{
        printf("Incorrect ");
    }
    printf("(0x%x)\n", get_short(&packet, 0));
    ip_begin = packet;
    printf("\t\tSender IP: ");
    print_ip(&packet);
    printf("\t\tDest IP: ");
    print_ip(&packet);
    printf("\n");

    switch(protocol){
        case 1:
            process_icmp_hdr(packet, ip_begin);
            break;
        case 6:
            process_tcp_hdr(packet, ip_begin, pdu_len - hdr_len, protocol);
            break;
        case 17:
            process_udp_hdr(packet, ip_begin);
            break;
        default:
            break;
    }
    return;
}

void process_arp_hdr(const unsigned char* packet){
    printf("\tARP Header\n");
    printf("\t\tOpcode: ");
    packet = packet + (2*SHORT_BYTES) + 2;
    uint16_t opcode = get_short(&packet, 1);
    switch(opcode){
        case 1:
            printf("Request\n");
            break;
        case 2:
            printf("Reply\n");
            break;
        default:
            printf("%i\n", opcode);
            break;
    }
    printf("\t\tSender MAC: ");
    print_mac(&packet);
    printf("\t\tSender IP: ");
    print_ip(&packet);
    printf("\t\tTarget MAC: ");
    print_mac(&packet);
    printf("\t\tTarget IP: ");
    print_ip(&packet);
    printf("\n");
    return;
}

void process_eth_hdr(const unsigned char* packet){
    printf("\tEthernet Header\n");
    printf("\t\tDest MAC: ");

    print_mac(&packet);

    printf("\t\tSource MAC: ");
    
    print_mac(&packet);

    printf("\t\tType: ");

    uint16_t type = get_short(&packet, 1);

    if(type == 0x0800){
        printf("IP\n\n");
        process_ip_hdr(packet);
    }else if(type == 0x0806){
        printf("ARP\n\n");
        process_arp_hdr(packet);   
    }else{
        printf("Unknown\n");
    }
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
        printf("\n");
        i++;
    }

    pcap_close(file);

}