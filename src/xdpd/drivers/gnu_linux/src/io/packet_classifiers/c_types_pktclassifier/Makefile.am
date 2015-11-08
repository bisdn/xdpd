MAINTAINERCLEANFILES = Makefile.in

#SUBDIRS = headers

noinst_LTLIBRARIES = libxdpd_driver_gnu_linux_io_classifiers_c_types.la

libxdpd_driver_gnu_linux_io_classifiers_c_types_la_SOURCES = \
	autogen_pkt_types.c\
	c_types_pktclassifier.c \
	../packet_operations.cc

AM_CFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/src/drivers/gnu_linux/src

