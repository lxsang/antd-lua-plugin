auth_or_die("User unauthorized. Please login")
local rq = (JSON.decodeString(REQUEST.query.json))
if(rq ~= nil and rq.table ~= nil) then
    local model = require("db.model").get(SESSION.iotos_user,rq.table, nil)
    local ret
    if model == nil then
        fail("Cannot get table metadata:"..rq.table)
    else
        if(rq.cond == nil ) then
            model:close()
            return fail("Unknow condition")
        else
            ret = model:find(rq.cond)
        end
        model:close()
        result(ret)
    end
else
    fail("Unknown database request")
end