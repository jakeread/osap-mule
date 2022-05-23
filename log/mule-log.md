## 2022 02 17 

I'm more or less tracking this in the osap log... better doc later, probably. 

## 2022 05 11 

We're back for the `Wire` linklayer.

I'm daydreaming then about a new, more-better motion control system, maybe to be ready for haystack, and bread-boardable, and using an I2C comms layer... though a lack of real hardware sync on that link makes it seem unlikely. 

But (!) we have a broadcast option, so there is some chance of actually coordinating. Here's what my best guess scheme would do:

- maintaining time siggies like
  - assume everyone can count pretty well in microseconds 
    - we have a 71 minute overflow if we store this in uint32. 
  - at every motion packet broadcast, declare what-time-it-is, in micros 
  - if time appears to have advanced faster or slower, actuators can add prescalers, etc... or something of the sort 
- issuing moves like 
  - motion broadcasts declare also start times & durations for moves 
  - moves are... you know, IDK, position-time beziers would be best... then things can do dead simple eval(time) fns, 
  - some kind of encoded position-time function, irregardless of whether beziers / others
- executing moves like 
  - actuator evaluates pos(time) function, acts accordingly 
  - is free to constrain own action w/ local rules for accel, etc, 

I think I would ideally dev this on the D21 (motors) and probably the teensy... or even a raspberry pi, which can speak Wire, at the head. The bezier-fitting would probably be the heaviest task at the head. There's some other stuff:

- we still have awkward limits on network time, meaning limits to the size of a segment. this is never not true of a motion sysytem

### Spreadsheets 

- how long is a barebones motion packet over I2C ? what is the rate/segsize ratio at 100% network utilization for n dof ? 
- what is dither on the microseconds count ? i.e. if we're broadcasting in the micros(), we probably want some error bands for clock tracking... i.e. time between interrupt-capture and data-retrieve, if we have just Wire() implementation, this is not going to be interrupt based, ffs, so we are potentially in trouble: oh, no, there's a Wire.onReceive function, looks like. 

I think the first move, then, is to "get into it" w/ a broadcaster and an rx'er. 

### Opening Strat

- revive D21 testbed project
- attach onReceive handlers, confirm we can do this
- broadcast from one, onReceive on the other(s), measure dither 
- first we should just try to get a good I2C link up, then the motion... 
- https://docs.google.com/spreadsheets/d/1VcMRa7rTkZCK__YczqhMpv1SFp23fkCifGg8K5mA5Yw/edit#gid=0 

Although - second thought - it might be more fun to do USB coordinated? Which would preface w/ a python link layer, so that would take us to the routing layer rewrite. 