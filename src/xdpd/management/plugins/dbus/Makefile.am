MAINTAINERCLEANFILES = Makefile.in

noinst_LTLIBRARIES = libxdpd_mgmt_dbus.la

libxdpd_mgmt_dbus_la_SOURCES = \
	dbusmod.cc \
	dbusmod.h \
	dbuserror.h \
	dbuserror.cc \
	dbusconnection.h \
	dbusconnection.cc

libxdpd_mgmt_dbus_la_LIBADD = \
        -ldbus-1
