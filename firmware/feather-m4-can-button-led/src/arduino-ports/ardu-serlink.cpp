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
  link->ser->begin(1000000);
}

// link packets are max 256 bytes in length, including the 0 delimiter 
// structured like:
// checksum | pck/ack key | pck id | cobs encoded data | 0 

void linkLoop(arduPortLink_t* link, Vertex* vt){
  // byte injestion: think of this like the rx interrupt stage, 
  while(link->ser->available() && link->inBufferLen == 0){
    // read ahn byte, 
    link->inBuffer[link->inBufferWp ++] = link->ser->read();
    // check delineation: the - 1 index might seem odd but is always incremented just above 
    if(link->inBuffer[link->inBufferWp - 1] == 0){
      if(link->inBuffer[0] != link->inBufferWp){
        // checksum failure, 
        ERROR(3, "p2p bad checksum, wp = " + String(link->inBufferWp) + " cs = " + String(link->inBuffer[0]));
      } else if (link->inBuffer[1] == P2PLINK_KEY_ACK){
        // it's an ack, we can read direct, 
        if(link->inBuffer[2] == link->outPck[2]){
          // if IDs match, we are now clear downstream, no further action, 
          link->outPckLen = 0;
        } // if no match, might be 2nd ack for previously cleared, idk 
      } else if (link->inBuffer[1] == P2PLINK_KEY_PCK){
        // it's a packet, check this corner case where we rx same pckt twice, 
        if(link->inBuffer[2] == link->lastPckIdRxd){
          // duplicate, it'll be ignored 
          ERROR(3, "p2p duplicate rx");
        } else {
          // looks like a new packet, track that state
          link->lastPckIdRxd = link->inBuffer[2];
          // then set fullness, will get picked up on load check 
          link->inBufferLen = link->inBufferWp;
        }
      } // the final case would be an unrecognized key, we bail on that as well, 
      // after *any* delimiter, we reset this,
      link->inBufferWp = 0;
    } // end delineated case, 
  } // end while(avail) 

  // load check, 
  if(link->inBufferLen > 0){
    if(stackEmptySlot(vt, VT_STACK_ORIGIN)){
      // load up... this adds a 2nd memcpy that'd be tite to delete 
      uint16_t len = cobsDecode(&(link->inBuffer[3]), link->inBufferLen - 4, link->temp);
      stackLoadSlot(vt, VT_STACK_ORIGIN, link->temp, len);
      // we need to issue an ack, 
      link->ackAwaiting[0] = 4; // checksum, always this, msg (incl. delimiter) is 4 bytes 
      link->ackAwaiting[1] = P2PLINK_KEY_ACK;
      link->ackAwaiting[2] = link->inBuffer[2]; // ack ID is pck ID 
      link->ackAwaiting[3] = 0; // delimiter
      link->ackIsAwaiting = true;
      // and we're now clear to read in, 
      link->inBufferLen = 0;
    } // end stack slot available, we'll just be awaiting here... 
  }

  // check & execute actual tx 
  linkCheckOutputStates(link);
}

void linkSend(arduPortLink_t* link, VPort* vp, uint8_t* data, uint16_t len){
  // double guard?
  if(!linkCTS(link, vp)) return;
  // setup,
  link->outPck[0] = len + 5; // len + 0 delimiter & cobs start + id + key + checksum
  link->outPck[1] = P2PLINK_KEY_PCK;
  link->outPck[2] = link->nextPckIdTx;
  link->nextPckIdTx ++; if(link->nextPckIdTx == 0) link->nextPckIdTx = 1;
  // encode in... stuffing after header & adding tail zero 
  link->outPckLen = cobsEncode(data, len, &(link->outPck[3]));
  // cobsEncode reports length of encode *without* the addnl delimiter, 
  // so just len + 1, 1 being the 1st byte that COBs inserts. we add an addnl 3, id, key, checksum,
  link->outPckLen += 4; 
  link->outPck[link->outPckLen - 1] = 0;
  // other output settings 
  link->outNTA = 0;
  link->outLTAT = 0;
  // check output: expedite this if we can, 
  linkCheckOutputStates(link);
}

boolean linkCTS(arduPortLink_t* link, VPort* vp){
  // if outBuffer is occupied, this msg not done txing 
  return (link->outPckLen == 0) ? true : false;
}

void linkCheckOutputStates(arduPortLink_t* link){
  // can we setup a new tx buffer? only if no action on existing transmit, 
  if(link->txBufferLen == 0 && link->txBufferRp == 0){
    // acks prioritized, 
    if(link->ackIsAwaiting){
      // simple, we copy in and tx, 
      memcpy(link->txBuffer, link->ackAwaiting, 4);
      link->ackIsAwaiting = false;
      link->txBufferLen = 4;
    } else if (link->outPckLen > 0){
      // we are still awaiting completion of this pck's tx... 
      unsigned long now = micros();
      // transmit states,
      if(link->outNTA == 0 || (link->outLTAT + P2PLINK_RETRY_TIME < now && link->outNTA < P2PLINK_RETRY_MACOUNT)){
        // first transmit, or time's up & haven't retried too many times, 
        memcpy(link->txBuffer, link->outPck, link->outPckLen);
        link->txBufferLen = link->outPckLen;
        link->outNTA ++;
        link->outLTAT = now;
      } else if (link->outLTAT + P2PLINK_RETRY_TIME > now){
        // waiting to retransmit, 
      } else {
        // time's up & we're over retransmit count 
        link->outPckLen = 0;
      }
    }
  } // end new txbuffer, 

  // finally, we write out so long as we can: we aren't guaranteed to get whole pckts out in each fn call 
  while(link->ser->availableForWrite() && link->txBufferLen != 0){
    // output next byte, 
    link->ser->write(link->txBuffer[link->txBufferRp ++]);
    // check for end of buffer; reset transmit states if so 
    if(link->txBufferRp >= link->txBufferLen) {
      link->txBufferLen = 0; 
      link->txBufferRp = 0;
    }
  }
}