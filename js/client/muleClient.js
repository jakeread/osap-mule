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
import DT from '../osapjs/client/interface/domTools.js'
import { Button, EZButton, TextBlock, TextInput } from '../osapjs/client/interface/basics.js'
import NetDoodler from '../osapjs/client/doodler/netDoodler.js'

console.log("hello mule ui")

// we make the OSAP root vertex like this, and give it a name:
let osap = new OSAP("mule-client-newtransport")
// attach that to the global space, some rendering functions use it 
window.osap = osap

// 'grid' is just boilerplate UI code... 
let grid = new Grid()

// in JS, we just instantiate a blank vport and then attach methods to it later on, 
let wscVPort = osap.vPort("wscVPort")

// just going to try writing packets,
let dEp1 = osap.endpoint("dummy1")
let dEp2 = osap.endpoint("dummy2")

//let dmRoute = PK.route().sib(0).pfwd().sib(258).pfwd().bbrd(12).end()
let dmRoute = PK.route().child(0).pfwd().sib(1).end() 
//let dmRoute = PK.route().child(1).sib(2).sib(0).pfwd().sib(1).end()
console.warn(dmRoute)
PK.logPacket(dmRoute, true)
setTimeout(() => {
  osap.ping(dmRoute).then((ms) => {
    console.log(`ping returns at ${ms}ms`)
  }).catch((err) => {
    console.error(`err from ping:`, err)
  })
}, 500)

// ---------------------------------------------- App... 

// we can instantiate new endpoints like this:
let demoEP = osap.endpoint("demoEndpoint")
// and attach handlers for data, 
demoEP.onData = (buffer) => {
  // onData handlers should return promises,
  // these are flowcontrolled... so if we are recieving a big stream of messages we can 
  // extert some backpressure by resolving *later on* 
  return new Promise((resolve, reject) => {
    console.log('demoEP rxd this buffer', buffer)
    // if we resolve it, that buffer's data should be written into the endpoint:
    resolve()
    // if we reject it, the data vanishes, and the endpoint keeps its old state 
  })
}

// we can add routes to endpoints like this:
//demoEP.addRoute(PK.route().sib(0).pfwd().sib(2).pfwd().end())
//demoEP.addRoute(PK.route().sib(0).pfwd().sib(1).end())


// the 'net doodler' is the rendering engine that is *very* new and a little uggo, 
// if you don't want to run it, just comment this line out. 
// it ain't pretty, but we set a window global net doodler instance 
// window.nd = new NetDoodler(osap, 10, 10)

// if you want to run the accelerometer demo, uncomment the lines below: 

/*

let x = null, y = null, z = null 

// throw a demo endpoint down, 
let xEP = osap.endpoint("x_orientation")
xEP.onData = (buffer) => {
  return new Promise((resolve, reject) => {
    let float = TS.read('float32', buffer, 0)
    x = float 
    // get that *float* 
    // console.warn('x rx', float)
    resolve()
  })
}

let yEP = osap.endpoint("y_orientation")
yEP.onData = (buffer) => {
  return new Promise((resolve, reject) => {
    let float = TS.read('float32', buffer, 0)
    y = float 
    // get that *float* 
    // console.warn('y rx', float)
    resolve()
  })
}

let zEP = osap.endpoint("z_orientation")
zEP.onData = (buffer) => {
  return new Promise((resolve, reject) => {
    let float = TS.read('float32', buffer, 0)
    z = float 
    // get that *float* 
    // console.warn('z rx', float)
    resolve()
  })
}

// let's make a three.js thing 

import * as THREE from './three.js'

// init

let threeWidth = 700, threeHeight = 500

const camera = new THREE.PerspectiveCamera( 70, threeWidth / threeHeight, 0.01, 10 );
camera.position.z = 1;

const scene = new THREE.Scene();

const geometry = new THREE.BoxGeometry( 0.5, 0.5, 0.5 );
const material = new THREE.MeshNormalMaterial();

const mesh = new THREE.Mesh( geometry, material );
scene.add( mesh );

const renderer = new THREE.WebGLRenderer( { antialias: true } );
renderer.setSize( threeWidth, threeHeight );
renderer.setAnimationLoop( animation );

let ground = $(`<div></div>`).get(0)
DT.placeField(ground, threeWidth, threeHeight, 10, 450)

ground.appendChild( renderer.domElement );

// animation

function animation( time ) {
  if(x) mesh.rotation.x = THREE.MathUtils.degToRad(360 - x)
  if(y) mesh.rotation.y = THREE.MathUtils.degToRad(360 - y) 
  if(z) mesh.rotation.z = THREE.MathUtils.degToRad(z)

  //mesh.rotation.x = time / 2000;
  //mesh.rotation.y = time / 1000;

  renderer.render( scene, camera );

}

*/

// -------------------------------------------------------- SETUP NETWORK / PORT 

// verbosity 
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
      // i.e. here we attach the "clear to send" function, 
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