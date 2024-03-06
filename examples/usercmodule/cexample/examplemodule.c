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
#include <sys/types.h>
#include "py/runtime.h"
#include "py/obj.h"
#include "examplemodule.h"
#include "py/objarray.h"
#include "extmod/ulab/code/ndarray.h"
const int INT_ERROR = -9999;



static void copy_np_array(ndarray_obj_t* src, uint8_t* dest)
{

	uint8_t* s_array = (uint8_t*) src->array;
	uint8_t* d = dest;
	for (size_t i=0; i<src->shape[ULAB_MAX_DIMS-1]; i++) {
		memcpy(d, s_array, src->itemsize);
		s_array += src->strides[ULAB_MAX_DIMS - 1];
		d += src->itemsize;


	}

}

static ndarray_obj_t* copy_to_np_array(ndarray_obj_t* ndarray, uint8_t* src, size_t len, size_t sz)
{

        ndarray = m_new_obj(ndarray_obj_t);
        
		ndarray->base.type = &ulab_ndarray_type;
		
		ndarray->dtype = NDARRAY_FLOAT;
        ndarray->boolean =  NDARRAY_NUMERIC;
        ndarray->ndim = 1;
        ndarray->len = len;
        ndarray->itemsize = sz;
        ndarray->shape[ULAB_MAX_DIMS - 1] = len;
        ndarray->strides[ULAB_MAX_DIMS - 1] = sz;

        ndarray->array = src ;
		return ndarray;	
}


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
	//else //mp_raise_TypeError(translate("input must be a dense ndarray hi"));
	uint8_t ndim = 1;
	size_t shape = 1;
	int32_t strides = 1;
	uint8_t dtype = NDARRAY_FLOAT;// 1;
	return ndarray_new_ndarray(ndim, &shape, &strides, dtype);
}

static mp_float_t getFloat(const mp_obj_t in)
{
	if (in == mp_const_none) return INT_ERROR;
	return mp_obj_get_float(in);
}

//define adcs module

///////////////////////////////////////////////////////
typedef struct _adcsSoh_obj_t {
    mp_obj_base_t base;	
    uint8_t adcsstatus;
	uint8_t state;
	ndarray_obj_t* moment;//[3];
	ndarray_obj_t* angularVelocity;//[3];
	ndarray_obj_t* quat;//[4];
	float adcstime;
	float timestamp;
		
} adcsSoh_obj_t;

const mp_obj_type_t adcsSoh_type;

STATIC mp_obj_t adcsSoh_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    
	
	
	
	enum {ARG_status, ARG_state, ARG_moment, ARG_angularVelocity, ARG_quat, ARG_adcstime,ARG_timestamp, NUM_ARGS};
	static const mp_arg_t allowed_args[] = {
		{MP_QSTR_status, MP_ARG_INT, {.u_int = INT_ERROR}},
		{MP_QSTR_state, MP_ARG_INT, {.u_int = INT_ERROR}},
		{MP_QSTR_moment, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_angularVelocity, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_quat, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_adcstime, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_timestamp, MP_ARG_OBJ, {.u_obj = mp_const_none}},
	};
	
	mp_arg_val_t arg_vals[MP_ARRAY_SIZE(allowed_args)];
	mp_arg_parse_all_kw_array(n_args, n_kw, args, MP_ARRAY_SIZE(allowed_args), allowed_args, arg_vals);
	
	
	
	
	
	//mp_arg_check_num(n_args, n_kw, 7, 7, true);
    adcsSoh_obj_t *self = m_new_obj(adcsSoh_obj_t);
    self->base.type = &adcsSoh_type;

    self->adcsstatus = arg_vals[0].u_int;
    self->state = arg_vals[1].u_int;
	self->moment = getNumpyArray(arg_vals[2].u_obj);
	self->angularVelocity = getNumpyArray(arg_vals[3].u_obj);
	self->quat = getNumpyArray(arg_vals[4].u_obj);
	self->adcstime = getFloat(arg_vals[5].u_obj);
	self->timestamp = getFloat(arg_vals[6].u_obj);	

	return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t adcsSoh_get_bytes(mp_obj_t self_in)
{
	adcsSoh_obj_t* self = MP_OBJ_TO_PTR(self_in);
	
	mp_obj_array_t* retval = array_new(BYTEARRAY_TYPECODE, 2* sizeof(uint8_t) + 12*sizeof(float));
	uint8_t* dest = retval->items;
	//memcpy(retval->items, self, sizeof(adcsSoh_obj_t));
	memcpy(dest++, &self->adcsstatus, 1);
	memcpy(dest++, &self->state, 1);
	copy_np_array(self->moment, dest);
	dest += 3*sizeof(float);
	copy_np_array(self->angularVelocity, dest);
	dest += 3*sizeof(float);
	copy_np_array(self->quat, dest);
	dest += 4*sizeof(float);
	memcpy(dest,&self->adcstime, sizeof(mp_float_t));
	dest += sizeof(float);
	memcpy(dest,&self->timestamp, sizeof(mp_float_t));


	return MP_OBJ_FROM_PTR(retval);
}

STATIC mp_obj_t adcsSoh_from_bytes(mp_obj_t self_in, mp_obj_t new_data)
{
	
    adcsSoh_obj_t *self = m_new_obj(adcsSoh_obj_t);
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
        //size_t len = bufinfo.len / sz;
	}
    self->base.type = &adcsSoh_type;

	memcpy(&self->adcsstatus, bufinfo.buf++, 1);
	memcpy(&self->state, bufinfo.buf++, 1);
	self->moment = copy_to_np_array(self->moment, bufinfo.buf,3,  sizeof(float));
	bufinfo.buf+=(3 * sizeof(float));
	self->angularVelocity = copy_to_np_array(self->angularVelocity, bufinfo.buf,3,  sizeof(float));
	bufinfo.buf+=(3 * sizeof(float));
	self->quat = copy_to_np_array(self->quat, bufinfo.buf,4,  sizeof(float));
	bufinfo.buf+=(4 * sizeof(float));
	memcpy(&self->adcstime, bufinfo.buf, sizeof(float));
	bufinfo.buf+=sizeof(float);
	memcpy(&self->timestamp, bufinfo.buf, sizeof(float));
	bufinfo.buf+=sizeof(float);

	return MP_OBJ_FROM_PTR(self);	

}

//getter only visible to c interface, called by propertyclass_attr
//if attr qstr == x's qster
STATIC mp_obj_t adcsSoh_state(mp_obj_t self_in) {
    adcsSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(self->state);
}

STATIC mp_obj_t adcsSoh_adcsstatus(mp_obj_t self_in) {
    adcsSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(self->adcsstatus);
}
STATIC mp_obj_t adcsSoh_adcstime(mp_obj_t self_in) {
    adcsSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->adcstime);
}
STATIC mp_obj_t adcsSoh_timestamp(mp_obj_t self_in) {
    adcsSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->timestamp);
}
STATIC mp_obj_t adcsSoh_moment(mp_obj_t self_in) {
    adcsSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return MP_OBJ_FROM_PTR(self->moment);
}

STATIC mp_obj_t adcsSoh_angularVelocity(mp_obj_t self_in) {
    adcsSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
	return MP_OBJ_FROM_PTR(self->angularVelocity);
}
STATIC mp_obj_t adcsSoh_quat(mp_obj_t self_in) {
	adcsSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
	return MP_OBJ_FROM_PTR(self->quat);
}



MP_DEFINE_CONST_FUN_OBJ_1(adcsSoh_get_bytes_obj, adcsSoh_get_bytes);
MP_DEFINE_CONST_FUN_OBJ_2(adcsSoh_from_bytes_obj, adcsSoh_from_bytes);

STATIC const mp_rom_map_elem_t adcsSoh_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_get_bytes), MP_ROM_PTR(&adcsSoh_get_bytes_obj) },
    { MP_ROM_QSTR(MP_QSTR_from_bytes), MP_ROM_PTR(&adcsSoh_from_bytes_obj) }
};
STATIC MP_DEFINE_CONST_DICT(adcsSoh_locals_dict, adcsSoh_locals_dict_table);
//define object with one parameter
/*
MP_DEFINE_CONST_FUN_OBJ_1(adcsSoh_state_obj, adcsSoh_state);
MP_DEFINE_CONST_FUN_OBJ_1(adcsSoh_moment_obj, adcsSoh_moment);
MP_DEFINE_CONST_FUN_OBJ_1(adcsSoh_angularVelocity_obj, adcsSoh_angularVelocity);
MP_DEFINE_CONST_FUN_OBJ_1(adcsSoh_quat_obj, adcsSoh_quat);
*/
//map qsttr for attribute to attribute object
/*STATIC const mp_rom_map_elem_t adcsSoh_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_state), MP_ROM_PTR(&adcsSoh_state_obj) },
    { MP_ROM_QSTR(MP_QSTR_moment), MP_ROM_PTR(&adcsSoh_moment_obj) },
    { MP_ROM_QSTR(MP_QSTR_angularVelocity), MP_ROM_PTR(&adcsSoh_angularVelocity_obj) },
    { MP_ROM_QSTR(MP_QSTR_quat), MP_ROM_PTR(&adcsSoh_quat_obj) },
};



STATIC MP_DEFINE_CONST_DICT(adcsSoh_locals_dict, adcsSoh_locals_dict_table);
*/
//called by class_instance.attribute
//checks if attribute is a member of the internal data struct
//destination[0] is the output, must be mp_obj type
STATIC void adcsSoh_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination) {
 
	switch (attribute) {	
		case MP_QSTR_state:
        destination[0] = adcsSoh_state(self_in);
    	break;
	
		case MP_QSTR_adcsstatus:
		destination[0] = adcsSoh_adcsstatus(self_in);
		break;
		
		case MP_QSTR_moment:
		destination[0] = adcsSoh_moment(self_in);
		break;

		case MP_QSTR_angularVelocity:
		destination[0] = adcsSoh_angularVelocity(self_in);
		break;
		case MP_QSTR_quat:
		destination[0] = adcsSoh_quat(self_in);
		break;
		case MP_QSTR_adcstime:
		destination[0] = adcsSoh_adcstime(self_in);
		break;
		case MP_QSTR_timestamp:
		destination[0] = adcsSoh_timestamp(self_in);
		break;
	}

}

//define how the class will be printed
STATIC void adcsSoh_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    (void)kind;
    adcsSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
//    mp_print_str(print, "adcsSoh(");
	mp_print_str(print, " state: ");
    mp_obj_print_helper(print, mp_obj_new_int(self->state), PRINT_REPR);
	mp_print_str(print, ", status: ");
    mp_obj_print_helper(print, mp_obj_new_int(self->adcsstatus), PRINT_REPR);
	
	mp_print_str(print, ", moment: ");
    mp_obj_print_helper(print,MP_OBJ_FROM_PTR(self->moment), PRINT_REPR);
	
	mp_print_str(print, ", angularVelocity: ");
    mp_obj_print_helper(print, MP_OBJ_FROM_PTR(self->angularVelocity), PRINT_REPR);
	
	mp_print_str(print, ", Quaternion: ");
    mp_obj_print_helper(print, MP_OBJ_FROM_PTR(self->quat), PRINT_REPR);


	mp_print_str(print, ", adcsTime: ");
    mp_obj_print_helper(print, mp_obj_new_float(self->adcstime), PRINT_REPR);
	
	mp_print_str(print, ", timestamp: ");
    mp_obj_print_helper(print, mp_obj_new_float(self->timestamp), PRINT_REPR);
//    mp_print_str(print, ")");
}



//register python object visible to interpreter
const mp_obj_type_t adcsSoh_type = {
    { &mp_type_type },
    .name = MP_QSTR_adcsSoh,
    .make_new = adcsSoh_make_new,
    .attr = adcsSoh_attr,
    .locals_dict = (mp_obj_dict_t*)&adcsSoh_locals_dict,
	.print = adcsSoh_print
};






///////////////////////////////////////////////////////
//define gps soh module
typedef struct _gpsSoh_obj_t {
    mp_obj_base_t base;
    mp_float_t gpstime;
	mp_float_t alt;
	mp_float_t lat;
	mp_float_t lon;
	mp_int_t quality;
	mp_float_t timestamp;
	mp_float_t speed_knots;
} gpsSoh_obj_t;

const mp_obj_type_t gpsSoh_type;

STATIC mp_obj_t gpsSoh_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {


	enum {ARG_gpstime, ARG_alt, ARG_lat, ARG_lon, ARG_quality,ARG_timestamp, ARG_speed_knots, NUM_ARGS};
	static const mp_arg_t allowed_args[] = {
		{MP_QSTR_gpstime, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_alt, MP_ARG_OBJ, {.u_obj =mp_const_none}},
		{MP_QSTR_lat, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_lon, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_quality, MP_ARG_INT, {.u_int = INT_ERROR}},
		{MP_QSTR_timestamp, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_speed_knots, MP_ARG_OBJ, {.u_obj = mp_const_none}},
	};
	
	mp_arg_val_t arg_vals[MP_ARRAY_SIZE(allowed_args)];
	mp_arg_parse_all_kw_array(n_args, n_kw, args, MP_ARRAY_SIZE(allowed_args), allowed_args, arg_vals);


    gpsSoh_obj_t *self = m_new_obj(gpsSoh_obj_t);
    self->base.type = &gpsSoh_type;
    self->gpstime = getFloat(arg_vals[ARG_gpstime].u_obj);
    self->alt = getFloat(arg_vals[ARG_alt].u_obj);
    self->lat = getFloat(arg_vals[ARG_lat].u_obj);
    self->lon = getFloat(arg_vals[ARG_lon].u_obj);
    self->quality = arg_vals[ARG_quality].u_int;
    self->timestamp = getFloat(arg_vals[ARG_timestamp].u_obj);
    self->speed_knots = getFloat(arg_vals[ARG_speed_knots].u_obj);
    return MP_OBJ_FROM_PTR(self);
}
//getter only visible to c interface, called by propertyclass_attr
//if attr qstr == x's qster
STATIC mp_obj_t gpsSoh_gpstime(mp_obj_t self_in) {
    gpsSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->gpstime);
}
STATIC mp_obj_t gpsSoh_alt(mp_obj_t self_in) {
    gpsSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->alt);
}
STATIC mp_obj_t gpsSoh_lat(mp_obj_t self_in) {
    gpsSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->lat);
}
STATIC mp_obj_t gpsSoh_lon(mp_obj_t self_in) {
    gpsSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->lon);
}
STATIC mp_obj_t gpsSoh_quality(mp_obj_t self_in) {
    gpsSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(self->quality);
}
STATIC mp_obj_t gpsSoh_timestamp(mp_obj_t self_in) {
    gpsSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->timestamp);
}
STATIC mp_obj_t gpsSoh_speed_knots(mp_obj_t self_in) {
    gpsSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->speed_knots);
}


STATIC mp_obj_t gpsSoh_get_bytes(mp_obj_t self_in)
{
	gpsSoh_obj_t* self = MP_OBJ_TO_PTR(self_in);
	
	mp_obj_array_t* retval = array_new(BYTEARRAY_TYPECODE, sizeof(mp_int_t) + 6*sizeof(float));
	uint8_t* dest = retval->items;
	uint8_t* src = (uint8_t*) self;
	src+= sizeof(mp_obj_base_t);
	memcpy(dest, src, sizeof(gpsSoh_obj_t) - sizeof(mp_obj_base_t));
	
	//memcpy(retval->items, self, sizeof(adcsSoh_obj_t));
	
	/*memcpy(dest,&self->gpstime, sizeof(mp_float_t));
	dest += sizeof(float);
	memcpy(dest,&self->alt, sizeof(mp_float_t));
	dest += sizeof(float);
	memcpy(dest,&self->lat, sizeof(mp_float_t));
	dest += sizeof(float);
	memcpy(dest,&self->lon, sizeof(mp_float_t));
	dest += sizeof(float);
	memcpy(dest,&self->quality, sizeof(mp_int_t));
	dest += sizeof(mp_int_t);
	memcpy(dest,&self->timestamp, sizeof(mp_float_t));
	dest += sizeof(float);
	memcpy(dest,&self->speed_knots, sizeof(mp_float_t));
	*/

	return MP_OBJ_FROM_PTR(retval);
}

STATIC mp_obj_t gpsSoh_from_bytes(mp_obj_t self_in, mp_obj_t new_data)
{
	
    gpsSoh_obj_t *self = m_new_obj(gpsSoh_obj_t);
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
        //size_t len = bufinfo.len / sz;
	}
    self->base.type = &gpsSoh_type;

	memcpy(&self->gpstime, bufinfo.buf, sizeof(float));
	bufinfo.buf+=sizeof(float);
	memcpy(&self->alt, bufinfo.buf, sizeof(float));
	bufinfo.buf+=sizeof(float);
	memcpy(&self->lat, bufinfo.buf, sizeof(float));
	bufinfo.buf+=sizeof(float);
	memcpy(&self->lon, bufinfo.buf, sizeof(float));
	bufinfo.buf+=sizeof(float);
	memcpy(&self->quality, bufinfo.buf, sizeof(mp_int_t));
	bufinfo.buf+=sizeof(mp_int_t);
	memcpy(&self->timestamp, bufinfo.buf, sizeof(float));
	bufinfo.buf+=sizeof(float);
	memcpy(&self->speed_knots, bufinfo.buf, sizeof(float));

	return MP_OBJ_FROM_PTR(self);	

}


MP_DEFINE_CONST_FUN_OBJ_1(gpsSoh_get_bytes_obj, gpsSoh_get_bytes);
MP_DEFINE_CONST_FUN_OBJ_2(gpsSoh_from_bytes_obj, gpsSoh_from_bytes);

STATIC const mp_rom_map_elem_t gpsSoh_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_get_bytes), MP_ROM_PTR(&gpsSoh_get_bytes_obj) },
    { MP_ROM_QSTR(MP_QSTR_from_bytes), MP_ROM_PTR(&gpsSoh_from_bytes_obj) }
};	
STATIC MP_DEFINE_CONST_DICT(gpsSoh_locals_dict, gpsSoh_locals_dict_table);
//define object with one parameter
/*
MP_DEFINE_CONST_FUN_OBJ_1(gpsSoh_gpstime_obj, gpsSoh_gpstime);
MP_DEFINE_CONST_FUN_OBJ_1(gpsSoh_alt_obj, gpsSoh_alt);
MP_DEFINE_CONST_FUN_OBJ_1(gpsSoh_lat_obj, gpsSoh_lat);
MP_DEFINE_CONST_FUN_OBJ_1(gpsSoh_lon_obj, gpsSoh_lon);
MP_DEFINE_CONST_FUN_OBJ_1(gpsSoh_quality_obj, gpsSoh_quality);
MP_DEFINE_CONST_FUN_OBJ_1(gpsSoh_timestamp_obj, gpsSoh_timestamp);
MP_DEFINE_CONST_FUN_OBJ_1(gpsSoh_speed_knots_obj, gpsSoh_speed_knots);
*/
//map qsttr for attribute to attribute object
/*
STATIC const mp_rom_map_elem_t gpsSoh_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_gpstime), MP_ROM_PTR(&gpsSoh_gpstime_obj) },
//    { MP_ROM_QSTR(MP_QSTR_alt), MP_ROM_PTR(&gpsSoh_alt_obj) },
//    { MP_ROM_QSTR(MP_QSTR_lat), MP_ROM_PTR(&gpsSoh_lat_obj) },
    { MP_ROM_QSTR(MP_QSTR_lon), MP_ROM_PTR(&gpsSoh_lon_obj) },
    { MP_ROM_QSTR(MP_QSTR_quality), MP_ROM_PTR(&gpsSoh_quality_obj) },
    { MP_ROM_QSTR(MP_QSTR_timestamp), MP_ROM_PTR(&gpsSoh_timestamp_obj) },
//    { MP_ROM_QSTR(MP_QSTR_speed_knots), MP_ROM_PTR(&gpsSoh_speed_knots_obj) },
};

STATIC MP_DEFINE_CONST_DICT(gpsSoh_locals_dict, gpsSoh_locals_dict_table);
*/
//called by class_instance.attribute
//checks if attribute is a member of the internal data struct
//destination[0] is the output, must be mp_obj type
STATIC void gpsSoh_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination) {
    switch (attribute) {
		case MP_QSTR_gpstime:
        destination[0] = gpsSoh_gpstime(self_in);
    	break;
		case MP_QSTR_alt:
        destination[0] = gpsSoh_alt(self_in);
    	break;
		case MP_QSTR_lat:
        destination[0] = gpsSoh_lat(self_in);
    	break;
		case MP_QSTR_lon:
        destination[0] = gpsSoh_lon(self_in);
    	break;
		case MP_QSTR_quality:
        destination[0] = gpsSoh_quality(self_in);
    	break;
		case MP_QSTR_timestamp:
        destination[0] = gpsSoh_timestamp(self_in);
    	break;
    	case MP_QSTR_speed_knots:
        destination[0] = gpsSoh_speed_knots(self_in);
    	break;
	}
}


STATIC void gpsSoh_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    (void)kind;
    gpsSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    //mp_print_str(print, "gpsSoh(");
	mp_print_str(print, " lat: ");
    mp_obj_print_helper(print, mp_obj_new_float(self->lat), PRINT_REPR);
	
	
	mp_print_str(print, ", lon: ");
    mp_obj_print_helper(print, mp_obj_new_float(self->lon), PRINT_REPR);
	
	mp_print_str(print, ", alt: ");
    mp_obj_print_helper(print,mp_obj_new_float(self->alt), PRINT_REPR);
	
	mp_print_str(print, ", quality: ");
    mp_obj_print_helper(print, mp_obj_new_int(self->quality), PRINT_REPR);
	

	mp_print_str(print, ", speed_knots: ");
    mp_obj_print_helper(print, mp_obj_new_float(self->speed_knots), PRINT_REPR);
	
	mp_print_str(print, ", gpstime: ");
    mp_obj_print_helper(print, mp_obj_new_float(self->gpstime), PRINT_REPR);
	
	mp_print_str(print, ", timestamp: ");
    mp_obj_print_helper(print, mp_obj_new_float(self->timestamp), PRINT_REPR);
   // mp_print_str(print, ")");
}


//register python object visible to interpreter
const mp_obj_type_t gpsSoh_type = {
    { &mp_type_type },
    .name = MP_QSTR_gpsSoh,
    .make_new = gpsSoh_make_new,
    .attr = gpsSoh_attr,
    .locals_dict = (mp_obj_dict_t*)&gpsSoh_locals_dict,
	.print = gpsSoh_print,
};











///////////////////////////////////////////////////////
//define power class
//

typedef struct _powerSoh_obj_t {
    mp_obj_base_t base;
    mp_float_t systemV;
	mp_float_t battV;
	mp_float_t busI;
	mp_float_t timestamp;

	mp_float_t solar_charge_1;
	mp_float_t solar_charge_2;
	mp_float_t charge_current;
	mp_float_t charge_voltage;
	mp_float_t battery_voltage;
	mp_float_t battery_current;
	mp_float_t bus_voltage;
	mp_float_t bus_current;
	mp_float_t v3v3_voltage;
	mp_float_t v3v3_current;
	mp_float_t payload_voltage;
	mp_float_t payload_current;
	mp_float_t rf_voltage;
	mp_float_t rf_current;
	mp_float_t solar1_voltage;
	mp_float_t solar2_voltage;
} powerSoh_obj_t;

const mp_obj_type_t powerSoh_type;

STATIC mp_obj_t powerSoh_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    
	
	enum {ARG_systemV, ARG_battV, ARG_busI,  ARG_timestamp,ARG_solar_charge_1, ARG_solar_charge_2, ARG_charge_current, ARG_charge_voltage, ARG_battery_voltage, ARG_battery_current, ARG_bus_voltage, ARG_bus_current, ARG_v3v3_voltage, ARG_v3v3_current, ARG_payload_voltage, ARG_payload_current, ARG_rf_voltage, ARG_rf_current, ARG_solar1_voltage, ARG_solar2_voltage, NUM_ARGS};
	static const mp_arg_t allowed_args[] = {
		{MP_QSTR_systemV, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_battV, MP_ARG_OBJ, {.u_obj =mp_const_none}},
		{MP_QSTR_busI, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_timestamp, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_solar_charge_1, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_solar_charge_2, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_charge_current, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_charge_voltage, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_battery_voltage, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_battery_current, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_bus_voltage, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_bus_current, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_v3v3_voltage, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_v3v3_current, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_payload_voltage, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_payload_current, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_rf_voltage, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_rf_current, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_solar1_voltage, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_solar2_voltage, MP_ARG_OBJ, {.u_obj = mp_const_none}},
	};
	
	mp_arg_val_t arg_vals[MP_ARRAY_SIZE(allowed_args)];
	mp_arg_parse_all_kw_array(n_args, n_kw, args, MP_ARRAY_SIZE(allowed_args), allowed_args, arg_vals);
	
	
    powerSoh_obj_t *self = m_new_obj(powerSoh_obj_t);
    self->base.type = &powerSoh_type;
    self->systemV = getFloat(arg_vals[ARG_systemV].u_obj);
    self->battV = getFloat(arg_vals[ARG_battV].u_obj);
    self->busI = getFloat(arg_vals[ARG_busI].u_obj);
    self->timestamp = getFloat(arg_vals[ARG_timestamp].u_obj);
    
	
    self->solar_charge_1  = getFloat(arg_vals[ARG_solar_charge_1].u_obj);
    self->solar_charge_2  = getFloat(arg_vals[ARG_solar_charge_2].u_obj);
    self->charge_current  = getFloat(arg_vals[ARG_charge_current].u_obj);
    self->charge_voltage  = getFloat(arg_vals[ARG_charge_voltage].u_obj);
    self->battery_voltage = getFloat(arg_vals[ARG_battery_voltage].u_obj);
    self->battery_current = getFloat(arg_vals[ARG_battery_current].u_obj);
    self->bus_voltage     = getFloat(arg_vals[ARG_bus_voltage].u_obj);
    self->bus_current     = getFloat(arg_vals[ARG_bus_current].u_obj);
    self->v3v3_voltage    = getFloat(arg_vals[ARG_v3v3_voltage].u_obj);
    self->v3v3_current    = getFloat(arg_vals[ARG_v3v3_current].u_obj);
    self->payload_voltage = getFloat(arg_vals[ARG_payload_voltage].u_obj);
    self->payload_current = getFloat(arg_vals[ARG_payload_current].u_obj);
    self->rf_voltage      = getFloat(arg_vals[ARG_rf_voltage].u_obj);
    self->rf_current      = getFloat(arg_vals[ARG_rf_current].u_obj);
    self->solar1_voltage  = getFloat(arg_vals[ARG_solar1_voltage].u_obj);
    self->solar2_voltage  = getFloat(arg_vals[ARG_solar2_voltage].u_obj);
		
	
	
	return MP_OBJ_FROM_PTR(self);
}
//getter only visible to c interface, called by propertyclass_attr
//if attr qstr == x's qster
STATIC mp_obj_t powerSoh_systemV(mp_obj_t self_in) {
    powerSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->systemV);
}
STATIC mp_obj_t powerSoh_battV(mp_obj_t self_in) {
    powerSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->battV);
}
STATIC mp_obj_t powerSoh_busI(mp_obj_t self_in) {
    powerSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->busI);
}
STATIC mp_obj_t powerSoh_timestamp(mp_obj_t self_in) {
    powerSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->timestamp);
}

STATIC mp_obj_t powerSoh_solar_charge_1(mp_obj_t self_in) {
    powerSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->solar_charge_1);
}
STATIC mp_obj_t powerSoh_solar_charge_2(mp_obj_t self_in) {
    powerSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->solar_charge_2);
}
STATIC mp_obj_t powerSoh_charge_current(mp_obj_t self_in) {
    powerSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->charge_current);
}
STATIC mp_obj_t powerSoh_charge_voltage(mp_obj_t self_in) {
    powerSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->charge_voltage);
}

STATIC mp_obj_t powerSoh_bus_current(mp_obj_t self_in) {
    powerSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->bus_current);
}

STATIC mp_obj_t powerSoh_battery_voltage(mp_obj_t self_in) {
    powerSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->battery_voltage);
}
STATIC mp_obj_t powerSoh_battery_current(mp_obj_t self_in) {
    powerSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->battery_current);
}
STATIC mp_obj_t powerSoh_bus_voltage(mp_obj_t self_in) {
    powerSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->bus_voltage);
}
STATIC mp_obj_t powerSoh_v3v3_voltage(mp_obj_t self_in) {
    powerSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->v3v3_voltage);
}
STATIC mp_obj_t powerSoh_v3v3_current(mp_obj_t self_in) {
    powerSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->v3v3_current);
}
STATIC mp_obj_t powerSoh_payload_voltage(mp_obj_t self_in) {
    powerSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->payload_voltage);
}
STATIC mp_obj_t powerSoh_payload_current(mp_obj_t self_in) {
    powerSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->payload_current);
}
STATIC mp_obj_t powerSoh_rf_voltage(mp_obj_t self_in) {
    powerSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->rf_current);
}
STATIC mp_obj_t powerSoh_rf_current(mp_obj_t self_in) {
    powerSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->rf_current);
}
STATIC mp_obj_t powerSoh_solar1_voltage(mp_obj_t self_in) {
    powerSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->solar1_voltage);
}
STATIC mp_obj_t powerSoh_solar2_voltage(mp_obj_t self_in) {
    powerSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->solar2_voltage);
}



STATIC mp_obj_t powerSoh_get_bytes(mp_obj_t self_in)
{
	powerSoh_obj_t* self = MP_OBJ_TO_PTR(self_in);
	
	mp_obj_array_t* retval = array_new(BYTEARRAY_TYPECODE, 20*sizeof(float));
	//uint8_t* dest = retval->items;
	//memcpy(retval->items, self, sizeof(adcsSoh_obj_t));
	uint8_t* src = (uint8_t*)self;
	uint8_t* dest = retval->items;
	src+=sizeof(mp_obj_base_t);
	memcpy(dest, src, sizeof(powerSoh_obj_t) - sizeof(mp_obj_base_t));

	return MP_OBJ_FROM_PTR(retval);
}

STATIC mp_obj_t powerSoh_from_bytes(mp_obj_t self_in, mp_obj_t new_data)
{
	
    powerSoh_obj_t *self = m_new_obj(powerSoh_obj_t);
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
        //size_t len = bufinfo.len / sz;
	}
    self->base.type = &powerSoh_type;

	uint8_t* dest = (uint8_t*)self;
	dest+= sizeof(mp_obj_base_t);
	memcpy(dest, bufinfo.buf, sizeof(powerSoh_obj_t)-sizeof(mp_obj_base_t));
	
	return MP_OBJ_FROM_PTR(self);	

}

MP_DEFINE_CONST_FUN_OBJ_1(powerSoh_get_bytes_obj, powerSoh_get_bytes);
MP_DEFINE_CONST_FUN_OBJ_2(powerSoh_from_bytes_obj, powerSoh_from_bytes);

STATIC const mp_rom_map_elem_t powerSoh_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_get_bytes), MP_ROM_PTR(&powerSoh_get_bytes_obj) },
    { MP_ROM_QSTR(MP_QSTR_from_bytes), MP_ROM_PTR(&powerSoh_from_bytes_obj) }
};
STATIC MP_DEFINE_CONST_DICT(powerSoh_locals_dict, powerSoh_locals_dict_table);
//define object with one parameter
/*
MP_DEFINE_CONST_FUN_OBJ_1(powerSoh_systemV_obj, powerSoh_systemV);
MP_DEFINE_CONST_FUN_OBJ_1(powerSoh_battV_obj, powerSoh_battV);
MP_DEFINE_CONST_FUN_OBJ_1(powerSoh_busI_obj, powerSoh_busI);
MP_DEFINE_CONST_FUN_OBJ_1(powerSoh_timestamp_obj, powerSoh_timestamp);
//map qsttr for attribute to attribute object
STATIC const mp_rom_map_elem_t powerSoh_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_systemV), MP_ROM_PTR(&powerSoh_systemV_obj) },
    { MP_ROM_QSTR(MP_QSTR_battV), MP_ROM_PTR(&powerSoh_battV_obj) },
    { MP_ROM_QSTR(MP_QSTR_busI), MP_ROM_PTR(&powerSoh_busI_obj) },
    { MP_ROM_QSTR(MP_QSTR_timestamp), MP_ROM_PTR(&powerSoh_timestamp_obj) },
};

STATIC MP_DEFINE_CONST_DICT(powerSoh_locals_dict, powerSoh_locals_dict_table);
*/
//called by class_instance.attribute
//checks if attribute is a member of the internal data struct
//destination[0] is the output, must be mp_obj type
STATIC void powerSoh_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination) {
    switch (attribute) {
		case MP_QSTR_systemV:
        destination[0] = powerSoh_systemV(self_in);
    	break;
		case MP_QSTR_battV:
        destination[0] = powerSoh_battV(self_in);
    	break;
    	case MP_QSTR_busI:
        destination[0] = powerSoh_busI(self_in);
    	break;
		case MP_QSTR_timestamp:
        destination[0] = powerSoh_timestamp(self_in);
    	break;
		case MP_QSTR_solar_charge_1:
        destination[0] = powerSoh_solar_charge_1(self_in);
    	break;
		case MP_QSTR_solar_charge_2:
        destination[0] = powerSoh_solar_charge_2(self_in);
    	break;
		case MP_QSTR_charge_current:
        destination[0] = powerSoh_charge_current(self_in);
    	break;
		case MP_QSTR_charge_voltage:
        destination[0] = powerSoh_charge_voltage(self_in);
    	break;
		case MP_QSTR_battery_voltage:
        destination[0] = powerSoh_battery_voltage(self_in);
    	break;
		case MP_QSTR_battery_current:
        destination[0] = powerSoh_battery_current(self_in);
    	break;
		case MP_QSTR_bus_voltage:
        destination[0] = powerSoh_bus_voltage(self_in);
    	break;
		case MP_QSTR_bus_current:
        destination[0] = powerSoh_bus_current(self_in);
    	break;
		case MP_QSTR_v3v3_voltage:
        destination[0] = powerSoh_v3v3_voltage(self_in);
    	break;
		case MP_QSTR_v3v3_current:
        destination[0] = powerSoh_v3v3_current(self_in);
    	break;
		case MP_QSTR_payload_voltage:
        destination[0] = powerSoh_payload_voltage(self_in);
    	break;
		case MP_QSTR_payload_current:
        destination[0] = powerSoh_payload_current(self_in);
    	break;
		case MP_QSTR_rf_voltage:
        destination[0] = powerSoh_rf_voltage(self_in);
    	break;
		case MP_QSTR_rf_current:
        destination[0] = powerSoh_rf_current(self_in);
    	break;
		case MP_QSTR_solar1_voltage:
        destination[0] = powerSoh_solar1_voltage(self_in);
    	break;
		case MP_QSTR_solar2_voltage:
        destination[0] = powerSoh_solar2_voltage(self_in);
    	break;
	}
}


STATIC void powerSoh_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    (void)kind;
    powerSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    //mp_print_str(print, "powerSoh(");
	mp_print_str(print, " systemV: ");
    mp_obj_print_helper(print, mp_obj_new_float(self->systemV), PRINT_REPR);
	
	
	mp_print_str(print, ", battV: ");
    mp_obj_print_helper(print, mp_obj_new_float(self->battV), PRINT_REPR);
	
	mp_print_str(print, ", busI: ");
    mp_obj_print_helper(print,mp_obj_new_float(self->busI), PRINT_REPR);
	


	mp_print_str(print, ", solarCharge1: ");
    mp_obj_print_helper(print,mp_obj_new_float(self->solar_charge_1), PRINT_REPR);
	mp_print_str(print, ", solarCharge2: ");
    mp_obj_print_helper(print,mp_obj_new_float(self->solar_charge_2), PRINT_REPR);
	mp_print_str(print, ", chargeI: ");
    mp_obj_print_helper(print,mp_obj_new_float(self->charge_current), PRINT_REPR);
	mp_print_str(print, ", chargeV: ");
    mp_obj_print_helper(print,mp_obj_new_float(self->charge_voltage), PRINT_REPR);
	mp_print_str(print, ", batteryV: ");
    mp_obj_print_helper(print,mp_obj_new_float(self->battery_voltage), PRINT_REPR);
	mp_print_str(print, ", batteryI: ");
    mp_obj_print_helper(print,mp_obj_new_float(self->battery_current), PRINT_REPR);
	mp_print_str(print, ", busV: ");
    mp_obj_print_helper(print,mp_obj_new_float(self->bus_voltage), PRINT_REPR);
	mp_print_str(print, ", busI: ");
    mp_obj_print_helper(print,mp_obj_new_float(self->bus_current), PRINT_REPR);
	mp_print_str(print, ", v3v3V: ");
    mp_obj_print_helper(print,mp_obj_new_float(self->v3v3_voltage), PRINT_REPR);
	mp_print_str(print, ", v3v3I: ");
    mp_obj_print_helper(print,mp_obj_new_float(self->v3v3_current), PRINT_REPR);
	mp_print_str(print, ", payV: ");
    mp_obj_print_helper(print,mp_obj_new_float(self->payload_voltage), PRINT_REPR);
	mp_print_str(print, ", payI: ");
    mp_obj_print_helper(print,mp_obj_new_float(self->payload_current), PRINT_REPR);
	mp_print_str(print, ", rfV: ");
    mp_obj_print_helper(print,mp_obj_new_float(self->rf_voltage), PRINT_REPR);
	mp_print_str(print, ", rfI: ");
    mp_obj_print_helper(print,mp_obj_new_float(self->rf_current), PRINT_REPR);
	mp_print_str(print, ", solar1V: ");
    mp_obj_print_helper(print,mp_obj_new_float(self->solar1_voltage), PRINT_REPR);
	mp_print_str(print, ", solar2V: ");
    mp_obj_print_helper(print,mp_obj_new_float(self->solar2_voltage), PRINT_REPR);


	mp_print_str(print, ", timestamp: ");
    mp_obj_print_helper(print, mp_obj_new_float(self->timestamp), PRINT_REPR);
    //mp_print_str(print, ")");
}

//register python object visible to interpreter
const mp_obj_type_t powerSoh_type = {
    { &mp_type_type },
    .name = MP_QSTR_powerSoh,
    .make_new = powerSoh_make_new,
    .attr = powerSoh_attr,
    .locals_dict = (mp_obj_dict_t*)&powerSoh_locals_dict,
	.print = powerSoh_print,
};



///////////////////////////////////////////////////////
//define temp class
//

typedef struct _tempSoh_obj_t {
    mp_obj_base_t base;
    mp_float_t battery;
	mp_float_t ntc1;
	mp_float_t ntc2;
	mp_float_t ntc3;
	mp_float_t ntc4;
	mp_float_t a;
	mp_float_t b;
	mp_float_t c;
	mp_float_t d;
	mp_float_t xMTQ;
	mp_float_t yMTQ;
	mp_float_t zMTQ;
	mp_float_t bTempA;
	mp_float_t bTempB;
	mp_float_t timestamp;
} tempSoh_obj_t;

const mp_obj_type_t tempSoh_type;

STATIC mp_obj_t tempSoh_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    
	
	enum {ARG_battery,ARG_ntc1, ARG_ntc2, ARG_ntc3, ARG_ntc4, ARG_a, ARG_b, ARG_c, ARG_d, ARG_xMTQ, ARG_yMTQ, ARG_zMTQ, ARG_bTempA, ARG_bTempB, ARG_timestamp, NUM_ARGS};
	static const mp_arg_t allowed_args[] = {
		{MP_QSTR_battery, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_ntc1, MP_ARG_OBJ, {.u_obj =mp_const_none}},
		{MP_QSTR_ntc2, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_ntc3, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_ntc4, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_a, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_b, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_c, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_d, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_xMTQ, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_yMTQ, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_zMTQ, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_bTempA, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_bTempB, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_timestamp, MP_ARG_OBJ, {.u_obj = mp_const_none}},
	};
	
	mp_arg_val_t arg_vals[MP_ARRAY_SIZE(allowed_args)];
	mp_arg_parse_all_kw_array(n_args, n_kw, args, MP_ARRAY_SIZE(allowed_args), allowed_args, arg_vals);
	
	
    tempSoh_obj_t *self = m_new_obj(tempSoh_obj_t);
    self->base.type = &tempSoh_type;

    self->battery = getFloat(arg_vals[ARG_battery].u_obj);
    self->ntc1 = getFloat(arg_vals[ARG_ntc1].u_obj);
    self->ntc2 = getFloat(arg_vals[ARG_ntc2].u_obj);
    self->ntc3 = getFloat(arg_vals[ARG_ntc3].u_obj);
    self->ntc4 = getFloat(arg_vals[ARG_ntc4].u_obj);
    self->a = getFloat(arg_vals[ARG_a].u_obj);
    self->b = getFloat(arg_vals[ARG_b].u_obj);
    self->c = getFloat(arg_vals[ARG_c].u_obj);
    self->d = getFloat(arg_vals[ARG_d].u_obj);
    self->xMTQ = getFloat(arg_vals[ARG_xMTQ].u_obj);
    self->yMTQ = getFloat(arg_vals[ARG_yMTQ].u_obj);
    self->zMTQ = getFloat(arg_vals[ARG_zMTQ].u_obj);
    self->bTempA = getFloat(arg_vals[ARG_bTempA].u_obj);
    self->bTempB = getFloat(arg_vals[ARG_bTempB].u_obj);
    self->timestamp = getFloat(arg_vals[ARG_timestamp].u_obj);
	

	
   	 
	return MP_OBJ_FROM_PTR(self);
}
//getter only visible to c interface, called by propertyclass_attr
//if attr qstr == x's qster
STATIC mp_obj_t tempSoh_battery(mp_obj_t self_in) {
    tempSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->battery);
}
STATIC mp_obj_t tempSoh_ntc1(mp_obj_t self_in) {
    tempSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->ntc1);
}
STATIC mp_obj_t tempSoh_ntc2(mp_obj_t self_in) {
    tempSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->ntc2);
}
STATIC mp_obj_t tempSoh_ntc3(mp_obj_t self_in) {
    tempSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->ntc4);
}
STATIC mp_obj_t tempSoh_ntc4(mp_obj_t self_in) {
    tempSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->ntc4);
}
STATIC mp_obj_t tempSoh_a(mp_obj_t self_in) {
    tempSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->a);
}
STATIC mp_obj_t tempSoh_b(mp_obj_t self_in) {
    tempSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->b);
}
STATIC mp_obj_t tempSoh_c(mp_obj_t self_in) {
    tempSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->c);
}
STATIC mp_obj_t tempSoh_d(mp_obj_t self_in) {
    tempSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->d);
}
STATIC mp_obj_t tempSoh_xMTQ(mp_obj_t self_in) {
    tempSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->xMTQ);
}
STATIC mp_obj_t tempSoh_yMTQ(mp_obj_t self_in) {
    tempSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->yMTQ);
}
STATIC mp_obj_t tempSoh_zMTQ(mp_obj_t self_in) {
    tempSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->zMTQ);
}
STATIC mp_obj_t tempSoh_bTempA(mp_obj_t self_in) {
    tempSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->bTempA);
}
STATIC mp_obj_t tempSoh_bTempB(mp_obj_t self_in) {
    tempSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->bTempB);
}
STATIC mp_obj_t tempSoh_timestamp(mp_obj_t self_in) {
    tempSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->timestamp);
}


STATIC mp_obj_t tempSoh_get_bytes(mp_obj_t self_in)
{
	tempSoh_obj_t* self = MP_OBJ_TO_PTR(self_in);
	
	mp_obj_array_t* retval = array_new(BYTEARRAY_TYPECODE, 15*sizeof(float));
	//uint8_t* dest = retval->items;
	//memcpy(retval->items, self, sizeof(adcsSoh_obj_t));
	uint8_t* src = (uint8_t*)self;
	src+=sizeof(mp_obj_base_t);
	memcpy(retval->items, src, sizeof(tempSoh_obj_t) - sizeof(mp_obj_base_t));

	return MP_OBJ_FROM_PTR(retval);
}

STATIC mp_obj_t tempSoh_from_bytes(mp_obj_t self_in, mp_obj_t new_data)
{
	
    tempSoh_obj_t *self = m_new_obj(tempSoh_obj_t);
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
        //size_t len = bufinfo.len / sz;
	}
    self->base.type = &tempSoh_type;

	uint8_t* dest = (uint8_t*)self;
	dest+= sizeof(mp_obj_base_t);
	memcpy(dest, bufinfo.buf, sizeof(tempSoh_obj_t)-sizeof(mp_obj_base_t));
	
	return MP_OBJ_FROM_PTR(self);	

}


MP_DEFINE_CONST_FUN_OBJ_1(tempSoh_get_bytes_obj, tempSoh_get_bytes);
MP_DEFINE_CONST_FUN_OBJ_2(tempSoh_from_bytes_obj, tempSoh_from_bytes);

STATIC const mp_rom_map_elem_t tempSoh_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_get_bytes), MP_ROM_PTR(&tempSoh_get_bytes_obj) },
    { MP_ROM_QSTR(MP_QSTR_from_bytes), MP_ROM_PTR(&tempSoh_from_bytes_obj) }
};

STATIC MP_DEFINE_CONST_DICT(tempSoh_locals_dict, tempSoh_locals_dict_table);
//define object with one parameter
/*MP_DEFINE_CONST_FUN_OBJ_1(tempSoh_battery_obj, tempSoh_battery);
MP_DEFINE_CONST_FUN_OBJ_1(tempSoh_ntc1_obj, tempSoh_ntc1);
MP_DEFINE_CONST_FUN_OBJ_1(tempSoh_ntc2_obj, tempSoh_ntc2);
MP_DEFINE_CONST_FUN_OBJ_1(tempSoh_ntc3_obj, tempSoh_ntc3);
MP_DEFINE_CONST_FUN_OBJ_1(tempSoh_ntc4_obj, tempSoh_ntc4);
MP_DEFINE_CONST_FUN_OBJ_1(tempSoh_a_obj, tempSoh_a);
MP_DEFINE_CONST_FUN_OBJ_1(tempSoh_b_obj, tempSoh_b);
MP_DEFINE_CONST_FUN_OBJ_1(tempSoh_c_obj, tempSoh_c);
MP_DEFINE_CONST_FUN_OBJ_1(tempSoh_d_obj, tempSoh_d);
MP_DEFINE_CONST_FUN_OBJ_1(tempSoh_timestamp_obj, tempSoh_timestamp);
//map qsttr for attribute to attribute object
STATIC const mp_rom_map_elem_t tempSoh_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_battery), MP_ROM_PTR(&tempSoh_battery_obj) },
    { MP_ROM_QSTR(MP_QSTR_ntc1), MP_ROM_PTR(&tempSoh_ntc1_obj) },
    { MP_ROM_QSTR(MP_QSTR_ntc2), MP_ROM_PTR(&tempSoh_ntc2_obj) },
    { MP_ROM_QSTR(MP_QSTR_ntc3), MP_ROM_PTR(&tempSoh_ntc3_obj) },
    { MP_ROM_QSTR(MP_QSTR_ntc4), MP_ROM_PTR(&tempSoh_ntc4_obj) },
    { MP_ROM_QSTR(MP_QSTR_a), MP_ROM_PTR(&tempSoh_a_obj) },
    { MP_ROM_QSTR(MP_QSTR_b), MP_ROM_PTR(&tempSoh_b_obj) },
    { MP_ROM_QSTR(MP_QSTR_c), MP_ROM_PTR(&tempSoh_c_obj) },
    { MP_ROM_QSTR(MP_QSTR_d), MP_ROM_PTR(&tempSoh_d_obj) },
    { MP_ROM_QSTR(MP_QSTR_timestamp), MP_ROM_PTR(&tempSoh_timestamp_obj) },
};

STATIC MP_DEFINE_CONST_DICT(tempSoh_locals_dict, tempSoh_locals_dict_table);
*/
//called by class_instance.attribute
//checks if attribute is a member of the internal data struct
//destination[0] is the output, must be mp_obj type
STATIC void tempSoh_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination) {
    switch (attribute) {
		case MP_QSTR_battery:
        destination[0] = tempSoh_battery(self_in);
    	break;
		case MP_QSTR_ntc1:
        destination[0] = tempSoh_ntc1(self_in);
    	break;
    	case MP_QSTR_ntc2:
        destination[0] = tempSoh_ntc2(self_in);
    	break;
		case MP_QSTR_ntc3:
        destination[0] = tempSoh_ntc3(self_in);
    	break;
    	case MP_QSTR_ntc4:
        destination[0] = tempSoh_ntc4(self_in);
    	break;
    	case MP_QSTR_a:
        destination[0] = tempSoh_a(self_in);
    	break;
		case MP_QSTR_b:
        destination[0] = tempSoh_b(self_in);
    	break;
		case MP_QSTR_c:
        destination[0] = tempSoh_c(self_in);
    	break;
		case MP_QSTR_d:
        destination[0] = tempSoh_d(self_in);
    	break;
		case MP_QSTR_xMTQ:
		destination[0] = tempSoh_xMTQ(self_in);
		break;
		case MP_QSTR_yMTQ:
		destination[0] = tempSoh_yMTQ(self_in);
		break;
		case MP_QSTR_zMTQ:
		destination[0] = tempSoh_zMTQ(self_in);
		break;
		case MP_QSTR_bTempA:
		destination[0] = tempSoh_bTempA(self_in);
		break;
		case MP_QSTR_bTempB:
		destination[0] = tempSoh_bTempB(self_in);
		break;
		case MP_QSTR_timestamp:
		destination[0] = tempSoh_timestamp(self_in);
		break;
	}
}

STATIC void tempSoh_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    (void)kind;
    tempSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    //mp_print_str(print, "powerSoh(");
	mp_print_str(print, " battery: ");
    mp_obj_print_helper(print, mp_obj_new_float(self->battery), PRINT_REPR);
	
	
	mp_print_str(print, ", ntc1: ");
    mp_obj_print_helper(print, mp_obj_new_float(self->ntc1), PRINT_REPR);
	
	mp_print_str(print, ", ntc2: ");
    mp_obj_print_helper(print, mp_obj_new_float(self->ntc2), PRINT_REPR);
	mp_print_str(print, ", ntc3: ");
    mp_obj_print_helper(print, mp_obj_new_float(self->ntc3), PRINT_REPR);
	mp_print_str(print, ", ntc4: ");
	mp_obj_print_helper(print, mp_obj_new_float(self->ntc4), PRINT_REPR);
	
	mp_print_str(print, ", a: ");
    mp_obj_print_helper(print,mp_obj_new_float(self->a), PRINT_REPR);
	mp_print_str(print, ", b: ");
    mp_obj_print_helper(print,mp_obj_new_float(self->b), PRINT_REPR);
	mp_print_str(print, ", c: ");
    mp_obj_print_helper(print,mp_obj_new_float(self->c), PRINT_REPR);
	mp_print_str(print, ", d: ");
    mp_obj_print_helper(print,mp_obj_new_float(self->d), PRINT_REPR);
	
	mp_print_str(print, ", xMTQ: ");
    mp_obj_print_helper(print,mp_obj_new_float(self->xMTQ), PRINT_REPR);
	mp_print_str(print, ", yMTQ: ");
    mp_obj_print_helper(print,mp_obj_new_float(self->yMTQ), PRINT_REPR);
	mp_print_str(print, ", zMTQ: ");
    mp_obj_print_helper(print,mp_obj_new_float(self->zMTQ), PRINT_REPR);
	mp_print_str(print, ", bTempA: ");
    mp_obj_print_helper(print,mp_obj_new_float(self->bTempA), PRINT_REPR);
	mp_print_str(print, ", bTempB: ");
    mp_obj_print_helper(print,mp_obj_new_float(self->bTempB), PRINT_REPR);
	
	
	mp_print_str(print, ", timestamp: ");
    mp_obj_print_helper(print, mp_obj_new_float(self->timestamp), PRINT_REPR);
    //mp_print_str(print, ")");
}


//register python object visible to interpreter
const mp_obj_type_t tempSoh_type = {
    { &mp_type_type },
    .name = MP_QSTR_tempSoh,
    .make_new = tempSoh_make_new,
    .attr = tempSoh_attr,
    .locals_dict = (mp_obj_dict_t*)&tempSoh_locals_dict,
	.print = tempSoh_print
};





///////////////////////////////////////////////////////
//define com class
//
typedef struct _comSoh_obj_t {
    mp_obj_base_t base;
    //lora
   	mp_float_t rssi;
	mp_int_t txcomplete;
	mp_int_t rxcomplete;
	mp_uint_t crc;
	mp_uint_t flags;
	//iridium
	mp_uint_t status;
	mp_int_t signal_quality;
	ndarray_obj_t* location;
	mp_float_t sys_time;
	mp_float_t energy;
		
	mp_float_t timestamp;
} comSoh_obj_t;

const mp_obj_type_t comSoh_type;

STATIC mp_obj_t comSoh_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    
	
	
	enum {ARG_rssi, ARG_txcomplete, ARG_rxcomplete, ARG_crc, ARG_flags, ARG_status, ARG_signal_quality, ARG_location, ARG_sys_time, ARG_timestamp,  NUM_ARGS};
	static const mp_arg_t allowed_args[] = {
		{MP_QSTR_rssi, MP_ARG_OBJ, {.u_obj =mp_const_none}},
		{MP_QSTR_txcomplete, MP_ARG_INT, {.u_int = INT_ERROR}},
		{MP_QSTR_rxcomplete, MP_ARG_INT, {.u_int = INT_ERROR}},
		{MP_QSTR_crc, MP_ARG_INT, {.u_int = INT_ERROR}},
		{MP_QSTR_flags, MP_ARG_INT, {.u_int = INT_ERROR}},
		{MP_QSTR_status, MP_ARG_INT, {.u_int = INT_ERROR}},
		{MP_QSTR_signal_quality, MP_ARG_INT, {.u_int = INT_ERROR}},
		{MP_QSTR_location, MP_ARG_OBJ, {.u_obj =mp_const_none}},
		{MP_QSTR_sys_time, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_timestamp, MP_ARG_OBJ, {.u_obj = mp_const_none}},
	};
	
	mp_arg_val_t arg_vals[MP_ARRAY_SIZE(allowed_args)];
	mp_arg_parse_all_kw_array(n_args, n_kw, args, MP_ARRAY_SIZE(allowed_args), allowed_args, arg_vals);
	
	
	
	
    comSoh_obj_t *self = m_new_obj(comSoh_obj_t);
    self->base.type = &comSoh_type;
    self->rssi = getFloat(arg_vals[ARG_rssi].u_obj);
    self->txcomplete = arg_vals[ARG_txcomplete].u_int;
    self->rxcomplete = arg_vals[ARG_rxcomplete].u_int;
    self->crc = arg_vals[ARG_crc].u_int;
    self->flags = arg_vals[ARG_flags].u_int;
    self->signal_quality = arg_vals[ARG_signal_quality].u_int;
    self->status = arg_vals[ARG_status].u_int;
    
	self->location= getNumpyArray(arg_vals[ARG_location].u_obj);
    self->sys_time = getFloat(arg_vals[ARG_sys_time].u_obj);
    self->timestamp = getFloat(arg_vals[ARG_timestamp].u_obj);




	return MP_OBJ_FROM_PTR(self);
}
//getter only visible to c interface, called by propertyclass_attr
//if attr qstr == x's qster

STATIC mp_obj_t comSoh_rssi(mp_obj_t self_in) {
    comSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->rssi);
}
STATIC mp_obj_t comSoh_txcomplete(mp_obj_t self_in) {
    comSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->txcomplete);
}
STATIC mp_obj_t comSoh_rxcomplete(mp_obj_t self_in) {
    comSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->rxcomplete);
}
STATIC mp_obj_t comSoh_crc(mp_obj_t self_in) {
    comSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->crc);
}
STATIC mp_obj_t comSoh_flags(mp_obj_t self_in) {
    comSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->flags);
}
STATIC mp_obj_t comSoh_status(mp_obj_t self_in) {
    comSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(self->status);
}
STATIC mp_obj_t comSoh_signal_quality(mp_obj_t self_in) {
    comSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(self->signal_quality);
}
STATIC mp_obj_t comSoh_location(mp_obj_t self_in) {
    comSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return MP_OBJ_FROM_PTR(self->location);
}
STATIC mp_obj_t comSoh_sys_time(mp_obj_t self_in) {
    comSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->sys_time);
}
STATIC mp_obj_t comSoh_energy(mp_obj_t self_in) {
    comSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->energy);
}


STATIC mp_obj_t comSoh_timestamp(mp_obj_t self_in) {
	comSoh_obj_t* self = MP_OBJ_TO_PTR(self_in);
	return mp_obj_new_float(self->timestamp);
}



STATIC mp_obj_t comSoh_get_bytes(mp_obj_t self_in)
{
	comSoh_obj_t* self = MP_OBJ_TO_PTR(self_in);
	
	mp_obj_array_t* retval = array_new(BYTEARRAY_TYPECODE,3*sizeof(mp_uint_t) + 3* sizeof(mp_int_t) + 6*sizeof(float));
	uint8_t* dest = retval->items;
	//memcpy(retval->items, self, sizeof(adcsSoh_obj_t));
	uint8_t* src = (uint8_t*)self;
	src+=sizeof(mp_obj_base_t);
	size_t n =  sizeof(mp_float_t) + 3*sizeof(mp_int_t) + 3*sizeof(mp_uint_t);
	//uint8_t* dest = retval->items;
	memcpy(dest, src, n);
	dest+=n;
	//self+=n;
	copy_np_array(self->location, dest);
	dest+= 3 * sizeof(mp_float_t);
	memcpy(dest, &self->sys_time, sizeof(mp_float_t));
	dest+= sizeof(mp_float_t);
	memcpy(dest, &self->timestamp, sizeof(mp_float_t));


	return MP_OBJ_FROM_PTR(retval);
}

STATIC mp_obj_t comSoh_from_bytes(mp_obj_t self_in, mp_obj_t new_data)
{
	
    comSoh_obj_t *self = m_new_obj(comSoh_obj_t);
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
        //size_t len = bufinfo.len / sz;
	}
    self->base.type = &comSoh_type;

	uint8_t* dest = (uint8_t*)self;
	dest+= sizeof(mp_obj_base_t);
	size_t n =  sizeof(mp_float_t) + 3*sizeof(mp_int_t) + 3*sizeof(mp_uint_t);
	memcpy(dest, bufinfo.buf, n);
	bufinfo.buf+=n;
	self->location = copy_to_np_array(self->location, bufinfo.buf, 3, sizeof(float));
	dest+= sizeof(ndarray_obj_t*);//3 *sizeof(float);
	bufinfo.buf+= 3*sizeof(float);
	//memcpy(dest, bufinfo.buf, 2*sizeof(float));
	memcpy(&self->sys_time, bufinfo.buf, sizeof(float));
	bufinfo.buf+= sizeof(float);
	memcpy(&self->timestamp, bufinfo.buf, sizeof(float));

	return MP_OBJ_FROM_PTR(self);	

}



MP_DEFINE_CONST_FUN_OBJ_1(comSoh_get_bytes_obj, comSoh_get_bytes);
MP_DEFINE_CONST_FUN_OBJ_2(comSoh_from_bytes_obj, comSoh_from_bytes);
STATIC const mp_rom_map_elem_t comSoh_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_get_bytes), MP_ROM_PTR(&comSoh_get_bytes_obj) },
    { MP_ROM_QSTR(MP_QSTR_from_bytes), MP_ROM_PTR(&comSoh_from_bytes_obj) }
};
STATIC MP_DEFINE_CONST_DICT(comSoh_locals_dict, comSoh_locals_dict_table);


/*
//define object with one parameter
MP_DEFINE_CONST_FUN_OBJ_1(comSoh_rssi_obj, comSoh_rssi);
MP_DEFINE_CONST_FUN_OBJ_1(comSoh_txcomplete_obj, comSoh_txcomplete);
MP_DEFINE_CONST_FUN_OBJ_1(comSoh_rxcomplete_obj, comSoh_rxcomplete);
MP_DEFINE_CONST_FUN_OBJ_1(comSoh_crc_obj, comSoh_crc);
MP_DEFINE_CONST_FUN_OBJ_1(comSoh_flags_obj, comSoh_flags);
MP_DEFINE_CONST_FUN_OBJ_1(comSoh_status_obj, comSoh_status);
MP_DEFINE_CONST_FUN_OBJ_1(comSoh_signal_quality_obj, comSoh_signal_quality);
MP_DEFINE_CONST_FUN_OBJ_1(comSoh_location_obj, comSoh_location);
MP_DEFINE_CONST_FUN_OBJ_1(comSoh_sys_time_obj, comSoh_sys_time);
MP_DEFINE_CONST_FUN_OBJ_1(comSoh_energy_obj, comSoh_energy);
MP_DEFINE_CONST_FUN_OBJ_1(comSoh_timestamp_obj, comSoh_timestamp);
//map qsttr for attribute to attribute object
STATIC const mp_rom_map_elem_t comSoh_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_rssi), MP_ROM_PTR(&comSoh_rssi_obj) },
    { MP_ROM_QSTR(MP_QSTR_txcomplete), MP_ROM_PTR(&comSoh_txcomplete_obj) },
    { MP_ROM_QSTR(MP_QSTR_rxcomplete), MP_ROM_PTR(&comSoh_rxcomplete_obj) },
    { MP_ROM_QSTR(MP_QSTR_crc), MP_ROM_PTR(&comSoh_crc_obj) },
    { MP_ROM_QSTR(MP_QSTR_flags), MP_ROM_PTR(&comSoh_flags_obj) },
    
    { MP_ROM_QSTR(MP_QSTR_status), MP_ROM_PTR(&comSoh_status_obj) },
    { MP_ROM_QSTR(MP_QSTR_signal_quality), MP_ROM_PTR(&comSoh_signal_quality_obj) },
    { MP_ROM_QSTR(MP_QSTR_location), MP_ROM_PTR(&comSoh_location_obj) },
    { MP_ROM_QSTR(MP_QSTR_sys_time), MP_ROM_PTR(&comSoh_sys_time_obj) },
    { MP_ROM_QSTR(MP_QSTR_energy), MP_ROM_PTR(&comSoh_energy_obj) },
	
	{ MP_ROM_QSTR(MP_QSTR_timestamp), MP_ROM_PTR(&comSoh_timestamp_obj) },
};

STATIC MP_DEFINE_CONST_DICT(comSoh_locals_dict, comSoh_locals_dict_table);
*/
//called by class_instance.attribute
//checks if attribute is a member of the internal data struct
//destination[0] is the output, must be mp_obj type
STATIC void comSoh_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination) {
    switch (attribute) {
		case MP_QSTR_rssi:
        destination[0] = comSoh_rssi(self_in);
    	break;
		case MP_QSTR_txcomplete:
        destination[0] = comSoh_txcomplete(self_in);
    	break;
		case MP_QSTR_rxcomplete:
        destination[0] = comSoh_rxcomplete(self_in);
    	break;
    	case MP_QSTR_crc:
        destination[0] = comSoh_crc(self_in);
    	break;
		case MP_QSTR_flags:
        destination[0] = comSoh_flags(self_in);
    	break;
	
		case MP_QSTR_status:
        destination[0] = comSoh_status(self_in);
    	break;
		case MP_QSTR_signal_quality:
        destination[0] = comSoh_signal_quality(self_in);
    	break;
		case MP_QSTR_location:
        destination[0] = comSoh_location(self_in);
    	break;
		case MP_QSTR_sys_time:
        destination[0] = comSoh_sys_time(self_in);
    	break;
		case MP_QSTR_energy:
        destination[0] = comSoh_energy(self_in);
    	break;
	
		case MP_QSTR_timestamp:
		destination[0] = comSoh_timestamp(self_in);
		break;
	}

}




STATIC void comSoh_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    (void)kind;
    comSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    //mp_print_str(print, "comSoh(");
	mp_print_str(print, " rssi: ");
    mp_obj_print_helper(print, mp_obj_new_float(self->rssi), PRINT_REPR);
	
	
	
	mp_print_str(print, ", txcomplete: ");
    mp_obj_print_helper(print,mp_obj_new_int(self->txcomplete), PRINT_REPR);
	
	
	mp_print_str(print, ", rxcomplete: ");
    mp_obj_print_helper(print, mp_obj_new_int(self->rxcomplete), PRINT_REPR);
	
	mp_print_str(print, ", crc: ");
    mp_obj_print_helper(print, mp_obj_new_int(self->rxcomplete), PRINT_REPR);
	
	mp_print_str(print, ", flags: ");
    mp_obj_print_helper(print, mp_obj_new_int(self->rxcomplete), PRINT_REPR);


	mp_print_str(print, ", rb_status: ");
    mp_obj_print_helper(print, mp_obj_new_int(self->status), PRINT_REPR);
	
	mp_print_str(print, ", rb_signal_quality: ");
    mp_obj_print_helper(print, mp_obj_new_int(self->signal_quality), PRINT_REPR);
	

	mp_print_str(print, ", rb_location: ");
    mp_obj_print_helper(print, MP_OBJ_FROM_PTR(self->location), PRINT_REPR);
	
	mp_print_str(print, ", sys_time: ");
    mp_obj_print_helper(print, mp_obj_new_float(self->sys_time), PRINT_REPR);
	
	mp_print_str(print, ", timestamp: ");
    mp_obj_print_helper(print, mp_obj_new_float(self->timestamp), PRINT_REPR);
    //mp_print_str(print, ")");
}

//register python object visible to interpreter
const mp_obj_type_t comSoh_type = {
    { &mp_type_type },
    .name = MP_QSTR_comSoh,
    .make_new = comSoh_make_new,
    .attr = comSoh_attr,
    .locals_dict = (mp_obj_dict_t*)&comSoh_locals_dict,
	.print = comSoh_print,
};






//register globals constants and dicts 
//
//
STATIC const mp_map_elem_t soh_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_soh) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_gpsSoh), (mp_obj_t)&gpsSoh_type },

    { MP_OBJ_NEW_QSTR(MP_QSTR_adcsSoh), (mp_obj_t)&adcsSoh_type },	
    { MP_OBJ_NEW_QSTR(MP_QSTR_powerSoh), (mp_obj_t)&powerSoh_type },	
    { MP_OBJ_NEW_QSTR(MP_QSTR_tempSoh), (mp_obj_t)&tempSoh_type },	
    { MP_OBJ_NEW_QSTR(MP_QSTR_comSoh), (mp_obj_t)&comSoh_type },	
};

STATIC MP_DEFINE_CONST_DICT (
    mp_module_soh_globals,
    soh_globals_table
);

const mp_obj_module_t soh_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_soh_globals,
};

MP_REGISTER_MODULE(MP_QSTR_soh, soh_user_cmodule, 1/*MODULE_PROPERTYCLASS_ENABLED*/);









