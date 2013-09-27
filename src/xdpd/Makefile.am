MAINTAINERCLEANFILES = Makefile.in

#Defines driver lib
FM_LIB= fwd-modules/$(PLATFORM)/libfwd_module_$(PLATFORM).la
FM_LIB_a= fwd-modules/$(PLATFORM)/.libs/libfwd_module_$(PLATFORM).a

SUBDIRS = management openflow fwd-modules

#Main program
sbin_PROGRAMS = xdpd
xdpd_SOURCES = xdpd.cc
xdpd_LDADD = \
	-lrofl_pipeline \
	openflow/.libs/libxdpd_openflow.a\
	$(FM_LIB_a)\
	openflow/.libs/libxdpd_openflow.a\
	management/libxdpd_mgmt.la \
	$(FM_LIB_a)\
	-lpthread \
	-lrofl \
	-lrofl_pipeline \
	-ldl

#The .a is a workaround to prevent libtool to strip it...

AM_LDFLAGS = -static
