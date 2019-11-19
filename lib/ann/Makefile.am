lib_LTLIBRARIES = ann.la
ann_la_LDFLAGS = -module -avoid-version -shared
ann_la_SOURCES =  ann.c
ann_la_CFLAGS = -I./fann/src/include/
ann_la_LIBADD = ./fann/build/src/libdoublefann.a
libdir=$(prefix)/lib/lua/
EXTRA_DIST = data.c extension.c lfann.h net.c fann
SUBDIRS = fann