#include <Arduino.h>
//#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_VL53L0X.h>
#include "osape/core/osap.h"
#include "osape/vertices/endpoint.h"
#include "osape_arduino/vp_arduinoSerial.h"
#include "osape_arduino/vb_arduinoWire.h"
#include "osap_debug.h"

#define BTN_INPUT_PIN A1
#define CLK_LED_PIN A2
#define OUTPUT_LED_PIN A3

#define SERPORT_BAUD 576000

// neopix to blink, 
Adafruit_NeoPixel pixel(1, 11, NEO_GRB + NEO_KHZ800);

// lox to lox, 
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

// ---------------- Root 

OSAP osap("osap-mule-wire");

// ---------------- USB: 0 

VPort_ArduinoSerial vpUSBSerial(&osap, "usbSerial", &Serial);

// ---------------- Serial1: 1

VPort_ArduinoSerial vpSerial1(&osap, "Serial1", &Serial1);

// ---------------- VLX: 2

Endpoint vlxEP(&osap, "range");

void updateVlxEndpoint(float range){
  if(range > 1000.0F) range = 1000.0F;
  if(range < 0.0F) range = 0.0F;
  range = range / 1000.0F;
  chunk_float32 chnk = { .f = range };
  vlxEP.write(chnk.bytes, 4);
}

// ---------------- ARDU 

void setup() {
  // start comms... 
  vpUSBSerial.begin();
  vpSerial1.begin(SERPORT_BAUD);
  //vbWire.begin();
  // hardware 
  pinMode(BTN_INPUT_PIN, INPUT_PULLUP);
  pinMode(OUTPUT_LED_PIN, OUTPUT);
  pinMode(CLK_LED_PIN, OUTPUT);
  // neopixel, to blink, data on 11, power on 12 
  pinMode(12, OUTPUT);
  digitalWrite(12, HIGH); // this is power to the neopixel, 
  pixel.begin();
  // lox...
  lox.begin();
  lox.startRangeContinuous();
  // add a route out to our pal, 
  // ep_button.addRoute((new EndpointRoute(EP_ROUTE_ACKLESS))->bfwd(1, ADDRESS_FRIEND)->sib(3));
}

uint32_t lastTx = 0;
boolean lastPixelState = false;

void loop() {
  osap.loop();
  if(lastTx + 50 < millis()){
    lastTx = millis();
    // flash, 
    lastPixelState = !lastPixelState;
    if(lastPixelState){
      pixel.setPixelColor(0, pixel.Color(100, 0, 0));
    } else {
      pixel.setPixelColor(0, pixel.Color(0,0,0));
    }
    pixel.show();
    // write new vlx state...
    updateVlxEndpoint(lox.readRange());
  }
}