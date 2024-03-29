#include <antd/ws.h>
#include <antd/base64.h>
#include <poll.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <time.h>
#include <fcntl.h>
#include "../lualib.h"
// add a length field, and
void lua_new_byte_array(lua_State *L, int n)
{
    size_t nbytes = sizeof(byte_array_t) + n * sizeof(unsigned char);
    byte_array_t *a = (byte_array_t *)lua_newuserdata(L, nbytes);
    a->data = &((char *)a)[sizeof(byte_array_t)];
    luaL_getmetatable(L, BYTEARRAY);
    lua_setmetatable(L, -2);
    a->size = n;
}

void lua_new_light_byte_array(lua_State *L, int n, char *ptr)
{
    size_t nbytes = sizeof(byte_array_t);
    byte_array_t *a = (byte_array_t *)lua_newuserdata(L, nbytes);
    a->size = n;
    a->data = ptr;
    luaL_getmetatable(L, BYTEARRAY);
    lua_setmetatable(L, -2);
}

static int l_new_barray(lua_State *L)
{
    int n = luaL_checknumber(L, 1);
    lua_new_byte_array(L, n);
    return 1; /* new userdatum is already on the stack */
}

static int l_new_lightbarray(lua_State *L)
{
    unsigned char *ptr = lua_touserdata(L, 1);
    int n = luaL_checknumber(L, 2);
    lua_new_light_byte_array(L, n, ptr);
    return 1; /* new userdatum is already on the stack */
}

byte_array_t *l_check_barray(lua_State *L, int idx)
{
    void *ud = luaL_checkudata(L, idx, BYTEARRAY);
    luaL_argcheck(L, ud != NULL, idx, "`byte array' expected");
    return (byte_array_t *)ud;
}

static unsigned char *get_bel(lua_State *L)
{
    byte_array_t *a = l_check_barray(L, 1);
    int index = luaL_checknumber(L, 2);
    luaL_argcheck(L, 1 <= index && index <= a->size, 2,
                  "index out of range");

    /* return element address */
    return &a->data[index - 1];
}

static int l_set_barray(lua_State *L)
{
    unsigned char value = luaL_checknumber(L, 3);
    *get_bel(L) = value;
    return 0;
}

static int l_get_barray(lua_State *L)
{
    lua_pushnumber(L, *get_bel(L));
    return 1;
}

static int l_get_barray_size(lua_State *L)
{
    byte_array_t *a = l_check_barray(L, 1);
    lua_pushnumber(L, a->size);
    return 1;
}

static int l_barray_to_string(lua_State *L)
{
    byte_array_t *a = l_check_barray(L, 1);
    char *d = (char *)malloc(a->size + 1);
    memcpy(d, a->data, a->size);
    d[a->size] = '\0';
    lua_pushstring(L, d);
    if (d)
        free(d);
    return 1;
}

static int l_barray_write(lua_State *L)
{
    byte_array_t *a = l_check_barray(L, 1);
    const char *f = luaL_checkstring(L, 2);
    FILE *fp;
    fp = fopen(f, "wb");

    if (!fp)
        lua_pushboolean(L, 0);
    else
    {
        fwrite(a->data, 1, a->size, fp);
        lua_pushboolean(L, 1);
        fclose(fp);
    }
    return 1;
}

static const struct luaL_Reg barraylib[] = {
    {"unew", l_new_lightbarray},
    {"new", l_new_barray},
    {"set", l_set_barray},
    {"get", l_get_barray},
    {"size", l_get_barray_size},
    {"__tostring", l_barray_to_string},
    {"write", l_barray_write},
    {NULL, NULL}};

int luaopen_bytes(lua_State *L)
{
    luaL_newmetatable(L, BYTEARRAY);
    luaL_newlib(L, barraylib);
    lua_pushstring(L, "__index");
    lua_pushstring(L, "get");
    lua_gettable(L, 2); /* get array.get */
    lua_settable(L, 1); /* metatable.__index = array.get */

    lua_pushstring(L, "__newindex");
    lua_pushstring(L, "set");
    lua_gettable(L, 2); /* get array.set */
    lua_settable(L, 1);
    // lua_pushstring(L, "__index");
    // lua_pushvalue(L, -2);  /* pushes the metatable */
    // lua_settable(L, -3);  /* metatable.__index = metatable */
    // luaL_setfuncs(L, barraylib_m, 0);
    // luaL_openlib(L, NULL, barraylib_m, 0);
    // luaL_newlib(L,  barraylib_f);
    return 1;
}

// ARRAY
void lua_new_array(lua_State *L, int n)
{
    size_t nbytes = sizeof(array_t) + (n - 1) * sizeof(double);
    array_t *a = (array_t *)lua_newuserdata(L, nbytes);
    luaL_getmetatable(L, ARRAY);
    lua_setmetatable(L, -2);
    a->size = n;
}
static int l_new_array(lua_State *L)
{
    int n = luaL_checknumber(L, 1);
    lua_new_array(L, n);
    return 1; /* new userdatum is already on the stack */
}

array_t *l_check_array(lua_State *L, int idx)
{
    void *ud = luaL_checkudata(L, idx, ARRAY);
    luaL_argcheck(L, ud != NULL, idx, "`array' expected");
    return (array_t *)ud;
}

static double *get_el(lua_State *L)
{
    array_t *a = l_check_array(L, 1);
    int index = luaL_checknumber(L, 2);
    luaL_argcheck(L, 1 <= index && index <= a->size, 2,
                  "index out of range");

    /* return element address */
    return &a->data[index - 1];
}

static int l_set_array(lua_State *L)
{
    double value = luaL_checknumber(L, 3);
    *get_el(L) = value;
    return 0;
}

static int l_get_array(lua_State *L)
{
    lua_pushnumber(L, *get_el(L));
    return 1;
}

static int l_get_array_size(lua_State *L)
{
    array_t *a = l_check_array(L, 1);
    lua_pushnumber(L, a->size);
    return 1;
}
static int l_array_to_string(lua_State *L)
{
    array_t *a = l_check_array(L, 1);
    lua_pushfstring(L, "number array(%d)", a->size);
    return 1;
}

static const struct luaL_Reg arraylib[] = {
    {"new", l_new_array},
    {"set", l_set_array},
    {"get", l_get_array},
    {"size", l_get_array_size},
    {"__tostring", l_array_to_string},
    {NULL, NULL}};
int luaopen_array(lua_State *L)
{
    luaL_newmetatable(L, ARRAY);
    luaL_newlib(L, arraylib);
    lua_pushstring(L, "__index");
    lua_pushstring(L, "get");
    lua_gettable(L, 2); /* get array.get */
    lua_settable(L, 1); /* metatable.__index = array.get */

    lua_pushstring(L, "__newindex");
    lua_pushstring(L, "set");
    lua_gettable(L, 2); /* get array.set */
    lua_settable(L, 1); /* metatable.__newindex = array.set */

    // lua_pushstring(L, "__index");
    // lua_pushvalue(L, -2);  /* pushes the metatable */
    // lua_settable(L, -3);  /* metatable.__index = metatable */
    // luaL_setfuncs(L, arraylib_m, 0);
    // luaL_openlib(L, NULL, arraylib_m, 0);
    // luaL_newlib(L, arraylib_f);
    return 1;
}

// int mime
static int l_mime(lua_State *L)
{
    const char *file = luaL_checkstring(L, 1);
    lua_pushstring(L, mime(file));
    return 1;
}
// int mime
static int l_ext(lua_State *L)
{
    const char *file = luaL_checkstring(L, 1);
    char *e = ext(file);
    lua_pushstring(L, e);
    if (e)
        free(e);
    return 1;
}
/*
static int l_is_bin(lua_State* L)
{
    const char* file = luaL_checkstring(L,1);
    lua_pushboolean(L, is_bin(file));
    return 1;
}*/
// int __ti(int,int);
/*
static int l_ti (lua_State *L) {
    void* client = lua_touserdata (L, 1);
    int v = (int)luaL_checknumber(L, 2);
    lua_pushnumber(L, __ti(client,v));
    return 1;
}
*/

// int __t(int, const char*,...);
static int l_t(lua_State *L)
{
    void *client = lua_touserdata(L, 1);
    const char *v = luaL_checkstring(L, 2);
    lua_pushnumber(L, __t(client, v));
    return 1; /* number of results */
}

static int l_antd_set_zlevel(lua_State* L)
{
    antd_client_t *client = (antd_client_t *)lua_touserdata(L, 1);
    const char * lvl_str = luaL_checkstring(L, 2);
    if(strncmp(lvl_str, "gzip", 4) == 0)
    {
        client->z_level = ANTD_CGZ;
        return 0;
    }
    if(strncmp(lvl_str,"deflate", 7) == 0)
    {
        client->z_level = ANTD_CDEFL;
    }
    return 0;
}

static int l_antd_recv(lua_State *L)
{
    void *client = lua_touserdata(L, 1);
    int len = 0;
    if (lua_isnumber(L, 2))
    {
        len = (int)luaL_checknumber(L, 2);
    }

    if (len == 0)
    {
        char buff[BUFFLEN * 2] = {0};
        len = read_buf(client, buff, sizeof(buff));
        if (len == 0)
        {
            lua_pushnil(L);
        }
        else
        {
            trim(buff, '\n');
            lua_pushstring(L, buff);
        }
        return 1;
    }
    if (len == 1)
    {
        char c = 0;
        len = antd_recv(client, &c, len);
        lua_pushnumber(L, c);
        return 1;
    }
    else
    {
        lua_new_byte_array(L, len);
        byte_array_t *arr = l_check_barray(L, -1);
        len = antd_recv(client, arr->data, len);
        lua_pushnumber(L, len);
        return 2; /* number of results */
    }
}

// TODO: add __b to LUA
// int __b(int, const unsigned char*, int);
static int l_b(lua_State *L)
{
    void *client = lua_touserdata(L, 1);
    byte_array_t *arr = l_check_barray(L, 2);
    lua_pushnumber(L, __b(client, arr->data, arr->size));
    return 1;
}

// int __f(int, const char*);
static int l_f(lua_State *L)
{
    void *client = lua_touserdata(L, 1);
    const char *v = luaL_checkstring(L, 2);
    lua_pushnumber(L, __f(client, v));
    return 1;
}

// int __fb(int, const char*);
/*static int l_fb (lua_State *L) {
    void* client = lua_touserdata (L, 1);
    const char*  v = luaL_checkstring(L, 2);
    lua_pushnumber(L, __fb(client,v));
    return 1;
}*/

// int upload(const char*, const char*);
static int l_upload(lua_State *L)
{
    const char *s = luaL_checkstring(L, 1);
    const char *d = luaL_checkstring(L, 2);
    lua_pushnumber(L, upload(s, d));
    return 1;
}

// char* route(const char*);
/*
static int l_route (lua_State *L) {
    const char* s = luaL_checkstring(L, 1);
    lua_pushstring(L, route(s));
    return 1;
}*/

// char* htdocs(const char*);
//#ifdef USE_DB
// sqldb getdb();
//#endif
// void set_cookie(int,dictionary);

// void unknow(int);
/*
static int l_unknow (lua_State *L) {
    void* client = lua_touserdata (L, 1);
    unknow(client);
    return 0;
}
*/

static int l_log(lua_State *L)
{
    const char *s = luaL_checkstring(L, 1);
    syslog(LOG_NOTICE, "%s", s);
    return 0;
}

dictionary_t iterate_lua_table(lua_State *L, int index)
{
    // Push another reference to the table on top of the stack (so we know
    // where it is, and this function can work for negative, positive and
    // pseudo indices
    dictionary_t dic = dict();
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
        if (lua_istable(L, -2))
        {
            // the element is a table
            // create new dictionary
            dictionary_t cdic = iterate_lua_table(L, -2);
            dput(dic, key, cdic);
        }
        else
        {
            // other value is converted to string
            const char *value = lua_tostring(L, -2);
            // LOG("%s=%s\n", key, value);
            dput(dic, key, strdup(value));
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

/*
static int l_set_cookie(lua_State* L)
{
    if (lua_istable(L, 3))
    {
        dictionary_t d = iterate_lua_table(L,-2);
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
}*/

static int l_simple_hash(lua_State *L)
{
    const char *s = luaL_checkstring(L, 1);
    lua_pushnumber(L, simple_hash(s));
    return 1;
}
static int l_md5(lua_State *L)
{
    const char *s = luaL_checkstring(L, 1);
    int len = strlen(s);
    char buff[256];
    md5((uint8_t *)s, len, buff);
    lua_pushstring(L, buff);
    return 1;
}
static int l_sha1(lua_State *L)
{
    const char *s = luaL_checkstring(L, 1);
    char buff[80];
    sha1(s, buff);
    lua_pushstring(L, buff);
    return 1;
}
static int l_base64_encode(lua_State *L)
{
    char *s;
    int len;
    char *dst;
    byte_array_t *arr = NULL;
    if (lua_isstring(L, 1))
    {
        s = (char *)luaL_checkstring(L, 1);
        len = strlen(s);
    }
    else
    {
        // this may be an bytearray
        arr = l_check_barray(L, 1);
        s = (char *)arr->data;
        len = arr->size;
    }
    if (len == 0)
    {
        lua_pushstring(L, "");
        return 1;
    }
    dst = (char *)malloc(((len * 4) / 3) + (len / 96) + 6); //(((4 * len / 3) + 3) & ~3)+1
    Base64encode(dst, s, len);
    lua_pushstring(L, dst);
    free(dst);
    return 1;
}
static int l_base64_decode(lua_State *L)
{
    const char *s = luaL_checkstring(L, 1);
    int len = Base64decode_len(s);
    // decode data to a byte array
    lua_new_byte_array(L, len);
    byte_array_t *arr = NULL;
    arr = l_check_barray(L, 2);
    len = Base64decode((char *)arr->data, s);
    arr->size = len;
    // lua_pushstring(L,dst);
    // free(dst);
    return 1;
}
/*
 * web socket handle for lua
 */
static int l_ws_read_header(lua_State *L)
{
    void *client = lua_touserdata(L, 1);
    ws_msg_header_t *header = ws_read_header(client);
    if (!header)
    {
        lua_pushnil(L);
        return 1;
    }
    else
    {
        // create newtable
        lua_newtable(L);

        lua_pushstring(L, "fin");
        lua_pushnumber(L, (int)header->fin);
        lua_settable(L, -3);

        lua_pushstring(L, "opcode");
        lua_pushnumber(L, (int)header->opcode);
        lua_settable(L, -3);

        lua_pushstring(L, "plen");
        lua_pushnumber(L, (int)header->plen);
        lua_settable(L, -3);

        lua_pushstring(L, "mask");
        lua_pushnumber(L, (int)header->mask);
        lua_settable(L, -3);

        lua_pushstring(L, "mask_key");
        lua_newtable(L);

        lua_pushnumber(L, 0);
        lua_pushnumber(L, (int)header->mask_key[0]);
        lua_settable(L, -3);

        lua_pushnumber(L, 1);
        lua_pushnumber(L, (int)header->mask_key[1]);
        lua_settable(L, -3);

        lua_pushnumber(L, 2);
        lua_pushnumber(L, (int)header->mask_key[2]);
        lua_settable(L, -3);

        lua_pushnumber(L, 3);
        lua_pushnumber(L, (int)header->mask_key[3]);
        lua_settable(L, -3);

        lua_settable(L, -3);

        free(header);
        return 1;
    }
}

/*
 * Read socket data given the header
 */
static int l_ws_read_data(lua_State *L)
{
    void *client = lua_touserdata(L, 1);
    if (!lua_istable(L, 2))
    {
        ws_close(client, 1011);
        lua_pushnil(L);
        return 1;
    }
    dictionary_t dic = iterate_lua_table(L, 2);
    if (dic)
    {
        // convert dictionary to header
        ws_msg_header_t *header = (ws_msg_header_t *)malloc(sizeof(*header));
        header->fin = (uint8_t)(R_INT(dic, "fin"));
        header->opcode = (uint8_t)(R_INT(dic, "opcode"));
        header->mask = 1;
        header->plen = R_INT(dic, "plen");
        dictionary_t d1 = (dictionary_t)dvalue(dic, "mask_key");
        if (d1)
        {
            header->mask_key[0] = (uint8_t)(R_INT(d1, "0"));
            header->mask_key[1] = (uint8_t)(R_INT(d1, "1"));
            header->mask_key[2] = (uint8_t)(R_INT(d1, "2"));
            header->mask_key[3] = (uint8_t)(R_INT(d1, "3"));
            freedict(d1);
            dput(dic, "mask_key", NULL);
        }
        freedict(dic);

        // read data
        // This return max 1024 bytes data,
        // user should handle consecutive read
        // by examining the header (FIN bit and plen)
        uint8_t *data = (uint8_t *)malloc(header->plen + 1);
        int size = ws_read_data(client, header, header->plen, data);
        if (size < 0)
        {
            lua_pushnil(L);
            free(header);
            if (data)
                free(data);
            return 1;
        }

        if (header->opcode == WS_TEXT)
        {
            data[size] = '\0';
            lua_pushstring(L, (char *)data);
        }
        else
        {
            // binary data as byte array
            // this is not an optimal way to store byte array
            // TODO: we may need a dedicated byte array type
            // for it
            lua_new_byte_array(L, size);
            byte_array_t *arr = NULL;
            arr = l_check_barray(L, 3);
            memcpy(arr->data, data, size);
            /*lua_newtable(L);
            for (int i = 0; i < size; i++)
            {
                lua_pushnumber(L,i);
                lua_pushnumber(L,(int)(data[i]));
                lua_settable(L,-3);
            }*/
        }
        if (data)
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
static int l_ws_t(lua_State *L)
{
    void *client = lua_touserdata(L, 1);
    char *str = (char *)luaL_checkstring(L, 2);
    ws_t(client, str);
    return 1;
}
/*static int l_status(lua_State*L)
{
    void* client = lua_touserdata (L, 1);
    int code = (int) luaL_checknumber(L,2);
    const char* msg = luaL_checkstring(L,3);
    set_status(client,code,msg);
    return 1;
}*/

/*
 * send a file as binary data
 */
static int l_ws_f(lua_State *L)
{
    void *client = lua_touserdata(L, 1);
    char *str = (char *)luaL_checkstring(L, 2);
    ws_f(client, str);
    return 1;
}
static int l_ws_bin(lua_State *L)
{
    byte_array_t *arr = NULL;
    void *client = lua_touserdata(L, 1);
    arr = l_check_barray(L, 2);
    ws_b(client, arr->data, arr->size);
    return 1;
}
static int l_ws_close(lua_State *L)
{
    void *client = lua_touserdata(L, 1);
    int status = (int)luaL_checknumber(L, 2);
    ws_close(client, status);
    return 1;
}

static int l_trim(lua_State *L)
{
    char *str = strdup((char *)luaL_checkstring(L, 1));
    const char *tok = luaL_checkstring(L, 2);

    trim(str, tok[0]);
    lua_pushstring(L, (const char *)str);
    free(str);
    return 1;
}

static int l_is_dir(lua_State *L)
{
    const char *file = (char *)luaL_checkstring(L, 1);
    lua_pushboolean(L, is_dir(file) == 1);
    return 1;
}

static int l_pfork(lua_State* L)
{
    antd_client_t* client = (antd_client_t*)lua_touserdata(L,1);
    int sock = client->sock;
    // create domain socket
    struct sockaddr_un address;
    address.sun_family = AF_UNIX;
    (void)snprintf(address.sun_path, sizeof(address.sun_path), "/tmp/antd_pfork_%d.sock",sock);
    unlink(address.sun_path);
    int pfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(pfd == -1)
    {
        ERROR( "Unable to create Unix domain socket: %s", strerror(errno));
        lua_pushnumber(L, -1);
        return 1;
    }
    if (bind(pfd, (struct sockaddr *)(&address), sizeof(address)) == -1)
    {
        ERROR("Unable to bind name: %s to a socket: %s", address.sun_path, strerror(errno));
        lua_pushnumber(L, -1);
        return 1;
    }
    // mark the socket as passive mode
    if (listen(pfd, 10) == -1)
    {
        ERROR("Unable to listen to socket: %d (%s): %s", pfd, address.sun_path, strerror(errno));
        lua_pushnumber(L, -1);
        close(pfd);
        return 1;
    }
    LOG( "Socket %s is created successfully", address.sun_path);
    set_nonblock(pfd);

    int pid = fork();
    if(pid == 0)
    {
        int child = socket(AF_UNIX, SOCK_STREAM, 0);
        if(child == -1)
        {
            ERROR( "Unable to create Unix domain socket: %s", strerror(errno));
            lua_pushnumber(L, -1);
            return -1;
        }
        if(connect(child, (struct sockaddr*)(&address), sizeof(address)) == -1)
        {
            ERROR( "Unable to connect to socket '%s': %s", address.sun_path, strerror(errno));
            lua_pushnumber(L, -1);
            close(child);
            return -1;
        }
        client->sock = child;
        client->z_level = ANTD_CNONE;
        lua_pushnumber(L, 0);
    }
    else
    {
        int ret, status;
        struct pollfd pfds[2];
        pfds[0].fd = pfd;
        pfds[0].events = POLLIN;
        ret = poll(pfds, 1, -1);
        if(ret < 0 || (pfds[0].revents & (POLLERR | POLLHUP)))
        {
            ERROR("Unable to wait for child process");
            lua_pushnumber(L, -1);
            close(pfd);
            return 1;
        }
        // now wait for child process
        int cfd = accept(pfd, NULL, NULL);
        if(cfd == -1)
        {
            ERROR("Unable to connect to child process");
            lua_pushnumber(L, -1);
            close(pfd);
            return 1;
        }
        LOG("Child process %d is accessible from %d", pid, cfd);
        set_nonblock(cfd);
        uint8_t buff[BUFFLEN];
        pfds[0].fd = sock;
        pfds[0].events = POLLIN;
        pfds[1].fd = cfd;
        pfds[1].events = POLLIN;
        memset(buff, 0, sizeof(buff));
        while((ret = poll(pfds, 2, 200)) !=  -1)
        {
            if(waitpid(pid, &status, WNOHANG) != 0)
            {
                break;
            }
            if(ret == 0)
            {
                continue;
            }
            if(pfds[0].revents & POLLIN)
            {
                ret = read(client->sock,buff, BUFFLEN);
                if(ret <= 0)
                {
                    LOG("antd_recv_upto() on %d: %s",sock,  strerror(errno));
                    break;
                }
                // write data to the other side
                if(write(cfd,buff, ret) != ret)
                {
                    ERROR("Error on send(): %s", strerror(errno));
                    break;
                }
            }
            else if (pfds[0].revents &(POLLERR | POLLHUP))
            {
                break;
            }

            if(pfds[1].revents & POLLIN)
            {
                ret = read(cfd, buff, sizeof(buff));
                if(ret <= 0)
                {
                    ERROR("error read() on %d: %s",cfd,  strerror(errno));
                    break;
                }
                if(antd_send(client,buff, ret) != ret)
                {
                    ERROR("Error atnd_send(): %s", strerror(errno));
                    break;
                }
            }
            else if (pfds[1].revents &(POLLERR | POLLHUP))
            {
                break;
            }
        }
        close(cfd);
    }
    close(pfd);
    unlink(address.sun_path);
    lua_pushnumber(L, pid);
    return 1;
}

static int l_std_error(lua_State *L)
{
    void *client = lua_touserdata(L, 1);
    int status = luaL_checknumber(L, 2);
    const char *msg = luaL_checkstring(L, 3);
    antd_error(client, status, msg);
    return 1;
}
static int l_send_header(lua_State *L)
{
    if (lua_istable(L, 3))
    {
        dictionary_t d = iterate_lua_table(L, -2);
        if (d)
        {
            void *client = lua_touserdata(L, 1);
            int status = luaL_checknumber(L, 2);
            antd_response_header_t h;
            h.status = status;
            h.header = d;
            dictionary_t c = iterate_lua_table(L, -1);
            h.cookie = NULL;
            if (c)
            {
                h.cookie = list_init();
                if (h.cookie)
                {
                    chain_t it;
                    for_each_assoc(it, c)
                    {
                        list_put_ptr(&h.cookie, strdup(it->value));
                    }
                }
                freedict(c);
            }
            antd_send_header(client, &h);
        }
    }
    return 1;
}

static const struct luaL_Reg standard[] = {
    //{"_header", l_header},
    //{"_redirect", l_redirect},
    //{"_html", l_html},
    //{"_text", l_text},
    //{"_json", l_json},
    //{"_jpeg", l_jpeg},
    {"antd_set_zlevel", l_antd_set_zlevel},
    {"antd_recv", l_antd_recv},
    {"_error", l_std_error},
    {"_send_header", l_send_header},
    {"b64encode", l_base64_encode},
    {"b64decode", l_base64_decode},
    //{"_octstream", l_octstream} ,
    //{"_textstream", l_textstream} ,
    //{"_ti", l_ti} ,
    {"_t", l_t},
    {"_f", l_f},
    {"_b", l_b},
    {"trim", l_trim},
    {"upload", l_upload},
    //{"route", l_route} ,
    {"mime", l_mime},
    //{"is_bin", l_is_bin} ,
    //{"_unknow", l_unknow} ,
    //{"_status", l_status},
    {"console", l_log},
    //{"_setCookie", l_set_cookie},
    {"hash", l_simple_hash},
    {"md5", l_md5},
    {"sha1", l_sha1},
    {"ext", l_ext},
    {"ws_header", l_ws_read_header},
    {"ws_read", l_ws_read_data},
    {"ws_f", l_ws_f},
    {"ws_t", l_ws_t},
    {"ws_b", l_ws_bin},
    {"ws_close", l_ws_close},
    {"is_dir", l_is_dir},
    {"pfork", l_pfork},
    {NULL, NULL}};

int luaopen_std(lua_State *L)
{
    luaL_newlib(L, standard);
    return 1;
}

static const struct luaL_Reg modules[] = {
    {"bytes", luaopen_bytes},
    {"array", luaopen_array},
    {"std", luaopen_std},
    {NULL, NULL}};

int luaopen_antd(lua_State *L)
{
    luaL_newlib(L, modules);
    lua_setglobal(L, "modules");
    return 1;
}
