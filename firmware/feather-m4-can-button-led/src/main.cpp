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
arduPortLink_t ser1Link(&Serial1);

// that hooks up to osap, 
void loopFn(Vertex* vt){
  linkLoop(&ser1Link, vt);
}

void sendFn(VPort* vp, uint8_t* data, uint16_t len){
  linkSend(&ser1Link, vp, data, len);
}

boolean ctsFn(VPort* vp){
  return linkCTS(&ser1Link, vp);
}

VPort avp_ser1(&root, "arduino-ser", loopFn, sendFn, ctsFn);

// ------------------------------------ Button Endpoint

uint8_t btn_down[1] = {1};
uint8_t btn_up[1] = {0};
//uint8_t btn_route[3] = { PK_SIB_KEY, 3, 0 };
uint8_t btn_route[7] = { PK_SIB_KEY, 1, 0, PK_PFWD_KEY, PK_SIB_KEY, 3, 0 };

Endpoint ep_button(&root, "button");

// ------------------------------------ LED Endpoint 

EP_ONDATA_RESPONSES onLEDData(uint8_t* data, uint16_t len){
  //digitalWrite(LED_PIN, !digitalRead(LED_PIN));
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
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  // this vv is a little confusing for the people, innit? 
  vp_usbSerial_setup(&vp_usbSerial);
  linkSetup(&ser1Link);
  // setup a route... 
  endpointAddRoute(&ep_button, btn_route, 7, EP_ROUTE_ACKED);
}

uint32_t lastTick = 0;
uint32_t tickTime = 1;

void loop() {
  // osap's gotta operate, 
  ERRLIGHT_ON;
  osapMainLoop(&root);
  ERRLIGHT_OFF;
  // endpoints have their own loop
  endpointMainLoop();
  // error light errand... 
  sysErrLightCheck();
  // clock light errand... 
  if(millis() > lastTick + tickTime){
      // button pusher,
    if(!digitalRead(BUTTON_PIN)){
      endpointWrite(&ep_button, btn_down, 2);
    } else {
      endpointWrite(&ep_button, btn_up, 2);
    }
    // clk 
    digitalWrite(13, !digitalRead(13));
    lastTick = millis();
  }
}