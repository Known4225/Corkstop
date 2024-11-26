// Arduino9x_RX
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messaging client (receiver)
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example Arduino9x_TX
// workspace link: https://adafruit.github.io/arduino-board-index/package_adafruit_index.json
// also uses RadioHead library, and the code is modified from: https://github.com/adafruit/RadioHead/blob/master/examples/feather/Feather9x_RX/Feather9x_RX.ino

/*
Radio datasheets:
https://cdn-learn.adafruit.com/downloads/pdf/adafruit-rfm69hcw-and-rfm96-rfm95-rfm98-lora-packet-padio-breakouts.pdf
https://cdn-shop.adafruit.com/product-files/3076/sx1231.pdf
*/

/*
itsy-bitsy pinouts:
https://learn.adafruit.com/introducting-itsy-bitsy-32u4/pinouts
*/

#include <SPI.h>
#include <RH_RF95.h>

/* Chip Select pin - A0, PF7 */
#define RFM95_CS_PIN         18
/* Reset pin - A1, PF6 */
#define RFM95_RST_PIN        19
/* G0 interrupt pin - PD0 */
#define RFM95_INT_PIN        0
/* lora LED indicator - PB7 */
#define LORA_LED_PIN         11

/* ADC - A2, PF5, ADC5 */
#define ADC_PIN              20
/* threshold ADC output to consider power on */
#define ADC_THRESH           100

/* continuity green and red channels - PD6, PC7 */
#define GREEN_CONT_LED_PIN   12
#define RED_CONT_LED_PIN     13

/* Relay pin - PD3 */
#define RELAY_PIN            1

/* Change to 433.0 or other frequency, must match RX's freq! */
#define RF95_FREQ 433.0

/* Singleton instance of the radio driver */
RH_RF95 lora(RFM95_CS_PIN, RFM95_INT_PIN);

void setup() {
    /* setup pins */
    pinMode(LORA_LED_PIN, OUTPUT);
    pinMode(RFM95_RST_PIN, OUTPUT);
    pinMode(GREEN_CONT_LED_PIN, OUTPUT);
    pinMode(RED_CONT_LED_PIN, OUTPUT);
    pinMode(RELAY_PIN, OUTPUT);
    // no need to setup ADC_PIN, that is handled by analogRead()

    digitalWrite(RELAY_PIN, LOW); // THE MOST IMPORTANT PIN
    digitalWrite(RFM95_RST_PIN, HIGH);
    digitalWrite(GREEN_CONT_LED_PIN, HIGH); // start with yellow
    digitalWrite(RED_CONT_LED_PIN, HIGH);

    Serial.begin(9600); // baud rate 9600
    delay(100);

    Serial.println("Arduino LoRa RX Test!");

    /* manual reset */
    digitalWrite(RFM95_RST_PIN, LOW);
    delay(10);
    digitalWrite(RFM95_RST_PIN, HIGH);
    delay(10);

    /* initialise lora */
    while (!lora.init()) {
        Serial.println("LoRa radio init failed");
        while (1);
    }
    Serial.println("LoRa radio init OK!");

    /* Defaults after init are 433.0MHz, modulation GFSK_Rb250Fd250, +13dbM */
    if (!lora.setFrequency(RF95_FREQ)) {
        Serial.println("setFrequency failed");
        while (1);
    }
    Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
    lora.setPayloadCRC(false);

    // The default transmitter power is 13dBm, using PA_BOOST.
    // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
    // you can set transmitter powers from 5 to 23 dBm:
    lora.setTxPower(20, false);

    // 433.0MHz, 20dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC off
}

uint8_t ledToggle = LOW;
uint32_t ledLastOn = 0;
uint16_t adcValue = 0;
uint8_t continuity = 0;
uint32_t lastADC = 0;
uint8_t printADC = 0;

void loop() {
    /* collect continuity info four times a second */
    if (millis() - lastADC > 250) {
        adcValue = analogRead(ADC_PIN);
        printADC++;
        if (printADC == 4) {
            Serial.print("ADC: ");
            Serial.println(adcValue);
            printADC = 0;
        }
        if (adcValue > ADC_THRESH) {
            digitalWrite(GREEN_CONT_LED_PIN, HIGH);
            digitalWrite(RED_CONT_LED_PIN, LOW);
            continuity = 1;
        } else {
            digitalWrite(GREEN_CONT_LED_PIN, LOW);
            digitalWrite(RED_CONT_LED_PIN, HIGH);
            continuity = 0;
        }
        lastADC = millis();
    }
    /* turn off lora LED */
    if (millis() - ledLastOn > 500) {
        digitalWrite(LORA_LED_PIN, LOW);
    }
    if (lora.available()) {
        /* Should be a message for us now */
        uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
        uint8_t len = sizeof(buf);

        if (lora.recv(buf, &len)) {
            Serial.print("Received: \"");
            Serial.print((char*) buf);
            Serial.print("\"\r\n");
            Serial.print("RSSI: ");
            Serial.println(lora.lastRssi(), DEC);
            if (len == 5 && strcmp(buf, "cork") == 0) {
                /* turn on lora LED */
                ledLastOn = millis();
                digitalWrite(LORA_LED_PIN, HIGH);
                
                /* Send a reply */
                if (continuity) {
                    uint8_t message[] = {'s', 't', 'o', 'p', '\0'};
                    lora.send(message, sizeof(message));
                    lora.waitPacketSent();
                    Serial.println("Sent reply \"stop\"\r\n");
                } else {
                    uint8_t message[] = {'s', 't', 'a', 'l', '\0'};
                    lora.send(message, sizeof(message));
                    lora.waitPacketSent();
                    Serial.println("Sent reply \"stal\"\r\n");
                }
            } else if (len == 7 && strcmp(buf, "IGNITE") == 0) {
                /* IGNITE */
                if (continuity) {
                    /* done */
                    digitalWrite(RELAY_PIN, HIGH);
                    delay(5); // relay pin triggers for 5ms
                    digitalWrite(RELAY_PIN, LOW);
                    uint8_t message[] = {'d', 'o', 'n', 'e', '\0'};
                    lora.send(message, sizeof(message));
                    lora.waitPacketSent();
                    Serial.println("Sent reply \"done\"\r\n");
                } else {
                    /* can't */
                    uint8_t message[] = {'c', 'a', 'n', 't', '\0'};
                    lora.send(message, sizeof(message));
                    lora.waitPacketSent();
                    Serial.println("Sent reply \"cant\"\r\n");
                }
            }
        } else {
            Serial.println("Receive failed");
        }
    }
}