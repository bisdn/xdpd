MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = \
	mmap \
	mockup\
	vlink	

noinst_LTLIBRARIES = libxdpd_driver_gnu_linux_io_ports.la

libxdpd_driver_gnu_linux_io_ports_la_SOURCES = \
	ioport.cc \
	ioport.h 

libxdpd_driver_gnu_linux_io_ports_la_LIBADD = \
	mmap/libxdpd_driver_gnu_linux_io_ports_mmap.la \
	mockup/libxdpd_driver_gnu_linux_io_ports_mockup.la\
	vlink/libxdpd_driver_gnu_linux_io_ports_vlink.la
