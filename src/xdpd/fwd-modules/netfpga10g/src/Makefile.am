MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = afa-imp netfpga pipeline-imp#Add more here... 

noinst_LTLIBRARIES = libfwd_module_netfpga10g_src.la

library_includedir=$(includedir)/fwd-modules/netfpga10g
include_HEADERS=

libfwd_module_netfpga10g_src_la_SOURCES =
libfwd_module_netfpga10g_src_la_LIBADD = \
		afa-imp/libfwd_module_netfpga10g_afa_imp.la\
		netfpga/libfwd_module_netfpga10g_netfpga.la\
		pipeline-imp/libfwd_module_netfpga10g_pipeline_imp.la 
		
library_include_HEADERS=
