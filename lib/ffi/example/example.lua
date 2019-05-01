require("cif")
local path = "/Users/mrsang/Google Drive/ushare/cwp/ant-http/plugins/antd-lua-plugin/lib/ffi/example/libtest.so"
local lib = nil
local fn = nil
local rettype = nil
local argtype = nil
lib = FFI.load(path)
if lib then
    -- now try to lookup for the greet function
    echo("looking for greet")
    fn = FFI.lookup(lib,"greet")
    if fn then
        -- now the function found
        -- tried to called it
        rettype = FFI.atomic(FFI.type.VOID) -- void
        if rettype then
            argtype = {
                FFI.atomic(FFI.type.POINTER),
                FFI.atomic(FFI.type.DOUBLE),
                FFI.atomic(FFI.type.SINT64),
                FFI.atomic(FFI.type.SINT8)
            } -- pointer
            if(argtype) then
                -- call the function
                local r = FFI.call(rettype, argtype, fn, {"hello world", 0.987, -76, 65})
                if r then
                    echo("BIG SUCCESS")
                else
                    echo("HELL FAIL")
                end
            else
                echo("argtype not found")
            end
        else
            echo("return type not found")
        end
    else
        echo("unable to find greet")
    end
   FFI.unloadAll()
else
    echo("unable to load the lib")
end
echo("end the day")