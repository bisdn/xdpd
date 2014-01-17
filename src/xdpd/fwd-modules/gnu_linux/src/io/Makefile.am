MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = packet_classifiers ports scheduler

noinst_LTLIBRARIES = libfwd_module_gnu_linux_io.la

libfwd_module_gnu_linux_io_ladir = $(includedir)/fwd-modules/gnu_linux/io

libfwd_module_gnu_linux_io_la_HEADERS =

libfwd_module_gnu_linux_io_la_SOURCES = \
	bufferpool.cc \
	bufferpool.h \
	pktin_dispatcher.cc \
	pktin_dispatcher.h \
	datapacket_storage.cc \
	datapacket_storage.h \
	datapacketx86.cc \
	datapacketx86.h \
	iface_utils.cc\
	iface_utils.h\
	iomanager.cc \
	iomanager.cc \
	iomanager.h

libfwd_module_gnu_linux_io_la_LIBADD = \
	packet_classifiers/libfwd_module_gnu_linux_io_classifiers.la \
	ports/libfwd_module_gnu_linux_io_ports.la \
	scheduler/libfwd_module_gnu_linux_io_scheduler.la
