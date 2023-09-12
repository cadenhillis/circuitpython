    
#include <stdio.h>
#include "py/runtime.h"
#include "py/obj.h"
#include "py/objarray.h"
#include "extmod/ulab/code/ndarray.h"
//#include "s_array.h" 
#include "circularqueue.h"




STATIC mp_obj_t CircularQueue_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 2, 2, true);
    CircularQueue_obj_t *self = m_new_obj(CircularQueue_obj_t);
    self->base.type = &CircularQueue_type;
	self->inptr = 0;
	self->outptr =0;
	self->curSize = 0;
	self->capacity = mp_obj_get_int(args[0]);

	uint32_t len = mp_obj_get_int(args[1]);
	self->items = m_new(S_Array, sizeof(S_Array)*len);
	//self->items = (S_Array*) malloc(sizeof(S_Array)*len);
	for (uint32_t i=0; i<self->capacity; i++)
	{
		self->items[i].len = len;
		self->items[i].data = m_new(byte, sizeof(byte)*len);
		//nice try, cross compiler dont have _sbrk...
		//self->items[i].data = (uint8_t*) malloc(sizeof(uint8_t)*len);
	}

	return MP_OBJ_FROM_PTR(self);
}


STATIC mp_obj_t CircularQueue_push(mp_obj_t self_in, mp_obj_t new_data)
{
	
	CircularQueue_obj_t* self = MP_OBJ_TO_PTR(self_in);
	mp_buffer_info_t bufinfo;		
    if (((MICROPY_PY_BUILTINS_BYTEARRAY)
         || (MICROPY_PY_ARRAY
             && (mp_obj_is_type(new_data, &mp_type_bytes)
                 || (MICROPY_PY_BUILTINS_BYTEARRAY && mp_obj_is_type(new_data, &mp_type_bytearray)))))
        && mp_get_buffer(new_data, &bufinfo, MP_BUFFER_READ)) {
        // construct array from raw bytes
        size_t sz = mp_binary_get_size('@', BYTEARRAY_TYPECODE, NULL);
        if (bufinfo.len % sz) {
            mp_raise_ValueError(MP_ERROR_TEXT("bytes length not a multiple of item size"));
        }
        size_t len = bufinfo.len / sz;
        if (len > self->items[0].len) mp_raise_ValueError(MP_ERROR_TEXT("bytes length too bit for cq"));

		memcpy(self->items[self->inptr].data, bufinfo.buf, len * sz);
		
		self->inptr = (self->inptr+1)%self->capacity;
		self->curSize+=1;
		if (self->curSize > self->capacity) self->curSize = self->capacity;
	}
	return MP_OBJ_FROM_PTR(self);
}

MP_DEFINE_CONST_FUN_OBJ_2(CircularQueue_push_obj, CircularQueue_push);
/*
STATIC const mp_rom_map_elem_t CircularQueue_locals_dict_table[] = {

	{ MP_OBJ_NEW_QSTR(MP_QSTR_CircularQueue), (mp_obj_t)&CircularQueue_type },
};
*/
//need this when adding attributes to class
//STATIC MP_DEFINE_CONST_DICT(CircularQueue_locals_dict, CircularQueue_locals_dict_table);

//STATIC MP_DEFINE_CONST_DICT(CircularQueue_locals_dict, CircularQueue_locals_dict_table);




STATIC void CircularQueue_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    (void)kind;
//    CircularQueue_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_print_str(print, "circular queue(");
	mp_print_str(print, " rssi: ");
    //mp_obj_print_helper(print, mp_obj_new_float(self->rssi), PRINT_REPR);
	
	
}

//register python object visible to interpreter
const mp_obj_type_t CircularQueue_type = {
    { &mp_type_type },
    .name = MP_QSTR_CircularQueue,
    .make_new = CircularQueue_make_new,
//    .attr = comSoh_attr,
//	.locals_dict = (mp_obj_dict_t*)&CircularQueue_locals_dict,
//	.globals = (mp_obj_dict_t*)&CircularQueue_locals_dict,
	.print = CircularQueue_print,
};






//register globals constants and dicts 
//
//
STATIC const mp_rom_map_elem_t structures_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_structures) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_CircularQueue), (mp_obj_t)&CircularQueue_type },
	{ MP_ROM_QSTR(MP_QSTR_push), MP_ROM_PTR(&CircularQueue_push_obj) },

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









