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

import { SerialPort, DelimiterParser } from 'serialport'

import COBS from './utes/cobs.js'

// we include an osap object - a node
let osap = new OSAP()
osap.name = "local-usb-bridge"
osap.description = "node featuring wss to client and usbserial cobs connection to hardware"

// -------------------------------------------------------- WSS VPort

let wssVPort = osap.vPort("wssVPort")   // 0
let serVPort = osap.vPort("serVPort");  // 1

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

// have some "protocol" at the link layer 
// buffer is max 256 long for that sweet sweet uint8_t alignment 
let SERLINK_BUFSIZE = 255
// -1 checksum, -1 packet id, -1 packet type, -2 cobs
let SERLINK_SEGSIZE = SERLINK_BUFSIZE - 5
// packet keys; 
let SERLINK_KEY_PCK = 170  // 0b10101010
let SERLINK_KEY_ACK = 171  // 0b10101011
let SERLINK_KEY_DBG = 172   
// retry settings 
let SERLINK_RETRY_MACOUNT = 2
let SERLINK_RETRY_TIME = 100  // milliseconds  

serVPort.maxSegLength = 255 // lettuce do this for embedded expectations
let LOGSER = true
let LOGSERTX = false 
let LOGSERRX = false  
let findSerialPort = (pid) => {
  if (LOGSER) console.log(`SERPORT hunt for productId: ${pid}`)
  return new Promise((resolve, reject) => {
    SerialPort.list().then((ports) => {
      let found = false
      for (let port of ports) {
        if(LOGSER) console.log(`found port w/ productId: ${port.productId}`)
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

let opencount = 0

// options: passthrough for node-serialport API
let startSerialPort = (pid, options) => {
  // implement status
  let status = "opening"
  let flowCondition = () => { return false }
  serVPort.cts = () => { 
    if(status == "open" && flowCondition()){
      return true
    } else {
      return false 
    }
  }
  // open up,
  findSerialPort(pid).then((portName) => {
    // hello open 
    if (true) console.log(`SERPORT contact at ${portName}, opening`)
    // the port, 
    let port = new SerialPort({
      path: portName, 
      baudRate: 9600 
    })
    let pcount = opencount
    opencount++
    // buffer... 
    let outAwaiting = null
    let outAwaitingId = 1
    let outAwaitingTimer = null 
    let numRetries = 0 
    let lastIdRxd = 0 
    port.on('open', () => {
      // we track remote open spaces, this is stateful per link... 
      console.log(`SERPORT at ${portName} #${pcount} OPEN`)
      // is now open,
      status = "open"
      // fc is now this, 
      flowCondition = () => { return (outAwaiting == null) }
      // to get, use delimiter
      let parser = port.pipe(new DelimiterParser({ delimiter: [0] }))
      //let parser = port.pipe(new ByteLength({ length: 1 }))
      // implement rx
      parser.on('data', (buf) => {
        if(LOGSERRX) console.log('SERPORT Rx', buf) 
        // checksum... 
        if(buf.length + 1 != buf[0]){
          console.log(`SERPORT Rx Bad Checksum, ${buf[0]} reported, ${buf.length} received`)
          return
        }
        // ack / pack: check and clear, or noop 
        if(buf[1] == SERLINK_KEY_ACK){
          if(buf[2] == outAwaitingId){
            outAwaiting = null 
          } 
        } else if(buf[1] == SERLINK_KEY_PCK){
          if(buf[2] == lastIdRxd){
            console.log(`SERPORT Rx double id`)
            return 
          } else {
            lastIdRxd = buf[2] 
            let decoded = COBS.decode(buf.slice(3))
            serVPort.awaitStackAvailableSpace(0, 2000).then(() => {
              //console.log('SERPORT RX Decoded', decoded)
              serVPort.receive(decoded)
              // output an ack, 
              let ack = new Uint8Array(4)
              ack[0] = 4
              ack[1] = SERLINK_KEY_ACK
              ack[2] = lastIdRxd 
              ack[3] = 0 
              port.write(ack)
            })
          }
        } else if (buf[1] == SERLINK_KEY_DBG){
          let decoded = COBS.decode(buf.slice(2))
          let str = TS.read('string', decoded, 0, true).value; console.log("LL: ", str);
        }
      })
      // implement tx
      serVPort.send = (buffer) => {
        // double guard, idk
        if(!flowCondition()) return;
        // buffers, uint8arrays, all the same afaik 
        // we are len + cobs start + cobs delimit + pck/ack + id + checksum ? 
        outAwaiting = new Uint8Array(buffer.length + 5)
        outAwaiting[0] = buffer.length + 5 
        outAwaiting[1] = SERLINK_KEY_PCK
        outAwaitingId ++; if(outAwaitingId > 255) outAwaitingId = 1;
        outAwaiting[2] = outAwaitingId
        outAwaiting.set(COBS.encode(buffer), 3)
        // reset retry states 
        clearTimeout(outAwaitingTimer)
        numRetries = 0 
        // ship eeeet 
        if(LOGSERTX) console.log('SERPORT Tx', outAwaiting)
        port.write(outAwaiting)
        // retry timeout, in reality USB is robust enough, but codes sometimes bungle messages too 
        outAwaitingTimer = setTimeout(() => {
          if(outAwaiting && numRetries < SERLINK_RETRY_MACOUNT){
            port.write(outAwaiting)
            numRetries ++ 
          } else if (!outAwaiting){
            // noop
          } else {
            // cancel 
            outAwaiting = null 
          }
        }, SERLINK_RETRY_TIME)
      }
    }) // end on-open
    port.on('error', (err) => {
      status = "closing"
      console.log(`SERPORT #${pcount} ERR`, err)
      port.close(() => { // await close callback, add 1s buffer
        console.log(`SERPORT #${pcount} CLOSED`)
        status = "closed"
      })
    })
    port.on('close', (evt) => {
      console.log('SERPORT closing')
      status = "closing"
      console.log(`SERPORT #${pcount} closed`)
    })
  }).catch((err) => {
    if (LOGSER) console.log(`SERPORT cannot find device at ${pid}`, err)
    status = "closed"
  })
} // end start serial

// D21 is 801E, D51 M4 is 8031, D51 CAN is 80CD 
startSerialPort('80CD')

/*
serVPort.requestOpen = () => {
  startSerialPort('801E')
}
*/