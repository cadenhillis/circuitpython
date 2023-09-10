#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "array.h"
#include <stdbool.h>
#include <string.h>
#include "py/obj.h"
typedef struct _CircularQueue_obj_t
{
	mp_obj_base_t base;
	Array* items;
	
	uint32_t inptr;
	uint32_t outptr;
	uint32_t curSize;
	uint32_t capacity;

} CircularQueue_obj_t;

const mp_obj_type_t CircularQueue_type;


void initCircularQueue(CircularQueue* cq, uint32_t initSize, uint32_t initElem);


void pushCircularQueue(CircularQueue* cq, uint8_t* buff, uint32_t length); 


void popCircularQueue(CircularQueue* cq, uint8_t* buff, uint32_t length, uint32_t* transferred);

bool isEmpty(CircularQueue* cq);

int size(CircularQueue* cq);

void destructCircularQueue(CircularQueue* cq);








