# configure.ac -- Autoconf script for gps garden-app
#
# Process this file with autoconf to produce a configure script

# Requires autoconf tool later than 2.61
AC_PREREQ(2.61)
# Initialize the gps garden-app package version 1.0.0
AC_INIT([garden-app],1.0.0)
# Does not strictly follow GNU Coding standards
AM_INIT_AUTOMAKE([foreign subdir-objects])
# Disables auto rebuilding of configure, Makefile.ins
AM_MAINTAINER_MODE
# Verifies the --srcdir is correct by checking for the path
AC_CONFIG_SRCDIR([GARDEn/Makefile.am])
# defines some macros variable to be included by source
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_LIBTOOL
AC_PROG_CXX
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_AWK
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
PKG_PROG_PKG_CONFIG

# Checks for libraries.
PKG_CHECK_MODULES([GPSUTILS], [gps-utils])
AC_SUBST([GPSUTILS_CFLAGS])
AC_SUBST([GPSUTILS_LIBS])

PKG_CHECK_MODULES([LOCCLIENTAPI], [location-client-api])
AC_SUBST([LOCCLIENTAPI_CFLAGS])
AC_SUBST([LOCCLIENTAPI_LIBS])

PKG_CHECK_MODULES([LOCINTEGRATIONAPI], [location-integration-api])
AC_SUBST([LOCINTEGRATIONAPI_CFLAGS])
AC_SUBST([LOCINTEGRATIONAPI_LIBS])

PKG_CHECK_MODULES([LOCAPIMSGPROTO], [location-api-msg-proto])
AC_SUBST([LOCAPIMSGPROTO_CFLAGS])

# External AP
AC_ARG_WITH([external_ap],
    AC_HELP_STRING([--with-external_ap=@<:@dir@:>@],
        [Using External Application Processor]),
    [],
    with_external_ap=no)

if test "x$with_external_ap" != "xno"; then
    CPPFLAGS="${CPPFLAGS} -DFEATURE_EXTERNAL_AP"
fi

AM_CONDITIONAL(USE_EXTERNAL_AP, test "x${with_external_ap}" = "xyes")

if (test "x$with_external_ap" == "xno"); then
    PKG_CHECK_MODULES([LOCCORE], [loc-core])
    AC_SUBST([LOCCORE_CFLAGS])
    AC_SUBST([LOCCORE_LIBS])

    PKG_CHECK_MODULES([LOCATIONAPI], [location-api])
    AC_SUBST([LOCATIONAPI_CFLAGS])
    AC_SUBST([LOCATIONAPI_LIBS])

    ## include diag modules
    PKG_CHECK_MODULES([LOCDIAGIFACE], [loc-diag-iface])
    AC_SUBST([LOCDIAGIFACE_CFLAGS])
    AC_SUBST([LOCDIAGIFACE_LIBS])

    PKG_CHECK_MODULES([DIAG], [diag])
    AC_SUBST([DIAG_CFLAGS])
    AC_SUBST([DIAG_LIBS])

    PKG_CHECK_MODULES([DATAITEMS], [data-items])
    AC_SUBST([DATAITEMS_CFLAGS])
    AC_SUBST([DATAITEMS_LIBS])

    PKG_CHECK_MODULES([IZATCLIENTAPI], [izat-client-api])
    AC_SUBST([IZATCLIENTAPI_CFLAGS])
    AC_SUBST([IZATCLIENTAPI_LIBS])

    PKG_CHECK_MODULES([LBSCORE], [lbs-core])
    AC_SUBST([LBSCORE_CFLAGS])
    AC_SUBST([LBSCORE_LIBS])
else
    AC_MSG_NOTICE([Target does not supports QCMAP.])
fi


AC_ARG_WITH([auto_feature],
      AC_HELP_STRING([--with-auto_feature=@<:@dir@:>@],
         [Using Automotive feature]))

if test "x$with_auto_feature" = "xyes"; then
        PKG_CHECK_MODULES([CDFW], [cdfw])
        AC_SUBST([CDFW_CFLAGS])
        AC_SUBST([CDFW_LIBS])
        PKG_CHECK_MODULES([CDFWREMOTEAPI], [cdfw-remote-api])
        AC_SUBST([CDFWREMOTEAPI_CFLAGS])
        AC_SUBST([CDFWREMOTEAPI_LIBS])
fi

AM_CONDITIONAL(USE_FEATURE_AUTOMOTIVE, test "x${with_auto_feature}" = "xyes")

AC_ARG_WITH([locpla_includes],
      AC_HELP_STRING([--with-locpla-includes=@<:@dir@:>@],
         [specify the path to locpla-includes in loc-pla_git.bb]),
      [locpla_incdir=$withval],
      with_locpla_includes=no)

if test "x$with_locpla_includes" != "xno"; then
   AC_SUBST(LOCPLA_CFLAGS, "-I${locpla_incdir}")
fi

AC_ARG_WITH([core_includes],
      AC_HELP_STRING([--with-core-includes=@<:@dir@:>@],
         [Specify the location of the core headers]),
      [core_incdir=$withval],
      with_core_includes=no)

if test "x$with_core_includes" != "xno"; then
   CPPFLAGS="${CPPFLAGS} -I${core_incdir}"
fi

AC_SUBST([CPPFLAGS])

AC_ARG_WITH([glib],
      AC_HELP_STRING([--with-glib],
         [enable glib, building HLOS systems which use glib]))

if (test "x${with_glib}" = "xyes"); then
        AC_DEFINE(ENABLE_USEGLIB, 1, [Define if HLOS systems uses glib])
        PKG_CHECK_MODULES(GTHREAD, gthread-2.0 >= 2.16, dummy=yes,
                                AC_MSG_ERROR(GThread >= 2.16 is required))
        PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.16, dummy=yes,
                                AC_MSG_ERROR(GLib >= 2.16 is required))
        GLIB_CFLAGS="$GLIB_CFLAGS $GTHREAD_CFLAGS"
        GLIB_LIBS="$GLIB_LIBS $GTHREAD_LIBS"

        AC_SUBST(GLIB_CFLAGS)
        AC_SUBST(GLIB_LIBS)
fi

AM_CONDITIONAL(USE_GLIB, test "x${with_glib}" = "xyes")

AC_CONFIG_FILES([ \
        Makefile \
        GARDEn/Makefile \
        garden-app.pc \
        ])

AC_OUTPUT
