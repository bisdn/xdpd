MAINTAINERCLEANFILES = \
	$(top_srcdir)/aclocal.m4 \
	$(top_srcdir)/configure \
	$(top_srcdir)/Makefile.in

SUBDIRS = src test 

noinst_LTLIBRARIES = libxdpd_driver_gnu_linux.la

libxdpd_driver_gnu_linux_la_SOURCES = 

libxdpd_driver_gnu_linux_la_LIBADD = \
		src/libxdpd_driver_gnu_linux_src.la

maintainer-clean-local:
	rm -rf $(top_srcdir)/m4
	rm -rf $(top_srcdir)/build-aux
