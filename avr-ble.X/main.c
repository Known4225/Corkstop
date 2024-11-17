/*
https://cdn-learn.adafruit.com/downloads/pdf/adafruit-rfm69hcw-and-rfm96-rfm95-rfm98-lora-packet-padio-breakouts.pdf
https://cdn-shop.adafruit.com/product-files/3076/sx1231.pdf
*/

#define F_CPU 3333333

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#include <string.h>

#include "uart.h"
#include "tca.h"
#include "lora.h"

uint8_t ledToggle = 0;
uint32_t millis = 0;
uint32_t ledMillis = 0;

void parse_lora(uint8_t * buf, uint8_t len, uint8_t status);

int main() {
    /* LED init */
    PORTC.DIR |= PIN0_bm;
    uart_init(9600);
    if (tca_init()) {
        uart_tx("RTC could not initialise\r\n");
        while (1);
    }
    uart_tx("RTC successfully initialised\r\n");
    if(lora_init()) {
        uart_tx("lora could not initialise\r\n");
		while(1); // If init returns 0, error occur. Check connections and try again.
	}
    uart_tx("lora successfully initialised\r\n");
    register_lora_rx_event_callback(parse_lora);
    sei();
	while(1) {
		lora_receive();
        if (millis - ledMillis > 500) {
            PORTC.OUT &= ~PIN0_bm;
        }
        _delay_ms(1);
        millis++;
	}
}

void parse_lora(uint8_t *buf, uint8_t len, uint8_t status) {
	//If error, return.
	if( status == IRQ_PAYLOAD_CRC_ERROR_MASK ) {
		// ...process error
		return;
	}
	uart_tx("buf: ");
    uart_tx((const char *) buf);
    uart_tx("\r\n");
}

/* TCA ISR - every second */
ISR(TCA0_OVF_vect) {
    uint8_t message[] = {0xFF, 0xFF, 0xFF, 0xFF, 'c', 'o', 'r', 'k', '\0'};
    lora_send(message, sizeof(message));
    uart_tx("rtc tick\r\n");
    PORTC.OUT |= PIN0_bm;
    ledMillis = millis;
    /* The interrupt flag has to be cleared manually */
    TCA0.SINGLE.INTFLAGS &= TCA_SINGLE_OVF_bm;
}