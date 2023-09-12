#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "s_array.h"
#include <stdbool.h>
#include <string.h>
#include "py/obj.h"
typedef struct _CircularQueue_obj_t
{
	mp_obj_base_t base;
	S_Array* items;
	
	uint32_t inptr;
	uint32_t outptr;
	uint32_t curSize;
	uint32_t capacity;

} CircularQueue_obj_t;

const mp_obj_type_t CircularQueue_type;


void initCircularQueue(CircularQueue_obj_t* cq, uint32_t initSize, uint32_t initElem);


void pushCircularQueue(CircularQueue_obj_t* cq, uint8_t* buff, uint32_t length); 


void popCircularQueue(CircularQueue_obj_t* cq, uint8_t* buff, uint32_t length, uint32_t* transferred);

bool isEmpty(CircularQueue_obj_t* cq);

int size(CircularQueue_obj_t* cq);

void destructCircularQueue(CircularQueue_obj_t* cq);








