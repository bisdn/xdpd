MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = $(PLUGIN_DIRS)
DIST_SUBDIRS = $(PLUGIN_DIRS)

noinst_LTLIBRARIES = libxdpd_mgmt.la
		
libxdpd_mgmt_la_SOURCES = \
	plugin_manager.cc \
	switch_manager.cc \
	port_manager.cc

libxdpd_mgmt_la_LIBADD = $(PLUGIN_LIBS)
