#include "lua-api.h"
// add a length field, and
void lua_new_byte_array(lua_State*L, int n)
{
	 size_t nbytes = sizeof(byte_array_t) + (n-1)*sizeof(unsigned char);
	 byte_array_t *a = (byte_array_t *)lua_newuserdata(L, nbytes);
 	luaL_getmetatable(L, BYTEARRAY);
 	lua_setmetatable(L, -2);
	 a->size = n;
}
static int l_new_barray (lua_State *L) {
	 int n = luaL_checknumber(L, 1);
	 lua_new_byte_array(L,n);
	 return 1;  /* new userdatum is already on the stack */
}
byte_array_t *l_check_barray (lua_State *L,int idx) {
	void *ud = luaL_checkudata(L, idx, BYTEARRAY);
	luaL_argcheck(L, ud != NULL, idx, "`byte array' expected");
	return (byte_array_t *)ud;
}

static unsigned char *get_bel(lua_State *L) {
	byte_array_t *a = l_check_barray(L,1);
	int index = luaL_checknumber(L, 2);
	luaL_argcheck(L, 1 <= index && index <= a->size, 2,
                       "index out of range");
    
	/* return element address */
	return &a->data[index - 1];
    }
	
static int l_set_barray (lua_State *L) {
	unsigned char value = luaL_checknumber(L, 3); 
	*get_bel(L) = value;
    return 0;
}

static int l_get_barray (lua_State *L) {
	lua_pushnumber(L, *get_bel(L));
	return 1;
}

static int l_get_barray_size (lua_State *L) {
	byte_array_t *a = l_check_barray(L,1);
	lua_pushnumber(L, a->size);
	return 1;
}

static int l_barray_to_string (lua_State *L) {
	 byte_array_t *a = l_check_barray(L,1);
	 char * d = (char*) malloc(a->size+1);
	 memcpy(d, a->data, a->size);
	 d[a->size] = '\0';
	 lua_pushstring(L, d);
	 if(d)
		 free(d);
	 return 1;
}

static int l_barray_write(lua_State* L)
{
	byte_array_t *a = l_check_barray(L,1);
	const char* f = luaL_checkstring(L,2);
	FILE *fp;
	fp = fopen(f,"wb");
	
	if(!fp)
		lua_pushboolean(L,0);
	else
	{
		fwrite(a->data ,1, a->size ,fp);
		lua_pushboolean(L,1);
		fclose(fp);
	}
	return 1;
}

static const struct luaL_Reg barraylib[] = {
	{"new", l_new_barray},
	{"set", l_set_barray},
	{"get", l_get_barray},
	{"size",l_get_barray_size},
	{"__tostring", l_barray_to_string},
	{"write", l_barray_write},
	{NULL, NULL}
};
	


// ARRAY
void lua_new_array(lua_State*L, int n)
{
	size_t nbytes = sizeof(array_t) + (n-1)*sizeof(double);
 	array_t *a = (array_t *)lua_newuserdata(L, nbytes);
 	luaL_getmetatable(L, ARRAY);
 	lua_setmetatable(L, -2);
 	a->size = n;
}
static int l_new_array (lua_State *L) {
	 int n = luaL_checknumber(L, 1);
	 lua_new_array(L,n);
	 return 1;  /* new userdatum is already on the stack */
}

array_t *l_check_array (lua_State *L, int idx) {
	void *ud = luaL_checkudata(L, idx, ARRAY);
	luaL_argcheck(L, ud != NULL, idx, "`array' expected");
	return (array_t *)ud;
}

static double *get_el(lua_State *L) {
	array_t *a = l_check_array(L,1);
	int index = luaL_checknumber(L, 2);
	luaL_argcheck(L, 1 <= index && index <= a->size, 2,
                       "index out of range");
    
	/* return element address */
	return &a->data[index - 1];
    }
	
static int l_set_array (lua_State *L) {
	double value = luaL_checknumber(L, 3); 
	*get_el(L) = value;
    return 0;
}

static int l_get_array (lua_State *L) {
	lua_pushnumber(L, *get_el(L));
	return 1;
}

static int l_get_array_size (lua_State *L) {
	array_t *a = l_check_array(L,1);
	lua_pushnumber(L, a->size);
	return 1;
}
static int l_array_to_string (lua_State *L) {
	 array_t *a = l_check_array(L,1);
	 lua_pushfstring(L, "number array(%d)", a->size);
	 return 1;
}
	
static const struct luaL_Reg arraylib [] = {
	{"new", l_new_array},
	{"set", l_set_array},
	{"get", l_get_array},
	{"size",l_get_array_size},
	{"__tostring", l_array_to_string},
	{NULL, NULL}
};
int luaopen_array (lua_State *L) {
	  luaL_newmetatable(L, ARRAY);
	  luaL_newlib(L, arraylib);
	  lua_pushstring(L, "__index");
	  lua_pushstring(L, "get");
	  lua_gettable(L, 2);  /* get array.get */
	  lua_settable(L, 1);  /* metatable.__index = array.get */
    
	   lua_pushstring(L, "__newindex");
	   lua_pushstring(L, "set");
	   lua_gettable(L, 2); /* get array.set */
	   lua_settable(L, 1); /* metatable.__newindex = array.set */
	  
	  //lua_pushstring(L, "__index");
	  //lua_pushvalue(L, -2);  /* pushes the metatable */
	  //lua_settable(L, -3);  /* metatable.__index = metatable */
	 // luaL_setfuncs(L, arraylib_m, 0);
	 // luaL_openlib(L, NULL, arraylib_m, 0);
	 // luaL_newlib(L, arraylib_f);
      return 1;
 }
int luaopen_barray (lua_State *L) {
	luaL_newmetatable(L, BYTEARRAY);
  	luaL_newlib(L, barraylib);
  	lua_pushstring(L, "__index");
 	lua_pushstring(L, "get");
  	lua_gettable(L, 2);  /* get array.get */
  	lua_settable(L, 1);  /* metatable.__index = array.get */
  
	lua_pushstring(L, "__newindex");
	lua_pushstring(L, "set");
	lua_gettable(L, 2); /* get array.set */
	lua_settable(L, 1);
	//lua_pushstring(L, "__index");
	//lua_pushvalue(L, -2);  /* pushes the metatable */
	//lua_settable(L, -3);  /* metatable.__index = metatable */
	//luaL_setfuncs(L, barraylib_m, 0);
	//luaL_openlib(L, NULL, barraylib_m, 0);
	//luaL_newlib(L,  barraylib_f);
    return 1;
}