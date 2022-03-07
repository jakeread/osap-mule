#include <Arduino.h>

#include "osape/core/osap.h"
#include "osape/core/vertex.h"
#include "osape/vertices/endpoint.h"
#include "arduino-ports/ardu-serlink.h"
#include "osap_debug.h"

// this vv should become another "link" w/ the Serial object passed in, 
// or whatever structure you use for them uart ports 
#include "arduino-ports/vp_usbSerial.h"

// application kit 
#define BUTTON_PIN 9 
#define LED_PIN 6 

// ------------------------------------ Root Vertex 

OSAP osap("osap-mule");

//Vertex root("osap-mule");

// ------------------------------------ USB Serial 

ArduLinkSerial ser0Link(&osap, "arduinoUSBSerial", &Serial);

// ------------------------------------ UART "Port"

// we want a link object, 
ArduLinkSerial ser1Link(&osap, "arduinoSer1", &Serial1);

// ------------------------------------ Button Endpoint

uint8_t btn_down[1] = {1};
uint8_t btn_up[1] = {0};

Endpoint ep_button(&osap, "button");

// ------------------------------------ LED Endpoint 

unsigned long lastDataPull = 0;

EP_ONDATA_RESPONSES onLEDData(uint8_t* data, uint16_t len){
  //DEBUG("rx: " + String(len) + " " + data[0] + " " + data[1]);
  //digitalWrite(A4, !digitalRead(A4));
  if(data[0]){
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
  return EP_ONDATA_ACCEPT;
}

Endpoint ep_led(&osap, "led", onLEDData);

// ------------------------------------ Arduino Stuff 

void setup() {
  pinMode(13, OUTPUT);  // 'clklight'
  pinMode(5, OUTPUT);   // 'errlight'
  pinMode(A4, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  // links need beginnings 
  ser0Link.begin();
  ser1Link.begin(1000000);
  // setup a route... 
  ep_button.addRoute((new EndpointRoute(EP_ROUTE_ACKED))->pfwd(1)->sib(3)); 
  //RouteBuilder* rt = (new RouteBuilder())->sib(3)->pfwd(1);
  // ep_button.addRoute(&epr_button);
}

uint32_t lastTick = 0;
uint32_t tickTime = 500;

void loop() {
  // osap's gotta operate, 
  osap.loop();
  // error light errand... 
  sysErrLightCheck();
  //digitalWrite(A4, HIGH);
  // high speed
  // if(!digitalRead(BUTTON_PIN)){
  //   ep_button.write(btn_down, 2);
  // } else {
  //   ep_button.write(btn_up, 2);
  // }
  // clock light errand... 
  if(millis() > lastTick + tickTime){
    //DEBUG("test");
    // button pusher,
    if(!digitalRead(BUTTON_PIN)){
      ep_button.write(btn_down, 2);
    } else {
      ep_button.write(btn_up, 2);
    }
    // clk 
    digitalWrite(13, !digitalRead(13));
    lastTick = millis();
  }
}