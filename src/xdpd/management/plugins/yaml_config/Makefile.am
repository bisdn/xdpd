MAINTAINERCLEANFILES = Makefile.in

SUBDIRS=interfaces openflow system

#Library MUST BE named libxdpd_mgmt_PLUGIN_NAME.la
noinst_LTLIBRARIES = libxdpd_mgmt_yaml_config.la

libxdpd_mgmt_yaml_config_la_SOURCES = \
        scope.cc \
        root_scope.cc \
	    yaml_config.cc

libxdpd_mgmt_yaml_config_la_LIBADD = \
        interfaces/libxdpd_mgmt_yaml_config_interfaces.la\
        openflow/libxdpd_mgmt_yaml_config_openflow.la\
        system/libxdpd_mgmt_yaml_config_system.la\
        -lyaml-cpp

AM_CXXFLAGS = -I$(top_srcdir)/src/

sysconf_DATA = xdpd.yaml

