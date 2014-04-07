MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = packet_classifiers

noinst_LTLIBRARIES = libxdpd_driver_netfpga10g_io.la

libxdpd_driver_netfpga10g_io_la_SOURCES = \
					bufferpool.cc\
					datapacket_storage.cc\
					datapacketx86.cc\
					packet_io.cc

CLASSIFIER_LIB=packet_classifiers/c_pktclassifier/libxdpd_driver_netfpga10g_io_classifiers_c.la

libxdpd_driver_netfpga10g_io_la_LIBADD =\
	$(CLASSIFIER_LIB)
