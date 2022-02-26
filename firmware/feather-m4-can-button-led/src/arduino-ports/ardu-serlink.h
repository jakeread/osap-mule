/*
arduino-ports/ardu-serlink.h

turns arduino serial objects into competent link layers, for OSAP 

Jake Read at the Center for Bits and Atoms
(c) Massachusetts Institute of Technology 2022

This work may be reproduced, modified, distributed, performed, and
displayed for any purpose, but must acknowledge the squidworks and ponyo
projects. Copyright is retained and must be preserved. The work is provided as
is; no warranty is provided, and users accept all liability.
*/

#ifndef ARDU_SERLINK_H_
#define ARDU_SERLINK_H_

#include <Arduino.h>
#include "../osape/core/vertex.h"

// buffer is max 256 long for that sweet sweet uint8_t alignment 
#define P2PLINK_BUFSIZE 255
// -1 checksum, -1 packet id, -1 packet type, -2 cobs
#define P2PLINK_SEGSIZE P2PLINK_BUFSIZE - 5
// packet keys; 
#define P2PLINK_KEY_PCK 170  // 0b10101010
#define P2PLINK_KEY_ACK 171  // 0b10101011
// retry settings 
#define P2PLINK_RETRY_MACOUNT 5 
#define P2PLINK_RETRY_TIME 2000  // 255 * 10-bit uart byte * 3mhz = 850us 

#define P2PLINK_LIGHT_ON_TIME 100 // in ms 

typedef struct arduPortLink_t arduPortLink_t;

void linkSetup(arduPortLink_t* link);
void linkLoop(arduPortLink_t* link, Vertex* vt);
void linkSend(arduPortLink_t* link, VPort* vp, uint8_t* data, uint16_t len);
boolean linkCTS(arduPortLink_t* link, VPort* vp);

// this one's internal, 
void linkCheckOutputStates(arduPortLink_t* link);

// note that we use uint8_t write ptrs / etc: and a size of 255, 
// so we are never dealing w/ wraps etc, god bless 

struct arduPortLink_t{
  // UART hardware:
  Uart* ser;
  // inbuffer & write ptr,
  uint8_t inBuffer[P2PLINK_BUFSIZE];
  uint8_t inBufferLen = 0;
  uint8_t inBufferWp = 0;
  // outgoing packet (stash for retransmits)
  uint8_t outPck[P2PLINK_BUFSIZE];
  uint8_t outPckLen = 0;
  // out transmit attmempts, etc:
  uint8_t outNTA = 0; // number of transmit attempts, 
  unsigned long outLTAT = 0; // last transmit attempt time 
  // actually being tx'd: packets or ackets 
  uint8_t txBuffer[P2PLINK_BUFSIZE];
  uint8_t txBufferLen = 0;
  uint8_t txBufferRp = 0;
  // out ack, 
  uint8_t ackAwaiting[4];
  boolean ackIsAwaiting = false;
  // out id state, 
  uint8_t nextPckIdTx = 1; // never zero... 
  // in ack state, 
  uint8_t lastPckIdRxd = 0; // init to zero, shouldn't ever be zero 
  // stash 
  uint8_t temp[P2PLINK_BUFSIZE];
  // constructors 
  arduPortLink_t(Uart* _ser);
};

#endif 