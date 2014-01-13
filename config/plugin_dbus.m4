#Define ALWAYS WITH_MGMT_$PLUGIN
AC_DEFINE(WITH_MGMT_DBUS)

CPPFLAGS+="-I/usr/include/dbus-1.0 -I/usr/lib64/dbus-1.0/include/"
LIBS+="-ldbus-1"

#Add files
AC_CONFIG_FILES([
	src/xdpd/management/plugins/dbus/Makefile
])

