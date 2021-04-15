
var replaceall = require("replaceall")

function mini(lvars,cmd) {
    //console.log(def)
    Object.entries(lvars).forEach(
        ([key, val]) => {
            //console.log("alias = "+key+" "+val)
            cmd = replaceall(key,val,cmd)
        }
    )
        
    console.log("cmd len = "+cmd.length)

    return cmd
}

module.exports = {
    mini : mini
}
