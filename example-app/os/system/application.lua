auth_or_die("User unauthorized. Please login")
local rq = nil
if REQUEST.query.json ~= nil then
	rq  = (JSON.decodeString(REQUEST.query.json))
else
	rq = REQUEST.query
end

if rq.path ~= nil then
	local pkg = require("fs.vfs").ospath(rq.path)
	if pkg == nil then
		pkg = OSROOT..'/packages/'..rq.path
		--die("unkown request path:"..rq.path) 
	end
	pkg = pkg.."/api.lua"
	if unix.exists(pkg) then
		dofile(pkg).exec(rq.method,rq.arguments)
	else
		fail("Uknown  application handler: "..pkg)
	end
else
	fail("Uknown request")
end

