#include "lua-api.h"

//void header(int,const char*);
static int l_header (lua_State *L) {
	//int client = (int)luaL_checknumber(L, 1);
	void* client = lua_touserdata (L, 1);
	const char* s = luaL_checkstring(L,2);
	ctype(client,s);
	return 0;  /* number of results */
}

//void redirect(int,const char*);
static int l_redirect (lua_State *L) {
	void* client = lua_touserdata (L, 1);
	const char* s = luaL_checkstring(L,2);
	redirect(client,s);
	return 0;  /* number of results */
}

//void html(int);
static int l_html (lua_State *L) {
	void* client = lua_touserdata (L, 1);
	html(client);
	return 0;  /* number of results */
}

//void text(int);
static int l_text (lua_State *L) {
	void* client = lua_touserdata (L, 1);
	text(client);
	return 0;  /* number of results */
}

//void json(int);
static int l_json (lua_State *L) {
	void* client = lua_touserdata (L, 1);
	json(client);
	return 0;  /* number of results */
}

//void jpeg(int);
static int l_jpeg (lua_State *L) {
	void* client = lua_touserdata (L, 1);
	jpeg(client);
	return 0;  /* number of results */
}

//void octstream(int, char*);
static int l_octstream (lua_State *L) {
	void* client = lua_touserdata (L, 1);
	const char* s = luaL_checkstring(L,2);
	octstream(client,s);
	return 0;  /* number of results */
}

//void textstream(int);
static int l_textstream (lua_State *L) {
	void* client = lua_touserdata (L, 1);
	textstream(client);
	return 0;  /* number of results */
}
// int mime
static int l_mime(lua_State* L)
{
	const char* file = luaL_checkstring(L,1);
	lua_pushstring(L,mime(file));
	return 1;
}
// int mime
static int l_ext(lua_State* L)
{
	const char* file = luaL_checkstring(L,1);
	char* e = ext(file);
	lua_pushstring(L,e);
	if(e)
		free(e);
	return 1;
}
static int l_is_bin(lua_State* L)
{
	const char* file = luaL_checkstring(L,1);
	lua_pushboolean(L, is_bin(file));
	return 1;
}
//int __ti(int,int);
static int l_ti (lua_State *L) {
	void* client = lua_touserdata (L, 1);
	int v = (int)luaL_checknumber(L, 2);
	lua_pushnumber(L, __ti(client,v));
	return 1;  /* number of results */
}

//int __t(int, const char*,...);
static int l_t (lua_State *L) {
	void* client = lua_touserdata (L, 1);
	const char*  v = luaL_checkstring(L, 2);
	lua_pushnumber(L, __t(client,v));
	return 1;  /* number of results */
}

//int __b(int, const unsigned char*, int);
//static int l_b (lua_State *L) {
//}

//int __f(int, const char*);
static int l_f (lua_State *L) {
	void* client = lua_touserdata (L, 1);
	const char*  v = luaL_checkstring(L, 2);
	lua_pushnumber(L, __f(client,v));
	return 1;
}

//int __fb(int, const char*);
static int l_fb (lua_State *L) {
	void* client = lua_touserdata (L, 1);
	const char*  v = luaL_checkstring(L, 2);
	lua_pushnumber(L, __fb(client,v));
	return 1;
}

//int upload(const char*, const char*);
static int l_upload (lua_State *L) {
	const char* s = luaL_checkstring(L, 1);
	const char* d = luaL_checkstring(L, 2);
	lua_pushnumber(L, upload(s,d));
	return 1;
}

//char* route(const char*);
static int l_route (lua_State *L) {
	const char* s = luaL_checkstring(L, 1);
	lua_pushstring(L, route(s));
	return 1;
}

//char* htdocs(const char*);
#ifdef USE_DB
//sqldb getdb();
#endif
//void set_cookie(int,dictionary);

//void unknow(int);
static int l_unknow (lua_State *L) {
	void* client = lua_touserdata (L, 1);
	unknow(client);
	return 0;
}

static int l_log(lua_State *L)
{
	const char* s = luaL_checkstring(L,1);
	LOG("%s",s);
	return 0;
}

dictionary iterate_lua_table(lua_State *L, int index)
{
    // Push another reference to the table on top of the stack (so we know
    // where it is, and this function can work for negative, positive and
    // pseudo indices
	dictionary dic = dict();
    lua_pushvalue(L, index);
    // stack now contains: -1 => table
    lua_pushnil(L);
    // stack now contains: -1 => nil; -2 => table
    while (lua_next(L, -2))
    {
        // stack now contains: -1 => value; -2 => key; -3 => table
        // copy the key so that lua_tostring does not modify the original
        lua_pushvalue(L, -2);
        // stack now contains: -1 => key; -2 => value; -3 => key; -4 => table
        const char *key = lua_tostring(L, -1);
        if(lua_istable(L,-2))
        {
			// the element is a table
			// create new dictionary
			dictionary cdic = iterate_lua_table(L,-2);
			dput(dic,key, cdic);
		}
		else
		{
			// other value is converted to string
			const char *value = lua_tostring(L, -2);
			//LOG("%s=%s\n", key, value);
			dput(dic,key, strdup(value));
		}
        
        // pop value + copy of key, leaving original key
        lua_pop(L, 2);
        // stack now contains: -1 => key; -2 => table
    }
    // stack now contains: -1 => table (when lua_next returns 0 it pops the key
    // but does not push anything.)
    // Pop table
    lua_pop(L, 1);
    // Stack is now the same as it was on entry to this function
	return dic;
}

static int l_set_cookie(lua_State* L)
{
	if (lua_istable(L, 3))
	{
		dictionary d = iterate_lua_table(L,-2);
		if(d)
		{
			void* client = lua_touserdata (L, 1);
			const char* type = luaL_checkstring(L,2);
			const char* path = luaL_checkstring(L,4);
			set_cookie(client, type,d,path);
			freedict(d);
		}
	}
	return 0;
}
static int l_simple_hash(lua_State* L)
{
	const char* s = luaL_checkstring(L,1);
	lua_pushnumber(L, simple_hash(s));
	return 1;
}
static int l_md5(lua_State* L)
{
	const char* s = luaL_checkstring(L,1);
	int len = strlen(s);
	char buff[256];
	md5(s,len,buff);
	lua_pushstring(L,buff);
	return 1;	
}
static int l_sha1(lua_State *L )
{
	const char* s = luaL_checkstring(L,1);
	char buff[80];
	sha1(s,buff);
	lua_pushstring(L,buff);
	return 1;
}
static int l_base64_encode(lua_State *L)
{
	const char* s;
	int len;
	char* dst;
	byte_array_t *arr = NULL;
	if(lua_isstring(L,1))
	{
		s= luaL_checkstring(L,1);
		len = strlen(s);
	}
	else
	{
		// this may be an bytearray
		arr = l_check_barray(L,1);
		s = arr->data;
		len = arr->size;
	}
	if(len == 0)
	{
		lua_pushstring(L,"");
		return 1;
	}
	dst = (char*) malloc( ((len * 4) / 3) + (len / 96) + 6 );//(((4 * len / 3) + 3) & ~3)+1
	Base64encode(dst, s,len);
	lua_pushstring(L,dst);
	free(dst);
	return 1;
}
static int l_base64_decode(lua_State *L)
{
	const char* s = luaL_checkstring(L,1);
	int len = Base64decode_len(s);
	// decode data to a byte array
	lua_new_byte_array(L,len);
	byte_array_t * arr = NULL;
	arr = l_check_barray(L,2);
	Base64decode(arr->data, s);
	//lua_pushstring(L,dst);
	//free(dst);
	return 1;
}
/*
* web socket handle for lua
*/
static int l_ws_read_header(lua_State *L)
{
	void* client = lua_touserdata (L, 1);
	ws_msg_header_t * header = ws_read_header(client);
	if(!header)
	{
		lua_pushnil(L);
		return 1;
	}
	else
	{
		// create newtable
		lua_newtable(L);
		
		lua_pushstring(L,"fin");
		lua_pushnumber(L,(int)header->fin);
		lua_settable(L,-3);
		
		lua_pushstring(L,"opcode");
		lua_pushnumber(L,(int)header->opcode);
		lua_settable(L,-3);
		
		lua_pushstring(L,"plen");
		lua_pushnumber(L,(int)header->plen);
		lua_settable(L,-3);
		
		lua_pushstring(L,"mask");
		lua_pushnumber(L,(int)header->mask);
		lua_settable(L,-3);
		
		lua_pushstring(L,"mask_key");
		lua_newtable(L);
		
		lua_pushnumber(L,0);
		lua_pushnumber(L,(int)header->mask_key[0]);
		lua_settable(L,-3);
		
		lua_pushnumber(L,1);
		lua_pushnumber(L,(int)header->mask_key[1]);
		lua_settable(L,-3);
		
		lua_pushnumber(L,2);
		lua_pushnumber(L,(int)header->mask_key[2]);
		lua_settable(L,-3);
		
		lua_pushnumber(L,3);
		lua_pushnumber(L,(int)header->mask_key[3]);
		lua_settable(L,-3);
		
		lua_settable(L,-3);
		
		free(header);
		return 1;
	}
}

/*
 * Read socket data given the header
 */
 static int l_ws_read_data(lua_State *L)
 {
	 void* client = lua_touserdata (L, 1);
	 if(!lua_istable(L,2))
	 {
		 ws_close(client,1011);
		 lua_pushnil(L);
		 return 1;
	 }
	 dictionary dic = iterate_lua_table(L,2);
	 if(dic)
	 {
		 // convert dictionary to header
		 ws_msg_header_t * header =  (ws_msg_header_t*) malloc(sizeof(*header));
		 header->fin = (uint8_t)(R_INT(dic,"fin"));
		 header->opcode = (uint8_t)(R_INT(dic,"opcode"));
		 header->mask = 1;
		 header->plen = R_INT(dic,"plen");
		 dictionary d1 = (dictionary)dvalue(dic,"mask_key");
		 if(d1)
		 {
			 header->mask_key[0] = (uint8_t)(R_INT(d1,"0"));
			 header->mask_key[1] = (uint8_t)(R_INT(d1,"1"));
			 header->mask_key[2] = (uint8_t)(R_INT(d1,"2"));
			 header->mask_key[3] = (uint8_t)(R_INT(d1,"3"));
			 freedict(d1);
			 dput(dic,"mask_key", NULL);
		 }
		 freedict(dic);
		
		// read data 
		// This return max 1024 bytes data, 
		//user should handle consecutive read 
		// by examining the header (FIN bit and plen)
		uint8_t* data = (uint8_t*) malloc(header->plen+1);
		int size = ws_read_data(client, header, header->plen, data);
		if(size < 0)
		{
			lua_pushnil(L);
			free(header);
			if(data) free(data);
			return 1;
		}
		
		if(header->opcode == WS_TEXT)
		{
			data[size] = '\0';
			lua_pushstring(L, (char*)data);
		}
		else
		{
			// binary data as byte array
			// this is not an optimal way to store byte array
			// TODO: we may need a dedicated byte array type
			// for it
			lua_new_byte_array(L,size);
			byte_array_t * arr = NULL;
			arr = l_check_barray(L,3);
			memcpy(arr->data,data,size);
			/*lua_newtable(L);
			for (int i = 0; i < size; i++)
			{
				lua_pushnumber(L,i);
				lua_pushnumber(L,(int)(data[i]));
				lua_settable(L,-3);
			}*/
			
		}
		if(data)
			free(data);
		free(header);
	 }
	 else
	 {
		 lua_pushnil(L);
	 }
	 return 1;
 }
 /*
  * Send a text to web socket
  */
static int l_ws_t(lua_State*L)
{
	void* client = lua_touserdata (L, 1);
	char* str = luaL_checkstring(L,2);
	ws_t(client,str);
	return 1;
}
static int l_status(lua_State*L)
{
	void* client = lua_touserdata (L, 1);
	int code = (int) luaL_checknumber(L,2);
	const char* msg = luaL_checkstring(L,3);
	set_status(client,code,msg);
	return 1;
}
/*
 * send a file as binary data
 */
 static int l_ws_f(lua_State*L)
 {
	void* client = lua_touserdata (L, 1);
	char* str = luaL_checkstring(L,2);
	ws_f(client,str);
	return 1;
 }
 static int l_ws_bin(lua_State*L)
 {
	byte_array_t * arr = NULL;
	void* client = lua_touserdata (L, 1);
	arr = l_check_barray(L,2);
	ws_b(client, arr->data, arr->size);
	return 1;
 }
 static int l_ws_close(lua_State *L)
 {
	 void* client = lua_touserdata (L, 1);
	 int status = (int) luaL_checknumber(L,2);
	 ws_close(client,status);
	 return 1;
 }

 static int l_trim(lua_State* L)
 {
	 const char* str = strdup((char*)luaL_checkstring(L,1));
	 const char* tok = luaL_checkstring(L,2);
	 
	 trim(str,tok[0]);
	 lua_pushstring(L,(const char*)str);
	 free(str);
	 return 1;
 }
 
 static int l_is_dir(lua_State* L)
 {
	 const char* file = (char*)luaL_checkstring(L,1);
	 lua_pushboolean(L,is_dir(file) == 1);
	 return 1;
 }


static const struct luaL_Reg standard [] = {
       {"_header", l_header},
	   {"_redirect", l_redirect},
	   {"_html", l_html},
	   {"_text", l_text},
	   {"_json", l_json},
	   {"_jpeg", l_jpeg},
	   {"b64encode", l_base64_encode},
	   {"b64decode", l_base64_decode},
	   {"_octstream", l_octstream} ,
	   {"_textstream", l_textstream} ,
	   {"_ti", l_ti} ,
	   {"_t", l_t} ,
	   {"_f", l_f} ,
	   {"_fb", l_fb} ,
	   {"trim", l_trim},
	   {"upload", l_upload} ,
	   {"route", l_route} ,
	   {"mime", l_mime} ,
	   {"is_bin", l_is_bin} ,
	   {"_unknow", l_unknow} ,
	   {"_status", l_status},
	   {"console", l_log} ,
	   {"_setCookie", l_set_cookie},
	   {"hash",l_simple_hash},
	   {"md5",l_md5},
	   {"sha1",l_sha1},
	   {"ext",l_ext},
	   {"ws_header",l_ws_read_header},
	   {"ws_read",l_ws_read_data},
	   {"ws_f",l_ws_f},
	   {"ws_t", l_ws_t},
	   {"ws_b", l_ws_bin},
	   {"ws_close", l_ws_close},
	   {"is_dir", l_is_dir},
	   {NULL,NULL}  
};

int luaopen_standard(lua_State *L)
{
	luaL_newlib(L, standard);
	return 1;
}
