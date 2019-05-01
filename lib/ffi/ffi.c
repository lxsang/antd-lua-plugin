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

typedef struct {
	uint8_t allocated;
	uint8_t is_pointer;
	void* data;
} arg_pointer_t;
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

void parser_value(lua_State* L, int idx, ffi_type* ffitype, arg_pointer_t * ptr)
{
	ptr->allocated = 1;
	ptr->is_pointer = 0;
	switch(ffitype->type)
	{
		case FFI_TYPE_POINTER:
			ptr->allocated = 0;
			ptr->is_pointer = 1;
			ptr->data = (void*)lua_tostring(L, idx);
			return;

		case FFI_TYPE_UINT8:
			ptr->data = (void*) malloc(ffitype->size);
			*((uint8_t*)ptr->data) = (uint8_t)lua_tonumber(L,idx);
			return;

		case FFI_TYPE_SINT8:
			ptr->data = (void*) malloc(ffitype->size);
			*((int8_t*)ptr->data) = (int8_t)lua_tonumber(L,idx);
			return;

		case FFI_TYPE_UINT16:
			ptr->data = (void*) malloc(ffitype->size);
			*((uint16_t*)ptr->data) = (uint16_t)lua_tonumber(L,idx);
			return;

		case FFI_TYPE_SINT16:
			ptr->data = (void*) malloc(ffitype->size);
			*((int16_t*)ptr->data) = (int16_t)lua_tonumber(L,idx);
			return;

		case FFI_TYPE_UINT32:
			ptr->data = (void*) malloc(ffitype->size);
			*((uint32_t*)ptr->data) = (uint32_t)lua_tonumber(L,idx);
			return;

		case FFI_TYPE_SINT32:
			ptr->data = (void*) malloc(ffitype->size);
			*((int32_t*)ptr->data) = (int32_t)lua_tonumber(L,idx);
			return;

		case FFI_TYPE_UINT64:
			ptr->data = (void*) malloc(ffitype->size);
			*((uint64_t*)ptr->data) = (uint64_t)lua_tonumber(L,idx);
			return;

		case FFI_TYPE_SINT64:
			ptr->data = (void*) malloc(ffitype->size);
			*((int64_t*)ptr->data) = (int64_t)lua_tonumber(L,idx);
			return; 

		case FFI_TYPE_LONGDOUBLE:
			/*This is bug in lua*/
			ptr->data = (void*) malloc(ffitype->size);
			*((long double*)ptr->data) = (long double)lua_tonumber(L,idx);
			return;

		case FFI_TYPE_FLOAT:
		case FFI_TYPE_DOUBLE:
			ptr->data = (void*) malloc(ffitype->size);
			*((float*)ptr->data) = (float)lua_tonumber(L,idx);
			return;

		
		/*case FFI_TYPE_STRUCT:
			// not implemented yet
			return NULL;
			break;*/
		default: 
			ptr->allocated = 0;
			ptr->is_pointer = 0;
			ptr->data = NULL;
			break;
	}
}

static void parser_arguments(lua_State* L, int idx, arg_pointer_t* argv, ffi_type** argvtype)
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
		parser_value(L, -1, argvtype[i],&argv[i]);
		i++;
		// pop the value, leaving the original key
		lua_pop(L,1);
		// stack now contains: -1 key; -2 table
	}
	// lua_next return 0, it popout the key at -1, leaving the table
	// so, popout the table
	lua_pop(L,1);
}

static void free_arguments(arg_pointer_t* argv, int argc)
{
	for(int i = 0; i< argc; i++)
	{
		if(argv[i].allocated)
			free(argv[i].data);
	}
	
}

static int l_ffi_call(lua_State* L)
{
	ffi_type * argvtype[MAX_FN_ARGC];
	void* argv[MAX_FN_ARGC];
	ffi_type * rettype = lua_touserdata(L,1);
	int argc = l_ffi_prepare(L, argvtype, 2);
	arg_pointer_t* args;
	ffi_arg ret;
	ffi_cif cif;
	if(ffi_prep_cif(&cif,FFI_DEFAULT_ABI,argc,rettype,argvtype) == FFI_OK)
	{
		void * fn = lua_touserdata(L,3);
		if(!fn)
		{
			LOG("%s\n", "function not found");
			lua_pushboolean(L,0);
			return 1;
		}
		if(lua_rawlen(L,4) != argc)
		{
			LOG("%s\n", "Argument count does not not match");
			lua_pushboolean(L,0);
			return 1;
		}
		// the arguments of the function is at 4th position on the stack
		// we need to loop through this table and check if argument type
		// is correct to the definition in argvtype
		args = (arg_pointer_t*)malloc(sizeof(arg_pointer_t)*(argc+1));
		//ret = (void*)malloc(rettype->size);
		// now parser the argument
		parser_arguments(L,4,args,argvtype);
		
		for(int i = 0; i< argc; i++)
		{
			if(args[i].is_pointer && args[i].data)
				argv[i] = &(args[i].data);
			else
				argv[i] = args[i].data;
		}
		ffi_call(&cif,fn, &ret, argv);
		free_arguments(args, argc);
		if(args) free(args);
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
	{"atomic", l_ffi_atomic_type},
	{"struct", l_ffi_struct },
	{NULL,NULL}
};

int luaopen_ffi(lua_State *L)
{
	luaL_newlib(L, _lib);
	return 1;
}
