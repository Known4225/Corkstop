#ifndef __LORA_H_
#define __LORA_H_

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "ecode.h"

//Registers
#define REG_FIFO					0x00
#define REG_OP_MODE					0x01
#define REG_FRF_MSB					0x06
#define REG_FRF_MID					0x07
#define REG_FRF_LSB					0x08
#define REG_PA_CONFIG				0x09
#define REG_OCP						0x0b
#define REG_LNA						0x0c
#define REG_FIFO_ADDR_PTR			0x0d
#define REG_FIFO_TX_BASE_ADDR		0x0e
#define REG_FIFO_RX_BASE_ADDR		0x0f
#define REG_FIFO_RX_CURRENT_ADDR	0x10
#define REG_IRQ_FLAGS				0x12
#define REG_RX_NB_BYTES				0x13
#define REG_PKT_SNR_VALUE			0x19
#define REG_PKT_RSSI_VALUE			0x1a
#define REG_RSSI_VALUE				0x1b
#define REG_MODEM_CONFIG_1			0x1d
#define REG_MODEM_CONFIG_2			0x1e
#define REG_PREAMBLE_MSB			0x20
#define REG_PREAMBLE_LSB			0x21
#define REG_PAYLOAD_LENGTH			0x22
#define REG_MODEM_CONFIG_3			0x26
#define REG_FREQ_ERROR_MSB			0x28
#define REG_FREQ_ERROR_MID			0x29
#define REG_FREQ_ERROR_LSB			0x2a
#define REG_RSSI_WIDEBAND			0x2c
#define REG_DETECTION_OPTIMIZE		0x31
#define REG_INVERTIQ				0x33
#define REG_DETECTION_THRESHOLD		0x37
#define REG_SYNC_WORD				0x39
#define REG_INVERTIQ2				0x3b
#define REG_DIO_MAPPING_1			0x40
#define REG_VERSION					0x42
#define REG_PA_DAC					0x4d

//Modes
#define MODE_LONG_RANGE_MODE		0x80
#define MODE_SLEEP					0x00
#define MODE_STDBY					0x01
#define MODE_TX						0x03
#define MODE_RX_CONTINUOUS			0x05
#define MODE_RX_SINGLE				0x06

//PA config
#define PA_BOOST					0x80

//IRQ masks
#define IRQ_TX_DONE_MASK			0x08
#define IRQ_PAYLOAD_CRC_ERROR_MASK	0x20
#define IRQ_RX_DONE_MASK			0x40

//RSSI constants
#define RF_MID_BAND_THRESHOLD	525E6
#define RSSI_OFFSET_HF_PORT		157
#define RSSI_OFFSET_LF_PORT		164

//Coding rate
#define CODING_RATE_4_5			0b001
#define CODING_RATE_4_6			0b010
#define CODING_RATE_4_7			0b011
#define CODING_RATE_4_8			0b100

//Bandwidth
#define BANDWIDTH_7_8_KHZ		0b0000
#define BANDWIDTH_10_4_KHZ		0b0001
#define BANDWIDTH_15_6_KHZ		0b0010
#define BANDWIDTH_20_8_KHZ		0b0011
#define BANDWIDTH_31_25_KHZ		0b0100
#define BANDWIDTH_41_7_KHZ		0b0101
#define BANDWIDTH_62_5_KHZ		0b0110
#define BANDWIDTH_125_KHZ		0b0111
#define BANDWIDTH_250_KHZ		0b1000
#define BANDWIDTH_500_KHZ		0b1001

//Spreading factor
#define SF6		6
#define SF7		7
#define SF8		8
#define SF9		9
#define SF10	10
#define SF11	11
#define SF12	12

#define MAX_PKT_LENGTH			255

//==============================================
//=================== CONFIG ===================
#define SPREADING_FACTOR		SF7
#define CODING_RATE				CODING_RATE_4_5
#define BANDWIDTH				BANDWIDTH_125_KHZ
#define FREQUENCY				433E6

#define RST_PIN     PIN2_bm

#define SS		(1<<PB2)
#define DDR_SS		DDRB
#define PORT_SS		PORTB

#define RST		(1<<PB1)
#define DDR_RST		DDRB
#define PORT_RST	PORTB

#define SCK		(1<<PB5)
#define DDR_SCK		DDRB
#define PORT_SCK	PORTB

#define MOSI		(1<<PB3)
#define DDR_MOSI	DDRB
#define PORT_MOSI	PORTB

#define MISO		(1<<PB4)
#define DDR_MISO	DDRB
#define PORT_MISO	PORTB
//==============================================
//==============================================

// Init SX1278 module
ECODE lora_init();

// Read register on 'reg' address
ECODE lora_read_register(uint8_t reg, uint8_t *output);

// Write register on address 'reg' with 'value' value
ECODE lora_write_register(uint8_t reg, uint8_t value);

// Put module into sleep mode with LoRa
void lora_sleep();
// Put module into standby mode with LoRa
void lora_standby();
// Put module into receive continuous mode
void lora_rx_continuous();

// Main library event function. This should run in non-blocked main loop
void lora_event();

//Register callback function for receiving data
void register_lora_rx_event_callback(void (*callback)(uint8_t * buf, uint8_t len, uint8_t status));

//Set over current protection on module
uint8_t lora_set_ocp(uint8_t max_current);

//Change bandwidth
//Use provided definitions from lora_mem.h
void lora_set_bandwidth(uint8_t mode);

//Change spreading factor
//Use provided definitions from lora_mem.h
void lora_set_spreading_factor(uint8_t sf);

//Set coding rate
//Use provided definitions from lora_mem.h
void lora_set_coding_rate(uint8_t rate);

//Read Received Signal Strength Indicator (RSSI) from last received packet
int16_t lora_last_packet_rssi(uint32_t freq);

//Use explicit header mode. Module send: Preamble + Header + CRC + Payload + Payload CRC
void lora_explicit_header();

//Set transmitter power value can be set between 2 and 20.
void lora_tx_power(uint8_t db);
//Set working frequency. For SX1278 default value is 433 MHz
void lora_set_freq(uint32_t freq);

//Transmit data from buf
void lora_putd(uint8_t * buf, uint8_t len);

#endif
