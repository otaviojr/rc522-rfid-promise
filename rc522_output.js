var rc522 = require('./build/Release/rc522');

var spi_dev = "/dev/spidev1.0";

var myArgs = process.argv.slice(2);

myArgs.forEach(function(element){
   option = element.split("=");
	 if(option[0] == "-D"){
		 	if(option[1] != ""){
		 		spi_dev = option[1];
			}
	 }
});

rc522(function(rfidTagSerialNumber) {
	console.log(rfidTagSerialNumber);
},spi_dev);
