MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = openflow10 openflow12 openflow13 pirl


noinst_LTLIBRARIES = libxdpd_openflow.la

libxdpd_openflow_la_SOURCES = \
	of_endpoint.h \
	of_endpoint.cc \
	openflow_switch.h \
	openflow_switch.cc 
	
libxdpd_openflow_la_LIBADD = \
		pirl/libxdpd_pirl.la \
		openflow10/libxdpd_openflow10.la \
		openflow12/libxdpd_openflow12.la \
		openflow13/libxdpd_openflow13.la
