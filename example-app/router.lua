OSROOT = __ROOT__.."/os"
package.path = package.path..";"..__ROOT__ .. '/os/?.lua'

unix = require("ulib")
require("sqlite")

if HEADER["User-Agent"] and HEADER["User-Agent"]:match("Mobi") then
    HEADER.mobile = true
end


function fail(msg)
	std.json()
	std.t(JSON.encode({error=msg}))
end

function result(obj)
	std.json()
	std.t(JSON.encode({result=obj, error=false}))
end

function die (msg)
  fail(msg)
  debug.traceback=nil
  error("Permission denied")
end

-- test only
if REQUEST.path:match("^%/*router%.lua$") or REQUEST.path:match("^%/*router$") then
    die("Recursive call to index.lua is not allown")
end

-- check if the sysdb is create, otherwise create the table
function sysdb()
	local meta = {}
	meta.sessionid = ""
	meta.username = ""
	meta.stamp = 0
	return require("db.model").get("sysdb", "sessions", meta)
end

function is_auth()
	if SESSION.sessionid == nil or SESSION.sessionid == '0' then return false end
	-- query session id from database
	local db = sysdb()
	if db == nil then return false end
	local cond = {exp= {["="] = { sessionid = SESSION.sessionid }}}
	local data = db:find(cond)
	--print(JSON.encode(data))
	db:close()
	if data == nil or data[1] == nil then die(msg) end
	-- next time check the stamp
	SESSION.iotos_user = data[1].username
	return true
end

function auth_or_die(msg)
	if(is_auth() == false) then
		die(msg)
	end
end

local m, s, p  = has_module(REQUEST.path)
if m then
    -- run the correct module
    if s then
		local r,e = loadscript(p)
		if r then r() else fail(e) end
    else
        require(p)
    end
else
	local hstr = REQUEST.path:match("^%a+/")
	if hstr == "os/" then
		--print("require module")
		require("os.router")(REQUEST.path:gsub(hstr,"",1))
	elseif hstr == "blog/" then
		require("blog.router")(REQUEST.path:gsub(hstr,"",1))
	else
		fail("Resource not found for request "..REQUEST.path)
	end
end
