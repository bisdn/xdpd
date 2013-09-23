MAINTAINERCLEANFILES = Makefile.in

SUBDIRS=openflow

noinst_LTLIBRARIES = libxdpd_mgmt_config.la

libxdpd_mgmt_config_la_SOURCES = \
	scope.cc \
	root_scope.cc \
	config.cc 

libxdpd_mgmt_config_la_LIBADD = \
	openflow/libxdpd_mgmt_config_openflow.la\
	-lconfig++
