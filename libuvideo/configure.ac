#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([libuvideo], [1.00], [kherbala{at}hotmail.com])

# Checks for host and target systems.
AC_CANONICAL_SYSTEM

# We don't want -g -O2 by default in CFLAGS
: ${CCASFLAGS=""}
: ${CFLAGS=""}
: ${LDFLAGS=""}

AC_PREREQ([2.65])
AM_INIT_AUTOMAKE()
AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADERS([config.h])

# Program name transformations
AC_ARG_PROGRAM

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AM_PROG_AS
AC_PROG_RANLIB

# Checks for libraries.
# none

# Checks for header files.
AC_CHECK_HEADERS()

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# set build compilation flags
BUILD_CCASFLAGS=""
BUILD_CFLAGS=""
BUILD_LDFLAGS=""

case "$target_cpu" in
   i386 | i686)
      BUILD_CCASFLAGS+=" -DARCHI=32 -m32"
      BUILD_CFLAGS+=" -DARCHI=32 -m32"
      ;;
   x86_64)
      BUILD_CCASFLAGS+=" -DARCHI=64 -m64"
      BUILD_CFLAGS+=" -DARCHI=64 -m64"
      ;;
   *)
      echo "Error: CPU not supported : $target_cpu"
      exit 1
      ;;
esac

case "$target_os" in
   linux-gnu)
      BUILD_CCASFLAGS+=" -DLINUX=1"
      BUILD_CFLAGS+=" -DLINUX=1"
      ;;
   mingw32)
      BUILD_CCASFLAGS+=" -DWIN=1"
      BUILD_CFLAGS+=" -DWIN=1"
      ;;
   *)
      echo "Error: OS not supported : $target_os"
      exit 1
      ;;
esac

AC_SUBST(BUILD_CCASFLAGS)
AC_SUBST(BUILD_CFLAGS)
AC_SUBST(BUILD_LDFLAGS)

# Set some conditionnal variables
AM_CONDITIONAL(OS_LINUX, [test x$target_os = xlinux-gnu])
AM_CONDITIONAL(OS_WINDOWS, [test x$target_os = xmingw32])

# Check pkg-config dependencies

## UC ##
AC_ARG_WITH([uc-dir], AS_HELP_STRING([--with-uc-dir=DIR], [location the uc directory]))

if test "x$with_uc_dir" = "x"; then
   PKG_CHECK_MODULES([LIBUC], [libuc], , AC_MSG_ERROR([libuc not found ...]))
   LIBUC_CFLAGS=`pkg-config libuc --cflags`
   LIBUC_LIBS=`pkg-config libuc --libs`
else
   LIBUC_CFLAGS=-I$with_uc_dir/include/uc
   LIBUC_LIBS=$with_uc_dir/lib/libuc.a
fi

AC_SUBST([LIBUC_CFLAGS])
AC_SUBST([LIBUC_LIBS])
## UC END ##


AC_OUTPUT(Makefile src/Makefile libuvideo.pc)


