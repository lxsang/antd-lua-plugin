auth_or_die("User unauthorized. Please login")

local vfs = require("fs.vfs")
local rq = (JSON.decodeString(REQUEST.query.json))

if rq ~= nil then
	local r,e = vfs.delete(rq.path)
	if r then
		result(r)
	else
		fail(e)
	end
else
	fail("Uknown request")
end