MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = 

noinst_LTLIBRARIES = libxmgmt.la

libxmgmt_la_SOURCES = \
	switch_manager.cc \
	switch_manager.h \
	port_manager.cc \
	port_manager.h

libxmgmt_la_LIBADD = 

if ENABLE_CONFIG_CLI
SUBDIRS += plugins/cli
CLILIB = plugins/cli/libxdpd_config_cli.la
libxmgmt_la_LIBADD += $(CLILIB)
endif
	
if ENABLE_CONFIG_QMF
SUBDIRS += plugins/qmf
QMFLIB = plugins/qmf/libxdpd_config_qmf.la
libxmgmt_la_LIBADD += $(QMFLIB)
endif
	
