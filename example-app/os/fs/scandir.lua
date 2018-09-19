auth_or_die("User unauthorized. Please login")
local vfspath = (JSON.decodeString(REQUEST.query.json)).path
local r = require("fs.vfs").readDir(vfspath)
if r == nil then
	fail("Resource not found")
else
	--print(JSON.encode(readDir(ospath, vfspath)))
	result(r)
end

