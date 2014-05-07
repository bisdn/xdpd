MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = packet_classifiers

noinst_LTLIBRARIES = libxdpd_driver_gnu_linux_dpdk_src_io.la

libxdpd_driver_gnu_linux_dpdk_src_io_la_SOURCES = \
					bufferpool.h\
					bufferpool.cc\
					dpdk_datapacket.h \
					dpdk_datapacket.c \
					datapacket_storage.h\
					datapacket_storage.cc\
					iface_manager.h\
					iface_manager.cc\
					pktin_dispatcher.h\
					pktin_dispatcher.cc\
					tx.cc\
					rx.cc

CLASSIFIER_LIB=packet_classifiers/c_pktclassifier/libxdpd_driver_gnu_linux_dpdk_io_classifiers_c.la
					
libxdpd_driver_gnu_linux_dpdk_src_io_la_LIBADD = \
					$(CLASSIFIER_LIB)
