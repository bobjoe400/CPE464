#define _GNU_SOURCE
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "serverUtil.h"
#include "safeUtil.h"
#include "networks.h"
#include "pollLib.h"

static struct LinkedList* clientList;

void initLinkedList(){
    clientList = (struct LinkedList*) sCalloc(1,sizeof(LinkedList));
    clientList->size = 0;
}

LinkedList* getList(){
    return clientList;
}

void addListElem(int socket, uint8_t* handle, int handleLen){

	//initialize new element
	ListElem* newElem = (ListElem*) sCalloc(1,sizeof(ListElem));
	char* newHandle = netHandleToString(handle, handleLen);
    newElem->handle = newHandle;
	newElem->socket = socket;
	newElem->next = NULL;

	if(isListEmpty()){
		clientList->head = newElem;
        clientList->size++;
		return;
	}

	//find end of list and add element
	ListElem* current = clientList->head;
	while(current->next != NULL){
		current = current->next;
	}

	current->next = newElem;
    clientList->size++;

	return;
}

void addListElem_l(LinkedList* list, int socket, char* handle){

	//initialize new element
	ListElem* newElem = (ListElem*) sCalloc(1, sizeof(ListElem));
	char* newHandle = strdup(handle);
    newElem->handle = newHandle;
	newElem->socket = socket;
	newElem->next = NULL;

	if(isListEmpty_l(list)){
		list->head = newElem;
		return;
	}

	//find end of list and add element
	ListElem* current = list->head;
	while(current->next != NULL){
		current = current->next;
	}

	current->next = newElem;
    list->size++;

	return;
}

ListElem* findInList_s(int socket){
    ListElem* current = clientList->head;
    while(current != NULL && current->socket != socket){
        current = current->next;
    }
    return current;
}

ListElem* findInList_h(uint8_t* handle, int handleLen){
    char* toFind = netHandleToString(handle, handleLen);
    ListElem* current = clientList->head;
    while(current != NULL && (strcmp(current->handle, toFind) != 0)){
        current = current->next;
    }
    return current;
}

ListElem* listDelete(int socket){
	struct ListElem* temp = clientList->head, *prev;
	struct ListElem* retVal = (ListElem*) sCalloc(1, sizeof(ListElem));
    
	//if first elements contains the data
    if (temp != NULL && temp->socket == socket) {
		memcpy(retVal, temp, sizeof(ListElem));
        clientList->head = temp->next; 
        free(temp->handle);
        free(temp); 
        clientList->size--;
        return retVal;
    }
	
	//in the middle or end
    while (temp != NULL && temp->socket != socket) {
        prev = temp;
        temp = temp->next;
    }
	
	//return if not found (shouldn't happen)
    if (temp == NULL) return NULL;
	
	//copy data into the return value, unlink the nodes and free the unlinked node and return
	memcpy(retVal, temp, sizeof(ListElem));
    prev->next = temp->next;

    free(temp->handle);
	free(temp);

    clientList->size--;
	return retVal;
}

ListElem* listDelete_l(LinkedList* list, int socket){
	struct ListElem* temp = list->head, *prev;
	struct ListElem* retVal = (ListElem*) sCalloc(1, sizeof(ListElem));
    
	//if first elements contains the data
    if (temp != NULL && temp->socket == socket) {
		memcpy(retVal, temp, sizeof(ListElem));
        list->head = temp->next; 
        free(temp->handle);
        free(temp); 
        return retVal;
    }
	
	//in the middle or end
    while (temp != NULL && temp->socket != socket) {
        prev = temp;
        temp = temp->next;
    }
	
	//return if not found (shouldn't happen)
    if (temp == NULL) return NULL;
	
	//copy data into the return value, unlink the nodes and free the unlinked node and return
	memcpy(retVal, temp, sizeof(ListElem));
    prev->next = temp->next;

    free(temp->handle);
	free(temp);

    list->size--;
	return retVal;
}

int isListEmpty(){
	return clientList->head == NULL;
}

int isListEmpty_l(LinkedList* list){
	return list->head == NULL;
}

char* netHandleToString(uint8_t* handle, int handleLen){
    char* newHandle = sCalloc(1, handleLen+1);
    newHandle[handleLen] = 0;
    memcpy(newHandle, handle, handleLen);
    return newHandle;
}