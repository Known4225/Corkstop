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
#include <stdio.h>

#include "uart.h"
#include "tca.h"
#include "lora.h"

/* ARM_BUTTON_PIN - PC1 */
#define ARM_BUTTON_PIN        PIN1_bm
/* IGNITE_BUTTON_PIN - PD6 */
#define IGNITE_BUTTON_PIN     PIN6_bm
/* DC_BUZZER_PIN - PD1 */
#define DC_BUZZER_PIN         PIN1_bm

/* LORA_LED_PIN - PC0 */
#define LORA_LED_PIN          PIN0_bm
/* continuity LED green channel pin - PD5 */
#define GREEN_CONT_LED_PIN    PIN5_bm
/* continuity LED red channel pin - PD7 */
#define RED_CONT_LED_PIN      PIN7_bm
/* ignite status LED green channel pin - PA0 */
#define GREEN_IGN_LED_PIN     PIN0_bm
/* ignite status LED red channel pin - PF3 */
#define RED_IGN_LED_PIN       PIN3_bm

uint8_t ledToggle = 0;
uint32_t millis = 0;
uint32_t ledMillis = 0;
uint8_t receivedGood = 0;
uint8_t hasConnection = 0;
uint8_t mustRelease = 0;

void parse_lora(uint8_t * buf, uint8_t len, uint8_t status);
void sendIgnite(); // send ignite key to receiver
uint32_t armButtonDown; // how many milliseconds the arm button has been down for
uint8_t armButtonBuffer = 0;
uint8_t igniteButtonBuffer = 0;
uint8_t firstTick = 0;

int main() {
    /* pin init */
    PORTC.DIR |= LORA_LED_PIN;
    PORTD.DIR |= GREEN_CONT_LED_PIN;
    PORTD.DIR |= RED_CONT_LED_PIN;
    PORTA.DIR |= GREEN_IGN_LED_PIN;
    PORTF.DIR |= RED_IGN_LED_PIN;
    PORTD.DIR |= DC_BUZZER_PIN;
    PORTC.DIR &= ~ARM_BUTTON_PIN;
    PORTC.PIN1CTRL |= PORT_PULLUPEN_bm;
    PORTD.DIR &= ~IGNITE_BUTTON_PIN;
    PORTD.PIN6CTRL |= PORT_PULLUPEN_bm;
    
    PORTD.OUT |= GREEN_CONT_LED_PIN; // start yellow
    PORTD.OUT |= RED_CONT_LED_PIN;
    PORTA.OUT &= ~GREEN_IGN_LED_PIN; // start off
    PORTF.OUT &= ~RED_IGN_LED_PIN;
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
    uart_tx("lora successfully initialised\r\n\r\n");
    register_lora_rx_event_callback(parse_lora);
    sei();
	while(1) {
		lora_receive();
        if ((PORTC.IN & ARM_BUTTON_PIN) == 0) {
            armButtonBuffer = 10;
        }
        if (armButtonBuffer) {
            armButtonDown++;
        }
        if (armButtonBuffer > 0) {
            armButtonBuffer--;
        } else {
            armButtonDown = 0;
        }
        if ((PORTD.IN & IGNITE_BUTTON_PIN) == 0) {
            igniteButtonBuffer = 10;
        }
        if (igniteButtonBuffer > 0) {
            igniteButtonBuffer--;
        }
        if (millis - ledMillis > 500 && receivedGood) {
            PORTC.OUT |= LORA_LED_PIN;
        }
        if (armButtonBuffer) {
            firstTick = 1;
            if (mustRelease == 0 && armButtonDown == 1) {
                /* turn off IGNITE LED */
                PORTA.OUT &= ~GREEN_IGN_LED_PIN;
                PORTF.OUT &= ~RED_IGN_LED_PIN;
            }
            if (hasConnection) {
                if (mustRelease == 0) {
                    PORTD.OUT |= DC_BUZZER_PIN;
                }
                if (armButtonDown == 1000) {
                    /* turn IGNITE LED to YELLOW */
                    PORTA.OUT |= GREEN_IGN_LED_PIN;
                    PORTF.OUT |= RED_IGN_LED_PIN;
                }
                if (igniteButtonBuffer && mustRelease == 0 && armButtonDown > 1000) {
                    /* turn IGNITE LED to YELLOW */
                    PORTA.OUT |= GREEN_IGN_LED_PIN;
                    PORTF.OUT |= RED_IGN_LED_PIN;
                    sendIgnite();
                    mustRelease = 1;
                }
            }
        } else {
            if (firstTick) {
                if (mustRelease == 0) {
                    /* turn off IGNITE LED */
                    PORTA.OUT &= ~GREEN_IGN_LED_PIN;
                    PORTF.OUT &= ~RED_IGN_LED_PIN;
                }
                if (igniteButtonBuffer == 0) {
                    mustRelease = 0;
                }
                firstTick = 0;
            }
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
    uart_tx("RSSI: ");
    uint16_t rssi = lora_last_packet_rssi(433);
    char rssiStr[10];
    sprintf(rssiStr, "%d", rssi);
    uart_tx(rssiStr);
    uart_tx("\r\n\r\n");
    if (strcmp((const char *) buf, "stop") == 0) {
        /* received continuity OK */
        PORTD.OUT |= GREEN_CONT_LED_PIN;
        PORTD.OUT &= ~RED_CONT_LED_PIN;
        receivedGood = 1;
        hasConnection = 1;
    }
    if (strcmp((const char *) buf, "stal") == 0) {
        /* received continuity ERROR (no continuity) */
        PORTD.OUT &= ~GREEN_CONT_LED_PIN;
        PORTD.OUT |= RED_CONT_LED_PIN;
        receivedGood = 1;
        hasConnection = 1;
    }
    if (strcmp((const char *) buf, "done") == 0) {
        /* received ignite OK */
        PORTA.OUT |= GREEN_IGN_LED_PIN;
        PORTF.OUT &= ~RED_IGN_LED_PIN;
    }
    if (strcmp((const char *) buf, "cant") == 0) {
        /* received ignite ERROR */
        PORTA.OUT &= ~GREEN_IGN_LED_PIN;
        PORTF.OUT |= RED_IGN_LED_PIN;
    }
}

void sendIgnite() {
    uint8_t message[] = {0xFF, 0xFF, 0xFF, 0xFF, 'I', 'G', 'N', 'I', 'T', 'E', '\0'};
    lora_send(message, sizeof(message));
    uart_tx("Sent \"IGNITE\"\r\n");
}

/* TCA ISR - every second */
ISR(TCA0_OVF_vect) {
    uint8_t message[] = {0xFF, 0xFF, 0xFF, 0xFF, 'c', 'o', 'r', 'k', '\0'};
    lora_send(message, sizeof(message));
    uart_tx("Sent \"cork\"\r\n");
    if (receivedGood == 0) {
        /* didn't receive it last time - toggle continuity LED yellow */
        PORTD.OUT |= GREEN_CONT_LED_PIN;
        PORTD.OUT |= RED_CONT_LED_PIN;
        hasConnection = 0;
    }
    receivedGood = 0;
    PORTC.OUT &= ~LORA_LED_PIN;
    ledMillis = millis;
    /* The interrupt flag has to be cleared manually */
    TCA0.SINGLE.INTFLAGS &= TCA_SINGLE_OVF_bm;
}