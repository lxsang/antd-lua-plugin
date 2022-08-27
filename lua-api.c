#define PLUGIN_IMPLEMENT 1
#include <dlfcn.h>
#include <antd/plugin.h>
#include <antd/scheduler.h>

#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include <netinet/in.h>

#include <sys/stat.h>

#define LUA_HDL_FN "lua_handle"
#define MAX_SOCK_NAME 64
#define SOCKET_NAME "lua.sock"

#define MAX_SESSION_TIMEOUT (15u * 60u) //15 min
#define PING_INTERVAL 10u               // 10s
#define PROCESS_TIMEOUT 200u          //100ms

typedef struct {
    plugin_header_t* __plugin__;
    int fd;
} lua_thread_data_t;

static pid_t pid = 0;
static char sock_path[108];

static int open_unix_socket()
{
    struct sockaddr_un address;
    address.sun_family = AF_UNIX;
    (void) strncpy(address.sun_path, sock_path, sizeof(address.sun_path));
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(fd == -1)
    {
        ERROR( "Unable to create Unix domain socket: %s", strerror(errno));
        return -1;
    }
    if(connect(fd, (struct sockaddr*)(&address), sizeof(address)) == -1)
    {
        ERROR( "Unable to connect to socket '%s': %s", address.sun_path, strerror(errno));
        return -1;
    }
    LOG( "Socket %s is created successfully", sock_path);
    return fd;
}

static int mk_socket()
{
    struct sockaddr_un address;
    address.sun_family = AF_UNIX;
    // create the socket
    (void)strncpy(address.sun_path, sock_path, sizeof(address.sun_path));
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1)
    {
        ERROR("Unable to create Unix domain socket: %s", strerror(errno));
        return -1;
    }
    if (bind(fd, (struct sockaddr *)(&address), sizeof(address)) == -1)
    {
        ERROR("Unable to bind name: %s to a socket: %s", address.sun_path, strerror(errno));
        return -1;
    }
    // mark the socket as passive mode
    if (listen(fd, 500) == -1)
    {
        ERROR("Unable to listen to socket: %d (%s): %s", fd, sock_path, strerror(errno));
        return -1;
    }
    LOG("Socket %s is created successfully: %d", sock_path, fd);
    return fd;
}

static void lua_serve()
{
    void* core = NULL;
    void* lua_handle = NULL;
    void *(*handle_fn)(void*);
    char path[BUFFLEN];
    char* error;
    (void)snprintf(path, BUFFLEN, "%s/lua/core.so", __plugin__.pdir);
    core = dlopen(path, RTLD_NOW| RTLD_GLOBAL);
    if(!core)
    {
        ERROR("Cannot load Lua core: %s", dlerror());
        return;
    }
    // now load the handle
    (void)snprintf(path, BUFFLEN, "%s/lua/handle.so", __plugin__.pdir);
    lua_handle = dlopen(path, RTLD_LAZY);
    if(!lua_handle)
    {
        ERROR("Cannot load lua_handle: %s", dlerror());
        return;
    }
    // find the fn
    handle_fn = (void *(*)(void*))dlsym(lua_handle, LUA_HDL_FN);
    if ((error = dlerror()) != NULL)
    {
        ERROR("Problem when finding %s method from handle : %s", LUA_HDL_FN, error);
        handle_fn = NULL;
        return;
    }
    int socket = mk_socket();
    if(socket != -1)
    {
        int fd;
        if (setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1)
        {
            ERROR("Unable to set reuse address on %d - setsockopt: %s", socket, strerror(errno));
        }
        LOG("LUA server online");
		/*set log level*/
		const char * enable_debug = getenv("ANTD_DEBUG");
		int log_level = LOG_ERR;
		if(enable_debug)
		{
			if(atoi(enable_debug))
			{
				LOG("LUA Debug is enabled");
				log_level = LOG_NOTICE;
			}
		}
		setlogmask(LOG_UPTO(log_level));
        while((fd = accept(socket, NULL, NULL)) > 0)
        {
            pthread_t thread;
            lua_thread_data_t* data = (lua_thread_data_t*)malloc(sizeof(lua_thread_data_t));
            data->__plugin__ = &__plugin__;
            data->fd = fd;
            set_nonblock(fd);
            if (pthread_create(&thread, NULL, (void *(*)(void*))handle_fn, (void *)data) != 0)
            {
                ERROR("pthread_create: cannot create lua thread: %s", strerror(errno));
                (void)close(fd);
            }
            else
            {
                LOG("Serve thread created for %d", fd);
                pthread_detach(thread);
            }

        }
        if (fd < 0)
        {
            ERROR("Unable to accept the new connection: %s", strerror(errno));
        }
    }
    if(core)
        (void)dlclose(core);
    if(lua_handle)
        (void)dlclose(lua_handle);
    LOG("lua_serve: stop serve due to error");
}

void init()
{
	 (void)snprintf(sock_path, sizeof(sock_path), "%s/%s", __plugin__.tmpdir, SOCKET_NAME);
    LOG("Lua socket will be stored in %s", sock_path);
    pid = fork();
    if (pid == 0)
    {
        // child
        lua_serve();
    }
    LOG("Lua module initialized");
}

static void push_dict_to_socket(antd_client_t* cl, char* name, char* parent_name, dictionary_t d)
{
    antd_send(cl,name, strlen(name));
    antd_send(cl,"\n", 1);
    chain_t as;
    if(d)
        for_each_assoc(as, d)
        {
            if(EQU(as->key,"COOKIE") || EQU(as->key,"REQUEST_HEADER") || EQU(as->key,"REQUEST_DATA") )
                push_dict_to_socket(cl, as->key, name, (dictionary_t)as->value);
            else if(as->value)
            {
                antd_send(cl,as->key, strlen(as->key));
                antd_send(cl,"\n", 1);
                antd_send(cl,as->value, strlen(as->value));
                antd_send(cl,"\n", 1);
            }
        }
    antd_send(cl,parent_name, strlen(parent_name));
    antd_send(cl,"\n", 1);
}

static void *process(void *data)
{
    fd_set fd_in;
    antd_request_t *rq = (antd_request_t *)data;
    antd_client_t* cl = (antd_client_t* ) dvalue(rq->request, "LUA_CL_DATA");
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = PROCESS_TIMEOUT;
    FD_ZERO(&fd_in);
    FD_SET(rq->client->sock, &fd_in);
    FD_SET(cl->sock, &fd_in);
    int max_fdm = rq->client->sock > cl->sock ? rq->client->sock : cl->sock;
    int rc = select(max_fdm + 1, &fd_in, NULL, NULL, &timeout);
    antd_task_t* task;
    uint8_t buff[BUFFLEN];
    int ret;
    switch (rc)
    {
        case -1:
            ERROR("Error on select(): %s", strerror(errno));
            antd_close(cl);
			dput(rq->request, "LUA_CL_DATA", NULL);
            return antd_create_task(NULL, data, NULL, rq->client->last_io);
        case 0:
            // time out
            task = antd_create_task(process, (void *)rq, NULL, time(NULL));
            //antd_task_bind_event(task, rq->client->sock, 0, TASK_EVT_ON_WRITABLE | TASK_EVT_ON_READABLE);
            //antd_task_bind_event(task, cl->sock, 0, TASK_EVT_ON_WRITABLE | TASK_EVT_ON_READABLE);
            return task;
        // we have data
        default:
            // If data is on webserver
            if (FD_ISSET(rq->client->sock, &fd_in))
            {
                while((ret = antd_recv_upto(rq->client,buff, BUFFLEN)) > 0)
                {
                    // write data to the other side
                    if(antd_send(cl,buff, ret) != ret)
                    {
                        ERROR("Error on atnd_send(): %s", strerror(errno));
                        antd_close(cl);
						dput(rq->request, "LUA_CL_DATA", NULL);
                        return antd_create_task(NULL, data, NULL, rq->client->last_io);
                    }
                }
                if(ret < 0)
                {
                    LOG("antd_recv_upto() on %d: %s",rq->client->sock,  strerror(errno));
                    antd_close(cl);
					dput(rq->request, "LUA_CL_DATA", NULL);
                    return antd_create_task(NULL, data, NULL, rq->client->last_io);
                }
            }
            else if(FD_ISSET(cl->sock, &fd_in))
            {
                while((ret = antd_recv_upto(cl,buff, BUFFLEN)) > 0)
                {
                    // write data to the other side
                    if(antd_send(rq->client,buff, ret) != ret)
                    {
                        ERROR("Error atnd_send(): %s", strerror(errno));
                        antd_close(cl);
						dput(rq->request, "LUA_CL_DATA", NULL);
                        return antd_create_task(NULL, data, NULL, rq->client->last_io);
                    }
                }
                if(ret < 0)
                {
                    LOG("antd_recv_upto() on %d: %s", cl->sock, strerror(errno));
                    antd_close(cl);
					dput(rq->request, "LUA_CL_DATA", NULL);
                    return antd_create_task(NULL, data, NULL, rq->client->last_io);
                }
            }
            task = antd_create_task(process, (void *)rq, NULL, time(NULL));
            antd_task_bind_event(task, rq->client->sock, 0, TASK_EVT_ON_WRITABLE | TASK_EVT_ON_READABLE);
            antd_task_bind_event(task, cl->sock, 0, TASK_EVT_ON_WRITABLE | TASK_EVT_ON_READABLE);
            return task;
    }
}

void* handle(void* data)
{
    antd_request_t *rq = (antd_request_t *)data;
    // connect to socket
    int fd = open_unix_socket();
    if(fd < 0)
    {
        antd_error(rq->client, 503, "Service unavailable");
        return antd_create_task(NULL, data, NULL, rq->client->last_io);
    }
    LOG("Connected to lua server at %d", fd);
    set_nonblock(fd);
    // write all header to lua
    antd_client_t* cl = (antd_client_t*) malloc(sizeof(antd_client_t));
    (void)memset(cl, 0, sizeof(antd_client_t));
    cl->sock = fd;
    time(&cl->last_io);
    cl->ssl = NULL;
    cl->state = ANTD_CLIENT_PLUGIN_EXEC;
    cl->z_status = 0;
    cl->z_level = ANTD_CNONE;
    cl->zstream = NULL;
	rq->client->z_level = ANTD_CNONE;
    push_dict_to_socket(cl, "request","HTTP_REQUEST", rq->request);
    antd_send(cl,"\r\n", 2);
    dput(rq->request, "LUA_CL_DATA", cl);
    antd_task_t* task = antd_create_task(process, (void *)rq, NULL, time(NULL));
    antd_task_bind_event(task, rq->client->sock, 0, TASK_EVT_ON_WRITABLE | TASK_EVT_ON_READABLE);
    antd_task_bind_event(task, fd, 0, TASK_EVT_ON_READABLE);
    return task;
}
void destroy()
{
    if(pid > 0)
    {
        kill(pid, SIGHUP);
    }
    LOG("Exit LUA Handle");
}
