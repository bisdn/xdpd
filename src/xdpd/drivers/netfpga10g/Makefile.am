MAINTAINERCLEANFILES =  \
	$(top_srcdir)/aclocal.m4 \
	$(top_srcdir)/configure \
	$(top_srcdir)/Makefile.in

SUBDIRS = src #test 

noinst_LTLIBRARIES = libxdpd_driver_netfpga10g.la

libxdpd_driver_netfpga10g_la_SOURCES = 

libxdpd_driver_netfpga10g_la_LIBADD = src/libxdpd_driver_netfpga10g_src.la -lpcap

maintainer-clean-local:
	rm -rf $(top_srcdir)/m4
	rm -rf $(top_srcdir)/build-aux

ACLOCAL_AMFLAGS = -I m4