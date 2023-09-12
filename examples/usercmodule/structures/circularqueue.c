#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
//#include "array.h"
#include "circularqueue.h"
#include <stdbool.h>
#include <string.h>



void initCircularQueue(CircularQueue_obj_t* cq, uint32_t initSize, uint32_t initElem)
{
	cq->inptr = 0;
	cq->outptr = 0;
	cq->curSize=0;
	cq->capacity = initElem;
	
	cq->items = (Array*)malloc(sizeof(Array)*initElem);
	for (uint32_t i=0; i<initElem; i++)
	{
		cq->items[i].data = (uint8_t*) malloc(sizeof(uint8_t)*initSize);
		cq->items[i].len = initSize;	
	}
}


void pushCircularQueue(CircularQueue_obj_t* cq, uint8_t* buff, uint32_t length)
{
	if (length > cq->items[0].len) return;
	memcpy(cq->items[cq->inptr].data, buff, length);
	cq->inptr = (cq->inptr+1)%cq->capacity;
	cq->curSize+=1;
	if (cq->curSize > cq->capacity) cq->curSize = cq->capacity;
}


void popCircularQueue(CircularQueue_obj_t* cq, uint8_t* buff, uint32_t length, uint32_t* transferred)
{
	if (cq->curSize == 0) return;
	(*transferred) = length > cq->items[0].len ? cq->items[0].len : length;
	memcpy(buff, cq->items[cq->outptr].data, *transferred);
	cq->outptr = (cq->outptr +1)% cq->capacity;	
	cq->curSize-=1;
}

bool isEmpty(CircularQueue_obj_t* cq)
{
	return cq->curSize == 0;
}

int size(CircularQueue_obj_t* cq)
{
	return cq->curSize;
}

void destructCircularQueue(CircularQueue_obj_t* cq)
{
	for (int i=0; i<cq->capacity; i++)
	{
		free(cq->items[i].data);
	}

	free(cq->items);
}
