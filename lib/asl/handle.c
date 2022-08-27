#include <antd/plugin.h>
#include <sys/stat.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/tcp.h> 
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include "../lualib.h"

typedef struct {
	plugin_header_t* __plugin__;
	int fd;
} lua_thread_data_t;

void* lua_handle(void* ptr)
{
	lua_thread_data_t* data = (lua_thread_data_t**)ptr;
	lua_State* L = NULL;
	antd_client_t cl = {0};
	cl.sock = data->fd;
	time(&cl.last_io);
	cl.ssl = NULL;
	cl.state = ANTD_CLIENT_PLUGIN_EXEC;
	cl.z_status = 0;
	cl.z_level = ANTD_CNONE;
	cl.zstream = NULL;
	//char * index = __s("%s/%s",__plugin__->htdocs,"router.lua");
	char* cnf = __s("%s%s%s", data->__plugin__->pdir,DIR_SEP, data->__plugin__->name);
	char * apis = __s("%s/%s",cnf,"api.lua");
	L = luaL_newstate();
	luaL_openlibs(L);
	//module loader
	//luaL_newlib(L, modules);
	//lua_setglobal(L, "modules");
	// set up global variable
	// API header
	lua_newtable(L);
	lua_pushstring(L,"name");
	lua_pushstring(L, data->__plugin__->name);
	lua_settable(L,-3);
	
	//lua_pushstring(L,"root");
	//htdocs(rq, buf);
	//lua_pushstring(L, data->__plugin__->htdocs);
	//lua_settable(L,-3);
	
	lua_pushstring(L,"apiroot");
	lua_pushstring(L, cnf);
	lua_settable(L,-3);

	lua_pushstring(L,"tmpdir");
	lua_pushstring(L, data->__plugin__->tmpdir);
	lua_settable(L,-3);

	lua_pushstring(L,"dbpath");
	lua_pushstring(L, data->__plugin__->dbpath);
	lua_settable(L,-3);
	
	lua_setglobal(L, "__api__");
	
	// Request
	lua_newtable(L);
	lua_pushstring(L,"id");
	lua_pushlightuserdata(L, &cl);
	//lua_pushnumber(L,client);
	lua_settable(L, -3);

	lua_pushstring(L,"socket");
	lua_pushnumber(L, cl.sock);
	//lua_pushnumber(L,client);
	lua_settable(L, -3);

	int flag = 1;

	if (setsockopt(cl.sock, IPPROTO_TCP, TCP_NODELAY, &(int){1}, sizeof(int)) == -1)
	{
		ERROR("Unable to set TCP_NODELAY on %d - setsockopt: %s", cl.sock, strerror(errno));
	}
	//lua_pushstring(L,"request");
	//push_dict_to_lua(L,rq->request);
	//lua_settable(L, -3);
	lua_setglobal(L, "HTTP_REQUEST");
	free(ptr);
	// load major apis
	if(is_file(apis))
		if (luaL_loadfile(L, apis) || lua_pcall(L, 0, 0, 0))
		{
			ERROR( "cannot start API file: [%s] %s\n", apis, lua_tostring(L, -1));
			antd_error(&cl, 503, "Internal server error");
		}
	
	/*if (luaL_loadfile(L, index) || lua_pcall(L, 0, 0, 0))
	{
		text(client);
		__t(client, "Cannot run router: %s", lua_tostring(L, -1));
	}
	free(index);*/
	LOG("LUA handle exit on %d", cl.sock);
	// clear request
	if(L)
		lua_close(L);
	if(cnf)
		free(cnf);
	if(apis)
		free(apis);
	(void) close(cl.sock);
	return 0;
	//lua_close(L);
}
