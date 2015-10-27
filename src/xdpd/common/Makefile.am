MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = utils 

noinst_LTLIBRARIES = libxdpd_common_base.la

libxdpd_common_base_la_SOURCES = \
		common.h \
		common.c

libxdpd_common_base_la_LIBADD=utils/libxdpd_common_utils.la

library_includedir=$(includedir)/xdpd/common
library_include_HEADERS= \
		common.h

AM_CPPFLAGS=-fPIC
