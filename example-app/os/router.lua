local handle = function(p)
	local hstr = p:match("^%a+/")
	if hstr == "fs/" then
		--print("require module")
		require("fs.fsh")(p:gsub(hstr,"",1))
	else
		fail("Resource not found for request "..p)
	end
end
return handle