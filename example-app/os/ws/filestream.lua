auth_or_die("User unauthorized. Please login")
if std.ws.enable() then
	-- read header
	local streaming = true
	while streaming do
		local header = std.ws.header()
		if header then
			if header.mask == 0 then
				print("Data is not masked")
				std.ws.close(1012)
				streaming = false
			elseif header.opcode == std.ws.CLOSE then
				print("Connection closed")
				std.ws.close(1000)
				streaming = false
			elseif header.opcode == std.ws.BIN then
				-- read the file
				local data = std.ws.read(header)
				local cmd = nil
				if data then
					local str = bytes.__tostring(data)
					local b,e = str:find("^%d+")
					if(b) then
						local size = tonumber(str:sub(b,e))
						local path = require("fs.vfs").ospath(str:sub(e+1))
						local file = io.open(path, "rb")
						if file then
							local sum, len = 0,0
							repeat
								local buffer = file:read(size)
								if buffer then
									len = len + #buffer
									cmd = bytes.new(#buffer)
									for i = 1, #buffer do
										cmd[i] = buffer:byte(i)
									end
									std.ws.write_bytes(cmd)
								end
							until not buffer
							file:close()
							print("ospath is  : "..path)
							print("length:",len)
						else
							print(path.." is not found")
							std.ws.close(1011)
						end
					else
						std.ws.close(1011)
					end
				else
					std.ws.close(1011)
					streaming = false
				end
			end
		else
			streaming = false
		end
	end
else
	print("Web socket is not available.")
end
print("Quit streaming")
