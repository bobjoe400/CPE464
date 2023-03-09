#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "safeUtil.h"
#include "slideWindow.h"

static windowBuff* slidingWindow;
static windowBuff* serverBuff;

void init_slideWindow(int windowsize, int buffsize){
    slidingWindow = sCalloc(1, sizeof(windowBuff));
    slidingWindow->capacity = windowsize;
    slidingWindow->size = 0;
    slidingWindow->buffer = sCalloc(windowsize, sizeof(windowBuffEntry));
}

void init_serverBuff(int windowsize, int buffsize){
    serverBuff = sCalloc(1, sizeof(windowBuff));
    serverBuff->capacity = windowsize;
    serverBuff->size = 0;
    serverBuff->buffer = sCalloc(windowsize, sizeof(windowBuffEntry));
}

windowBuffEntry getClientPacket(int seqNum){
    int index = seqNum%slidingWindow->capacity;
    return slidingWindow->buffer[index];
}

void addClientPacket(int seqNum, char* data, int nbytes){
    int windowsize = slidingWindow->capacity;
    int currentIndex = seqNum%windowsize;

    slidingWindow->buffer[currentIndex].seqNum = seqNum;
    slidingWindow->buffer[currentIndex].dataLen = nbytes;
    slidingWindow->buffer[currentIndex].data = sCalloc(1, nbytes);
    
    memcpy(slidingWindow->buffer[currentIndex].data, data, nbytes);

    slidingWindow->size++;
}

void removeClientPacket(int seqNum){
    int index = seqNum%slidingWindow->capacity;
    slidingWindow->buffer[index].seqNum = -1;
    slidingWindow->buffer[index].dataLen = 0;
    free(slidingWindow->buffer[index].data);
    slidingWindow->size--;
}

char isClientFull(){
    if(slidingWindow->size == slidingWindow->capacity){
        return 1;
    }
    return 0;
}

windowBuffEntry getServerPacket(int seqNum){
    int index = seqNum%slidingWindow->capacity;
    return slidingWindow->buffer[index];
}

void addServerPacket(int seqNum, char* data, int nbytes){
    int windowsize = serverBuff->capacity;
    int currentIndex = seqNum%windowsize;

    serverBuff->buffer[currentIndex].seqNum = seqNum;
    slidingWindow->buffer[currentIndex].dataLen = nbytes;
    serverBuff->buffer[currentIndex].data = sCalloc(1, nbytes);

    memcpy(serverBuff->buffer[currentIndex].data, data, nbytes);

    serverBuff->size++;
}

void removeServerPacket(int seqNum){
    int index = seqNum%serverBuff->capacity;
    serverBuff->buffer[index].seqNum = -1;
    serverBuff->buffer[index].dataLen = 0;
    free(serverBuff->buffer[index].data);
    serverBuff->size--;
}
