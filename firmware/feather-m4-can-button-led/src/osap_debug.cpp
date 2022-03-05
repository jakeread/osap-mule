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
  ERRLIGHT_ON;
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
      ERRLIGHT_OFF;
      errLightOn = false;
    }
  }
}

// config-your-own-ll-escape-hatch
void debugPrint(String msg){
  // whatever you want,
  uint32_t len = msg.length();
  errBuf[0] = len + 8;  // len, key, cobs start + end, strlen (4) 
  errBuf[1] = 172;      // serialLink debug key 
  errBuf[2] = len & 255;
  errBuf[3] = (len >> 8) & 255;
  errBuf[4] = (len >> 16) & 255;
  errBuf[5] = (len >> 24) & 255;
  msg.getBytes(&(errBuf[6]), len + 1);
  size_t ecl = cobsEncode(&(errBuf[2]), len + 4, errEncoded);
  memcpy(&(errBuf[2]), errEncoded, ecl);
  errBuf[errBuf[0] - 1] = 0;
  // direct escape 
  Serial.write(errBuf, errBuf[0]);
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
  debugPrint(errmsg);
}