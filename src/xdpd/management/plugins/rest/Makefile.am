MAINTAINERCLEANFILES = Makefile.in

SUBDIRS=server

#Library MUST BE named libxdpd_mgmt_PLUGIN_NAME.la
noinst_LTLIBRARIES = libxdpd_mgmt_rest.la

libxdpd_mgmt_rest_la_SOURCES = \
  endpoints.cpp \
	rest.cc 

libxdpd_mgmt_rest_la_LIBADD = \
  server/libxdpd_mgmt_rest_server.la
