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
  link->ser->begin(9600);
}

void linkLoop(arduPortLink_t* link, vertex_t* vt){
  while(link->ser->available()){
    link->inBuffer[0][link->inBufferLen[0] ++] = link->ser->read();
    if(link->inBufferLen[0] >= UP_BUFSIZE) break;
  }
  // rm for now, so we should just see a blink whenever we get bytes, 
  if(link->inBufferLen[0] > 0){
    link->inBufferLen[0] = 0;
    ERRLIGHT_TOGGLE;
  }
}

void linkSend(arduPortLink_t* link, vport_t* vp, uint8_t* data, uint16_t len){
  link->ser->write(data, len);
}

boolean linkCTS(arduPortLink_t* link, vport_t* vp){
  return true;  
}