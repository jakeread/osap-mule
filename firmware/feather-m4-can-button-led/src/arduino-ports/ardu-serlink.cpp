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

arduPortLink_t::arduPortLink_t(Uart* _ser){
  ser = _ser;
}

void linkLoop(arduPortLink_t* link, vertex_t* vt){

}

void linkSend(arduPortLink_t* link, vport_t* vp, uint8_t* data, uint16_t len){

}

void linkCTS(arduPortLink_t* link, vport_t* vp){
  
}