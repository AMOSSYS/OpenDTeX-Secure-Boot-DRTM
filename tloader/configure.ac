#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([tloader], [1.00], [goulven.guiheux{at}hotmail.com])

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

# Checks for header files.
AC_CHECK_HEADERS()

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC

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

## UVIDEO ##
AC_ARG_WITH([uvideo-dir], AS_HELP_STRING([--with-uvideo-dir=DIR], [location the uvideo directory]))

if test "x$with_uvideo_dir" = "x"; then
   PKG_CHECK_MODULES([LIBUVIDEO], [libuvideo], , AC_MSG_ERROR([libuvideo not found ...]))
   LIBUVIDEO_CFLAGS=`pkg-config libuvideo --cflags`
   LIBUVIDEO_LIBS=`pkg-config libuvideo --libs`
else
   LIBUVIDEO_CFLAGS=-I$with_uvideo_dir/include/uvideo
   LIBUVIDEO_LIBS=$with_uvideo_dir/lib/libuvideo.a
fi

AC_SUBST([LIBUVIDEO_CFLAGS])
AC_SUBST([LIBUVIDEO_LIBS])
## UVIDEO END ##

## TPM ##
AC_ARG_WITH([tpm-dir], AS_HELP_STRING([--with-tpm-dir=DIR], [location the libtpm directory]))

if test "x$with_tpm_dir" = "x"; then
   PKG_CHECK_MODULES([LIBTPM], [libtpm], , AC_MSG_ERROR([libtpm not found ...]))
   LIBTPM_CFLAGS=`pkg-config libtpm --cflags`
   LIBTPM_LIBS=`pkg-config libtpm --libs`
else
   LIBTPM_CFLAGS=-I$with_tpm_dir/include/libtpm
   LIBTPM_LIBS=$with_tpm_dir/lib/libtpm.a
fi

AC_SUBST([LIBTPM_CFLAGS])
AC_SUBST([LIBTPM_LIBS])
## TPM END ##

## TXT ##
AC_ARG_WITH([txt-dir], AS_HELP_STRING([--with-txt-dir=DIR], [location the libtxt directory]))

if test "x$with_txt_dir" = "x"; then
   PKG_CHECK_MODULES([LIBTXT], [libtxt], , AC_MSG_ERROR([libtxt not found ...]))
   LIBTXT_CFLAGS=`pkg-config libtxt --cflags`
   LIBTXT_LIBS=`pkg-config libtxt --libs`
else
   LIBTXT_CFLAGS=-I$with_txt_dir/include/libtxt
   LIBTXT_LIBS=$with_txt_dir/lib/libtxt.a
fi

AC_SUBST([LIBTXT_CFLAGS])
AC_SUBST([LIBTXT_LIBS])
## TXT END ##

AC_OUTPUT(Makefile src/Makefile)


