/*
radio datasheets:
https://cdn-learn.adafruit.com/downloads/pdf/adafruit-rfm69hcw-and-rfm96-rfm95-rfm98-lora-packet-padio-breakouts.pdf
https://cdn-shop.adafruit.com/product-files/3076/sx1231.pdf

avr-ble:
https://ww1.microchip.com/downloads/aemDocuments/documents/MCU08/ProductDocuments/UserGuides/AVR-BLE-Hardware-User-Guide-DS50002956B.pdf
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

/* ARM_BUTTON_PIN - PC1 */
#define ARM_BUTTON_PIN      PIN1_bm
/* IGNITE_BUTTON_PIN - PD6 */
#define IGNITE_BUTTON_PIN   PIN6_bm
/* DC_BUZZER_PIN - PD1 */
#define DC_BUZZER_PIN       PIN1_bm

/* LORA_LED_PIN - PC0 */
#define LORA_LED_PIN        PIN0_bm
/* continuity LED green channel pin - PD5 */
#define GREEN_LED_PIN       PIN5_bm
/* continuity LED red channel pin - PD7 */
#define RED_LED_PIN         PIN7_bm

uint8_t ledToggle = 0;
uint32_t millis = 0;
uint32_t ledMillis = 0;
uint8_t receivedGood = 0;
uint8_t hasConnection = 0;
uint8_t mustRelease = 0;

void parse_lora(uint8_t * buf, uint8_t len, uint8_t status);
void sendIgnite(); // send ignite key to receiver

int main() {
    /* pin init */
    PORTC.DIR |= LORA_LED_PIN;
    PORTD.DIR |= GREEN_LED_PIN;
    PORTD.DIR |= RED_LED_PIN;
    PORTD.DIR |= DC_BUZZER_PIN;
    PORTC.DIR &= ~ARM_BUTTON_PIN;
    
    PORTD.OUT |= GREEN_LED_PIN; // start yellow
    PORTD.OUT |= RED_LED_PIN;
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
        if (millis - ledMillis > 500 && receivedGood) {
            PORTC.OUT |= LORA_LED_PIN;
        }
        if (PORTC.OUT & ARM_BUTTON_PIN) {
            if (hasConnection) {
                PORTD.OUT |= DC_BUZZER_PIN;
                if (PORTD.OUT & IGNITE_BUTTON_PIN && mustRelease == 0) {
                    sendIgnite();
                    mustRelease = 1;
                }
            }
        } else {
            mustRelease = 0;
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
	uart_tx("Received: \"");
    uart_tx((const char *) buf);
    uart_tx("\"\r\n");
    if (strcmp((const char *) buf, "stop") == 0) {
        /* received continuity OK */
        PORTD.OUT |= GREEN_LED_PIN;
        PORTD.OUT &= ~RED_LED_PIN;
        receivedGood = 1;
        hasConnection = 1;
    }
    if (strcmp((const char *) buf, "stal") == 0) {
        /* received continuity ERROR (no continuity) */
        PORTD.OUT &= ~GREEN_LED_PIN;
        PORTD.OUT |= RED_LED_PIN;
        receivedGood = 1;
        hasConnection = 1;
    }
}

void sendIgnite() {
    
}

/* TCA ISR - every second */
ISR(TCA0_OVF_vect) {
    uint8_t message[] = {0xFF, 0xFF, 0xFF, 0xFF, 'c', 'o', 'r', 'k', '\0'};
    lora_send(message, sizeof(message));
    uart_tx("sent \"cork\"\r\n");
    if (receivedGood == 0) {
        /* didn't receive it last time - toggle LED yellow */
        PORTD.OUT |= GREEN_LED_PIN;
        PORTD.OUT |= RED_LED_PIN;
        hasConnection = 0;
    }
    receivedGood = 0;
    PORTC.OUT &= ~LORA_LED_PIN;
    ledMillis = millis;
    /* The interrupt flag has to be cleared manually */
    TCA0.SINGLE.INTFLAGS &= TCA_SINGLE_OVF_bm;
}