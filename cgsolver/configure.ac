#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.59])
AC_INIT([GP], [1.0], [lintao87@gmail.com])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile])
AC_CONFIG_MACRO_DIR([m4])
#AC_CONFIG_SUBDIRS(lemon)

# Checks for programs.
AC_PROG_CXX

# Checks for openmp support
AC_OPENMP

# Checks for libraries.
#AC_CHECK_LIB(emon, [], [lemon_flag=yes], [lemon_flag=no])


#if test x"$lemon_flag" == x"no"; then
#	AC_MSG_ERROR([Lemon not found])
#fi



# Checks for header files.
AC_HEADER_STDC
#AC_CHECK_HEADERS(/lemon/include/lemon/list_graph.h /lemon/include/lemon/network_simplex.h)

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T

# Checks for library functions.

AC_OUTPUT
