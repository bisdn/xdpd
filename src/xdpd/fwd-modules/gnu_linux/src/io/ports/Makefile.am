MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = \
	mmap \
	mockup\
	vlink	

noinst_LTLIBRARIES = libfwd_module_gnu_linux_io_ports.la

libfwd_module_gnu_linux_io_ports_ladir = $(includedir)/fwd-modules/gnu_linux/io/ports

libfwd_module_gnu_linux_io_ports_la_HEADERS = ioport.h 

libfwd_module_gnu_linux_io_ports_la_SOURCES = \
	ioport.cc \
	ioport.h 

libfwd_module_gnu_linux_io_ports_la_LIBADD = \
	mmap/libfwd_module_gnu_linux_io_ports_mmap.la \
	mockup/libfwd_module_gnu_linux_io_ports_mockup.la\
	vlink/libfwd_module_gnu_linux_io_ports_vlink.la
