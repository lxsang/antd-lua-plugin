include ../../var.mk
include var.mk

PL_NAME=lua-api
PLUGINS=$(PL_NAME).$(EXT)
LLIBS=wurl.llib ulib.llib  ann.llib stmr.llib #pibot.llib
 
OBJS = 		$(PLUGINS_BASE)/plugin.o 

PLUGINSDEP = 	$(OBJS) \
				$(WRAPI) \
				plugin-wrapper.o\
				3rd/jsmn/jsmn.o \
				array-wrapper.o \
				json-wrapper.o \
				db-wrapper.o
		
PLUGINLIBS = -lm -lpthread -lsqlite3 libantd.$(EXT)
				
PCFLAGS=-W -Wall -g -std=c99 -D DEBUG  $(PPF_FLAG) -D USE_DB
main: lua $(PLUGINSDEP)  $(PLUGINS) api lib
lua:
	cd 3rd/lua-5.3.4 && CC=$(CC) make $(FL_LUA)
%.o: %.c
		$(CC) $(PCFLAGS) -fPIC $(INCFLAG) -c  $< -o $@

%.$(EXT): %.o
		-ln -s $(PBUILDIRD)/libantd.$(EXT) .
		$(CC) $(PCFLAGS) $(PLUGINLIBS) -shared -o $(PBUILDIRD)/$(basename $@).$(EXT) \
		$(PLUGINSDEP) $(basename $@).o 3rd/lua-5.3.4/liblua.a

deepclean: luaclean clean

clean: libclean
		-rm -f *.o 3rd/jsmn/*.o   *.$(EXT) $(PBUILDIRD)/$(PLUGINS) 
		- rm ./libantd.$(EXT)
		-rm $(PLUGINS_BASE)/plugin.o
		-rm $(PBUILDIRD)/$(PL_NAME)/*.$(LIB_EXT)

libclean:
	for file in lib/* ;do \
		if [ -d "$$file" ]; then \
			echo "Cleaning $$file" ;\
			make -C  "$$file" clean; \
		fi \
	done
		
luaclean:
	- cd 3rd/lua-5.3.4 && make clean
	
lib:$(LLIBS)
	
%.llib:
	-ln -s $(PBUILDIRD)/libantd.$(EXT) lib/$(basename $@)
	-ln -s $(PBUILDIRD)/$(PLUGINS) lib/$(basename $@)
	-cd lib/$(basename $@) && make

api:
	-mkdir $(PBUILDIRD)/$(PL_NAME)
	cp APIs/*.lua $(PBUILDIRD)/$(PL_NAME)
app:
	cp -rf example-app/* $(APP_DIR)
.PRECIOUS: %.o
.PHONY: lib clean
full: clean main



