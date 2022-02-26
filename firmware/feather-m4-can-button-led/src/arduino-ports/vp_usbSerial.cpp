/*
osap/vp_usbSerial.cpp

serial port, virtualized
does single-ended flowcontrol (from pc -> here) 

Jake Read at the Center for Bits and Atoms
(c) Massachusetts Institute of Technology 2019

This work may be reproduced, modified, distributed, performed, and
displayed for any purpose, but must acknowledge the osap project.
Copyright is retained and must be preserved. The work is provided as is;
no warranty is provided, and users accept all liability.
*/

#include "vp_usbSerial.h"
//#include "../osape/core/ts.h"
//#include "../../drivers/indicators.h"
#include "../osape/utils/cobs.h"
//#include "../../syserror.h"

// incoming 
uint8_t _inBuffer[VPUSB_SPACE_SIZE];
uint8_t _pwp = 0; // packet to write into 
uint16_t _bwp = 0; // byte to write at 

// acks left to transmit 
uint8_t _acksAwaiting = 0;

// outgoing
uint8_t _encodedOut[VPUSB_SPACE_SIZE];
uint8_t _encodedIn[VPUSB_SPACE_SIZE];

void vp_usbSerial_setup(VPort* vp){
  Serial.begin(9600);
}

void vp_usbSerial_loop(Vertex* vt){
  // want to count through previous occupied-ness states, and on falling edge
  // of stack education, ack... 
  // ack if necessary (if didn't tx ack out on reciprocal send last)
  if(_acksAwaiting){
    vp_usbSerial_send(vt->vport, _encodedOut, 0);
  }
  // then check about new messages: 
  while(Serial.available()){
    _inBuffer[_bwp] = Serial.read();
    if(_inBuffer[_bwp] == 0){
      // end of COBS-encoded frame, 
      // decode into packet slot, record length (to mark fullness) and record arrival time 
      // check if space in origin stack, 
      uint8_t slot = 0;
      if(stackEmptySlot(vt, VT_STACK_ORIGIN)){
        // decode into decodebuf, load into stack 
        // cobsDecode returns the length of the decoded packet
        uint16_t len = cobsDecode(_inBuffer, _bwp, _encodedIn); 
        stackLoadSlot(vt, VT_STACK_ORIGIN, _encodedIn, len);
        // reset byte write pointer, and find the next empty packet write space 
        _bwp = 0;
      } else {
        //sysError("! serial no space !");
      }
      // reset for next write, 
      _bwp = 0;     
    } else {
      _bwp ++;
    }
  }
}

// to clear packets out... for us to track flowcontrol
void vp_usbSerial_onOriginStackClear(Vertex* vt, uint8_t slot){
  // this is all, 
  _acksAwaiting ++;
}

// there's at the moment no usb -> up flowcontrol 
boolean vp_usbSerial_cts(VPort* vp){
  return true;
}

uint8_t _shift[VPUSB_SPACE_SIZE];

void vp_usbSerial_send(VPort* vp, uint8_t* data, uint16_t len){
  // damn, this is not fast: shifting one byte in for acks,
  // probably faster than sending seperate packet though 
  _shift[0] = _acksAwaiting;
  _acksAwaiting = 0;
  memcpy(&(_shift[1]), data, len);
  // now encode out, 
  size_t encLen = cobsEncode(_shift, len + 1, _encodedOut);
  if(Serial.availableForWrite()){
    _encodedOut[encLen] = 0; // write in final trailing zero, to delineate 
    Serial.write(_encodedOut, encLen + 1);
    Serial.flush();
  } else {
    //sysError("on write, serial not available");
  }
}