auth_or_die("User unauthorized. Please login")
local user = SESSION.iotos_user
if user then
	local ospath = require('fs.vfs').ospath("home:///",user)
	if REQUEST.query and REQUEST.query.json then
		local f = io.open(ospath.."/"..".settings.json", "w")
		if f then
			f:write(REQUEST.query.json)
			f:close()
			result(true)
		else
			fail("Cannot save setting")
		end
	else
		fail("No setting founds")
	end
else
	fail("User not found")
end
