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

// link packets are max 256 bytes in length, including the 0 delimiter 
// structured like:
// checksum | pck/ack key | pck id | cobs encoded data | 0 

void linkLoop(arduPortLink_t* link, vertex_t* vt){
  // byte injestion: think of this like the rx interrupt stage, 
  while(link->ser->available() && link->inBufferLen == 0){
    // read ahn byte, 
    link->inBuffer[link->inBufferWp ++] = link->ser->read();
    // check delineation, 
    if(link->inBuffer[link->inBufferWp] == 0){
      if(link->inBuffer[0] != link->inBufferWp){
        // checksum failure, 
        ERROR(3, "p2p bad checksum");
      } else if (link->inBuffer[1] == P2PLINK_KEY_ACK){
        // it's an ack, we can read direct, 
        if(link->inBuffer[2] == link->outBuffer[2]){
          // if IDs match, we are now clear downstream, 
          link->outBufferLen = 0;
        } // if no match, might be 2nd ack for previously cleared, idk 
      } else if (link->inBuffer[1] == P2PLINK_KEY_PCK){
        // it's a packet, check this corner case where we rx same pckt twice, 
        if(link->inBuffer[2] == link->lastPckIdRxd){
          // duplicate, it'll be ignored 
          ERROR(3, "p2p duplicate rx");
        } else {
          // passes that test, track new 
          link->lastPckIdRxd = link->inBuffer[2];
          // set this state, prevents overwrite on next while()
          // and will get picked up in the load check, 
          link->inBufferLen = link->inBufferWp;
        }
      }
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
      link->ackAwaiting[0] = 3; // checksum, always this,
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

void linkSend(arduPortLink_t* link, vport_t* vp, uint8_t* data, uint16_t len){
  // double guard?
  if(!linkCTS(link, vp)) return;
  // setup,
  link->outBuffer[0] = len + 5; // len + 0 delimiter & cobs start + id + key + checksum
  link->outBuffer[1] = P2PLINK_KEY_PCK;
  link->outBuffer[2] = link->nextPckIdTx;
  link->nextPckIdTx ++; if(link->nextPckIdTx == 0) link->nextPckIdTx = 1;
  // encode in... stuffing after header & adding tail zero 
  link->outBufferLen = cobsEncode(data, len, &(link->outBuffer[3]));
  link->outBufferLen += 5;
  link->outBuffer[link->outBufferLen - 1] = 0;
  // check output: expedite this if we can, 
  linkCheckOutputStates(link);
}

boolean linkCTS(arduPortLink_t* link, vport_t* vp){
  // if outBuffer is occupied, this msg not done txing 
  return (link->outBufferLen > 0) ? false : true;
}

void linkCheckOutputStates(arduPortLink_t* link){
  // we always want to prioritize acks, 
  if(link->outBufferLen == 0 && link->ackIsAwaiting){
    memcpy(link->outBuffer, link->ackAwaiting, 4);
    link->ackIsAwaiting = false;
    link->outBufferLen = 4;
  }
  // tx check, first we drain the buffer... ?, 
  while(link->ser->availableForWrite() && link->outBufferLen > 0){
    link->ser->write(link->outBuffer[link->outBufferRp ++]);
    if(link->outBufferRp >= link->outBufferLen){
      link->outBufferLen = 0;
    }
  }
}