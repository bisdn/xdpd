MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = plugins/cli

noinst_LTLIBRARIES = libxmgmt.la

libxmgmt_la_SOURCES = \
	switch_manager.cc \
	switch_manager.h \
	port_manager.cc \
	port_manager.h

if ENABLE_CONFIG_CLI
CLILIB = plugins/cli/libxdpd_config_cli.la
endif

libxmgmt_la_LIBADD = \
	$(CLILIB)
