#include <Arduino.h>
#include <Wire.h>
#include "osape/core/osap.h"
#include "osape/vertices/endpoint.h"
#include "osape_arduino/vp_arduinoSerial.h"
#include "osape_arduino/vb_arduinoWire.h"

#define CLK_LED_PIN A0 
#define OUTPUT_LED_PIN A2
#define BTN_INPUT_PIN A3

//#define ONE 

#ifdef ONE 
#define ADDRESS_SELF 12
#define ADDRESS_FRIEND 14 
#else 
#define ADDRESS_SELF 14
#define ADDRESS_FRIEND 12 
#endif 

// ---------------- Root 

OSAP osap("osap-mule-wire");

// ---------------- USB: 0 

VPort_ArduinoSerial vpUSBSerial(&osap, "usbSerial", &Serial);

// ---------------- VBus Wire: 1 

VBus_ArduinoWire vbWire(&osap, "wire-vb", &Wire, ADDRESS_SELF);

// ---------------- Button: 2 

uint8_t btn_down[1] = {1};
uint8_t btn_up[1] = {0};

Endpoint ep_button(&osap, "button");

// ---------------- LED: 3 

EP_ONDATA_RESPONSES onLEDData(uint8_t* data, uint16_t len){
  //DEBUG("rx: " + String(len) + " " + data[0] + " " + data[1]);
  //digitalWrite(A4, !digitalRead(A4));
  if(data[0]){
    digitalWrite(OUTPUT_LED_PIN, HIGH);
  } else {
    digitalWrite(OUTPUT_LED_PIN, LOW);
  }
  return EP_ONDATA_ACCEPT;
}

Endpoint ep_led(&osap, "led", onLEDData);

// ---------------- ARDU 

void setup() {
  // start comms... 
  vpUSBSerial.begin();
  vbWire.begin();
  // hardware 
  pinMode(A1, OUTPUT);
  pinMode(BTN_INPUT_PIN, INPUT_PULLUP);
  pinMode(OUTPUT_LED_PIN, OUTPUT);
  pinMode(CLK_LED_PIN, OUTPUT);
  // add a route out to our pal, 
  ep_button.addRoute((new EndpointRoute(EP_ROUTE_ACKLESS))->bfwd(1, ADDRESS_FRIEND)->sib(3));
}

unsigned long lastTx = 0;

void loop() {
  osap.loop();
  if(lastTx + 50 < millis()){
    lastTx = millis();
    // flash, 
    digitalWrite(CLK_LED_PIN, !digitalRead(CLK_LED_PIN));
    // write new state, 
    if(!digitalRead(BTN_INPUT_PIN)){
      ep_button.write(btn_down, 1);
    } else {
      ep_button.write(btn_up, 1);
    }
  }
}