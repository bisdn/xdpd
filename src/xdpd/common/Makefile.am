MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = utils 

noinst_LTLIBRARIES = libxdpd_common_base.la

libxdpd_common_base_la_SOURCES = \
		exception.h \
		logging.h \
		logging.cc \
		cparam.h \
		cparam.cc \
		cparams.h \
		cparams.cc \
		cunixenv.h \
		cunixenv.cc \
		cdaemon.h \
		cdaemon.cc \
		cpipe.h \
		cpipe.cc \
		csocket.h \
		csocket.cc \
		csocket_strings.h

libxdpd_common_base_la_LIBADD=utils/libxdpd_common_utils.la

library_includedir=$(includedir)/xdpd/common
#library_include_HEADERS= \
#		exception.h
#		logging.h \
#		cparam.h \
#		cparams.h \
#		cunixenv.h \
#		cdaemon.h \
#		cpipe.h \
#		csocket.h \
#		csocket_strings.h

AM_CPPFLAGS=-fPIC
AM_CXXFLAGS=-I$(top_srcdir)/src/

