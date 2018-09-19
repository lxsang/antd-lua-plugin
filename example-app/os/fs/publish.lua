auth_or_die("User unauthorized. Please login")

local rq = (JSON.decodeString(REQUEST.query.json))

if rq ~= nil then
    local p = nil
    if rq.publish then
        p = require("fs.vfs").ospath(rq.path)
    else
        p = require("fs.shared").ospath(rq.path)
    end
    local user = SESSION.iotos_user
	local uid = unix.uid(user)
    local st = unix.file_stat(p)
    if uid.id ~= st.uid then die("Only the owner can share or unshare this file") end
    local entry = { sid = std.sha1(p), user = SESSION.iotos_user, path = p, uid = uid.id }
    local db =  require("db.model").get("sysdb", "shared", entry)
    if db == nil then die("Cannot get system database") end
    local cond = nil
    if rq.publish then
        cond = { exp = { ["="] = { path = p } } }
        local data = db:find(cond)
        if data == nil or data[0] == nil then
            -- insert entry
            db:insert(entry)
        end
    else
        cond = { ["="] = { sid = rq.path } }
        db:delete(cond)
    end
    db:close()
    result(entry.sid)
    
else
	fail("Uknown request")
end