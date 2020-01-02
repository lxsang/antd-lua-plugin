#include <antd/dbhelper.h>
#include "../lualib.h"
typedef sqlite3* sqldb;
//sqldb db = NULL;

static int l_getdb (lua_State *L) {
	const char* s = luaL_checkstring(L,1);
//	if(db)
//		dbclose(db);
	//printf("OPEN: %s\n",s);
	sqldb db = (sqldb)database(s);
	if(db)
		lua_pushlightuserdata(L, db);
	else
		lua_pushnil(L);
	return 1;  /* number of results */
}
static int l_db_close(lua_State *L)
{
	sqldb db = (sqldb) lua_touserdata(L, 1);
	if(db)
	{
		//printf("close database\n");
		dbclose(db);
	}
	db = NULL;
	return 0;
}
static int l_db_query(lua_State *L)
{
	sqldb db = (sqldb) lua_touserdata(L, 1);
	const char* s = luaL_checkstring(L,2);
	int r = 0;
	if(db)
		r = dbquery(db,s,NULL);
	lua_pushnumber(L,r);
	return 1;
}
static int l_db_lastid(lua_State *L)
{
	sqldb db = (sqldb) lua_touserdata(L, 1);
	
	int idx = -1;
	if(db)
		idx = sqlite3_last_insert_rowid(db);
	lua_pushnumber(L,idx);
	return 1;
}
static int l_db_select(lua_State *L)
{
	sqldb db = (sqldb) lua_touserdata(L, 1);
	const char* tbl = luaL_checkstring(L,2);
	const char* fname = luaL_checkstring(L,3);
	const char* cond = luaL_checkstring(L,4);
	if(!db)
	{
		lua_pushnil(L);
		return 1;
	}
	dbrecord records = dbselect(db,tbl, fname,cond);
	int cnt = 1;
	//new table for data
	lua_newtable(L);
	for(dbrecord r = records;r != NULL; r=r->next)
	{
	 	dbfield row = r-> fields;
		if(row)
		{
			lua_pushnumber(L,cnt);
			lua_newtable(L);
	 		for(dbfield c = row; c != NULL; c= c->next)
			{
				if(c->name)
				{
					//LOG("%s->%s",c->name,c->value);
					lua_pushstring(L,c->name);
					lua_pushstring(L,c->value);
					lua_settable(L, -3);
				}
			}
	 		//free(row);
			lua_settable(L, -3);
	 		cnt++;
		}
	 }
	 if(records)
		 freerecord(&records);
	 return 1; 
}
static int l_hastable(lua_State *L)
{
	sqldb db = (sqldb) lua_touserdata(L, 1);
	const char* tbl = luaL_checkstring(L,2);
	if(db)
		lua_pushnumber(L,hastable(db,tbl));
	else
		lua_pushnumber(L,0);
	return 1;
}
static const struct luaL_Reg sqlite [] = {
       {"_getdb", l_getdb},
	   {"dbclose", l_db_close},
	   {"query", l_db_query},
	   {"lastInsertID", l_db_lastid},
	   {"select", l_db_select},
	   {"hasTable", l_hastable},
	   {NULL,NULL}  
};

int luaopen_sqlitedb(lua_State *L)
{
	luaL_newlib(L, sqlite);
	return 1;
}