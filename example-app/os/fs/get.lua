local get
get =  function(uri)
	vfsfile  = utils.decodeURI(uri)
	auth_or_die("User unauthorized. Please login")
	local r,m = require("fs.vfs").checkperm(vfsfile,'read')
	if r then
		local mime = std.mimeOf(m)
		if mime == "audio/mpeg" then
			local finfo = unix.file_stat(m)
			local len = tostring(math.floor(finfo.size))
			local len1 = tostring(math.floor(finfo.size-1))
			std.status(200, "OK")
			std.custom_header("Pragma","public")
			std.custom_header("Expires","0")
			std.custom_header("Content-Type",mime)
			std.custom_header("Content-Length",len)
			std.custom_header("Content-Disposition","inline; filename="..std.basename(m))
			std.custom_header("Content-Range:","bytes 0-"..len1.."/"..len)
			std.custom_header("Accept-Ranges","bytes")
			std.custom_header("X-Pad", "avoid browser bug")
			std.custom_header("Content-Transfer-Encoding", "binary")
			std.custom_header("Cache-Control","no-cache, no-store")
			std.custom_header("Connection", "Keep-Alive")
			std.custom_header("Etag","a404b-c3f-47c3a14937c80")
			std.header_flush()
		else
			std.header(mime)
		end
		
		if std.is_bin(m) then
			std.fb(m)
		else
			std.f(m)
		end
	else
		fail(m)
	end
end

return get;
