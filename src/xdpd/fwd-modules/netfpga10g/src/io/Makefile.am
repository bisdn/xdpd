MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = packet_classifiers

noinst_LTLIBRARIES = libfwd_module_netfpga10g_io.la

libfwd_module_netfpga10g_io_la_SOURCES = \
					bufferpool.cc\
					datapacket_storage.cc\
					datapacketx86.cc\
					packet_io.cc

libfwd_module_netfpga10g_io_la_LIBADD = packet_classifiers/libfwd_module_netfpga10g_io_classifiers.la 
