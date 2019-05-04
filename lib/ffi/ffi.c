/*
	This lib use libffi
	so libffi should be installed in the system
*/

#include "../lualib.h"
#include "../../lua-api.h"
#include "utils.h"


#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
// for library access
#include <dlfcn.h>
// ffi
#include <ffi.h>

#define MAX_FN_ARGC 32

// define atomic type
typedef enum ffi_atomic_t {
	L_FFI_TYPE_VOID,
	L_FFI_TYPE_UINT8,
	L_FFI_TYPE_SINT8,
	L_FFI_TYPE_UINT16,
	L_FFI_TYPE_SINT16,
	L_FFI_TYPE_UINT32,
	L_FFI_TYPE_SINT32,
	L_FFI_TYPE_UINT64,
	L_FFI_TYPE_SINT64,
	L_FFI_TYPE_FLOAT,
	L_FFI_TYPE_DOUBLE,
	L_FFI_TYPE_UCHAR,
	L_FFI_TYPE_SCHAR,
	L_FFI_TYPE_USHORT,
	L_FFI_TYPE_SSHORT,
	L_FFI_TYPE_UINT,
	L_FFI_TYPE_SINT,
	L_FFI_TYPE_ULONG,
	L_FFI_TYPE_SLONG,
	L_FFI_TYPE_LONGDOUBLE,
	L_FFI_TYPE_POINTER
};

static const ffi_type* ffi_atomic_type_ptrs[] = 
{
	&ffi_type_void,
	&ffi_type_uint8,
	&ffi_type_sint8,
	&ffi_type_uint16,
	&ffi_type_sint16,
	&ffi_type_uint32,
	&ffi_type_sint32,
	&ffi_type_uint64,
	&ffi_type_sint64,
	&ffi_type_float,
	&ffi_type_double,
	&ffi_type_uchar,
	&ffi_type_schar,
	&ffi_type_ushort,
	&ffi_type_sshort,
	&ffi_type_uint,
	&ffi_type_sint,
	&ffi_type_ulong,
	&ffi_type_slong,
	&ffi_type_longdouble,
	&ffi_type_pointer,
	NULL
};

static int l_dlopen(lua_State* L)
{
    const char* path = luaL_checkstring(L,1);
    void* lib_handle = dlopen(path, RTLD_LAZY);
	if(!lib_handle)
	{
		lua_pushnil(L);
		return 1;
	}
	// push the handle pointer to lua
    lua_pushlightuserdata(L, lib_handle);
    return 1;
}

static int l_dlclose(lua_State* L)
{
	void* handle = lua_touserdata(L,1);
	if(!handle)
	{
		lua_pushboolean(L,0);
		return 1;
	}
	dlclose(handle);
	lua_pushboolean(L,1);
	return 1;
}

static int l_dlsym(lua_State* L)
{
	char* error;
	void* handle = lua_touserdata(L,1);
	const char* fname = luaL_checkstring(L,2);
	void* fn = dlsym(handle, fname);
	if ((error = dlerror()) != NULL) 
	{
		lua_pushnil(L);
		return 1;
	}
	lua_pushlightuserdata(L, fn);
	return 1;
}

static int l_ffi_prepare(lua_State* L, ffi_type** argvtype, int idx)
{
	// argument count not more than 64
	int argc = 0;
	// now loop through the args type table, then fill the argvtype

	lua_pushvalue(L,idx);
	// stack now contains: -1 => table
	lua_pushnil(L);
	// stack now contains: -1 => nil, -2 => table
	while(lua_next(L, -2))
	{
		// stack now contains: -1 => value; -2 key; -3 table
		argvtype[argc] = lua_touserdata(L, -1);
		argc++;
		// pop the value, leaving the original key
		lua_pop(L,1);
		// stack now contains: -1 key; -2 table
	}
	// lua_next return 0, it popout the key at -1, leaving the table
	// so, popout the table
	argvtype[argc] = NULL;
	lua_pop(L,1);
	return argc;
}

void parser_value(lua_State* L, int idx, ffi_type* ffitype, void * data)
{
	intptr_t address = (intptr_t) data;
	int offset = 0;
	int pad = 0;
	int i = 0;
	switch(ffitype->type)
	{
		case FFI_TYPE_POINTER:
			if(lua_isstring(L,idx))
				*((void**)data) = (void*)lua_tostring(L, idx);
			else
				*((void**)data) = (void*)lua_touserdata(L, idx);
			return;

		case FFI_TYPE_UINT8:
			*((uint8_t*)data) = (uint8_t)lua_tonumber(L,idx);
			return;

		case FFI_TYPE_SINT8:
			*((int8_t*)data) = (int8_t)lua_tonumber(L,idx);
			return;

		case FFI_TYPE_UINT16:
			*((uint16_t*)data) = (uint16_t)lua_tonumber(L,idx);
			return;

		case FFI_TYPE_SINT16:
			*((int16_t*)data) = (int16_t)lua_tonumber(L,idx);
			return;

		case FFI_TYPE_UINT32:
			*((uint32_t*)data) = (uint32_t)lua_tonumber(L,idx);
			return;

		case FFI_TYPE_SINT32:
			*((int32_t*)data) = (int32_t)lua_tonumber(L,idx);
			return;

		case FFI_TYPE_UINT64:
			*((uint64_t*)data) = (uint64_t)lua_tonumber(L,idx);
			return;

		case FFI_TYPE_SINT64:
			*((int64_t*)data) = (int64_t)lua_tonumber(L,idx);
			return; 

		/*case FFI_TYPE_LONGDOUBLE:
			//This is bug in lua
			*((long double*)data) = (long double)lua_tonumber(L,idx);
			return;*/

		case FFI_TYPE_FLOAT:
		case FFI_TYPE_DOUBLE:
			*((float*)data) = (float)lua_tonumber(L,idx);
			return;

		
		case FFI_TYPE_STRUCT:
			// loop through the table
			lua_pushvalue(L,idx);
			// stack now contains: -1 => table
			lua_pushnil(L);
			// stack now contains: -1 => nil, -2 => table
			while(lua_next(L, -2))
			{
				// stack now contains: -1 => value; -2 key; -3 table
				parser_value(L, -1, ffitype->elements[i],data+offset);
				// pop the value, leaving the original key
				lua_pop(L,1);
				// stack now contains: -1 key; -2 table
				// recalculate offs
				address += ffitype->elements[i]->size;
				if(ffitype->elements[i+1])
				{
					pad = address % ffitype->elements[i+1]->alignment;
					if( pad != 0)
					{
						pad =  ffitype->elements[i+1]->alignment - pad;
					}
					
					address +=  pad;
					offset += ffitype->elements[i]->size + pad;
				}
				i++;
			}
			// lua_next return 0, it popout the key at -1, leaving the table
			// so, popout the table
			lua_pop(L,1);
			return;
			
		default: return;
	}
}

static void parser_arguments(lua_State* L, int idx, void** argv, ffi_type** argvtype)
{
	// loop through table
	lua_pushvalue(L,idx);
	// stack now contains: -1 => table
	lua_pushnil(L);
	int i = 0;
	// stack now contains: -1 => nil, -2 => table
	while(lua_next(L, -2))
	{
		// stack now contains: -1 => value; -2 key; -3 table
		argv[i] = (void*)malloc(argvtype[i]->size);
		parser_value(L, -1, argvtype[i],argv[i]);
		i++;
		// pop the value, leaving the original key
		lua_pop(L,1);
		// stack now contains: -1 key; -2 table
	}
	// lua_next return 0, it popout the key at -1, leaving the table
	// so, popout the table
	lua_pop(L,1);
}


static void ffi_post_call(lua_State* L, void* ret, ffi_type* rettype)
{
	if(!ret)
	{
		lua_pushnil(L);
		return;
	}
	int i = 0;
	intptr_t address = (intptr_t) ret;
	int offset = 0;
	int pad = 0;
	switch (rettype->type)
	{
		case FFI_TYPE_POINTER:
			lua_pushlightuserdata(L,ret);
			break;

		case FFI_TYPE_UINT8:
			lua_pushnumber(L, (lua_Number)(*((uint8_t*)ret)));
			break;
		case FFI_TYPE_SINT8:
			lua_pushnumber(L, (lua_Number)(*((int8_t*)ret)));
			break;
		case FFI_TYPE_UINT16:
			lua_pushnumber(L, (lua_Number)(*((uint16_t*)ret)));
			break;
		case FFI_TYPE_SINT16:
			lua_pushnumber(L, (lua_Number)(*((int16_t*)ret)));
			break;
		case FFI_TYPE_UINT32:
			lua_pushnumber(L, (lua_Number)(*((uint32_t*)ret)));
			break;
		case FFI_TYPE_SINT32:
			lua_pushnumber(L, (lua_Number)(*((int32_t*)ret)));
			break;
		case FFI_TYPE_UINT64:
			lua_pushnumber(L, (lua_Number)(*((uint64_t*)ret)));
			break;
		case FFI_TYPE_SINT64:
			lua_pushnumber(L, (lua_Number)(*((int64_t*)ret)));
			break;
		//case FFI_TYPE_LONGDOUBLE:
		case FFI_TYPE_FLOAT:
		case FFI_TYPE_DOUBLE:
			lua_pushnumber(L, *((double*)ret));
			break;
		case FFI_TYPE_STRUCT:
			lua_newtable(L);
			for ( i = 0; rettype->elements[i] != NULL; i++)
			{
				lua_pushnumber(L,i);
				ffi_post_call(L, ret + offset,rettype->elements[i]);
				lua_settable(L, -3);
				address += rettype->elements[i]->size;
				if(rettype->elements[i+1])
				{
					pad = address % rettype->elements[i+1]->alignment;
					if( pad != 0)
					{
						pad =  rettype->elements[i+1]->alignment - pad;
					}
					
					address +=  pad;
					offset += rettype->elements[i]->size + pad;
				}
			}
			break;
			
		default:
			lua_pushnil(L);
			break;
	}
}
/*
static void dump(ffi_type* st)
{
	printf("Type: %d %d\n", st->size, st->alignment);
	if(st->type == FFI_TYPE_STRUCT)
	for (int i = 0; st->elements[i] != NULL; i++)
	{
		dump(st->elements[i]);
	}
	
}
*/
static int l_ffi_call(lua_State* L)
{
	ffi_type * argvtype[MAX_FN_ARGC];
	void* argv[MAX_FN_ARGC];
	ffi_type * rettype = lua_touserdata(L,1);
	int argc = l_ffi_prepare(L, argvtype, 2);
	int len = lua_rawlen(L,4);
	void* ret = NULL;
	ffi_cif cif;
	//dump(argvtype[0]);
	if(ffi_prep_cif(&cif,FFI_DEFAULT_ABI,argc,rettype,argvtype) == FFI_OK)
	{
		void * fn = lua_touserdata(L,3);
		if(!fn)
		{
			LOG("%s\n", "function not found");
			lua_pushboolean(L,0);
			return 1;
		}
		if(len != argc)
		{
			LOG("Argument count does not not match: expected %d, but have: %d\n", argc, len);
			lua_pushboolean(L,0);
			return 1;
		}
		// the arguments of the function is at 4th position on the stack
		// we need to loop through this table and check if argument type
		// is correct to the definition in argvtype
		parser_arguments(L,4,argv,argvtype);
		if(rettype->type != FFI_TYPE_VOID)
			ret = (void*)malloc(rettype->size);
		ffi_call(&cif,fn, ret, argv);
		for(int i = 0; i< argc; i++)
		{
			if(argv[i]) free(argv[i]);
		}
		ffi_post_call(L,ret, rettype);
		if(ret) free(ret);
		//lua_pushboolean(L,1);
		return 1;
	}
	lua_pushboolean(L,0);
	
	return 1;
}

static int l_ffi_atomic_type(lua_State* L)
{
	int etype = (int)luaL_checknumber(L,1);
	ffi_type* type = NULL; 
	if(etype > L_FFI_TYPE_POINTER)
	{
		lua_pushnil(L);
		return 1;
	}
	type = ffi_atomic_type_ptrs[etype];
	lua_pushlightuserdata(L,type);
	return 1;
}

static int l_ffi_struct(lua_State* L)
{
	// 1st element in the stack is the
	// struct table
	int len = lua_rawlen(L,1);
	ffi_type* cstruct = lua_newuserdata(L,sizeof(ffi_type) + (len+1) * sizeof(ffi_type*));
	void *ptr = (void*)cstruct+(sizeof(ffi_type));
	cstruct->elements = (ffi_type**)ptr;
	int i = 0;
    cstruct->size = cstruct->alignment = 0;
    cstruct->type = FFI_TYPE_STRUCT;
    // now iterate the lua table to pick all the type

	lua_pushvalue(L,1);
	// stack now contains: -1 => table
	lua_pushnil(L);
	// stack now contains: -1 => nil, -2 => table
	while(lua_next(L, -2))
	{
		// stack now contains: -1 => value; -2 key; -3 table
		cstruct->elements[i] = lua_touserdata(L, -1);
		i++;
		// pop the value, leaving the original key
		lua_pop(L,1);
		// stack now contains: -1 key; -2 table
	}
	// null terminated elements
	cstruct->elements[i] = NULL;
	// lua_next return 0, it popout the key at -1, leaving the table
	// so, popout the table
	lua_pop(L,1);
	// the top of the stack is now the new user data
	return 1;
}

static int l_ffi_new(lua_State* L)
{
	int size = luaL_checkinteger(L, 1);
	void* ptr = lua_newuserdata(L, size);
	memset(ptr,size,0);
	return 1;
}

static int l_ffi_meta(lua_State* L)
{
	ffi_type* type = lua_touserdata(L,1);
	if(type)
	{
		lua_newtable(L);
		lua_pushstring(L,"size");
		lua_pushnumber(L, type->size);
		lua_settable(L, -3);

		lua_pushstring(L,"alignment");
		lua_pushnumber(L, type->alignment);
		lua_settable(L, -3);

		lua_pushstring(L,"type");
		switch (type->type)
		{
			case FFI_TYPE_POINTER:
				lua_pushstring(L, "POINTER");
				break;

			case FFI_TYPE_UINT8:
				lua_pushstring(L, "UINT8");
				break;
			case FFI_TYPE_SINT8:
				lua_pushstring(L, "SINT8");
				break;
			case FFI_TYPE_UINT16:
				lua_pushstring(L, "UINT16");
				break;
			case FFI_TYPE_SINT16:
				lua_pushstring(L, "SINT16");
				break;
			case FFI_TYPE_UINT32:
				lua_pushstring(L, "UINT32");
				break;
			case FFI_TYPE_SINT32:
				lua_pushstring(L, "SINT32");
				break;
			case FFI_TYPE_UINT64:
				lua_pushstring(L, "UINT64");
				break;
			case FFI_TYPE_SINT64:
				lua_pushstring(L, "SINT64");
				break;
			/*case FFI_TYPE_LONGDOUBLE:
				lua_pushstring(L, "LONGDOUBLE");
				break;*/
			case FFI_TYPE_FLOAT:
				lua_pushstring(L, "FLOAT");
				break;
			case FFI_TYPE_DOUBLE:
				lua_pushstring(L, "DOUBLE");
				break;
			case FFI_TYPE_STRUCT:
				lua_pushstring(L, "STRUCT");
				break;
		default:
			lua_pushnil(L);
			break;
		}

		lua_settable(L, -3);
		return 1;
	}
	lua_pushnil(L);
	return 1;
}

static int l_ffi_offset(lua_State* L)
{
	void* ptr = lua_touserdata(L, 1);
	int off = luaL_checkinteger(L,2);
	if(ptr)
	{
		lua_pushlightuserdata(L, ptr + off);
		return 1;
	}
	lua_pushnil(L);
	return 1;
}
static int l_ffi_string(lua_State* L)
{
	void* ptr = lua_touserdata(L,1);
	if(ptr)
		lua_pushstring(L, (const char*)ptr);
	else
		lua_pushstring(L, "");
	return 1;
}
static int l_ffi_free(lua_State* L)
{
	void* ptr = lua_touserdata(L,1);
	if(ptr)
		free(ptr);
	lua_pushboolean(L, 1);
	return 1;
}
static const struct luaL_Reg _lib [] = {
	{"dlopen", l_dlopen},
	{"dlsym",l_dlsym},
	{"dlclose",l_dlclose},
	{"call",l_ffi_call},
	{"atomic", l_ffi_atomic_type},
	{"struct", l_ffi_struct },
	{"new", l_ffi_new},
	{"meta", l_ffi_meta},
	{"at", l_ffi_offset},
	// special case: pointer to string
	{"string", l_ffi_string},
	{"free", l_ffi_free},
	{NULL,NULL}
};

int luaopen_ffi(lua_State *L)
{
	luaL_newlib(L, _lib);
	return 1;
}
