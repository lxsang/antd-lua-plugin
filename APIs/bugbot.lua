local pibot = require("pibot")

local bugbot = {}
local cmd = bytes.new(8)
--1 IRL R
--2 IRR R
--3 SNL R 
--4 SNH R 
--5 ML RW 
--6 MR RW 
--7 MLS RW 
--8 MRL RW
bugbot.init = function()
    return pibot.init()
end

bugbot.scan = function()
    local raw = pibot.read(64)
    if raw then
        local data = {}
        data.leftIR = raw[0]
        data.rightIR = raw[1]
        data.sonar = raw[2] + bit32.lshift(raw[3], 8)
        data.motors = {}
        data.motors.left = {}
        data.motors.right = {}
        data.motors.left.status = raw[4]
        data.motors.left.speed = raw[5]
        data.motors.right.status = raw[6]
        data.motors.right.speed = raw[7]
        return data
    end
    return nil
end


bugbot.forward = function(sp)
    cmd[5] = 1 -- fw 
    cmd[6] = sp -- fw 
    cmd[7] = 1
    cmd[8] = sp 
    pibot.write(cmd)
end

bugbot.action = function(st1,sp1,st2,sp2)

    cmd[5] = st1 -- bw 
    cmd[6] = sp1
    cmd[7] = st2 -- bw 
    cmd[8] = sp2
    
    pibot.write(cmd)
end

bugbot.backward = function(sp)
    cmd[5] = 2 -- bw 
    cmd[6] = sp -- bw 
    cmd[7] = 2
    cmd[8] = sp 
    pibot.write(cmd)
end

bugbot.stop = function()
    cmd[5] = 0 -- s 
    cmd[6] = 0 -- s
    cmd[7] = 0 
    cmd[8] = 0
    pibot.write(cmd)
end

bugbot.rotateLeft = function(sp)
    cmd[5] = 2 -- bw 
    cmd[6] = sp -- fw 
    cmd[7] = 1
    cmd[8] = sp 
    pibot.write(cmd)
end

bugbot.rotateRight = function(sp)
    cmd[5] = 1 -- fw 
    cmd[6] = sp -- bw 
    cmd[7] = 2
    cmd[8] = sp 
    pibot.write(cmd)
end

bugbot.release = function()
    return pibot.release()
end
return bugbot