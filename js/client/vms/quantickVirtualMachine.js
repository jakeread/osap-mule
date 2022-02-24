/*
quantickVirtalMachine.js

vm for smart motors  

Jake Read at the Center for Bits and Atoms
(c) Massachusetts Institute of Technology 2022

This work may be reproduced, modified, distributed, performed, and
displayed for any purpose, but must acknowledge the open systems assembly protocol (OSAP) project.
Copyright is retained and must be preserved. The work is provided as is;
no warranty is provided, and users accept all liability.
*/

import { PK, TS, TIMES } from '../../osapjs/core/ts.js'

export default function QuantickVM(osap, route){
  // stacked up states 
  let pulseCountQuery = osap.query(PK.route(route).sib(2).end())
  this.getPulseCount = () => {
    return new Promise((resolve, reject) => {
      pulseCountQuery.pull().then((data) => {
        let count = TS.read('uint32', data, 0)
        let pulseWidth = TS.read('float32', data, 4)
        let mpErrCount = TS.read('uint32', data, 8)
        let idxErrCount = TS.read('uint32', data, 12)
        let qErrCount = TS.read('uint32', data, 16)
        resolve([count, pulseWidth, mpErrCount, idxErrCount, qErrCount])
      })
    })
  }
  // make torque requests 
  let torqueRequestEP = osap.endpoint()
  torqueRequestEP.addRoute(PK.route(route).sib(3).end())
  this.setTorque = (flt) => {
    return new Promise((resolve, reject) => {
      // -1.0 -> 1.0, embedded will clamp 
      let wptr = 0
      let datagram = new Uint8Array(4)
      wptr += TS.write('float32', flt, datagram, wptr)
      torqueRequestEP.write(datagram, "acked").then(() => {
        resolve()
      }).catch((err) => { reject(err) })
    })
  }

}