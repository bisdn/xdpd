MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = afa-imp pipeline-imp#Add more here... 

noinst_LTLIBRARIES = libfwd_module_gnu_linux_dpdk_src.la

library_includedir=$(includedir)/fwd-modules/gnu_linux_dpdk
include_HEADERS=

libfwd_module_gnu_linux_dpdk_src_la_SOURCES =
libfwd_module_gnu_linux_dpdk_src_la_LIBADD = \
		afa-imp/libfwd_module_gnu_linux_dpdk_afa_imp.la\
		pipeline-imp/libfwd_module_gnu_linux_dpdk_pipeline_imp.la 
		
library_include_HEADERS=
