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
#include "examplemodule.h"
#include "py/objarray.h"
#include "extmod/ulab/code/ndarray.h"
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
    mp_arg_check_num(n_args, n_kw, 7, 7, true);
    adcsSoh_obj_t *self = m_new_obj(adcsSoh_obj_t);
    self->base.type = &adcsSoh_type;

    self->adcsstatus = mp_obj_get_int(args[0]);
    self->state = mp_obj_get_int(args[1]);
	self->moment = getNumpyArray(args[2]);
	self->angularVelocity = getNumpyArray(args[3]);
	self->quat = getNumpyArray(args[4]);
	self->adcstime = mp_obj_get_float(args[5]);
	self->timestamp = mp_obj_get_float(args[6]);	

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
    //.locals_dict = (mp_obj_dict_t*)&adcsSoh_locals_dict,
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
    mp_arg_check_num(n_args, n_kw, 7, 7, true);
    gpsSoh_obj_t *self = m_new_obj(gpsSoh_obj_t);
    self->base.type = &gpsSoh_type;
    self->gpstime = mp_obj_get_float(args[0]);
    self->alt = mp_obj_get_float(args[1]);
    self->lat = mp_obj_get_float(args[2]);
    self->lon = mp_obj_get_float(args[3]);
    self->quality = mp_obj_get_int(args[4]);
    self->timestamp = mp_obj_get_float(args[5]);
    self->speed_knots = mp_obj_get_float(args[6]);
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
    //.locals_dict = (mp_obj_dict_t*)&gpsSoh_locals_dict,
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
    mp_arg_check_num(n_args, n_kw, 21, 21, true);
    powerSoh_obj_t *self = m_new_obj(powerSoh_obj_t);
    self->base.type = &powerSoh_type;
    self->systemV = mp_obj_get_float(args[0]);
    self->battV = mp_obj_get_float(args[1]);
    self->busI = mp_obj_get_float(args[2]);
    self->timestamp = mp_obj_get_float(args[3]);
    
	
    self->solar_charge_1  = mp_obj_get_float(args[4]);
    self->solar_charge_2  = mp_obj_get_float(args[6]);
    self->charge_current  = mp_obj_get_float(args[7]);
    self->charge_voltage  = mp_obj_get_float(args[8]);
    self->battery_voltage = mp_obj_get_float(args[9]);
    self->battery_current = mp_obj_get_float(args[10]);
    self->bus_voltage     = mp_obj_get_float(args[11]);
    self->bus_current     = mp_obj_get_float(args[12]);
    self->v3v3_voltage    = mp_obj_get_float(args[13]);
    self->v3v3_current    = mp_obj_get_float(args[14]);
    self->payload_voltage = mp_obj_get_float(args[15]);
    self->payload_current = mp_obj_get_float(args[16]);
    self->rf_voltage      = mp_obj_get_float(args[17]);
    self->rf_current      = mp_obj_get_float(args[18]);
    self->solar1_voltage  = mp_obj_get_float(args[19]);
    self->solar2_voltage  = mp_obj_get_float(args[20]);
		
	
	
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
    //.locals_dict = (mp_obj_dict_t*)&powerSoh_locals_dict,
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
	mp_float_t timestamp;
} tempSoh_obj_t;

const mp_obj_type_t tempSoh_type;

STATIC mp_obj_t tempSoh_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 10, 10, true);
    tempSoh_obj_t *self = m_new_obj(tempSoh_obj_t);
    self->base.type = &tempSoh_type;
    self->battery = mp_obj_get_float(args[0]);
    self->ntc1 = mp_obj_get_float(args[1]);
    self->ntc2 = mp_obj_get_float(args[2]);
    self->ntc3 = mp_obj_get_float(args[3]);
    self->ntc4 = mp_obj_get_float(args[4]);
    self->a = mp_obj_get_float(args[5]);
    self->b = mp_obj_get_float(args[6]);
    self->c = mp_obj_get_float(args[7]);
    self->d = mp_obj_get_float(args[8]);
    self->timestamp = mp_obj_get_float(args[9]);
   	 
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
STATIC mp_obj_t tempSoh_timestamp(mp_obj_t self_in) {
    tempSoh_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->timestamp);
}
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
    //.locals_dict = (mp_obj_dict_t*)&tempSoh_locals_dict,
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
    mp_arg_check_num(n_args, n_kw, 10, 10, true);
    comSoh_obj_t *self = m_new_obj(comSoh_obj_t);
    self->base.type = &comSoh_type;
    self->rssi = mp_obj_get_float(args[0]);
    self->txcomplete = mp_obj_get_int(args[1]);
    self->rxcomplete = mp_obj_get_int(args[2]);
    self->crc = mp_obj_get_int(args[3]);
    self->flags = mp_obj_get_int(args[4]);
    
	self->status = mp_obj_get_int(args[5]);	
	self->signal_quality = mp_obj_get_int(args[6]);	
	self->location = getNumpyArray(args[7]);//mp_obj_get_float(args[7]);	
	self->sys_time = mp_obj_get_float(args[8]);	
	
	self->timestamp = mp_obj_get_float(args[9]);
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
   // .locals_dict = (mp_obj_dict_t*)&comSoh_locals_dict,
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









