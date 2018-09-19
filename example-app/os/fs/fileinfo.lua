auth_or_die("User unauthorized. Please login")
local vfspath = (JSON.decodeString(REQUEST.query.json)).path
local r,m = require("fs.vfs").fileinfo(vfspath)
if r then 
	result(m)
else
	fail(m)
end
