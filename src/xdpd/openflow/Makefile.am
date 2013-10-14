MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = openflow10 openflow12


noinst_LTLIBRARIES = libxdpd_openflow.la

libxdpd_openflow_la_SOURCES = \
	of_endpoint.h \
	openflow_switch.h \
	openflow_switch.cc 
	
libxdpd_openflow_la_LIBADD = \
		openflow10/libxdpd_openflow10.la \
		openflow12/libxdpd_openflow12.la
	
