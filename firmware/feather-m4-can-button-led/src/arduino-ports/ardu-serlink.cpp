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
  while(ser->available() && inBufferLen == 0){
    // read ahn byte, 
    inBuffer[inBufferWp ++] = ser->read();
    // check delineation: the - 1 index might seem odd but is always incremented just above 
    if(inBuffer[inBufferWp - 1] == 0){
      if(inBuffer[0] != inBufferWp){
        // checksum failure, 
        ERROR(3, "p2p bad checksum, wp = " + String(inBufferWp) + " cs = " + String(inBuffer[0]));
      } else if (inBuffer[1] == P2PLINK_KEY_ACK){
        // it's an ack, we can read direct, 
        if(inBuffer[2] == outPck[2]){
          // if IDs match, we are now clear downstream, no further action, 
          outPckLen = 0;
        } // if no match, might be 2nd ack for previously cleared, idk 
      } else if (inBuffer[1] == P2PLINK_KEY_PCK){
        // it's a packet, check this corner case where we rx same pckt twice, 
        if(inBuffer[2] == lastPckIdRxd){
          // duplicate, it'll be ignored 
          ERROR(3, "p2p duplicate rx");
        } else {
          // looks like a new packet, track that state
          lastPckIdRxd = inBuffer[2];
          // then set fullness, will get picked up on load check 
          inBufferLen = inBufferWp;
        }
      } // the final case would be an unrecognized key, we bail on that as well, 
      // after *any* delimiter, we reset this,
      inBufferWp = 0;
    } // end delineated case, 
  } // end while(avail) 

  // load check, 
  if(inBufferLen > 0){
    if(stackEmptySlot(this, VT_STACK_ORIGIN)){
      // load up... this adds a 2nd memcpy that'd be tite to delete 
      uint16_t len = cobsDecode(&(inBuffer[3]), inBufferLen - 4, temp);
      stackLoadSlot(this, VT_STACK_ORIGIN, temp, len);
      // we need to issue an ack, 
      ackAwaiting[0] = 4; // checksum, always this, msg (incl. delimiter) is 4 bytes 
      ackAwaiting[1] = P2PLINK_KEY_ACK;
      ackAwaiting[2] = inBuffer[2]; // ack ID is pck ID 
      ackAwaiting[3] = 0; // delimiter
      ackIsAwaiting = true;
      // and we're now clear to read in, 
      inBufferLen = 0;
    } // end stack slot available, we'll just be awaiting here... 
  }

  // check & execute actual tx 
  checkOutputStates();
}

void ArduLinkSerial::send(uint8_t* data, uint16_t len){
  // double guard?
  if(!cts()) return;
  // setup, 
  // setup,
  outPck[0] = len + 5; // len + 0 delimiter & cobs start + id + key + checksum
  outPck[1] = P2PLINK_KEY_PCK;
  outPck[2] = nextPckIdTx;
  nextPckIdTx ++; if(nextPckIdTx == 0) nextPckIdTx = 1;
  // encode in... stuffing after header & adding tail zero 
  outPckLen = cobsEncode(data, len, &(outPck[3]));
  // cobsEncode reports length of encode *without* the addnl delimiter, 
  // so just len + 1, 1 being the 1st byte that COBs inserts. we add an addnl 3, id, key, checksum,
  outPckLen += 4; 
  outPck[outPckLen - 1] = 0;
  // other output settings 
  outNTA = 0;
  outLTAT = 0;
  // check output: expedite this if we can, 
  checkOutputStates();
}

// we are CTS if outPck is not occupied, 
boolean ArduLinkSerial::cts(void){
  return (outPckLen == 0);
}

void ArduLinkSerial::checkOutputStates(void){
  // can we setup a new tx buffer? only if no action on existing transmit, 
  if(txBufferLen == 0 && txBufferRp == 0){
    // acks prioritized, 
    if(ackIsAwaiting){
      // simple, we copy in and tx, 
      memcpy(txBuffer, ackAwaiting, 4);
      ackIsAwaiting = false;
      txBufferLen = 4;
    } else if (outPckLen > 0){
      // we are still awaiting completion of this pck's tx... 
      unsigned long now = micros();
      // transmit states,
      if(outNTA == 0 || (outLTAT + P2PLINK_RETRY_TIME < now && outNTA < P2PLINK_RETRY_MACOUNT)){
        // first transmit, or time's up & haven't retried too many times, 
        memcpy(txBuffer, outPck, outPckLen);
        txBufferLen = outPckLen;
        outNTA ++;
        outLTAT = now;
      } else if (outLTAT + P2PLINK_RETRY_TIME > now){
        // waiting to retransmit, 
      } else {
        // time's up & we're over retransmit count 
        outPckLen = 0;
      }
    }
  } // end new txbuffer, 

  // finally, we write out so long as we can: we aren't guaranteed to get whole pckts out in each fn call 
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