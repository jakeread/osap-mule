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

ArduLinkSerial::ArduLinkSerial( Vertex* _parent, String _name, Uart* _ser
) : VPort ( _parent, _name ){
  ser = _ser;
}

void ArduLinkSerial::begin(uint32_t baudRate){
  ser->begin(baudRate);
}

// link packets are max 256 bytes in length, including the 0 delimiter 
// structured like:
// checksum | pck/ack key | pck id | cobs encoded data | 0 

void ArduLinkSerial::loop(void){
  // byte injestion: think of this like the rx interrupt stage, 
  while(ser->available() && rxBufferLen == 0){
    // read byte into the current stub, 
    rxBuffer[rxBufferWp ++] = ser->read();
    if(rxBuffer[rxBufferWp - 1] == 0){
      rxBufferLen = rxBufferWp;
      // always reset on delimiter, 
      rxBufferWp = 0;
    }
  } // end while-receive 

  // check for new pcks, 
  if(rxBufferLen){
    if(rxBuffer[0] != rxBufferLen){
      ERROR(3, "p2p bad checksum, wp = " + String(rxBufferLen) + " cs = " + String(rxBuffer[0]));
    } else {
      // can we write it ?
      if(stackEmptySlot(this, VT_STACK_ORIGIN)){
        digitalWrite(A4, !digitalRead(A4));
        uint16_t len = cobsDecode(&(rxBuffer[1]), rxBufferLen - 1, temp);
        stackLoadSlot(this, VT_STACK_ORIGIN, temp, len);
      } else {
        //digitalWrite(A4, LOW);
      }
    }
    // in both cases, clear it
    rxBufferLen = 0;
  }
  // check & execute actual tx 
  checkOutputStates();
}

void ArduLinkSerial::send(uint8_t* data, uint16_t len){
  // double guard?
  if(!cts()) return;
  // setup, 
  txBuffer[0] = len + 2;                  // pck[0] is checksum, len + delimiter + cobs start, 
  cobsEncode(data, len, &(txBuffer[1]));  // encode 
  txBuffer[len + 1] = 0;                  // stuff delimiter, 
  txBufferLen = len + 2;                  // how many bytes to write, 
  txBufferRp = 0;                         // reset to start writing at zero, 
  checkOutputStates();                    // start write 
}

// we are CTS if outPck is not occupied, 
boolean ArduLinkSerial::cts(void){
  return (txBufferLen == 0);
}

void ArduLinkSerial::checkOutputStates(void){
  // finally, we write out so long as we can: 
  // we aren't guaranteed to get whole pckts out in each fn call 
  while(ser->availableForWrite() && txBufferLen != 0){
    // output next byte, 
    ser->write(txBuffer[txBufferRp ++]);
    // check for end of buffer; reset transmit states if so 
    if(txBufferRp >= txBufferLen) {
      txBufferLen = 0; 
      txBufferRp = 0;
    }
  }
}