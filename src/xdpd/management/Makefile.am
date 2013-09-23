MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = 

noinst_LTLIBRARIES = libxdpd_mgmt.la
		
libxdpd_mgmt_la_SOURCES = \
	plugin_manager.cc \
	switch_manager.cc \
	port_manager.cc

libxdpd_mgmt_la_LIBADD =

##Plugins: add yours CONDITIONALLY!
if WITH_MGMT_CONFIG
SUBDIRS += plugins/config
libxdpd_mgmt_la_LIBADD +=plugins/config/libxdpd_mgmt_config.la
endif

if WITH_MGMT_QMF
SUBDIRS += plugins/qmf
libxdpd_mgmt_la_LIBADD +=plugins/qmf/libxdpd_mgmt_qmf.la
endif

if WITH_MGMT_CLI
SUBDIRS += plugins/cli
libxdpd_mgmt_la_LIBADD +=plugins/cli/libxdpd_mgmt_cli.la
endif
