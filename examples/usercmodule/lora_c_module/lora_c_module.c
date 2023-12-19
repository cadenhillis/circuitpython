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
#include "lora_c_module.h"
#include "py/objarray.h"
#include "extmod/ulab/code/ndarray.h"



STATIC void initRegisterBits(registerBits* rb, uint8_t address, uint8_t offset, uint8_t bits)
{
	rb->address = address;
	rb->mask = 0;
	for (uint8_t i=0; i<bits; i++) {
		rb->mask <<= 1;
		rb->mask |=1;
	}
	rb->mask <<= offset;
	rb->offset = offset;

}

uint8_t lora_tx_done(lora_driver_obj_t* self)
{
	uint8_t reg_value ; 
	common_hal_busio_spi_read(self->spi, &reg_value, 1, _RH_RF95_REG_12_IRQ_FLAGS);
	return (reg_value & 0x8) >> 3;
}

void lora_listen(lora_driver_obj_t* self)
{
	set_register(&self->operation_mode, self->spi, RX_MODE);
	set_register(&self->dio0_mapping, self->spi, 0);
}

void lora_transmit(lora_driver_obj_t* self)
{
	set_register(&self->operation_mode, self->spi, TX_MODE);
	set_register(&self->dio0_mapping, self->spi, 1);
}

void set_tx_power(lora_driver_obj_t* self, uint8_t val)
{
	if (self->max_output) {
	uint8_t buf[] = {_RH_RF95_REG_0B_OCP, 0x3F};
	common_hal_busio_spi_write(self->spi, buf, 2);
	set_register(&self->pa_dac, self->spi, _RH_RF95_PA_DAC_ENABLE);	
	set_register(&self->pa_select, self->spi, 1);
	set_register(&self->max_power, self->spi, 7);
	set_register(&self->output_power, self->spi, 0x0F);
	//NOT DONE!
	}
	else if (self->high_power) {
		if (val < 5 || val > 23) mp_raise_ValueError(MP_ERROR_TEXT("tx power must be between 5 and 23"));
		if (val > 20) {
			set_register(&self->pa_dac, self->spi, _RH_RF95_PA_DAC_ENABLE);	
			val-=3;
		}
		else {
			set_register(&self->pa_dac, self->spi, _RH_RF95_PA_DAC_DISABLE);	
		}

		set_register(&self->pa_select, self->spi, 1);
		set_register(&self->max_power, self->spi, (val - 5) & 0x0F);
		
	 }

	else {
		set_register(&self->pa_select, self->spi, 0);
		set_register(&self->max_power, self->spi, 7);
		set_register(&self->output_power, self->spi, (val + 1) & 0x0F);
		
	}
}

void lora_reset(lora_driver_obj_t* self)
{
	common_hal_digitalio_digitalinout_switch_to_output(self->rst, false, PULL_UP);
	common_hal_mcu_delay_us(100);
	common_hal_digitalio_digitalinout_switch_to_input(self->rst,PULL_UP );
	common_hal_mcu_delay_us(5000);
}

void lora_sleep(lora_driver_obj_t* self)
{

	set_register(&self->operation_mode, self->spi, SLEEP_MODE); 
}

void idle(lora_driver_obj_t* self)
{
	set_register(&self->operation_mode, self->spi, STANDBY_MODE); 
}


uint8_t get_register(registerBits* rb, busio_spi_obj_t* spi)
{
	uint8_t reg_value;
	common_hal_busio_spi_read(spi, &reg_value, 1, rb->address);
	return (reg_value & rb->mask) >> rb->offset;
}

void set_register(registerBits* rb, busio_spi_obj_t* spi, uint8_t val)
{
	uint8_t reg_value;
	common_hal_busio_spi_read(spi, &reg_value, 1, rb->address);
	reg_value &= ~rb->mask;
	reg_value |= (val & 0xFF) << rb->offset;
	uint8_t write_val[] = {(rb->address | 0x80) & 0xFF, reg_value};
	common_hal_busio_spi_write(spi, write_val, 2);
}

STATIC mp_obj_t lora_driver_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 9, 9, true);
    lora_driver_obj_t *self = m_new_obj(lora_driver_obj_t);
    self->base.type = &lora_driver_type;


	enum { ARG_spi, ARG_cs, ARG_rst, NUM_ARGS };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_spi, MP_ARG_OBJ, {.u_obj = mp_const_none } },
        { MP_QSTR_cs, MP_ARG_OBJ, {.u_obj = mp_const_none } },
        { MP_QSTR_rst, MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };

	mp_arg_val_t args2[MP_ARRAY_SIZE(allowed_args)];
    //mp_arg_parse_all_kw_array(n_args, n_kw, args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
	mp_arg_parse_all_kw_array(n_args, n_kw, args, MP_ARRAY_SIZE(allowed_args), allowed_args, args2);





	busio_spi_obj_t *spi = validate_obj_is_spi_bus(args2[0].u_obj, MP_QSTR_spi);
	const mcu_pin_obj_t* cs = validate_obj_is_free_pin(args2[1].u_obj, MP_QSTR_cs);
	const mcu_pin_obj_t* rst = validate_obj_is_free_pin(args2[2].u_obj, MP_QSTR_rst);

	digitalinout_result_t result = common_hal_digitalio_digitalinout_construct(self->cs, cs);
    if (result != DIGITALINOUT_OK) {
        mp_raise_ValueError_varg(MP_ERROR_TEXT("%q init failed"), MP_QSTR_cs);
    }

	result = common_hal_digitalio_digitalinout_construct(self->rst, rst);
    if (result != DIGITALINOUT_OK) {
        mp_raise_ValueError_varg(MP_ERROR_TEXT("%q init failed"), MP_QSTR_cs);
    }

	self->frequency_mhz = mp_obj_get_int(args[3]);
	self->preamble_length = mp_obj_get_int(args[4]);
	self->coding_rate = mp_obj_get_int(args[5]);
	self->high_power = mp_obj_get_int(args[6]);
	self->baudrate = mp_obj_get_int(args[7]);
	self->max_output = mp_obj_get_int(args[8]);

	self->dio0 = false;


	initRegisterBits(&self->operation_mode, _RH_RF95_REG_01_OP_MODE, 0, 3);
	initRegisterBits(&self->low_frequency_mode, _RH_RF95_REG_01_OP_MODE, 3, 1);
	initRegisterBits(&self->osc_calibration, _RH_RF95_REG_24_HOP_PERIOD, 3, 1);
	initRegisterBits(&self->modulation_type, _RH_RF95_REG_01_OP_MODE, 5, 2);
	initRegisterBits(&self->long_range_mode, _RH_RF95_REG_01_OP_MODE, 7, 1);
	initRegisterBits(&self->lna_boost, _RH_RF95_REG_0C_LNA, 0, 2);
	initRegisterBits(&self->output_power, _RH_RF95_REG_09_PA_CONFIG, 0, 4);
	initRegisterBits(&self->modulation_shaping, _RH_RF95_REG_0A_PA_RAMP, 0, 2);
	initRegisterBits(&self->pa_ramp, _RH_RF95_REG_0A_PA_RAMP, 0, 4);
	initRegisterBits(&self->max_power, _RH_RF95_REG_09_PA_CONFIG, 4, 3);
	initRegisterBits(&self->pa_select, _RH_RF95_REG_09_PA_CONFIG, 7, 1);
	initRegisterBits(&self->pa_dac, _RH_RF95_REG_4D_PA_DAC, 0, 3);
	initRegisterBits(&self->dio0_mapping, _RH_RF95_REG_40_DIO_MAPPING1, 6, 2);
	initRegisterBits(&self->low_datarate_optimize, _RH_RF95_REG_26_MODEM_CONFIG3, 3, 1);
	initRegisterBits(&self->auto_agc, _RH_RF95_REG_26_MODEM_CONFIG3, 2, 1);

	//self.reset()
	common_hal_digitalio_digitalinout_switch_to_input(self->rst,PULL_UP );
	lora_reset(self);
	
	
	uint8_t version;
	common_hal_busio_spi_read(spi, &version, 1, _RH_RF95_REG_42_VERSION);
	if (version != 18) {
		mp_raise_ValueError(MP_ERROR_TEXT("failed to find rfm9x with expected version. Check wiring"));
	}


	//
	
	
	idle(self);
	common_hal_mcu_delay_us(1000);
	//sleep(.01); //time.sleep(.01)
	set_register(&self->osc_calibration, self->spi, 1);
	
	common_hal_mcu_delay_us(1000);//sleep(1);

	//self.sleep()
	lora_sleep(self);
	common_hal_mcu_delay_us(1000);// sleep(.01); //time.sleep(.01)
	set_register(&self->long_range_mode, self->spi, 1);
	if (get_register(&self->operation_mode, self->spi) != SLEEP_MODE || !(get_register(&self->long_range_mode, self->spi)))
		mp_raise_ValueError(MP_ERROR_TEXT("failed to configure rfm9x for lora mode. Check wiring"));
	if (get_register(&self->operation_mode, self->spi) != SLEEP_MODE)
		mp_raise_ValueError(MP_ERROR_TEXT("failed to configure rfm9x for lora mode"));
	
	if (self->frequency_mhz > 525) set_register(&self->low_frequency_mode, self->spi,0);

	uint8_t buffer[] = {_RH_RF95_REG_0E_FIFO_TX_BASE_ADDR, 0, _RH_RF95_REG_0F_FIFO_RX_BASE_ADDR, 0, _RH_RF95_REG_24_HOP_PERIOD, 0};

	common_hal_busio_spi_write(self->spi, buffer, 2);
	common_hal_busio_spi_write(self->spi, buffer + 2, 2);
	common_hal_busio_spi_write(self->spi, buffer + 4, 2);

	idle(self);


	self->signal_bandwidth = 125000;
	self->spreading_factor = 7;
	self->enable_crc = false;

	uint8_t buffer2[] = {_RH_RF95_REG_26_MODEM_CONFIG3, 0};
	common_hal_busio_spi_write(self->spi, buffer2, 2);

	self->tx_power = 13;
	self->last_rssi = 0.0;



	self->ack_wait = 0.5;
	self->receive_timeout = .5;
	self->xmit_timeout = 2;
	self->ack_retries = 5;
	self->ack_delay = 0;
	self->sequence_number = 0;
	
	self->node = _RH_BROADCAST_ADDRESS;

	self->destination = _RH_BROADCAST_ADDRESS;


	self->identifier = 0;
	self->flags = 0;
	self->crc_error_count = 0;

	set_register(&self->auto_agc, self->spi, 1);
	set_register(&self->pa_ramp, self->spi, 0);
	set_register(&self->lna_boost, self->spi, 1);


	return MP_OBJ_FROM_PTR(self);
}


  
//getter only visible to c interface, called by propertyclass_attr
//if attr qstr == x's qster
STATIC mp_obj_t lora_driver_send(size_t n_args, const mp_obj_t *args) {
//STATIC mp_obj_t lora_driver_send(mp_obj_t self_in, mp_obj_t data, mp_obj_t keep_listening, mp_obj_t destination, mp_obj_t node, mp_obj_t identifier, mp_obj_t flags) {
    //lora_driver_obj_t *self = m_new_obj(lora_driver_obj_t);

	//parameters:
	//self, data, keep_listening, destination,node,identifier,flags

	lora_driver_obj_t* self =  MP_OBJ_FROM_PTR(args[0]);

	//get data buffer
	mp_buffer_info_t bufinfo;		
    size_t len, sz;
	if (((MICROPY_PY_BUILTINS_BYTEARRAY)
         || (MICROPY_PY_ARRAY
             && (mp_obj_is_type(args[1], &mp_type_bytes)
                 || (MICROPY_PY_BUILTINS_BYTEARRAY && mp_obj_is_type(args[1], &mp_type_bytearray)))))
        && mp_get_buffer(args[1], &bufinfo, MP_BUFFER_READ)) {
        // construct array from raw bytes
        sz = mp_binary_get_size('@', BYTEARRAY_TYPECODE, NULL);
        if (bufinfo.len % sz) {
            mp_raise_ValueError(MP_ERROR_TEXT("bytes length not a multiple of item size"));
        }
        len = bufinfo.len / sz;
		if (len >252) mp_raise_ValueError(MP_ERROR_TEXT("data must be 252 bytes or less"));
		//memcpy(self->items[self->inptr].data, bufinfo.buf, len * sz);
		
	}

	
	bool keep_listening = mp_obj_get_int(args[2]);
	uint8_t destination = mp_obj_get_int(args[3]);
	uint8_t node = mp_obj_get_int(args[4]);
	uint8_t identifier = mp_obj_get_int(args[5]);
	uint8_t flags = mp_obj_get_int(args[6]);

	idle(self);




	uint8_t buf[] = {_RH_RF95_REG_0D_FIFO_ADDR_PTR, 0};
	common_hal_busio_spi_write(self->spi, buf, 2);

	if (destination) self->payload[0] = destination;
	else self->payload[0] = self->destination;
	
	if (node) self->payload[1] = node;
	else self->payload[1] = self->node;
	
	if (identifier) self->payload[2] = identifier;
	else self->payload[2] = self->identifier;

	if (flags) self->payload[3] = flags;
	else self->payload[3] = self->flags;
	
	memcpy(self->payload + 4, bufinfo.buf, len*sz);

	len+=4;

	//_write_from
	buf[0] = (_RH_RF95_REG_00_FIFO | 0x80) & 0xFF;
	common_hal_busio_spi_write(self->spi, buf, 1);
	common_hal_busio_spi_write(self->spi, self->payload, len);
	buf[0] = _RH_RF95_REG_22_PAYLOAD_LENGTH;
	buf[1] = len;
	common_hal_busio_spi_write(self->spi, buf, 2);

	lora_transmit(self);



	//timeout stuff
	bool timed_out = false;
	uint64_t start = supervisor_ticks_ms64();//common_hal_time_monotonic_ms();// mp_hal_time_ns(); //time.monotinic
	while (!timed_out && !lora_tx_done(self)) {
		if (supervisor_ticks_ms64() - start >= self->xmit_timeout) timed_out = true;
	}
	
	if (keep_listening) lora_listen(self);
	else idle(self);


	buf[0] = _RH_RF95_REG_12_IRQ_FLAGS;
	buf[1] = 0xFF;
	common_hal_busio_spi_write(self->spi, buf, 2);
    return mp_obj_new_int(timed_out);
}

//define object with one parameter
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(lora_driver_send_obj,2, 7, lora_driver_send);
//map qsttr for attribute to attribute object
STATIC const mp_rom_map_elem_t lora_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_send), MP_ROM_PTR(&lora_driver_send_obj) },
};



STATIC MP_DEFINE_CONST_DICT(lora_locals_dict, lora_locals_dict_table);

//called by class_instance.attribute
//checks if attribute is a member of the internal data struct
//destination[0] is the output, must be mp_obj type
//STATIC void lora_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination) {

//}

//define how the class will be printed
STATIC void lora_driver_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    (void)kind;
    //lora_driver_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_print_str(print, "class(");
	
}



//register python object visible to interpreter
const mp_obj_type_t lora_driver_type = {
    { &mp_type_type },
    .name = MP_QSTR_lora_driver,
    .make_new = lora_driver_make_new,
    //.attr = adcsSoh_attr,
    .locals_dict = (mp_obj_dict_t*)&lora_locals_dict,
	.print = lora_driver_print
};












//register globals constants and dicts 
//
//
STATIC const mp_map_elem_t lora_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_rfm9x) },

    { MP_OBJ_NEW_QSTR(MP_QSTR_lora_driver), (mp_obj_t)&lora_driver_type },	
};

STATIC MP_DEFINE_CONST_DICT (
    mp_module_lora_globals,
    lora_globals_table
);

const mp_obj_module_t lora_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_lora_globals,
};

MP_REGISTER_MODULE(MP_QSTR_lora, lora_user_cmodule, 1/*MODULE_PROPERTYCLASS_ENABLED*/);









