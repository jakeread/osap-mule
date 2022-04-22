## OSAP-Mule JS

To run this side of the system, do

`node mule` 

You'll want to install these npm packages first:

`npm install esm`
`npm install ws`
`npm install serialport` 

`mule.js` is the entry point: that launches a server. It will print in the terminal an address to point your browser at, it's normally `localhost:8080` 

The real "entry point" is `client/muleClient.js` - inspect that code first, to get a sense of what's going on. 