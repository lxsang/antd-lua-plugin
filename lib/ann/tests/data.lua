#! /usr/bin/env lua

require("lfann")

for i = 1, 1000 do
    local data = fann.Data.read_from_file("train.data")
    assert(data:length() == 4)
    local data2 = data:subset(0, data:length() - 1)
    assert(data2:length() == 3)
    local data3 = data:merge(data2)
    assert(data3:length() == data:length() + data2:length())
    assert(data:num_input() == 2)
    assert(data:num_output() == 1)
    assert(data2:num_input() == 2)
    assert(data2:num_output() == 1)
    assert(data3:num_input() == 2)
    assert(data3:num_output() == 1)

    local row = data:get_row(1)
    assert(row[1] == 1 and row[2] == 1 and row[3] == -1)
    row = data:get_row(2)
    assert(row[1] == 1 and row[2] == 0 and row[3] == 1)
    row = data:get_row(3)
    assert(row[1] == 0 and row[2] == 1 and row[3] == 1)
    row = data:get_row(4)
    assert(row[1] == 0 and row[2] == 0 and row[3] == -1)

    local n_rows, n_inp, n_out = 50, 10, 20
    local data = fann.Data.create_from_callback(n_rows, n_inp, n_out,
        function(ud, line, n_inp, n_out)
            local out = {}
            
            for i = 1, n_inp do
                table.insert(out, line + i)
            end
            
            for i = 1, n_out do
                table.insert(out, line * 2 - i)
            end
            
            return unpack(out)
        end
    )

    for r = 1, n_rows do
        local row = data:get_row(r)
        
        for i = 1, n_inp do
            assert(row[i] == r + i)
        end
            
        for i = 1, n_out do
            assert(row[i + n_inp] == r * 2 - i)
        end
    end
    
    collectgarbage("collect")
end

print("End.")

if arg[1] == "mem" then
	io.read("*n")
end
