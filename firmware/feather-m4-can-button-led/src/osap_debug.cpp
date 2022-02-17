/*
osap_debug.cpp

optional escape hatches & indicators 

Jake Read at the Center for Bits and Atoms
(c) Massachusetts Institute of Technology 2020

This work may be reproduced, modified, distributed, performed, and
displayed for any purpose, but must acknowledge the osap project.
Copyright is retained and must be preserved. The work is provided as is;
no warranty is provided, and users accept all liability.
*/

#include "osap_debug.h"
#include "./osape/core/ts.h"
#include "./osape/utils/cobs.h"

uint8_t errBuf[1028];
uint8_t errEncoded[1028];

volatile unsigned long errLightLastOn = 0;
volatile unsigned long errLightOnTime = 0;
volatile boolean errLightBlink = true;
volatile boolean errLightOn = false;

void sysErrLightFlash(uint8_t level){
  //ERRLIGHT_ON;
  errLightOn = true;
  errLightLastOn = millis();
  switch(level){
    case 0:
      errLightBlink = false;
      break;
    case 1:
      errLightOnTime = 5000;
      break;
    case 2: 
      errLightOnTime = 1000;
      break;
    case 3: 
      errLightOnTime = 250;
      break;
  }
}

void sysErrLightCheck(void){
  if(errLightOn && errLightBlink){
    if(errLightLastOn + errLightOnTime < millis()){
      //ERRLIGHT_OFF;
      errLightOn = false;
    }
  }
}

// config-your-own-ll-escape-hatch
void sysError(String msg){
  // whatever you want,
  //ERRLIGHT_ON;
  uint32_t len = msg.length();
  errBuf[0] = 0; // serport looks for acks in each msg, this is not one
  errBuf[1] = PK_PTR; 
  errBuf[2] = PK_LLESCAPE_KEY; // the ll-errmsg-key
  errBuf[3] = len & 255;
  errBuf[4] = (len >> 8) & 255;
  errBuf[5] = (len >> 16) & 255;
  errBuf[6] = (len >> 24) & 255;
  msg.getBytes(&(errBuf[7]), len + 1);
  size_t ecl = cobsEncode(errBuf, len + 7, errEncoded);
  errEncoded[ecl] = 0;
  // direct escape 
  //if(Serial.availableForWrite() > (int64_t)ecl){
    Serial.write(errEncoded, ecl + 1);
    //Serial.flush();
  //} else {
  //  ERRLIGHT_ON;
  //}
}

//#endif 

void logPacket(uint8_t* pck, uint16_t len){
  String errmsg;
  errmsg.reserve(1024);
  errmsg = "pck: "; // max 64 
  for(uint8_t i = 0; i < 64; i ++){
    if(i >= len) break;
    errmsg += String(pck[i]);
    errmsg += ", ";
  }
  sysError(errmsg);
}