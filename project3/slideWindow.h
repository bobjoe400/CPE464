#ifndef slide_Window_H
#define slide_Window_H

#include <stdio.h>
#include <stdlib.h>

typedef struct{
    int seqNum;
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

windowBuffEntry* getServerPacket();
void addServerPacket(int seqNum, char* data, int nbytes);

#endif