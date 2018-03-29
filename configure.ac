dnl Autoconf settings for LZSS

AC_INIT([LZSS], [0.1], [victorien.lecouviour.tuffet@gmail.com], [], [https://en.wikipedia.org/wiki/Lempel%E2%80%93Ziv%E2%80%93Storer%E2%80%93Szymanski])
AM_INIT_AUTOMAKE([foreign subdir-objects])

dnl
dnl  Check for tools
dnl

: ${CXXFLAGS=""}
AC_PROG_CXX

AM_PROG_AR

dnl
dnl  Libtool
dnl

LT_INIT

dnl
dnl  Check for compiler properties
dnl

AC_C_INLINE
AX_CXX_COMPILE_STDCXX([17], [noext], [mandatory])

dnl
dnl  Compiler flags
dnl

AX_APPEND_COMPILE_FLAGS([-Wall -Werror -Wextra], [CXXFLAGS])
AX_APPEND_LINK_FLAGS([-rdynamic], [LDFLAGS])

dnl
dnl  Package options
dnl

AC_ARG_ENABLE([debug],
              AS_HELP_STRING([--enable-debug], [enable debugging (default is no)]),
              [case "${enableval}" in
                    yes)    debug=true ;;
                    no)     debug=false ;;
                    *)      AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
               esac],
               [debug=false])
AM_CONDITIONAL([DEBUG], [test x"$debug" = x"true"])
AM_COND_IF([DEBUG], [CPPFLAGS="-DDEBUG"
                     AX_APPEND_COMPILE_FLAGS([-g3 -O0], [CXXFLAGS])],
                    [AX_APPEND_COMPILE_FLAGS([-O3], [CXXFLAGS])])

dnl
dnl  Configuration done
dnl

AC_CONFIG_MACRO_DIRS([m4])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
    Makefile
    codec/Makefile
])

dnl
dnl  Generate makefiles
dnl

AC_OUTPUT
