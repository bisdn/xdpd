MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = hal-imp io processing pipeline-imp util#Add more here... 

noinst_LTLIBRARIES = libxdpd_driver_gnu_linux_dpdk_src.la

library_includedir=$(includedir)/fwd-modules/gnu_linux_dpdk
include_HEADERS=

libxdpd_driver_gnu_linux_dpdk_src_la_SOURCES = bg_taskmanager.cc config.cc
libxdpd_driver_gnu_linux_dpdk_src_la_LIBADD = \
		hal-imp/libxdpd_driver_gnu_linux_dpdk_src_hal_imp.la\
		io/libxdpd_driver_gnu_linux_dpdk_src_io.la\
		pipeline-imp/libxdpd_driver_gnu_linux_dpdk_src_pipeline_imp.la\
		processing/libxdpd_driver_gnu_linux_dpdk_src_processing.la\
		util/libxdpd_driver_gnu_linux_dpdk_src_util.la\
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
		-lrte_pmd_vmxnet3_uio\
		-lrte_pmd_virtio_uio\
		-lrte_power\
		-lrte_ring\
		-lrte_timer\
		-lrte_pmd_ring\
		-lrte_eal

library_include_HEADERS=
