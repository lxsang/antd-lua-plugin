#define PLUGIN_IMPLEMENT 1
#include <dlfcn.h>
#include <antd/plugin.h>
#include <antd/scheduler.h>
#include <antd/dbhelper.h>
#include <sys/stat.h>
#include "lib/lualib.h"

#define LUA_HDL_FN "lua_handle"
static void* core = NULL;
static void* lua_handle = NULL;
static void *(*handle_fn)(void *, void*);

void init()
{
	char* error;
	char* path = __s("%s/lua/core.so", __plugin__.pdir);
	core = dlopen(path, RTLD_NOW| RTLD_GLOBAL);
	free(path);
	if(!core)
	{
		ERROR("Cannot load Lua core: %s", dlerror());
		return;
	}
	LOG("Lua core loaded");
	// now load the handle
	path = __s("%s/lua/handle.so", __plugin__.pdir);
	lua_handle = dlopen(path, RTLD_LAZY);
	free(path);
	if(!lua_handle)
	{
		ERROR("Cannot load lua_handle: %s", dlerror());
		return;
	}
	// find the fn
	handle_fn = (void *(*)(void *))dlsym(lua_handle, LUA_HDL_FN);
	if ((error = dlerror()) != NULL)
	{
		ERROR("Problem when finding %s method from handle : %s", LUA_HDL_FN, error);
		handle_fn = NULL;
		return;
	}
	LOG("Lua module initialized");
}

void* handle(void* data)
{
	plugin_header_t* meta_ptr = (void*)meta();
	antd_request_t *rq = (antd_request_t *)data;
	// find the handle function and execute it
	if(!handle_fn)
	{
		antd_error(rq->client, 503, "Requested service not found");
		return antd_create_task(NULL, (void *)rq, NULL, rq->client->last_io);
	}
	return handle_fn(data, meta_ptr);
}
void destroy()
{
	if(core)
		(void)dlclose(core);
	if(lua_handle)
		(void)dlclose(lua_handle);
	LOG("Exit LUA Handle");
}
