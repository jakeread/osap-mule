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

Vertex root("osap-mule");

// ------------------------------------ (to be improved) USB Serial 

VPort vp_usbSerial(
  &root, "usbSerial", 
  &vp_usbSerial_loop, 
  &vp_usbSerial_send, 
  &vp_usbSerial_cts,
  &vp_usbSerial_onOriginStackClear 
);

// ------------------------------------ UART "Port"

// we want a link object, 
ArduLinkSerial ser1Link(&root, "arduinoSer1", &Serial1);

// ------------------------------------ Button Endpoint

uint8_t btn_down[1] = {1};
uint8_t btn_up[1] = {0};

// #define BTN_ROUTE_LEN 3
// uint8_t btn_route[BTN_ROUTE_LEN] = { PK_SIB_KEY, 3, 0 };
#define BTN_ROUTE_LEN 7
uint8_t btn_route[BTN_ROUTE_LEN] = { PK_SIB_KEY, 1, 0, PK_PFWD_KEY, PK_SIB_KEY, 3, 0 };

Endpoint ep_button(&root, "button");

// ------------------------------------ LED Endpoint 

unsigned long lastDataPull = 0;

EP_ONDATA_RESPONSES onLEDData(uint8_t* data, uint16_t len){
  //digitalWrite(A4, !digitalRead(A4));
  if(data[0]){
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
  return EP_ONDATA_ACCEPT;
}

Endpoint ep_led(&root, "led", onLEDData);

// ------------------------------------ Arduino Stuff 

void setup() {
  pinMode(13, OUTPUT);  // 'clklight'
  pinMode(5, OUTPUT);   // 'errlight'
  pinMode(A4, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  // this vv is a little confusing for the people, innit? 
  vp_usbSerial_setup(&vp_usbSerial);
  ser1Link.begin(1000000);
  // setup a route... 
  ep_button.addRoute(btn_route, BTN_ROUTE_LEN, EP_ROUTE_ACKED);
}

uint32_t lastTick = 0;
uint32_t tickTime = 50;

void loop() {
  // osap's gotta operate, 
  //ERRLIGHT_ON;
  osapMainLoop(&root);
  delay(10);
  // should look like osap.loop(); for the sake of arduino-ness 
  //ERRLIGHT_OFF;
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