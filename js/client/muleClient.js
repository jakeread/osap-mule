/*
muleClient.js

Jake Read at the Center for Bits and Atoms
(c) Massachusetts Institute of Technology 2022

This work may be reproduced, modified, distributed, performed, and
displayed for any purpose, but must acknowledge the open systems assembly protocol (OSAP) project.
Copyright is retained and must be preserved. The work is provided as is;
no warranty is provided, and users accept all liability.
*/

'use strict'

import OSAP from '../osapjs/core/osapRoot.js'
import { PK, TS, VT, EP, TIMES } from '../osapjs/core/ts.js'

import Grid from '../osapjs/client/interface/grid.js' // main drawing API 
import { Button, EZButton, TextBlock, TextInput } from '../osapjs/client/interface/basics.js'
import NetDoodler from '../osapjs/client/doodler/netDoodler.js'

console.log("hello mule ui")

// the osap root node:
let osap = new OSAP("mule-client")

let grid = new Grid()

// -------------------------------------------------------- SETUP NETWORK / PORT 

let wscVPort = osap.vPort("wscVPort")

let LOGPHY = false

// to test these systems, the client (us) will kickstart a new process
// on the server, and try to establish connection to it.
console.log("making client-to-server request to start remote process,")
console.log("and connecting to it w/ new websocket")
// ok, let's ask to kick a process on the server,
// in response, we'll get it's IP and Port,
// then we can start a websocket client to connect there,
// automated remote-proc. w/ vPort & wss medium,
// for args, do '/processName.js?args=arg1,arg2'

jQuery.get('/startLocal/osapSerialBridge.js', (res) => {
  if (res.includes('OSAP-wss-addr:')) {
    let addr = res.substring(res.indexOf(':') + 2)
    if (addr.includes('ws://')) {
      let status = "opening"
      wscVPort.cts = () => {
        if (status == "open") {
          return true
        } else {
          return false
        }
      }
      // start up, 
      console.log('starting socket to remote at', addr)
      let ws = new WebSocket(addr)
      ws.binaryType = "arraybuffer"
      // opens, 
      ws.onopen = (evt) => {
        status = "open"
        // implement rx
        ws.onmessage = (msg) => {
          let uint = new Uint8Array(msg.data)
          wscVPort.receive(uint)
        }
        // implement tx 
        wscVPort.send = (buffer) => {
          if (LOGPHY) console.log('PHY WSC Send', buffer)
          ws.send(buffer)
        }
      }
      ws.onerror = (err) => {
        status = "closed"
        console.log('sckt err', err)
      }
      ws.onclose = (evt) => {
        status = "closed"
        console.log('sckt closed', evt)
      }
    }
  } else {
    console.error('remote OSAP not established', res)
  }
})

// ---------------------------------------------- App... 

// it ain't pretty, but we set a window global net doodler instance 
window.nd = new NetDoodler(osap, 0, 0)

/*
let collect = async () => {
  if(!runState) return 
  try {
    window.setState('traversing')
    let net = await osap.netRunner.sweep()
    // check for drags here... 
    if(window.state != 'traversing'){
      console.error('should hang, right?')
    }
    await ddlr.redraw(net)
    setTimeout(collect, 1000)
  } catch (err) {
    console.error(err)
  }
}

let checkRunState = () => {
  if(runState){
    runBtn.green()
    collect()
  } else {
    runBtn.red()
  }
}

checkRunState()
*/