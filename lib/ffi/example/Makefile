CC = gcc
main: lib.o
	$(CC) -shared lib.o -o libtest.so
	
%.o: %.c
	$(CC)  -fPIC -c $< -o $@
