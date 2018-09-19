if REQUEST.query.json ~= nil then
	local request = JSON.decodeString(REQUEST.query.json)
	local r = unix.auth(request.username,request.password)
	if r == true then
		local cookie = {sessionid=std.sha1(request.username..request.password)} -- iotos_user = request.username
		local db = sysdb();
		if db == nil then return fail("Cannot setup session") end
		local cond = {exp= {["="] = { sessionid = cookie.sessionid }}}
		local data = db:find(cond)
		--print(data)
		if data == nil or data[1] == nil then
			--print("insert new data")
			data = {sessionid = cookie.sessionid, username=request.username, stamp=os.time(os.date("!*t"))}
		else
			data = data[1]
			--print("Update old data")
			data.stamp = os.time(os.date("!*t"))
		end
		if data.id == nil then
			db:insert(data)
		else
			db:update(data)
		end
		db:close()
		std.cjson(cookie)
		SESSION.iotos_user = request.username
		local user = {
	    	result = require("system.uman").userinfo(request.username),
	    	error = false
		}
		std.t(JSON.encode(user))
	else
		fail("Invalid login")
	end
else
	fail("Invalid request")
end


