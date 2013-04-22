MAINTAINERCLEANFILES = Makefile.in

#Defines driver lib
FM_LIB= fwd-modules/$(PLATFORM)/libfwd_module_$(PLATFORM).la

SUBDIRS = management openflow #fwd-modules

#Main program
sbin_PROGRAMS = xdpd

xdpd_SOURCES = xdpd.cc

xdpd_LDADD = \
	management/libxmgmt.la \
	openflow/libxdpd_openflow.la \
	-lpthread \
	-lrofl \
	-lrofl_pipeline \
	-lrofl_control \
	$(FM_LIB) \
	-ldl

AM_LDFLAGS = -static
