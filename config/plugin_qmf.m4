#Define ALWAYS WITH_MGMT_$PLUGIN
echo "QMFFFFF"
AC_DEFINE(WITH_MGMT_QMF)

#TODO add here LIB checks

#Add files
AC_CONFIG_FILES([
	src/xdpd/management/plugins/qmf/Makefile
])

