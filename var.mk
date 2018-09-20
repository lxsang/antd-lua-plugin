ifeq ($(UNAME_S),Linux)
	FL_LUA=linux
    LUA_LIB=lua-api.dylib libantd.dylib -lm -lcrypt
endif
ifeq ($(UNAME_S),Darwin)
	FL_LUA=macosx
    LUA_LIB= lua-api.dylib libantd.dylib -lm 
endif
LUA_H= -I../../3rd/lua-5.3.4/ 
REAL_PLUGINS_BASE=../../$(PLUGINS_BASE)
LIB_EXT=llib
LIB_BUILD_DIR=$(PBUILDIRD)/lua-api