auth_or_die("User unauthorized. Please login")
	
local packages={}
local vfs = require("fs.vfs")
local uid = unix.uid(SESSION.iotos_user)

packages._cache = function(y)
	local p = vfs.ospath(y)
	local f = io.open(p.."/packages.json", "w")
	local has_cache = false
	local i = 1
	local meta = {}
	if f then
		local files = vfs.readDir(y)
		for k,v in pairs(files) do
			if v.type == "dir"  then
				local f1 = io.open(vfs.ospath(v.path.."/package.json"))
				if f1 then
					
					local name = std.basename(v.path)
					local mt = JSON.decodeString(f1:read("*all"))
					mt.path = v.path
					meta[i] ='"'..name..'":'..JSON.encode(mt)
					i = i+1
					f1:close()
					has_cache = true;
				end
			end
		end
		f:write(table.concat(meta, ","))
		f:close()
		if has_cache == false then
			unix.delete(p.."/packages.json");
		end
	end
end

-- we will change this later
packages.list = function(paths)
	std.json()
	std.t("{\"result\" : { ")
	local first = true
	--std.f(__ROOT__.."/system/packages.json")
	for k,v in pairs(paths) do
		local osp = vfs.ospath(v.."/packages.json")
		if  unix.exists(osp) == false then
			packages._cache(v)
		end
		if unix.exists(osp) then
			if first == false then 
				std.t(",")
			else
				first = false
			end
			std.f(osp)
		end
	end
	std.t("}, \"error\":false}")
end

-- generate the packages caches
packages.cache = function(args)
	-- perform a packages caches
	for x,y in pairs(args.paths) do
		packages._cache(y)
	end 
	result(true)
end
-- install a function from zip file
packages.install = function(args)
	local path = vfs.ospath(args.dest)
	local zip = vfs.ospath(args.zip)
	if(unix.exists(path) == false) then
		-- create directory if not exist
		unix.mkdir(path)
		-- change permission
		unix.chown(path, uid.id, uid.gid)
	end
	-- extract the zip file to it
	if(unix.unzip(zip, path)) then
		-- read metadata
		local meta = JSON.decodeFile(path.."/metadata.json")
		meta.path = args.dest
		meta.scope = "user"
		local f=io.open(path.."/package.json","w")
		if f then 
			f:write(JSON.encode(meta)) 
			f:close()
		end
		result(true)
	else
		fail("Problem extracting zip file")
	end
	
end
-- uninstall the package
packages.uninstall = function(path)
	local osf = vfs.ospath(path)
	if(osf and unix.exists(osf) ) then
		--remove it
		unix.delete(osf)
		result(true)
	else
		fail("Cannot find package")
	end
end
-- set user packages environment
packages.init = function(paths)
	if(paths) then
		for k,v in pairs(paths) do
			local p = vfs.ospath(v)
			if p and (unix.exists(p) == false) then
				unix.mkdir(p)
				-- change permission
				unix.chown(p, uid.id, uid.gid)
			end
		end
	end
end

-- main()

local rq = (JSON.decodeString(REQUEST.query.json))
packages.init(rq.args.paths)
if rq ~= nil then
	-- check user command here
	if(rq.command == "install") then
		packages.install(rq.args)
	elseif rq.command == "cache" then
		packages.cache(rq.args)
	elseif rq.command == "list" then
		packages.list(rq.args.paths)
	elseif rq.command == "uninstall" then
		packages.uninstall(rq.args.path)
	else
		fail("Uknown packages command")
	end
else
	fail("Uknown request")
end
