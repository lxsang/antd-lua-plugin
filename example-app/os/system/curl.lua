auth_or_die("User unauthorized. Please login")
local rq = (JSON.decodeString(REQUEST.query.json))

if rq ~= nil then
	local r,m = require("web").get(rq.url)
	if r then 
		if r.binary then
			result({body="data:"..r.contentType..";base64,"..r.data})
		else
			result({body=r.data})
		end 
	else 
		fail(m) 
	end
else
	fail("Uknown request")
end