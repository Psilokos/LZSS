dnl Autoconf settings for LZSS

AC_INIT([LZSS], [0.1], [victorien.lecouviour.tuffet@gmail.com], [], [https://en.wikipedia.org/wiki/Lempel%E2%80%93Ziv%E2%80%93Storer%E2%80%93Szymanski])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])

dnl
dnl  Check for tools
dnl

AC_PROG_CXX

dnl
dnl  Check for compiler properties
dnl

AC_C_INLINE

dnl
dnl  Compiler flags
dnl

AX_APPEND_COMPILE_FLAGS([-Wextra], [CXXFLAGS])
AX_APPEND_LINK_FLAGS([-rdynamic], [LDFLAGS])

dnl
dnl  Configuration done
dnl

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
    Makefile
    src/Makefile
])

dnl
dnl  Generate makefiles
dnl

AC_OUTPUT
