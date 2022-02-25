#include <Arduino.h>

#include "osape/core/osap.h"
#include "osape/core/vertex.h"
#include "arduino-ports/ardu-serlink.h"
#include "osap_debug.h"

// this vv should become another "link" w/ the Serial object passed in, 
// or whatever structure you use for them uart ports 
#include "arduino-ports/vp_usbSerial.h"

// ------------------------------------ Root Vertex 

vertex_t root("osap-mule");

// ------------------------------------ (to be improved) USB Serial 

vport_t vp_usbSerial(
  &root, "usbSerial", 
  &vp_usbSerial_loop, 
  &vp_usbSerial_send, 
  &vp_usbSerial_cts,
  &vp_usbSerial_onOriginStackClear 
);

// ------------------------------------ Serial Codes 

// we want a link object, 
arduPortLink_t ser1Link(&Serial1);

// that hooks up to osap, 
void loopFn(vertex_t* vt){
  linkLoop(&ser1Link, vt);
}

void sendFn(vport_t* vp, uint8_t* data, uint16_t len){
  linkSend(&ser1Link, vp, data, len);
}

boolean ctsFn(vport_t* vp){
  return linkCTS(&ser1Link, vp);
}

vport_t avp_ser1(&root, "arduino-ser", loopFn, sendFn, ctsFn);

void setup() {
  pinMode(13, OUTPUT);
  pinMode(5, OUTPUT);
  // this vv is a little confusing for the people, innit? 
  vp_usbSerial_setup(&vp_usbSerial);
  linkSetup(&ser1Link);
}

uint32_t lastTick = 0;
uint32_t tickTime = 100;

void loop() {
  osapMainLoop(&root);
  if(millis() > lastTick + tickTime){
    digitalWrite(13, !digitalRead(13));
    lastTick = millis();
  }
}