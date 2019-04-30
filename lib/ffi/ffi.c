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
	LOG("%s\n","Begin close");
	void* handle = lua_touserdata(L,1);
	if(!handle)
	{
		LOG("%s\n","Cannot close that thing, handle not found");
		lua_pushboolean(L,0);
		return 1;
	}
	LOG("%s\n","the handle is found");
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

void * parser_value(lua_State* L, int idx, ffi_type* ffitype)
{
	lua_Number * value;
	switch(ffitype->type)
	{
		case FFI_TYPE_VOID : return NULL;
		case FFI_TYPE_POINTER:
			// TODO: need to fix this to universal pointer
			return lua_tostring(L, idx);

		case FFI_TYPE_UINT8:
		case FFI_TYPE_SINT8:
		case FFI_TYPE_UINT16:
		case FFI_TYPE_SINT16:
		case FFI_TYPE_UINT32:
		case FFI_TYPE_SINT32:
		case FFI_TYPE_UINT64:
		case FFI_TYPE_SINT64:
		case FFI_TYPE_LONGDOUBLE:
		case FFI_TYPE_FLOAT:
		case FFI_TYPE_DOUBLE:
			value = (lua_Number*) malloc(ffitype->size);
			*value = lua_tonumber(L,idx);
			break;

		
		case FFI_TYPE_STRUCT:
			// not implemented yet
			return NULL;
			break;
		default: return NULL;
	}
	return (void*)value;
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
		argv[i] = parser_value(L, -1, argvtype[i]);
		i++;
		// pop the value, leaving the original key
		lua_pop(L,1);
		// stack now contains: -1 key; -2 table
	}
	// lua_next return 0, it popout the key at -1, leaving the table
	// so, popout the table
	lua_pop(L,1);
}

static void free_arguments(void** argv, ffi_type** argvtype)
{
	ffi_type * ffitype;
	for(int i = 0; argvtype[i] != NULL; i++)
	{
		ffitype = argvtype[i];
		switch (ffitype->type)
		{
			case FFI_TYPE_UINT8:
			case FFI_TYPE_SINT8:
			case FFI_TYPE_UINT16:
			case FFI_TYPE_SINT16:
			case FFI_TYPE_UINT32:
			case FFI_TYPE_SINT32:
			case FFI_TYPE_UINT64:
			case FFI_TYPE_SINT64:
			case FFI_TYPE_LONGDOUBLE:
			case FFI_TYPE_FLOAT:
			case FFI_TYPE_DOUBLE:
				if(argv[i]) free(argv[i]);
				break;
			default: break;
		}
	}
	
}

static int l_ffi_call(lua_State* L)
{
	ffi_type * argvtype[MAX_FN_ARGC];
	ffi_type * rettype = lua_touserdata(L,1);
	int argc = l_ffi_prepare(L, argvtype, 2);
	printf("Argument count %d\n", argc);
	printf("ret type: %d\n", rettype->type);
	void* argv[MAX_FN_ARGC];
	ffi_arg ret;
	ffi_cif cif;
	if(ffi_prep_cif(&cif,FFI_DEFAULT_ABI,argc,rettype,argvtype) == FFI_OK)
	{
		void(* fn)(const char*) = lua_touserdata(L,3);
		if(!fn)
		{
			LOG("%s\n", "function not found");
			lua_pushboolean(L,0);
			return 1;
		}
		// the arguments of the function is at 4th position on the stack
		// we need to loop through this table and check if argument type
		// is correct to the definition in argvtype
		//argv = (void**)malloc(sizeof(void*)*argc);
		//ret = (void*)malloc(rettype->size);
		// now parser the argument
		parser_arguments(L,4,argv,argvtype);
		// test
		char* tmp = (char*)argv[0];
		argv[0] = &tmp;
		ffi_call(&cif,fn, &ret, argv);
		free_arguments(argv, argvtype);
		//if(argv) free(argv);
		//if(ret) free(ret);
		lua_pushboolean(L,1);
		return 1;
	}
	printf("fail to prepare %d\n");
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
	ffi_type* cstruct = lua_newuserdata(L,  (len+2) * sizeof(ffi_type)  );
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

static const struct luaL_Reg _lib [] = {
	{"dlopen", l_dlopen},
	{"dlsym",l_dlsym},
	{"dlclose",l_dlclose},
	{"call",l_ffi_call},
	{"atomic_type", l_ffi_atomic_type},
	{"struct", l_ffi_struct },
	{NULL,NULL}
};

int luaopen_ffi(lua_State *L)
{
	luaL_newlib(L, _lib);
	return 1;
}
