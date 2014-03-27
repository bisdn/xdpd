MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = afa-imp io netfpga pipeline-imp util

noinst_LTLIBRARIES = libfwd_module_netfpga10g_src.la

library_includedir=$(includedir)/fwd-modules/netfpga10g
include_HEADERS=

libfwd_module_netfpga10g_src_la_SOURCES = bg_taskmanager.cc config.cc 
libfwd_module_netfpga10g_src_la_LIBADD = \
		afa-imp/libfwd_module_netfpga10g_afa_imp.la\
		io/libfwd_module_netfpga10g_io.la\
		netfpga/libfwd_module_netfpga10g_netfpga.la\
		pipeline-imp/libfwd_module_netfpga10g_pipeline_imp.la\
		util/libfwd_module_netfpga10g_util.la \
		-lpcap
		
library_include_HEADERS=
