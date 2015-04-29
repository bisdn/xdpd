MAINTAINERCLEANFILES = Makefile.in

SUBDIRS=server

bin_SCRIPTS = xcli
CLEANFILES = $(bin_SCRIPTS)

noinst_LTLIBRARIES = libxdpd_mgmt_rest_js.la libxdpd_mgmt_rest.la

#JSON spirit
libxdpd_mgmt_rest_js_la_SOURCES = 		\
	json_spirit/json_spirit_reader.cpp	\
	json_spirit/json_spirit_value.cpp	\
	json_spirit/json_spirit_writer.cpp
libxdpd_mgmt_rest_js_la_CXXFLAGS = -Wno-unused-local-typedefs

#Rest main library
libxdpd_mgmt_rest_la_SOURCES = 			\
	misc-controllers.cc			\
	port-controllers.cc			\
	sw-controllers.cc			\
	sys-controllers.cc			\
	rest.cc

libxdpd_mgmt_rest_la_LIBADD =			\
	libxdpd_mgmt_rest_js.la			\
	server/libxdpd_mgmt_rest_server.la
