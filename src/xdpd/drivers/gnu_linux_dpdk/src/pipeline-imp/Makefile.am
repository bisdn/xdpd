MAINTAINERCLEANFILES = Makefile.in

noinst_LTLIBRARIES = libxdpd_driver_gnu_linux_dpdk_src_pipeline_imp.la

libxdpd_driver_gnu_linux_dpdk_src_pipeline_imp_la_SOURCES = \
					memory.c\
					packet.cc\
					platform_hooks_of1x.cc\
					rte_atomic_operations.c\
					timing.c
				
#pthread	
libxdpd_driver_gnu_linux_dpdk_src_pipeline_imp_la_SOURCES += pthread_lock.c

#rte
#libxdpd_driver_gnu_linux_dpdk_src_pipeline_imp_la_SOURCES += rte_lock.c

libxdpd_driver_gnu_linux_dpdk_src_pipeline_imp_la_LIBADD = 
