
package.cpath = __api__.apiroot..'/?.so'
require("antd")

local readline = function()
    local s = ""
    repeat
        local c = modules.std().antd_recv(HTTP_REQUEST.id,1)
        if c ~= 0 and c ~= 10 then
            s = s..utf8.char(c)
        end
    until(c == 0 or c == 10)
    return s
end
local read_header =function()
    repeat
        local l = readline()
        if l ~= '\r' then
            if l == "HTTP_REQUEST" or l == "request" or l == "COOKIE" or l == "REQUEST_HEADER" or l == "REQUEST_DATA" then
                coroutine.yield(l, "LUA_TABLE")
            else
                local l1 = readline()
                if l1 ~= '\r' then
                    coroutine.yield(l, l1)
                end
                l = l1
            end
        end
    until l == '\r'
end


local read_headers = function()
    local co = coroutine.create(function () read_header() end)
      return function ()   -- iterator
        local code, k, v = coroutine.resume(co)
        return k,v
      end
end

local parse_headers =function()
    local lut = {
        HTTP_REQUEST = HTTP_REQUEST
    }
    local curr_tbl = "HTTP_REQUEST"
    for k,v in read_headers() do
        if v == "LUA_TABLE" then
            if not lut[k] then
                lut[k] = {}
            end
            curr_tbl = k
        else
            lut[curr_tbl][k] = v
        end
    end
    HTTP_REQUEST.request = lut.request
    HTTP_REQUEST.request.COOKIE = lut.COOKIE
    HTTP_REQUEST.request.REQUEST_HEADER = lut.REQUEST_HEADER
    HTTP_REQUEST.request.REQUEST_DATA = lut.REQUEST_DATA
end

-- parsing the header
parse_headers()
-- root dir
__ROOT__ = HTTP_REQUEST.request.SERVER_WWW_ROOT
-- set require path
package.path = __ROOT__ .. '/?.lua;'..__api__.apiroot..'/?.lua'
require("std")
require("utils")
require("extra_mime")
ulib = require("ulib")
-- set session
SESSION = {}

REQUEST = HTTP_REQUEST.request.REQUEST_DATA
REQUEST.method = HTTP_REQUEST.request.METHOD
if HTTP_REQUEST.request.COOKIE then
	SESSION = HTTP_REQUEST.request.COOKIE
end
HEADER = HTTP_REQUEST.request.REQUEST_HEADER
HEADER.mobile = false

if HEADER["User-Agent"] and HEADER["User-Agent"]:match("Mobi") then
    HEADER.mobile = true
end

function LOG_INFO(fmt,...)
    ulib.syslog(5,string.format(fmt, table.unpack({...})))
end

function LOG_ERROR(fmt,...)
    ulib.syslog(3,string.format(fmt, table.unpack({...})))
end

function has_module(m)
	if utils.file_exists(__ROOT__..'/'..m) then
		if m:find("%.ls$") then
			return true, true,  __ROOT__..'/'..m
		else
			return true, false, m:gsub(".lua$","")
		end
	elseif utils.file_exists(__ROOT__..'/'..string.gsub(m,'%.','/')..'.lua') then
		return true, false, m
	elseif utils.file_exists(__ROOT__..'/'..string.gsub(m,'%.','/')..'.ls') then
		return true, true, __ROOT__..'/'..string.gsub(m,'%.','/')..'.ls'
	end
	return false, false, nil
end

function echo(m)
    if m then std.t(m) else std.t("Undefined value") end
end

function loadscript(file, args)
    local f = io.open(file, "rb")
    local content = ""
    if f then
        local html = ""
        local pro = "local fn = function(...)"
        local s,e, mt
        local mtbegin = true -- find begin of scrit, 0 end of scrit
        local i = 1
        if args then
            pro = "local fn = function("..table.concat( args, ",")..")"
        end
        for line in io.lines(file) do
            line = std.trim(line, " ")
            if(line ~= "") then
                if(mtbegin) then
                    mt = "^%s*<%?lua"
                else
                    mt = "%?>%s*$"
                end
                s,e = line:find(mt)
                if(s) then
                    if mtbegin then
                        if html ~= "" then
                            pro= pro.."echo(\""..utils.escape(html).."\")\n"
                            html = ""
                        end
                        local b,f  = line:find("%?>%s*$")
                        if b then
                            pro = pro..line:sub(e+1,b-1).."\n"
                        else
                            pro = pro..line:sub(e+1).."\n"
                            mtbegin = not mtbegin
                        end
                    else
                        pro = pro..line:sub(0,s-1).."\n"
                        mtbegin = not mtbegin
                    end
                else -- no match
                    if mtbegin then
                        -- detect if we have inline lua with format <?=..?>
                        local b,f = line:find("<%?=")
                        if b then
                            local tmp = line
                            pro= pro.."echo("
                            while(b) do
                                -- find the close
                                local x,y = tmp:find("%?>")
                                if x then
                                    pro = pro.."\""..utils.escape(html..tmp:sub(0,b-1):gsub("%%","%%%%")).."\".."
                                    pro = pro..tmp:sub(f+1,x-1)..".."
                                    html = ""
                                    tmp = tmp:sub(y+1)
                                    b,f = tmp:find("<%?=")
                                else
                                    error("Syntax error near line "..i)
                                end
                            end
                            pro = pro.."\""..utils.escape(tmp:gsub("%%","%%%%")).."\")\n"
                        else
                            html = html..std.trim(line," "):gsub("%%","%%%%").."\n"
                        end
                    else
                        if line ~= "" then pro = pro..line.."\n" end
                    end
                end
            end
            i = i+ 1
        end
        f:close()
        if(html ~= "") then
            pro = pro.."echo(\""..utils.escape(html).."\")\n"
        end
        pro  = pro.."\nend \n return fn"
        local r,e = load(pro)
        if r then return r(), e else return nil,e end
    end
end

local decode_post_data = function(ctype, clen, is_url)
    local raw_data,size = std.antd_recv(HTTP_REQUEST.id, clen)
    if not raw_data or size ~= clen then
        LOG_ERROR("Unable to read request data: received %d bytes expected %d bytes", size, clen)
        return 400, "Unable to read request data"
    end
    if is_url then
        local str = tostring(raw)
        local arr = explode(str, "&")
        LOG_INFO("encoded POST URI: %s", str)
        for i,v in ipairs(arr) do
            local assoc = explode(v,"=")
            if #assoc == 2 then
                REQUEST[assoc[1]] = untils.decodeURI(assoc[2])
            else
                REQUEST[assoc[1]] = ""
            end
        end
    else
        local key = ctype:gsub("^[^/]*", "")
        REQUEST[key] = raw_data
    end
    return 0
end

local decode_multi_part = function(ctype, clen)
    --[[
        local arr = explode(ctype, "=")
    if #arr ~= 2 then
        LOG_ERROR("Unable to parsed boundary for: %s", ctype)
        return 400, "Multipart Boundary not found"
    end
    local boundary = std.trim(arr[2]," ")
    local boundary_end = boundary.."--"
    LOG_INFO("Boundary found: %s", boundary)
    local line = nil
    repeat
        line = readline()
    until not line or line:find(boundary) or line == ""
    if not line or line == "" then
        LOG_ERROR("Cannot find first match for boundary %s", boundary)
        return 400, "Unable to decode data based on content boundary"
    end
    repeat
        line = readline()
    until not line or line:find("Content-Disposition:") or line == ""
    if not line or line == "" then
        LOG_ERROR("Content-Disposition meta data not fond")
        return 400, "Unable to query Content-Disposition from request"
    end
    line = line:gsub("Content-Disposition:",""):gsub("\r\n","")
    -- extract parameters from header
    arr = explode(line,";")
    local part_name, part_file = nil, nil
    for i,v in ipairs(arr) do
        LOG_INFO('Decoding: %s', v)
        local assoc = explode(v, "=")
        local key = std.trim(assoc[1])
        local val = assoc[1]
        if val then
            val = std.trim(val, " ")
            if key == "name" then
                LOG_INFO("Part name: %s", val)
                part_name = val
            end
            if key == "filename" then
                LOG_INFO("Part file: %s", val)
                part_file = val
            end
        end
    end
    -- TODO: to be continue
    ]]
    return 0
end
-- decode post data if any
local decode_request = function()
    LOG_INFO("Request method %s", REQUEST.method)
    if (not REQUEST.method)
        or (REQUEST.method ~= "POST"
            and REQUEST.method ~= "PUT"
            and REQUEST.method ~= "PATCH") then
            return 0
    end
    local ctype = HEADER['Content-Type']
    local clen = HEADER['Content-Length'] or -1
    if clen then
        clen = tonumber(clen)
    end
    if not ctype or clen == -1 then
        LOG_ERROR("Invalid content type %s or content length %d", ctype, clen)
        return 400, "Bad Request, missing content description"
    end
    if ctype == "application/x-www-form-urlencoded" then
        return decode_post_data(ctype, clen, true)
    elseif ctype == "multipart/form-data" then
        return decode_multi_part(ctype, clen)
    else
        return decode_post_data(ctype, clen, false)
    end
end

local code, error = decode_request()

if code ~= 0 then
    LOG_ERROR(error)
    std.error(code, error)
    return
end

-- OOP support
--require("OOP")
-- load sqlite helper
--require("sqlite")
-- enable extra mime

-- run the file


local m, s, p  = has_module(HTTP_REQUEST.request.RESOURCE_PATH)
if m then
    -- run the correct module
    if s then
		local r,e = loadscript(p)
		if r then r() else unknow(e) end
    else
        require(p)
    end
else
	unknow("Resource not found for request "..HTTP_REQUEST.request.RESOURCE_PATH)
end


--require('router')
