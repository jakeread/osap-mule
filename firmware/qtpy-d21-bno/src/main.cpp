#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
//#include <utility/imumaths.h>
#include <SPI.h>
#include "osape/core/osap.h"
#include "osape/vertices/endpoint.h"
#include "osape_arduino/vp_arduinoSerial.h"

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);

OSAP osap("osap-bno");

VPort_ArduinoSerial vpUSBSerial(&osap, "usbSerial", &Serial);

Endpoint xEP(&osap, "x");
Endpoint yEP(&osap, "y");
Endpoint zEP(&osap, "z");

void setup() {
  pinMode(10, OUTPUT); // "MOSI" label 
  vpUSBSerial.begin();
  if(!bno.begin()){
    while(1);
  }
}

uint32_t lastTick = 0;
sensors_event_t orientationData;
// code hangs occasionally, I suspect some unsafe memcpy in the chain from here to endpoints... 
chunk_float32 tempChunk;

void loop() {
  osap.loop();
  if(lastTick + 50 < millis()){
    digitalWrite(10, !digitalRead(10));
    lastTick = millis();
    bno.getEvent(&orientationData, Adafruit_BNO055::VECTOR_EULER);
    tempChunk.f = orientationData.orientation.x;
    xEP.write(tempChunk.bytes, 4);
    tempChunk.f = orientationData.orientation.y;
    yEP.write(tempChunk.bytes, 4);
    tempChunk.f = orientationData.orientation.z;
    zEP.write(tempChunk.bytes, 4);
  }
}