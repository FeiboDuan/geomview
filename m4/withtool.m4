dnl Copyright (C) 2007 Claus-Justus Heine
dnl  
dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2 of the License, or
dnl (at your option) any later version.
dnl 
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl 
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software 
dnl Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
dnl 
dnl As a special exception to the GNU General Public License, if you 
dnl distribute this file as part of a program that contains a configuration 
dnl script generated by Autoconf, you may include it under the same 
dnl distribution terms that you use for the rest of that program.

# Find a program, with user override.
#
# $1: symbolic name, fancy. Converted to uppercase for the defintion
#     of variables
# $2: default program to check for. Should be _without_ path,
#     just the name
# $3: optional search path
#
# Example:
#
# GV_PATH_PROG(tk-interpreter, wish, [$PATH:blah])

AC_DEFUN([GV_PATH_PROG],
[m4_define([UPNAME], [m4_bpatsubst(m4_toupper([$1]),-,_)])
m4_if($#,1,
     [m4_define([PROG],[$1])],[m4_define([PROG],[$2])])
AC_ARG_WITH($1,
  AC_HELP_STRING([--with-$1=PROGRAM],
[Set PROGRAM to the name of the `$1' program (usually `PROG').
(default: autodetected)]),
  [case "${withval}" in
    no)
      AC_MSG_ERROR(["--without-$1" or "--with-$1=no" is not an option here])
      ;;
    yes) # simply ignore that, use auto-detection
      ;;
    *) 
      UPNAME[OPT]="${withval}"
      ;;
    esac],
  [UPNAME[OPT]=PROG])
acgv_path=`dirname "${UPNAME[OPT]}"|sed -e 's|^\./\?||g'`
if test -n "${acgv_path}"; then
  UPNAME=`basename "${UPNAME[OPT]}"`
  if echo "${acgv_path}" | egrep -q -s '^/'; then
    acgv_path="${acgv_path}"
  else
    acgv_path="`pwd`/${acgv_path}"
  fi
  AC_PATH_PROGS(UPNAME, ${UPNAME}, [not found], [${acgv_path}])
else
  UPNAME="${UPNAME[OPT]}"
  m4_if($#,3,[acgv_path="$3:${PATH}"],[acgv_path="${PATH}"])
  AC_CHECK_PROGS(UPNAME, ${UPNAME}, [not found], [${acgv_path}])
fi
if test "${UPNAME}" = "not found"; then
  AC_MSG_ERROR([`$1' executable not found. Check your installation.])
  exit 1
fi])
