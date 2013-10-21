MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = packet_classifiers

noinst_LTLIBRARIES = libfwd_module_gnu_linux_dpdk_src_io.la

library_includedir=$(includedir)/fwd-modules/gnu_linux_dpdk/io
include_HEADERS=

libfwd_module_gnu_linux_dpdk_src_io_la_SOURCES = \
					bufferpool.h\
					bufferpool.cc\
					datapacketx86.h\
					datapacketx86.cc\
					datapacket_storage.h\
					datapacket_storage.cc\
					port_manager.h\
					port_manager.cc\
					pktin_dispatcher.h\
					pktin_dispatcher.cc
libfwd_module_gnu_linux_dpdk_src_io_la_LIBADD = \
					packet_classifiers/libfwd_module_gnu_linux_dpdk_src_io_packet_classifiers.la 
		
library_include_HEADERS=
