MAINTAINERCLEANFILES = Makefile.in

#SUBDIRS = headers

noinst_LTLIBRARIES = libfwd_module_netfpga10g_io_classifiers_c.la

libfwd_module_netfpga10g_io_classifiers_c_ladir = $(includedir)/fwd-modules/netfpga10g/io/packet_classifiers/c_pktclassifier

libfwd_module_netfpga10g_io_classifiers_c_la_HEADERS = c_pktclassifier.h \
	cpc_utils.h \
	headers/cpc_arpv4.h \
	headers/cpc_ethernet.h \
	headers/cpc_gtpu.h \
	headers/cpc_icmpv4.h \
	headers/cpc_icmpv6.h \
	headers/cpc_ipv4.h \
	headers/cpc_ipv6.h \
	headers/cpc_mpls.h \
	headers/cpc_ppp.h \
	headers/cpc_pppoe.h \
	headers/cpc_tcp.h \
	headers/cpc_udp.h \
	headers/cpc_vlan.h \
	../packet_operations.h

libfwd_module_netfpga10g_io_classifiers_c_la_SOURCES = \
	c_pktclassifier.c \
	../packet_operations.cc
