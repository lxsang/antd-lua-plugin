lib_LTLIBRARIES = ulib.la
ulib_la_LDFLAGS = -module -avoid-version -shared
ulib_la_SOURCES =  3rd/zip/zip.c ulib.c

lib_LTLIBRARIES += handle.la
handle_la_LDFLAGS = -module -avoid-version -shared
handle_la_SOURCES =  handle.c

lib_LTLIBRARIES += antd.la
antd_la_LDFLAGS = -module -avoid-version -shared
antd_la_SOURCES =  antd.c

lib_LTLIBRARIES += stmr.la
stmr_la_LDFLAGS = -module -avoid-version -shared
stmr_la_SOURCES =  stmr.c

lib_LTLIBRARIES += json.la
json_la_LDFLAGS = -module -avoid-version -shared
json_la_SOURCES =  3rd/jsmn/jsmn.c json.c


lib_LTLIBRARIES += wurl.la
wurl_la_LDFLAGS = -module -avoid-version -shared
wurl_la_SOURCES =  wurl.c

if HAS_FFI
lib_LTLIBRARIES += ffi.la
ffi_la_LDFLAGS = -module -avoid-version -shared -lffi
ffi_la_SOURCES =   ffi.c
endif

if DB
lib_LTLIBRARIES += sqlitedb.la
sqlitedb_la_LDFLAGS = -module -avoid-version -shared
sqlitedb_la_SOURCES =   sqlitedb.c
endif



libdir=$(prefix)/lib/lua/

EXTRA_DIST = 3rd/zip/miniz.c 3rd/zip/zip.h 3rd/jsmn/jsmn.h