# initialise autoconf and set up some basic information about the program we’re packaging
AC_INIT([lua], [0.5.2b], [xsang.le@gmail.com])

# We’re going to use automake for this project
# [subdir-objects] if needed
AM_INIT_AUTOMAKE([subdir-objects])

# custom definition

AC_DEFUN(AC_PROG_CMAKE, [AC_CHECK_PROG(has_cmake,cmake,yes)])
AC_PROG_CMAKE

# dependencies
# C compiler
AC_PROG_CC
# libtool for linking
AC_PROG_LIBTOOL

has_antd=no
# check for lib antd
AC_CHECK_HEADER([antd/plugin.h],[
    has_antd=yes
    # check if the library exists
],[
    AC_MSG_ERROR([Unable to find antd, please install it first])
])
AC_CHECK_LIB([antd],[antd_send],[],[
    if test "$has_antd" = "yes"; then
        AC_MSG_ERROR([Unable to find antd shared library, please install it first])
    fi
])

# check if sqlite3 header exists
use_db=no
AC_CHECK_HEADER([sqlite3.h],[
    AC_DEFINE([USE_DB], [1],[Use sqlite3])
    use_db=yes
    # check if the library exists
],[])
AC_CHECK_LIB([sqlite3],[sqlite3_open],[],[
    if test "$use_db" = "yes"; then
        AC_MSG_ERROR([Unable to find sqlite3 shared library])
    fi
])

# check for pthread
AC_CHECK_LIB([pthread], [pthread_create], [],  [
    AC_MSG_ERROR([libpthread is not found])])

# check for dl
AC_CHECK_LIB([dl], [dlopen], [], [
  AC_MSG_ERROR([unable to find libdl])
])

# check for lm
AC_CHECK_LIB([m],[cos],[],[
    AC_MSG_ERROR([unable to find libm])
])

# check for libreadline
AC_CHECK_HEADER([readline/readline.h],[],[
    AC_MSG_ERROR([unable to find libreadline headers])
])

AC_CHECK_LIB([readline],[read_history],[],[
    AC_MSG_ERROR([unable to find libreadline])
])

# check for libffi using by lua module
use_ffi=no
AC_CHECK_HEADER([ffi.h],[
    AC_CHECK_LIB([ffi],[ffi_prep_cif],[
        use_ffi=yes
    ],[])
],[])

AC_DEFINE([_GNU_SOURCE], [1],[Use GNU source])
# AC_CANONICAL_HOST is needed to access the 'host_os' variable    

# debug option
AC_ARG_ENABLE([debug],
    [  --enable-debug    Turn on debugging],
    [case "${enableval}" in
        yes) AC_DEFINE([DEBUG], [1],[Enable debug]) ;;
        no) ;;
        *) AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
    esac],[debug=false])

AC_CANONICAL_HOST
build_linux=no
build_windows=no
build_mac=no
# Detect the target system
case "${host_os}" in
    linux*)
        AC_DEFINE([LINUX], [1],[Linux system])
        build_linux=yes
        ;;
    darwin*)
        build_mac=yes
        AC_DEFINE([MACOS], [1],[MacOS system])
        ;;
    *)
        AC_MSG_ERROR(["OS $host_os is not supported"])
        ;;
esac

if test "$build_linux" = "yes"; then
    AC_CHECK_LIB([crypt],[crypt],[],[
        AC_MSG_ERROR([unable to find libcrypt])
    ])
fi

# case for window:
# cygwin*|mingw*)
#    build_windows=yes
#    ;;
# Pass the conditionals to automake
AM_CONDITIONAL([DB], [test "$use_db" = "yes"])
AM_CONDITIONAL([LINUX], [test "$build_linux" = "yes"])
AM_CONDITIONAL([WINDOWS], [test "$build_windows" = "yes"])
AM_CONDITIONAL([OSX], [test "$build_mac" = "yes"])
AM_CONDITIONAL([HAS_FFI], [test "$use_ffi" = "yes"])
AM_CONDITIONAL([HAS_CMAKE], [test "$has_cmake" = "yes"])
# find all config files
AC_CONFIG_FILES([
    Makefile 
    lib/Makefile
    lib/core/Makefile
    lib/asl/Makefile
    lib/md/Makefile
])

#if test x"${has_cmake}" == x"yes" ; then
#    AC_CONFIG_FILES([lib/ann/Makefile lib/ann/fann/Makefile])
#fi

# AC_SUBST([my_CPPFLAGS]) pass my_CPPFLAGS to the makefile.am
# output the script:
AC_OUTPUT
