std = modules.std()
bytes = modules.bytes()
array = modules.array()
function std.html()
	std._html(REQUEST.id)
end
function std.text()
	std._text(REQUEST.id)
end
function std.status(code, msg)
	std._status(REQUEST.id, code, msg)
end
function std.custom_header(k,v)
	--print(k..":"..v)
	std.t(k..": "..v)
end
function std.header_flush()
	std.t("")
end
--_redirect
function std.redirect(s)
	std._redirect(REQUEST.id,s)
end
function std.json()
	std._json(REQUEST.id)
end
function std.jpeg()
	std._jpeg(REQUEST.id)
end
function std.header(s)
	std._header(REQUEST.id,s)
end
function std.octstream(s)
	std._octstream(REQUEST.id,s)
end
function std.textstream()
	std._textstream(REQUEST.id)
end
function std.ti(v)
	std._ti(REQUEST.id,v)
end
function std.t(s)
	std._t(REQUEST.id,s)
end
function std.f(v)
	std._f(REQUEST.id,v)
end
function std.fb(v)
	std._fb(REQUEST.id,v)
end
function std.setCookie(t,v,p)
	p = p or ""
	std._setCookie(REQUEST.id,t,v,p)
end
function std.cjson(v, p)
	
	std.setCookie("application/json; charset=utf-8",v)
end
function std.chtml(v)
	std.setCookie("text/html; charset=utf-8",v)
end
function std.ctext(v)
	std.setCookie("text/plain; charset=utf-8",v)
end
--_upload
--_route
function std.unknow(s)
	std._unknow(REQUEST.id,s)
end

function std.readOnly(t) -- bugging
    local proxy = {}
    local mt = {       -- create metatable
		__index = t,
        __newindex = function (t,k,v)
          error("attempt to update a read-only table", 2)
        end
    }
    setmetatable(proxy, mt)
    return proxy
 end
    

-- web socket
std.ws = {}
function std.ws.header()
	local h = std.ws_header(REQUEST.id)
	if(h) then
		return h --std.readOnly(h)
	else
		return nil
	end
end

function std.ws.read(h)
	return std.ws_read(REQUEST.id,h)
end
function std.ws.swrite(s)
	std.ws_t(REQUEST.id,s)
end
function std.ws.fwrite(s)
	std.ws_f(REQUEST.id,s)
end
function std.ws.write_bytes(arr)
	std.ws_b(REQUEST.id,arr)
end
function std.ws.enable()
	return REQUEST.query ~= nil and REQUEST.query["__web_socket__"] == "1"
end
function std.ws.close(code)
	std.ws_close(REQUEST.id,code)
end
function std.basename(str)
	local name = string.gsub(std.trim(str,"/"), "(.*/)(.*)", "%2")
	return name
end
function std.is_file(f)
	return  std.is_dir(f) == false
end
std.ws.TEXT = 1
std.ws.BIN = 2
std.ws.CLOSE = 8
