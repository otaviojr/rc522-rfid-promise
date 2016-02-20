var spi_dev = "/dev/spidev0.0";
var enable_log = 0;

var myArgs = process.argv.slice(2);

myArgs.forEach(function(element){
   option = element.split("=");
	 if(option[0] == "-D"){
		 	if(option[1] != ""){
		 		spi_dev = option[1];
			}
	 } else if(option[0] == "-L") {
     if(option[1] != ""){
        enable_log = option[1];
     }
   }
});

var rc522 = require('./build/Release/rc522');
rc522.enableLog(enable_log);
rc522.init(spi_dev);
rc522.read(function(rfidTagSerialNumber) {
	console.log(rfidTagSerialNumber);
});
