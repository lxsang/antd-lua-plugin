
auth_or_die("User unauthorized. Please login")
local rq = (JSON.decodeString(REQUEST.query.json))

if rq ~= nil then
	local r,m = require("fs.vfs").write(rq.path, rq.data)
	sqlite.dbclose()
	if r then result(r) else fail(m) end
else
	fail("Uknown request")
end


