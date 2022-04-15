#include <Arduino.h>

#include "osape/core/osap.h"
#include "osape/core/vertex.h"
#include "osape/vertices/endpoint.h"
#include "osape_arduino/vp_arduinoSerial.h"
#include "osap_debug.h"

// application kit 
#define BUTTON_PIN 9 
#define LED_PIN 6 

// ------------------------------------ Root Vertex 

OSAP osap("osap-mule");

//Vertex root("osap-mule");

// ------------------------------------ USB Serial VPort 

VPort_ArduinoSerial vpUSBSer(&osap, "arduinoUSBSerial", &Serial);

// ------------------------------------ UART VPort 

VPort_ArduinoSerial vpSer1(&osap, "arduinoSer1", &Serial1);

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
  pinMode(A4, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  // links need beginnings 
  vpUSBSer.begin();
  vpSer1.begin(1000000);
  //setup a route... 
  ep_button.addRoute((new EndpointRoute(EP_ROUTEMODE_ACKED))->pfwd(1)->sib(3)); 
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
  // clock light errand... 
  if(millis() > lastTick + tickTime){
    digitalWrite(13, !digitalRead(13));
    lastTick = millis();
    // button pusher,
    if(!digitalRead(BUTTON_PIN)){
      ep_button.write(btn_down, 2);
    } else {
      ep_button.write(btn_up, 2);
    }
  }
}