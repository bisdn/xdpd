MAINTAINERCLEANFILES = Makefile.in

#Do first the config
SUBDIRS = hal-imp io util pipeline-imp processing 

noinst_LTLIBRARIES = libxdpd_driver_gnu_linux_src.la

libxdpd_driver_gnu_linux_src_la_SOURCES = \
	config.cc\
	bg_taskmanager.cc 

libxdpd_driver_gnu_linux_src_la_LIBADD = \
		hal-imp/libxdpd_driver_gnu_linux_hal_imp.la \
		io/libxdpd_driver_gnu_linux_io.la \
		pipeline-imp/libxdpd_driver_gnu_linux_pipeline_imp.la \
		processing/libxdpd_driver_gnu_linux_processing.la \
		util/libxdpd_driver_gnu_linux_util.la

AM_CXXFLAGS = -I$(top_srcdir)/src/

