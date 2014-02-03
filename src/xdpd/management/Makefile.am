MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = plugins 

noinst_LTLIBRARIES = libxdpd_mgmt.la

libxdpd_mgmt_la_SOURCES = \
	monitoring_manager.cc \
	plugin_manager.cc \
	switch_manager.cc \
	port_manager.cc

libxdpd_mgmt_la_LIBADD = plugins/libxdpd_mgmt_plugins.la
 
