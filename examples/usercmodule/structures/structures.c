/*
 * This file is part of the micropython-usermod project, 
 *
 * https://github.com/v923z/micropython-usermod
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019-2020 Zoltán Vörös
*/
    
#include <stdio.h>
#include "py/runtime.h"
#include "py/obj.h"
#include "py/objarray.h"
#include "extmod/ulab/code/ndarray.h"
#include "array.h" 
#include "circularqueue.h"


/**
 * @brief: Get float array from python object
 * @param args arguments passed by interpreter
 * @param index index of args that stores the list
 * @param arry dest array
 */
ndarray_obj_t* getNumpyArray(const mp_obj_t arg)
{

	if (mp_obj_is_type(arg,&ulab_ndarray_type)) {
		ndarray_obj_t* arry = MP_OBJ_TO_PTR(arg);
		return ndarray_copy_view(arry);
	}
	else mp_raise_TypeError(translate("input must be a dense ndarray hi"));
	uint8_t ndim = 1;
	size_t shape = 1;
	int32_t strides = 1;
	uint8_t dtype = 1;
	return ndarray_new_ndarray(ndim, &shape, &strides, dtype);
}


///////////////////////////////////////////////////////
//define com class
//


STATIC mp_obj_t CircularQueue_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 2, 2, true);
    CircularQueue_obj_t *self = m_new_obj(CircularQueue_obj_t);
    self->base.type = &CircularQueue_type;
	self->inptr = 0;
	self->outptr =0;
	self->curSize = 0;
	self->capacity = mp_obj_get_int(args[0]);

	self->items = (Array*) malloc(sizeof(Array)*initElem);
	uint32_t len = mp_obj_get_int(args[1]);
	for (int i=0; i<self->capacity; i++)
	{
		cq->items[i].len = len;
		cq->items[i].data = (uint8_t*) malloc(sizeof(uint8_t)*len);
	}

	return MP_OBJ_FROM_PTR(self);
}


STATIC MP_DEFINE_CONST_DICT(CircularQueue_locals_dict, CircularQueue_locals_dict_table);


STATIC mp_obj_t CircularQueue_push(size_t n_args, size_t n_kw, const mp_obj_t * args)
{

    mp_arg_check_num(n_args, n_kw, 1, 1, true);
}


STATIC void CircularQueue_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    (void)kind;
    CircularQueue_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_print_str(print, "comSoh(");
	mp_print_str(print, " rssi: ");
    mp_obj_print_helper(print, mp_obj_new_float(self->rssi), PRINT_REPR);
	
	
}

//register python object visible to interpreter
const mp_obj_type_t CircularQueue_type = {
    { &mp_type_type },
    .name = MP_QSTR_CircularQueue,
    .make_new = CircularQueue_make_new,
//    .attr = comSoh_attr,
    .locals_dict = (mp_obj_dict_t*)&CircularQueue_locals_dict,
	.print = CircularQueue_print,
};






//register globals constants and dicts 
//
//
STATIC const mp_map_elem_t structures_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_structures) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_CircularQueue), (mp_obj_t)&CircularQueue_type },

};

STATIC MP_DEFINE_CONST_DICT (
    mp_module_structures_globals,
    structures_globals_table
);

const mp_obj_module_t structures_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_structures_globals,
};

MP_REGISTER_MODULE(MP_QSTR_structures, structures_user_cmodule, 1/*MODULE_PROPERTYCLASS_ENABLED*/);









