MAINTAINERCLEANFILES = Makefile.in

SUBDIRS=openflow/openflow1x

noinst_LTLIBRARIES = libxdpd_driver_gnu_linux_hal_imp.la

libxdpd_driver_gnu_linux_hal_imp_la_SOURCES = \
	driver.cc	

libxdpd_driver_gnu_linux_hal_imp_la_LIBADD = \
	openflow/openflow1x/libxdpd_driver_gnu_linux_hal_imp_of1x.la

AM_LDFLAGS = -static
AM_CXXFLAGS = -I$(top_srcdir)/src/

