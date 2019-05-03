local path = "/home/blackjack/workspace/ant-http/plugins/antd-lua-plugin/lib/ffi/example/libtest.so"
require("cif")
local lib = nil
local fn = nil
local rettype = nil
local argtype = nil
lib = FFI.load(path)
if lib then
    -- now try to lookup for the greet function
    fn = FFI.lookup(lib,"greet")
    if fn then
        -- now the function
        -- tried to called it
        rettype = FFI.atomic(FFI.type.UINT8) -- voidn
        argtype = {
            FFI.atomic(FFI.type.POINTER),
            FFI.atomic(FFI.type.DOUBLE),
            FFI.atomic(FFI.type.SINT64),
            FFI.atomic(FFI.type.SINT8)
        } -- pointer
        -- call the function
        local r = FFI.call(rettype, argtype, fn, {"hello world", 0.987, -76, 65})
        if r then
            echo(r)
        else
            echo("HELL FAIL")
        end
    end
    fn = FFI.lookup(lib, "test_struct")
    if fn then
        local struct1 = FFI.struct({
            FFI.atomic(FFI.type.UINT8), -- char
            FFI.atomic(FFI.type.SINT32), -- char
            FFI.atomic(FFI.type.SINT16), -- char
            FFI.atomic(FFI.type.UINT8), -- char
        })
        local struct = FFI.struct({
            FFI.atomic(FFI.type.UINT8), -- char
            struct1,
            FFI.atomic(FFI.type.SINT32), -- int 
            FFI.atomic(FFI.type.UINT8), -- char
        })
        rettype = struct --FFI.atomic(FFI.type.DOUBLE)
        echo("call with struct")
       local r  = FFI.call(rettype,{struct},fn,{{65, {66, 2048, -97,67},-1024, 68}})
        if r then echo(JSON.encode(r)) end
        --echo(JSON.encode(FFI.meta(struct1)))
    end
    
    fn = FFI.lookup(lib, "test_string")
    if(fn) then
        local buff = FFI.new(256)
        FFI.call(FFI.atomic(FFI.type.VOID),{FFI.atomic(FFI.type.POINTER), FFI.atomic(FFI.type.POINTER)}, fn, {buff,"Hello world"})
        echo(FFI.string(buff))
        
    end
    
   FFI.unloadAll()
end
echo("end the day")