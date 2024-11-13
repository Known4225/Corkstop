#define F_CPU 3333333

#include "spi.h"

ECODE spi_init() {
    PORTA.DIR |= MOSI_PIN; /* Set MOSI pin direction to output */
    PORTA.DIR &= ~MISO_PIN; /* Set MISO pin direction to input */
    PORTA.DIR |= CLK_PIN; /* Set SCK pin direction to output */
    PORTA.DIR |= CS_PIN; /* Set CS pin direction to output */
    SPI0.CTRLA = SPI_CLK2X_bm /* Enable double-speed */ /* MSB is transmitted first */
    | SPI_ENABLE_bm /* Enable module */
    | SPI_MASTER_bm /* SPI module in Host mode */
    | SPI_PRESC_DIV4_gc; /* System Clock divided by 4 (0.8333 MHz) */
    SPI0.CTRLB |= SPI_MODE_3_gc; /* Data Mode 3 */
    return ECODE_OK;
}

void spi_enable() {
    PORTA.OUT &= ~CS_PIN; // Set CS pin value to LOW
}

void spi_disable() {
    PORTA.OUT |= CS_PIN; // Set CS pin value to HIGH
}

ECODE spi_tx(uint8_t input) {
    uint8_t dummy;
    return spi_txrx(input, &dummy);
}

ECODE spi_rx(uint8_t *output) {
    uint8_t dummy = 0;
    return spi_txrx(dummy, output);
}

ECODE spi_txrx(uint8_t input, uint8_t *output) {
    SPI0.DATA = input;
    uint16_t attempts = 0;
    while (!(SPI0.INTFLAGS & SPI_IF_bm)) {
        if (attempts > 1000) {
            return ECODE_FAIL;
        }
        attempts++;
    }
    *output = SPI0.DATA;
    return ECODE_OK;
}