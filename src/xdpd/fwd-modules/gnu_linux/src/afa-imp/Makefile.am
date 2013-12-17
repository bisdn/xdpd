MAINTAINERCLEANFILES = Makefile.in

SUBDIRS=openflow/openflow1x

noinst_LTLIBRARIES = libfwd_module_gnu_linux_afa_imp.la

libfwd_module_gnu_linux_afa_imp_la_SOURCES = \
	forwarding_module.cc	

libfwd_module_gnu_linux_afa_imp_la_LIBADD = \
	openflow/openflow1x/libfwd_module_gnu_linux_afa_imp_of1x.la

AM_LDFLAGS = -static
