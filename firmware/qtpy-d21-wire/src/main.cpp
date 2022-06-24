#include <Arduino.h>
//#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include "osape/core/osap.h"
#include "osape/core/ts.h"
#include "osape/vertices/endpoint.h"
#include "osape_arduino/vp_arduinoSerial.h"
#include "osape_arduino/vb_arduinoWire.h"

#define BTN_INPUT_PIN A1
#define CLK_LED_PIN A2
#define OUTPUT_LED_PIN A3

#define SERPORT_BAUD 576000

// neopix to blink, 

Adafruit_NeoPixel pixel(1, 11, NEO_GRB + NEO_KHZ800);

// ---------------- Root 

OSAP osap("osap-mule-wire");

// ---------------- USB: 0 

VPort_ArduinoSerial vpUSBSerial(&osap, "usbSerial", &Serial);

// ---------------- Serial1: 1

VPort_ArduinoSerial vpSerial1(&osap, "Serial1", &Serial1);

// ---------------- VBus Wire: 2 
/*
//#define ONE 

#ifdef ONE 
#define ADDRESS_SELF 12
#define ADDRESS_FRIEND 14 
#else 
#define ADDRESS_SELF 14
#define ADDRESS_FRIEND 12 
#endif 

VBus_ArduinoWire vbWire(&osap, "wire-vb", &Wire, ADDRESS_SELF);
*/
// ---------------- Button: 2 

/*

uint8_t btn_down[1] = {1};
uint8_t btn_up[1] = {0};

Endpoint ep_button(&osap, "button");

*/

// ---------------- LED: 3 (2 w/o button... (!))

EP_ONDATA_RESPONSES onLEDData(uint8_t* data, uint16_t len){
  //DEBUG("rx: " + String(len) + " " + data[0] + " " + data[1]);
  //digitalWrite(A4, !digitalRead(A4));
  String msg = "endpoint rx " + String(len) + " bytes: ";
  for(uint8_t i = 0; i < len; i ++){
    msg += String(data[i]) + ", ";
  }
  OSAP::debug(msg);
  /*
  if(data[0]){
    digitalWrite(OUTPUT_LED_PIN, HIGH);
  } else {
    digitalWrite(OUTPUT_LED_PIN, LOW);
  }
  */
  return EP_ONDATA_ACCEPT;
}

Endpoint ep_led(&osap, "led", onLEDData);

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
  // add a route out to our pal, 
  ep_led.addRoute((new Route())->sib(0)->pfwd()->sib(0)->pfwd()->sib(2), EP_ROUTEMODE_ACKED);
}

uint32_t lastTx = 0;
boolean lastPixelState = false;
uint8_t writeDiv = 0;
uint8_t writeData[5] = { 1, 2, 3, 4, 5 };

void loop() {
  // loop occasionally, for debug... 
  if(lastTx + 100 < millis()){
    osap.loop();
    lastTx = millis();
    // flash, 
    lastPixelState = !lastPixelState;
    if(lastPixelState){
      pixel.setPixelColor(0, pixel.Color(100, 0, 0));
    } else {
      pixel.setPixelColor(0, pixel.Color(0,0,0));
    }
    pixel.show();
    // ...
    if(ep_led.clearToWrite()){
      for(uint8_t i = 0; i < 5; i ++){
        writeData[i] ++;
      }
      ep_led.write(writeData, 5);
    }
    // OSAP::debug("debug test 123...");
    /*
    // write new state, 
    if(!digitalRead(BTN_INPUT_PIN)){
      ep_button.write(btn_down, 1);
    } else {
      ep_button.write(btn_up, 1);
    }
    */
  }
}