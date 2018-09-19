auth_or_die("User unauthorized. Please login")
local rq = (JSON.decodeString(REQUEST.query.json))
if rq then
		if rq.command == "list" then
			users = {}
			local uid = unix.uid(SESSION.iotos_user)
			if uid then
				users[0] = {
					username = SESSION.iotos_user,
					id = uid.id,
					name = SESSION.iotos_user,
					groups = {"admin"}
				}
				result(users)
			else
				fail("Problem when retreive users")
			end
		else
			fail("command "..rq.command.." is not supported yet")
		end
else
	fail("Unknow request")
end
