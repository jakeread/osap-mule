/*
osap/vt_usbSerial.h

virtual port, p2p

Jake Read at the Center for Bits and Atoms
(c) Massachusetts Institute of Technology 2019

This work may be reproduced, modified, distributed, performed, and
displayed for any purpose, but must acknowledge the osap project.
Copyright is retained and must be preserved. The work is provided as is;
no warranty is provided, and users accept all liability.
*/

#ifndef VPORT_USBSERIAL_H_
#define VPORT_USBSERIAL_H_

#include <Arduino.h>
#include "../osape/core/vertex.h" // this is a 'vertex' definition, extending osap for device specific COM 

#define VPUSB_NUM_SPACES 16
#define VPUSB_SPACE_SIZE 512

// uuuuh classes are cancelled? 
void vp_usbSerial_setup(vport_t* vp);
void vp_usbSerial_loop(vertex_t* vt);
void vp_usbSerial_onOriginStackClear(vertex_t* vt, uint8_t slot);
boolean vp_usbSerial_cts(vport_t* vp);
void vp_usbSerial_send(vport_t* vp, uint8_t* data, uint16_t len);

#endif
