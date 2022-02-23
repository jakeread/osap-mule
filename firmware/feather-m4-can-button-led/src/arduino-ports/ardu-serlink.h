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

void linkLoop(arduPortLink_t* link, vertex_t* vt);
void linkSend(arduPortLink_t* link, vport_t* vp, uint8_t* data, uint16_t len);
void linkCTS(arduPortLink_t* link, vport_t* vp);

struct arduPortLink_t{
  // UART hardware:
  Uart* ser;
  // the port # 
  uint8_t portNum = 0;
  // the current time;
  volatile unsigned long now = 0;
  // ack awaiting,
  uint8_t ackAwaiting[4];
  volatile boolean ackIsAwaiting = false;
  // item awaiting tx,
  uint8_t outPck[UP_BUFSIZE];
  volatile uint8_t outPckLen = 0;             // length of pck to tx 
  volatile uint8_t outPckNTA = 0;             // number of transmit attempts
  volatile unsigned long outPckLTAT = 0;      // last time attempted transmit 
  // flying packet: what we are currently writing to hw 
  uint8_t outBuffer[UP_BUFSIZE];
  volatile uint8_t outBufferWp = 0;           // write ptr
  volatile uint8_t outBufferLen = 0;          // len of thing 
  volatile uint8_t lastPckIdTxd = 1;          // most recently tx'd packet id 
  // incoming data 
  uint8_t inBuffer[2][UP_BUFSIZE];            // incoming catch 
  uint8_t inBufferLen[2] = { 0, 0 };          // len / presence state 
  volatile uint8_t inHead = 0;                // which 2 write 2 
  volatile uint8_t inTail = 1;                // which 2 read from 
  volatile uint8_t inBufferWp = 0;            // which byte 2 write 
  volatile uint8_t lastPckIdRxd = 0;
  uint8_t inBufferDecoded[UP_BUFSIZE];        // to cobs-decode into 
  // ... light info 
  volatile unsigned long portLightLastOn = 0;
  volatile boolean portLightOn = false; 
  // constructors 
  arduPortLink_t(Uart* _ser);
};

#endif 