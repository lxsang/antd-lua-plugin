local use_ws = false
if REQUEST.query and REQUEST.query.ws == "1" then
	-- override the global echo command
	echo = std.ws.swrite
	use_ws = true
else
	std.json()
end
local exec_with_user_priv = function(data)
    local uid = unix.uid(SESSION.iotos_user)
	if not unix.setgid(uid.gid) or not unix.setuid(uid.id) then
	    echo("Cannot set permission to execute the code")
	    return
    end
	local r,e
	e = "{'error': 'Unknow function'}"
	if data.code then
		r,e = load(data.code)
		if r then
			local status,result = pcall(r)
			if(status) then
				echo(JSON.encode(result))
			else
				echo(result)
			end 
		else
			echo(e)
		end
	elseif data.path then
		r,e = loadfile(data.path)
		if r then
			local status,result = pcall(r, data.parameters)
			if(status) then
				echo(JSON.encode(result))
			else
				echo(result)
			end 
		else
			echo(e)
		end
	else
		echo(e)
	end
end

if(is_auth()) then
	local pid = unix.fork()
	if(pid == -1) then
		echo("{'error':'Cannot create process'}")
	elseif pid > 0 then -- parent
		-- wait for the child exit
		unix.waitpid(pid)
		print("Parent exit")
	else -- child
		if use_ws then
			if std.ws.enable() then
				-- read header
				local header = std.ws.header()
				if header then
					if header.mask == 0 then
						print("Data is not masked")
						std.ws.close(1012)
					elseif header.opcode == std.ws.CLOSE then
						print("Connection closed")
						std.ws.close(1000)
					elseif header.opcode == std.ws.TEXT then
						-- read the file
						local data = std.ws.read(header)
						if data then
							data = (JSON.decodeString(data))
							exec_with_user_priv(data)
							std.ws.close(1011)
						else
							echo("Error: Invalid  request")
							std.ws.close(1011)
						end
					end
				else
					std.ws.close(1011)
				end
			else
				print("Web socket is not available.")
			end
		else
			if REQUEST.query.json then
				data = JSON.decodeString(REQUEST.query.json)
				std.json()
				exec_with_user_priv(data)
			else
				fail("Unkown request")
			end
		end
		print("Child exit")
	end
else
	echo('{"error":"User unauthorized. Please login"}')
end