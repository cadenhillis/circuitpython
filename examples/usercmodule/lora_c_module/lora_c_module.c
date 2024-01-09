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



/*
 * @brief write byte over spi bus to lora
 * should mimic def _write_u8(self,address,val):
 * @param self: pointer to lora driver struct
 * @param reg: register to be written to
 * @param val: value to be written to the aformentioned register
 * @author Owen DelBene
 * @date 12/20/2023
 */
void write_byte(lora_driver_obj_t* self, uint8_t reg, uint8_t val)
{
	common_hal_digitalio_digitalinout_set_value(&self->cs, false);
	self->buf[0] = (reg | 0x80) & 0xFF;
	self->buf[1] = val & 0xFF;	
	common_hal_busio_spi_write(self->spi, self->buf, 2);
	common_hal_digitalio_digitalinout_set_value(&self->cs, true);
}


/*
 * @brief write bytes over spi bus to lora
 * should mimic _write_from*self, addresss, buf, length=None)
 * @param self: pointer to lora driver struct
 * @param reg: register to be written to
 * @param val: array of values to be written to the aformentioned register
 * @param len: number of bytes in the val array
 * @author Owen DelBene
 * @date 12/20/2023
 */
void write_bytes(lora_driver_obj_t* self, uint8_t reg, uint8_t* val, size_t len)
{
	common_hal_digitalio_digitalinout_set_value(&self->cs, false);
	reg = (reg | 0x80) | 0xFF;
	common_hal_busio_spi_write(self->spi, &reg, 1);
	common_hal_busio_spi_write(self->spi, val, len);
	common_hal_digitalio_digitalinout_set_value(&self->cs, true);
}


/*
 * @brief read byte over spi bus to lora
 * @param self: pointer to lora driver struct
 * @param reg: register to be written to
 * @returns value stored in that register
 * @author Owen DelBene
 * @date 12/20/2023
 */
uint8_t read_byte(lora_driver_obj_t* self, uint8_t reg)
{

	common_hal_digitalio_digitalinout_set_value(&self->cs, false);
	uint8_t retval;
	common_hal_busio_spi_read(self->spi, &retval,1, reg);
	common_hal_digitalio_digitalinout_set_value(&self->cs, true);
	return retval;
}


/*
 * @brief read bytes over spi bus to lora
 * should mimic _read_into(self, addresss, buf, length=None):
 * @param self: pointer to lora driver struct
 * @param reg: register to be written to
 * @param buf: destination array
 * @param len: number of bytes expected to be read
 * @author Owen DelBene
 * @date 12/20/2023
 */
void read_bytes(lora_driver_obj_t* self, uint8_t reg, uint8_t* buf, size_t len)
{

	common_hal_digitalio_digitalinout_set_value(&self->cs, false);
	reg &=0x7F;
	common_hal_busio_spi_read(self->spi, buf,len, reg);
	common_hal_digitalio_digitalinout_set_value(&self->cs, true);
}




/*
 * @brief create a new array object that can be directly converted 
 * to a python object visible to the interpreter
 * @param typecode: the datatype the array will store
 * @param n: the number of elements in the array
 * @returns empty micropython array can be returned in a function
 * visible to the interpretr by passing into MP_OBJ_FROM_PTR()
 * @author Owen DelBene
 * @date 12/20/2023
 */
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


/*
 * @brief send acknowledge over lora, used in receive() to indicate success
 * @param self: lora driver struct used in receive
 * @param destination: used in receive()
 * @param node: used in receive()
 * @param identifier: used in receive()
 * @param keep_listeneing: used in receive()
 * @author Owen DelBene
 * @date 12/20/2023
 */
bool lora_send_ack(lora_driver_obj_t* self,uint8_t destination, uint8_t node, uint8_t identifier, uint8_t flags, bool keep_listening)
{

	idle(self);


	uint8_t len = 5;

	
	write_byte(self, _RH_RF95_REG_0D_FIFO_ADDR_PTR, 0);
	
	if (destination) self->payload[0] = destination;
	else self->payload[0] = self->destination;
	
	if (node) self->payload[1] = node;
	else self->payload[1] = self->node;
	
	if (identifier) self->payload[2] = identifier;
	else self->payload[2] = self->identifier;

	if (flags) self->payload[3] = flags;
	else self->payload[3] = self->flags;

	uint8_t data = 33; //b'!', the ACK code for lora	
	self->payload[4] = data;

	//_write_from
	write_bytes(self, _RH_RF95_REG_00_FIFO, self->payload, len); 
	
	write_byte(self, _RH_RF95_REG_22_PAYLOAD_LENGTH, len);
	lora_transmit(self);



	//timeout stuff
	bool timed_out = false;
	uint64_t start = supervisor_ticks_ms64();
	while (!timed_out && !lora_tx_done(self)) {
		if (supervisor_ticks_ms64() - start >= self->xmit_timeout) timed_out = true;
	}
	
	if (keep_listening) lora_listen(self);
	else idle(self);


	write_byte(self, _RH_RF95_REG_12_IRQ_FLAGS, 0xFF);
	return !timed_out;
}


/* 
 * @brief Initialize the Registerbits structs, meant to mimic the __init__
 * function for the registerBits subclass in pycubed_rfm9x.py
 * @param rb: the registerbits to be initialized
 * @param address: the value of the register
 * @param offset: how many bits into the register the value will occur
 * ranges from 0 to 7
 * @param bits: how many bits wide is the value, ranges from 1 to (8-offset)
 * @author Owen DelBene
 * @date 12/20/2023
 */
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



/*
 * @brief check if the receiver encountered a crc error in the last transmission
 * @param self: lora driver struct
 * @returns wether or not a crc error occured
 * @author Owen DelBene
 * @date 12/20/2023
 */
bool lora_crc_error(lora_driver_obj_t* self)
{
	uint8_t reg_value; 
	
	reg_value = read_byte(self, _RH_RF95_REG_12_IRQ_FLAGS);
	return (reg_value & 0x20) >> 5;
}


/*
 * @brief fetch the rssi of the last transmission
 * @param self: lora driver struct
 * @param raw: if true return the value straight from the register
 * else return the value using the equation from the datasheet
 * @returns rssi value
 * @author Owen DelBene
 * @date 12/20/2023
 */
int16_t lora_rssi(lora_driver_obj_t* self, bool raw)
{
	
	uint8_t reg_value ; 
	reg_value = read_byte(self, _RH_RF95_REG_1A_PKT_RSSI_VALUE); 
	if (raw) return reg_value;
	return reg_value - 137;
}

/*
 * @brief check if the radio is done transmitting
 * @param self: lora driver struct
 * @returns if the transmission is done
 * @author Owen DelBene
 * @date 12/20/2023
 */
uint8_t lora_tx_done(lora_driver_obj_t* self)
{
	uint8_t reg_value ; 
	reg_value = read_byte(self, _RH_RF95_REG_12_IRQ_FLAGS);
	return (reg_value & 0x8) >> 3;
}
/*
 * @brief check if the radio is done receiving
 * @param self: lora driver struct
 * @returns if the transmission is done
 * @author Owen DelBene
 * @date 12/20/2023
 */

uint8_t lora_rx_done(lora_driver_obj_t* self)
{
	//if (self->dio0) return self->dio;
	uint8_t reg_value ; 
	//common_hal_busio_spi_read(self->spi, &reg_value, 1, _RH_RF95_REG_12_IRQ_FLAGS);
	reg_value = read_byte(self, _RH_RF95_REG_12_IRQ_FLAGS);
	return (reg_value & 0x40) >> 6;
	
}

/*
 * @brief set the radio to listen mode, mimics self.listen()
 * @param self: lora driver struct
 * @author Owen DelBene
 * @date 12/20/2023
 */
void lora_listen(lora_driver_obj_t* self)
{
	set_register(self, &self->operation_mode,  RX_MODE);
	set_register(self, &self->dio0_mapping,  0);
}

/*
 * @brief set the radio to transmit mode, mimics self.transmit()
 * @param self: lora driver struct
 * @author Owen DelBene
 * @date 12/20/2023
 */
void lora_transmit(lora_driver_obj_t* self)
{
	set_register(self, &self->operation_mode,  TX_MODE);
	set_register(self, &self->dio0_mapping,  1);
}

/*
 * @brief set the radio's transmit power, mimics self.tx_power.__set__(val)
 * @param self: lora driver struct
 * @param val: value to set the transmitter power to
 * @author Owen DelBene
 * @date 12/20/2023
 */
void set_tx_power(lora_driver_obj_t* self, uint8_t val)
{
	if (self->max_output) {
	write_byte(self, _RH_RF95_REG_0B_OCP, 0x3F);
	set_register(self, &self->pa_dac,  _RH_RF95_PA_DAC_ENABLE);	
	set_register(self, &self->pa_select,  1);
	set_register(self, &self->max_power, 7);
	set_register(self, &self->output_power,  0x0F);
	}
	else if (self->high_power) {
		if (val < 5 || val > 23) mp_raise_ValueError(MP_ERROR_TEXT("tx power must be between 5 and 23"));
		if (val > 20) {
			set_register(self, &self->pa_dac,  _RH_RF95_PA_DAC_ENABLE);	
			val-=3;
		}
		else {
			set_register(self, &self->pa_dac,  _RH_RF95_PA_DAC_DISABLE);	
		}

		set_register(self, &self->pa_select,  1);
		set_register(self, &self->max_power,  (val - 5) & 0x0F);
		
	 }

	else {
		set_register(self, &self->pa_select,  0);
		set_register(self, &self->max_power,  7);
		set_register(self, &self->output_power,  (val + 1) & 0x0F);
		
	}
}

/*
 * @brief reset the radio, mimics self.reset()
 * @param self: pointer to lora driver struct
 * @author Owen DelBene
 * @date 12/20/2023
 */
void lora_reset(lora_driver_obj_t* self)
{
	common_hal_digitalio_digitalinout_switch_to_output(&self->rst, false, PULL_UP);
	common_hal_mcu_delay_us(100);
	common_hal_digitalio_digitalinout_switch_to_input(&self->rst,PULL_UP );
	common_hal_mcu_delay_us(5000);
}

/*
 * @brief set the radio to sleep mode, mimics self.sleep()
 * @param self: pointer to lora driver struct
 * @author Owen DelBene
 * @date 12/20/2023
 */
void lora_sleep(lora_driver_obj_t* self)
{

	set_register(self, &self->operation_mode,  SLEEP_MODE); 
}

/*
 * @brief set the radio to idle mode, mimics self.idle()
 * @param self: pointer to lora driver struct
 * @author Owen DelBene
 * @date 12/20/2023
 */
void idle(lora_driver_obj_t* self)
{
	set_register(self, &self->operation_mode,  STANDBY_MODE); 
}


/*
 * @brief get the value from a register bit, mimics registerbits.__get__()
 * @param self: pointer to lora driver struct
 * @param rb: pointer to registerBits struct
 * @returns the value stored in the register with mask/offset specified by registerBits
 * @author Owen DelBene
 * @date 12/20/2023
 */
uint8_t get_register(lora_driver_obj_t* self, registerBits* rb )
{
	uint8_t reg_value =  read_byte(self, rb->address);
	return (reg_value & rb->mask) >> rb->offset;
}

/*
 * @brief set the value from a register bit, mimics registerbits.__set__()
 * @param self: pointer to lora driver struct
 * @param rb: pointer to registerBits struct
 * @param spi: spi bus
 * @param val: value to be stored in register
 * @author Owen DelBene
 * @date 12/20/2023
 */
void set_register(lora_driver_obj_t* self, registerBits* rb, uint8_t val)
{
	uint8_t reg_value = read_byte(self, rb->address);
	reg_value &= ~rb->mask;
	reg_value |= (val & 0xFF) << rb->offset;
	write_byte(self,rb->address , reg_value );
}

/*
 * @brief This is the constructor that is visible to the runtime enviroment
 * ex) l = lora_driver(param1, param2, ...)
 * @param spi: busio spi object
 * @param cs: chip select mcu pin object 
 * @param rst: reset mcu pin object
 * @param frequency: frequency in MHz
 * @param preamble_length
 * @param coding_rate
 * @param high_power
 * @param baudrate
 * @param max_output
 * @author Owen DelBene
 * @date 12/20/2023
 */
STATIC mp_obj_t lora_driver_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    //mp_arg_check_num(n_args, n_kw, 9, 9, true);
    lora_driver_obj_t *self = m_new_obj(lora_driver_obj_t);
    self->base.type = &lora_driver_type;


	enum {ARG_spi, ARG_cs, ARG_rst, ARG_freq, ARG_code_rate, ARG_baudrate, NUM_ARGS};
	static const mp_arg_t allowed_args[] = {
		{MP_QSTR_spi, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_cs, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_rst, MP_ARG_OBJ, {.u_obj = mp_const_none}},
		{MP_QSTR_freq, MP_ARG_INT, {.u_int = 0}},
		{MP_QSTR_code_rate, MP_ARG_INT, {.u_int = 5}},
		{MP_QSTR_baudrate, MP_ARG_INT, {.u_int = 5000000}},
	};
	
	mp_arg_val_t arg_vals[MP_ARRAY_SIZE(allowed_args)];
	mp_arg_parse_all_kw_array(n_args, n_kw, args, MP_ARRAY_SIZE(allowed_args), allowed_args, arg_vals);

	
	self->spi = validate_obj_is_spi_bus(arg_vals[ARG_spi].u_obj, MP_QSTR_spi);

	
	const mcu_pin_obj_t* cs = validate_obj_is_free_pin(arg_vals[ARG_cs].u_obj, MP_QSTR_cs);
	const mcu_pin_obj_t* rst = validate_obj_is_free_pin(arg_vals[ARG_cs].u_obj, MP_QSTR_rst);

	digitalinout_result_t result = common_hal_digitalio_digitalinout_construct(&self->cs, cs);
    if (result != DIGITALINOUT_OK) {
        mp_raise_ValueError_varg(MP_ERROR_TEXT("%q init failed"), MP_QSTR_cs);
    }

	result = common_hal_digitalio_digitalinout_construct(&self->rst, rst);
    if (result != DIGITALINOUT_OK) {
        mp_raise_ValueError_varg(MP_ERROR_TEXT("%q init failed"), MP_QSTR_cs);
    }

	//set member variables with defaults not used in pycubed.py	
	self->frequency_mhz = arg_vals[ARG_freq].u_int;
	self->preamble_length = 8;
	self->coding_rate = arg_vals[ARG_code_rate].u_int; 
	self->high_power = true;
	self->baudrate = arg_vals[ARG_code_rate].u_int;
	self->max_output = false;

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
	common_hal_digitalio_digitalinout_switch_to_input(&self->rst,PULL_UP );
	
	lora_reset(self);
	
	
	uint8_t version;
	version = read_byte(self, _RH_RF95_REG_42_VERSION);
	if (version != 18) {
		mp_raise_ValueError(MP_ERROR_TEXT("failed to find rfm9x with expected version. Check wiring"));
	}


	//
	
	
	idle(self);
	common_hal_mcu_delay_us(1000);
	//sleep(.01); //time.sleep(.01)
	set_register(self, &self->osc_calibration,  1);
	
	common_hal_mcu_delay_us(1000);//sleep(1);

	//self.sleep()
	lora_sleep(self);
	common_hal_mcu_delay_us(1000);// sleep(.01); //time.sleep(.01)
	set_register(self, &self->long_range_mode,  1);
	if (get_register(self, &self->operation_mode ) != SLEEP_MODE || !(get_register(self, &self->long_range_mode )))
		mp_raise_ValueError(MP_ERROR_TEXT("failed to configure rfm9x for lora mode. Check wiring"));
	if (get_register(self, &self->operation_mode ) != SLEEP_MODE)
		mp_raise_ValueError(MP_ERROR_TEXT("failed to configure rfm9x for lora mode"));
	
	if (self->frequency_mhz > 525) set_register(self, &self->low_frequency_mode, 0);

	write_byte(self, _RH_RF95_REG_0E_FIFO_TX_BASE_ADDR, 0);
	write_byte(self, _RH_RF95_REG_0F_FIFO_RX_BASE_ADDR, 0);
	write_byte(self, _RH_RF95_REG_24_HOP_PERIOD, 0);
	idle(self);


	self->signal_bandwidth = 125000;
	self->spreading_factor = 7;
	self->enable_crc = false;

	write_byte(self, _RH_RF95_REG_26_MODEM_CONFIG3, 0);
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

	set_register(self, &self->auto_agc, 1);
	set_register(self, &self->pa_ramp,  0);
	set_register(self, &self->lna_boost,1);

	return MP_OBJ_FROM_PTR(self);
}


/*
 * @brief rech if the transmission is done, this is visible to the runtime enviroment 
 * @param self_in: pointer to lora driver struct
 * @returns python object wether or not the transmission is complete
 * @author Owen DelBene
 * @date 12/20/2023
 */
STATIC mp_obj_t lora_driver_tx_done(mp_obj_t self_in)
{
	//begin spi transaction
	lora_driver_obj_t* self = MP_OBJ_TO_PTR(self_in);
	bool done =  lora_tx_done(self);
	return mp_obj_new_int(done);
	

}
  
/*
 * @brief send data over lora, this is visible to the runtime enviroment 
 * @param self_in: pointer to lora driver struct
 * @param data: bytearray object to send over lora
 * @param keep_listening: bool
 * @param destination
 * @param node
 * @param identifier
 * @param flags
 * @returns python object wether or not the transmission timed out
 * false means no timeout
 * @author Owen DelBene
 * @date 12/20/2023
 */
STATIC mp_obj_t lora_driver_send(size_t n_args, const mp_obj_t *args) {

	//parameters:
	//self, data, keep_listening, destination,node,identifier,flags

	lora_driver_obj_t* self =  MP_OBJ_TO_PTR(args[0]);

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




	write_byte(self, _RH_RF95_REG_0D_FIFO_ADDR_PTR, 0);
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
	write_bytes(self, (_RH_RF95_REG_00_FIFO | 0x80) & 0xFF, self->payload, len);

	write_byte(self, _RH_RF95_REG_22_PAYLOAD_LENGTH, len);

	lora_transmit(self);



	//timeout stuff
	bool timed_out = false;
	uint64_t start = supervisor_ticks_ms64();//common_hal_time_monotonic_ms();// mp_hal_time_ns(); //time.monotinic
	while (!timed_out && !lora_tx_done(self)) {
		if (supervisor_ticks_ms64() - start >= self->xmit_timeout) timed_out = true;
	}
	
	if (keep_listening) lora_listen(self);
	else idle(self);


	write_byte(self, _RH_RF95_REG_12_IRQ_FLAGS, 0xFF);
    return mp_obj_new_int(timed_out);
}



/*
 * @brief receive data over lora, this is visible to the runtime enviroment 
 * @param self_in: pointer to lora driver struct
 * @param keep_listening: bool
 * @param with_header: bool
 * @param with_ack: bool
 * @param timeout
 * @returns python bytearray with the data received
 * false means no timeout
 * @author Owen DelBene
 * @date 12/20/2023
 */
STATIC mp_obj_t lora_driver_receive(size_t n_args, const mp_obj_t *args) {

	lora_driver_obj_t* self = MP_OBJ_FROM_PTR(args[0]);
	
	bool keep_listening = mp_obj_get_int(args[1]);
	bool with_header = mp_obj_get_int(args[2]);
	bool with_ack = mp_obj_get_int(args[3]);
	uint64_t timeout = mp_obj_get_int(args[4]);

	bool timed_out = false;
	
	lora_listen(self);
	uint64_t start = supervisor_ticks_ms64();
	while (!timed_out && !lora_rx_done(self)) {
		if (supervisor_ticks_ms64() - start >=timeout) timed_out = true;
	}

	self->last_rssi = lora_rssi(self, false);
	idle(self);
	
	uint8_t offset;
	uint8_t fifo_length;
	if (!timed_out) {
		if (self->enable_crc && lora_crc_error(self)) self->crc_error_count +=1;
		
		else {
			fifo_length = read_byte(self, _RH_RF95_REG_13_RX_NB_BYTES);
			if (fifo_length >0) {
				uint8_t current_addr;
				current_addr = read_byte(self, _RH_RF95_REG_10_FIFO_RX_CURRENT_ADDR);
				write_byte(self, _RH_RF95_REG_0D_FIFO_ADDR_PTR, current_addr);
				
				read_bytes(self, _RH_RF95_REG_00_FIFO, self->packet, fifo_length);
			}
			
			if (fifo_length < 5) memset(self->packet, 0, 256); //idunno

			else {
				if (self->node != _RH_BROADCAST_ADDRESS 
					&& self->packet[0] != _RH_BROADCAST_ADDRESS 
					&& self->packet[0] != self->node )
					memset(self->packet, 0, 256);
				else if (with_ack 
					  && ((self->packet[3] & _RH_FLAGS_ACK) == 0)
					  && (self->packet[0] != _RH_BROADCAST_ADDRESS)) {
						
						if (self->ack_delay) common_hal_mcu_delay_us(self->ack_delay); //CORRECT UNITS?
						lora_send_ack(self,self->destination, self->node, self->identifier, self->flags, keep_listening);
						self->seen_ids[self->packet[1]] = self->packet[2];
					  }
				if (!with_header && (fifo_length !=0) )
					offset = 4;
			}

		}
	}

	if (keep_listening) lora_listen(self);
	else idle(self);
	
	write_byte(self, _RH_RF95_REG_12_IRQ_FLAGS, 0xFF);

	mp_obj_array_t* bytearray = array_new(BYTEARRAY_TYPECODE, fifo_length - offset);
	memcpy(bytearray->items, self->packet + offset, fifo_length - offset);
	return bytearray;	


}

/*
 * @brief fetch the last rssi, this is visible to the runtime enviroment 
 * @param self_in: pointer to lora driver struct
 * @returns python object containing the last rssi (not raw)
 * @author Owen DelBene
 * @date 12/20/2023
 */
STATIC mp_obj_t lora_driver_last_rssi(mp_obj_t self_in)
{
	lora_driver_obj_t* self = MP_OBJ_TO_PTR(self_in);
	int16_t rssi = lora_rssi(self, false);
	return mp_obj_new_int(rssi);
}


//create objects for all functions visible to runtime enviroment
//one parameter
MP_DEFINE_CONST_FUN_OBJ_1(lora_driver_tx_done_obj, lora_driver_tx_done);
MP_DEFINE_CONST_FUN_OBJ_1(lora_driver_last_rssi_obj, lora_driver_last_rssi);
//multible parameters
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(lora_driver_send_obj,2, 7, lora_driver_send);
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(lora_driver_receive_obj,0, 6, lora_driver_receive);


//create QSTR so the function objects can be accessed by typing send, receive, tx_done, and last_rssi
STATIC const mp_rom_map_elem_t lora_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_send), MP_ROM_PTR(&lora_driver_send_obj) },
    { MP_ROM_QSTR(MP_QSTR_receive), MP_ROM_PTR(&lora_driver_receive_obj) },
    { MP_ROM_QSTR(MP_QSTR_tx_done), MP_ROM_PTR(&lora_driver_tx_done_obj) },
    { MP_ROM_QSTR(MP_QSTR_last_rssi), MP_ROM_PTR(&lora_driver_last_rssi_obj) },
};

//set the above table as the locals for the lora module
//this allows dir(lora) to print the QSTR listed in the table above
STATIC MP_DEFINE_CONST_DICT(lora_locals_dict, lora_locals_dict_table);


//define how the class will be printed
STATIC void lora_driver_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    (void)kind;
    mp_print_str(print, "lora driver, why are you printing this?");
	
}



//register python object visible to interpreter
const mp_obj_type_t lora_driver_type = {
    { &mp_type_type },
    .name = MP_QSTR_lora_driver,
    .make_new = lora_driver_make_new,
    .locals_dict = (mp_obj_dict_t*)&lora_locals_dict,
	.print = lora_driver_print
};












//register globals constants and dicts 
//lora driver will be available in the module as lora_driver
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
//module will be available as import lora
MP_REGISTER_MODULE(MP_QSTR_lora, lora_user_cmodule, 1/*MODULE_PROPERTYCLASS_ENABLED*/);









