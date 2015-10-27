MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = utils 

noinst_LTLIBRARIES = libxdpd_common_base.la

libxdpd_common_base_la_SOURCES = \
		exception.h \
		cparam.h \
		cparam.cc \
		cparams.h \
		cparams.cc

libxdpd_common_base_la_LIBADD=utils/libxdpd_common_utils.la

library_includedir=$(includedir)/xdpd/common
library_include_HEADERS= \
		exception.h
		cparam.h \
		cparams.h

AM_CPPFLAGS=-fPIC
