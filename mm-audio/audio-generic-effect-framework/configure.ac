#                                               -*- Autoconf -*-
# configure.ac -- Autoconf script for audio-generic-effect-framework
#

# Process this file with autoconf to produce a configure script.

# Requires autoconf tool later than 2.61
AC_PREREQ([2.69])
# Initialize the audio-effects package version 1.0.0
AC_INIT(audio-generic-effect-framework,1.0.0)
# Does not strictly follow GNU Coding standards
AM_INIT_AUTOMAKE([foreign])
# Disables auto rebuilding of configure, Makefile.ins
AM_MAINTAINER_MODE
# Check for the source file path
AC_CONFIG_SRCDIR([src/qti_gef_api.c])
AC_CONFIG_HEADERS([config.h])
# defines some macros variable to be included by source
AC_CONFIG_MACRO_DIR([m4])

AM_CONDITIONAL([INSTANCE_ID], [test x$AUDIO_FEATURE_ENABLED_INSTANCE_ID = xtrue])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_PROG_AWK
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
PKG_PROG_PKG_CONFIG

PKG_CHECK_MODULES([CUTILS], [libcutils])
AC_SUBST([CUTILS_CFLAGS])
AC_SUBST([SYSTEM_MEDIA_CFLAGS],[-I$(includedir)/])

AC_ARG_WITH(sanitized-headers,
        AS_HELP_STRING([--with-sanitized-headers=DIR],
                [Specify the location of the sanitized Linux headers]),
        [CPPFLAGS="$CPPFLAGS -idirafter $withval"])

AC_CONFIG_FILES([Makefile \
                 audio-generic-effect-framework.pc])

AC_OUTPUT
