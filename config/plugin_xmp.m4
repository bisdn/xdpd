#Define ALWAYS WITH_MGMT_$PLUGIN
AC_DEFINE(WITH_MGMT_XMP)

#TODO add here LIB checks

#Add files
AC_CONFIG_FILES([
	src/xdpd/management/plugins/xmp/Makefile
	src/xdpd/management/plugins/xmp/tools/Makefile
])

