#define F_CPU 3333333
// Code in the getting started guide appears to do same thing, but they do not
// know how to code cleanly.
// This is all based on formula in Table 23-1 of the ATmega 3208 data sheet
// Need to shift value left 6 bits to follow format specified in data sheet:
// 16-bit number, 10 bits are whole number part, bottom 6 are fractional part
// The +0.5 is to force the result to be rounded *up* rather than down.
// SAMPLES_PER_BIT: 16, for normal asynchronous mode. Given in data sheet.
#define SAMPLES_PER_BIT 16
#define USART_BAUD_VALUE(BAUD_RATE) (uint16_t) ((F_CPU << 6) / (((float) SAMPLES_PER_BIT) * (BAUD_RATE)) + 0.5)

#include "uart.h"

ECODE uart_init(uint32_t baud_rate) {
    PORTF.DIR |= TX_PIN;
    PORTF.DIR &= ~RX_PIN;
    // PORTMUX.USARTROUTEA |= PORTMUX_USART2_0_bm;
    // PORTF.PIN0CTRL |= PORT_PULLUPEN_bm;

    // USART2.DBGCTRL = USART_DBGRUN_bm;
    USART2.BAUD = USART_BAUD_VALUE(baud_rate);
    USART2.CTRLB |= USART_TXEN_bm | USART_RXEN_bm;
    USART2.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_CHSIZE_8BIT_gc | USART_RXMODE_NORMAL_gc;
    uart_tx("uart initialised\r\n");
    return ECODE_OK;
}

void uart_putc(char c) {
    while (!(USART2.STATUS & USART_DREIF_bm)) {}
    USART2.TXDATAL = c;
}

ECODE uart_tx(const char *send) {
    for (uint8_t i = 0; send[i] != '\0'; i++) {
        uart_putc(send[i]);
    }
    return ECODE_OK;
}

