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
    slidingWindow->capacity = windowsize;
    slidingWindow->size = 0;
    slidingWindow->buffer = sCalloc(windowsize, sizeof(windowBuffEntry));
}

windowBuffEntry getClientPacket(int seqNum){
    int index = seqNum%slidingWindow->capacity;
    return slidingWindow->buffer[index];
}
void addClientPacket(int seqNum, char* data, int nbytes){
    int windowsize = slidingWindow->capacity;
    int currentIndex = seqNum%windowsize;
    slidingWindow->buffer[currentIndex].seqNum = seqNum;
    slidingWindow->buffer[currentIndex].data = sCalloc(1, nbytes);
    memcpy(slidingWindow->buffer[currentIndex].data, data, nbytes);
    slidingWindow->size++;
}
char isClientFull(){
    if(slidingWindow->size == slidingWindow->capacity){
        return 1;
    }
    return 0;
}

windowBuffEntry* getServerPacket(int seqNum){

}
void addServerPacket(int seqNum, char* data, int nbytes){
    int windowsize = serverBuff->capacity;
    int currentIndex = seqNum%windowsize;
    serverBuff->buffer[currentIndex].seqNum = seqNum;
    serverBuff->buffer[currentIndex].data = sCalloc(1, nbytes);
    memcpy(serverBuff->buffer[currentIndex].data, data, nbytes);
    serverBuff->size++;
}