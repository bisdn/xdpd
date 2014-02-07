MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = packet_classifiers

noinst_LTLIBRARIES = libfwd_module_netfpga10g_io.la

libfwd_module_netfpga10g_io_la_SOURCES = \
					bufferpool.cc\
					datapacket_storage.cc\
					datapacketx86.cc\
					packet_io.cc

if C_CLASSIFIER
CLASSIFIER_LIB=packet_classifiers/c_pktclassifier/libfwd_module_netfpga10g_io_classifiers_c.la
else
CLASSIFIER_LIB=packet_classifiers/cpp_pktclassifier/libfwd_module_netfpga10g_io_classifiers_cpp.la
endif

libfwd_module_netfpga10g_io_la_LIBADD =\
	$(CLASSIFIER_LIB)
