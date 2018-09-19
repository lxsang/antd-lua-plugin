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
				local data = std.ws.read(header)
				if data then
					local path = (__ROOT__.."/ws/img%d.jpg"):format(data[0])
					print("writing : "..path)
					std.ws.fwrite(path)
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
