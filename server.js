const http = require("http");
const fs = require("fs");
const socketio = require("socket.io");
const serialPort = require("serialport");

const USB = require("./usb.js").USB;
const mini = require("./mini").mini;

const xbee = new USB("/dev/ttyACM0",{callback : 
	function (s) {
			 if (s.startsWith("{")) {
				 var o = JSON.parse(s)
				 if (typeof o.mm !== 'undefined') {
					console.log("servo = "+o.servo+" mm = "+o.mm+ " rate = "+o.rate)
				}
				if (typeof o.v1 !== 'undefined') {
					console.log("v1 = "+o.v1+" v2 = "+o.v2+ " v3 = "+o.v3+ " v4 = "+o.v4+ " v5 = "+o.v5)
				}
		   }
			 else {
				console.log("XBEE : "+s)
			 }
	}

});

const DB = [];

const server = http.createServer((req, res) => {
    try {
        if(req.url === "/") {
            res.writeHead(200, { "Content-Type": "text/html"});
            const content = fs.readFileSync(__dirname + "/interface.html");
            res.write(content);
        }
    } catch {
        res.write("Une erreur");
    } finally {
        return res.end();
    }
});
const io = socketio(server);

io.on('connection', (socket) => {

    socket.on('command:run', (commands) => {
        xbee.port.write(commands + "\n");
console.log(commands.length);
        io.emit("command:retrieve", commands)
    });


    socket.on('command:get', () => {
        io.emit('command:retrieve', DB.join(""));
    });
});



server.listen(1200);