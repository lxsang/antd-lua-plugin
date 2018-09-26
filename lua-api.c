#include "lua-api.h"

static const struct luaL_Reg modules [] = {
#ifdef USE_DB
       {"sqlite", luaopen_sqlite},
#endif
	   {"std", luaopen_standard},
	   {"JSON", luaopen_json},
	   {"bytes",luaopen_barray},
	   {"array",luaopen_array},
	   {NULL,NULL}  
};

void init()
{
	//signal(SIGPIPE, SIG_IGN);
	//signal(SIGABRT, SIG_IGN);
	// init the plugin here
	LOG("%s \n","INIT LUA HANDLER");
}
/**

* Plugin handler, reads request from the server and processes it
* 
*/
static void push_dict_to_lua(lua_State* L, dictionary d)
{
	lua_newtable(L);
	
	association as;
	if(d)
		for_each_assoc(as, d)
		{
			lua_pushstring(L,as->key);
			//printf("KEY %s\n", as->key);
			if(EQU(as->key,"cookie") || EQU(as->key,"__xheader__"))
				push_dict_to_lua(L, (dictionary)as->value);
			else
			{
				lua_pushstring(L,as->value);
				//printf("VALUE : %s\n",as->value );
			}
			lua_settable(L, -3);
		}
}
void handle(void* client, const char* method, const char* path, dictionary rq)
{
	lua_State* L = NULL;
	//char * index = __s("%s/%s",__plugin__.htdocs,"router.lua");
	char* cnf = config_dir();
	char * apis = __s("%s/%s",cnf,"api.lua");
	L = luaL_newstate();
	luaL_openlibs(L);
	//module loader
	luaL_newlib(L, modules);
	lua_setglobal(L, "modules");
	// set up global variable
	// API header
	lua_newtable(L);
	lua_pushstring(L,"name");
	lua_pushstring(L, __plugin__.name);
	lua_settable(L,-3);
	
	lua_pushstring(L,"root");
	lua_pushstring(L, __plugin__.htdocs);
	lua_settable(L,-3);
	
	lua_pushstring(L,"apiroot");
	lua_pushstring(L, cnf);
	lua_settable(L,-3);
	
	lua_setglobal(L, "__api__");
	
	// Request
	lua_newtable(L);
	lua_pushstring(L,"id");
	lua_pushlightuserdata(L, client);
	//lua_pushnumber(L,client);
	lua_settable(L, -3);
	
	lua_pushstring(L,"method");
	lua_pushstring(L,method);
	lua_settable(L, -3);
	
	lua_pushstring(L,"path");
	lua_pushstring(L,path);
	lua_settable(L, -3);
	
	lua_pushstring(L,"query");
	push_dict_to_lua(L,rq);
	lua_settable(L, -3);
	lua_setglobal(L, "REQUEST");
	
	// load major apis
	if(is_file(apis))
		if (luaL_loadfile(L, apis) || lua_pcall(L, 0, 0, 0))
		{
			LOG( "cannot run apis. file: %s\n", lua_tostring(L, -1));
		}
	
	/*if (luaL_loadfile(L, index) || lua_pcall(L, 0, 0, 0))
	{
		text(client);
		__t(client, "Cannot run router: %s", lua_tostring(L, -1));
	}
	free(index);*/
	// clear request
	if(L)
		lua_close(L);
	if(cnf)
		free(cnf);
	if(apis)
		free(apis);
	//lua_close(L);
}
void pexit()
{
	LOG("%s \n","Exit LUA Handler");
}
