MAINTAINERCLEANFILES = Makefile.in

#Defines driver lib
FM_LIB= fwd-modules/$(PLATFORM)/libfwd_module_$(PLATFORM).la

SUBDIRS = management openflow fwd-modules

noinst_LTLIBRARIES = libxdpd_wrap.la
libxdpd_wrap_la_SOURCES = 
libxdpd_wrap_la_LIBADD = \
	management/libxdpd_mgmt.la \
	openflow/libxdpd_openflow.la 

#Main program
sbin_PROGRAMS = xdpd
xdpd_SOURCES = xdpd.cc
xdpd_LDADD = \
	-lpthread \
	-lrofl \
	-lrofl_pipeline \
	management/libxdpd_mgmt.la \
	openflow/libxdpd_openflow.la\
	$(FM_LIB) \
	libxdpd_wrap.la \
	-ldl

AM_LDFLAGS = -static
