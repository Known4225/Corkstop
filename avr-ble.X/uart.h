/* data:
https://ww1.microchip.com/downloads/en/AppNotes/TB3216-Getting-Started-with-USART-90003216A.pdf 
 
page 5
https://ww1.microchip.com/downloads/aemDocuments/documents/MCU08/ProductDocuments/UserGuides/AVR-BLE-Hardware-User-Guide-DS50002956B.pdf
Check out the CDC stuff

 * Observed Behaviour:
 * ATMega is doing fine, UART is coming out of PF0 exactly correctly. But the avr-ble's debugger is messing with it. Turning it into this hyperspeed
 * nothingness that can't be parsed. Also the nothingness persists (in the micro-usb data pins) regardless of any activity on PF0.
 * I don't know whats going on
 * I could just use USART1 but then i wont be able to easily plug & play with debugging and sending commands and that frustrates me
*/


#ifndef __UART_H_
#define __UART_H_

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "ecode.h"

/* F0 - tx
 * F1 - rx */
#define TX_PIN    PIN0_bm
#define RX_PIN    PIN1_bm

ECODE uart_init(uint32_t baud_rate);
ECODE uart_tx(const char *send);

#endif /* __UART_H_ */