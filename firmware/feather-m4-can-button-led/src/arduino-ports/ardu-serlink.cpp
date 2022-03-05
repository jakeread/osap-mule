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

ArduLinkSerial::ArduLinkSerial( Vertex* _parent, String _name, Uart* _uart
) : VPort ( _parent, _name ){
  stream = _uart; // should convert Uart* to Stream*, as Uart inherits stream 
  uart = _uart; 
}

ArduLinkSerial::ArduLinkSerial( Vertex* _parent, String _name, Serial_* _usbcdc
) : VPort ( _parent, _name ){
  stream = _usbcdc;
  usbcdc = _usbcdc;
}

void ArduLinkSerial::begin(uint32_t baudRate){
  if(uart != nullptr){
    uart->begin(baudRate);
  } else if (usbcdc != nullptr){
    usbcdc->begin(baudRate); 
  }
}

void ArduLinkSerial::begin(void){
  if(uart != nullptr){
    uart->begin(1000000);
  } else if (usbcdc != nullptr){
    usbcdc->begin(9600);  // baud ignored on cdc begin  
  }
}

// link packets are max 256 bytes in length, including the 0 delimiter 
// structured like:
// checksum | pck/ack key | pck id | cobs encoded data | 0 

void ArduLinkSerial::loop(void){
  // byte injestion: think of this like the rx interrupt stage, 
  while(stream->available()){
    // read byte into the current stub, 
    rxBuffer[rxBufferWp ++] = stream->read();
    if(rxBuffer[rxBufferWp - 1] == 0){
      // 1st, we checksum:
      if(rxBuffer[0] != rxBufferWp){ 
        ERROR(3, "serLink bad checksum, cs: " + String(rxBuffer[0]) + " wp: " + String(rxBufferWp + 1));
      } else {
        // acks, packs, or broken things 
        if(rxBuffer[1] == SERLINK_KEY_PCK){
          // dirty guard for retransmitted packets, 
          if(rxBuffer[2] != lastIdRxd){
            inAwaitingId = rxBuffer[2]; // stash ID 
            inAwaitingLen = cobsDecode(&(rxBuffer[3]), rxBufferWp - 2, inAwaiting); // fill inAwaiting 
          } else {
            ERROR(3, "serLink double rx");
          }
        } else if (rxBuffer[1] == SERLINK_KEY_ACK){
          if(rxBuffer[2] == outAwaitingId){
            // clear now, 
            outAwaitingLen = 0;
          }
        } else {
          // a bonkers, broken to shit packet 
        }
      }
      // always reset on delimiter, 
      rxBufferWp = 0;
    }
  } // end while-receive 

  // check insertion & genny the ack if we can 
  if(inAwaitingLen && stackEmptySlot(this, VT_STACK_ORIGIN) && !ackIsAwaiting){
    stackLoadSlot(this, VT_STACK_ORIGIN, inAwaiting, inAwaitingLen);
    ackIsAwaiting = true;
    ackAwaiting[0] = 4;                 // checksum still, innit 
    ackAwaiting[1] = SERLINK_KEY_ACK;   // it's an ack bruv 
    ackAwaiting[2] = inAwaitingId;      // which pck r we akkin m8 
    ackAwaiting[3] = 0;                 // delimiter 
    inAwaitingLen = 0;
  }

  // check & execute actual tx 
  checkOutputStates();
}

void ArduLinkSerial::send(uint8_t* data, uint16_t len){
  //digitalWrite(A4, !digitalRead(A4));
  // double guard?
  if(!cts()) return;
  // setup, 
  outAwaiting[0] = len + 4;               // pck[0] is checksum = len + checksum + cobs start + cobs delimit + ack/pack + id 
  outAwaiting[1] = SERLINK_KEY_PCK;       // this ones a packet m8 
  outAwaitingId ++; if(outAwaitingId == 0) outAwaitingId = 1;
  outAwaiting[2] = outAwaitingId;         // an id     
  cobsEncode(data, len, &(outAwaiting[3]));  // encode 
  outAwaiting[len + 3] = 0;               // stuff delimiter, 
  outAwaitingLen = outAwaiting[0];        // track... 
  // transmit attempts etc 
  outAwaitingNTA = 0;
  outAwaitingLTAT = 0;
  // try it 
  checkOutputStates();                    // try / start write 
}

// we are CTS if outPck is not occupied, 
boolean ArduLinkSerial::cts(void){
  return (outAwaitingLen == 0);
}

void ArduLinkSerial::checkOutputStates(void){
  // can we ack? no real acks for now, 
  if(ackIsAwaiting && txBufferLen == 0){
    memcpy(txBuffer, ackAwaiting, 4);
    txBufferLen = 4;
    txBufferRp = 0;
    ackIsAwaiting = false;
  }
  // would we be clear to tx ? 
  if(outAwaitingLen > 0 && txBufferLen == 0){
    // check retransmit cases, 
    if(outAwaitingLTAT == 0 || outAwaitingLTAT + SERLINK_RETRY_TIME < micros()){
      memcpy(txBuffer, outAwaiting, outAwaitingLen);
      outAwaitingLTAT = micros();
      txBufferLen = outAwaitingLen;
      txBufferRp = 0;
      outAwaitingNTA ++;
    } 
    // check if last attempt, 
    if(outAwaitingNTA >= SERLINK_RETRY_MACOUNT){
      outAwaitingLen = 0;
    }
  }
  // finally, we write out so long as we can: 
  // we aren't guaranteed to get whole pckts out in each fn call 
  while(stream->availableForWrite() && txBufferLen != 0){
    // output next byte, 
    stream->write(txBuffer[txBufferRp ++]);
    // check for end of buffer; reset transmit states if so 
    if(txBufferRp >= txBufferLen) {
      txBufferLen = 0; 
      txBufferRp = 0;
    }
  }
}