var USB = require("./usb.js").USB
var mini = require("./mini").mini

// minicom -D /dev/ttyACM0 -b 9600

var mm = null;

var xbee = new USB("/dev/ttyACM0",{callback : 
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

})

setTimeout(() => {
	xbee.port.write("[don][mr 150][ml 150]\n")
}, 1000)

setTimeout(() => {
	xbee.port.write("[don][mr 0][ml 0]\n")
}, 2000)

setTimeout(() => {
	xbee.port.write("[don][mr -150][ml -150]\n")
}, 3000)

setTimeout(() => {
	xbee.port.write("[don][mr 0][ml 0]\n")
}, 4000)

// setTimeout(() => {
// 	xbee.port.write("[s 40]\n")
// }, 1000)

// setTimeout(() => {
// 	xbee.port.write("[dist]\n")
// }, 2000)
// setTimeout(() => {
// 	xbee.port.write("[print]\n")
// }, 3000)

var lvars = {
	speed1 : "v1", 
	speed2 : "v2", 
	servo_dir : "v3",
	servo_incr : "v4",
	boucle : "b0", 
}

setTimeout(() => {
	
	// mesure des distances sur 180° devant le robot
	// une mesure par seconde
	var cmd1 =
		"[don]"+ // debug on
		"[servo_dir = 0][servo_incr = 10]"+
		"[:boucle]"+
		"[s servo_dir][dist][pdist][pvar]"+
		"[servo_dir += servo_incr]"+
		"[if servo_dir > 180 b1]"+
		"[if servo_dir < 0 b2]"+
		"[g b3]"+
		"[:b1][servo_incr = -10][servo_dir += servo_incr][servo_dir += servo_incr][g b3]"+
		"[:b2][servo_incr = 10][servo_dir += servo_incr][servo_dir += servo_incr][g b3]"+
		
		"[:b3][w 1000]"+
		"[g boucle]"+
		"\n"
		
	// boucle : avancer une seconde - reculer une seconde - attendre
	var cmd2 = 
		"[don]"+ // debug on
		"[speed1 = 150][speed2 = -150]"+
		"[:boucle]"+
		"[mr speed1][ml speed1]"+
		"[w 1000]"+
		"[mr 0][ml 0]"+
		"[w 1000]"+
		"[mr speed2][ml speed2]"+
		"[w 1000]"+
		"[mr 0][ml 0]"+
		"[w 1000000]"+
		"[g boucle]"+
		"\n"
		
		

		var cmd = cmd2

		xbee.port.write(mini(lvars,cmd))
		

	
}, 5000)

/*
[s1 = 0][s2 = 10]
[b0:][s1 += s2][s s1]
[dist][print]
[t1 = time][t1 += 3000]
[b1:]
[if time < t1 b1]
[g b0]
*/

// setTimeout(() => {
// 	xbee.sendFrame(
// 		"[s1 = 0]" + // position servo
// 		"[s2 = 10]"+ // increment servo
// 		"[b1:]"+
// 		"[s s1][dist][print]"+
// 		"[t1 = time][t1 += 1000]"+
// 		"[b2:][if time < t1 b2]"+
// 		"[s1 += s2]"+
// 		"[if s1 > 180 b3]"+
// 		"[if s1 < 0 b4]"+
// 		"[g b1]"+
// 		"[b3:][s1 = 180][s2 = -10][s1 += s2][g b1]"+
// 		"[b4:][s1 = 0][s2 = 10][s1 += s2][g b1]"
// 	)
// }, 5000)






