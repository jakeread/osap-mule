/*
osap_debug.h

optional escape hatches & indicators 

Jake Read at the Center for Bits and Atoms
(c) Massachusetts Institute of Technology 2020

This work may be reproduced, modified, distributed, performed, and
displayed for any purpose, but must acknowledge the osap project.
Copyright is retained and must be preserved. The work is provided as is;
no warranty is provided, and users accept all liability.
*/

#ifndef OSAP_DEBUG_H_
#define OSAP_DEBUG_H_

#include <Arduino.h>

void debugPrint(String msg);
void logPacket(uint8_t* pck, uint16_t len);
//void sysError(uint8_t* bytes, uint16_t len);

void sysErrLightFlash(uint8_t level);
void sysErrLightCheck(void);

#define ERROR(level, msg) sysErrLightFlash(level); debugPrint(msg)
#define DEBUG(msg) debugPrint(msg)

#define ERRLIGHT_ON 
#define ERRLIGHT_OFF
#define ERRLIGHT_TOGGLE 

#endif 