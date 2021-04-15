
// code ES6

var SerialPort = require("serialport");

 
class USB {
	
	constructor(tty,objCallback) {
		
	  this.tty = tty;
	  this.objCallback = objCallback; // objet devant traiter le callback

	  this.port = null;
	  this.chunk = "";
	  

	  this.start();
	}
 
	info() {
		return "USB " + this.tty
	}

	callback(s) {
		if (this.objCallback != null) {
			//console.log("redirect callback USB : "+s)
			this.objCallback.callback(s);
		}
		else {
			console.log("callback USB : "+s)
		}
	}

	start() {
	 var usb = this;
	 
		console.log('init : '+usb.tty);
		
		var port = new SerialPort(usb.tty, {
			  baudRate: 9600
			});
		
		usb.port = port;

		port.on('open', function() {
			  console.log('opened : '+usb.tty);
			  
		});

		port.on('error', function(err) {
			  console.log('error ', err.message);
		});

		port.on('close', function () {
			console.log("closed ");
		});

		port.on('data', function(data) {
			//console.log("data = "+data)
			usb.chunk += data;
			//console.log(typeof(port.data)) 
			for (;;) {
				var pos = usb.chunk.indexOf("\n");
				if (pos < 0) break;
				//console.log(usb.chunk.substring(0,pos));
				usb.callback(usb.chunk.substring(0,pos))
				usb.chunk = usb.chunk.substring(pos+1);
			}
		})
		
 
 }
}

module.exports = {
		USB : USB
}
