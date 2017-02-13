AC_INIT([djondb-nodejs], [0.1], [cross@djondb.com], [djon], [http://d-jon.com/])
AC_PREREQ([2.59])
AM_INIT_AUTOMAKE([1.10 -Wall no-define])
# AC_CONFIG_SUBDIRS(dinnerd libdinner hungryhacker vdinner)

AC_ARG_ENABLE(debug,
AS_HELP_STRING([debug],
               [enable debugging, default: no]),
[case "${enableval}" in
             yes) debug=true ;;
             no)  debug=false ;;
             *)   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
esac],
[debug=false])
AM_CONDITIONAL(DEBUG, test x"$debug" = x"true")

AC_ARG_ENABLE(profile,
AS_HELP_STRING([profile],
               [enable profiling, default: no]),
[case "${enableval}" in
             yes) profile=true ;;
             no)  profile=false ;;
             *)   AC_MSG_ERROR([bad value ${enableval} for --enable-profile]) ;;
esac],
[profile=false])
AM_CONDITIONAL(PROFILE, test x"$profile" = x"true") # AC_CONFIG_SUBDIRS(dinnerd libdinner hungryhacker vdinner)
AC_CONFIG_HEADERS([config.h])
AM_COND_IF([DEBUG], AC_DEFINE([DEBUG], [1], [Define Debug variable]))
AM_COND_IF([PROFILE], AC_DEFINE([PROFILE], [1], [Define Profile variable]))

LT_INIT([disable-static])

UNAME=$(uname -s)

AM_CONDITIONAL(MAC, test x"$UNAME" == x"Darwin")

AC_PROG_CXX
AC_SUBST([DJON_CLIENT_SO_VERSION], [0:1:0])
AC_SUBST([DJON_CLIENT_API_VERSION], [0.1])
AC_SUBST([LIB_NAME], [djon-client])

# LIBS="$X_PRE_LIBS $X_LIBS -lX11 -lXext -lXmu -lXt $X_EXTRA_LIBS -lGL -lGLU -lglut -lm"

#hungryhacker_LDADD = ../libdinner/libdinner.la

#required headers
#AC_CHECK_HEADERS([util.h bson.h],,AC_MSG_ERROR[required header file missing])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
