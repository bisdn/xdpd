MAINTAINERCLEANFILES = Makefile.in

SUBDIRS=scopes

noinst_LTLIBRARIES = libxdpd_mgmt_config.la

libxdpd_mgmt_config_la_SOURCES = \
	scope.cc \
	config.cc 

libxdpd_mgmt_config_la_LIBADD = \
	scopes/libxdpd_mgmt_config_scopes.la\
	-lconfig++
