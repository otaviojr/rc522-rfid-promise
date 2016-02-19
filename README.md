#rc522-rfid-promise
Module to access an rfid reader with rc522 chipset using linux spidev. Using promises.

## Fork of
This is a fork with added functionality. For a nice and simpler module please see the original
[https://www.npmjs.com/package/rc522-rfid](https://www.npmjs.com/package/rc522-rfid)
[https://www.npmjs.com/package/rc522-rfid-promise](https://www.npmjs.com/package/rc522-rfid-promise)

## Purpose
This node module is to access RFID reader with a rc522 chipset (e.g. http://amzn.com/B00GYR1KJ8) via SPI interface using the linux spidev.

The original projects works only at raspberry pi, since, they used a broadcom lib to communicate with the spi.
At this fork we changed it to use linux spidev instead and works with other boards like beaglebone.

## Functionality
The module is currently only able to read the serial number of the tag which is hold onto the reader.

## Requirements
- The RFID reader is plugged at spidev1.0
- The GCC compiler is installed ```sudo apt-get install build-essential```
- node-gyp is installed ```npm install -g node-gyp```

## Installation
npm install --save otaviojr/rc522-rfid-promise
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
var rc522 = require("rc522-rfid-promise");

rc522.startListening(5000)
	.then(function(rfidTag){
    	console.log(rfidTag);
	});
```
