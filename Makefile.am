AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

all_includes = -I$(srcdir)/..

include $(top_srcdir)/common.mk

AM_CFLAGS += $(all_includes)
AM_CPPFLAGS += $(all_includes)

lib_LTLIBRARIES = libdjon-nodejs.la
libdjon_nodejs_la_LDFLAGS=-shared# -fPIC
libdjon_nodejs_la_LIBADD = -ldjon-client -lantlr3c -lv8

if !MAC
libdjon_nodejs_la_LIBADD += -lrt -lpthread -luuid
endif
libdjon_nodejs_la_SOURCES= wrapconnection.cpp \
									wrapconnection.h
									
