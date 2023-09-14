    
#include <stdio.h>
#include "py/runtime.h"
#include "py/obj.h"
#include "py/objarray.h"
#include "extmod/ulab/code/ndarray.h"
//#include "s_array.h" 
#include "circularqueue.h"


//stolen from py/objarray.c
//used to make new bytearray obj
STATIC mp_obj_array_t *array_new(char typecode, size_t n) {
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
}

//called whenever a CircularQueue is constructed in the runtime enviroment
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

//module level function, are member functions not supported in circuitpython?
//called by structures.push()
//later will want to generalize for other structure types
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
	return mp_obj_new_int(0);
}
//
//module level function, are member functions not supported in circuitpython?
//called by structures.pop()
//later will want to generalize for other structure types
STATIC mp_obj_t CircularQueue_pop(mp_obj_t self_in)
{
	
	CircularQueue_obj_t* self = MP_OBJ_TO_PTR(self_in);
	if (self->curSize ==0) return mp_const_none;

	size_t sz = mp_binary_get_size('@', BYTEARRAY_TYPECODE, NULL);
	mp_obj_array_t* retval = array_new(BYTEARRAY_TYPECODE, self->items[0].len);
	memcpy(retval->items, self->items[self->outptr].data, self->items[self->outptr].len * sz);
	
	self->outptr = (self->outptr + 1) % self->capacity; 
	self->curSize -=1;	
	return MP_OBJ_FROM_PTR(retval);	

}

STATIC mp_obj_t CircularQueue_size(mp_obj_t self_in)
{
	CircularQueue_obj_t* self = MP_OBJ_TO_PTR(self_in);
	return mp_obj_new_int(self->curSize);
}

//defining module level functions, gives runtime enviroment access to function ptrs
MP_DEFINE_CONST_FUN_OBJ_2(CircularQueue_push_obj, CircularQueue_push);
MP_DEFINE_CONST_FUN_OBJ_1(CircularQueue_pop_obj, CircularQueue_pop);
MP_DEFINE_CONST_FUN_OBJ_1(CircularQueue_size_obj, CircularQueue_size);
/*
STATIC const mp_rom_map_elem_t CircularQueue_locals_dict_table[] = {

	{ MP_OBJ_NEW_QSTR(MP_QSTR_CircularQueue), (mp_obj_t)&CircularQueue_type },
};
*/
//need this when adding attributes to class
//STATIC MP_DEFINE_CONST_DICT(CircularQueue_locals_dict, CircularQueue_locals_dict_table);

//STATIC MP_DEFINE_CONST_DICT(CircularQueue_locals_dict, CircularQueue_locals_dict_table);



//prints circular queue in runtime enviroment
STATIC void CircularQueue_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    (void)kind;
	CircularQueue_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_print_str(print, ",\n");
    mp_print_str(print, "circular queue(");
   	for (size_t i=0; i <self->curSize; i++)
	{
		size_t index = (self->inptr + i) % self->capacity; 
		mp_obj_print_helper(print, mp_obj_new_bytearray(self->items[index].len, self->items[index].data), PRINT_REPR);

    	mp_print_str(print, ",\n");
	}
    mp_print_str(print, ")");
	
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
//Mapping strings from runtime to functions or objs defined in this file
STATIC const mp_rom_map_elem_t structures_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_structures) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_CircularQueue), (mp_obj_t)&CircularQueue_type },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_push), MP_ROM_PTR(&CircularQueue_push_obj) },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_pop), MP_ROM_PTR(&CircularQueue_pop_obj) },
	{ MP_OBJ_NEW_QSTR(MP_QSTR_size), MP_ROM_PTR(&CircularQueue_size_obj) },

};
//all modules have the dict defined above
STATIC MP_DEFINE_CONST_DICT (
    mp_module_structures_globals,
    structures_globals_table
);
//allowing the globals defined above to be viewed after importing
const mp_obj_module_t structures_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_structures_globals,
};
//registering module to the runtime enviroment
MP_REGISTER_MODULE(MP_QSTR_structures, structures_user_cmodule, 1/*MODULE_PROPERTYCLASS_ENABLED*/);









