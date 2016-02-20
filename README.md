#rc522-rfid-promise-spidev
Module to access an rfid reader with rc522 chipset using linux spidev. Using promises.

## Fork of
This is a fork to make the module works with many other boards through linux spidev.

For a nice and simpler module, working with raspberry pi, please see the original:

[https://www.npmjs.com/package/rc522-rfid](https://www.npmjs.com/package/rc522-rfid)

[https://www.npmjs.com/package/rc522-rfid-promise](https://www.npmjs.com/package/rc522-rfid-promise)

## Purpose
This node module is to access RFID reader with a rc522 chipset (e.g. http://amzn.com/B00GYR1KJ8) via SPI interface using the linux spidev.

The original projects only works with raspberry pi, since they used a broadcom library to communicate with the spi port.

With this fork I changed it to use linux's spidev instead, which allows it to work with many other boards like beaglebone.

## Functionality
The module is currently only able to read the serial number of the tag which is hold onto the reader.

## Requirements
- The RFID reader is plugged at spidev1.0
- The GCC compiler is installed ```sudo apt-get install build-essential```
- node-gyp is installed ```npm install -g node-gyp```

## Installation
```
npm install --save otaviojr/rc522-rfid-promise-spidev
```

## Api
```
startListening(timeout)
// timeout is optional
// returns a promise

stopListening()
// closes the child process and rejects the promise if still unresolved
```

## Usage
```
var rc522 = require("rc522-rfid-promise-spidev");

var read = function(){
  rc522.startListening({
    spi:"/dev/spidev1.0",
    enableLog: 0
  },5000)
    .then(function(rfidTag){
        console.log("RFID Tag:" + rfidTag);
        setTimeout(read,10);
    }, function(){
        console.log("Timeout - No Card readed");
        setTimeout(read,10);
    });
}

setTimeout(read,10);
```
