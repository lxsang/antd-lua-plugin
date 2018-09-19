function std.extra_mime(name)
	local ext = std.ext(name);
	local mpath = __ROOT__.."/".."mimes.json"
	local xmimes = {}
	if utils.file_exists(mpath) then
		local f = io.open(mpath, "r")
		if f then
			xmimes = JSON.decodeString(f:read("*all"))
			f:close()
		end
	end
	if(name:find("Makefile$")) then return "text/makefile",false
	elseif ext == "php" then return "text/php",false
	elseif ext == "c" or ext == "h" then return "text/c",false
	elseif ext == "cpp" or ext == "hpp" then return "text/cpp",false
	elseif ext == "md" then return "text/markdown",false
	elseif ext == "lua" then return "text/lua",false
	elseif ext == "yaml" then return "application/x-yaml", false
	elseif xmimes[ext] then return xmimes[ext].mime, xmimes[ext].binary
	--elseif ext == "pgm" then return "image/x-portable-graymap", true
	else 
		return "application/octet-stream",true
	end
end

function std.mimeOf(name)
	local mime = std.mime(name)
	if mime ~= "application/octet-stream" then
		return mime
	else
		return std.extra_mime(name)
	end
end

function std.isBinary(name)
	local mime = std.mime(name)
	if mime ~= "application/octet-stream" then
		return std.is_bin(name)
	else
		local xmime,bin = std.extra_mime(name)
		return bin
	end
end
