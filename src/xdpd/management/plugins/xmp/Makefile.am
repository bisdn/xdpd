MAINTAINERCLEANFILES = Makefile.in

noinst_LTLIBRARIES = libxdpd_mgmt_xmp.la
lib_LTLIBRARIES = libxdpd_xmp.la \
	libxdpd_xmp_client.la

libxdpd_mgmt_xmp_la_SOURCES = \
	xmp.h \
	xmp.cc

libxdpd_mgmt_xmp_la_LIBADD = libxdpd_xmp.la

libxdpd_xmp_la_SOURCES = \
	xdpd_mgmt_protocol.h \
	cxmpmsg.h \
	cxmpmsg.cc \
	cxmpie.h \
	cxmpie.cc \
	cxmpie_command.h \
	cxmpie_command.cc \
	cxmpie_dpid.h \
	cxmpie_dpid.cc \
	cxmpie_name.h \
	cxmpie_name.cc \
	cxmpie_portinfo.h \
	cxmpie_portinfo.cc \
	cxmpie_lsiinfo.h \
	cxmpie_lsiinfo.cc \
	cxmpie_controller.h \
	cxmpie_controller.cc \
	cxmpie_lsixlsi.h \
	cxmpie_lsixlsi.cc \
	cxmpie_multipart.cc \
	cxmpie_multipart.h \
	cxmpies.h \
	cxmpies.cc

libxdpd_xmp_ladir=$(includedir)/xdpd/xmp
libxdpd_xmp_la_HEADERS = \
	xdpd_mgmt_protocol.h \
	cxmpmsg.h            \
	cxmpie.h             \
	cxmpie_command.h     \
	cxmpie_dpid.h        \
	cxmpie_name.h        \
	cxmpie_portinfo.h    \
	cxmpie_lsiinfo.h     \
	cxmpie_controller.h  \
	cxmpie_lsixlsi.h     \
	cxmpie_multipart.h   \
	cxmpies.h

libxdpd_xmp_client_ladir=$(includedir)/xdpd/xmp/client
libxdpd_xmp_client_la_HEADERS = \
	tools/cxmpobserver.h \
	tools/cxmpclient.h

libxdpd_xmp_client_la_SOURCES = \
	tools/cxmpobserver.h \
	tools/cxmpobserver.cc \
	tools/cxmpclient.h \
	tools/cxmpclient.cc

sbin_PROGRAMS = xmpclient

xmpclient_SOURCES = \
	tools/main.cc

xmpclient_LDADD = libxdpd_xmp_client.la \
	libxdpd_mgmt_xmp.la \
	libxdpd_xmp.la


pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = xdpd_xmp.pc \
	xdpd_xmp_client.pc
