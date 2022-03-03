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
#define P2PLINK_RETRY_MACOUNT 2
#define P2PLINK_RETRY_TIME 10000  // microseconds 

#define P2PLINK_LIGHT_ON_TIME 100 // in ms 

// note that we use uint8_t write ptrs / etc: and a size of 255, 
// so we are never dealing w/ wraps etc, god bless 

class ArduLinkSerial : public VPort {
  public:
    // arduino std begin 
    void begin(uint32_t baud);
    // -------------------------------- our own gd send & cts & loop fns, 
    void loop(void) override;
    void checkOutputStates(void);
    void send(uint8_t* data, uint16_t len) override;
    boolean cts(void) override;
    // -------------------------------- Data 
    // UART hardware:
    Uart* ser;
    // incoming, always kept clear to receive: 
    uint8_t rxBuffer[P2PLINK_BUFSIZE];
    uint8_t rxBufferWp = 0;
    // guard on double transmits 
    uint8_t lastIdRxd = 0;
    // incoming stash
    uint8_t inAwaiting[P2PLINK_BUFSIZE];
    uint8_t inAwaitingId = 0;
    uint8_t inAwaitingLen = 0;
    // outgoing ack, 
    uint8_t ackAwaiting[4];
    boolean ackIsAwaiting = false;
    // outgoing await,
    uint8_t outAwaiting[P2PLINK_BUFSIZE];
    uint8_t outAwaitingId = 1;
    uint8_t outAwaitingLen = 0;
    uint8_t outAwaitingNTA = 0;
    unsigned long outAwaitingLTAT = 0;
    // outgoing buffer,
    uint8_t txBuffer[P2PLINK_BUFSIZE];
    uint8_t txBufferLen = 0;
    uint8_t txBufferRp = 0;
    // -------------------------------- Constructors 
    ArduLinkSerial(Vertex* _parent, String _name, Uart* _ser);
};

#endif 