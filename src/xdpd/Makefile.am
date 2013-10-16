MAINTAINERCLEANFILES = Makefile.in

#Defines driver lib
FM_LIB= fwd-modules/$(PLATFORM)/libfwd_module_$(PLATFORM).la
FM_LIB_a= fwd-modules/$(PLATFORM)/.libs/libfwd_module_$(PLATFORM).a

SUBDIRS = management openflow fwd-modules

#Wrap
noinst_LTLIBRARIES = libxdpd_wrap.la
libxdpd_wrap_la_SOURCES = 
libxdpd_wrap_la_LIBADD = \
	management/libxdpd_mgmt.la \
	-lrofl_pipeline \
	openflow/libxdpd_openflow.la\
	$(FM_LIB)

#Main program
sbin_PROGRAMS = xdpd
xdpd_SOURCES = cmm.cc xdpd.cc
xdpd_LDADD = \
	libxdpd_wrap.la\
	$(LIBS)\
	openflow/libxdpd_openflow.la\
	-lrofl_pipeline \
	$(FM_LIB)\
	-lpthread \
	-lrofl \
	-ldl

#The .a is a workaround to prevent libtool to strip it...

AM_LDFLAGS = -static
