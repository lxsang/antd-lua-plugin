auth_or_die("User unauthorized. Please login")
local rq = (JSON.decodeString(REQUEST.query.json))
if(rq ~= nil and rq.table ~= nil) then
    local model = require("db.model").get(SESSION.iotos_user, rq.table, nil)
    local ret
    if model == nil then
        fail("Cannot get table metadata:"..rq.table)
    else
        if(rq.id == nil ) then
            if(rq.cond) then
                ret = model:delete(rq.cond)
                model:close()
            else
                model:close()
                return fail("Unknow element to delete")
            end
        else
            ret = model:deleteByID(rq.id)
            model:close()
        end
        if ret then
            result(ret)
        else
            fail("Querry error or database is locked")
        end
    end
else
    fail("Unknown database request")
end