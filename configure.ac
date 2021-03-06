# Copyright © 2013, 2014 Richard Kettlewell
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
# 02110-1301, USA.
AC_PREREQ([2.61])
AC_INIT([mrtgdf], [2], [rjk@greenend.org.uk])
AC_CONFIG_AUX_DIR([config.aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AC_CONFIG_SRCDIR([src/mrtgdf.cc])
AM_CONFIG_HEADER([config.h])
AC_PROG_CXX
AC_PROG_RANLIB
AC_SET_MAKE

## Dependencies

AC_CHECK_HEADERS([sys/vfs.h sys/param.h sys/mount.h])

## Definitions

AC_DEFINE([_GNU_SOURCE], [1], [use GNU extensions])
AH_BOTTOM([#ifdef __GNUC__
# define attribute(x) __attribute__(x)
#else
# define attribute(x)
#endif])

if test "x$GXX" = xyes; then
  werror="-Werror"
  CXX="$CXX -std=c++11 -Wall -W -Wno-pointer-arith -Wwrite-strings -Wmissing-declarations -Wno-switch"
fi

AC_ARG_ENABLE([werror],[AS_HELP_STRING([--disable-werror],[don't use -Werror])],
              [],[enable_werror=yes])
if test "$enable_werror" = yes; then
  CXX="$CXX $werror"
fi

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
