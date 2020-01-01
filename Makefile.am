AUTOMAKE_OPTIONS = foreign



# check for system
if LINUX
    AM_CPPFLAGS = -Wl,--no-as-needed
    FL_LUA=linux
else
    AM_CPPFLAGS = -Wl,-undefined,dynamic_lookup
    FL_LUA=macosx
endif


AM_CPPFLAGS += -W  -Wall -g -std=c99 -fPIC
# main plugin
lib_LTLIBRARIES = lua.la
lua_la_LDFLAGS = -module -avoid-version -shared
lua_la_SOURCES =  plugin-wrapper.c\
				3rd/jsmn/jsmn.c \
				array-wrapper.c \
				json-wrapper.c \
                lua-api.c

# lua libraris &  modules
SUBDIRS = 3rd . lib

if DB
    lua_la_SOURCES += db-wrapper.c
endif

EXTRA_DIST = README.md 3rd/jsmn/jsmn.h APIs lua-api.h

install-data-local:
	mkdir -p $(prefix)/lib/lua
	cp -v APIs/* $(prefix)/lib/lua
	cp -v 3rd/lua-5.3.4/core.so $(prefix)/lib/lua