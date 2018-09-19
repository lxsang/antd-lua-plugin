-- require the utils library to work
--require("utils")
-- require("std")
local wurl = require("wurl")

local web = {}

web.undestand = function(proto)
	if proto == "http" or proto == "https" then 
		return true 
	else 
		return false 
	end
end

web.get = function(url)
	local obj = utils.url_parser(url)
	if web.undestand(obj.protocol) then
		return wurl._get(obj.hostname,obj.port, obj.query)
	else
		return nil,"Protocol is unsupported: "..obj.protocol
	end
end


web.post = function(url,data)
	local obj = utils.url_parser(url)
	if web.undestand(obj.protocol) then
		if type(data) == "string" then
			return wurl._post(obj.hostname,
					obj.port,
					obj.query,
					"application/x-www-form-urlencoded",data)
		else
			return wurl._post(obj.hostname,
					obj.port,
					obj.query,
					data.contentType,data.value)
		end
	else
		return nil,"Protocol is unsupported: "..obj.protocol
	end
end

web.download = function(url,to)
	local obj = utils.url_parser(url)
	if web.undestand(obj.protocol) then
		local file
		if std.is_dir(to) then
			-- need to find file name here
			local pattern = "^[^%?]*/([%w.]*)%??.*$"
			local filename = string.gsub(obj.query,pattern,"%1")
			if filename == "" then filename = "index" end
			file = to.."/"..filename
		else
			file = to
		end
		local obj = utils.url_parser(url)
		return wurl._download(obj.hostname,obj.port,obj.query,file)
	else
		return false,"Protocol is unsupported: "..obj.protocol
	end
end

web.upload = function(url,name,file)
	local obj = utils.url_parser(url)
	if web.undestand(obj.protocol) then
		return wurl._upload(obj.hostname,obj.port,obj.query,name,file)
	else
		return nil,"Protocol is unsupported: "..obj.protocol
	end
end

return web