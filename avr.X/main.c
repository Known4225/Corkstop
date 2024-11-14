#define F_CPU 3333333

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#include <string.h>

#include "lora.h"
#include "uart.h"

void parse_lora(uint8_t * buf, uint8_t len, uint8_t status);

int main() {
    uart_init(9600);
    if(!lora_init()) {
        uart_tx("lora could not initialise\r\n");
		while(1); // If init returns 0, error occur. Check connections and try again.
	}
    uart_tx("lora successfully initialised\r\n");
    register_lora_rx_event_callback(parse_lora);
    sei();
	while(1) {
		lora_receive();
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
