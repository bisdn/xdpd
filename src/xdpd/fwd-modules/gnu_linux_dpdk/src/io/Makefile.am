MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = packet_classifiers

noinst_LTLIBRARIES = libfwd_module_gnu_linux_dpdk_src_io.la

library_includedir=$(includedir)/fwd-modules/gnu_linux_dpdk/io
include_HEADERS=

libfwd_module_gnu_linux_dpdk_src_io_la_SOURCES = \
					datapacketx86.h\
					datapacketx86.cc
libfwd_module_gnu_linux_dpdk_src_io_la_LIBADD = \
					packet_classifiers/libfwd_module_gnu_linux_dpdk_src_io_packet_classifiers.la 
		
library_include_HEADERS=
