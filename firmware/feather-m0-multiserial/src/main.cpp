#include <Arduino.h>
#include "wiring_private.h"

void setup() {
  // these are OK by default, I think ? 
  Serial1.begin(115200);  // 0: RX, 1: TX
  // this needs pin reassignment, 
  Serial2.begin(115200); // 11: RX, 10: TX
  pinPeripheral(10, PIO_SERCOM);
  pinPeripheral(11, PIO_SERCOM);
  // on ?
  Serial3.begin(115200); // 22 (MISO): RX, 23 (MOSI): TX
  pinPeripheral(23, PIO_SERCOM_ALT);
  pinPeripheral(22, PIO_SERCOM_ALT);
  // good luck...
  Serial4.begin(115200); // 12: RX, 6: TX
  pinPeripheral(12, PIO_SERCOM_ALT);
  pinPeripheral(6, PIO_SERCOM_ALT);
  // setup 
  pinMode(13, OUTPUT);
}

uint32_t lastTick = 0;

void loop() {
  if(lastTick + 50 < millis()){
    lastTick = millis();
    digitalWrite(13, !digitalRead(13));
    Serial1.write(171);
    Serial2.write(171);
    Serial3.write(200);
    Serial4.write(225);
  }
}