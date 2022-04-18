#include <Arduino.h>
#include "wiring_private.h"
#include "osape/core/osap.h"
#include "osape/vertices/endpoint.h"
#include "osape_arduino/vp_arduinoSerial.h"

OSAP osap("osap-mule-router");

VPort_ArduinoSerial vpUSBSerial(&osap, "usbSerial", &Serial);
VPort_ArduinoSerial vpSerial1(&osap, "serial1", &Serial1);
VPort_ArduinoSerial vpSerial2(&osap, "serial2", &Serial2);
VPort_ArduinoSerial vpSerial3(&osap, "serial3", &Serial3);
VPort_ArduinoSerial vpSerial4(&osap, "serial4", &Serial4);

void setup() {
  // setup all of the serials... 
  // these are OK by default, I think ? 
  vpUSBSerial.begin(9600);
  vpSerial1.begin(1000000);
  //Serial1.begin(1000000);  // 0: RX, 1: TX
  // this needs pin reassignment, 
  vpSerial2.begin(1000000);
  //Serial2.begin(1000000); // 11: RX, 10: TX
  pinPeripheral(10, PIO_SERCOM);
  pinPeripheral(11, PIO_SERCOM);
  // on ?
  vpSerial3.begin(1000000);
  //Serial3.begin(1000000); // 22 (MISO): RX, 23 (MOSI): TX
  pinPeripheral(23, PIO_SERCOM_ALT);
  pinPeripheral(22, PIO_SERCOM_ALT);
  // good luck...
  vpSerial4.begin(1000000);
  //Serial4.begin(1000000); // 12: RX, 6: TX
  pinPeripheral(12, PIO_SERCOM_ALT);
  pinPeripheral(6, PIO_SERCOM_ALT);
  // setup 
  pinMode(13, OUTPUT);
}

uint32_t lastTick = 0;

void loop() {
  osap.loop();
  if(lastTick + 50 < millis()){
    lastTick = millis();
    digitalWrite(13, !digitalRead(13));
    // Serial1.write(171);
    // Serial2.write(171);
    // Serial3.write(200);
    // Serial4.write(225);
  }
}