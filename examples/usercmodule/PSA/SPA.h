#include <stdio.h>
#include "py/runtime.h"
#include "py/obj.h"
#include "py/binary.h"
#include "py/objarray.h"
#include "extmod/ulab/code/ndarray.h"
//mp_obj_t array_append(mp_obj_t self_in, mp_obj_t arg);
//mp_obj_t create_new_myclass(uint16_t a, uint16_t b);
ndarray_obj_t* getNumpyArray(const mp_obj_t);


