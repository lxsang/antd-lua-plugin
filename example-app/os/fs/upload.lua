auth_or_die("User unauthorized. Please login")
local vfs = require("fs.vfs")
if REQUEST.query then
	local r,m = require("fs.vfs").upload(REQUEST.query.path)
	if r then result(r) else fail(m) end
else
	fail("Query not found")
end
