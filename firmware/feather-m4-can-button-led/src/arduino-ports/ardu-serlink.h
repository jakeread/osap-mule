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
#define UP_BUFSIZE 255
// -1 checksum, -1 packet id, -1 packet type, -2 cobs
#define UP_SEGSIZE UP_BUFSIZE - 5
// packet keys; 
#define UP_KEY_PCK 170  // 0b10101010
#define UP_KEY_ACK 171  // 0b10101011
// retry settings 
#define UP_RETRY_MACOUNT 3 
#define UP_RETRY_TIME 2000  // 255 * 10-bit uart byte * 3mhz = 850us 

#define UP_LIGHT_ON_TIME 100 // in ms 

typedef struct arduPortLink_t arduPortLink_t;

void linkSetup(arduPortLink_t* link);
void linkLoop(arduPortLink_t* link, vertex_t* vt);
void linkSend(arduPortLink_t* link, vport_t* vp, uint8_t* data, uint16_t len);
boolean linkCTS(arduPortLink_t* link, vport_t* vp);

struct arduPortLink_t{
  // UART hardware:
  Uart* ser;
  // inbuffer, 
  uint8_t inBuffer[UP_BUFSIZE];
  uint16_t inBufferWp = 0;
  // stash 
  uint8_t temp[UP_BUFSIZE];
  // constructors 
  arduPortLink_t(Uart* _ser);
};

#endif 