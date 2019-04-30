local ffi = require("ffi")
local path = "/home/mrsang/Desktop/testffi/libtest.so"
local lib = nil
local fn = nil
local rettype = nil
local argtype = nil
if ffi then
    -- now ffi exist
    -- try to load the test library
    lib = ffi.dlopen(path)
    if lib then
        -- now try to lookup for the greet function
        echo("looking for greet")
        fn = ffi.dlsym(lib,"greet")
        if fn then
            -- now the function found
            -- tried to called it
            rettype = ffi.atomic_type(0) -- void
            if rettype then
                argtype = ffi.atomic_type(20) -- pointer
                if(argtype) then
                    -- call the function
                    local r = ffi.call(rettype, {argtype}, fn, {"hello world"})
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
       ffi.dlclose(lib)
    else
        echo("unable to load the lib")
    end
else
    echo("cannot find ffi")
end
echo("end the day")