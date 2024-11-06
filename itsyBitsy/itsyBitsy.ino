/*
Datasheets: 
https://learn.adafruit.com/introducting-itsy-bitsy-32u4/pinouts
https://learn.adafruit.com/adafruit-rfm69hcw-and-rfm96-rfm95-rfm98-lora-packet-padio-breakouts/downloads
https://ww1.microchip.com/downloads/en/DeviceDoc/RN2483-LoRa-Technology-Module-Command-Reference-User-Guide-DS40001784G.pdf

https://learn.adafruit.com/adafruit-rfm69hcw-and-rfm96-rfm95-rfm98-lora-packet-padio-breakouts/rfm9x-test

*/

#include <SPI.h>
#include <RH_RF95.h>

#define SCK   15
#define COPI  16
#define CIPO  14
#define CS    18

RH_RF95 rf95(CS, 2);

void sendCommand(char *command) {
    int len = strlen(command);
    digitalWrite(CS, LOW); // enable device
    SPI.transfer(command, len);
    digitalWrite(CS, HIGH); // disable device
}

void receive(char *buf) {
    digitalWrite(CS, LOW); // enable device
    int i = 0;
    int rec = 'r';
    while (rec != 0) {
        rec = SPI.transfer(0);
        buf[i] = rec;
        Serial.println(rec);
        i++;
    }
    digitalWrite(CS, HIGH); // disable device
    buf[i] = '\0';
}

/* the setup function runs once when you press reset or power the board */
void setup() {
    /* initialize digital pin 13 as an output */
    pinMode(13, OUTPUT);

    Serial.begin(9600);
    /* while the serial stream is not open, do nothing */
    while (!Serial);

    /* SPI pins */
    pinMode(SCK, OUTPUT);
    pinMode(COPI, OUTPUT);
    pinMode(CIPO, INPUT);
    pinMode(CS, OUTPUT);
    digitalWrite(CS, HIGH); // disable device

    SPI.begin();
    SPI.setClockDivider(SPI_CLOCK_DIV8); //divide the clock by 8
    sendCommand("sys get ver\r\n"); // send command sys get ver
    char rec[128];
    receive(rec);
    Serial.println("Received:");
    Serial.println(rec);
}

// the loop function runs over and over again forever
void loop() {
    // Serial.println("Received:");
    digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(1000);              // wait for a second
    digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
    delay(1000);              // wait for a second
}
