MAINTAINERCLEANFILES = Makefile.in

#Defines driver lib
FM_LIB= fwd-modules/$(PLATFORM)/libfwd_module_$(PLATFORM).la

SUBDIRS = management openflow fwd-modules

#Wraps hal driver and pipeline for static linking (cheating on libtool)
#noinst_LTLIBRARIES= libhal_wrapper.la
#libhal_wrapper_la_SOURCES=
#libhal_wrapper_la_LIBADD= $(FM_LIB) -lrofl_pipeline $(FM_LIB)

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
#	libhal_wrapper.la
#config/libxdpd_config.la 
