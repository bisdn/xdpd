MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = afa-imp io processing pipeline-imp#Add more here... 

noinst_LTLIBRARIES = libfwd_module_gnu_linux_dpdk_src.la

library_includedir=$(includedir)/fwd-modules/gnu_linux_dpdk
include_HEADERS=

libfwd_module_gnu_linux_dpdk_src_la_SOURCES = config.cc
libfwd_module_gnu_linux_dpdk_src_la_LIBADD = \
		afa-imp/libfwd_module_gnu_linux_dpdk_src_afa_imp.la\
		io/libfwd_module_gnu_linux_dpdk_src_io.la\
		pipeline-imp/libfwd_module_gnu_linux_dpdk_src_pipeline_imp.la\
		processing/libfwd_module_gnu_linux_dpdk_src_processing.la\
		-lrte_eal\
		-lethdev\
		-lrte_hash\
		-lrte_kni\
		-lrte_lpm\
		-lrte_malloc\
		-lrte_mbuf\
		-lrte_mempool\
		-lrte_pmd_e1000\
		-lrte_pmd_ixgbe\
		-lrte_ring\
		-lrte_timer
library_include_HEADERS=
