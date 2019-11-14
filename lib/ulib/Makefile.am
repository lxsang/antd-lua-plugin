lib_LTLIBRARIES = ulib.la
ulib_la_LDFLAGS = -module -avoid-version -shared
ulib_la_SOURCES =  3rd/zip/zip.c ulib.c
# ulib_la_LIBADD = ../../.libs/lua.la
libdir=$(prefix)/lib/lua/
EXTRA_DIST = 3rd/zip/miniz.c 3rd/zip/zip.h

#ulib.c: liblinking

#liblinking:
#	-ln -s ../../.libs/lua.so

#clean-local:
#	-rm ./*.so