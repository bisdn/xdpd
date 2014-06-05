MAINTAINERCLEANFILES = Makefile.in

#Library MUST BE named libxdpd_mgmt_PLUGIN_NAME.la
noinst_LTLIBRARIES = libxdpd_mgmt_node_orchestrator.la

libxdpd_mgmt_node_orchestrator_la_SOURCES = \
	node_orchestrator.cc \
	LSI.cc \
	message_handler.cc

libxdpd_mgmt_node_orchestrator_la_LIBADD = \
	-ljson_spirit
