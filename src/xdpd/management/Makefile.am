MAINTAINERCLEANFILES = Makefile.in

##Plugins: add yours CONDITIONALLY!
if WITH_MGMT_CONFIG
PLUGIN_DIRS += plugins/config
PLUGIN_LIBS += plugins/config/libxdpd_mgmt_config.la
endif

if WITH_MGMT_QMF
PLUGIN_DIRS += plugins/qmf
PLUGIN_LIBS += plugins/qmf/libxdpd_mgmt_qmf.la
endif

if WITH_MGMT_EXAMPLE
PLUGIN_DIRS += plugins/example
PLUGIN_LIBS += plugins/example/libxdpd_mgmt_example.la
endif

#Add more here...

SUBDIRS = $(PLUGIN_DIRS)

noinst_LTLIBRARIES = libxdpd_mgmt.la
		
libxdpd_mgmt_la_SOURCES = \
	plugin_manager.cc \
	switch_manager.cc \
	port_manager.cc

libxdpd_mgmt_la_LIBADD = $(PLUGIN_LIBS)
