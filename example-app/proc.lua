std.html()
local pid = unix.fork()
if pid == -1 then
    echo("Fail to fork")
elseif pid > 0 then
    for i = 1,10 do
        print("parent "..i)
    end
    unix.waitpid(pid)
    print("Child finish")
else
    for i = 1,20 do
        print("child "..i)
    end
end

print "reach for both"