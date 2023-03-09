#ifndef slide_Window_H
#define slide_Window_H

#include <stdio.h>
#include <stdlib.h>

typedef struct{
    int seqNum;
    int dataLen;
    char* data;
}windowBuffEntry;

typedef struct{
    int size;
    int capacity;
    windowBuffEntry* buffer;
}windowBuff;

void init_slideWindow(int windowsize, int buffsize);
void init_serverBuff(int windowsize, int buffsize);

windowBuffEntry getClientPacket(int seqNum);
void addClientPacket(int seqNum, char* data, int nbytes);
void removeClientPacket(int seqNum);

windowBuffEntry getServerPacket();
void addServerPacket(int seqNum, char* data, int nbytes);
void removeServerPacket(int seqNum);

#endif