MAINTAINERCLEANFILES = Makefile.in

#Defines driver lib
FM_LIB= fwd-modules/$(PLATFORM)/libfwd_module_$(PLATFORM).la
FM_LIB_a= fwd-modules/$(PLATFORM)/.libs/libfwd_module_$(PLATFORM).a

SUBDIRS = management openflow fwd-modules

#Main program
sbin_PROGRAMS = xdpd
xdpd_SOURCES = cmm.cc xdpd.cc
xdpd_LDADD = \
	openflow/libxdpd_openflow.la\
	-lrofl_pipeline \
	$(FM_LIB)\
	openflow/libxdpd_openflow.la\
	-lrofl_pipeline \
	management/libxdpd_mgmt.la \
	openflow/libxdpd_openflow.la\
	$(FM_LIB)\
	-lpthread \
	-lrofl \
	-ldl

#The .a is a workaround to prevent libtool to strip it...

AM_LDFLAGS = -static
