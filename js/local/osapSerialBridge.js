/*
osap-usb-bridge.js

osap bridge to firmwarelandia

Jake Read at the Center for Bits and Atoms
(c) Massachusetts Institute of Technology 2020

This work may be reproduced, modified, distributed, performed, and
displayed for any purpose, but must acknowledge the open systems assembly protocol (OSAP) project.
Copyright is retained and must be preserved. The work is provided as is;
no warranty is provided, and users accept all liability.
*/

// big s/o to https://github.com/standard-things/esm for allowing this
import OSAP from '../osapjs/core/osapRoot.js'
import { TS, PK, TIMES } from '../osapjs/core/ts.js'

import WSSPipe from './utes/wssPipe.js'
import VPortSerial from '../osapjs/vport/vPortSerial.js'

import { SerialPort } from 'serialport'

// we include an osap object - a node
let osap = new OSAP()
osap.name = "local-usb-bridge"
osap.description = "node featuring wss to client and usbserial cobs connection to hardware"

// -------------------------------------------------------- WSS VPort

let wssVPort = osap.vPort("wssVPort")   // 0

// test endpoint, lives at indice #2 
let ep2 = osap.endpoint()
// we can attach 'onData' handlers, which fire whenever something is tx'd to us: 
ep2.onData = (buffer) => {
  return new Promise((resolve, reject) => {
    try {
      // data isn't typed: these are a 'typedarray' (a native javascript type / class, which is memory-competent)
      console.log('the buffer', buffer)
      // in this example, I've transmitted a floating point number, so I want to read that out... as a float, not this serialized set of bytes 
      let fp = TS.read('float32', buffer, 0)
      console.log('the floating point', fp)
    } catch (err) {
      console.error(err)
    }
    resolve()
  })
}

// then resolves with the connected webSocketServer to us 
let LOGWSSPHY = false 
wssVPort.maxSegLength = 16384
WSSPipe.start().then((ws) => {
  // no loop or init code, 
  // implement status 
  let status = "open"
  wssVPort.cts = () => {
    if (status == "open") {
      return true
    } else {
      return false
    }
  }
  // implement rx,
  ws.onmessage = (msg) => {
    if (LOGWSSPHY) console.log('PHY WSS Recv')
    if (LOGWSSPHY) TS.logPacket(msg.data)
    wssVPort.receive(msg.data)
  }
  // implement transmit 
  wssVPort.send = (buffer) => {
    if (LOGWSSPHY) console.log('PHY WSS Send')
    if (LOGWSSPHY) PK.logPacket(buffer)
    ws.send(buffer)
  }
  // local to us, 
  ws.onerror = (err) => {
    status = "closed"
    console.log('wss error', err)
  }
  ws.onclose = (evt) => {
    status = "closed"
    // because this local script is remote-kicked,
    // we shutdown when the connection is gone
    console.log('wss closes, exiting')
    process.exit()
    // were this a standalone network node, this would not be true
  }
})

// -------------------------------------------------------- USB Serial VPort

let findSerialPort = (pid) => {
  console.log(`SERPORT hunt for productId: ${pid}`)
  return new Promise((resolve, reject) => {
    SerialPort.list().then((ports) => {
      let found = false
      for (let port of ports) {
        console.log(`found port w/ productId: ${port.productId}`)
        if (port.productId === pid) {
          found = true
          resolve(port.path)
          break
        }
      }
      if (!found) reject(`serialport w/ productId: ${pid} not found`)
    }).catch((err) => {
      reject(err)
    })
  })
}

// D21 Gemma is 801E, 
// D21 QTPY is 80CB,
// D51 M4 is 8031, 
// D51 CAN is 80CD, 

findSerialPort('80CD').then((portName) => {
  console.log(`FOUND desired prt at ${portName}, launching vport...`)
  let vp = new VPortSerial(osap, portName)
})