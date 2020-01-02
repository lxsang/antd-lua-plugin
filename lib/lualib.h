#ifndef LUALIB_H
#define LUALIB_H
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>

#include <antd/handle.h>
#include <antd/utils.h>

#include "core/lua-5.3.4/lua.h"
#include "core/lua-5.3.4/lauxlib.h"
#include "core/lua-5.3.4/lualib.h"


#define ARRAY "modules.array"
#define BYTEARRAY "modules.bytes"
// add byte array support
 typedef struct{
	 int size;
	 unsigned char data[1];
 } byte_array_t;
 
typedef struct{
	int size;
	double data[1];
} array_t;
// new byte array
void lua_new_byte_array(lua_State*L, int n);
byte_array_t *l_check_barray(lua_State *L, int idx);
// new array
void lua_new_array(lua_State*L, int n);
array_t *l_check_array(lua_State *L, int idx);

#endif