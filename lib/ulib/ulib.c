#ifdef LINUX
 #include <unistd.h>
 #include <limits.h>
 #include <pwd.h>
 #include <shadow.h>
#endif
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <ctype.h>
#include "../lualib.h"
#include "utils.h"
// zip library
#include "3rd/zip/zip.h"

static int l_check_login (lua_State *L) {
#ifdef LINUX
	const char* username = luaL_checkstring(L,1);
	const char* password = luaL_checkstring(L,2);
	
    char *encrypted;
    struct passwd *pwd;
    struct spwd *spwd;
   /* Look up password and shadow password records for username */
    pwd = getpwnam(username);
    if (pwd == NULL)
    {
    	lua_pushboolean(L,0);
		printf("Cannot find pwd record of %s\n", username );
		return 1;
    }
    spwd = getspnam(username);
    if (spwd == NULL)
	{
		lua_pushboolean(L,0);
        printf("no permission to read shadow password file\n");
		return 1;
	}

    if (spwd != NULL)           /* If there is a shadow password record */
	{
        pwd->pw_passwd = spwd->sp_pwdp;     /* Use the shadow password */
	}
	else
	{
    	lua_pushboolean(L,0);
		printf("shadow record is null \n" );
		return 1;
	}
    /* Encrypt password and erase cleartext version immediately */
    encrypted = crypt(password, pwd->pw_passwd);
    if (encrypted == NULL)
	{
    	lua_pushboolean(L,0);
		printf("Cant crypt \n" );
		return 1;
	}
    if(strcmp(encrypted, pwd->pw_passwd) == 0)
	{
    	lua_pushboolean(L,1);
		printf("%s\n","Successful login" );
		return 1;
	} else
	{
    	lua_pushboolean(L,0);
		printf("Password incorrect \n" );
		return 1;
    }
#else
	// macos
	// just pass the check, for test only
	lua_pushboolean(L,1);
	printf("%s\n","Successful login" );
	return 1;
#endif
}
static void get_userId(const char* name, uid_t* uid,gid_t* gid )
{
    struct passwd *pwd;
    uid_t u;
    char *endptr;
	if (name == NULL || *name == '\0')
	{
		*uid = -1; *gid=-1;
		return;
	}
	u = strtol(name, &endptr, 10);
	if (*endptr == '\0')
	{
		*uid = u;
		*gid = -1;
		return;
	}
	pwd = getpwnam(name);
	if (pwd == NULL)
	{
		*uid = -1; *gid=-1;
		return;
	}
	 *uid = pwd->pw_uid;
	 *gid = pwd->pw_gid;
}
static int l_fork(lua_State* L)
{
	int pid = fork();
	lua_pushnumber(L, pid);
	return 1;
}
static int l_waitpid(lua_State* L)
{
	int pid = luaL_checknumber(L,1);
	int status;
    waitpid(pid, &status, 0);
	lua_pushnumber(L, status);
	return 1;
}
static int l_kill(lua_State* L)
{
	int pid = luaL_checknumber(L,1);
	if(pid == -1) pid = getpid();
	int status = kill(pid, SIGKILL);
	lua_pushnumber(L, status);
	return 1;
}
static int l_setuid(lua_State* L)
{
	uid_t uid = (uid_t) luaL_checknumber(L,1);
	if((int)uid != -1)
	{
		if(seteuid(uid) < 0)
		{
			printf("UID set problem: %s\n", strerror(errno));
			lua_pushboolean(L,0);
		}
		else
		{
			//printf("UID set\n");
			lua_pushboolean(L,1);
		}
	}
	else
		lua_pushboolean(L,0);
	return 1;
}
static int l_setgid(lua_State* L)
{
	uid_t gid = (uid_t) luaL_checknumber(L,1);
	if((int)gid != -1)
	{
		if(setegid(gid) < 0)
		{
			printf("GID set problem: %s\n", strerror(errno));
			lua_pushboolean(L,0);
		}
		else
		{
			//printf("GID set\n");
			lua_pushboolean(L,1);
		}
	}
	else
		lua_pushboolean(L,0);
	return 1;
}
static int l_getuid(lua_State* L)
{
	const char* name = luaL_checkstring(L,1);
	uid_t uid = -1;
	uid_t gid = -1;
	get_userId(name,&uid,&gid);
	lua_newtable(L);
	
	lua_pushstring(L,"id");
	lua_pushnumber(L,uid);
	lua_settable(L,-3);
	
	lua_pushstring(L,"gid");
	lua_pushnumber(L,gid);
	lua_settable(L,-3);

	int j, ngroups = 30; //only first 10 group
    gid_t *groups;
	struct group *gr;
	// find all the groups
	if((int)uid != -1 && (int)gid != -1)
	{
		 /* Retrieve group list */
		groups = malloc(ngroups * sizeof (gid_t));
           if (groups == NULL) {
               LOG("malloc eror \n");
               return 1;
           }
		if (getgrouplist(name, gid, groups, &ngroups) == -1) {
			free(groups);
			LOG("getgrouplist() returned -1; ngroups = %d\n", ngroups);
			return 1;
		}
		/* retrieved groups, along with group names */
		lua_pushstring(L,"groups");
		lua_newtable(L);

		for (j = 0; j < ngroups; j++) {
			gr = getgrgid(groups[j]);
			if (gr != NULL)
			{
				//printf("%d: (%s)\n", groups[j],gr->gr_name);
				lua_pushnumber(L, groups[j]);
				lua_pushstring(L, gr->gr_name);
				lua_settable(L,-3);
			}
		}
		lua_settable(L, -3);
		free(groups);
	}
	return 1;
}


static int l_file_stat(lua_State* L, const char* path)
{
	//const char* path = luaL_checkstring(L,-1);
	//printf("PATH %s\n", path);
	//lua_pop(L,1);
	char date[64];
	struct stat st;
	if( stat(path, &st) == 0 )
	{
		// recore for file
		lua_newtable(L);
		
		//type
		lua_pushstring(L,"type");
		if(S_ISDIR(st.st_mode))
			lua_pushstring(L,"dir");
		else
			lua_pushstring(L,"file");
		lua_settable(L,-3);
		
		//ctime
		lua_pushstring(L,"ctime");
		strftime(date, sizeof(date), "%Y-%m-%dT%H:%M:%S", localtime(&(st.st_ctime)));
		lua_pushstring(L,date);
		lua_settable(L,-3);
		//mtime
		
		lua_pushstring(L,"mtime");
		strftime(date, sizeof(date), "%Y-%m-%dT%H:%M:%S", localtime(&(st.st_mtime)));
		lua_pushstring(L,date);
		lua_settable(L,-3);
		
		//size
		lua_pushstring(L,"size");
		lua_pushnumber(L,st.st_size);
		lua_settable(L,-3);
		//mime
		if(S_ISDIR(st.st_mode))
		{
			lua_pushstring(L,"mime");
			lua_pushstring(L, "dir");
			lua_settable(L,-3);
		}
		else
		{
			lua_pushstring(L,"mime");
			lua_pushstring(L, mime(path));
			lua_settable(L,-3);
		}
		
		// uid
		lua_pushstring(L,"uid");
		lua_pushnumber(L,st.st_uid);
		lua_settable(L,-3);
		// gid
		lua_pushstring(L,"gid");
		lua_pushnumber(L,st.st_gid);
		lua_settable(L,-3);
		
		lua_pushstring(L,"permissions");
		sprintf(date," (%3o)", st.st_mode&0777);
		lua_pushstring(L,date);
		lua_settable(L,-3);
		
		// permission
		lua_pushstring(L,"perm");
		lua_newtable(L);
		
		lua_pushstring(L,"owner");
		lua_newtable(L);
		lua_pushstring(L,"read");
		lua_pushboolean(L, (st.st_mode & S_IRUSR)==0?0:1);
		lua_settable(L,-3);
		lua_pushstring(L,"write");
		lua_pushboolean(L, (st.st_mode & S_IWUSR)==0?0:1);
		lua_settable(L,-3);
		lua_pushstring(L,"exec");
		lua_pushboolean(L, (st.st_mode & S_IXUSR)==0?0:1);
		lua_settable(L,-3);
		//end owner
		lua_settable(L,-3);
		
		lua_pushstring(L,"group");
		lua_newtable(L);
		lua_pushstring(L,"read");
		lua_pushboolean(L, (st.st_mode & S_IRGRP)==0?0:1);
		lua_settable(L,-3);
		lua_pushstring(L,"write");
		lua_pushboolean(L, (st.st_mode & S_IWGRP)==0?0:1);
		lua_settable(L,-3);
		lua_pushstring(L,"exec");
		lua_pushboolean(L, (st.st_mode & S_IXGRP)==0?0:1);
		lua_settable(L,-3);
		//end owner
		lua_settable(L,-3);
		
		lua_pushstring(L,"other");
		lua_newtable(L);
		lua_pushstring(L,"read");
		lua_pushboolean(L, (st.st_mode & S_IROTH)==0?0:1);
		lua_settable(L,-3);
		lua_pushstring(L,"write");
		lua_pushboolean(L, (st.st_mode & S_IWOTH)==0?0:1);
		lua_settable(L,-3);
		lua_pushstring(L,"exec");
		lua_pushboolean(L, (st.st_mode & S_IXOTH)==0?0:1);
		lua_settable(L,-3);
		//end owner
		lua_settable(L,-3);
		
		// end permission
		lua_settable(L,-3);
	}
	else
	{
		lua_newtable(L);
	}
	return 1;
}

static int l_file_info(lua_State* L)
{
	const char* path = luaL_checkstring(L,1);
	l_file_stat(L,path);
	return 1;
}

static int l_file_move(lua_State* L)
{
	const char* old = luaL_checkstring(L,1);
	const char* new = luaL_checkstring(L,2);
	//printf("MOVE  %s to %s\n",old,new);
	lua_pushboolean(L, rename(old,new)==0);
	return 1;
}

static int l_read_dir(lua_State* L)
{
	const char* path = luaL_checkstring(L,1);
	const char* prefix = luaL_checkstring(L,2);
	DIR           *d;
  	struct dirent *dir;
	d = opendir(path);
	char buff[1024];
	lua_newtable(L);
  	if (d)
  	{
		int id=0;
   		while ((dir = readdir(d)) != NULL)
    	{
    		//ignore curent directory, parent directory 
    		if(strcmp(dir->d_name,".") == 0 || 
    			strcmp(dir->d_name,"..")==0/*|| *(dir->d_name)=='.'*/) continue;
			sprintf(buff,"%s/%s",path,dir->d_name);
			lua_pushnumber(L,id);
			//lua_pushstring(L,buff);
			l_file_stat(L,buff);
			
			lua_pushstring(L,"filename");
			lua_pushstring(L,dir->d_name);
			lua_settable(L,-3);
			
			sprintf(buff,"%s/%s",prefix,dir->d_name);
			//printf("FILE %s\n",buff );
			lua_pushstring(L,"path");
			lua_pushstring(L,buff);
			lua_settable(L,-3);
			
			lua_settable(L,-3);
			
      		id++;
    	}
    	closedir(d);
	}
	else
	{
		lua_pushstring(L,"error");
		lua_pushstring(L,"Resource not found");
		lua_settable(L,-3);
	}
	return 1;
}

static int l_chown(lua_State* L)
{
	const char* path = luaL_checkstring(L,1);
	int id = luaL_checknumber(L,2);
	int gid = luaL_checknumber(L,3);
	lua_pushboolean(L, chown(path,id,gid) == 0);
	return 1;
}

static int l_mkdir(lua_State* L)
{
	const char* path = luaL_checkstring(L,1);
	lua_pushboolean(L, mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0);
	return 1;
}
static int l_exist(lua_State* L)
{
	const char* path = luaL_checkstring(L,1);
	lua_pushboolean(L,access( path, F_OK ) != -1 );
	return 1;
}
static int _recursive_delete(const char* path)
{
	if(is_dir(path) == 1)
	{
		DIR           *d;
	  	struct dirent *dir;
		d = opendir(path);
		char buff[1024];
	  	if (d)
	  	{
	   		while ((dir = readdir(d)) != NULL)
	    	{
	    		//ignore current & parent dir
	    		if(strcmp(dir->d_name,".") == 0 || strcmp(dir->d_name,"..")==0) continue;
				// get the file
				sprintf(buff,"%s/%s",path,dir->d_name);
				if(_recursive_delete(buff) == -1) return -1;
			}
			closedir(d);
			// remove dir
			if(rmdir(path) != 0) return -1;
		} else return -1;
	}
	else
	{
		// file remove
		if(remove(path) != 0) return -1;
	}
	return 0;
}
static int l_delete(lua_State* L)
{
	const char* f = luaL_checkstring(L,1);
	lua_pushboolean(L,_recursive_delete(f) == 0);
	return 1;
}
/*
* Only use this for simple command
* Be careful to expose this function
* to user. This can cause a serious 
* security problem
*/
static int l_cmd_open(lua_State* L)
{
	  FILE *fp;
	  const char* cmd = luaL_checkstring(L,1);

	  /* Open the command for reading. */
	  fp = popen(cmd, "r");
	  if (fp == NULL) {
	  	lua_pushnil(L);
		return 1;
	  }
	  /*return the fd to lua
	  * user can use this to 
	  read the output data*/
	  intptr_t pt = (intptr_t)fp;
	  lua_pushnumber(L, pt);
	  return 1;
}
static int l_cmd_read(lua_State* L)
{
  	/* Read the output a line at a time - output it.
		read user defined data of std
	 */
	FILE * fd;
	intptr_t pt = (intptr_t)luaL_checknumber(L, 1);
	fd = (FILE*)pt;
	if(fd == NULL)
	{
		lua_pushnil(L);
		return 1;
	}
	char buff[1024];
  	if(fgets(buff, sizeof(buff)-1, fd) != NULL) {
    	lua_pushstring(L,buff);
  	} else
	{
		lua_pushnil(L);
	}
  /* close */
  //pclose(fp);

  return 1;
}
static int l_cmd_close(lua_State* L)
{
  	/* Read the output a line at a time - output it.
		read user defined data of std
	 */
	FILE * fd;
	intptr_t pt = (intptr_t)luaL_checknumber(L, 1);
	fd = (FILE*)pt;
	if(fd == NULL)
	{
		pclose(fd);
	}
	return 0;
}
/*zip file handler 
* using miniz to compress and uncompress zip file
*/

static int l_unzip(lua_State* L)
{
	const char* src = luaL_checkstring(L,1);
	const char* dest = luaL_checkstring(L,2);
	
	if(zip_extract(src, dest, NULL, NULL) == -1)
	{
		lua_pushboolean(L,0);
		return 1;
	}
	lua_pushboolean(L,1);
	return 1;
}

static int _add_to_zip(struct zip_t * zip, const char* path, const char* root)
{
	int st = is_dir(path);
	if(st == -1) return -1;
	if(st)
	{
		// read directory
		DIR           *d;
	  	struct dirent *dir;
	  	//struct stat st;
	  	d = opendir(path);
	  	if (d)
	  	{
	   		while ((dir = readdir(d)) != NULL)
	    	{
	    		//ignore curent directory, parent directory 
	    		if(strcmp(dir->d_name,".") == 0 || strcmp(dir->d_name,"..")==0) continue;
				// add file to zip
				char* p1 = __s("%s/%s", path, dir->d_name);
				char* p2 =  __s("%s/%s",root,dir->d_name);
				if(_add_to_zip(zip,p1,p2) == -1)
				{
					free(p1);
					free(p2);
					return -1;
				}
				free(p1);
				free(p2);
			}
			closedir(d);
		}
	}
	else
	{
		// if it is a file
		// add it to zip
	    if(zip_entry_open(zip, root) == -1) return -1;
	    if(zip_entry_fwrite(zip, path) == -1) return -1;
	    zip_entry_close(zip);
	}
	return 0;
}

static int l_zip(lua_State* L)
{
	const char* src = luaL_checkstring(L,1);
	const char* dest = luaL_checkstring(L,2);
	
	// create a zip handler
    struct zip_t *zip = zip_open(dest, ZIP_DEFAULT_COMPRESSION_LEVEL, 0);
    
	if(zip == NULL) goto pfalse;

    if(_add_to_zip(zip,src,"") == -1) goto pfalse;
	       
    zip_close(zip);
	
	lua_pushboolean(L,1);
	return 1;
	
	// return false
	pfalse:
	// TODO remove if filed is created
	if(zip) zip_close(zip);
	lua_pushboolean(L,0);
	return 1;
}

static const struct luaL_Reg _lib [] = {
	{"auth", l_check_login},
	{"read_dir",l_read_dir},
	{"file_stat",l_file_info},
	{"uid",l_getuid},
	{"setuid", l_setuid},
	{"setgid", l_setgid},
	{"fork", l_fork},
	{"kill", l_kill},
	{"waitpid", l_waitpid},
	{"chown",l_chown},
	{"delete", l_delete},
	{"exists",l_exist},
	{"mkdir",l_mkdir},
	{"cmdopen",l_cmd_open},
	{"cmdread",l_cmd_read},
	{"cmdclose",l_cmd_close},
	{"move",l_file_move},
	{"unzip",l_unzip},
	{"zip",l_zip},
	{NULL,NULL}
};

int luaopen_ulib(lua_State *L)
{
	luaL_newlib(L, _lib);
	return 1;
}
