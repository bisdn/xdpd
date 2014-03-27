MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = hal-imp io netfpga pipeline-imp util

noinst_LTLIBRARIES = libxdpd_driver_netfpga10g_src.la

libxdpd_driver_netfpga10g_src_la_SOURCES = bg_taskmanager.cc config.cc 
libxdpd_driver_netfpga10g_src_la_LIBADD = \
		hal-imp/libxdpd_driver_netfpga10g_hal_imp.la\
		io/libxdpd_driver_netfpga10g_io.la\
		netfpga/libxdpd_driver_netfpga10g_netfpga.la\
		pipeline-imp/libxdpd_driver_netfpga10g_pipeline_imp.la\
		util/libxdpd_driver_netfpga10g_util.la \
		-lpcap
