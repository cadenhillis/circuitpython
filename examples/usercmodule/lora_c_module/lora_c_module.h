#include <stdio.h>
#include "py/obj.h"
#include "py/binary.h"
#include "py/runtime.h"
#include "shared-bindings/digitalio/DigitalInOut.h"
#include "shared-bindings/busio/SPI.h"
//#include "common-hal/busio/SPI.h"
#include "common-hal/digitalio/DigitalInOut.h"
#include "shared-bindings/microcontroller/Pin.h"
#include <stdint.h>
#include "shared-bindings/microcontroller/__init__.h"
#include "py/mphal.h"
#include "supervisor/shared/tick.h"
#include "py/objarray.h"

#define _RH_RF95_REG_00_FIFO  (0x00)
#define _RH_RF95_REG_01_OP_MODE  (0x01)
#define _RH_RF95_REG_06_FRF_MSB  (0x06)
#define _RH_RF95_REG_07_FRF_MID  (0x07)
#define _RH_RF95_REG_08_FRF_LSB  (0x08)
#define _RH_RF95_REG_09_PA_CONFIG  (0x09)
#define _RH_RF95_REG_0A_PA_RAMP  (0x0A)
#define _RH_RF95_REG_0B_OCP  (0x0B)
#define _RH_RF95_REG_0C_LNA  (0x0C)
#define _RH_RF95_REG_0D_FIFO_ADDR_PTR  (0x0D)
#define _RH_RF95_REG_0E_FIFO_TX_BASE_ADDR  (0x0E)
#define _RH_RF95_REG_0F_FIFO_RX_BASE_ADDR  (0x0F)
#define _RH_RF95_REG_10_FIFO_RX_CURRENT_ADDR  (0x10)
#define _RH_RF95_REG_11_IRQ_FLAGS_MASK  (0x11)
#define _RH_RF95_REG_12_IRQ_FLAGS  (0x12)
#define _RH_RF95_REG_13_RX_NB_BYTES  (0x13)
#define _RH_RF95_REG_14_RX_HEADER_CNT_VALUE_MSB  (0x14)
#define _RH_RF95_REG_15_RX_HEADER_CNT_VALUE_LSB  (0x15)
#define _RH_RF95_REG_16_RX_PACKET_CNT_VALUE_MSB  (0x16)
#define _RH_RF95_REG_17_RX_PACKET_CNT_VALUE_LSB  (0x17)
#define _RH_RF95_REG_18_MODEM_STAT  (0x18)
#define _RH_RF95_REG_19_PKT_SNR_VALUE  (0x19)
#define _RH_RF95_REG_1A_PKT_RSSI_VALUE  (0x1A)
#define _RH_RF95_REG_1B_RSSI_VALUE  (0x1B)
#define _RH_RF95_REG_1C_HOP_CHANNEL  (0x1C)
#define _RH_RF95_REG_1D_MODEM_CONFIG1  (0x1D)
#define _RH_RF95_REG_1E_MODEM_CONFIG2  (0x1E)
#define _RH_RF95_REG_1F_SYMB_TIMEOUT_LSB  (0x1F)
#define _RH_RF95_REG_20_PREAMBLE_MSB  (0x20)
#define _RH_RF95_REG_21_PREAMBLE_LSB  (0x21)
#define _RH_RF95_REG_22_PAYLOAD_LENGTH  (0x22)
#define _RH_RF95_REG_23_MAX_PAYLOAD_LENGTH  (0x23)
#define _RH_RF95_REG_24_HOP_PERIOD  (0x24)
#define _RH_RF95_REG_25_FIFO_RX_BYTE_ADDR  (0x25)
#define _RH_RF95_REG_26_MODEM_CONFIG3  (0x26)
#define _RH_RF95_REG_40_DIO_MAPPING1  (0x40)
#define _RH_RF95_REG_41_DIO_MAPPING2  (0x41)
#define _RH_RF95_REG_42_VERSION  (0x42)
#define _RH_RF95_REG_4B_TCXO  (0x4B)
#define _RH_RF95_REG_4D_PA_DAC  (0x4D)
#define _RH_RF95_REG_5B_FORMER_TEMP  (0x5B)
#define _RH_RF95_REG_61_AGC_REF  (0x61)
#define _RH_RF95_REG_62_AGC_THRESH1  (0x62)
#define _RH_RF95_REG_63_AGC_THRESH2  (0x63)
#define _RH_RF95_REG_64_AGC_THRESH3  (0x64)
#define _RH_RF95_DETECTION_OPTIMIZE  (0x31)
#define _RH_RF95_DETECTION_THRESHOLD  (0x37)
#define _RH_RF95_PA_DAC_DISABLE  (0x04)
#define _RH_RF95_PA_DAC_ENABLE  (0x07)
#define _RH_RF95_FSTEP  32000000 / 524288
#define _RH_BROADCAST_ADDRESS  (0xFF)
#define _RH_FLAGS_ACK  (0x80)
#define _RH_FLAGS_RETRY  (0x40)
#define SLEEP_MODE   (0)
#define STANDBY_MODE (1)
#define FS_TX_MODE   (2)
#define TX_MODE      (3)
#define FS_RX_MODE   (4)
#define RX_MODE      (5)



typedef struct {

	uint8_t offset; //default to zero
	uint8_t address;
	uint8_t mask;

} registerBits;


typedef struct {

		mp_obj_base_t base;


		busio_spi_obj_t *spi;
		digitalio_digitalinout_obj_t cs;
		digitalio_digitalinout_obj_t rst;
		uint32_t frequency_mhz;
		uint8_t polarity;
		uint8_t phase;

		uint8_t preamble_length;
		uint8_t coding_rate;
		bool high_power;
		uint32_t baudrate;
		bool max_output;
		bool dio0;


		uint32_t signal_bandwidth;
		uint16_t spreading_factor;
		bool enable_crc;

		uint32_t tx_power;

		int16_t last_rssi;
		float ack_wait;
		float receive_timeout;
		uint64_t xmit_timeout;
		uint8_t ack_retries;
		uint8_t ack_delay;

		uint32_t sequence_number;

		uint8_t seen_ids[256];

		uint8_t node;
		uint8_t destination;
		uint8_t identifier;
		uint8_t flags;

		uint32_t crc_error_count;


		registerBits operation_mode;
		registerBits low_frequency_mode;
		registerBits osc_calibration;
		registerBits modulation_type;
		registerBits long_range_mode;
		registerBits lna_boost;
		registerBits output_power;
		registerBits modulation_shaping;
		registerBits pa_ramp;
		registerBits max_power;
		registerBits pa_select;
		registerBits pa_dac;
		registerBits dio0_mapping;
		registerBits low_datarate_optimize;
		registerBits auto_agc;


		uint8_t payload[256];
		uint8_t packet[256];
		uint8_t buf[2];
} lora_driver_obj_t;





const  mp_obj_type_t lora_driver_type;
void lora_reset(lora_driver_obj_t* self);
void lora_sleep(lora_driver_obj_t* self);
void idle(lora_driver_obj_t* self);

uint8_t get_register(lora_driver_obj_t* self, registerBits* rb);
void set_register(lora_driver_obj_t* self, registerBits* rb,  uint8_t val);

void set_tx_power(lora_driver_obj_t* self, uint8_t val);
void lora_listen(lora_driver_obj_t* self);
void lora_transmit(lora_driver_obj_t* self);
uint8_t lora_tx_done(lora_driver_obj_t* self);
uint8_t lora_rx_done(lora_driver_obj_t* self);
bool lora_crc_error(lora_driver_obj_t* self);
int16_t lora_rssi(lora_driver_obj_t* self, bool raw);
STATIC mp_obj_array_t *array_new(char typecode, size_t n);
bool lora_send_ack(lora_driver_obj_t* self,uint8_t destination, uint8_t node, uint8_t identifier, uint8_t flags, bool keep_listening);
void write_byte(lora_driver_obj_t* self, uint8_t reg, uint8_t val);
uint8_t read_byte(lora_driver_obj_t* self, uint8_t reg);
void write_bytes(lora_driver_obj_t* self, uint8_t reg, uint8_t* val, size_t len);
void read_bytes(lora_driver_obj_t* self, uint8_t reg, uint8_t* buf, size_t len);
