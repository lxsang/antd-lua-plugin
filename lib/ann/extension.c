/*
    This file is part of lfann.

    lfann is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    lfann is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with lfann.  If not, see <http://www.gnu.org/licenses/>.

    Copyright (C) 2009 - 2013 Lucas Hermann Negri
*/

static int lfann_data_create_from_callback(lua_State* L)
{
    unsigned int num_data, num_input, num_output;
    unsigned int i, j;
    struct fann_train_data* data;
    int top, n_params;

    luaL_checktype(L, 1, LUA_TNUMBER);
    luaL_checktype(L, 2, LUA_TNUMBER);
    luaL_checktype(L, 3, LUA_TNUMBER);
    luaL_checktype(L, 4, LUA_TFUNCTION);
    
    // Userdata?
    top = lua_gettop(L);
    if(top == 4) {lua_pushnil(L); ++top;} 
    
    num_data   = lua_tointeger(L, 1);
    num_input  = lua_tointeger(L, 2);
    num_output = lua_tointeger(L, 3);
    
    data = fann_create_train(num_data, num_input, num_output);
    
    n_params = num_input + num_output;
    
    /* Get all the training data from the callback */
    for(i = 0; i < num_data; ++i)
    {
        /* Call the function */
        lua_pushvalue(L, 4);  /* function */
        lua_pushvalue(L, 5); /* ud */
        lua_pushinteger(L, i + 1);
        lua_pushinteger(L, num_input);
        lua_pushinteger(L, num_output);
        lua_call(L, 4, n_params);
        
        /* Get the input */
        for(j = 1; j <= num_input; ++j)
            data->input[i][j - 1] = lua_tonumber(L, top + j);
            
        /* Get the output */
        for(j = 1; j <= num_output; ++j)
            data->output[i][j - 1] = lua_tonumber(L, top + j + num_input);
            
        /* clear the stack */
        lua_settop(L, top);
    }
    
    priv_push_data(L, data);
    
    return 1;
}

static int lfann_data_get_row(lua_State* L)
{
    Object* obj;
    struct fann_train_data* data;
    unsigned int i, n_data;

    luaL_checktype(L, 1, LUA_TUSERDATA);
    luaL_checktype(L, 2, LUA_TNUMBER);
    obj = lua_touserdata(L, 1);
    
    data = obj->pointer;
    n_data = lua_tonumber(L, 2) - 1;
    if(n_data < 0 || n_data > data->num_data) luaL_error(L, "Invalid index\n"); 
    
    lua_newtable(L);
    
    /* set the input */
    for(i = 0; i < data->num_input; ++i)
    {
        lua_pushinteger(L, i + 1); 
        lua_pushnumber(L,  data->input[n_data][i]);
        lua_rawset(L, -3);
    }
    
    /* set the output */
    for(i = 0; i < data->num_output; ++i)
    {
        lua_pushinteger(L, i + data->num_input + 1); 
        lua_pushnumber(L,  data->output[n_data][i]);
        lua_rawset(L, -3);
    }
    
    return 1;
}

#define SMALL 0.000001

static void priv_data_get_bounds(fann_type** array, size_t rows, size_t cols,
    fann_type* omin, fann_type* omax)
{
    fann_type rmin = array[0][0], rmax = array[0][0], aux;
    size_t i, j;

    if(rows < 1 || cols < 1)
    {
        *omin = 0;
        *omax = 0;
        return;
    }
    
    for(i = 0; i < rows; ++i)
        for(j = 0; j < cols; ++j)
        {
            aux = array[i][j];
            if(aux < rmin) rmin = aux;
            if(aux > rmax) rmax = aux;
        }
    
    *omin = rmin;
    *omax = rmax;
}

static void priv_data_scale_array(fann_type** array, size_t rows, size_t cols,
    fann_type rmin, fann_type rmax, fann_type dmin, fann_type dmax)
{
    fann_type rscale;
    fann_type dscale;
    fann_type mean;

    size_t i, j;
    
    rscale = rmax - rmin;
    dscale = dmax - dmin;
    
    /* If the desired span is too close or the contents are too close,
     * just set the values to the mean */
    if(rscale < SMALL || dscale < SMALL)
    {
        mean = (dmax + dmin) * 2;
        
        for(i = 0; i < rows; ++i)
            for(j = 0; j < cols; ++j)
                array[i][j] = mean;
    }
    else
    {   
        /* Second pass: do the scaling */
        for(i = 0; i < rows; ++i)
            for(j = 0; j < cols; ++j)
                array[i][j] = ( (array[i][j] - rmin) / rscale ) * dscale + dmin;
    }
}

static int lfann_data_scale_input(lua_State* L)
{
    Object* obj = lua_touserdata(L, 1);
    struct fann_train_data* data = obj->pointer;
    
    fann_type rmin, rmax;

    luaL_checktype(L, 1, LUA_TUSERDATA);
    luaL_checktype(L, 2, LUA_TNUMBER);
    luaL_checktype(L, 3, LUA_TNUMBER);
    
    obj = lua_touserdata(L, 1);
    data = obj->pointer;
    
    priv_data_get_bounds(data->input, data->num_data, data->num_input,
        &rmin, &rmax);
    
    priv_data_scale_array(data->input, data->num_data, data->num_input,
        rmin, rmax, lua_tonumber(L, 2), lua_tonumber(L, 3));
    
    return 0;
}

static int lfann_data_scale_output(lua_State* L)
{
    Object* obj;
    struct fann_train_data* data;
    fann_type rmin, rmax;

    luaL_checktype(L, 1, LUA_TUSERDATA);
    luaL_checktype(L, 2, LUA_TNUMBER);
    luaL_checktype(L, 3, LUA_TNUMBER);
    
    obj = lua_touserdata(L, 1);
    data = obj->pointer;
    
    priv_data_get_bounds(data->output, data->num_data, data->num_output,
        &rmin, &rmax);
    
    priv_data_scale_array(data->output, data->num_data, data->num_output,
        rmin, rmax, lua_tonumber(L, 2), lua_tonumber(L, 3));
    
    return 0;
}

static int lfann_data_scale(lua_State* L)
{
    Object* obj;
    struct fann_train_data* data;
    fann_type rmin_in, rmax_in;
    fann_type rmin_out, rmax_out;

    luaL_checktype(L, 1, LUA_TUSERDATA);
    luaL_checktype(L, 2, LUA_TNUMBER);
    luaL_checktype(L, 3, LUA_TNUMBER);
    
    obj = lua_touserdata(L, 1);
    data = obj->pointer;
    
    priv_data_get_bounds(data->input, data->num_data, data->num_input,
        &rmin_in, &rmax_in);
    
    priv_data_get_bounds(data->output, data->num_data, data->num_output,
        &rmin_out, &rmax_out);
    
    /* Scale them with the unified bounds */
    if(rmin_out < rmin_in) rmin_in = rmin_out;
    if(rmax_out > rmax_in) rmax_in = rmax_out;
    
    priv_data_scale_array(data->input, data->num_data, data->num_input,
        rmin_in, rmax_in, lua_tonumber(L, 2), lua_tonumber(L, 3));
        
    priv_data_scale_array(data->output, data->num_data, data->num_output,
        rmin_in, rmax_in, lua_tonumber(L, 2), lua_tonumber(L, 3));
    
    return 0;
}

static int lfann_data_get_bounds_input(lua_State* L)
{
    Object* obj;
    struct fann_train_data* data;
    fann_type rmin, rmax;

    luaL_checktype(L, 1, LUA_TUSERDATA);
    
    obj = lua_touserdata(L, 1);
    data = obj->pointer;
    
    priv_data_get_bounds(data->input, data->num_data, data->num_input,
        &rmin, &rmax);
    
    lua_pushnumber(L, rmin);
    lua_pushnumber(L, rmax);
    
    return 2;
}

static int lfann_data_get_bounds_output(lua_State* L)
{
    Object* obj;
    struct fann_train_data* data;
    fann_type rmin, rmax;

    luaL_checktype(L, 1, LUA_TUSERDATA);
    
    obj = lua_touserdata(L, 1);
    data = obj->pointer;
    
    priv_data_get_bounds(data->output, data->num_data, data->num_output,
        &rmin, &rmax);
    
    lua_pushnumber(L, rmin);
    lua_pushnumber(L, rmax);
    
    return 2;
}

static int lfann_data_get_bounds(lua_State* L)
{
    Object* obj;
    struct fann_train_data* data;
    fann_type rmin_in, rmax_in;
    fann_type rmin_out, rmax_out;

    luaL_checktype(L, 1, LUA_TUSERDATA);
    
    obj = lua_touserdata(L, 1);
    data = obj->pointer;
    
    priv_data_get_bounds(data->input, data->num_data, data->num_input,
        &rmin_in, &rmax_in);
    
    priv_data_get_bounds(data->output, data->num_data, data->num_output,
        &rmin_out, &rmax_out);
    
    /* Scale them with the unified bounds */
    if(rmin_out < rmin_in) rmin_in = rmin_out;
    if(rmax_out > rmax_in) rmax_in = rmax_out;
    
    lua_pushnumber(L, rmin_in);
    lua_pushnumber(L, rmax_in);
    
    return 2;
}
