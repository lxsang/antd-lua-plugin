local path = "/home/blackjack/workspace/ant-http/plugins/antd-lua-plugin/lib/ffi/example/libtest.so"
local path = "/Users/mrsang/Google Drive/ushare/cwp/ant-http/plugins/antd-lua-plugin/lib/ffi/example/libtest.so"
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
        echo(tostring(FFI.bytearray(buff,5)[1]))
        FFI.byteAtPut(buff,0, 11)
        echo(tostring(FFI.byteAt(buff,0)))
        
    end
    local size = 1024
    local struct_ptr = FFI.new(12)
    local buff = FFI.new(5)
    FFI.byteAtPut(buff,0,64)
    FFI.byteAtPut(buff,1,65)
    FFI.byteAtPut(buff,2,66)
    FFI.byteAtPut(buff,3,67)
    FFI.byteAtPut(buff,4,0)
    FFI.byteAtPut(struct_ptr, 0, 10)
    FFI.byteAtPut(struct_ptr, 1, 100)
    FFI.byteAtPut(struct_ptr, 2, 0) -- pad
    FFI.byteAtPut(struct_ptr, 3, 0) -- pad
    FFI.byteAtPut(struct_ptr, 4, size & 0xFF)
    FFI.byteAtPut(struct_ptr, 5, (size >> 8) & 0xFF)
    FFI.byteAtPut(struct_ptr, 6, (size >> 16) & 0xFF)
    FFI.byteAtPut(struct_ptr, 7, (size >> 24) & 0xFF)
    FFI.atPutPtr(struct_ptr, 8, buff)
    
    fn = FFI.lookup(lib, "test_struct_ptr")
    if(fn) then
        echo("calling test_struct_ptr")
        FFI.call(FFI.atomic(FFI.type.SINT),{FFI.atomic(FFI.type.POINTER)}, fn, {struct_ptr})
    end
    
   FFI.unloadAll()
end
echo("end the day")