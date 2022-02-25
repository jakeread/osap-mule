/*
arduino-ports/ardu-vport.h

turns serial objects into competent link layers 

Jake Read at the Center for Bits and Atoms
(c) Massachusetts Institute of Technology 2022

This work may be reproduced, modified, distributed, performed, and
displayed for any purpose, but must acknowledge the squidworks and ponyo
projects. Copyright is retained and must be preserved. The work is provided as
is; no warranty is provided, and users accept all liability.
*/

#include "ardu-serlink.h"
#include "./osape/utils/cobs.h"
#include "./osap_debug.h"

arduPortLink_t::arduPortLink_t(Uart* _ser){
  ser = _ser;
}

void linkSetup(arduPortLink_t* link){
  link->ser->begin(115200);
}

void linkLoop(arduPortLink_t* link, vertex_t* vt){
  while(link->ser->available()){
    // read ahn byte, 
    link->inBuffer[link->inBufferWp] = link->ser->read();
    // check delineation, 
    if(link->inBuffer[link->inBufferWp] == 0){
      if(stackEmptySlot(vt, VT_STACK_ORIGIN)){
        // load up, 
        uint16_t len = cobsDecode(link->inBuffer, link->inBufferWp, link->temp);
        stackLoadSlot(vt, VT_STACK_ORIGIN, link->temp, len);
        // reset write ptr 
        link->inBufferWp = 0;
      } else {
        // bad case: can miss 
      }
    } else {
      // increment, but don't go over  
      link->inBufferWp ++;
      if(link->inBufferWp >= UP_BUFSIZE) link->inBufferWp = 0;
    }
  }
}

void linkSend(arduPortLink_t* link, vport_t* vp, uint8_t* data, uint16_t len){
  uint16_t encLen = cobsEncode(data, len, link->temp);
  // stash delimiter 
  link->temp[encLen] = 0;
  link->ser->write(link->temp, encLen + 1);
}

boolean linkCTS(arduPortLink_t* link, vport_t* vp){
  return true;  
}