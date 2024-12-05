#ifndef __SPI_H_
#define __SPI_H_

#include <avr/io.h>
#include "ecode.h"

/*
SPI module
*/

#define CS_PIN     PIN7_bm
#define CLK_PIN    PIN6_bm
#define MOSI_PIN   PIN4_bm
#define MISO_PIN   PIN5_bm

ECODE spi_init();
void spi_enable();
void spi_disable();
ECODE spi_tx(uint8_t input);
ECODE spi_rx(uint8_t *output);
ECODE spi_txrx(uint8_t input, uint8_t *output);

#endif
