include ../../var.mk
main:$(LIB_OBJ) $(LIB_NAME)

%.o: %.c
		$(CC) $(LIB_CFLAGS) $(LIB_INC)  -fPIC $(LUA_H) -I$(REAL_PLUGINS_BASE) -c  $< -o $@
		
%.$(LIB_EXT): %.o
		-mkdir $(LIB_BUILD_DIR)
		$(CC) $(LIB_CFLAGS) $(LUA_LIB) -shared -o $(LIB_BUILD_DIR)/$(basename $@).$(LIB_EXT) $(LIB_OBJ) $(LIB_CONF)
		
clean: 
	- rm  $(LIB_BUILD_DIR)/$(LIB_NAME)  *.o *.dylib
.PRECIOUS: %.o
.PHONY: clean
