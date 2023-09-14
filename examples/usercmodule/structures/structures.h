#include <stdio.h>
#include "py/runtime.h"
#include "py/obj.h"
#include "py/binary.h"
#include "py/objarray.h"
#include "extmod/ulab/code/ndarray.h"
//mp_obj_t array_append(mp_obj_t self_in, mp_obj_t arg);
//mp_obj_t create_new_myclass(uint16_t a, uint16_t b);
ndarray_obj_t* getNumpyArray(const mp_obj_t);


STATIC mp_obj_array_t *array_new(char typecode, size_t n);/* {
    if (typecode == 'x') {
        mp_raise_ValueError(MP_ERROR_TEXT("bad typecode"));
    }
    int typecode_size = mp_binary_get_size('@', typecode, NULL);
    mp_obj_array_t *o = m_new_obj(mp_obj_array_t);
    #if MICROPY_PY_BUILTINS_BYTEARRAY && MICROPY_PY_ARRAY
    o->base.type = (typecode == BYTEARRAY_TYPECODE) ? &mp_type_bytearray : &mp_type_array;
    #elif MICROPY_PY_BUILTINS_BYTEARRAY
    o->base.type = &mp_type_bytearray;
    #else
    o->base.type = &mp_type_array;
    #endif
    o->typecode = typecode;
    o->free = 0;
    o->len = n;
    o->items = m_new(byte, typecode_size * o->len);
    return o;
}*/
