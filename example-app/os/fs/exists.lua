auth_or_die("User unauthorized. Please login")

local vfs = require("fs.vfs")
local rq = (JSON.decodeString(REQUEST.query.json))

if rq ~= nil then
	result(vfs.exists(rq.path))
else
	fail("Uknown request")
end
