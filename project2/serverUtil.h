#ifndef SERVERUTIL_H
#define SERVERUTIL_H

#include <stdint.h>


typedef struct ListElem ListElem;
typedef struct LinkedList LinkedList;

struct ListElem{
    int socket;
    char* handle;
    ListElem* next;
};

struct LinkedList{
    ListElem* head;
    int size;
};

void initLinkedList();
LinkedList* getList();

void addListElem(int socket, uint8_t* handle, int handleLen);
void addListElem_l(LinkedList* list, int socket, char* handle);

ListElem* listDelete(int socket);
ListElem* listDelete_l(LinkedList* list, int socket);

ListElem* findInList_s(int socket);
ListElem* findInList_h(uint8_t* handle, int handleLen);

int isListEmpty();
int isListEmpty_l(LinkedList* list);

char* netHandleToString(uint8_t* handle, int handleLen);

#endif